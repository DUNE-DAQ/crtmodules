/**
 * @file CRTReader.hpp
 *
 * CRTReader is a simple DAQModule implementation that
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CRTMODULES_PLUGINS_CRTREADER_HPP_
#define CRTMODULES_PLUGINS_CRTREADER_HPP_

#include "crtmodules/crtreader/Nljs.hpp"

#include "crtmodules/CRTInterface.hh"
#include "crtmodules/CRTdecode.hh"

#include "fdreadoutlibs/CRTTypeAdapter.hpp"

#include "appfwk/DAQModule.hpp"
#include "iomanager/Receiver.hpp"
#include "iomanager/Sender.hpp"
#include "utilities/WorkerThread.hpp"

#include <ers/Issue.hpp>

#include <memory>
#include <string>
#include <vector>

namespace dunedaq {
namespace crtmodules {

/**
 * @brief CRTReader reads lists of integers from one queue,
 * reverses the order of the list, and writes out the reversed list.
 */
class CRTReader : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief CRTReader Constructor
   * @param name Instance name for this CRTReader instance
   */
  explicit CRTReader(const std::string& name);

  CRTReader(const CRTReader&) = delete;            ///< CRTReader is not copy-constructible
  CRTReader& operator=(const CRTReader&) = delete; ///< CRTReader is not copy-assignable
  CRTReader(CRTReader&&) = delete;                 ///< CRTReader is not move-constructible
  CRTReader& operator=(CRTReader&&) = delete;      ///< CRTReader is not move-assignable

  void init(const nlohmann::json& iniobj) override;

private:
  // Commands
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);
  void do_conf(const nlohmann::json& obj);
  void do_scrap(const nlohmann::json& obj);

  // Threading
  dunedaq::utilities::WorkerThread thread_;
  void do_work(std::atomic<bool>&);

  // Configuration
  //using sink_t = dunedaq::iomanager::SenderConcept<IntList>;
  using sink_t = dunedaq::iomanager::SenderConcept<dunedaq::fdreadoutlibs::types::CRTTypeAdapter>;
  std::shared_ptr<sink_t> outputQueue_;
  std::chrono::milliseconds queueTimeout_;

  std::unique_ptr<CRTInterface> hardware_interface_;
  char* readout_buffer_;
  crtreader::Conf cfg_;
};
} // namespace crtmodules
} // namespace dunedaq

#endif // CRTMODULES_PLUGINS_CRTREADER_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
