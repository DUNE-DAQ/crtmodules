/**
 * @file CRTBernFrameBuilderModule.hpp
 *
 * Reads data from the HW then puts it in a queue
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CRTMODULES_PLUGINS_CRTBERNFRAMEBUILDERMODULE_HPP_
#define CRTMODULES_PLUGINS_CRTBERNFRAMEBUILDERMODULE_HPP_

#include "appfwk/DAQModule.hpp"
#include "utilities/ReusableThread.hpp"
#include "fddetdataformats/CRTBernFrame.hpp"

#include <memory>
#include <map>
#include <string>
#include <vector>

namespace dunedaq::crtmodules {

class CRTBernFrameBuilderModule : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief CRTBernFrameBuilderModule constructor
   * @param name DAQ module instance name
   */
  explicit CRTBernFrameBuilderModule(const std::string& name);

  CRTBernFrameBuilderModule(const CRTBernFrameBuilderModule&) = delete;            ///< CRTBernFrameBuilderModule is not copy-constructible
  CRTBernFrameBuilderModule& operator=(const CRTBernFrameBuilderModule&) = delete; ///< CRTBernFrameBuilderModule is not copy-assignable
  CRTBernFrameBuilderModule(CRTBernFrameBuilderModule&&) = delete;                 ///< CRTBernFrameBuilderModule is not move-constructible
  CRTBernFrameBuilderModule& operator=(CRTBernFrameBuilderModule&&) = delete;      ///< CRTBernFrameBuilderModule is not move-assignable

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
   * @brief Data produce thread function
   * @param fake_stream_id Fake packet stream ID
   */     
  void run_produce(uint32_t fake_stream_id); // NOLINT(build/unsigned)

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
   * @brief Data producer threads
   */       
  std::vector<std::unique_ptr<utilities::ReusableThread>> m_producer_threads;  

  /**
   * @brief Data sender
   */
  std::shared_ptr<iomanager::SenderConcept<fddetdataformats::CRTBernFrame>> m_sender;

  /**
   * @brief Fake packet stream IDs
   */  
  std::vector<uint32_t> m_fake_stream_ids; // NOLINT(build/unsigned)

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

#endif // CRTMODULES_PLUGINS_CRTBERNFRAMEBUILDERMODULE_HPP_
