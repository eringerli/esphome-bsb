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
    using NumberMap = std::unordered_multimap< uint32_t, BsbNumber* >;

    class BsbComponent
        : public Component
        , public uart::UARTDevice {
    public:
      BsbComponent();

      void     setup() override;
      void     dump_config() override;
      void     loop() override;
      float    get_setup_priority() const override { return setup_priority::DATA; };
      void     set_update_interval( uint32_t val ) { update_interval_ms = val; }
      uint32_t get_update_interval() { return update_interval_ms; }
      void     set_source_address( uint32_t val ) { source_address = val; }
      void     set_destination_address( uint32_t val ) { destination_address = val; }
      void     set_query_interval( uint32_t val ) { query_interval = val; }

      void register_sensor( BsbSensorBase* sensor ) { this->sensors.insert( { sensor->get_field_id(), sensor } ); }
      void register_number( BsbNumber* number ) { this->numbers.insert( { number->get_field_id(), number } ); }

    protected:
      void callbackPacket( const BsbPacket* packet );

      BsbPacketReceive bsbPacketReceive = BsbPacketReceive( [&]( const BsbPacket* packet ) { callbackPacket( packet ); } );

      SensorMap sensors;
      NumberMap numbers;

      uint32_t update_interval_ms = 0;
      uint32_t query_interval;

      uint8_t source_address;
      uint8_t destination_address;

    private:
      void sendNumber( const BsbNumber* number );
      void writePacket( const BsbPacket& packet );

      uint32_t lastQuery = 0;
    };

  } // namespace bsb
} // namespace esphome
