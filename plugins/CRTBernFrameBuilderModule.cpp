/**
 * @file CRTBernFrameBuilderModule.cpp

 * Reads data from the HW then puts it in a queue
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CRTBernFrameBuilderModule.hpp"

#include "crtmodules/opmon/CRTBernFrameBuilderModule.pb.h"

#include "datahandlinglibs/utils/RateLimiter.hpp"
#include "datahandlinglibs/DataHandlingIssues.hpp"

#include "appmodel/DetectorFrameBuilderModule.hpp"
#include "appmodel/SocketDetectorToDaqConnection.hpp"
#include "appmodel/NWDetDataSender.hpp"

#include "confmodel/QueueWithSourceId.hpp"
#include "confmodel/DetectorStream.hpp"
#include "confmodel/GeoId.hpp"

#include "detdataformats/DetID.hpp"

#include <utility>
#include <memory>
#include <string>

DUNE_DAQ_TYPESTRING(dunedaq::fddetdataformats::CRTBernFrame, "CRTBernFrame")

namespace dunedaq::crtmodules {

/**
 * @brief Maximum packet sequence ID before reset
 */
constexpr uint64_t max_seq_id = 4095; // NOLINT(build/unsigned)

/**
 * @brief Fake packet detector ID
 */
constexpr uint8_t fake_det_id = static_cast<uint8_t>(detdataformats::DetID::Subdetector::kVD_BernCRT); // NOLINT(build/unsigned)

/**
 * @brief Fake packet block length
 */
constexpr uint64_t fake_block_length = 0x382; // NOLINT(build/unsigned)

/**
 * @brief Calculate the next fake sequence ID for a packet
 * @param seq_id Fake packet sequence ID
 */
void
fake_sequence_id(uint64_t& seq_id) // NOLINT(build/unsigned)
{
  seq_id = (seq_id == max_seq_id ? 0 : seq_id+1);
}

/**
 * @brief Calculate the next fake timestamp for a packet
 * @param timestamp Fake packet timestamp
 */
void
fake_timestamp(uint64_t& timestamp) // NOLINT(build/unsigned)
{
    auto time_now = std::chrono::steady_clock::now().time_since_epoch();
    uint64_t current_time = // NOLINT (build/unsigned)
    std::chrono::duration_cast<std::chrono::nanoseconds>(time_now).count();
    timestamp = current_time / 16; // 625/10000 (same as 625*us/10)
}

/**
 * @brief Fake ADC of the given packet
 * @param frame Fake packet
 */
void
fake_adc(fddetdataformats::CRTBernFrame& frame)
{
  for (int channel = 0; channel < fddetdataformats::CRTBernFrame::s_num_channels; ++channel) {
    frame.set_adc(channel, 0);
  }
}

/**
 * @brief Create a fake packet
 * @param frame Fake packet
 * @param seq_id Fake packet sequence ID
 * @param timestamp Fake packet timestamp
 * @param stream_id Fake packet stream ID
 */
void
fake_data(fddetdataformats::CRTBernFrame& frame, uint64_t& seq_id, uint64_t& timestamp, uint32_t stream_id) // NOLINT(build/unsigned)
{
  frame.daq_header.det_id = fake_det_id & 0x3f; //6 bits for det id
  frame.daq_header.crate_id = 1;
  frame.daq_header.slot_id = 1;
  frame.daq_header.stream_id = stream_id;
  fake_sequence_id(seq_id);
  frame.daq_header.seq_id = seq_id;
  frame.daq_header.block_length = fake_block_length;
  fake_timestamp(timestamp);
  frame.daq_header.timestamp = timestamp;
  fake_adc(frame);
}

CRTBernFrameBuilderModule::CRTBernFrameBuilderModule(const std::string& name)
  : DAQModule(name)
{
  register_command("conf", &CRTBernFrameBuilderModule::do_conf);
  register_command("start", &CRTBernFrameBuilderModule::do_start);
  register_command("stop_trigger_sources", &CRTBernFrameBuilderModule::do_stop);
  register_command("scrap", &CRTBernFrameBuilderModule::do_scrap);
}

void
CRTBernFrameBuilderModule::init(const std::shared_ptr<appfwk::ConfigurationManager> mcfg)
{
  auto* mdal = mcfg->get_dal<appmodel::DetectorFrameBuilderModule>(get_name());
    
  auto* d2d_conn = mdal->get_connection();
  auto* socket_d2d_conn = d2d_conn->cast<appmodel::SocketDetectorToDaqConnection>();
  if (socket_d2d_conn == nullptr) {
    auto err = datahandlinglibs::InitializationError(ERS_HERE, "Connection is not of type SocketDetectorToDaqConnection.");
    ers::fatal(err);
    throw err;
  } 

  auto* nw_sender = socket_d2d_conn->get_net_senders()[0]; // there's only 1 sender

  for (auto det_stream : nw_sender->get_streams()) {
    if (det_stream->is_excluded(*(mcfg->get_session()))) {
      continue;
    }

    m_fake_stream_ids.push_back(det_stream->get_geo_id()->get_stream_id());

    m_producer_threads.emplace_back(std::make_unique<utilities::ReusableThread>());
  }

  auto* con = mdal->get_outputs()[0]; // there's only 1 output  
  auto* queue = con->cast<confmodel::Queue>();
  if (queue == nullptr) {
    auto err = datahandlinglibs::InitializationError(ERS_HERE, "Output is not of type Queue.");
    ers::fatal(err);
    throw err;
  }
  
  auto connection_name = queue->UID();
  m_sender = get_iom_sender<fddetdataformats::CRTBernFrame>(connection_name);
}

void
CRTBernFrameBuilderModule::do_conf(const CommandData_t& /*obj*/)
{
  // Configure HW interface?
  if (!m_run_marker.load()) {
    set_running(true);
  } else {
    TLOG_DEBUG(5) << "Already running!";
  }
}

void
CRTBernFrameBuilderModule::do_scrap(const CommandData_t& /*obj*/)
{
  if (m_run_marker.load()) {
    TLOG() << "Raising stop through variables!";
    set_running(false);
    for (const auto& producer : m_producer_threads) {
      while (!producer->get_readiness()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  } else {
    TLOG_DEBUG(5) << "Already stopped!";
  }
}

void
CRTBernFrameBuilderModule::do_start(const CommandData_t& /*startobj*/)
{
  enable_flow();

  m_packet_count = 0;
  m_t0 = std::chrono::steady_clock::now();

  uint32_t i = 0; // NOLINT(build/unsigned)
  for (auto& producer : m_producer_threads) {
    producer->set_work(&CRTBernFrameBuilderModule::run_produce, this, m_fake_stream_ids[i++]);
  }
}

void
CRTBernFrameBuilderModule::do_stop(const CommandData_t& /*stopobj*/)
{
  disable_flow();
}

void
CRTBernFrameBuilderModule::generate_opmon_data()
{
  opmon::CRTBernFrameBuilderInfo i;

  auto now = std::chrono::steady_clock::now();
  int new_packets = m_packet_count.exchange(0);
  double seconds = std::chrono::duration_cast<std::chrono::microseconds>(now - m_t0).count() / 1000000.;
  m_t0 = now;

  i.set_packet_rate_khz(new_packets / seconds / 1000.);

  publish(std::move(i));
}

void
CRTBernFrameBuilderModule::run_produce(uint32_t fake_stream_id) // NOLINT(build/unsigned)
{
  TLOG() << "Producer thread started..."; // TODO (DTE): Debug log instead

  fddetdataformats::CRTBernFrame frame;
  uint64_t seq_id = 0; // NOLINT(build/unsigned)
  uint64_t timestamp = 0; // NOLINT(build/unsigned)

  datahandlinglibs::RateLimiter rate_limiter(m_configured_packet_rate_khz);

  while (m_run_marker.load()) {
    // Create a fake packet for stream
    fake_data(frame, seq_id, timestamp, fake_stream_id); // TODO: To be filled by the CRT experts

    if (m_enable_flow.load()) [[likely]] {   
      m_sender->try_send(std::move(frame), iomanager::Sender::s_no_block);        
      ++m_packet_count;
    }

    rate_limiter.limit();    
  }

  TLOG() << "Producer thread joins... "; // TODO (DTE): Debug log instead
}

void 
CRTBernFrameBuilderModule::set_running(bool should_run)
{
  bool was_running = m_run_marker.exchange(should_run);
  TLOG_DEBUG(5) << "Active state was toggled from " << was_running << " to " << should_run;
}

} // namespace dunedaq::crtmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTBernFrameBuilderModule)
