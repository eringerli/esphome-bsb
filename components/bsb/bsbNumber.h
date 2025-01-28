#pragma once

#include "esphome/components/number/number.h"
#include <cstdint>

namespace esphome {
  namespace bsb {
    extern const char* const TAG;

    enum class BsbNumberValueType { UInt8, Int8, Int16, Int32, Temperature, RoomTemperature };

    class BsbNumber : public number::Number {
    public:
      virtual void control( float value ) override {
        this->state = value;
        dirty       = true;
      }

      void     set_field_id( const uint32_t fieldId ) { this->fieldId = fieldId; }
      uint32_t get_field_id() const { return fieldId; }

      void set_divisor( const float divisor ) { this->divisor = divisor; }
      void set_factor( const float factor ) { this->factor = factor; }
      void set_enable_byte( const uint8_t enable_byte ) { this->enable_byte = enable_byte; }
      void set_broadcast( const bool broadcast ) { this->broadcast = broadcast; }

      void     set_update_interval( const uint32_t val ) { updateInterval_ms = val; }
      uint32_t get_update_interval() { return updateInterval_ms; }

      void                     set_value_type( const int type ) { this->valueType = ( BsbNumberValueType )type; }
      const BsbNumberValueType get_value_type() const { return this->valueType; }

      bool is_ready_to_update( const uint32_t timestamp ) {
        if( sent >= 5 ) {
          ESP_LOGE( TAG, "BsbNumber::is_ready_to_update(), sent >= 5: %08X", get_field_id() );

          if( timestamp >= ( nextUpdateTimestamp + 30000 ) ) {
            sent = 0;
          }
        }
        return ( sent < 5 ) && ( !broadcast && timestamp >= nextUpdateTimestamp );
      }
      bool is_ready_to_set( const uint32_t timestamp ) {
        if( sent >= 5 ) {
          ESP_LOGE( TAG, "BsbNumber::is_ready_to_set(), sent >= 5: %08X", get_field_id() );

          if( timestamp >= ( nextUpdateTimestamp + 30000 ) ) {
            sent = 0;
          }
        }
        return ( sent < 5 ) && dirty /*|| ( broadcast && timestamp >= nextUpdateTimestamp )*/;
      }
      void update_timestamp( const uint32_t timestamp ) {
        sent                = 0;
        nextUpdateTimestamp = timestamp + updateInterval_ms;
      }
      void reset_dirty() {
        sent  = 0;
        dirty = false;
      }

      const float getValueToSend() const { return state * divisor / factor; }

      uint8_t  enable_byte = 0x01;
      bool     broadcast   = false;
      uint16_t sent        = 0;

    protected:
      uint32_t           fieldId;
      BsbNumberValueType valueType = BsbNumberValueType::Temperature;
      float              divisor   = 1.;
      float              factor    = 1.;
      uint32_t           updateInterval_ms;

      uint32_t nextUpdateTimestamp = 0;

      bool dirty = false;
    };

  } // namespace bsb
} // namespace esphome
