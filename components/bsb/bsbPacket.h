
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "esphome/core/helpers.h"

// BsbPacket and derived classes work with already flipped data!

namespace esphome {
  namespace bsb {
    class BsbPacket {
    public:
      enum class Command : uint8_t { None = 0, Inf = 2, Set = 3, Ack = 4, Nack = 5, Get = 6, Ret = 7 };

      BsbPacket() {
        buffer.reserve( 32 );
        payload.reserve( 16 );
      }

      static uint16_t CRC( const std::vector< uint8_t >::const_iterator& begin, const std::vector< uint8_t >::const_iterator& end ) {
        uint16_t crc = 0;

        std::for_each( begin, end, [&crc]( const uint8_t& b ) { crc = crc_xmodem_update( crc, b ); } );

        return crc;
      }

      static uint16_t crc_xmodem_update( uint16_t crc, uint8_t data ) {
        crc = crc ^ ( ( uint16_t )data << 8 );
        for( uint8_t i = 0; i < 8; i++ ) {
          if( crc & 0x8000 ) {
            crc = ( crc << 1 ) ^ 0x1021;
          } else {
            crc <<= 1;
          }
        }
        return crc;
      }

      std::string print_packet() const {
        std::string output;
        output = "BSB Packet: ";

        char str[100];

        switch( command ) {
          case Command::Inf:
            output += "Inf";
            break;
          case Command::Set:
            output += "Set";
            break;
          case Command::Ack:
            output += "Ack";
            break;
          case Command::Nack:
            output += "Nack";
            break;
          case Command::Get:
            output += "Get";
            break;
          case Command::Ret:
            output += "Ret";
            break;
          default:
            snprintf( str, 100, "UNK (%02hhX)", ( uint8_t )command );
            output += str;
            break;
        }

        snprintf( str, 100, " %02hhX->%02hhX, len: %2hhu", sourceAddress, destinationAddress, lenght );
        output += str;

        snprintf( str, 100, ", field: %08X, CRC: %04hX", fieldId, crc );
        output += str;

        if( payload.size() ) {
          output += ", payload: ";
          output += format_hex_pretty( payload );
        }

        output += " (";
        output += esphome::format_hex_pretty( buffer );
        output += ") ";

        return output;
      }

      int8_t parse_as_int8() const {
        if( payload.size() != 2 ) {
          return 0;
        }

        return payload.back();
      }

      uint8_t parse_as_uint8() const {
        if( payload.size() != 2 ) {
          return 0;
        }

        return payload.back();
      }

      int16_t parse_as_int16() const {
        if( payload.size() != 3 ) {
          return 0;
        }

        return payload[1] << 8 | payload[2];
      }

      float parse_as_temperature() const { return parse_as_int16() / 64.; }

      int32_t parse_as_int32() const {
        if( payload.size() != 5 ) {
          return 0;
        }

        return payload[1] << 24 | payload[2] << 16 | payload[3] << 8 | payload[4];
      }

      std::string parse_as_text() const { return std::string( payload.cbegin(), payload.cend() ); }

      std::string parse_as_time() const {
        if( payload.size() != 3 ) {
          return "";
        }

        uint8_t flag = payload.front();

        if( flag == 0x01 ) {
          return "00:00";
        } else {
          char str[10];
          snprintf( str, 10, "%02u:%02u", payload[1], payload[2] );

          return str;
        }
      }

      std::string parse_as_schedule() const {
        if( payload.size() != 12 ) {
          return "";
        }

        char str[100];
        snprintf( str,
                  100,
                  "%02u:%02u-%02u:%02u %02u:%02u-%02u:%02u %02u:%02u-%02u:%02u",
                  payload[0],
                  payload[1],
                  payload[2],
                  payload[3],
                  payload[4],
                  payload[5],
                  payload[6],
                  payload[7],
                  payload[8],
                  payload[9],
                  payload[10],
                  payload[11] );

        return str;
      }

      void create_packet() {
        buffer.clear();
        lenght = PacketSizeWithoutPyload + payload.size();

        buffer.push_back( 0xDC );
        buffer.push_back( sourceAddress | 0x80 );
        buffer.push_back( destinationAddress );
        buffer.push_back( lenght );
        buffer.push_back( ( uint8_t )command );

        if( command == Command::Get || command == Command::Set || command == Command::Inf ) {
          // beware: to send the first two bytes have to be swapped
          buffer.push_back( ( fieldId >> 16 ) & 0xFF );
          buffer.push_back( ( fieldId >> 24 ) & 0xFF );
        } else {
          buffer.push_back( ( fieldId >> 24 ) & 0xFF );
          buffer.push_back( ( fieldId >> 16 ) & 0xFF );
        }

        buffer.push_back( ( fieldId >> 8 ) & 0xFF );
        buffer.push_back( ( fieldId ) & 0xFF );
        buffer.insert( buffer.end(), payload.cbegin(), payload.cend() );

        crc = CRC( buffer.cbegin(), buffer.cend() );
        buffer.push_back( ( crc >> 8 ) & 0xFF );
        buffer.push_back( ( crc ) & 0xFF );

        lenght = buffer.size();
      }

      std::vector< uint8_t > buffer;
      uint8_t                sourceAddress;
      uint8_t                destinationAddress;
      uint8_t                lenght;
      Command                command;
      uint32_t               fieldId;
      std::vector< uint8_t > payload;
      uint16_t               crc;

      static constexpr uint8_t PacketSizeWithoutPyload = 11;
    };
  }
}
