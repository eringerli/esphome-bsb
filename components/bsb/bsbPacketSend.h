
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
      BsbPacketSet( const uint8_t sourceAddress, const uint8_t destinationAddress, const uint32_t fieldId ) : BsbPacket() {
        this->command = Command::Set;

        this->sourceAddress      = sourceAddress;
        this->destinationAddress = destinationAddress;
        this->fieldId            = fieldId;
      }

      BsbPacketSet() = delete;
    };

    class BsbPacketSetUInt8 : public BsbPacketSet {
    public:
      BsbPacketSetUInt8( const uint8_t  sourceAddress,
                         const uint8_t  destinationAddress,
                         const uint32_t fieldId,
                         const int8_t   value,
                         const uint8_t  enable_byte )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value );

        create_packet();
      }

      BsbPacketSetUInt8() = delete;
    };

    class BsbPacketSetInt8 : public BsbPacketSet {
    public:
      BsbPacketSetInt8( const uint8_t  sourceAddress,
                        const uint8_t  destinationAddress,
                        const uint32_t fieldId,
                        const int8_t   value,
                        const uint8_t  enable_byte )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value );

        create_packet();
      }

      BsbPacketSetInt8() = delete;
    };

    class BsbPacketSetInt16 : public BsbPacketSet {
    public:
      BsbPacketSetInt16( const uint8_t  sourceAddress,
                         const uint8_t  destinationAddress,
                         const uint32_t fieldId,
                         const int16_t  value,
                         const uint8_t  enable_byte )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value >> 8 );
        payload.push_back( value );

        create_packet();
      }

      BsbPacketSetInt16() = delete;
    };

    class BsbPacketSetInt32 : public BsbPacketSet {
    public:
      BsbPacketSetInt32( const uint8_t  sourceAddress,
                         const uint8_t  destinationAddress,
                         const uint32_t fieldId,
                         const int32_t  value,
                         const uint8_t  enable_byte )
          : BsbPacketSet( sourceAddress, destinationAddress, fieldId ) {
        if( enable_byte == 0x06 && value == 0. ) {
          payload.push_back( 0x05 );
        } else {
          payload.push_back( enable_byte );
        }

        payload.push_back( value >> 24 );
        payload.push_back( value >> 16 );
        payload.push_back( value >> 8 );
        payload.push_back( value );

        create_packet();
      }

      BsbPacketSetInt32() = delete;
    };

    class BsbPacketSetTemperature : public BsbPacketSetInt16 {
    public:
      BsbPacketSetTemperature( const uint8_t  sourceAddress,
                               const uint8_t  destinationAddress,
                               const uint32_t fieldId,
                               const float    value,
                               const uint8_t  enable_byte )
          : BsbPacketSetInt16( sourceAddress, destinationAddress, fieldId, int16_t( value * 64. ), enable_byte ) {
        create_packet();
      }

      BsbPacketSetTemperature() = delete;
    };

    class BsbPacketInf : public BsbPacket {
    public:
      BsbPacketInf( const uint8_t sourceAddress, const uint32_t fieldId ) : BsbPacket() {
        this->command = Command::Inf;

        this->sourceAddress      = sourceAddress;
        this->destinationAddress = 0x7f;
        this->fieldId            = fieldId;
      }

      BsbPacketInf() = delete;
    };

    class BsbPacketInfTemperature : public BsbPacketInf {
    public:
      BsbPacketInfTemperature( const uint8_t sourceAddress, const uint32_t fieldId, const float value, const uint8_t enable_byte = 0x01 )
          : BsbPacketInf( sourceAddress, fieldId ) {
        int16_t val = value * 64.;
        payload.push_back( enable_byte );
        payload.push_back( val >> 8 );
        payload.push_back( val );

        create_packet();
      }

      BsbPacketInfTemperature() = delete;
    };

    class BsbPacketInfRoomTemperature : public BsbPacketInf {
    public:
      BsbPacketInfRoomTemperature( const uint8_t sourceAddress, const uint32_t fieldId, const float value, const uint8_t enable_byte )
          : BsbPacketInf( sourceAddress, fieldId ) {
        int16_t val = value * 64.;
        payload.push_back( val >> 8 );
        payload.push_back( val );
        payload.push_back( 0 );

        create_packet();
      }

      BsbPacketInfRoomTemperature() = delete;
    };

    class BsbPacketGet : public BsbPacket {
    public:
      BsbPacketGet( uint8_t sourceAddress, uint8_t destinationAddress, uint32_t fieldId ) : BsbPacket() {
        this->command            = Command::Get;
        this->sourceAddress      = sourceAddress;
        this->destinationAddress = destinationAddress;
        this->fieldId            = fieldId;

        create_packet();
      }

      BsbPacketGet() = delete;
    };
  }
}
