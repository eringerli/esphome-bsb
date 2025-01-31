#pragma once

#include "bsbPacketSend.h"

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
        dirty_      = true;
      }

      void set_value( const float value ) { publish_state( value * factor_ / divisor_ ); }

      void           set_field_id( const uint32_t field_id ) { this->field_id_ = field_id; }
      const uint32_t get_field_id() const { return field_id_; }

      void set_enable_byte( const uint8_t enable_byte ) { this->enable_byte_ = enable_byte; }

      void        set_divisor( const float divisor ) { this->divisor_ = divisor; }
      const float get_divisor() const { return this->divisor_; }

      void        set_factor( const float factor ) { this->factor_ = factor; }
      const float get_factor() const { return this->factor_; }

      void       set_broadcast( const bool broadcast ) { this->broadcast_ = broadcast; }
      const bool get_broadcast() const { return this->broadcast_; }

      void           set_update_interval( const uint32_t val ) { update_interval_ms_ = val; }
      const uint32_t get_update_interval() const { return update_interval_ms_; }

      void set_retry_interval( const uint32_t val ) { retry_interval_ms_ = val; }
      void set_retry_count( uint8_t val ) { retry_count_ = val; }

      void                     set_value_type( const int type ) { this->value_type_ = ( BsbNumberValueType )type; }
      const BsbNumberValueType get_value_type() const { return this->value_type_; }

      bool is_ready_to_update( const uint32_t timestamp ) {
        if( sent_get_ >= 5 ) {
          ESP_LOGE( TAG, "BsbNumber %08X: retries exhausted, next try in %fs ", get_field_id(), retry_interval_ms_ / 1000. );

          if( timestamp >= ( next_update_timestamp_ + retry_interval_ms_ ) ) {
            sent_get_ = 0;
          }
        }
        return ( sent_get_ < 5 ) && ( !broadcast_ && timestamp >= next_update_timestamp_ );
      }

      bool is_ready_to_set( const uint32_t timestamp ) {
        if( sent_set_ >= 5 ) {
          ESP_LOGE( TAG, "BsbNumber %08X: retries exhausted, next try in %fs ", get_field_id(), retry_interval_ms_ / 1000. );

          if( timestamp >= ( next_update_timestamp_ + retry_interval_ms_ ) ) {
            sent_set_ = 0;
          }
        }
        return ( sent_set_ < 5 ) && dirty_ /*|| ( broadcast_ && timestamp >= next_update_timestamp_ )*/;
      }

      void schedule_next_regular_update( const uint32_t timestamp ) {
        sent_get_              = 0;
        next_update_timestamp_ = timestamp + update_interval_ms_;
      }
      void schedule_next_update( const uint32_t timestamp, const uint32_t interval ) {
        sent_get_              = 0;
        next_update_timestamp_ = timestamp + interval;
      }

      void reset_dirty() {
        sent_set_ = 0;
        dirty_    = false;
      }

      const BsbPacket createPackageSet( uint8_t source_address, uint8_t destination_address ) {
        ++sent_set_;

        switch( get_value_type() ) {
          case BsbNumberValueType::UInt8: {
            return BsbPacketSetUInt8( source_address, destination_address, get_field_id(), ( uint8_t )( getValueToSend() ), enable_byte_ );
          } break;
          case BsbNumberValueType::Int8: {
            return BsbPacketSetInt8( source_address, destination_address, get_field_id(), ( int8_t )( getValueToSend() ), enable_byte_ );
          } break;

          case BsbNumberValueType::Int16: {
            return BsbPacketSetInt16( source_address, destination_address, get_field_id(), ( int16_t )( getValueToSend() ), enable_byte_ );
          } break;

          case BsbNumberValueType::Int32: {
            return BsbPacketSetInt32( source_address, destination_address, get_field_id(), ( int32_t )( getValueToSend() ), enable_byte_ );
          } break;

          case BsbNumberValueType::Temperature: {
            if( broadcast_ ) {
              return BsbPacketInfTemperature( source_address, get_field_id(), getValueToSend(), enable_byte_ );
            } else {
              return BsbPacketSetTemperature( source_address, get_field_id(), getValueToSend(), enable_byte_ );
            }
          }
          case BsbNumberValueType::RoomTemperature: {
            if( broadcast_ ) {
              return BsbPacketInfRoomTemperature( source_address, get_field_id(), getValueToSend(), enable_byte_ );
            } else {
              return BsbPacket();
            }
          } break;

          default:
            return BsbPacket();
        }
      }

      const BsbPacket createPackageGet( uint8_t source_address, uint8_t destination_address ) {
        ++sent_get_;

        return BsbPacketGet( source_address, destination_address, get_field_id() );
      }

    protected:
      const float getValueToSend() const { return state * divisor_ / factor_; }

      // uint16_t           parameterNumber_ = 0;
      uint32_t           field_id_    = 0;
      uint8_t            enable_byte_ = 0x01;
      bool               broadcast_   = false;
      BsbNumberValueType value_type_  = BsbNumberValueType::Temperature;
      float              divisor_     = 1.;
      float              factor_      = 1.;

      uint32_t update_interval_ms_;
      uint32_t retry_interval_ms_;
      uint8_t  retry_count_;

      uint32_t next_update_timestamp_ = 0;

      uint16_t sent_set_ = 0;
      uint16_t sent_get_ = 0;
      bool     dirty_    = false;
    };

  } // namespace bsb
} // namespace esphome
