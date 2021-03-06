/*
 *****************************************************************
 *                     String Toolkit Library                    *
 *                                                               *
 * String Toolkit Tokenizer vs BOOST Tokenizer                   *
 * Author: Arash Partow - 2003                                   *
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


/*
   Description: This comparison aims to determine the running time difference between
                the Boost string tokenizer and the String Toolkit Library's string tokenizer.
                The comparison involves each tokenizer accepting a set of possible delimiters
                and a string to be tokenized. The provided string is tokenized and the tokens
                are subsequently concatenated to each other.
                Running time is measured around the tokenization loop, and printed out along
                with a measure indicating the average number of tokens parsed per second.
                Furthermore, the tests include a simple comparison of integer, double to string
                and vice-versa conversion routines between the standard library, BOOST and
                String Toolkit.

                Definitions:
                tks/sec  : Tokens per second
                nums/sec : numbers per second
*/


#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <iterator>

#include "strtk.hpp"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

//Note: Define USE_SPIRIT to include Karma and Qi (requires Boost 1.42+)
//#define USE_SPIRIT
#ifdef USE_SPIRIT
 #define INCLUDE_KARMA
 #define INCLUDE_QI_S2I
 #define INCLUDE_QI_S2D
 #include <boost/spirit/include/qi.hpp>
 #include <boost/spirit/include/karma.hpp>
#endif

#ifdef WIN32
 #include <windows.h>
#else
 #include <sys/time.h>
 #include <sys/types.h>
#endif

#ifdef WIN32

class timer
{
public:
   timer()      { QueryPerformanceFrequency(&clock_frequency); }
   void start() { QueryPerformanceCounter(&start_time);        }
   void stop()  { QueryPerformanceCounter(&stop_time);         }
   double time(){ return (1.0 *(stop_time.QuadPart - start_time.QuadPart)) / (1.0 * clock_frequency.QuadPart); }
 private:
  LARGE_INTEGER start_time;
  LARGE_INTEGER stop_time;
  LARGE_INTEGER clock_frequency;
};

#else

class timer
{
public:
   void start() { gettimeofday(&start_time, 0); }
   void stop()  { gettimeofday(&stop_time,  0); }
   double time()
   {
      double diff = (stop_time.tv_sec - start_time.tv_sec) * 1000000.0;
      if (stop_time.tv_usec > start_time.tv_usec)
         diff += (1.0 * (stop_time.tv_usec - start_time.tv_usec));
      else if (stop_time.tv_usec < start_time.tv_usec)
         diff -= (1.0 * (start_time.tv_usec - stop_time.tv_usec));

      return (diff / 1000000.0);
   }
 private:
  struct timeval start_time;
  struct timeval stop_time;
  struct timeval clock_frequency;
};

#endif

static const std::string base = "a|bc?def,ghij;klmno_p|qr?stu,vwxy;z1234_56789 0 A|BC?DEF,GHIJ;KLMNO_P|QR?STU,VWXY;Z1234_56789";
static const std::size_t replicate_count = 1000000;
static const std::string delimiters = "|?,;_ ";

void print_mode(const std::string& mode)
{
   static const std::size_t test_mode_length = sizeof("[sprintf]");
   printf("%s", strtk::text::left_align(test_mode_length, ' ', mode).c_str());
   fflush(stdout);
}

void strtk_tokenizer_timed_test()
{
   print_mode("[strtk]");
   std::string s = "";
   std::string so = "";
   s.reserve(base.size() * replicate_count);
   so.reserve(s.size());
   for (unsigned int i = 0; i < replicate_count; ++i) s += base;
   strtk::multiple_char_delimiter_predicate predicate(delimiters);
   strtk::std_string::tokenizer<strtk::multiple_char_delimiter_predicate>::type tokenizer(s,predicate);
   strtk::std_string::tokenizer<strtk::multiple_char_delimiter_predicate>::type::iterator itr = tokenizer.begin();
   unsigned int token_count = 0;
   timer t;
   t.start();
   while (itr != tokenizer.end())
   {
      so += std::string((*itr).first,(*itr).second);
      ++itr;
      ++token_count;
   }
   t.stop();
   printf("Token Count: %u\tTotal time: %8.4fsec\tRate: %8.4ftks/sec\n",
          token_count,
          t.time(),
          token_count / (1.0 * t.time()));
}

void boost_tokenizer_timed_test()
{
   print_mode("[boost]");
   std::string s = "";
   std::string so = "";
   s.reserve(base.size() * replicate_count);
   so.reserve(s.size());
   for (unsigned int i = 0; i < replicate_count; ++i) s += base;
   typedef boost::tokenizer<boost::char_separator<char> > tokenizer_type;
   tokenizer_type tokenizer(s,boost::char_separator<char>(delimiters.c_str()));
   tokenizer_type::iterator itr = tokenizer.begin();
   unsigned int token_count = 0;
   timer t;
   t.start();
   while (itr != tokenizer.end())
   {
      so += *itr;
      ++itr;
      ++token_count;
   }
   t.stop();
   printf("Token Count: %u\tTotal time: %8.4fsec\tRate: %8.4ftks/sec\n",
          token_count,
          t.time(),
          token_count / (1.0 * t.time()));
}

void strtk_split_timed_test()
{
   print_mode("[strtk]");
   std::string s = "";
   s.reserve(base.size() * replicate_count);
   for (unsigned int i = 0; i < replicate_count; ++i) s += base;
   std::deque<std::string> token_list;
   timer t;
   t.start();
   strtk::parse(s,delimiters,token_list);
   t.stop();
   printf("Token Count: %lu\tTotal time: %8.4fsec\tRate: %8.4ftks/sec\n",
          static_cast<unsigned long>(token_list.size()),
          t.time(),
          token_list.size() / (1.0 * t.time()));
}

void boost_split_timed_test()
{
   print_mode("[boost]");
   std::string s = "";
   s.reserve(base.size() * replicate_count);
   for (unsigned int i = 0; i < replicate_count; ++i) s += base;
   std::deque<std::string> token_list;
   timer t;
   t.start();
   boost::split(token_list, s, boost::is_any_of(delimiters));
   t.stop();
   printf("Token Count: %lu\tTotal time: %8.4fsec\tRate: %8.4ftks/sec\n",
          static_cast<unsigned long>(token_list.size()),
          t.time(),
          token_list.size() / (1.0 * t.time()));
}

void sprintf_lexical_cast_test_i2s()
{
   print_mode("[sprintf]");
   std::string s;
   s.reserve(32);
   const std::size_t max = 10000000;
   std::size_t total_length = 0;
   timer t;
   t.start();
   for (int i = (-static_cast<int>(max) / 2); i < (static_cast<int>(max) / 2); ++i)
   {
      s.resize(sprintf(const_cast<char*>(s.c_str()),"%d",i));
      total_length += s.size();
   }
   t.stop();
   printf("Numbers: %lu\tTotal length: %lu\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(max),
          static_cast<unsigned long>(total_length),
          t.time(),
          max / (1.0 * t.time()));
}

void boost_lexical_cast_test_i2s()
{
   print_mode("[boost]");
   std::string s;
   s.reserve(32);
   const std::size_t max = 10000000;
   std::size_t total_length = 0;
   timer t;
   t.start();
   for (int i = (-static_cast<int>(max) / 2); i < (static_cast<int>(max) / 2); ++i)
   {
      s = boost::lexical_cast<std::string>(i);
      total_length += s.size();
   }
   t.stop();
   printf("Numbers: %lu\tTotal length: %lu\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(max),
          static_cast<unsigned long>(total_length),
          t.time(),
          max / (1.0 * t.time()));
}

#ifdef INCLUDE_KARMA
#include <boost/spirit/include/karma.hpp>
inline bool karma_int_to_string(const int& value, std::string& str)
{
  using namespace boost::spirit;
  using boost::spirit::karma::generate;
  char buffer[16];
  char* x = buffer;
  if (!generate(x, int_, value))
     return false;
  str.assign(buffer, x - buffer);
  return true;
}

void karma_lexical_cast_test_i2s()
{
   print_mode("[karma]");
   std::string s;
   s.reserve(32);
   const std::size_t max = 10000000;
   std::size_t total_length = 0;
   timer t;
   t.start();
   for (int i = (-static_cast<int>(max) / 2); i < (static_cast<int>(max) / 2); ++i)
   {
      karma_int_to_string(i,s);
      total_length += s.size();
   }
   t.stop();
   printf("Numbers: %lu\tTotal length: %lu\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(max),
          static_cast<unsigned long>(total_length),
          t.time(),
          max / (1.0 * t.time()));
}
#else
void karma_lexical_cast_test_i2s(){}
#endif

void strtk_lexical_cast_test_i2s()
{
   print_mode("[strtk]");
   std::string s;
   s.reserve(32);
   const std::size_t max = 10000000;
   std::size_t total_length = 0;
   timer t;
   t.start();
   for (int i = (-static_cast<int>(max) / 2); i < (static_cast<int>(max) / 2); ++i)
   {
      strtk::type_to_string(i,s);
      total_length += s.size();
   }
   t.stop();
   printf("Numbers: %lu\tTotal length: %lu\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(max),
          static_cast<unsigned long>(total_length),
          t.time(),
          max / (1.0 * t.time()));
}

static const std::string strint_list[] =
             {
                       "0",         "1",         "2",         "3",         "4",         "5",         "6",         "7",         "8",         "9",
                  "917049",   "4931205",   "6768064",   "6884243",   "5647132",   "7371203",  "-8629878",   "4941840",   "4543268",   "1075600",
                    "+290",       "823",      "+111",       "715",      "-866",      "+367",       "666",      "-706",       "850",      "-161",
                 "9922547",   "6960207",   "1883152",   "2300759",   "-279294",   "4187292",   "3699841",  "+8386395",  "-1441129",   "-887892",
                 "-635422",   "9742573",   "2326186",  "-5903851",   "5648486",   "3057647",   "2980079",   "2957468",   "7929158",   "1925615",
                     "879",      "+130",       "292",       "705",       "817",       "446",       "576",       "750",       "523",      "-527",
                 "4365041",   "5624958",   "8990205",   "2652177",   "3993588",   "-298316",   "2901599",   "3887387",  "-5202979",   "1196268",
                 "5968501",   "7619928",   "3565643",   "1885272",   "-749485",   "2961381",   "2982579",   "2387454",   "4250081",   "5958205",
                   "00000",     "00001",     "00002",     "00003",     "00004",     "00005",     "00006",     "00007",     "00008",     "00009",
                 "4907034",   "2592882",   "3269234",    "549815",   "6256292",   "9721039",   "-595225",  "+5587491",   "4596297",  "-3885009",
                     "673",      "-899",       "174",       "354",       "870",       "147",       "898",      "-510",       "369",      "+859",
                 "6518423",   "5149762",   "8834164",  "-8085586",   "3233120",   "8166948",   "4172345",   "6735549",   "-934295",   "9481935",
                 "-430406",   "6932717",   "4087292",   "4047263",   "3236400",  "-3863050",   "4312079",   "6956261",   "5689446",   "3871332",
                    "+535",       "691",      "+326",      "-409",       "704",      "-568",      "+301",       "951",       "121",       "384",
                 "4969414",   "9378599",   "7971781",   "5380630",   "5001363",   "1715827",   "6044615",   "9118925",   "9956168",  "-8865496",
                 "5962464",   "7408980",   "6646513",   "-634564",   "4188330",   "9805948",   "5625691",  "+7641113",  "-4212929",   "7802447",
                      "+0",        "+1",        "+2",        "+3",        "+4",        "+5",        "+6",        "+7",        "+8",        "+9",
                 "2174248",   "7449361",   "9896659",    "-25961",   "1706598",   "2412368",  "-4617035",   "6314554",   "2225957",   "7521434",
                "-9530566",   "3914164",   "2394759",   "7157744",   "9919392",   "6406949",   "-744004",   "9899789",   "8380325",  "-1416284",
                 "3402833",   "2150043",   "5191009",   "8979538",   "9565778",   "3750211",   "7304823",   "2829359",   "6544236",   "-615740",
                     "363",      "-627",       "129",      "+656",       "135",       "113",       "381",      "+646",       "198",        "38",
                 "8060564",   "-176752",   "1184717",   "-666343",  "-1273292",   "-485827",   "6241066",   "6579411",   "8093119",   "7481306",
                "-4924485",   "7467889",   "9813178",   "7927100",   "3614859",   "7293354",   "9232973",   "4323115",   "1133911",   "9511638",
                 "4443188",   "2289448",   "5639726",   "9073898",   "8540394",   "5389992",   "1397726",   "-589230",   "1017086",   "1852330",
                    "-840",       "267",       "201",       "533",      "-675",       "494",       "315",       "706",      "-920",       "784",
                 "9097353",   "6002251",   "-308780",  "-3830169",   "4340467",   "2235284",   "3314444",   "1085967",   "4152107",  "+5431117",
                   "-0000",     "-0001",     "-0002",     "-0003",     "-0004",     "-0005",     "-0006",     "-0007",     "-0008",     "-0009",
                 "-444999",   "2136400",   "6925907",   "6990614",   "3588271",   "8422028",  "-4034772",   "5804039",  "-6740545",   "9381873",
                 "-924923",   "1652367",   "2302616",   "6776663",   "2567821",   "-248935",   "2587688",   "7076742",  "-6461467",   "1562896",
                 "-768116",   "2338768",   "9887307",   "9992184",   "2045182",   "2797589",   "9784597",   "9696554",   "5113329",   "1067216",
               "-76247763",  "58169007",  "29408062",  "85342511",  "42092201", "-95817703",  "-1912517", "-26275135",  "54656606", "-58188878",
                    "+473",        "74",       "374",       "-64",       "266",       "715",       "937",      "-249",       "249",       "780",
                 "3907360", "-23063423",  "59062754",  "83711047", "-95221044",  "34894840", "-38562139", "-82018330",  "14226223", "-10799717",
                 "8529722",  "88961903",  "25608618", "-39988247",  "33228241",  "38598533",  "21161480", "-33723784",   "8873948",  "96505557",
               "-47385048", "-79413272", "-85904404",  "87791158",  "49194195",  "13051222",  "57773302",  "31904423",   "3142966",  "27846156",
                 "7420011", "-72376922", "-68873971",  "23765361",   "4040725", "-22359806",  "85777219",  "10099223", "-90364256", "-40158172",
                "-7948696", "-64344821",  "34404238",  "84037448", "-85084788", "-42078409", "-56550310",  "96898389",   "-595829", "-73166703",
                      "-0",        "-1",        "-2",        "-3",        "-4",        "-5",        "-6",        "-7",        "-8",        "-9",
               "-82838342",  "64441808",  "43641062", "-64419642", "-44421934",  "75232413", "-75773725", "-89139509",  "12812089", "-97633526",
                "36090916", "-57706234",  "17804655",   "4189936",  "-4100124",  "38803710", "-39735126", "-62397437",  "75801648",  "51302332",
                "73433906",  "13015224", "-12624818",  "91360377",  "11576319", "-54467535",   "8892431",  "36319780",  "38832042",  "50172572",
                    "-317",       "109",      "-888",       "302",      "-463",       "716",       "916",       "665",       "826",       "513",
                "42423473",  "41078812",  "40445652", "-76722281",  "95092224",  "12075234",  "-4045888", "-74396490", "-57304222", "-21726885",
                "92038121", "-31899682",  "21589254", "-30260046",  "56000244",  "69686659",  "93327838",  "96882881", "-91419389",  "77529147",
                "43288506",   "1192435", "-74095920",  "76756590", "-31184683", "-35716724",   "9451980", "-63168350",  "62864002",  "26283194",
                "37188395",  "29151634",  "99343471", "-69450330", "-55680090", "-64957599",  "47577948",  "47107924",   "2490477",  "48633003",
               "-82740809", "-24122215",  "67301713", "-63649610",  "75499016",  "82746620",  "17052193",   "4602244", "-32721165",  "20837836",
                     "674",      "+467",      "+706",       "889",       "172",      "+282",      "-795",       "188",       "+87",       "153",
                "64501793",  "53146328",   "5152287",  "-9674493",  "68105580",  "57245637",  "39740229", "-74071854",  "86777268",  "86484437",
               "-86962508",  "12644427", "-62944073",  "59539680",  "43340539",  "30661534",  "20143968", "-68183731", "-48250926",  "42669063",
                    "+000",      "+001",      "+002",      "+003",      "+004",      "+005",      "+006",      "+007",      "+008",      "+009",
                "87736852",  "-4444906", "-48094147",  "54774735",  "54571890", "-22473078",  "95053418",    "393654", "-33229960",  "32276798",
               "-48361110",  "44295939", "-79813406",  "11630865",  "38544571",  "70972830",  "-9821748", "-60965384", "-13096675", "-24569041",
                     "708",      "-467",      "-794",       "610",      "+929",       "766",       "152",       "482",       "397",      "-191",
                "97233152",  "51028396", "-13796948",  "95437272",  "71352512", "-83233730", "-68517318",  "61832742", "-42667174", "-18002395",
               "-92239407",  "12701336", "-63830875",  "41514172",  "-5726049",  "18668677",  "69555144", "-13737009", "-22626233", "-55078143",
             };
static const std::size_t strint_list_size = sizeof(strint_list) / sizeof(std::string);

void atoi_lexical_cast_test_s2i()
{
   print_mode("[atoi]");
   const std::size_t rounds = 100000;
   int total = 0;
   int n = 0;
   timer t;
   t.start();
   for (std::size_t x = 0; x < rounds; ++x)
   {
      for (std::size_t i = 0; i < strint_list_size; ++i)
      {
         n = ::atoi(strint_list[i].c_str());
         total += n;
      }
   }
   t.stop();
   printf("Numbers: %lu\tTotal: %d\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * strint_list_size),
          total,
          t.time(),
          (rounds * strint_list_size) / (1.0 * t.time()));
}

void boost_lexical_cast_test_s2i()
{
   print_mode("[boost]");
   const std::size_t rounds = 100000;
   int total = 0;
   int n = 0;
   timer t;
   t.start();
   for (std::size_t x = 0; x < rounds; ++x)
   {
      for (std::size_t i = 0; i < strint_list_size; ++i)
      {
         n = boost::lexical_cast<int>(strint_list[i]);
         total += n;
      }
   }
   t.stop();
   printf("Numbers: %lu\tTotal: %d\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * strint_list_size),
          total,
          t.time(),
          (rounds * strint_list_size) / (1.0 * t.time()));
}

#ifdef INCLUDE_QI_S2I
inline bool qi_string_to_int(const std::string& str, int & value)
{
  using namespace boost::spirit;
  using boost::spirit::qi::parse;
  char* begin = const_cast<char*>(str.c_str());
  char* end = begin + str.size();
  return parse(begin,end, int_, value);
}

void qi_lexical_cast_test_s2i()
{
   print_mode("[qi]");
   const std::size_t rounds = 100000;
   int total = 0;
   int n = 0;
   timer t;
   t.start();
   for (std::size_t x = 0; x < rounds; ++x)
   {
      for (std::size_t i = 0; i < strint_list_size; ++i)
      {
         qi_string_to_int(strint_list[i],n);
         total += n;
      }
   }
   t.stop();
   printf("Numbers: %lu\tTotal: %d\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * strint_list_size),
          total,
          t.time(),
          (rounds * strint_list_size) / (1.0 * t.time()));
}
#else
 void qi_lexical_cast_test_s2i() {}
#endif

void strtk_lexical_cast_test_s2i()
{
   print_mode("[strtk]");
   const std::size_t rounds = 100000;
   int total = 0;
   int n = 0;
   timer t;
   t.start();
   for (std::size_t x = 0; x < rounds; ++x)
   {
      for (std::size_t i = 0; i < strint_list_size; ++i)
      {
         strtk::string_to_type_converter(strint_list[i],n);
         total += n;
      }
   }
   t.stop();
   printf("Numbers: %lu\tTotal: %d\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * strint_list_size),
          total,
          t.time(),
          (rounds * strint_list_size) / (1.0 * t.time()));
}

static const std::string v[] = {
                                   "+004",                     "+005",                      "+006",               "+007",
                                   "+929",                      "766",                       "152",                "482",
                        "7379973.6624700",         "-2187496.9290991",            "384281.3720002",   "-6603499.4918257",
                                      "0",                        "1",                         "2",                  "3",
                                    "879",                     "+130",                       "292",                "705",
                                  "00000",                    "00001",                     "00002",              "00003",
                       "-8017367.1481995",           "633693.2789865",           "-734898.6366299",    "+295494.2335175",
                                                  "+1.0", "+2.0", "+3.0", "+4.0", "+5.0", "+6.0", "+7.0", "+8.0", "+9.0",
                       "+6924520.6426735",         "-8219629.8338549",           "7201183.8124936",   "-4505255.6409222",
                                    "708",                     "-467",                      "-794",                "610",
                       "-4125977.4243394",         "+7770880.2044321",           "-936309.6851826",    "7140657.6729261",
                        "3374994.2530944",         "-5933062.7257800",          "+8664205.9476258",   "-7144810.5337246",
                                   "-317",                      "109",                      "-888",                "302",
                                    "674",                     "+467",                      "+706",                "889",
                                           "-.0",  "-.1",  "-.2",  "-.3",  "-.4",  "-.5",  "-.6",  "-.7",  "-.8",  "-.9",
                                           "+.0",  "+.1",  "+.2",  "+.3",  "+.4",  "+.5",  "+.6",  "+.7",  "+.8",  "+.9",
                                   "-840",                      "267",                       "201",                "533",
                                  "-0000",                    "-0001",                     "-0002",              "-0003",
                      "03374994.25309440",       "-05933062.72578000",        "+08664205.94762580", "-07144810.53372460",
                                    "363",                     "-627",                       "129",               "+656",
                      "+0391059.70005360",        "08018726.47909600",        "-07762532.52738580",  "+0191677.58537590",
                                                           "1.0", "2.0", "3.0", "4.0", "5.0", "6.0", "7.0", "8.0", "9.0",
                     "-02610193.23338290",       "-05536426.67419050",         "09868713.74445040", "-04187461.84990080",
                                    "870",                      "147",                       "898",               "-510",
                                     "-4",                       "-5",                        "-6",                 "-7",
                       "08623428.8529150",        "02932018.49830140",        "-09816436.72970750",  "02183188.58659720",
                                                   "-1.0", "-2.0", "-3.0", "-4.0", "-5.0", "-6.0", "-7.0","-8.0", "-9.0",
                     "-04663787.27402000",        "-08719869.9822470",        "+08001455.68056840",  "-01800837.1690770",
                                   "+000",                     "+001",                      "+002",               "+003",
                     "+06987314.11422220",        "+0112136.87073620",        "-02120543.96610760", "+03903634.53641490",
                                   "+473",                       "74",                       "374",                "-64",
                     "-05446002.77153270",       "-04552784.60364020",         "06178318.60245260",  "-0340491.53994340",
                       "07557628.4385290",        "03899301.70689570",         "-0292643.48119340",  "08639015.45291470",
                                    "704",                     "-568",                      "+301",                "951",
                     "-01566675.10980440",       "-03695567.66366290",        "+05888357.88912280",  "-0550930.46713720",
                      ".00e+0", ".01e+1", ".02e+2", ".03e+3", ".04e+4", ".05e+5", ".06e+6", ".07e+7", ".08e+8", ".09e+9",
                      ".0e+00", ".1e+01", ".2e+02", ".3e+03", ".4e+04", ".5e+05", ".6e+06", ".7e+07", ".8e+08", ".9e+09",
                         "0.668364937915",          "-0.491401141241",           "-0.333129579862",    "-0.365807899800",
                                     "+0",                       "+1",                        "+2",                 "+3",
                                  "00004",                    "00005",                     "00006",              "00007",
                                   "-463",                      "716",                       "916",                "665",
                        "-0.742998928523",          "+0.795740637927",           "+0.738890192202",     "0.838420189244",
                                   "+290",                      "823",                      "+111",                "715",
                         "0.136955290570",          "-0.528184172516",           "-0.922485979112",    "-0.832170413268",
                        "-0.945828557068",           "0.879257644420",            "0.244450812369",    "+0.305035653490",
                                   "+535",                      "691",                      "+326",               "-409",
                        "+0.125634105601",          "-0.194399721385",           "-0.820243318264",    "-0.880230396260",
                        "-0.360550459424",          "+0.565867297929",           "+0.443986746193",     "0.327767716805",
                                    "172",                     "+282",                      "-795",                "188",
                         "0.586630908690",          "-0.605294588008",           "-0.517170498609",    "-0.883414731381",
                                                    "123.456e3",                "123.456E3",                "123.456e+3",
                                                   "123.456E+3",               "123.456e03",                "123.456E03",
                                                  "123.456e+03",              "123.456E+03",              "123.456e0003",
                                                 "123.456E0003",            "123.456e+0003",             "123.456E+0003",
                       ".0E+0",  ".1E+1",  ".2E+2",  ".3E+3",  ".4E+4",  ".5E+5",  ".6E+6",  ".7E+7",  ".8E+8",  ".9E+9",
                      "+.0e+0", "+.1e+1", "+.2e+2", "+.3e+3", "+.4e+4", "+.5e+5", "+.6e+6", "+.7e+7", "+.8e+8", "+.9e+9",
                                                   "123.4560e3",              "123.45600E3",             "123.456000e+3",
                                                "123.456000E+3",           "123.4560000e03",           "123.45600000E03",
                                            "123.456000000e+03",       "123.4560000000E+03",      "123.45600000000e0003",
                                        "123.456000000000E0003",  "123.4560000000000e+0003",  "123.45600000000000E+0003",
                                               "+0123.456e+003",          "-00123.456E+003",           "+000123.45600E3",
                                                   "-123.456e3",               "+123.456E3",               "-123.456e+3",
                                                  "+123.456E+3",              "-123.456e03",               "+123.456E03",
                                                 "-123.456e+03",             "+123.456E+03",             "-123.456e0003",
                                                    "+0.", "+1.", "+2.", "+3.", "+4.", "+5.", "+6.", "+7.", "+8.", "+9.",
                                                    "-0.", "-1.", "-2.", "-3.", "-4.", "-5.", "-6.", "-7.", "-8.", "-9.",
                                                "+123.456E0003",           "-123.456e+0003",            "+123.456E+0003",
                                                  "-123.4560e3",             "+123.45600E3",            "-123.456000e+3",
                                               "+123.456000E+3",          "-123.4560000e03",          "+123.45600000E03",
                                           "-123.456000000e+03",      "+123.4560000000E+03",     "-123.45600000000e0003",
                                                     "0.",  "1.",  "2.",  "3.",  "4.",  "5.",  "6.",  "7.",  "8.",  "9.",
                                                     ".0",  ".1",  ".2",  ".3",  ".4",  ".5",  ".6",  ".7",  ".8",  ".9",
                                       "+123.456000000000E0003", "-123.4560000000000e+0003", "+123.45600000000000E+0003",
                                               "-0123.456e+003",          "+00123.456E+003",           "-000123.45600E3",
                                          "000000123.456e+0003",                      "0.0",                     "00.00",
                                                      "000.000",                "0000.0000",                     "1234.",
                                                         "01.2",                 "0012.340",               "00012.34500",
                                          "+0.0", "+1.0", "+2.0", "+3.0", "+4.0", "+5.0", "+6.0", "+7.0", "+8.0", "+9.0",
                                           "-0.595932",  "+0.780005", "-0.892725",  "0.241765", "-0.168054", "+0.687922",
                                           "+0.326186",  "-0.557688",  "0.524772", "-0.316610", "+0.451794", "-0.233149",
                       ".0e+0",  ".1e+1",  ".2e+2",  ".3e+3",  ".4e+4",  ".5e+5",  ".6e+6",  ".7e+7",  ".8e+8",  ".9e+9",
                      "-.0e+0", "-.1e+1", "-.2e+2", "-.3e+3", "-.4e+4", "-.5e+5", "-.6e+6", "-.7e+7", "-.8e+8", "-.9e+9",
                                           "-0.513733",   "0.827170", "-0.324329", "+0.944863", "-0.824447",  "0.963773",
                                            "0.782587",  "-0.941195", "+0.977240", "-0.140493",  "0.884950", "-0.503467",
                      ".00E+0", ".01E+1", ".02E+2", ".03E+3", ".04E+4", ".05E+5", ".06E+6", ".07E+7", ".08E+8", ".09E+9",
                      ".0E+00", ".1E+01", ".2E+02", ".3E+03", ".4E+04", ".5E+05", ".6E+06", ".7E+07", ".8E+08", ".9E+09",
                                           "-0.609128",  "+0.480234", "-0.125272", "+0.404477", "-0.687858", "+0.454091",
                                           "+0.628872",  "-0.438190",  "0.779150", "-0.965766", "+0.384968", "-0.620200",
                                           "-0.681751",   "0.517473", "-0.700753", "+0.940319", "-0.882250",  "0.591237",
                                           "0.0",  "1.0",  "2.0",  "3.0",  "4.0",  "5.0",  "6.0",  "7.0",  "8.0",  "9.0"
                  };

static const std::size_t v_size = sizeof(v) / sizeof(std::string);
static const std::size_t rounds = 50000;

void atof_cast_test_s2d()
{
   print_mode("[atof]");
   double sum = 0.0;
   double d   = 0.0;
   timer t;
   t.start();
   for (std::size_t r = 0; r < rounds; ++r)
   {
      for (std::size_t i = 0; i < v_size; ++i)
      {
         d = atof(v[i].c_str());
         if (r & 1)
            sum += d;
         else
            sum -= d;
      }
   }
   t.stop();
   printf("Numbers: %lu\tError: %12.10f\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * v_size),
          sum,
          t.time(),
          (rounds * v_size) / (1.0 * t.time()));
}

void boost_cast_test_s2d()
{
   print_mode("[boost]");
   double sum = 0.0;
   double d   = 0.0;
   timer t;
   t.start();
   for (std::size_t r = 0; r < rounds; ++r)
   {
      for (std::size_t i = 0; i < v_size; ++i)
      {
         d = boost::lexical_cast<double>(v[i]);
         if (r & 1)
            sum += d;
         else
            sum -= d;
      }
   }
   t.stop();
   printf("Numbers: %lu\tError: %12.10f\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * v_size),
          sum,
          t.time(),
          (rounds * v_size) / (1.0 * t.time()));
}

#ifdef INCLUDE_QI_S2D
inline bool qi_string_to_double(const std::string& str, double& value)
{
  using namespace boost::spirit;
  using boost::spirit::qi::parse;
  char* begin = const_cast<char*>(str.c_str());
  char* end = begin + str.size();
  return parse(begin,end, double_, value);
}

void qi_cast_test_s2d()
{
   print_mode("[qi]");
   double sum = 0.0;
   double d   = 0.0;
   timer t;
   t.start();
   for (std::size_t r = 0; r < rounds; ++r)
   {
      for (std::size_t i = 0; i < v_size; ++i)
      {
         qi_string_to_double(v[i],d);
         if (r & 1)
            sum += d;
         else
            sum -= d;
      }
   }
   t.stop();
   printf("Numbers: %lu\tError: %12.10f\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * v_size),
          sum,
          t.time(),
          (rounds * v_size) / (1.0 * t.time()));
}
#else
 void qi_cast_test_s2d(){}
#endif

void strtk_cast_test_s2d()
{
   print_mode("[strtk]");
   double sum = 0.0;
   double d   = 0.0;
   timer t;
   t.start();
   for (std::size_t r = 0; r < rounds; ++r)
   {
      for (std::size_t i = 0; i < v_size; ++i)
      {
         strtk::string_to_type_converter(v[i],d);
         if (r & 1)
            sum += d;
         else
            sum -= d;
      }
   }
   t.stop();
   printf("Numbers: %lu\tError: %12.10f\tTotal time: %8.4fsec\tRate: %8.4fnums/sec\n",
          static_cast<unsigned long>(rounds * v_size),
          sum,
          t.time(),
          (rounds * v_size) / (1.0 * t.time()));
}

int main()
{
   std::cout << "Tokenizer Test" << std::endl;
   boost_tokenizer_timed_test();
   strtk_tokenizer_timed_test();
   std::cout << "Split Test" << std::endl;
   boost_split_timed_test();
   strtk_split_timed_test();
   std::cout << "Integer To String Test" << std::endl;
   sprintf_lexical_cast_test_i2s();
   boost_lexical_cast_test_i2s();
   karma_lexical_cast_test_i2s();
   strtk_lexical_cast_test_i2s();
   std::cout << "String To Integer Test" << std::endl;
   atoi_lexical_cast_test_s2i();
   boost_lexical_cast_test_s2i();
   qi_lexical_cast_test_s2i();
   strtk_lexical_cast_test_s2i();
   std::cout << "String To Double Test" << std::endl;
   atof_cast_test_s2d();
   boost_cast_test_s2d();
   qi_cast_test_s2d();
   strtk_cast_test_s2d();
   return 0;
}
