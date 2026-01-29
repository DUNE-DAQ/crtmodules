/**
 * @file CreateSource.hpp Specific SourceConcept creator.
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef CRTMODULES_SRC_CREATESOURCE_HPP_
#define CRTMODULES_SRC_CREATESOURCE_HPP_

#include "SourceConcept.hpp"
#include "SourceModel.hpp"
#include "datahandlinglibs/DataHandlingIssues.hpp"

#include "fdreadoutlibs/CRTBernTypeAdapter.hpp"
#include "fdreadoutlibs/CRTGrenobleTypeAdapter.hpp"

#include <memory>
#include <string>

namespace dunedaq {

DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::CRTBernTypeAdapter, "CRTBernFrame")
DUNE_DAQ_TYPESTRING(dunedaq::fdreadoutlibs::types::CRTGrenobleTypeAdapter, "CRTGrenobleFrame")

namespace crtmodules {

std::shared_ptr<SourceConcept>
createSourceModel(const appmodel::RawDataCallbackConf* conf)
{
  auto datatype = conf->get_data_type();
  TLOG() << "Choosing specializations for SourceModel for output connection "
         << " [uid:" << conf->UID() << " , data_type:" << datatype << ']';

  if (datatype.find("CRTBernFrame") != std::string::npos) {
    auto source_model = std::make_shared<SourceModel<fdreadoutlibs::types::CRTBernTypeAdapter>>();
    source_model->set_sink_config(conf);
    return source_model;
  } else if (datatype.find("CRTGrenobleFrame") != std::string::npos) {
    auto source_model = std::make_shared<SourceModel<fdreadoutlibs::types::CRTGrenobleTypeAdapter>>();
    source_model->set_sink_config(conf);
    return source_model;
  }

  return nullptr;
}

} // namespace crtmodules
} // namespace dunedaq

#endif // CRTMODULES_SRC_CREATESOURCE_HPP_
