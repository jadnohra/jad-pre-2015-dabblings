/*
 *****************************************************************
 *                     String Toolkit Library                    *
 *                                                               *
 * Serializer Example                                            *
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
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <deque>
#include <list>

#include "strtk.hpp"


struct person
{
public:
   std::string  name;
   unsigned int age;
   double       height;
   float        weight;
   bool         is_insane;

   person()
   {
      clear();
   }

   bool operator==(const person& p)
   {
      return (p.name       ==       name) &&
             (p.age        ==        age) &&
             (p.weight     ==     weight) &&
             (p.height     ==     height) &&
             (p.is_insane  ==  is_insane);
   }

   bool operator!=(const person& p)
   {
      return !operator==(p);
   }

   void clear()
   {
      name = "";
      age = 0;
      height = 0.0;
      weight = 0.0f;
      is_insane = false;
   }

   void read(strtk::serializer& s)
   {
      s >> name;
      s >> age;
      s >> height;
      s >> weight;
      s >> is_insane;
   }

   void write(strtk::serializer& s)
   {
      s << name;
      s << age;
      s << height;
      s << weight;
      s << is_insane;
   }
};


bool test01(char* buffer, const unsigned int buffer_size)
{
   strtk::serializer s(buffer,buffer_size);
   s.clear();

   person p1;
   person p2;

   p1.name      = "Mr. Rumpelstilzchen";
   p1.age       = 637;
   p1.height    = 123.4567;
   p1.weight    = 765.345f;
   p1.is_insane = true;

   p1.write(s);

   s.reset();

   p2.read(s);

   if (p1 != p2)
   {
      std::cout << "Test01 failed!" << std::endl;
      return false;
   }

   return true;
}

bool test02(char* buffer, const unsigned int buffer_size)
{
   strtk::serializer s(buffer,buffer_size);
   s.clear();

   person p1;

   p1.name       = "Mr. Rumpelstilzchen";
   p1.age        = 0;
   p1.height     = 123.4567;
   p1.weight     = 765.345f;
   p1.is_insane  = true;

   const std::size_t rounds = 1000;

   for (unsigned int i = 0; i < rounds; ++i)
   {
      p1.write(s);
      p1.age++;
      p1.height += 1.23;
      p1.weight += 4.567f;
      p1.is_insane = !p1.is_insane;
   }

   s.reset();

   p1.name      = "Mr. Rumpelstilzchen";
   p1.age       = 0;
   p1.height    = 123.4567;
   p1.weight    = 765.345f;
   p1.is_insane = true;

   person p2;

   for (unsigned int i = 0; i < rounds; ++i)
   {
      p2.clear();
      p2.read(s);
      if (p1 != p2)
      {
         std::cout << "Test02 failed!" << std::endl;
         return false;
      }
      p1.age++;
      p1.height += 1.23;
      p1.weight += 4.567f;
      p1.is_insane = !p1.is_insane;
   }

   return true;
}

bool test03(char* buffer, const unsigned int buffer_size)
{
   strtk::serializer s(buffer,buffer_size);
   s.clear();

   person p1;

   p1.name      = "Mr. Rumpelstilzchen";
   p1.age       = 0;
   p1.height    = 123.4567;
   p1.weight    = 765.345f;
   p1.is_insane = true;

   const std::size_t rounds = 1000;

   for (unsigned int i = 0; i < rounds; ++i)
   {
      p1.write(s);
      p1.age++;
      p1.height += 1.23;
      p1.weight += 4.567f;
      p1.is_insane = !p1.is_insane;
   }

   std::ofstream o_stream("data.txt",std::ios::binary);
   if (!o_stream)
   {
      std::cout << "Test03() ERROR - Could not open file!(1)" << std::endl;
      return false;
   }

   s.write_to_stream(o_stream);
   o_stream.close();

   std::size_t length = s.length();

   std::ifstream i_stream("data.txt",std::ios::binary);
   if (!i_stream)
   {
      std::cout << "Test03() ERROR - Could not open file!(2)" << std::endl;
      return false;
   }

   s.read_from_stream(i_stream,length);
   s.reset();

   p1.name      = "Mr. Rumpelstilzchen";
   p1.age       = 0;
   p1.height    = 123.4567;
   p1.weight    = 765.345f;
   p1.is_insane = true;

   person p2;

   for (unsigned int i = 0; i < rounds; ++i)
   {
      p2.clear();
      p2.read(s);
      if (p1 != p2)
      {
         std::cout << "Test03 failed!" << std::endl;
         return false;
      }
      p1.age++;
      p1.height += 1.23;
      p1.weight += 4.567f;
      p1.is_insane = !p1.is_insane;
   }
   return true;
}

bool test04(char* buffer, const unsigned int buffer_size)
{
   {
      // Read out then back in an array of unsigned ints.
      strtk::serializer s(buffer,buffer_size);
      s.clear();
      std::deque<unsigned int> lst;
      const unsigned int max_count = 1000;
      for (unsigned int i = 0; i < max_count; lst.push_back(i++)) ;

      s.write_from_external_sequence(lst);

      lst.clear();
      s.reset();

      s.read_into_external_sequence(lst);

      for (unsigned int i = 0; i < max_count; ++i)
      {
         if (lst[i] != i)
         {
            std::cout << "test04 - 'unsigned int' failure at index: " << i << std::endl;
            return false;
         }
      }
   }

   {
      // Read out then back in an array of floats.
      strtk::serializer s(buffer,buffer_size);
      s.clear();
      std::vector<float> lst;
      const unsigned int max_count = 1000;
      const std::size_t magic_count = 6;
      const float magic[magic_count] = { 111.111f, 333.333f, 555.555f,
                                         777.777f, 135.531f, 357.753f };
      for (unsigned int i = 0; i < max_count; ++i)
      {
         lst.push_back(magic[i % magic_count] * i);
      }

      s.write_from_external_sequence(lst);

      lst.clear();
      s.reset();

      s.read_into_external_sequence(lst);

      for (unsigned int i = 0; i < max_count; ++i)
      {
         const float d = magic[i % magic_count] * i;
         if (lst[i] != d)
         {
            std::cout << "test04 - 'float' failure at index: " << i
                      << " expected value: "                 << d << std::endl;
            return false;
         }
      }
   }

   {
      // Read out then back in an array of doubles.
      strtk::serializer s(buffer,buffer_size);
      s.clear();
      std::list<double> lst;
      const unsigned int max_count = 1000;
      const std::size_t magic_count = 6;
      const double magic[magic_count] = { 111.111, 333.333, 555.555,
                                          777.777, 135.531, 357.753 };
      for (unsigned int i = 0; i < max_count; ++i)
      {
         lst.push_back(magic[i % magic_count] * i);
      }

      s.write_from_external_sequence(lst);

      lst.clear();
      s.reset();

      s.read_into_external_sequence(lst);

      std::list<double>::iterator itr = lst.begin();
      for (unsigned int i = 0; i < max_count; ++i, ++itr)
      {
         const double d = magic[i % magic_count] * i;
         if (*itr != d)
         {
            std::cout << "test04 - 'double' failure at index: " << i
                      << " expected value: "                  << d << std::endl;
            return false;
         }
      }
   }

   return true;
}

bool test05(char* buffer)
{
   char in_char = -17;
   unsigned char in_uchar = 200;
   short in_short = -20000;
   unsigned short in_ushort = 55555;
   int in_int = -1111111;
   unsigned int in_uint = 79797979;
   long in_long = -43294761;
   unsigned long in_ulong = 78292365;
   float in_float = 1234.5678f;
   double in_double = 9876.54321;
   long double in_ldouble = 456789.123456;

   char out_char = 0;
   unsigned char out_uchar = 0;
   short out_short = 0;
   unsigned short out_ushort = 0;
   int out_int = 0;
   unsigned int out_uint = 0;
   long out_long = 0;
   unsigned long out_ulong = 0;
   float out_float = 0.0f;
   double out_double = 0.0;
   long double out_ldouble = 0.0;

   unsigned char* ptr = reinterpret_cast<unsigned char*>(buffer);

   strtk::write_pod(ptr,
                    in_char,
                    in_uchar,
                    in_short,
                    in_ushort,
                    in_int,
                    in_uint,
                    in_long,
                    in_ulong,
                    in_float,
                    in_double,
                    in_ldouble);

   strtk::read_pod(ptr,
                   out_char,
                   out_uchar,
                   out_short,
                   out_ushort,
                   out_int,
                   out_uint,
                   out_long,
                   out_ulong,
                   out_float,
                   out_double,
                   out_ldouble);

   if (in_char    != out_char)    { std::cout << "test05 - Failed char"    << std::endl; return false; }
   if (in_uchar   != out_uchar)   { std::cout << "test05 - Failed uchar"   << std::endl; return false; }
   if (in_short   != out_short)   { std::cout << "test05 - Failed short"   << std::endl; return false; }
   if (in_ushort  != out_ushort)  { std::cout << "test05 - Failed ushort"  << std::endl; return false; }
   if (in_int     != out_int)     { std::cout << "test05 - Failed int"     << std::endl; return false; }
   if (in_uint    != out_uint)    { std::cout << "test05 - Failed uint"    << std::endl; return false; }
   if (in_long    != out_long)    { std::cout << "test05 - Failed long"    << std::endl; return false; }
   if (in_ulong   != out_ulong)   { std::cout << "test05 - Failed ulong"   << std::endl; return false; }
   if (in_float   != out_float)   { std::cout << "test05 - Failed float"   << std::endl; return false; }
   if (in_double  != out_double)  { std::cout << "test05 - Failed double"  << std::endl; return false; }
   if (in_ldouble != out_ldouble) { std::cout << "test05 - Failed ldouble" << std::endl; return false; }

   return true;
}

bool test06(char* buffer)
{
   const size_t size = 10;
   const int intlst[size] = { -1, 2, -3, 4, -5, 6, -7, 8, -9, 10 };
   const unsigned int uintlst[size] = { 
                                        734594, 1375762, 5432543,     3454, 32132, 
                                         65463,  976765, 2355754, 74239542, 32523 
                                      };
   const float fltlst[size] = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.10f };
   const double dbllst[size] = { 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.10 };

   int r_intlst[size] = { 0 };
   unsigned int r_uintlst[size] = { 0 };
   float r_fltlst[size] = { 0.0f };
   double r_dbllst[size] = { 0.0 };

   unsigned char* ptr = reinterpret_cast<unsigned char*>(buffer);
   ptr = strtk::write_pod(ptr,intlst);
   ptr = strtk::write_pod(ptr,uintlst);
   ptr = strtk::write_pod(ptr,fltlst);
   ptr = strtk::write_pod(ptr,dbllst);

   ptr = reinterpret_cast<unsigned char*>(buffer);
   ptr = strtk::read_pod(ptr,r_intlst);
   ptr = strtk::read_pod(ptr,r_uintlst);
   ptr = strtk::read_pod(ptr,r_fltlst);
   ptr = strtk::read_pod(ptr,r_dbllst);

   if (!std::equal(intlst, intlst + size, r_intlst))
   {
      std::cout << "test06 - failed int list compare." << std::endl;
      return false;
   }

   if (!std::equal(uintlst, uintlst + size, r_uintlst))
   {
      std::cout << "test06 - failed unsigned int list compare." << std::endl;
      return false;
   }

   if (!std::equal(fltlst, fltlst + size, r_fltlst))
   {
      std::cout << "test06 - failed float list compare." << std::endl;
      return false;
   }

   if (!std::equal(dbllst, dbllst + size, r_dbllst))
   {
      std::cout << "test06 - failed double list compare." << std::endl;
      return false;
   }

   return true;
}

int main()
{
   static const std::size_t max_buffer_size = 64 * strtk::one_kilobyte; // 64KB
   char* buffer = new char[max_buffer_size];
   test01(buffer,max_buffer_size);
   test02(buffer,max_buffer_size);
   test03(buffer,max_buffer_size);
   test04(buffer,max_buffer_size);
   test05(buffer);
   test06(buffer);
   delete[] buffer;
   return 0;
}
