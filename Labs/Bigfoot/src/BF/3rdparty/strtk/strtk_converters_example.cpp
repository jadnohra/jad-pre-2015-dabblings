/*
 *****************************************************************
 *                     String Toolkit Library                    *
 *                                                               *
 * Converters Examples                                           *
 * Author: Arash Partow (2002-2010)                              *
 * URL: http://www.partow.net/programming/strtk/index.html       *
 *                                                               *
 * Copyright notice:                                             *
 * Free use of the String Toolkit Library is permitted under the *
 * guidelines and in accordance with the most current version of *
 * the Common Public License.                                    *
 * http://www.opensource.org/licenses/cpl1.0.php                 *
 *                                                               *
 *****************************************************************
*/


#include <cstddef>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <string>
#include <deque>

#include "strtk.hpp"

void example01()
{
   std::string text = "A professional is a person who knows more and more about less and less "
                      "until they know everything about nothing.";
   std::string hex;
   std::string base64;
   std::string hexbin;
   std::string base64bin;

   strtk::convert_bin_to_hex(text,hex);
   strtk::convert_hex_to_bin(hex,hexbin);
   strtk::convert_bin_to_base64(text,base64);
   strtk::convert_base64_to_bin(base64,base64bin);

   std::cout << text << std::endl;
   std::cout << hex << std::endl;
   std::cout << hexbin << std::endl;
   std::cout << base64 << std::endl;
   std::cout << base64bin << std::endl;
}

void example02()
{
   static const std::size_t size = 1024;
   unsigned char buffer[size];
   unsigned char hex_buffer[2 * size];
   unsigned char b64_buffer[2 * size];
   for (std::size_t i = 0; i < size; ++i) buffer[i] = static_cast<unsigned char>(i);
   strtk::convert_bin_to_hex(buffer,buffer + size, hex_buffer);
   strtk::convert_bin_to_base64(buffer,buffer + size, b64_buffer);
}

void example03()
{
   static const std::size_t size = 1024;
   unsigned char buffer1[size];
   unsigned char buffer2[size];
   unsigned char b64_buffer[2 * size];
   for (std::size_t k = 1; k < size; ++k)
   {
      for (std::size_t i = 0; i < k; ++i) buffer1[i] = static_cast<unsigned char>(i);
      std::size_t b64_size = strtk::convert_bin_to_base64(buffer1,buffer1 + k, b64_buffer);
      std::fill_n(buffer2,k,0x00);
      strtk::convert_base64_to_bin(b64_buffer,b64_buffer + b64_size, buffer2);
      for (std::size_t i = 0; i < k; ++i)
      {
         if (buffer1[i] != buffer2[i])
         {
            std::cout << "example03 - failed bin-b64 conversions, error at position: " << i << std::endl;
            return;
         }
      }
   }
}

int main()
{
   example01();
   example02();
   example03();
   return 0;
}
