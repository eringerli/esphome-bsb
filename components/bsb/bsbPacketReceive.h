
#include <algorithm>
#include <cstdint>

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "esphome/core/helpers.h"

#include "bsbPacket.h"

namespace esphome {
  namespace bsb {
    class BsbPacketReceive : public BsbPacket {
    public:
      enum class ProtocolStates { Start, SourceAddr, DestAddr, Lenght, Type, FieldId1, FieldId2, FieldId3, FieldId4, Payload, CRC1, CRC2 };

      BsbPacketReceive( std::function< void( const BsbPacket* ) > callback ) : callback( callback ), BsbPacket() {}

      BsbPacketReceive() = delete;

      void loop( const uint8_t data ) {
        // ESP_LOGE( "BsbPacketReceive", "State: %u (%x)", ( uint8_t )state, data );

        switch( state ) {
          case ProtocolStates::Start:
            buffer.clear();

            if( data == 0xDC ) {
              buffer.push_back( data );
              state = ProtocolStates::SourceAddr;
            }
            break;

          case ProtocolStates::SourceAddr:
            if( data & 0x80 ) {
              buffer.push_back( data );
              sourceAddress = data & 0x7F;
              state         = ProtocolStates::DestAddr;
            } else {
              state = ProtocolStates::Start;
            }
            break;

          case ProtocolStates::DestAddr:
            buffer.push_back( data );
            destinationAddress = data;
            state              = ProtocolStates::Lenght;
            break;

          case ProtocolStates::Lenght:
            buffer.push_back( data );
            lenght = data;
            state  = ProtocolStates::Type;
            break;

          case ProtocolStates::Type:
            buffer.push_back( data );
            command = ( Command )data;
            state   = ProtocolStates::FieldId1;
            break;

          case ProtocolStates::FieldId1:
            buffer.push_back( data );
            fieldId = data << 24;

            state = ProtocolStates::FieldId2;
            break;

          case ProtocolStates::FieldId2:
            buffer.push_back( data );
            fieldId |= data << 16;

            state = ProtocolStates::FieldId3;
            break;

          case ProtocolStates::FieldId3:
            buffer.push_back( data );
            fieldId |= data << 8;

            state = ProtocolStates::FieldId4;
            break;

          case ProtocolStates::FieldId4:
            buffer.push_back( data );
            fieldId |= data;

            payload.clear();

            if( lenght > PacketSizeWithoutPyload ) {
              state = ProtocolStates::Payload;
            } else {
              state = ProtocolStates::CRC1;
            }
            break;

          case ProtocolStates::Payload:
            buffer.push_back( data );
            payload.push_back( data );
            if( payload.size() == ( lenght - PacketSizeWithoutPyload ) ) {
              state = ProtocolStates::CRC1;
            }

            break;

          case ProtocolStates::CRC1:
            buffer.push_back( data );

            crc = data << 8;

            state = ProtocolStates::CRC2;
            break;

          case ProtocolStates::CRC2:
            buffer.push_back( data );

            crc |= data;

            uint16_t crcCalculated = CRC( buffer.cbegin(), buffer.cend() - 2 );

            // ESP_LOGE( "BsbPacketReceive", "CRC: %u|%u", crc, crcCalculated );
            //
            // ESP_LOGD( "BsbPacketReceive", "%s", printPacket().c_str() );

            if( crc == crcCalculated ) {
              // parsePaload();
              callback( this );
            }

            state = ProtocolStates::Start;
            break;
        }
      }

      std::function< void( const BsbPacket* ) > callback;
      ProtocolStates                            state = ProtocolStates::Start;
    };
  }
}
