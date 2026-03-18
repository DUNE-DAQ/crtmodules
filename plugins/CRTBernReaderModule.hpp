/**
 * @file CRTBernReaderModule.hpp
 *
 * Reads data from the HW then puts it in a queue
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CRTMODULES_PLUGINS_CRTBERNREADERMODULE_HPP_
#define CRTMODULES_PLUGINS_CRTBERNREADERMODULE_HPP_

#include "appfwk/DAQModule.hpp"
#include "utilities/ReusableThread.hpp"

#include <memory>
#include <map>
#include <string>

namespace dunedaq::crtmodules {

class SourceConcept;

class CRTBernReaderModule : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief CRTBernReaderModule constructor
   * @param name DAQ module instance name
   */
  explicit CRTBernReaderModule(const std::string& name);

  CRTBernReaderModule(const CRTBernReaderModule&) = delete;            ///< CRTBernReaderModule is not copy-constructible
  CRTBernReaderModule& operator=(const CRTBernReaderModule&) = delete; ///< CRTBernReaderModule is not copy-assignable
  CRTBernReaderModule(CRTBernReaderModule&&) = delete;                 ///< CRTBernReaderModule is not move-constructible
  CRTBernReaderModule& operator=(CRTBernReaderModule&&) = delete;      ///< CRTBernReaderModule is not move-assignable

  /**
   * @brief Handles initialization on boot
   * @param mcfg DAQ configuration data
   */    
  void init(const std::shared_ptr<appfwk::ConfigurationManager> mcfg) override;

private:
  // Commands
  void do_conf(const CommandData_t& obj);
  void do_start(const CommandData_t& obj);
  void do_stop(const CommandData_t& obj);
  void do_scrap(const CommandData_t& obj);

  void generate_opmon_data() override;

  /**
   * @brief Raw data produce thread function
   */     
  void run_produce();

  /**
   * @brief Sets run marker
   * @param should_run Whether producer thread should continue
   */      
  void set_running(bool /*should_run*/);  
  
  /**
   * @brief Enables data taking
   */   
  void enable_flow() { m_enable_flow.store(true); }

  /**
   * @brief Disables data taking
   */   
  void disable_flow() { m_enable_flow.store(false); }  

  /**
   * @brief Whether producer thread should continue
   */      
  std::atomic<bool> m_run_marker{ false };

  /**
   * @brief Whether data taking should continue
   */        
  std::atomic<bool> m_enable_flow{ false }; // this is for queue ops

  // PRODUCER
  /**
   * @brief Raw data producer thread
   */       
  utilities::ReusableThread m_producer_thread;  

  using sid_to_source_map_t = std::map<uint32_t, std::shared_ptr<SourceConcept>>; // NOLINT(build/unsigned)
  /**
   * @brief Data sources
   */
  sid_to_source_map_t m_sources;

  using sid_to_fake_stream_id_map_t = std::map<uint32_t, uint32_t>; // NOLINT(build/unsigned)
  /**
   * @brief Fake packet stream IDs
   */  
  sid_to_fake_stream_id_map_t m_fake_stream_ids;

  /**
   * @brief Configured packet transmission rate in kHz
   */
  double m_configured_packet_rate_khz{ 10 };
    
  /**
   * @brief Counts packets since last opmon data generation
   */
  std::atomic<int> m_packet_count{ 0 };   

  // RUN START T0
  /**
   * @brief Timestamp used to measure time between opmon reports
   */   
  std::chrono::time_point<std::chrono::steady_clock> m_t0;      
};
} // namespace dunedaq::crtmodules

#endif // CRTMODULES_PLUGINS_CRTBERNREADERMODULE_HPP_
