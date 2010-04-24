#include "LoaderBVH.h"
#include "strtk/strtk.hpp"

namespace BF
{

struct parse_line
{
public:

   parse_line()
   {
	   mDelimiters = " \t\r";
   }

   inline void operator()(const std::string& s)
   {
	   strtk::multiple_char_delimiter_predicate predicate(mDelimiters);
	   std::vector<std::string> strlist;
	   
	   strtk::split(predicate,s,strtk::range_to_type_back_inserter(strlist));
   }

private:

	std::string mDelimiters;
};

bool LoaderBVH::Load(const char* inFilePath, Skeleton& outSkeleton)
{
	 parse_line pl;
	 strtk::for_each_line(std::string(inFilePath),pl);

	return false;	
}

/*
#include <iostream>
#include <iterator>
#include <utility>
#include <string>
#include <deque>

#include "strtk.hpp"

template<typename Container, typename Predicate>
struct parse_line
{
public:

   parse_line(Container& c, Predicate& p)
   : c_(&c),
     p_(&p)
   {}

   inline void operator()(const std::string& s)
   {
      strtk::split(*p_,
                    s,
                    strtk::range_to_type_back_inserter(*c_),
                    strtk::split_options::compress_delimiters);
   }

private:
   Container* c_;
   Predicate* p_;
};

template<typename Container>
void parse_text(const std::string& file_name, Container& c)
{
   std::string delimiters = " ,.;:<>'[]{}()_?/\\'`~!@#$%^&*|-_\"=+\t\r";
   strtk::multiple_char_delimiter_predicate predicate(delimiters);
   parse_line<Container,strtk::multiple_char_delimiter_predicate> pl(c,predicate);
   strtk::for_each_line(file_name,pl);
}

int main()
{
   std::string text_file_name = "text.txt";
   std::deque< std::string > word_list;
   parse_text(text_file_name,word_list);
   std::cout << "Token Count: " << word_list.size() << std::endl;
   return 0;
}
*/

}