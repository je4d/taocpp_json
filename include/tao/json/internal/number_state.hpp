// Copyright (c) 2016-2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#ifndef TAOCPP_JSON_INCLUDE_INTERNAL_NUMBER_STATE_HPP
#define TAOCPP_JSON_INCLUDE_INTERNAL_NUMBER_STATE_HPP

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <inttypes.h>
#include <stdexcept>

#include "../external/double.hpp"

namespace tao
{
   namespace json
   {
      namespace internal
      {
         static const std::size_t max_mantissa_digits = 772;

         struct number_state
         {
            using exponent10_t = int32_t;
            using msize_t = uint16_t;

            number_state()
            {
            }

            number_state( const number_state& ) = delete;
            void operator=( const number_state& ) = delete;

            exponent10_t exponent10 = 0;
            msize_t msize = 0;  // Excluding sign.
            bool isfp = false;
            bool mneg = false;
            bool eneg = false;
            bool drop = false;
            bool nan = false;
            bool infinity = false;
            char mantissa[ max_mantissa_digits + 1 ];

            template< typename Consumer, typename... Ts >
            void success( Consumer& consumer, Ts&&... )
            {
               if( !isfp && msize <= 20 ) {
                  mantissa[ msize ] = 0;
                  char* p;
                  errno = 0;
                  const std::uint64_t ull = std::strtoull( mantissa, &p, 10 );
                  if( ( errno != ERANGE ) && ( p == mantissa + msize ) ) {
                     if( mneg ) {
                        if( ull < 9223372036854775808ull ) {
                           consumer.number( -static_cast< std::int64_t >( ull ) );
                           return;
                        }
                        else if( ull == 9223372036854775808ull ) {
                           consumer.number( static_cast< std::int64_t >( -9223372036854775807ll - 1 ) );
                           return;
                        }
                     }
                     else {
                        consumer.number( ull );
                        return;
                     }
                  }
               }
               if( drop ) {
                  mantissa[ msize++ ] = '1';
                  --exponent10;
               }
               if( nan ) {
                  consumer.number( mneg ? -NAN : NAN );
                  return;
               }
               if( infinity ) {
                  consumer.number( mneg ? -INFINITY : INFINITY );
                  return;
               }
               const auto d = json_double_conversion::Strtod( json_double_conversion::Vector< const char >( mantissa, msize ), exponent10 );
               if( !std::isfinite( d ) ) {
                  throw std::runtime_error( "invalid double value" );
               }
               consumer.number( mneg ? -d : d );
            }
         };

      }  // namespace internal

   }  // namespace json

}  // namespace tao

#endif
