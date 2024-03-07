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

#include <string>
#include <iostream>

namespace dunedaq::crtmodules {

CRTControllerModule::CRTControllerModule(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &CRTControllerModule::do_conf);
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
  auto conf_as_cpp = conf_as_json.get<crtcontrollermodule::Conf>();
  m_some_configured_value = conf_as_cpp.some_configured_value;

  std::cout << "HEY!!! WE CONFIGURED!!! " << m_some_configured_value << std::endl;
}

} // namespace dunedaq::crtmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTControllerModule)
