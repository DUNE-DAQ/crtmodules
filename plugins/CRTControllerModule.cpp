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
#include "CRT.cc"

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
// auto conf_as_cpp = obj.get<crtcontrollermodule::board_confs>();
 auto cfg_ = conf_as_json;
 int usb_serial, pmt_board, hv_setting, dac_threshold, pipedelay, trigger_mode, force_trigger;
 bool use_maroc2gain;
 string gate;
 int gain[64];
 for(nlohmann::json::iterator it = cfg_.begin(); it != cfg_.end(); it++){
   std::cout << "iterator key: " << it.key() << std::endl;
   if(it.key()=="BoardConfs"){
     std::vector<crtcontrollermodule::BoardConf> v_conf = (*it).get<std::vector<crtcontrollermodule::BoardConf>>();
     for(int i=0;i<v_conf.size();i++){ //Loop over boards
       usb_serial = v_conf[i].usb_serial;
       pmt_board = v_conf[i].pmt_board;
       hv_setting = v_conf[i].hv_setting;
       dac_threshold = v_conf[i].dac_threshold;
       use_maroc2gain = v_conf[i].use_maroc2gain;
       gate = v_conf[i].gate;
       pipedelay = v_conf[i].pipedelay;
       trigger_mode = v_conf[i].trigger_mode;
       force_trigger = v_conf[i].force_trigger;
       for(int j = 0; j<64; j++){
         gain[j] = v_conf[i].gain[j];
       }
       std::cout << "--Configuring PMT board #" << i << ", with settings:\n";
       std::cout << "----USB Serial: " << usb_serial << "\n";
       std::cout << "----PMT Board: " << pmt_board << "\n";
       std::cout << "----HV Setting: " << hv_setting << "\n";
       std::cout << "----DAC Threshold: " << dac_threshold << "\n";
       std::cout << "----Use Maroc2 Gain?: " << use_maroc2gain << "\n";
       std::cout << "----Gate: " << gate << "\n";
       std::cout << "----Pipe Delay: " << pipedelay << "\n";
       std::cout << "----Trigger Mode: " << trigger_mode << "\n";
       std::cout << "----Force Trigger: " << force_trigger << "\n";
       std::cout << "-----------------------------------------\n";
     
       loadconfig_json(usb_serial, pmt_board, hv_setting, dac_threshold, use_maroc2gain, gate, pipedelay, trigger_mode, force_trigger, gain);
     }
   }
   char indir[] = "/data0";
  
   int PMTINI, PMTFIN;
  
   std::cout << "Killing previous readout processes, if any.\n";
   string cmd = "killall crt_readout";
   system(cmd.c_str());

   std::cout << "Removing all existing message queues, if they exist.\n";
   cmd = "ipcrm -Q 0x0000270f -Q 0x0000271e -Q 0x00002713 -Q 0x00002726 -Q 0x0000271d";
   //With the usb numbers we're using, the queues will have keys 9999, 10003, 10013, 10014, 10022
   system(cmd.c_str());

   cmd = "crt_readout -d 1 &";
   system(cmd.c_str());

   sleep(1); //Need time to be sure readout processes are started

   PMTINI = 1;
   PMTFIN = dunedaq::crtmodules::getnumpmt();

   dunedaq::crtmodules::initializeboard("auto",1000,PMTINI,PMTFIN,indir);
   int res = dunedaq::crtmodules::eventbuilder("auto",PMTINI,PMTFIN,indir);

   dunedaq::crtmodules::starttakedata(PMTINI,PMTFIN);
 
 }

 std::cout << "Exiting do_conf() method\n";

}

void
CRTControllerModule::do_start(const data_t&)
{
  std::cout << "In do_start() method\n";
 
  std::cout << "Exiting do_start() method\n";
}

void
CRTControllerModule::do_stop(const data_t&)
{
  std::cout << "In do_stop() method\n";

  std::cout << "Exiting do_stop() method\n"; 
}

void
CRTControllerModule::do_scrap(const data_t&)
{
  std::cout << "In do_scrap() method\n";
  char indir[] = "/data0";

  stopallboards(indir);

  string cmd = "ipcrm -Q 0x0000270f -Q 0x0000271e -Q 0x00002713 -Q 0x00002726 -Q 0x0000271d";
  system(cmd.c_str());

  std::cout << "Exiting do_scrap() method\n";
}

} // namespace dunedaq::crtmodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::crtmodules::CRTControllerModule)
