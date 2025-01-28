
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
    class BsbPacketSet : public BsbPacket {
    public:
      BsbPacketSet( const uint8_t sourceAddress, const uint8_t destinationAddress, const uint32_t fieldId, bool broadcast = false )
          : BsbPacket() {
        if( broadcast ) {
          this->command = Command::Inf;
        } else {
          this->command = Command::Set;
        }

        this->sourceAddress      = sourceAddress;
        this->destinationAddress = broadcast ? 0x7f : destinationAddress;
        this->fieldId            = fieldId;
      }

      BsbPacketSet( const uint8_t  sourceAddress,
                    const uint8_t  destinationAddress,
                    const uint32_t fieldId,
                    const int8_t   value,
                    const bool     broadcast   = false,
                    const uint8_t  enable_byte = 0x01 )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId, broadcast ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value );

        createPacket();
      }

      BsbPacketSet( const uint8_t  sourceAddress,
                    const uint8_t  destinationAddress,
                    const uint32_t fieldId,
                    const int16_t  value,
                    const bool     broadcast   = false,
                    const uint8_t  enable_byte = 0x01 )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId, broadcast ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value >> 8 );
        payload.push_back( value );

        createPacket();
      }

      BsbPacketSet( const uint8_t  sourceAddress,
                    const uint8_t  destinationAddress,
                    const uint32_t fieldId,
                    const int32_t  value,
                    const bool     broadcast   = false,
                    const uint8_t  enable_byte = 0x01 )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId, broadcast ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value >> 24 );
        payload.push_back( value >> 16 );
        payload.push_back( value >> 8 );
        payload.push_back( value );

        createPacket();
      }

      BsbPacketSet( const uint8_t  sourceAddress,
                    const uint8_t  destinationAddress,
                    const uint32_t fieldId,
                    const float    value,
                    const bool     broadcast       = false,
                    const uint8_t  enable_byte     = 0x01,
                    const bool     roomTemperature = false )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId, int16_t( value * 64. ), broadcast, enable_byte ) {
        if( roomTemperature ) {
          payload.clear();

          int16_t val = value * 64.;
          payload.push_back( val >> 8 );
          payload.push_back( val );
          payload.push_back( 0 );
        }

        createPacket();
      }

      BsbPacketSet() = delete;
    };

    class BsbPacketGet : public BsbPacket {
    public:
      BsbPacketGet( uint8_t sourceAddress, uint8_t destinationAddress, uint32_t fieldId ) : BsbPacket() {
        this->command            = Command::Get;
        this->sourceAddress      = sourceAddress;
        this->destinationAddress = destinationAddress;
        this->fieldId            = fieldId;

        createPacket();
      }

      BsbPacketGet() = delete;
    };
  }
}
