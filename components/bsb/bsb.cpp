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
      LOG_UPDATE_INTERVAL( this );
      ESP_LOGCONFIG( TAG, "  query interval: %.3fs", this->query_interval / 1000.0f );
      ESP_LOGCONFIG( TAG, "  source address: 0x%02X", this->source_address );
      ESP_LOGCONFIG( TAG, "  destination address: 0x%02X", this->destination_address );

      ESP_LOGCONFIG( TAG, "  Sensors:" );
      for( const auto& item : sensors ) {
        BsbSensorBase* s = item.second;
        // ESP_LOGCONFIG( TAG, "    parameter number: %u", s->get_parameter_number() );
        ESP_LOGCONFIG( TAG, "  - field ID: 0x%08X", s->get_field_id() );
        ESP_LOGCONFIG( TAG, "    update_interval: %.3fs", s->get_update_interval() / 1000.0f );
      }
      ESP_LOGCONFIG( TAG, "  Numbers:" );
      for( const auto& item : numbers ) {
        BsbNumber* n = item.second;
        // ESP_LOGCONFIG( TAG, "    parameter number: %u", s->get_parameter_number() );
        ESP_LOGCONFIG( TAG, "  - field ID: 0x%08X", n->get_field_id() );
        ESP_LOGCONFIG( TAG, "    update_interval: %.3fs", n->get_update_interval() / 1000.0f );
      }
    }

    void BsbComponent::loop() {
      const uint32_t now = millis();

      while( this->available() ) {
        bsbPacketReceive.loop( this->read() ^ 0xff );
      }

      if( now > lastQuery ) {
        lastQuery = now + query_interval;

        bool packetSent = false;

        for( auto& number : numbers ) {
          if( number.second->is_ready_to_set( now ) ) {
            switch( number.second->get_value_type() ) {
              case BsbNumberValueType::UInt8: /*{
                auto packet = BsbPacketSet( source_address,
                                            destination_address,
                                            number.second->get_field_id(),
                                            ( uint8_t )(number.second->getValueToSend()),
                                            number.second->broadcast,
                                            number.second->enable_byte );

                writePacket( packet );
              } break;*/
              case BsbNumberValueType::Int8: {
                auto packet = BsbPacketSet( source_address,
                                            destination_address,
                                            number.second->get_field_id(),
                                            ( int8_t )( number.second->getValueToSend() ),
                                            number.second->broadcast,
                                            number.second->enable_byte );

                writePacket( packet );
              } break;

              case BsbNumberValueType::Int16: {
                auto packet = BsbPacketSet( source_address,
                                            destination_address,
                                            number.second->get_field_id(),
                                            ( int16_t )( number.second->getValueToSend() ),
                                            number.second->broadcast,
                                            number.second->enable_byte );

                writePacket( packet );
              } break;

              case BsbNumberValueType::Int32: {
                auto packet = BsbPacketSet( source_address,
                                            destination_address,
                                            number.second->get_field_id(),
                                            ( int32_t )( number.second->getValueToSend() ),
                                            number.second->broadcast,
                                            number.second->enable_byte );

                writePacket( packet );
              } break;

              case BsbNumberValueType::Temperature:
              case BsbNumberValueType::RoomTemperature: {
                auto packet = BsbPacketSet( source_address,
                                            destination_address,
                                            number.second->get_field_id(),
                                            ( float )( number.second->getValueToSend() ),
                                            number.second->broadcast,
                                            number.second->enable_byte,
                                            number.second->get_value_type() == BsbNumberValueType::RoomTemperature );

                writePacket( packet );
              } break;

              default:
                ESP_LOGE( TAG, "switch( number.second->get_value_type() ): %u", uint8_t( number.second->get_value_type() ) );
            }

            if( number.second->broadcast ) {
              number.second->reset_dirty();
            }

            number.second->sent++;

            packetSent = true;
            break;
          }
          if( number.second->is_ready_to_update( now ) ) {
            auto packet = BsbPacketGet( source_address, destination_address, number.second->get_field_id() );

            writePacket( packet );

            // number.second->update_timestamp( millis() );

            number.second->sent++;

            packetSent = true;
            break;
          }
        }

        if( !packetSent ) {
          for( auto& sensor : sensors ) {
            if( sensor.second->is_ready( now ) ) {
              auto packet = BsbPacketGet( source_address, destination_address, sensor.second->get_field_id() );

              writePacket( packet );

              sensor.second->sent++;

              // sensor.second->update_timestamp( millis() );

              break;
            }
          }
        }
      }
    }

    void BsbComponent::callbackPacket( const BsbPacket* packet ) {
      ESP_LOGD( TAG, "<<< %s", ( packet->printPacket() ).c_str() );

      if( packet->command == BsbPacket::Command::Inf || packet->command == BsbPacket::Command::Ret ) {
        {
          auto range = sensors.equal_range( packet->fieldId );

          for( auto sensor = range.first; sensor != range.second; ++sensor ) {
            // ESP_LOGE( TAG, "%08X|%08X", sensor.second->get_field_id(), packet->fieldId );

            if( sensor->second->get_type() == SensorType::SENSOR ) {
              BsbSensor* bsbSensor = ( BsbSensor* )sensor->second;
              bsbSensor->update_timestamp( millis() );
              switch( bsbSensor->get_value_type() ) {
                case BsbSensorValueType::UInt8:
                  bsbSensor->set_value( packet->parseAsUInt8() );
                  break;
                case BsbSensorValueType::Int8:
                  bsbSensor->set_value( packet->parseAsInt8() );
                  break;
                case BsbSensorValueType::Int16:
                  bsbSensor->set_value( packet->parseAsInt16() );
                  break;
                case BsbSensorValueType::Int32:
                  bsbSensor->set_value( packet->parseAsInt32() );
                  break;
                case BsbSensorValueType::Temperature:
                  bsbSensor->set_value( packet->parseAsTemperature() );
                  break;
              }
              bsbSensor->publish();
            }
            if( sensor->second->get_type() == SensorType::TEXT_SENSOR ) {
              BsbTextSensor* bsbSensor = ( BsbTextSensor* )sensor->second;
              bsbSensor->update_timestamp( millis() );
              bsbSensor->set_value( packet->parseAsText() );
              bsbSensor->publish();
            }
          }
        }

        {
          auto range = numbers.equal_range( packet->fieldId );
          for( auto number = range.first; number != range.second; ++number ) {
            // ESP_LOGE( TAG, "%08X|%08X", sensor.second->get_field_id(), packet->fieldId );

            BsbNumber* bsbNumber = number->second;
            bsbNumber->update_timestamp( millis() );
            switch( bsbNumber->get_value_type() ) {
              case BsbNumberValueType::Int8:
                bsbNumber->publish_state( packet->parseAsInt8() );
                break;
              case BsbNumberValueType::Int16:
                bsbNumber->publish_state( packet->parseAsInt16() );
                break;
              case BsbNumberValueType::Int32:
                bsbNumber->publish_state( packet->parseAsInt32() );
                break;
              case BsbNumberValueType::Temperature:
                bsbNumber->publish_state( packet->parseAsTemperature() );
                break;
            }
          }
        }
      }

      if( packet->command == BsbPacket::Command::Ack || packet->command == BsbPacket::Command::Nack ) {
        auto range = numbers.equal_range( packet->fieldId );

        for( auto sensor = range.first; sensor != range.second; ++sensor ) {
          // ESP_LOGE( TAG, "%08X|%08X", sensor.second->get_field_id(), packet->fieldId );

          sensor->second->reset_dirty();
        }
      }
    }

    void BsbComponent::sendNumber( const BsbNumber* number ) {}

    void BsbComponent::writePacket( const BsbPacket& packet ) {
      ESP_LOGD( TAG, ">>> %s", ( packet.printPacket() ).c_str() );

      auto buffer = packet.buffer;
      for( auto& b : buffer ) {
        b ^= 0xff;
      }
      write_array( buffer );
    }

  } // namespace bsb
} // namespace esphome
