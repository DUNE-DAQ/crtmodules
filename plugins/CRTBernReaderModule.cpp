/**
 * @file CRTBernReaderModule.cpp

 * Reads data from the HW then puts it in a queue
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CRTBernReaderModule.hpp"

#include "crtmodules/opmon/CRTBernReaderModule.pb.h"

#include "datahandlinglibs/utils/RateLimiter.hpp"
#include "datahandlinglibs/DataHandlingIssues.hpp"

#include "appmodel/DataReaderModule.hpp"
#include "appmodel/SocketDetectorToDaqConnection.hpp"
#include "appmodel/NWDetDataSender.hpp"

#include "confmodel/QueueWithSourceId.hpp"
#include "confmodel/DetectorStream.hpp"
#include "confmodel/GeoId.hpp"

#include "detdataformats/DetID.hpp"

DUNE_DAQ_TYPESTRING(dunedaq::fddetdataformats::CRTBernFrame, "CRTBernFrame")

namespace dunedaq {
namespace crtmodules{

/**
 * @brief Maximum packet sequence ID before reset
 */
constexpr uint64_t max_seq_id = 4095;

/**
 * @brief Fake packet detector ID
 */
constexpr uint8_t fake_det_id = (uint8_t)detdataformats::DetID::Subdetector::kVD_BernCRT;

/**
 * @brief Fake packet block length
 */
constexpr uint64_t fake_block_length = 0x382;

/**
 * @brief Calculate the next fake sequence ID for a packet
 * @param seq_id Fake packet sequence ID
 */
void
fake_sequence_id(uint64_t& seq_id)
{
  seq_id = (seq_id == max_seq_id ? 0 : seq_id+1);
}

/**
 * @brief Calculate the next fake timestamp for a packet
 * @param timestamp Fake packet timestamp
 */
void
fake_timestamp(uint64_t& timestamp)
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
fake_data(fddetdataformats::CRTBernFrame& frame, uint64_t& seq_id, uint64_t& timestamp, uint32_t stream_id)
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

CRTBernReaderModule::CRTBernReaderModule(const std::string& name)
  : DAQModule(name)
{
  register_command("conf", &CRTBernReaderModule::do_conf);
  register_command("start", &CRTBernReaderModule::do_start);
  register_command("stop_trigger_sources", &CRTBernReaderModule::do_stop);
  register_command("scrap", &CRTBernReaderModule::do_scrap);
}

void
CRTBernReaderModule::init(const std::shared_ptr<appfwk::ConfigurationManager> mcfg)
{
  auto* mdal = mcfg->get_dal<appmodel::DataReaderModule>(get_name());
  
  auto* d2d_conn = mdal->get_connections()[0]; // there's only 1 connection
  auto* socket_d2d_conn = d2d_conn->cast<appmodel::SocketDetectorToDaqConnection>();
  if (socket_d2d_conn == nullptr) {
    auto err = datahandlinglibs::InitializationError(ERS_HERE, "Connection is not of type SocketDetectorToDaqConnection.");
    ers::fatal(err);
    throw err;
  } 

  for (auto nw_sender : socket_d2d_conn->get_net_senders()) {
    if (nw_sender->is_disabled(*(mcfg->get_session()))) {
      continue;
    }

    for (auto det_stream : nw_sender->get_streams()) {
      if (det_stream->is_disabled(*(mcfg->get_session()))) {
        continue;
      }

      m_fake_stream_ids[det_stream->get_source_id()] = det_stream->get_geo_id()->get_stream_id();
    }    
  }

  if (mdal->get_outputs().empty()) {
    auto err = datahandlinglibs::InitializationError(ERS_HERE,
                                                              "No outputs defined for CRT Bern reader in configuration.");
    ers::fatal(err);
    throw err;
  }

  for (auto* con : mdal->get_outputs()) {
    auto* queue = con->cast<confmodel::QueueWithSourceId>();
    if (queue == nullptr) {
      auto err = datahandlinglibs::InitializationError(ERS_HERE, "Outputs are not of type QueueWithGeoId.");
      ers::fatal(err);
      throw err;
    }
    
    // CRTBernReaderModule does not support callbacks
    auto connection_name = queue->UID();
    m_raw_data_senders[queue->get_source_id()] = get_iom_sender<fddetdataformats::CRTBernFrame>(connection_name);
  }
}

void
CRTBernReaderModule::do_conf(const CommandData_t& /*obj*/)
{
  // Configure HW interface?
  if (!m_run_marker.load()) {
    set_running(true);
  } else {
    TLOG_DEBUG(5) << "Already running!";
  }
}

void
CRTBernReaderModule::do_scrap(const CommandData_t& /*obj*/)
{
  if (m_run_marker.load()) {
    TLOG() << "Raising stop through variables!";
    set_running(false);
    while (!m_producer_thread.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  } else {
    TLOG_DEBUG(5) << "Already stopped!";
  }
}

void
CRTBernReaderModule::do_start(const CommandData_t& /*startobj*/)
{
  enable_flow();

  m_packet_count = 0;
  m_t0 = std::chrono::steady_clock::now();

  m_producer_thread.set_work(&CRTBernReaderModule::run_produce, this);
}

void
CRTBernReaderModule::do_stop(const CommandData_t& /*stopobj*/)
{
  disable_flow();
}

void
CRTBernReaderModule::generate_opmon_data()
{
  opmon::CRTBernReaderInfo i;

  auto now = std::chrono::steady_clock::now();
  int new_packets = m_packet_count.exchange(0);
  double seconds = std::chrono::duration_cast<std::chrono::microseconds>(now - m_t0).count() / 1000000.;
  m_t0 = now;

  i.set_packet_rate_khz(new_packets / seconds / 1000.);

  publish(std::move(i));
}

void
CRTBernReaderModule::run_produce()
{
  TLOG() << "Producer thread started..."; // TODO (DTE): Debug log instead

  fddetdataformats::CRTBernFrame frame;
  uint64_t seq_id = 0;
  uint64_t timestamp = 0;

  datahandlinglibs::RateLimiter rate_limiter(m_configured_packet_rate_khz);

  while (m_run_marker.load()) {
    // Create a fake packet for each stream
    for (const auto& [sid, sender] : m_raw_data_senders) {
      fake_data(frame, seq_id, timestamp, m_fake_stream_ids[sid]); // TODO: To be filled by the CRT experts
  
      if (m_enable_flow.load()) [[likely]] {   
        sender->try_send(std::move(frame), iomanager::Sender::s_no_block);
        ++m_packet_count;
      }

      rate_limiter.limit();    
    }
  }

  TLOG() << "Producer thread joins... "; // TODO (DTE): Debug log instead
}

void 
CRTBernReaderModule::set_running(bool should_run)
{
  bool was_running = m_run_marker.exchange(should_run);
  TLOG_DEBUG(5) << "Active state was toggled from " << was_running << " to " << should_run;
}

}
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTBernReaderModule)
