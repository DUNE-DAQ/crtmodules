/**
 * @file SourceModel.hpp FELIX CR's ELink concept wrapper
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef CRTMODULES_SRC_SOURCEMODEL_HPP_
#define CRTMODULES_SRC_SOURCEMODEL_HPP_

#include "SourceConcept.hpp"


#include "iomanager/IOManager.hpp"
#include "iomanager/Sender.hpp"
#include "logging/Logging.hpp"

#include "crtmodules/opmon/SourceModel.pb.h"

// #include "datahandlinglibs/utils/ReusableThread.hpp"
#include "datahandlinglibs/DataMoveCallbackRegistry.hpp"
#include "datahandlinglibs/utils/BufferCopy.hpp" 

// #include <folly/ProducerConsumerQueue.h>
// #include <nlohmann/json.hpp>
// NOLINT(build/unsigned)
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace dunedaq::crtmodules {

template<class TargetPayloadType>
class SourceModel : public SourceConcept
{
public:
  using sink_t = iomanager::SenderConcept<TargetPayloadType>;
  using inherited = SourceConcept;
  using data_t = nlohmann::json;

  /**
   * @brief Buffer size based on TargetPayloadType
   */
  static constexpr auto buffer_size = sizeof(TargetPayloadType);

  /**
   * @brief SourceModel Constructor
   * @param name Instance name for this SourceModel instance
   */
  SourceModel()
    : SourceConcept()
  {}
  ~SourceModel() {}

  void acquire_callback() override
  {
      if (m_callback_is_acquired) {
        TLOG_DEBUG(5) << "SourceModel callback is already acquired!";
      } else {
        // Getting DataMoveCBRegistry
        auto dmcbr = datahandlinglibs::DataMoveCallbackRegistry::get();
        m_sink_callback = dmcbr->get_callback<TargetPayloadType>(inherited::m_sink_conf);
        m_callback_is_acquired = true;
      }
  }

  bool handle_payload(char* message, std::size_t size) override
  {
    bool push_out = true;
    if (push_out) {

      TargetPayloadType& target_payload = *reinterpret_cast<TargetPayloadType*>(message);
        (*m_sink_callback)(std::move(target_payload));

    } else {
      TargetPayloadType target_payload;
      uint32_t bytes_copied = 0; // NOLINT(build/unsigned)
      datahandlinglibs::buffer_copy(message, size, static_cast<void*>(&target_payload), bytes_copied, sizeof(target_payload));
    }

    return true;
  }

  void generate_opmon_data() override {

    opmon::SourceInfo info;
    info.set_dropped_frames( m_dropped_packets.load() ); 

    publish( std::move(info) );
  }
  
private:
  // Callback internals
  bool m_callback_is_acquired{ false };
  using sink_cb_t = std::shared_ptr<std::function<void(TargetPayloadType&&)>>;
  sink_cb_t m_sink_callback;

  std::atomic<uint64_t> m_dropped_packets{0}; // NOLINT(build/unsigned)

};

} // namespace dunedaq::crtmodules

#endif // CRTMODULES_SRC_SOURCEMODEL_HPP_
