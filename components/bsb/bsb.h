#pragma once

#include "bsbPacket.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
  #include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#include "bsbNumber.h"
#include "bsbSensor.h"

#include <cstdint>
#include <unordered_map>

#include "bsbPacketReceive.h"

namespace esphome {
  namespace bsb {

    extern const char* const TAG;

    using SensorMap = std::unordered_multimap< uint32_t, BsbSensorBase* >;
    using NumberMap = std::unordered_multimap< uint32_t, BsbNumberBase* >;

    class BsbComponent
        : public Component
        , public uart::UARTDevice {
    public:
      BsbComponent();

      void  setup() override;
      void  dump_config() override;
      void  loop() override;
      float get_setup_priority() const override { return setup_priority::DATA; };

      void set_source_address( uint32_t val ) { source_address_ = val; }
      void set_destination_address( uint32_t val ) { destination_address_ = val; }

      void set_query_interval( uint32_t val ) { query_interval_ = val; }

      void           set_retry_interval( uint32_t val ) { retry_interval_ = val; }
      const uint32_t get_retry_interval() const { return retry_interval_; }
      void           set_retry_count( uint8_t val ) { retry_count_ = val; }
      const uint8_t  get_retry_count() const { return retry_count_; }

      void register_sensor( BsbSensorBase* sensor ) { this->sensors_.insert( { sensor->get_field_id(), sensor } ); }
      void register_number( BsbNumberBase* number ) { this->numbers_.insert( { number->get_field_id(), number } ); }

    protected:
      void callback_packet( const BsbPacket* packet );

      void write_packet( const BsbPacket& packet );

      BsbPacketReceive bsbPacketReceive = BsbPacketReceive( [&]( const BsbPacket* packet ) { callback_packet( packet ); } );

      SensorMap sensors_;
      NumberMap numbers_;

      uint32_t query_interval_;
      uint32_t retry_interval_;
      uint8_t  retry_count_;

      uint8_t source_address_;
      uint8_t destination_address_;

    private:
      uint32_t last_query_ = 0;

      static constexpr uint32_t IntervalGetAfterSet = 1000;
    };

  } // namespace bsb
} // namespace esphome
