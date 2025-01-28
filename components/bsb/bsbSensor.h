#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
  namespace bsb {

    enum SensorType { SENSOR, TEXT_SENSOR };

    enum class BsbSensorValueType { UInt8, Int8, Int16, Int32, Temperature, RoomTemperature };

    class BsbSensorBase {
    public:
      virtual SensorType get_type() = 0;
      virtual void       publish()  = 0;

      void     set_field_id( const uint32_t fieldId ) { this->fieldId = fieldId; }
      uint32_t get_field_id() { return fieldId; }

      void     set_update_interval( const uint32_t val ) { updateInterval_ms = val; }
      uint32_t get_update_interval() { return updateInterval_ms; }

      void               set_value_type( const int type ) { this->valueType = ( BsbSensorValueType )type; }
      BsbSensorValueType get_value_type() { return this->valueType; }

      bool is_ready( const uint32_t timestamp ) {
        if( sent >= 5 ) {
          ESP_LOGE( TAG, "BsbSensor::is_ready(), sent >= 5: %08X", get_field_id() );
          if( timestamp >= ( nextUpdateTimestamp + 30000 ) ) {
            sent = 0;
          }
        }

        return ( sent < 5 ) && ( timestamp >= nextUpdateTimestamp );
      }
      void update_timestamp( const uint32_t timestamp ) {
        sent                = 0;
        nextUpdateTimestamp = timestamp + updateInterval_ms;
      }

      uint16_t sent = 0;

    protected:
      uint32_t           updateInterval_ms;
      BsbSensorValueType valueType = BsbSensorValueType::Temperature;

      uint32_t nextUpdateTimestamp = 0;

      uint32_t fieldId;
    };

    class BsbSensor
        : public BsbSensorBase
        , public sensor::Sensor {
    public:
      SensorType get_type() override { return SENSOR; }
      void       publish() override { publish_state( value ); }

      void set_value( float value ) { this->value = value * factor / divisor; }

      void set_divisor( const float divisor ) { this->divisor = divisor; }
      void set_factor( const float factor ) { this->factor = factor; }
      void set_enable_byte( const uint8_t enable_byte ) { this->enable_byte = enable_byte; }

    protected:
      float   divisor     = 1.;
      float   factor      = 1.;
      uint8_t enable_byte = 0x01;
      float   value;
    };

    class BsbTextSensor
        : public BsbSensorBase
        , public text_sensor::TextSensor {
    public:
      SensorType get_type() override { return TEXT_SENSOR; }
      void       publish() override { publish_state( value ); }

      void set_value( const std::string value ) { this->value = value; }

    protected:
      std::string value;
    };

  } // namespace bsb
} // namespace esphome
