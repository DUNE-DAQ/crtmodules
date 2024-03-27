/**
 * @file CRTReader.cpp CRTReader class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CRTReader.hpp"
#include "CommonIssues.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "iomanager/IOManager.hpp"
#include "logging/Logging.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "CRTReader" // NOLINT
#define TLVL_ENTER_EXIT_METHODS 10
#define TLVL_CRTREADER 15

namespace dunedaq {
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::CRTTypeAdapter, "CRTFrame")
namespace crtmodules{

CRTReader::CRTReader(const std::string& name)
  : DAQModule(name)
  , thread_(std::bind(&CRTReader::do_work, this, std::placeholders::_1))
  , outputQueue_(nullptr)
  , queueTimeout_(100)
{
  register_command("conf", &CRTReader::do_conf);
  register_command("start", &CRTReader::do_start);
  register_command("stop", &CRTReader::do_stop);
  register_command("scrap", &CRTReader::do_scrap);
}

void
CRTReader::init(const nlohmann::json& iniobj)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  auto qi = appfwk::connection_index(iniobj, { "output_1003" });
  try {
    outputQueue_ = get_iom_sender<fdreadoutlibs::types::CRTTypeAdapter>(qi["output_1003"]);
  } catch (const ers::Issue& excpt) {
    throw InvalidQueueFatalError(ERS_HERE, get_name(), "crt_stream", excpt);
  }
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
CRTReader::do_conf(const nlohmann::json& obj)
{
    TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_configure() method";

    cfg_ = obj.get<crtreader::Conf>();
    std::string data_directory = cfg_.data_directory;
    unsigned int usb = cfg_.usb;
    auto hwi_ptr_ = std::make_unique<CRTInterface>(data_directory,usb);
    hardware_interface_.swap(hwi_ptr_);
    hardware_interface_->AllocateReadoutBuffer(&readout_buffer_);
    hardware_interface_->StartDatataking();
    hardware_interface_->SetBaselines();

    TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_configure() method";
}

void
CRTReader::do_scrap(const nlohmann::json& /*obj*/)
{
    hardware_interface_->FreeReadoutBuffer(readout_buffer_);
}

void
CRTReader::do_start(const nlohmann::json& /*startobj*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";
  thread_.start_working_thread();

  TLOG() << get_name() << " successfully started";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
CRTReader::do_stop(const nlohmann::json& /*stopobj*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";
  thread_.stop_working_thread();

  hardware_interface_->StopDatataking();
  hardware_interface_->FreeReadoutBuffer(readout_buffer_);

  TLOG() << get_name() << " successfully stopped";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

void
CRTReader::do_work(std::atomic<bool>& running_flag)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_work() method";
  int sentCount = 0;
  while (running_flag.load()) {
      size_t bytes_read = 0;
      hardware_interface_->FillBuffer(readout_buffer_, &bytes_read);
      if(bytes_read>0){
          fdreadoutlibs::types::CRTTypeAdapter to_send;
          for(int k=0;k<288;k++){
	        to_send.data[k] = *(char*)(readout_buffer_+k);
          }
	    bool successfullyWasSent = false;
        while (!successfullyWasSent && running_flag.load()) {
            TLOG_DEBUG(TLVL_CRTREADER) << get_name() << ": Pushing the reversed list onto the output queue";
            try {
                outputQueue_->send(std::move(to_send), queueTimeout_);
                successfullyWasSent = true;
                ++sentCount;
            } catch (const dunedaq::iomanager::TimeoutExpired& excpt) {
                std::ostringstream oss_warn;
                oss_warn << "push to output queue \"" << outputQueue_->get_name() << "\"";
                ers::warning(dunedaq::iomanager::TimeoutExpired(
                             ERS_HERE,
                             get_name(),
                             oss_warn.str(),
                             std::chrono::duration_cast<std::chrono::milliseconds>(queueTimeout_).count()));
            }
        } 
      
      }
      else{
        continue;
      }
  }
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_work() method";
}

} // namespace fdreadoutmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTReader)

// Local Variables:
// c-basic-offset: 2
// End:
