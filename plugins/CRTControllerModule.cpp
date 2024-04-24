/**
 * @file CRTControllerModule.cpp
 *
 * Implementations of CRTControllerModule's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CRTControllerModule.hpp"

#include "crtmodules/crtcontrollermodule/Nljs.hpp"
#include "crtmodules/crtcontrollermoduleinfo/InfoNljs.hpp"

#include "startallboards.cc"
#include "stopallboards.cc"

#include <string>
#include <iostream>

namespace dunedaq::crtmodules {

CRTControllerModule::CRTControllerModule(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &CRTControllerModule::do_conf);
  register_command("start", &CRTControllerModule::do_start);
  register_command("stop", &CRTControllerModule::do_stop);
  register_command("scrap", &CRTControllerModule::do_scrap);
}

void
CRTControllerModule::init(const data_t& /* structured args */)
{}

void
CRTControllerModule::get_info(opmonlib::InfoCollector& ci, int /* level */)
{
  crtcontrollermoduleinfo::Info info;
  info.total_amount = m_total_amount;
  info.amount_since_last_get_info_call = m_amount_since_last_get_info_call.exchange(0);

  ci.add(info);
}

void
CRTControllerModule::do_conf(const data_t& conf_as_json)
{
  auto conf_as_cpp = conf_as_json.get<crtcontrollermodule::BoardConf>();
  m_some_configured_value = conf_as_cpp.hv_setting;

  std::cout << "HEY!!! WE CONFIGURED!!! " << m_some_configured_value << std::endl;

  

}

void
CRTControllerModule::do_start(const data_t&)
{
  std::cout << "In do_start() method\n";
  char indir[] = "/data0";
  char configfile[] = "/nfs/home/madmurph/VT_daq/ICARUS_DAQ/DAQ_CPP_v1/fcl_oneboard.fcl";
  startallboards(configfile,indir);
  std::cout << "Exiting do_start() method\n";
}

void
CRTControllerModule::do_stop(const data_t&)
{
  std::cout << "In do_stop() method\n";
  char indir[] = "/data0";
  char configfile[] = "/nfs/home/madmurph/VT_daq/ICARUS_DAQ/DAQ_CPP_v1/fcl_oneboard.fcl";

  stopallboards(configfile,indir);

  std::cout << "Exiting do_stop() method\n"; 
}

void
CRTControllerModule::do_scrap(const data_t&)
{
}

} // namespace dunedaq::crtmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTControllerModule)
