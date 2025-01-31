#include "bsb.h"
#include "bsbNumber.h"
#include "bsbPacket.h"
#include "bsbPacketReceive.h"
#include "bsbPacketSend.h"
#include "bsbSensor.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <cstdio>
#include <cstring>

namespace esphome {
  namespace bsb {

    const char* const TAG = "bsb.component";

    BsbComponent::BsbComponent() {}

    void BsbComponent::setup() { ESP_LOGCONFIG( TAG, "Setting up BSB component..." ); }

    void BsbComponent::dump_config() {
      ESP_LOGCONFIG( TAG, "BSB:" );
      ESP_LOGCONFIG( TAG, "  query interval: %.3fs", this->query_interval_ / 1000.0f );
      ESP_LOGCONFIG( TAG, "  retry interval: %.3fs", this->retry_interval_ / 1000.0f );
      ESP_LOGCONFIG( TAG, "  source address: 0x%02X", this->source_address_ );
      ESP_LOGCONFIG( TAG, "  destination address: 0x%02X", this->destination_address_ );

      ESP_LOGCONFIG( TAG, "  Sensors:" );
      for( const auto& item : sensors_ ) {
        BsbSensorBase* s = item.second;
        // ESP_LOGCONFIG( TAG, "    parameter number: %u", s->get_parameter_number() );
        switch( s->get_type() ) {
          case SensorType::Sensor:
            ESP_LOGCONFIG( TAG, "  - type: Sensor" );
            ESP_LOGCONFIG( TAG, "    factor: %.3f", ( ( BsbSensor* )s )->get_factor() );
            ESP_LOGCONFIG( TAG, "    divisor: %.3f", ( ( BsbSensor* )s )->get_divisor() );
            break;

          case SensorType::TextSensor:
            ESP_LOGCONFIG( TAG, "  - type: Text Sensor" );
            break;
        }
        ESP_LOGCONFIG( TAG, "    field ID: 0x%08X", s->get_field_id() );
        ESP_LOGCONFIG( TAG, "    update_interval: %.3fs", s->get_update_interval() / 1000.0f );
      }
      ESP_LOGCONFIG( TAG, "  Numbers:" );
      for( const auto& item : numbers_ ) {
        BsbNumber* n = item.second;
        // ESP_LOGCONFIG( TAG, "    parameter number: %u", s->get_parameter_number() );
        ESP_LOGCONFIG( TAG, "  - field ID: 0x%08X", n->get_field_id() );
        ESP_LOGCONFIG( TAG, "    update_interval: %.3fs", n->get_update_interval() / 1000.0f );
        ESP_LOGCONFIG( TAG, "    factor: %.3f", n->get_factor() );
        ESP_LOGCONFIG( TAG, "    divisor: %.3f", n->get_divisor() );
        ESP_LOGCONFIG( TAG, "    broadcast: %s", YESNO( n->get_broadcast() ) );
      }
    }

    void BsbComponent::loop() {
      const uint32_t now = millis();

      while( this->available() ) {
        bsbPacketReceive.loop( this->read() ^ 0xff );
      }

      if( now > last_query_ ) {
        last_query_ = now + query_interval_;

        bool packetSent = false;

        for( auto& number : numbers_ ) {
          if( number.second->is_ready_to_set( now ) ) {
            write_packet( number.second->createPackageSet( source_address_, destination_address_ ) );

            if( number.second->get_broadcast() ) {
              number.second->reset_dirty();
              number.second->publish_state( number.second->state );
            } else {
              number.second->schedule_next_update( now, IntervalGetAfterSet );
            }

            packetSent = true;
            break;
          }
          if( number.second->is_ready_to_update( now ) ) {
            write_packet( number.second->createPackageGet( source_address_, destination_address_ ) );

            packetSent = true;
            break;
          }
        }

        if( !packetSent ) {
          for( auto& sensor : sensors_ ) {
            if( sensor.second->is_ready( now ) ) {
              write_packet( sensor.second->createPackageGet( source_address_, destination_address_ ) );

              break;
            }
          }
        }
      }
    }

    void BsbComponent::callback_packet( const BsbPacket* packet ) {
      ESP_LOGD( TAG, "<<< %s", ( packet->print_packet() ).c_str() );

      if( packet->command == BsbPacket::Command::Inf || packet->command == BsbPacket::Command::Ret ) {
        {
          auto range = sensors_.equal_range( packet->fieldId );

          for( auto sensor = range.first; sensor != range.second; ++sensor ) {
            switch( sensor->second->get_type() ) {
              case SensorType::Sensor: {
                BsbSensor* bsbSensor = ( BsbSensor* )sensor->second;
                bsbSensor->schedule_next_regular_update( millis() );
                switch( bsbSensor->get_value_type() ) {
                  case BsbSensorValueType::UInt8:
                    bsbSensor->set_value( packet->parse_as_uint8() );
                    break;
                  case BsbSensorValueType::Int8:
                    bsbSensor->set_value( packet->parse_as_int8() );
                    break;
                  case BsbSensorValueType::Int16:
                    bsbSensor->set_value( packet->parse_as_int16() );
                    break;
                  case BsbSensorValueType::Int32:
                    bsbSensor->set_value( packet->parse_as_int32() );
                    break;
                  case BsbSensorValueType::Temperature:
                    bsbSensor->set_value( packet->parse_as_temperature() );
                    break;
                }
                bsbSensor->publish();
              } break;

              case SensorType::TextSensor: {
                BsbTextSensor* bsbSensor = ( BsbTextSensor* )sensor->second;
                bsbSensor->schedule_next_regular_update( millis() );
                bsbSensor->set_value( packet->parse_as_text() );
                bsbSensor->publish();
              } break;
            }
          }
        }

        {
          auto range = numbers_.equal_range( packet->fieldId );
          for( auto number = range.first; number != range.second; ++number ) {
            BsbNumber* bsbNumber = number->second;
            bsbNumber->schedule_next_regular_update( millis() );
            switch( bsbNumber->get_value_type() ) {
              case BsbNumberValueType::UInt8:
                bsbNumber->set_value( packet->parse_as_uint8() );
                break;
              case BsbNumberValueType::Int8:
                bsbNumber->set_value( packet->parse_as_int8() );
                break;
              case BsbNumberValueType::Int16:
                bsbNumber->set_value( packet->parse_as_int16() );
                break;
              case BsbNumberValueType::Int32:
                bsbNumber->set_value( packet->parse_as_int32() );
                break;
              case BsbNumberValueType::Temperature:
                bsbNumber->set_value( packet->parse_as_temperature() );
                break;
              default:
                break;
            }
          }
        }
      }

      if( packet->command == BsbPacket::Command::Ack || packet->command == BsbPacket::Command::Nack ) {
        auto range = numbers_.equal_range( packet->fieldId );

        for( auto sensor = range.first; sensor != range.second; ++sensor ) {
          sensor->second->reset_dirty();
        }
      }
    }

    void BsbComponent::write_packet( const BsbPacket& packet ) {
      if( !packet.buffer.empty() ) {
        ESP_LOGD( TAG, ">>> %s", ( packet.print_packet() ).c_str() );

        auto buffer = packet.buffer;
        for( auto& b : buffer ) {
          b ^= 0xff;
        }
        write_array( buffer );
      }
    }

  } // namespace bsb
} // namespace esphome
