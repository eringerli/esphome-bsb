#pragma once

#include "bsbPacketSend.h"

#include "esphome/components/sensor/sensor.h"

#ifdef USE_BINARY_SENSOR
  #include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#ifdef USE_TEXT_SENSOR
  #include "esphome/components/text_sensor/text_sensor.h"
#endif

namespace esphome {
  namespace bsb {

    enum SensorType { Sensor, TextSensor, BinarySensor };

    enum class BsbSensorValueType { UInt8, Int8, Int16, Int32, Temperature, RoomTemperature };

    class BsbSensorBase {
    public:
      virtual SensorType get_type() = 0;
      virtual void       publish()  = 0;

      void           set_field_id( const uint32_t field_id ) { this->field_id_ = field_id; }
      const uint32_t get_field_id() const { return field_id_; }

      void           set_update_interval( const uint32_t update_interval_ms ) { update_interval_ms_ = update_interval_ms; }
      const uint32_t get_update_interval() const { return update_interval_ms_; }

      void set_retry_interval( const uint32_t retry_interval_ms ) { retry_interval_ms_ = retry_interval_ms; }
      void set_retry_count( uint8_t retry_count ) { retry_count_ = retry_count; }

      void                     set_value_type( const int value_type ) { this->value_type_ = ( BsbSensorValueType )value_type; }
      const BsbSensorValueType get_value_type() const { return this->value_type_; }

      const bool is_ready( const uint32_t timestamp ) {
        if( sent_get_ >= 5 ) {
          ESP_LOGE( TAG, "BsbNumber Get %08X: retries exhausted, next try in %fs ", get_field_id(), retry_interval_ms_ / 1000. );
          if( timestamp >= ( next_update_timestamp_ + retry_interval_ms_ ) ) {
            ESP_LOGE( TAG, "BsbNumber Set %08X: retrying", get_field_id(), retry_interval_ms_ / 1000. );
            sent_get_ = 0;
            return true;
          }
        }

        return ( sent_get_ < 5 ) && ( timestamp >= next_update_timestamp_ );
      }

      void schedule_next_regular_update( const uint32_t timestamp ) {
        sent_get_              = 0;
        next_update_timestamp_ = timestamp + update_interval_ms_;
      }

      const BsbPacket createPackageGet( uint8_t source_address, uint8_t destination_address ) {
        ++sent_get_;

        return BsbPacketGet( source_address, destination_address, get_field_id() );
      }

    protected:
      uint32_t           field_id_;
      BsbSensorValueType value_type_ = BsbSensorValueType::Temperature;

      uint32_t update_interval_ms_;
      uint32_t retry_interval_ms_;
      uint8_t  retry_count_;

    private:
      uint32_t next_update_timestamp_ = 0;
      uint16_t sent_get_              = 0;
    };

    class BsbSensor
        : public BsbSensorBase
        , public sensor::Sensor {
    public:
      SensorType get_type() override { return SensorType::Sensor; }
      void       publish() override { publish_state( value_ ); }

      void set_enable_byte( const uint8_t enable_byte ) { this->enable_byte_ = enable_byte; }

      void set_value( float value ) { this->value_ = value * factor_ / divisor_; }

      void        set_divisor( const float divisor ) { this->divisor_ = divisor; }
      const float get_divisor() const { return this->divisor_; }

      void        set_factor( const float factor ) { this->factor_ = factor; }
      const float get_factor() const { return this->factor_; }

    protected:
      float   divisor_     = 1.;
      float   factor_      = 1.;
      uint8_t enable_byte_ = 0x01;

      float value_;
    };

#ifdef USE_TEXT_SENSOR
    class BsbTextSensor
        : public BsbSensorBase
        , public text_sensor::TextSensor {
    public:
      SensorType get_type() override { return SensorType::TextSensor; }
      void       publish() override { publish_state( value_ ); }

      void set_value( const std::string value ) { this->value_ = value; }

    protected:
      std::string value_;
    };
#endif

#ifdef USE_BINARY_SENSOR
    class BsbBinarySensor
        : public BsbSensorBase
        , public binary_sensor::BinarySensor {
    public:
      SensorType get_type() override { return SensorType::BinarySensor; }
      void       publish() override { publish_state( value_ ); }

      void set_enable_byte( const uint8_t enable_byte ) { this->enable_byte_ = enable_byte; }

      void set_value( uint32_t value ) {
        if( value == on_value_ ) {
          value_ = true;
        }
        if( value == off_value_ ) {
          value_ = false;
        }
      }

      void           set_on_value( const uint32_t on_value ) { this->on_value_ = on_value; }
      const uint32_t get_on_value() const { return this->on_value_; }

      void           set_off_value( const uint32_t off_value ) { this->off_value_ = off_value; }
      const uint32_t get_off_value() const { return this->off_value_; }

    protected:
      uint32_t on_value_    = 1;
      uint32_t off_value_   = 0;
      uint8_t  enable_byte_ = 0x01;

      bool value_;
    };
#endif

  } // namespace bsb
} // namespace esphome
