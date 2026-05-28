/**
 * @file CRTGrenobleReaderModule.cpp
 *
 * Reads data from the HW then puts it in a queue
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CRTGrenobleReaderModule.hpp"

#include "CreateSource.hpp"

#include "crtmodules/opmon/CRTGrenobleReaderModule.pb.h"

#include "datahandlinglibs/utils/RateLimiter.hpp"

#include "appmodel/DataReaderModule.hpp"

#include "confmodel/QueueWithSourceId.hpp"

#include "fddetdataformats/CRTGrenobleFrame.hpp"

#include "detdataformats/DetID.hpp"

namespace dunedaq {
namespace crtmodules{

/**
 * @brief Maximum packet sequence ID before reset
 */
constexpr uint64_t max_seq_id = 4095;

/**
 * @brief Fake packet detector ID
 */
constexpr uint8_t fake_det_id = (uint8_t)detdataformats::DetID::Subdetector::kVD_GrenobleCRT;

/**
 * @brief Fake packet stream ID
 */
constexpr uint64_t fake_stream_id = 0;

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
fake_adc(fddetdataformats::CRTGrenobleFrame& frame)
{
  for (int channel = 0; channel < fddetdataformats::CRTGrenobleFrame::s_num_channels; ++channel) {
    frame.set_adc(channel, 0);
  }
}

/**
 * @brief Create a fake packet
 * @param frame Fake packet
 * @param seq_id Fake packet sequence ID
 * @param timestamp Fake packet timestamp
 */
void
fake_data(fddetdataformats::CRTGrenobleFrame& frame, uint64_t& seq_id, uint64_t& timestamp)
{
  frame.set_geoid(1, 1, fake_stream_id); // crate, slot, stream
  frame.get_daqheader().det_id = fake_det_id & 0x3f; //6 bits for det id
  fake_sequence_id(seq_id);
  frame.get_daqheader().seq_id = seq_id;
  frame.get_daqheader().block_length = fake_block_length;
  fake_timestamp(timestamp);
  frame.set_timestamp(timestamp);
  fake_adc(frame);
}

CRTGrenobleReaderModule::CRTGrenobleReaderModule(const std::string& name)
  : DAQModule(name)
{
  register_command("conf", &CRTGrenobleReaderModule::do_conf);
  register_command("start", &CRTGrenobleReaderModule::do_start);
  register_command("stop_trigger_sources", &CRTGrenobleReaderModule::do_stop);
  register_command("scrap", &CRTGrenobleReaderModule::do_scrap);
}

inline void
tokenize(std::string const& str, const char delim, std::vector<std::string>& out)
{
  std::size_t start;
  std::size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
    end = str.find(delim, start);
    out.push_back(str.substr(start, end - start));
  }
}

void
CRTGrenobleReaderModule::init(const std::shared_ptr<appfwk::ConfigurationManager> mfcg)
{
  auto* mdal = mfcg->get_dal<appmodel::DataReaderModule>(get_name());

  if (mdal->get_raw_data_callbacks().empty()) {
    auto err = dunedaq::datahandlinglibs::InitializationError(ERS_HERE,
                                                              "No outputs defined for CRT Grenoble reader in configuration.");
    ers::fatal(err);
    throw err;
  }

  for (auto* con : mdal->get_raw_data_callbacks()) {
    m_source_id = con->get_source_id();
    auto ptr = m_sources[con->get_source_id()] = createSourceModel(con);
    register_node(con->UID(), ptr);
  }
}

void
CRTGrenobleReaderModule::do_conf(const CommandData_t& /*obj*/)
{
  // Configure HW interface?
  if (!m_run_marker.load()) {
    set_running(true);
  } else {
    TLOG_DEBUG(5) << "Already running!";
  }
}

void
CRTGrenobleReaderModule::do_scrap(const CommandData_t& /*obj*/)
{
  if (m_run_marker.load()) {
    TLOG() << "Raising stop through variables!";
    set_running(false);
//  if (!m_callback_mode) {
    while (!m_producer_thread.get_readiness()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
//  }
  } else {
    TLOG_DEBUG(5) << "Already stopped!";
  }
}

void
CRTGrenobleReaderModule::do_start(const CommandData_t& /*startobj*/)
{
  // Setup callbacks on all sourcemodels
  for (auto& [sourceid, source] : m_sources) {
    source->acquire_callback();
  }

  m_packet_count = 0;

  m_t0 = std::chrono::steady_clock::now();

  enable_flow();

  //if (!m_callback_mode) {
    m_producer_thread.set_work(&CRTGrenobleReaderModule::run_produce, this);
  //}
}

void
CRTGrenobleReaderModule::do_stop(const CommandData_t& /*stopobj*/)
{
  disable_flow();
}

void
CRTGrenobleReaderModule::generate_opmon_data()
{
  opmon::CRTGrenobleReaderInfo i;

  auto now = std::chrono::steady_clock::now();
  int new_packets = m_packet_count.exchange(0);
  double seconds = std::chrono::duration_cast<std::chrono::microseconds>(now - m_t0).count() / 1000000.;
  m_t0 = now;

  i.set_packet_rate_khz(new_packets / seconds / 1000.);

  publish(std::move(i));
}

void
CRTGrenobleReaderModule::run_produce()
{
  TLOG() << "Producer thread started..."; // TODO (DTE): Debug log instead

  fddetdataformats::CRTGrenobleFrame frame;
  uint64_t seq_id = 0;
  uint64_t timestamp = 0;

  datahandlinglibs::RateLimiter rate_limiter(m_configured_packet_rate_khz);

  while (m_run_marker.load()) {
    fake_data(frame, seq_id, timestamp); // TODO: To be filled by the CRT experts

    if (m_enable_flow.load()) [[likely]] {
      handle_eth_payload(reinterpret_cast<char*>(&frame), sizeof(frame));
      ++m_packet_count;
    }

    rate_limiter.limit();
  }

  TLOG() << "Producer thread joins... "; // TODO (DTE): Debug log instead
}

void
CRTGrenobleReaderModule::handle_eth_payload(char* payload, std::size_t size)
{
  // Get DAQ Header and its StreamID
  //auto* daq_header = reinterpret_cast<dunedaq::detdataformats::DAQEthHeader*>(payload);
  //auto src_id = m_stream_id_to_source_id[src_rx_q][(unsigned)daq_header->stream_id];

  if ( auto src_it = m_sources.find(m_source_id); src_it != m_sources.end()) {
    src_it->second->handle_payload(payload, size);
  } else {
    // Really bad -> unexpeced StreamID in UDP Payload.
    // This check is needed in order to avoid dynamically add thousands
    // of Sources on the fly, in case the data corruption is extremely severe.
    //if (m_num_unexid_frames.count(0) == 0) {
    //  m_num_unexid_frames[0] = 0;
    //}
    //m_num_unexid_frames[0]++;
  }
}

void
CRTGrenobleReaderModule::set_running(bool should_run)
{
  bool was_running = m_run_marker.exchange(should_run);
  TLOG_DEBUG(5) << "Active state was toggled from " << was_running << " to " << should_run;
}

}
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTGrenobleReaderModule)
