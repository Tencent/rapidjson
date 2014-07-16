/*
 *****************************************************************
 *                     String Toolkit Library                    *
 *                                                               *
 * Author: Arash Partow (2002-2014)                              *
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


#ifndef INCLUDE_STRTK_HPP
#define INCLUDE_STRTK_HPP


#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <deque>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


#ifndef strtk_no_tr1_or_boost
   #define strtk_enable_lexical_cast
   #define strtk_enable_random
   #define strtk_enable_regex
#endif

#ifdef strtk_enable_lexical_cast
   #include <boost/lexical_cast.hpp>
#endif

#ifdef strtk_enable_random
   // Requires definition of a TR1 compatible random library header
   //#include <random>
   #include <boost/random.hpp>
#endif

#ifdef strtk_enable_regex
   // Requires definition of a TR1 compatible regex library header
   //#include <regex>
   #include <boost/regex.hpp>
#endif


namespace strtk
{

   static const std::size_t one_kilobyte = 1024;
   static const std::size_t one_megabyte = 1024 * one_kilobyte;
   static const std::size_t one_gigabyte = 1024 * one_megabyte;
   static const std::size_t magic_seed   = 0xA5A5A5A5;

   template <typename Tokenizer, typename Function>
   inline std::size_t for_each_token(const std::string& buffer,
                                     Tokenizer& tokenizer,
                                     Function function)
   {
      std::size_t token_count = 0;
      tokenizer.assign(buffer);
      typename Tokenizer::iterator itr = tokenizer.begin();
      typename Tokenizer::const_iterator end = tokenizer.end();
      while (end != itr)
      {
         function(*itr);
         ++itr;
         ++token_count;
      }
      return token_count;
   }

   template <typename Function>
   inline std::size_t for_each_line(std::istream& stream,
                                    Function function,
                                    const std::size_t& buffer_size = one_kilobyte)
   {
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         function(buffer);
         ++line_count;
      }
      return line_count;
   }

   template <typename Function>
   inline std::size_t for_each_line_n(std::istream& stream,
                                      const std::size_t& n,
                                      Function function,
                                      const std::size_t& buffer_size = one_kilobyte)
   {
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         function(buffer);
         if (n == ++line_count)
            break;
      }
      return line_count;
   }

   template <typename Function>
   inline std::size_t for_each_line(const std::string& file_name,
                                    Function function,
                                    const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (stream)
         return for_each_line(stream,function,buffer_size);
      else
         return 0;
   }

   template <typename Function>
   inline std::size_t for_each_line_n(const std::string& file_name,
                                      const std::size_t& n,
                                      Function function,
                                      const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (stream)
         return for_each_line_n(stream,n,function,buffer_size);
      else
         return 0;
   }

   template <typename Function>
   inline std::size_t for_each_line_conditional(std::istream& stream,
                                                Function function,
                                                const std::size_t& buffer_size = one_kilobyte)
   {
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         if (!function(buffer))
         {
            return line_count;
         }
         ++line_count;
      }
      return line_count;
   }

   template <typename Function>
   inline std::size_t for_each_line_n_conditional(std::istream& stream,
                                                  const std::size_t& n,
                                                  Function function,
                                                  const std::size_t& buffer_size = one_kilobyte)
   {
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         if (!function(buffer))
         {
            return line_count;
         }
         if (n == ++line_count)
            break;
      }
      return line_count;
   }

   template <typename Function>
   inline std::size_t for_each_line_conditional(const std::string& file_name,
                                                Function function,
                                                const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (stream)
         return for_each_line_conditional(stream,function,buffer_size);
      else
         return 0;
   }

   template <typename Function>
   inline std::size_t for_each_line_n_conditional(const std::string& file_name,
                                                  const std::size_t& n,
                                                  Function function,
                                                  const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (stream)
         return for_each_line_n_conditional(stream,n,function,buffer_size);
      else
         return 0;
   }

   template <typename T>
   inline bool read_line_as_value(std::istream& stream,
                                  T& t,
                                  const std::size_t& buffer_size = one_kilobyte)
   {
      std::string buffer;
      buffer.reserve(buffer_size);
      if (!std::getline(stream,buffer))
         return false;
      return string_to_type_converter(buffer,t);
   }

   namespace details
   {
      struct not_supported_type_tag   {};
      struct unsigned_type_tag        {};
      struct signed_type_tag          {};
      struct real_type_tag            {};
      struct byte_type_tag            {};
      struct bool_type_tag            {};
      struct hex_number_type_tag      {};
      struct hex_string_type_tag      {};
      struct base64_type_tag          {};
      struct ignore_token_type_tag    {};
      struct stdstring_type_tag       {};
      struct stdstring_range_type_tag {};
      struct value_type_tag           {};
      struct sink_type_tag            {};
      struct stl_seq_type_tag         {};
      struct attribute_type_tag       {};
      struct semantic_action_type_tag {};
      struct expect_type_tag          {};
      struct like_type_tag            {};
      struct inrange_type_tag         {};
      struct trim_type_tag            {};
      struct lcase_type_tag           {};
      struct ucase_type_tag           {};
      struct fillchararray_type_tag   {};
      struct truncint_type_tag        {};

      template <typename T>
      struct supported_conversion_to_type
      {
         typedef not_supported_type_tag type;
      };

      template <typename T>
      struct supported_conversion_from_type
      {
         typedef not_supported_type_tag type;
      };

      template <bool, typename T = void>
      struct enable_if {};

      template <typename T>
      struct enable_if<true, T> { typedef T type; };

      template <typename T>
      struct supported_iterator_type
      {
         enum { value = false };
      };

      template <typename T>
      struct is_valid_iterator
      {
         typedef typename details::enable_if<details::supported_iterator_type<T>::value,T>::type type;
      };

      template <typename T> struct numeric;
      template <typename T> inline std::size_t type_length(const T&);

      struct no_t  {};
      struct yes_t {};

      template <typename T>
      struct is_pod
      {
         typedef no_t result_t;
         enum { result = false };
      };

      template <typename T>
      struct is_stl_container
      { typedef no_t result_t; };

      #define register_stl_container1(C) \
      template <typename T1, typename T2>struct is_stl_container<C<T1,T2> >{ typedef yes_t result_t; };

      #define register_stl_container2(C) \
      template <typename T1, typename T2, typename T3>struct is_stl_container<C<T1,T2,T3> >{ typedef yes_t result_t; };

      register_stl_container1(std::vector)
      register_stl_container1(std::deque)
      register_stl_container1(std::list)
      register_stl_container1(std::queue)
      register_stl_container1(std::stack)
      register_stl_container2(std::set)
      register_stl_container2(std::multiset)
      register_stl_container2(std::priority_queue)

      #undef register_stl_container1
      #undef register_stl_container2

      template <typename T>
      void convert_type_assert(){}

   } // namespace details

   template <typename Iterator, typename T>
   inline bool string_to_type_converter(const Iterator begin, const Iterator end, T& t)
   {
      typedef typename details::is_valid_iterator<Iterator>::type itr_type;
      typename details::supported_conversion_to_type<T>::type type;
      details::convert_type_assert<itr_type>();
      Iterator itr = begin;
      return string_to_type_converter_impl(itr,end,t,type);
   }

   template <typename Iterator, typename T>
   inline bool string_to_type_converter(const std::pair<Iterator,Iterator>& range, T& t)
   {
      return string_to_type_converter(range.first,range.second,t);
   }

   template <typename T, typename Iterator>
   inline T string_to_type_converter(const Iterator begin, const Iterator end)
   {
      typedef typename details::is_valid_iterator<Iterator>::type itr_type;
      typename details::supported_conversion_to_type<T>::type type;
      details::convert_type_assert<itr_type>();
      T t;
      Iterator itr = begin;
      if (string_to_type_converter_impl(itr,end,t,type))
         return t;
      else
         throw std::invalid_argument("string_to_type_converter() - Failed to convert: " +
                                     std::string(begin,end));
   }

   template <typename T, typename Iterator>
   inline T string_to_type_converter(const std::pair<Iterator,Iterator>& range)
   {
      return string_to_type_converter<T>(range.first,range.second);
   }

   template <typename T>
   inline bool string_to_type_converter(const std::string& s, T& t)
   {
      return string_to_type_converter<const char*,T>(s.data(),s.data() + s.size(),t);
   }

   template <typename T>
   inline T string_to_type_converter(const std::string& s)
   {
      return string_to_type_converter<T>(s.data(),s.data() + s.size());
   }

   template <typename T>
   inline bool type_to_string(const T& t, std::string& s)
   {
      typename details::supported_conversion_from_type<T>::type type;
      return type_to_string_converter_impl(t,s,type);
   }

   template <typename T>
   inline std::string type_to_string(const T& t)
   {
      std::string s;
      if (type_to_string<T>(t,s))
         return s;
      else
         throw std::invalid_argument("type_to_string() - Failed to convert type to string");
   }

   #define strtk_begin_register_string_to_type \
   namespace strtk { namespace details {       \

   #define strtk_end_register_string_to_type \
   }}                                        \

   #define strtk_string_to_type_begin(Type)                                               \
   namespace strtk { namespace details { template <typename Iterator>                     \
   inline bool string_to_type_converter_impl(const Iterator& begin, const Iterator& end,  \
                                             Type& t, details::not_supported_type_tag&) { \

   #define strtk_string_to_type_end() \
   }}}                                \

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t load_from_text_file(std::istream& stream,
                                          Sequence<T,Allocator>& sequence,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      if (!stream) return 0;
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         ++line_count;
         sequence.push_back(string_to_type_converter<T>(buffer));
      }
      return line_count;
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t load_from_text_file(std::istream& stream,
                                          std::set<T,Comparator,Allocator>& set,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      if (!stream) return 0;
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         ++line_count;
         set.insert(string_to_type_converter<T>(buffer));
      }
      return line_count;
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t load_from_text_file(std::istream& stream,
                                          std::multiset<T,Comparator,Allocator>& multiset,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      if (!stream) return 0;
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         ++line_count;
         multiset.insert(string_to_type_converter<T>(buffer));
      }
      return line_count;
   }

   template <typename T, typename Container>
   inline std::size_t load_from_text_file(std::istream& stream,
                                          std::queue<T,Container>& queue,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      if (!stream) return 0;
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         ++line_count;
         queue.push(string_to_type_converter<T>(buffer));
      }
      return line_count;
   }

   template <typename T, typename Container>
   inline std::size_t load_from_text_file(std::istream& stream,
                                          std::stack<T,Container>& stack,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      if (!stream) return 0;
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         ++line_count;
         stack.push(string_to_type_converter<T>(buffer));
      }
      return line_count;
   }

   template <typename T,
             typename Container,
             typename Comparator>
   inline std::size_t load_from_text_file(std::istream& stream,
                                          std::priority_queue<T,Container,Comparator>& priority_queue,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      if (!stream) return 0;
      std::string buffer;
      buffer.reserve(buffer_size);
      std::size_t line_count = 0;
      while (std::getline(stream,buffer))
      {
         ++line_count;
         priority_queue.push(string_to_type_converter<T>(buffer));
      }
      return line_count;
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t load_from_text_file(const std::string& file_name,
                                          Sequence<T,Allocator>& sequence,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return load_from_text_file(stream,sequence,buffer_size);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t load_from_text_file(const std::string& file_name,
                                          std::set<T,Comparator,Allocator>& set,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return load_from_text_file(stream,set,buffer_size);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t load_from_text_file(const std::string& file_name,
                                          std::multiset<T,Comparator,Allocator>& multiset,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return load_from_text_file(stream,multiset,buffer_size);
   }

   template <typename T, typename Container>
   inline std::size_t load_from_text_file(const std::string& file_name,
                                          std::queue<T,Container>& queue,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return load_from_text_file(stream,queue,buffer_size);
   }

   template <typename T, typename Container>
   inline std::size_t load_from_text_file(const std::string& file_name,
                                          std::stack<T,Container>& stack,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return load_from_text_file(stream,stack,buffer_size);
   }

   template <typename T,
             typename Container,
             typename Comparator>
   inline std::size_t load_from_text_file(const std::string& file_name,
                                          std::priority_queue<T,Container,Comparator>& priority_queue,
                                          const std::size_t& buffer_size = one_kilobyte)
   {
      std::ifstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return load_from_text_file(stream,priority_queue,buffer_size);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t write_to_text_file(std::ostream& stream,
                                         const Sequence<T,Allocator>& sequence,
                                         const std::string& delimiter = "")
   {
      if (!stream) return 0;

      std::size_t count = 0;
      typename Sequence<T,Allocator>::const_iterator itr = sequence.begin();
      typename Sequence<T,Allocator>::const_iterator end = sequence.end();

      if (!delimiter.empty())
      {
         while (end != itr)
         {
            stream << (*itr) << delimiter;
            ++itr;
            ++count;
         }
      }
      else
      {
         while (end != itr)
         {
            stream << (*itr);
            ++itr;
            ++count;
         }
      }
      return count;
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t write_to_text_file(std::ostream& stream,
                                         const std::set<T,Comparator,Allocator>& set,
                                         const std::string& delimiter = "")
   {
      if (!stream) return 0;

      std::size_t count = 0;
      typename std::set<T,Comparator,Allocator>::const_iterator itr = set.begin();
      typename std::set<T,Comparator,Allocator>::const_iterator end = set.end();

      if (!delimiter.empty())
      {
         while (end != itr)
         {
            stream << (*itr) << delimiter;
            ++itr;
            ++count;
         }
      }
      else
      {
         while (end != itr)
         {
            stream << (*itr);
            ++itr;
            ++count;
         }
      }
      return count;
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t write_to_text_file(std::ostream& stream,
                                         const std::multiset<T,Comparator,Allocator>& multiset,
                                         const std::string& delimiter = "")
   {
      if (!stream) return 0;

      std::size_t count = 0;
      typename std::multiset<T,Comparator,Allocator>::const_iterator itr = multiset.begin();
      typename std::multiset<T,Comparator,Allocator>::const_iterator end = multiset.end();

      if (!delimiter.empty())
      {
         while (end != itr)
         {
            stream << (*itr) << delimiter;
            ++itr;
            ++count;
         }
      }
      else
      {
         while (end != itr)
         {
            stream << (*itr);
            ++itr;
            ++count;
         }
      }
      return count;
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t write_to_text_file(const std::string& file_name,
                                         const Sequence<T,Allocator>& sequence,
                                         const std::string& delimiter = "")
   {
      std::ofstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return write_to_text_file(stream,sequence,delimiter);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t write_to_text_file(const std::string& file_name,
                                         const std::set<T,Comparator,Allocator>& set,
                                         const std::string& delimiter = "")
   {
      std::ofstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return write_to_text_file(stream,set,delimiter);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t write_to_text_file(const std::string& file_name,
                                         const std::multiset<T,Comparator,Allocator>& multiset,
                                         const std::string& delimiter = "")
   {
      std::ofstream stream(file_name.c_str());
      if (!stream)
         return 0;
      else
         return write_to_text_file(stream,multiset,delimiter);
   }

   template <typename InputIterator, typename OutputIterator>
   inline void copy_n(InputIterator itr, std::size_t n, OutputIterator out)
   {
      while (n)
      {
         (*out) = (*itr);
         ++itr;
         ++out;
         --n;
      }
   }

   template <typename Predicate,
             typename InputIterator,
             typename OutputIterator>
   inline void copy_if(Predicate predicate,
                       const InputIterator begin, const InputIterator end,
                       OutputIterator out)
   {
      InputIterator itr = begin;
      while (end != itr)
      {
         if (predicate(*itr))
         {
            *(out++) = (*itr);
         }
         ++itr;
      }
   }

   template <typename Predicate,
             typename InputIterator,
             typename OutputIterator>
   inline InputIterator copy_while(Predicate predicate,
                                   const InputIterator begin, const InputIterator end,
                                   OutputIterator out)
   {
      InputIterator itr = begin;
      while (end != itr)
      {
         if (!predicate(*itr))
            return itr;
         *(out++) = *(itr++);
      }
      return end;
   }

   template <typename Predicate,
             typename InputIterator,
             typename OutputIterator>
   inline InputIterator copy_until(Predicate predicate,
                                   const InputIterator begin, const InputIterator end,
                                   OutputIterator out)
   {
      InputIterator itr = begin;
      while (end != itr)
      {
         if (predicate(*itr))
            return itr;
         *(out++) = *(itr++);
      }
      return end;
   }

   template <typename InputIterator, typename OutputIterator>
   inline void extract_unique(const InputIterator begin, const InputIterator end,
                              OutputIterator out)
   {
      typedef typename std::iterator_traits<InputIterator>::value_type T;
      std::vector<T> buffer(begin,end);
      std::sort(buffer.begin(),buffer.end());
      std::unique_copy(buffer.begin(),buffer.end(),out);
   }

   template <typename Predicate, typename InputIterator>
   inline bool range_only_contains(Predicate predicate,
                                   const InputIterator begin,
                                   const InputIterator end)
   {
      InputIterator itr = begin;
      while (end != itr)
      {
         if (!predicate(*itr))
         {
            return false;
         }
         ++itr;
      }
      return true;
   }

   namespace range
   {
      template <typename T>
      class adapter
      {
      public:

         typedef T value_type;
         typedef T* iterator;
         typedef const iterator const_iterator;

         adapter(T* const begin, T* const end)
         : begin_(begin),
           end_(end)
         {}

         adapter(const std::pair<T*,T*>& r)
         : begin_(r.first),
           end_(r.second)
         {}

         adapter(T* const begin, const std::size_t length)
         : begin_(begin),
           end_(begin_ + length)
         {}

         inline iterator begin() const
         {
            return begin_;
         }

         inline iterator end() const
         {
            return end_;
         }

         inline std::size_t size() const
         {
            return std::distance(begin_,end_);
         }

         inline operator std::string() const
         {
            return stringify(begin_,end_);
         }

         inline const T& operator[](const std::size_t& index) const
         {
            return *(begin_ + index);
         }

         inline T& operator[](const std::size_t& index)
         {
            return *(begin_ + index);
         }

      private:

         template <typename Type>
         static inline std::string stringify(Type*,Type*)
         {
            static std::string result = "";
            return result;
         }

         static inline std::string stringify(const char* begin, const char* end)
         {
            return std::string(begin,end);
         }

         iterator begin_;
         iterator end_;
      };

      typedef adapter<const char> string;
      typedef adapter<const unsigned char> ustring;

      template <typename T>
      inline adapter<T> type(const T* begin, const T* end)
      {
         return adapter<T>(begin,end);
      }

      template <typename T, std::size_t N>
      inline adapter<T> type(const T (&t)[N])
      {
         return adapter<T>(t,N);
      }

      static inline adapter<const char> type(const std::string& s)
      {
         return adapter<const char>(s.data(),s.size());
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline adapter<typename Sequence<T,Allocator>::iterator> type(const Sequence<T,Allocator>& seq)
      {
         return adapter<typename Sequence<T,Allocator>::iterator>(seq.begin(),seq.end());
      }

      inline std::string as_string(const adapter<const char>& a)
      {
         return std::string(a.begin(),a.end());
      }

      inline std::string as_string(const adapter<const unsigned char>& a)
      {
         return std::string(a.begin(),a.end());
      }

   } // namespace range

   template <typename T>
   struct single_delimiter_predicate
   {
   public:

      typedef T value_type;

      single_delimiter_predicate(const T& d)
      : delimiter_(d)
      {}

      inline bool operator()(const T& d) const
      {
         return delimiter_ == d;
      }

   private:

      single_delimiter_predicate<T>& operator=(const single_delimiter_predicate<T>&);
      const T delimiter_;
   };

   template <typename T>
   struct multiple_delimiter_predicate
   {
   public:

      typedef T value_type;

      multiple_delimiter_predicate(const T* d_begin, const T* d_end)
      : length_(std::distance(d_begin,d_end)),
        delimiter_((length_ <= sbo_buffer_size) ? sbo_buffer : new T[length_]),
        delimiter_end_(delimiter_ + length_)
      {
         std::copy(d_begin,d_end, delimiter_);
      }

      multiple_delimiter_predicate(const T d[], const std::size_t& length)
      : length_(length),
        delimiter_((length_ <= sbo_buffer_size) ? sbo_buffer : new T[length_]),
        delimiter_end_(delimiter_ + length_)
      {
         std::copy(d,d + length, delimiter_);
      }

      template <typename Iterator>
      multiple_delimiter_predicate(const Iterator begin, const Iterator end)
      : length_(std::distance(begin,end)),
        delimiter_((length_ <= sbo_buffer_size) ? sbo_buffer : new T[length_]),
        delimiter_end_(delimiter_ + length_)
      {
         //static_assert(T == std::iterator_traits<Iterator>::value_type);
         std::copy(begin,end, delimiter_);
      }

      template <typename Type>
      multiple_delimiter_predicate(const range::adapter<Type>& r)
      : length_(std::distance(r.begin(),r.end())),
        delimiter_((length_ <= sbo_buffer_size) ? sbo_buffer : new T[length_]),
        delimiter_end_(delimiter_ + length_)
      {
         //static_assert(T == std::iterator_traits<Iterator>::value_type);
         std::copy(r.begin(),r.end(), delimiter_);
      }

     ~multiple_delimiter_predicate()
      {
         if (length_ > sbo_buffer_size)
         {
            delete[] delimiter_;
         }
      }

      inline bool operator()(const T& d) const
      {
         return (std::find(delimiter_,delimiter_end_,d) != delimiter_end_);
      }

   private:

      multiple_delimiter_predicate(const multiple_delimiter_predicate<T>& mdp);
      multiple_delimiter_predicate& operator=(const multiple_delimiter_predicate<T>& mdp);

      std::size_t length_;
      T* delimiter_;
      T* delimiter_end_;
      enum { sbo_buffer_size = 32 };
      T sbo_buffer[sbo_buffer_size];
   };

   struct multiple_char_delimiter_predicate
   {
   public:

      template <typename Iterator>
      multiple_char_delimiter_predicate(const Iterator begin, const Iterator end)
      {
         setup_delimiter_table(begin,end);
      }

      multiple_char_delimiter_predicate(const std::string& s)
      {
         setup_delimiter_table(s.data(),s.data() + s.size());
      }

      inline bool operator()(const unsigned char& c) const
      {
         return (delimiter_table_[c]);
      }

      inline bool operator()(const char& c) const
      {
         return operator()(static_cast<unsigned char>(c));
      }

   private:

      static const std::size_t table_size = 256;

      template <typename Iterator>
      inline void setup_delimiter_table(const Iterator begin, const Iterator end)
      {
         std::fill_n(delimiter_table_,table_size,false);
         for (Iterator itr = begin; itr != end; ++itr)
         {
            delimiter_table_[static_cast<unsigned char>(*itr)] = true;
         }
      }

      bool delimiter_table_[table_size];
   };

   namespace details
   {
      template <typename Allocator,
                template <typename,typename> class Sequence>
      struct index_remover_impl
      {
         typedef Sequence<std::size_t,Allocator> sequence_t;
         index_remover_impl(const sequence_t& sequence)
         : itr_(sequence.begin()),
           end_(sequence.end()),
           current_index_(0),
           check_(true)
         {}

         template <typename T>
         inline bool operator()(const T&)
         {
            if (check_)
            {
               if (current_index_++ == *itr_)
               {
                  if (end_ == ++itr_)
                  {
                     check_ = false;
                  }
                  return true;
               }
            }
            return false;
         }

         typename sequence_t::const_iterator itr_;
         typename sequence_t::const_iterator end_;
         std::size_t current_index_;
         bool check_;
      };
   }

   template <typename Allocator,
             template <typename,typename> class Sequence>
   inline details::index_remover_impl<Allocator,Sequence> index_remover(const Sequence<std::size_t,Allocator>& sequence)
   {
      return details::index_remover_impl<Allocator,Sequence>(sequence);
   }

   template <typename Iterator, typename Predicate>
   inline std::size_t remove_inplace(Predicate predicate,
                                     Iterator begin,
                                     Iterator end)
   {
      Iterator itr1 = begin;
      Iterator itr2 = begin;
      std::size_t removal_count = 0;
      while (end != itr1)
      {
         if (predicate(*itr1))
         {
            ++itr1;
            ++removal_count;
         }
         else
         {
            if (itr1 != itr2)
            {
               (*itr2) = (*itr1);
            }
            ++itr1;
            ++itr2;
         }
      }
      return removal_count;
   }

   template <typename T, typename Predicate>
   inline std::size_t remove_inplace(Predicate predicate, const range::adapter<T>& r)
   {
      return remove_inplace(predicate,r.begin(),r.end());
   }

   template <typename Predicate,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t remove_inplace(Predicate predicate, Sequence<T,Allocator>& sequence)
   {
      const std::size_t removal_count = remove_inplace(predicate,sequence.begin(),sequence.end());
      sequence.resize(sequence.size() - removal_count);
      return removal_count;
   }

   inline void remove_inplace(const std::string::value_type c, std::string& s)
   {
      const std::size_t removal_count = remove_inplace(single_delimiter_predicate<std::string::value_type>(c),
                                                       const_cast<char*>(s.data()),
                                                       const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Predicate>
   inline void remove_inplace(Predicate predicate, std::string& s)
   {
      const std::size_t removal_count = remove_inplace(predicate,
                                                       const_cast<char*>(s.data()),
                                                       const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Iterator, typename Predicate>
   inline std::size_t remove_consecutives_inplace(Predicate predicate,
                                                  Iterator begin,
                                                  Iterator end)
   {
      if (0 == std::distance(begin,end)) return 0;
      Iterator itr1 = begin;
      Iterator itr2 = begin;
      typename std::iterator_traits<Iterator>::value_type prev = *begin;
      std::size_t removal_count = 0;
      ++itr1;
      ++itr2;
      while (end != itr1)
      {
         while ((end != itr1) && (!predicate(*itr1) || !predicate(prev)))
         {
            if (itr1 != itr2)
            {
               (*itr2) = (*itr1);
            }
            prev = (*itr1);
            ++itr1;
            ++itr2;
         }

         while ((end != itr1) && predicate(*itr1))
         {
            ++itr1;
            ++removal_count;
         }
      }

      return removal_count;
   }

   template <typename T, typename Predicate>
   inline std::size_t remove_consecutives_inplace(Predicate predicate, const range::adapter<T>& r)
   {
      return remove_consecutives_inplace(predicate,r.begin(),r.end());
   }

   inline void remove_consecutives_inplace(const std::string::value_type c, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_consecutives_inplace(single_delimiter_predicate<std::string::value_type>(c),
                                                                    const_cast<char*>(s.data()),
                                                                    const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline void remove_consecutives_inplace(const std::string& rem_chars, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_consecutives_inplace(multiple_char_delimiter_predicate(rem_chars),
                                                                    const_cast<char*>(s.data()),
                                                                    const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   namespace details
   {
      #if (defined(__MINGW32_VERSION)) ||                                                   \
          (defined(__CYGWIN__) || defined(__CYGWIN32__)) ||                                 \
          (defined(__APPLE__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)) || \
          (defined(_WIN32) && (_MSC_VER < 1400))
         inline std::size_t strnlength(const char* s, const std::size_t& n)
         {
            const char *end = reinterpret_cast<const char*>(memchr(s, '\0', n));
            return end ? (size_t) (end - s) : n;
         }
      #else
         inline std::size_t strnlength(const char* s, const std::size_t& n)
         {
            return strnlen(s,n);
         }
      #endif
   }

   inline void remove_consecutives_inplace(const char* rem_chars, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_consecutives_inplace(multiple_char_delimiter_predicate(
                                                                    rem_chars,
                                                                    rem_chars + details::strnlength(rem_chars,256)),
                                                                    const_cast<char*>(s.data()),
                                                                    const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Predicate>
   inline void remove_consecutives_inplace(Predicate predicate, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_consecutives_inplace(predicate,
                                                                    const_cast<char*>(s.data()),
                                                                    const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Iterator>
   inline std::size_t remove_consecutives_inplace(Iterator begin, Iterator end)
   {
      if (0 == std::distance(begin,end)) return 0;

      Iterator itr1 = begin; ++itr1;
      Iterator itr2 = begin; ++itr2;
      typename std::iterator_traits<Iterator>::value_type prev = *begin;
      std::size_t removal_count = 0;

      while (end != itr1)
      {
         while ((end != itr1) && (prev != (*itr1)))
         {
            if (itr1 != itr2)
            {
               (*itr2) = (*itr1);
            }
            prev = (*itr1);
            ++itr1;
            ++itr2;
         }

         while ((end != itr1) && (prev == (*itr1)))
         {
            ++itr1;
            ++removal_count;
         }
      }

      return removal_count;
   }

   template <typename T>
   inline std::size_t remove_consecutives_inplace(const range::adapter<T>& r)
   {
      return remove_consecutives_inplace(r.begin(),r.end());
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void remove_consecutives_inplace(Sequence<T,Allocator>& sequence)
   {
      const std::size_t removal_count = remove_consecutives_inplace(sequence.begin(),sequence.end());
      sequence.resize(sequence.size() - removal_count);
   }

   inline void remove_consecutives_inplace(std::string& s)
   {
      std::size_t removal_count = remove_consecutives_inplace(const_cast<char*>(s.data()),
                                                              const_cast<char*>(s.data() + s.size()));

      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline std::string remove_duplicates(const std::string& str)
   {
      std::string::value_type table[0xFF];
      std::fill_n(table,0xFF,static_cast<char>(0));
      std::string result;
      result.reserve(str.size());
      for (std::size_t i = 0; i < str.size(); ++i)
      {
         const char c = str[i];
         if (0 == table[static_cast<std::size_t>(c)])
         {
            table[static_cast<std::size_t>(c)] = 0x01;
            result += c;
         }
      }
      return result;
   }

   inline std::string remove_duplicates_inplace(std::string& str)
   {
      return remove_duplicates(str);
   }

   template <typename Iterator, typename Predicate>
   inline std::size_t remove_trailing(Predicate predicate,
                                      Iterator begin,
                                      Iterator end)
   {
      const std::size_t length = std::distance(begin,end);
      if (0 == length)
         return 0;
      Iterator itr = begin + (length - 1);
      std::size_t removal_count = 0;

      while ((begin != itr) && predicate(*itr))
      {
         --itr;
         ++removal_count;
      }

      return removal_count;
   }

   template <typename T, typename Predicate>
   inline std::size_t remove_trailing(Predicate predicate, const range::adapter<T>& r)
   {
      return remove_trailing(predicate,r.begin(),r.end());
   }

   inline void remove_trailing(const std::string::value_type c, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_trailing(single_delimiter_predicate<std::string::value_type>(c),
                                                        const_cast<char*>(s.data()),
                                                        const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline void remove_trailing(const std::string& rem_chars, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_trailing(multiple_char_delimiter_predicate(rem_chars),
                                                        const_cast<char*>(s.data()),
                                                        const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline void remove_trailing(const char* rem_chars, std::string& s)
   {
      const std::size_t removal_count = remove_trailing(multiple_char_delimiter_predicate(
                                                        rem_chars,
                                                        rem_chars + details::strnlength(rem_chars,256)),
                                                        const_cast<char*>(s.data()),
                                                        const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Predicate>
   inline void remove_trailing(Predicate predicate, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_trailing(predicate,
                                                        const_cast<char*>(s.data()),
                                                        const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Iterator, typename Predicate>
   inline std::size_t remove_leading(Predicate predicate,
                                     Iterator begin,
                                     Iterator end)
   {
      const std::size_t length = std::distance(begin,end);
      if (0 == length)
         return 0;
      Iterator itr = begin;
      std::size_t removal_count = 0;

      while ((end != itr) && predicate(*itr))
      {
         ++itr;
         ++removal_count;
      }

      std::copy(itr,end,begin);
      return removal_count;
   }

   template <typename T, typename Predicate>
   inline std::size_t remove_leading(Predicate predicate, const range::adapter<T>& r)
   {
      return remove_leading(predicate,r.begin(),r.end());
   }

   inline void remove_leading(const std::string::value_type c, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_leading(single_delimiter_predicate<std::string::value_type>(c),
                                                       const_cast<char*>(s.data()),
                                                       const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline void remove_leading(const std::string& rem_chars, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_leading(multiple_char_delimiter_predicate(rem_chars),
                                                       const_cast<char*>(s.data()),
                                                       const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline void remove_leading(const char* rem_chars, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_leading(multiple_char_delimiter_predicate(
                                                       rem_chars,
                                                       rem_chars + details::strnlength(rem_chars,256)),
                                                       const_cast<char*>(s.data()),
                                                       const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   inline void remove_leading_trailing(const std::string& rem_chars, std::string& s)
   {
      remove_leading(rem_chars,s);
      remove_trailing(rem_chars,s);
   }

   template <typename Predicate>
   inline void remove_leading(Predicate predicate, std::string& s)
   {
      if (s.empty()) return;
      const std::size_t removal_count = remove_leading(predicate,
                                                       const_cast<char*>(s.data()),
                                                       const_cast<char*>(s.data() + s.size()));
      if (removal_count > 0)
      {
         s.resize(s.size() - removal_count);
      }
   }

   template <typename Allocator,
             template <typename,typename> class Sequence>
   void remove_empty_strings(Sequence<std::string,Allocator>& seq)
   {
      struct is_empty { static inline bool check(const std::string& s) { return s.empty(); } };
      seq.erase(std::remove_if(seq.begin(),seq.end(),is_empty::check),seq.end());
   }

   template <typename Allocator>
   void remove_empty_strings(std::list<std::string,Allocator>& l)
   {
      struct is_empty { static inline bool check(const std::string& s) { return s.empty(); } };
      l.remove_if(is_empty::check);
   }

   template <typename Comparator, typename Allocator>
   void remove_empty_strings(std::set<std::string,Comparator,Allocator>& set)
   {
      struct is_empty { static inline bool check(const std::string& s) { return s.empty(); } };
      typename std::set<std::string,Comparator,Allocator>::iterator itr = set.begin();
      while (set.end() != itr)
      {
         if ((*itr).empty())
            set.erase(itr++);
         else
            ++itr;
      }
   }

   template <typename Comparator, typename Allocator>
   void remove_empty_strings(std::multiset<std::string,Comparator,Allocator>& set)
   {
      struct is_empty { static inline bool check(const std::string& s) { return s.empty(); } };
      typename std::multiset<std::string,Comparator,Allocator>::iterator itr = set.begin();
      while (set.end() != itr)
      {
         if ((*itr).empty())
            set.erase(itr++);
         else
            ++itr;
      }
   }

   template <typename Iterator>
   inline void replace(const typename std::iterator_traits<Iterator>::value_type& c1,
                       const typename std::iterator_traits<Iterator>::value_type& c2,
                       const Iterator begin,
                       const Iterator end)
   {
      for (Iterator itr = begin; end != itr; ++itr)
      {
         if (c1 == (*itr))
         {
            (*itr) = c2;
         }
      }
   }

   inline void replace(const std::string::value_type& c0,
                       const std::string::value_type& c1,
                       std::string& s)
   {
      replace(c0,c1,const_cast<char*>(s.data()),const_cast<char*>(s.data() + s.size()));
   }

   template <typename T>
   inline void replace(const T& c1, const T& c2, const range::adapter<T>& r)
   {
      replace(c1,c2,r.begin(),r.end());
   }

   inline void replace_pattern(const std::string& s, // input
                               const std::string& p, // pattern
                               const std::string& r, // replacement
                               std::string& n)
   {
      if (p.empty() || (p == r))
      {
         n.assign(s);
         return;
      }

      const std::size_t p_size = p.size();
      const std::size_t r_size = r.size();
      int inc = static_cast<int>(r_size) - static_cast<int>(p_size);
      std::size_t pos = 0;
      std::vector<std::size_t> delta_list;
      delta_list.reserve(std::min<std::size_t>(32,(s.size() / p_size) + 1));

      while (std::string::npos != (pos = s.find(p,pos)))
      {
         delta_list.push_back(pos);
         pos += p_size;
      }

      if (delta_list.empty())
      {
         n.assign(s);
         return;
      }

      n.resize(delta_list.size() * inc + s.size(), 0x00);
      char* n_itr = const_cast<char*>(n.data());
      const char* s_end = s.data() + s.size();
      const char* s_itr = s.data();
      const char* r_begin = r.data();
      const char* r_end = r.data() + r_size;
      const std::size_t delta_list_size = delta_list.size();
      std::size_t i = 0;
      std::size_t delta = delta_list[0];

      for ( ; ; )
      {
         std::copy(s_itr, s_itr + delta, n_itr);
         s_itr += p_size + delta;
         n_itr += delta;
         std::copy(r_begin, r_end, n_itr);
         n_itr += r_size;
         if (++i >= delta_list_size)
            break;
         delta =  delta_list[i] - (delta_list[i - 1] + p_size);
      }

      if (s_end != s_itr)
      {
         std::copy(s_itr, s_end, n_itr);
      }
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t replace_pattern(const InputIterator s_begin, const InputIterator s_end, // input
                                      const InputIterator p_begin, const InputIterator p_end, // pattern
                                      const InputIterator r_begin, const InputIterator r_end, // replacement
                                      OutputIterator out)
   {
      InputIterator s_itr  = s_begin;
      InputIterator r_itr  = r_begin;
      InputIterator p_itr  = p_begin;

      const std::size_t p_size = std::distance(p_begin,p_end);
      const std::size_t r_size = std::distance(r_begin,r_end);

      if ((0 == p_size) || ((p_size == r_size) && std::equal(p_begin,p_end,r_begin)))
      {
         std::copy(s_begin,s_end,out);
         return std::distance(s_begin,s_end);
      }

      std::size_t pos = 0;
      std::size_t prev_pos = 0;
      std::size_t count = 0;
      std::size_t new_size = std::distance(s_begin,s_end);
      int inc = r_size - p_size;

      InputIterator temp_s_itr = s_itr;

      while (s_end != s_itr)
      {
         /*
            Need to replace the following search code with
            Knuth-Pratt-Morris or Boyer-Moore string search
            algorithms.
         */
         bool found = true;
         p_itr = p_begin;
         temp_s_itr = s_itr;

         while ((p_end != p_itr) && (s_end != temp_s_itr))
         {
            if (*(temp_s_itr++) != *(p_itr++))
            {
               found = false;
               break;
            }
         }

         if (found && (p_itr == p_end))
         {
            ++count;
            new_size += inc;
            s_itr = temp_s_itr;
         }
         else
            ++s_itr;
      }

      s_itr = s_begin;
      p_itr = p_begin;

      pos = 0;
      prev_pos = 0;

      temp_s_itr = s_itr;

      while (0 < count)
      {
         p_itr = p_begin;
         bool found = true;
         InputIterator pattern_start = temp_s_itr;

         while ((p_end != p_itr) && (s_end != temp_s_itr))
         {
            if (*(temp_s_itr++) != *(p_itr++))
            {
               found = false;
               temp_s_itr = pattern_start;
               ++temp_s_itr;
               break;
            }
         }

         if (!found || (p_itr != p_end)) continue;

         pos = std::distance(s_begin,temp_s_itr) - p_size;
         int diff = pos - prev_pos;

         std::copy(s_itr,s_itr + diff, out);
         s_itr = temp_s_itr;
         std::copy(r_itr,r_end, out);

         pos += p_size;
         prev_pos = pos;
         --count;
      }

      std::copy(s_itr,s_end,out);

      return new_size;
   }

   inline void remove_pattern(const std::string& s,
                              const std::string& p,
                              std::string& n)
   {
      static const std::string r("");
      replace_pattern(s,p,r,n);
   }

   inline void sort(std::string& s)
   {
      std::sort(s.begin(),s.end());
   }

   template <typename Iterator>
   inline bool match(const Iterator pattern_begin,
                     const Iterator pattern_end,
                     const Iterator data_begin,
                     const Iterator data_end,
                     const typename std::iterator_traits<Iterator>::value_type& zero_or_more,
                     const typename std::iterator_traits<Iterator>::value_type& zero_or_one)
   {
      /*
         Credits: Adapted from code by Jack Handy (2001)
      */
      if (0 == std::distance(data_begin,data_end)) return false;

      Iterator d_itr = data_begin;
      Iterator p_itr = pattern_begin;
      Iterator c_itr = data_begin;
      Iterator m_itr = data_begin;

      while ((data_end != d_itr) && (zero_or_more != (*p_itr)))
      {
         if (((*p_itr) != (*d_itr)) && (zero_or_one != (*p_itr)))
         {
            return false;
         }
         ++p_itr;
         ++d_itr;
      }

      while (data_end != d_itr)
      {
         if (zero_or_more == (*p_itr))
         {
            if (pattern_end == (++p_itr))
            {
               return true;
            }
            m_itr = p_itr;
            c_itr = d_itr;
            ++c_itr;
         }
         else if (((*p_itr) == (*d_itr)) || (zero_or_one == (*p_itr)))
         {
            ++p_itr;
            ++d_itr;
         }
         else
         {
            p_itr = m_itr;
            d_itr = c_itr++;
         }
      }

      while ((p_itr != pattern_end) && (zero_or_more == (*p_itr))) ++p_itr;

      return (p_itr == pattern_end);
   }

   inline bool match(const std::string& wild_card,
                     const std::string& str)
   {
      /*
         * : Zero or more match
         ? : Zero or one match
      */
      return match(wild_card.data(),
                   wild_card.data() + wild_card.size(),
                   str.data(),
                   str.data() + str.size(),
                   '*',
                   '?');
   }

   inline bool imatch_char(const char c1, const char c2)
   {
      return std::toupper(c1) == std::toupper(c2);
   }

   template <typename InputIterator>
   inline bool imatch(const InputIterator begin1, const InputIterator end1,
                      const InputIterator begin2, const InputIterator end2)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (std::distance(begin1,end1) != std::distance(begin2,end2))
      {
         return false;
      }

      InputIterator itr1 = begin1;
      InputIterator itr2 = begin2;

      while (end1 != itr1)
      {
         //if (std::toupper(*itr1, std::locale::classic()) != std::toupper(*it2, std::locale::classic()))
         if (std::toupper(*itr1) != std::toupper(*itr2))
         {
            return false;
         }
         ++itr1;
         ++itr2;
      }

      return true;
   }

   template <typename T>
   inline bool imatch(const range::adapter<T>& r1, const range::adapter<T>& r2)
   {
      return imatch(r1.begin(),r1.end(),r2.begin(),r2.end());
   }

   inline bool imatch(const std::string& s1, const std::string& s2)
   {
      return imatch(s1.data(),
                    s1.data() + s1.size(),
                    s2.data(),
                    s2.data() + s2.size());
   }

   template <typename Iterator>
   inline Iterator imatch(const std::string& s, const Iterator begin, const Iterator end)
   {
      for (const std::string* itr = begin; end != itr; ++itr)
      {
         if (imatch(s,*itr))
         {
            return itr;
         }
      }
      return end;
   }

   template <typename Allocator,
             template <typename,typename> class Sequence>
   inline bool imatch(const std::string& s, const Sequence<std::string,Allocator>& sequence)
   {
      return (sequence.end() != imatch(s,sequence.begin(),sequence.end()));
   }

   template <typename Comparator, typename Allocator>
   inline bool imatch(const std::string& s, const std::set<std::string,Comparator,Allocator>& set)
   {
      return imatch(s,set.begin(),set.end());
   }

   template <typename Comparator, typename Allocator>
   inline bool imatch(const std::string& s, const std::multiset<std::string,Comparator,Allocator>& multiset)
   {
      return imatch(s,multiset.begin(),multiset.end());
   }

   template <typename Iterator, typename OutputIterator>
   inline std::size_t find_all(const Iterator pattern_begin,
                               const Iterator pattern_end,
                               const Iterator begin,
                               const Iterator end,
                               OutputIterator out)
   {
      Iterator itr = begin;
      const std::size_t pattern_length = std::distance(pattern_begin,pattern_end);
      std::size_t match_count = 0;
      while (end != (itr = std::search(itr, end, pattern_begin, pattern_end)))
      {
         (*out) = std::make_pair(itr,itr + pattern_length);
         itr += pattern_length;
         ++out;
         ++match_count;
      }
      return match_count;
   }

   template <typename Iterator,
             typename Range,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t find_all(const Iterator pattern_begin,
                               const Iterator pattern_end,
                               const Iterator begin,
                               const Iterator end,
                               Sequence<Range,Allocator>& seq)
   {
      return find_all(pattern_begin,pattern_end,begin,end,std::back_inserter(seq));
   }

   inline std::size_t ifind(const std::string& pattern, const std::string& data)
   {
      if (pattern.size() > data.size())
         return std::string::npos;
      const char* result_itr = std::search(data.data(),data.data() + data.size(),
                                           pattern.data(), pattern.data() + pattern.size(),
                                           imatch_char);
      if ((data.data() + data.size()) == result_itr)
         return std::string::npos;
      else
         return std::distance(data.data(),result_itr);
   }

   template <typename Iterator, typename OutputIterator>
   inline std::size_t ifind_all(const Iterator pattern_begin,
                                const Iterator pattern_end,
                                const Iterator begin,
                                const Iterator end,
                                OutputIterator out)
   {
      Iterator itr = begin;
      const std::size_t pattern_length = std::distance(pattern_begin,pattern_end);
      std::size_t match_count = 0;

      while (end != (itr = std::search(itr, end, pattern_begin, pattern_end, imatch_char)))
      {
         (*out) = std::make_pair(itr,itr + pattern_length);
         itr += pattern_length;
         ++out;
         ++match_count;
      }

      return match_count;
   }

   template <typename OutputIterator>
   inline std::size_t find_all(const std::string& pattern,
                               const std::string& data,
                               OutputIterator out)
   {
      return find_all(pattern.data(), pattern.data() + pattern.size(),
                      data.data(), data.data() + data.size(),
                      out);
   }

   template <typename Range,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t find_all(const std::string& pattern,
                               const std::string& data,
                               Sequence<Range,Allocator>& seq)
   {
      return find_all(pattern,data,std::back_inserter(seq));
   }

   template <typename OutputIterator>
   inline std::size_t ifind_all(const std::string& pattern,
                                const std::string& data,
                                OutputIterator out)
   {
      return ifind_all(pattern.data(), pattern.data() + pattern.size(),
                       data.data(), data.data() + data.size(),
                       out);
   }

   template <typename Range,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t ifind_all(const std::string& pattern,
                                const std::string& data,
                                Sequence<Range,Allocator>& seq)
   {
      return ifind_all(pattern,data,std::back_inserter(seq));
   }

   template <typename InputIterator>
   inline bool begins_with(const InputIterator pattern_begin,
                           const InputIterator pattern_end,
                           const InputIterator begin,
                           const InputIterator end)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (std::distance(pattern_begin,pattern_end) <= std::distance(begin,end))
      {
         return std::equal(pattern_begin,pattern_end,begin);
      }
      else
         return false;
   }

   inline bool begins_with(const std::string& pattern, const std::string& data)
   {
      if (pattern.size() <= data.size())
      {
         return begins_with(pattern.data(),
                            pattern.data() + pattern.size(),
                            data.data(),
                            data.data() + data.size());
      }
      else
         return false;
   }

   template <typename InputIterator>
   inline bool ibegins_with(const InputIterator pattern_begin,
                            const InputIterator pattern_end,
                            const InputIterator begin,
                            const InputIterator end)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (std::distance(pattern_begin,pattern_end) <= std::distance(begin,end))
      {
         return std::equal(pattern_begin,pattern_end,begin,imatch_char);
      }
      else
         return false;
   }

   inline bool ibegins_with(const std::string& pattern, const std::string& data)
   {
      if (pattern.size() <= data.size())
      {
         return ibegins_with(pattern.data(),
                             pattern.data() + pattern.size(),
                             data.data(),
                             data.data() + data.size());
      }
      else
         return false;
   }

   template <typename InputIterator>
   inline bool ends_with(const InputIterator pattern_begin,
                         const InputIterator pattern_end,
                         const InputIterator begin,
                         const InputIterator end)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      const std::size_t pattern_length = std::distance(pattern_begin,pattern_end);
      const std::size_t data_length = std::distance(begin,end);
      if (pattern_length <= data_length)
      {
         return std::equal(pattern_begin,
                           pattern_end,
                           begin + (data_length - pattern_length));
      }
      else
         return false;
   }

   inline bool ends_with(const std::string& pattern, const std::string& data)
   {
      if (pattern.size() <= data.size())
      {
         return ends_with(pattern.data(),
                          pattern.data() + pattern.size(),
                          data.data(),
                          data.data() + data.size());
      }
      else
         return false;
   }

   template <typename InputIterator>
   inline bool iends_with(const InputIterator pattern_begin,
                          const InputIterator pattern_end,
                          const InputIterator begin,
                          const InputIterator end)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      const std::size_t pattern_length = std::distance(pattern_begin,pattern_end);
      const std::size_t data_length = std::distance(begin,end);
      if (pattern_length <= data_length)
      {
         return std::equal(pattern_begin,
                           pattern_end,
                           begin + (data_length - pattern_length),
                           imatch_char);
      }
      else
         return false;
   }

   inline bool iends_with(const std::string& pattern, const std::string& data)
   {
      if (pattern.size() <= data.size())
      {
         return iends_with(pattern.data(),
                           pattern.data() + pattern.size(),
                           data.data(),
                           data.data() + data.size());
      }
      else
         return false;
   }

   inline std::size_t index_of(const std::string& pattern, const std::string& data)
   {
      if (pattern.empty())
         return std::string::npos;
      else if (data.empty())
         return std::string::npos;
      else if (pattern.size() > data.size())
         return std::string::npos;
      const char* itr = std::search(data.data(),
                                    data.data() + data.size(),
                                    pattern.data(),
                                    pattern.data() + pattern.size());
      return ((data.data() + data.size()) == itr) ? std::string::npos : std::distance(data.data(),itr);
   }

   namespace tokenize_options
   {
      typedef std::size_t type;
      enum
      {
         default_mode            = 0,
         compress_delimiters     = 1,
         include_1st_delimiter   = 2,
         include_all_delimiters  = 4
      };

      static inline bool perform_compress_delimiters(const type& split_opt)
      {
         return compress_delimiters == (split_opt & compress_delimiters);
      }

      static inline bool perform_include_1st_delimiter(const type& split_opt)
      {
         return include_1st_delimiter == (split_opt & include_1st_delimiter);
      }

      static inline bool perform_include_all_delimiters(const type& split_opt)
      {
         return include_all_delimiters == (split_opt & include_all_delimiters);
      }

   } // namespace tokenize_options

   template <typename Iterator, typename DelimiterPredicate>
   class tokenizer
   {
   private:

      template <typename Iterartor,
                typename Predicate,
                typename T = std::pair<Iterator,Iterator> >
      class tokenizer_iterator : public std::iterator<std::forward_iterator_tag,T>
      {
      protected:

         typedef Iterator iterator;
         typedef const iterator const_iterator;
         typedef typename std::pair<iterator,iterator> range_type;

      public:

         explicit inline tokenizer_iterator(const iterator begin,
                                            const iterator end,
                                            const Predicate& predicate,
                                            const tokenize_options::type tokenize_option = tokenize_options::default_mode)
         : predicate_(predicate),
           end_(end),
           range_(begin,begin),
           current_token_(end,end),
           compress_delimiters_(tokenize_options::perform_compress_delimiters(tokenize_option)),
           include_1st_delimiter_(tokenize_options::perform_include_1st_delimiter(tokenize_option)),
           include_all_delimiters_(tokenize_options::perform_include_all_delimiters(tokenize_option)),
           include_delimiters_(include_1st_delimiter_ || include_all_delimiters_),
           last_token_done_(false)
         {
            if (end != begin)
            {
               this->operator++();
            }
         }

         inline tokenizer_iterator& operator++()
         {
            if (last_token_done_)
            {
               range_.first = range_.second;
               return (*this);
            }
            else if (end_ != range_.second)
            {
               range_.first = range_.second;
            }

            while (end_ != range_.second)
            {
              if (predicate_(*(range_.second)))
              {
                 if (include_delimiters_)
                 {
                    if (include_1st_delimiter_)
                       ++range_.second;
                    else if (include_all_delimiters_)
                       while ((end_ != range_.second) && predicate_(*(range_.second))) ++range_.second;
                    current_token_ = range_;
                    if ((!include_all_delimiters_) && compress_delimiters_)
                       while ((end_ != range_.second) && predicate_(*(range_.second))) ++range_.second;
                 }
                 else
                 {
                    current_token_ = range_;
                    if (compress_delimiters_)
                       while ((end_ != (++range_.second)) && predicate_(*(range_.second))) ;
                    else
                       ++range_.second;
                 }
                 return (*this);
              }
              else
                 ++range_.second;
            }

            if (range_.first != range_.second)
            {
               current_token_.second = range_.second;
               if (!last_token_done_)
               {
                  if (predicate_(*(range_.second - 1)))
                     current_token_.first = range_.second;
                  else
                     current_token_.first = range_.first;
                  last_token_done_ = true;
               }
               else
                  range_.first = range_.second;
            }

            return (*this);
         }

         inline tokenizer_iterator operator++(int)
         {
            tokenizer_iterator tmp = (*this);
            this->operator++();
            return tmp;
         }

         inline tokenizer_iterator& operator+=(const int inc)
         {
            if (inc > 0)
            {
               for (int i = 0; i < inc; ++i, ++(*this)) ;
            }
            return (*this);
         }

         inline T operator*() const
         {
            return current_token_;
         }

         inline std::string as_string() const
         {
            return std::string(current_token_.first,current_token_.second);
         }

         inline bool operator==(const tokenizer_iterator& itr) const
         {
            return (range_ == itr.range_) && (end_ == itr.end_);
         }

         inline bool operator!=(const tokenizer_iterator& itr) const
         {
            return (range_ != itr.range_) || (end_ != itr.end_);
         }

         inline tokenizer_iterator& operator=(const tokenizer_iterator& itr)
         {
            if (this != &itr)
            {
               range_                  = itr.range_;
               current_token_          = itr.current_token_;
               end_                    = itr.end_;
               compress_delimiters_    = itr.compress_delimiters_;
               include_1st_delimiter_  = itr.include_1st_delimiter_;
               include_all_delimiters_ = itr.include_all_delimiters_;
               include_delimiters_     = itr.include_delimiters_;
               last_token_done_        = itr.last_token_done_;
            }
            return (*this);
         }

         inline std::string remaining() const
         {
            return std::string(current_token_.first,end_);
         }

      protected:

         const Predicate& predicate_;
         iterator end_;
         range_type range_;
         range_type current_token_;
         bool compress_delimiters_;
         bool include_1st_delimiter_;
         bool include_all_delimiters_;
         bool include_delimiters_;
         bool last_token_done_;
      };

   public:

      typedef typename std::iterator_traits<Iterator>::value_type value_type;
      typedef DelimiterPredicate predicate;
      typedef tokenizer_iterator<Iterator,DelimiterPredicate> iterator;
      typedef const iterator const_iterator;
      typedef iterator& iterator_ref;
      typedef const_iterator& const_iterator_ref;

      inline tokenizer(const Iterator begin,
                       const Iterator end,
                       const DelimiterPredicate& predicate,
                       const tokenize_options::type tokenize_options = tokenize_options::default_mode)
      : tokenize_options_(tokenize_options),
        predicate_(predicate),
        begin_(begin),
        end_(end),
        begin_itr_(begin_,end_,predicate_,tokenize_options_),
        end_itr_(end_,end_,predicate_,tokenize_options_)
      {}

      inline tokenizer(const std::string& s,
                       const DelimiterPredicate& predicate,
                       const tokenize_options::type tokenize_options = tokenize_options::default_mode)
      : tokenize_options_(tokenize_options),
        predicate_(predicate),
        begin_(s.data()),
        end_(s.data() + s.size()),
        begin_itr_(begin_,end_,predicate_,tokenize_options_),
        end_itr_(end_,end_,predicate_,tokenize_options_)
      {}

      inline tokenizer& operator=(const tokenizer& t)
      {
         if (this != &t)
         {
            begin_            = t.begin_;
            end_              = t.end_;
            end_itr_          = t.end_itr_;
            begin_itr_        = t.begin_itr_;
            tokenize_options_ = t.tokenize_options_;
         }
         return (*this);
      }

      inline void assign(const std::string& s) const
      {
         assign(s.data(),s.data() + s.size());
      }

      inline void assign(const std::string& s)
      {
         assign(s.data(),s.data() + s.size());
      }

      inline void assign(const Iterator begin, const Iterator end)
      {
        begin_ = begin;
        end_ = end;
        begin_itr_ = iterator(begin_,end_,predicate_,tokenize_options_);
        end_itr_ = iterator(end_,end_,predicate_,tokenize_options_);
      }

      inline const_iterator_ref begin() const
      {
         return begin_itr_;
      }

      inline const_iterator_ref end() const
      {
         return end_itr_;
      }

   private:

      tokenize_options::type tokenize_options_;
      const DelimiterPredicate& predicate_;
      Iterator begin_;
      Iterator end_;
      iterator begin_itr_;
      iterator end_itr_;
   };

   namespace std_string
   {
      template <typename DelimiterPredicate = single_delimiter_predicate<std::string::value_type> >
      struct tokenizer
      {
         typedef DelimiterPredicate predicate_type;
         typedef const std::string::value_type* string_iterator_type;
         typedef strtk::tokenizer<string_iterator_type,DelimiterPredicate> type;
         typedef strtk::tokenizer<string_iterator_type,multiple_char_delimiter_predicate> md_type;
         typedef std::pair<string_iterator_type,string_iterator_type> iterator_type;
      };

      typedef tokenizer<>::iterator_type iterator_type;
      typedef tokenizer<>::iterator_type range_t;

      typedef std::vector<iterator_type> token_vector_type;
      typedef std::deque<iterator_type> token_deque_type;
      typedef std::list<iterator_type> token_list_type;

   } // namespace std_string

   template <typename Sequence>
   class range_to_type_back_inserter_iterator : public std::iterator<std::output_iterator_tag,
                                                                     void,
                                                                     void,
                                                                     void,
                                                                     void>
   {
   public:

      typedef typename Sequence::value_type value_type;

      explicit inline range_to_type_back_inserter_iterator(Sequence& sequence)
      : sequence_(sequence)
      {}

      range_to_type_back_inserter_iterator(const range_to_type_back_inserter_iterator& it)
      : sequence_(it.sequence_)
      {}

      inline range_to_type_back_inserter_iterator& operator=(const range_to_type_back_inserter_iterator& it)
      {
         if (this != &it)
         {
            this->sequence_ = it.sequence_;
         }
         return (*this);
      }

      template <typename Iterator>
      inline range_to_type_back_inserter_iterator& operator=(const std::pair<Iterator,Iterator>& r)
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
            sequence_.push_back(t);
         return (*this);
      }

      inline range_to_type_back_inserter_iterator& operator=(const std::string& s)
      {
         value_type t;
         if (string_to_type_converter(s.data(),s.data() + s.size(),t))
            sequence_.push_back(t);
         return (*this);
      }

      template <typename Iterator>
      inline void operator()(const std::pair<Iterator,Iterator>& r) const
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
            sequence_.push_back(t);
      }

      template <typename Iterator>
      inline void operator()(const Iterator begin, const Iterator end)
      {
         sequence_.push_back(string_to_type_converter<value_type>(begin,end));
      }

      inline range_to_type_back_inserter_iterator& operator*()
      {
         return (*this);
      }

      inline range_to_type_back_inserter_iterator& operator++()
      {
         return (*this);
      }

      inline range_to_type_back_inserter_iterator operator++(int)
      {
         return (*this);
      }

   private:

      Sequence& sequence_;
   };

   template <typename Sequence>
   inline range_to_type_back_inserter_iterator<Sequence> range_to_type_back_inserter(Sequence& sequence)
   {
      return (range_to_type_back_inserter_iterator<Sequence>(sequence));
   }

   template <typename Set>
   class range_to_type_inserter_iterator : public std::iterator<std::output_iterator_tag,
                                                                void,
                                                                void,
                                                                void,
                                                                void>
   {
   public:

      typedef typename Set::value_type value_type;

      explicit inline range_to_type_inserter_iterator(Set& set)
      : set_(set)
      {}

      range_to_type_inserter_iterator(const range_to_type_inserter_iterator& it)
      : set_(it.set_)
      {}

      inline range_to_type_inserter_iterator& operator=(const range_to_type_inserter_iterator& it)
      {
         if (this != &it)
         {
            this->set_ = it.set_;
         }
         return (*this);
      }

      template <typename Iterator>
      inline range_to_type_inserter_iterator& operator=(const std::pair<Iterator,Iterator>& r)
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
            set_.insert(t);
         return (*this);
      }

      template <typename Iterator>
      inline void operator()(const std::pair<Iterator,Iterator>& r)
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
            set_.insert(t);
      }

      inline range_to_type_inserter_iterator& operator*()
      {
         return (*this);
      }

      inline range_to_type_inserter_iterator& operator++()
      {
         return (*this);
      }

      inline range_to_type_inserter_iterator operator++(int)
      {
         return (*this);
      }

   private:

      Set& set_;
   };

   template <typename Set>
   inline range_to_type_inserter_iterator<Set> range_to_type_inserter(Set& set)
   {
      return (range_to_type_inserter_iterator<Set>(set));
   }

   template <typename Container>
   class range_to_type_push_inserter_iterator : public std::iterator<std::output_iterator_tag,
                                                                     void,
                                                                     void,
                                                                     void,
                                                                     void>
   {
   public:

      typedef typename Container::value_type value_type;

      explicit inline range_to_type_push_inserter_iterator(Container& container)
      : container_(container)
      {}

      range_to_type_push_inserter_iterator(const range_to_type_push_inserter_iterator& it)
      : container_(it.container_)
      {}

      inline range_to_type_push_inserter_iterator& operator=(const range_to_type_push_inserter_iterator& it)
      {
         if (this != &it)
         {
            this->container_ = it.container_;
         }
         return (*this);
      }

      template <typename Iterator>
      inline range_to_type_push_inserter_iterator& operator=(const std::pair<Iterator,Iterator>& r)
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
            container_.push(t);
         return (*this);
      }

      template <typename Iterator>
      inline void operator()(const std::pair<Iterator,Iterator>& r)
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
            container_.push(t);
      }

      inline range_to_type_push_inserter_iterator& operator*()
      {
         return (*this);
      }

      inline range_to_type_push_inserter_iterator& operator++()
      {
         return (*this);
      }

      inline range_to_type_push_inserter_iterator operator++(int)
      {
         return (*this);
      }

   private:

      Container& container_;
   };

   template <typename Container>
   inline range_to_type_push_inserter_iterator<Container> range_to_type_push_inserter(Container& container)
   {
      return (range_to_type_push_inserter_iterator<Container>(container));
   }

   template <typename Sequence>
   class back_inserter_with_valuetype_iterator : public std::iterator<std::output_iterator_tag,
                                                                      typename Sequence::value_type,
                                                                      void,
                                                                      void,
                                                                      void>
   {
   public:

      explicit inline back_inserter_with_valuetype_iterator(Sequence& sequence)
      : sequence_(sequence)
      {}

      back_inserter_with_valuetype_iterator(const back_inserter_with_valuetype_iterator& it)
      : sequence_(it.sequence_)
      {}

      inline back_inserter_with_valuetype_iterator& operator=(const back_inserter_with_valuetype_iterator& it)
      {
         if (this != &it)
         {
            this->sequence_ = it.sequence_;
         }
         return (*this);
      }

      inline back_inserter_with_valuetype_iterator& operator=(const typename Sequence::value_type& v)
      {
         sequence_.push_back(v);
         return (*this);
      }

      inline void operator()(const typename Sequence::value_type& v)
      {
         sequence_.push_back(v);
      }

      inline back_inserter_with_valuetype_iterator& operator*()
      {
         return (*this);
      }

      inline back_inserter_with_valuetype_iterator& operator++()
      {
         return (*this);
      }

      inline back_inserter_with_valuetype_iterator operator++(int)
      {
         return (*this);
      }

   private:

      Sequence& sequence_;
   };

   template <typename Sequence>
   inline back_inserter_with_valuetype_iterator<Sequence> back_inserter_with_valuetype(Sequence& sequence_)
   {
      return (back_inserter_with_valuetype_iterator<Sequence>(sequence_));
   }

   template <typename Set>
   class inserter_with_valuetype_iterator : public std::iterator<std::output_iterator_tag,
                                                                 typename Set::value_type,
                                                                 void,
                                                                 void,
                                                                 void>
   {
   public:

      explicit inline inserter_with_valuetype_iterator(Set& set)
      : set_(set)
      {}

      inserter_with_valuetype_iterator(const inserter_with_valuetype_iterator& itr)
      : set_(itr.set_)
      {}

      inline inserter_with_valuetype_iterator& operator=(const inserter_with_valuetype_iterator& itr)
      {
         if (this != &itr)
         {
            this->set_ = itr.set_;
         }
         return (*this);
      }

      inline inserter_with_valuetype_iterator& operator=(const typename Set::value_type& v)
      {
         set_.insert(v);
         return (*this);
      }

      inline void operator()(const typename Set::value_type& v)
      {
         set_.insert(v);
      }

      inline inserter_with_valuetype_iterator& operator*()
      {
         return (*this);
      }

      inline inserter_with_valuetype_iterator& operator++()
      {
         return (*this);
      }

      inline inserter_with_valuetype_iterator operator++(int)
      {
         return (*this);
      }

   private:

      Set& set_;
   };

   template <typename Set>
   inline inserter_with_valuetype_iterator<Set> inserter_with_valuetype(Set& set_)
   {
      return (inserter_with_valuetype_iterator<Set>(set_));
   }

   template <typename Container>
   class push_inserter_iterator : public std::iterator<std::output_iterator_tag,
                                                       void,
                                                       void,
                                                       void,
                                                       void>
   {
   public:

      explicit inline push_inserter_iterator(Container& container)
      : container_(container)
      {}

      inline push_inserter_iterator& operator=(const push_inserter_iterator& itr)
      {
         if (this != &itr)
         {
            this->container_ = itr.container_;
         }
         return (*this);
      }

      inline push_inserter_iterator<Container>& operator=(typename Container::const_reference v)
      {
         container_.push(v);
         return (*this);
      }

      inline push_inserter_iterator<Container>& operator*()
      {
         return (*this);
      }

      inline push_inserter_iterator<Container>& operator++()
      {
         return (*this);
      }

      inline push_inserter_iterator<Container> operator++(int)
      {
         return (*this);
      }

   private:

      Container& container_;
   };

   template <typename Container>
   inline push_inserter_iterator<Container> push_inserter(Container& c)
   {
      return push_inserter_iterator<Container>(c);
   }

   template <typename T>
   class range_to_ptr_type_iterator : public std::iterator<std::output_iterator_tag,
                                                           void,
                                                           void,
                                                           void,
                                                           void>
   {
   public:

      typedef T value_type;

      explicit inline range_to_ptr_type_iterator(T* pointer, std::size_t& insert_count)
      : pointer_(pointer),
        insert_count_(insert_count)
      {}

      range_to_ptr_type_iterator(const range_to_ptr_type_iterator& it)
      : pointer_(it.pointer_)
      {}

      inline range_to_ptr_type_iterator& operator=(const range_to_ptr_type_iterator& it)
      {
         if (this != &it)
         {
            this->pointer_ = it.pointer_;
         }
         return (*this);
      }

      template <typename Iterator>
      inline range_to_ptr_type_iterator& operator=(const std::pair<Iterator,Iterator>& r)
      {
         value_type t = value_type();
         if (string_to_type_converter(r.first,r.second,t))
         {
            (*pointer_) = t;
            ++pointer_;
            ++insert_count_;
         }
         return (*this);
      }

      inline range_to_ptr_type_iterator& operator=(const std::string& s)
      {
         value_type t = value_type();
         if (string_to_type_converter(s.data(),s.data() + s.size(),t))
         {
            (*pointer_) = t;
            ++pointer_;
            ++insert_count_;
         }
         return (*this);
      }

      template <typename Iterator>
      inline void operator()(const std::pair<Iterator,Iterator>& r) const
      {
         value_type t;
         if (string_to_type_converter(r.first,r.second,t))
         {
            (*pointer_) = t;
            ++pointer_;
            ++insert_count_;
         }
      }

      template <typename Iterator>
      inline void operator()(const Iterator begin, const Iterator end)
      {
         (*pointer_) = string_to_type_converter<T>(begin,end);
         ++pointer_;
         ++insert_count_;
      }

      inline range_to_ptr_type_iterator& operator*()
      {
         return (*this);
      }

      inline range_to_ptr_type_iterator& operator++()
      {
         return (*this);
      }

      inline range_to_ptr_type_iterator operator++(int)
      {
         return (*this);
      }

   private:

      T* pointer_;
      std::size_t& insert_count_;
   };

   template <typename T>
   inline range_to_ptr_type_iterator<T> range_to_ptr_type(T* pointer, std::size_t& insert_count)
   {
      return (range_to_ptr_type_iterator<T>(pointer,insert_count));
   }

   template <typename T>
   inline range_to_ptr_type_iterator<T> range_to_ptr_type(T* pointer)
   {
      static std::size_t insert_count = 0;
      return (range_to_ptr_type_iterator<T>(pointer,insert_count));
   }

   template <typename T>
   class counting_back_inserter_iterator : public std::iterator<std::output_iterator_tag,
                                                                T,
                                                                void,
                                                                void,
                                                                void>
   {
   public:

      explicit inline counting_back_inserter_iterator(std::size_t& counter)
      : counter_(counter)
      {}

      counting_back_inserter_iterator(const counting_back_inserter_iterator& itr)
      : counter_(itr.counter_)
      {}

      inline counting_back_inserter_iterator& operator=(const counting_back_inserter_iterator& itr)
      {
         if (this != &itr)
         {
            this->counter_ = itr.counter_;
         }
         return (*this);
      }

      inline counting_back_inserter_iterator& operator=(const T&)
      {
         ++counter_;
         return (*this);
      }

      inline void operator()(const T&)
      {
         ++counter_;
      }

      inline counting_back_inserter_iterator& operator*()
      {
         return (*this);
      }

      inline counting_back_inserter_iterator& operator++()
      {
         return (*this);
      }

      inline counting_back_inserter_iterator operator++(int)
      {
         return (*this);
      }

   private:

      std::size_t& counter_;
   };

   template <typename T>
   inline counting_back_inserter_iterator<T> counting_back_inserter(std::size_t& counter_)
   {
      return (counting_back_inserter_iterator<T>(counter_));
   }

   template <typename Function>
   class functional_inserter_iterator : public std::iterator<std::output_iterator_tag,
                                                             void,
                                                             void,
                                                             void,
                                                             void>
   {
   public:

      explicit inline functional_inserter_iterator(Function function)
      : function_(function)
      {}

      functional_inserter_iterator(const functional_inserter_iterator& it)
      : function_(it.function_)
      {}

      inline functional_inserter_iterator& operator=(const functional_inserter_iterator& it)
      {
         if (this != &it)
         {
            this->function_ = it.function_;
         }
         return (*this);
      }

      template <typename T>
      inline functional_inserter_iterator& operator=(const T& t)
      {
         function_(t);
         return (*this);
      }

      template <typename T>
      inline void operator()(const T& t)
      {
         function_(t);
      }

      inline functional_inserter_iterator& operator*()
      {
         return (*this);
      }

      inline functional_inserter_iterator& operator++()
      {
         return (*this);
      }

      inline functional_inserter_iterator operator++(int)
      {
         return (*this);
      }

   private:

      Function function_;
   };

   template <typename Function>
   inline functional_inserter_iterator<Function> functional_inserter(Function function)
   {
      return (functional_inserter_iterator<Function>(function));
   }

   namespace split_options
   {
      typedef std::size_t type;
      enum
      {
         default_mode            = 0,
         compress_delimiters     = 1,
         include_1st_delimiter   = 2,
         include_all_delimiters  = 4
      };

      static inline bool perform_compress_delimiters(const type& split_opt)
      {
         return compress_delimiters == (split_opt & compress_delimiters);
      }

      static inline bool perform_include_1st_delimiter(const type& split_opt)
      {
         return include_1st_delimiter == (split_opt & include_1st_delimiter);
      }

      static inline bool perform_include_all_delimiters(const type& split_opt)
      {
         return include_all_delimiters == (split_opt & include_all_delimiters);
      }

   } // namespace split_options

   namespace details
   {
      template <typename DelimiterPredicate,
                typename Iterator,
                typename OutputIterator>
      inline std::size_t split_compress_delimiters(const DelimiterPredicate& delimiter,
                                                   const Iterator begin,
                                                   const Iterator end,
                                                   OutputIterator out)
      {
         std::size_t token_count = 0;
         std::pair<Iterator,Iterator> range(begin,begin);

         while (end != range.second)
         {
            if (delimiter(*range.second))
            {
               (*out) = range;
               ++out;
               while ((end != ++range.second) && delimiter(*range.second));
               range.first = range.second;
               if (end != range.second)
                  ++range.second;
               ++token_count;
            }
            else
               ++range.second;
         }

         if ((range.first != range.second) || delimiter(*(range.second - 1)))
         {
            (*out) = range;
            ++out;
            ++token_count;
         }

         return token_count;
      }
   }

   template <typename DelimiterPredicate,
             typename Iterator,
             typename OutputIterator>
   inline std::size_t split(const DelimiterPredicate& delimiter,
                            const Iterator begin,
                            const Iterator end,
                            OutputIterator out,
                            const split_options::type split_option = split_options::default_mode)
   {
      if (begin == end) return 0;
      const bool compress_delimiters = split_options::perform_compress_delimiters(split_option);
      const bool include_1st_delimiter = split_options::perform_include_1st_delimiter(split_option);
      const bool include_all_delimiters = (!include_1st_delimiter) && split_options::perform_include_all_delimiters(split_option);
      const bool include_delimiters = include_1st_delimiter || include_all_delimiters;

      if (compress_delimiters && (!include_delimiters))
      {
         return details::split_compress_delimiters(delimiter,begin,end,out);
      }

      std::size_t token_count = 0;
      std::pair<Iterator,Iterator> range(begin,begin);

      while (end != range.second)
      {
         if (delimiter(*range.second))
         {
            if (include_delimiters)
            {
               if (include_1st_delimiter)
                  ++range.second;
               else if (include_all_delimiters)
                  while ((end != range.second) && delimiter(*range.second)) ++range.second;
               (*out) = range;
               ++out;
               if ((!include_all_delimiters) && compress_delimiters)
                  while ((end != range.second) && delimiter(*range.second)) ++range.second;
            }
            else
            {
               (*out) = range;
               ++out;
                  ++range.second;
            }
            ++token_count;
            range.first = range.second;
         }
         else
            ++range.second;
      }

      if ((range.first != range.second) || delimiter(*(range.second - 1)))
      {
         (*out) = range;
         ++out;
         ++token_count;
      }

      return token_count;
   }

   template <typename DelimiterPredicate,
             typename Iterator,
             typename OutputIterator>
   inline std::size_t split(const DelimiterPredicate& delimiter,
                            const std::pair<Iterator,Iterator>& range,
                            OutputIterator out,
                            const split_options::type split_option = split_options::default_mode)
   {
      return split(delimiter,
                   range.first,range.second,
                   out,
                   split_option);
   }

   template <typename DelimiterPredicate,
             typename Iterator,
             typename OutputIterator>
   inline std::size_t split(const char* delimiters,
                            const std::pair<Iterator,Iterator>& range,
                            OutputIterator out,
                            const split_options::type split_option = split_options::default_mode)
   {
      if (1 == details::strnlength(delimiters,256))
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      out,
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      out,
                      split_option);
   }

   template <typename DelimiterPredicate,
             typename Iterator,
             typename OutputIterator>
   inline std::size_t split(const std::string& delimiters,
                            const std::pair<Iterator,Iterator>& range,
                            OutputIterator out,
                            const split_options::type split_option = split_options::default_mode)
   {
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      out,
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      out,
                      split_option);
   }

   template <typename OutputIterator>
   inline std::size_t split(const char* delimiters,
                            const std::string& str,
                            OutputIterator out,
                            const split_options::type& split_option = split_options::default_mode)
   {
      if (1 == details::strnlength(delimiters,256))
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      str.data(), str.data() + str.size(),
                      out,
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      str.data(), str.data() + str.size(),
                      out,
                      split_option);
   }

   template <typename OutputIterator>
   inline std::size_t split(const std::string& delimiters,
                            const std::string& str,
                            OutputIterator out,
                            const split_options::type& split_option = split_options::default_mode)
   {
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      str.data(), str.data() + str.size(),
                      out,
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      str.data(), str.data() + str.size(),
                      out,
                      split_option);
   }

   template <typename OutputIterator>
   inline std::size_t split(const std::string::value_type delimiter,
                            const std::string& str,
                            OutputIterator out,
                            const split_options::type& split_option = split_options::default_mode)
   {
      return split(single_delimiter_predicate<std::string::value_type>(delimiter),
                   str.data(), str.data() + str.size(),
                   out,
                   split_option);
   }

   template <typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t split(const char* delimiters,
                            const std::string& str,
                            Sequence<std::pair<const char*, const char*>,Allocator>& sequence,
                            const split_options::type& split_option = split_options::default_mode)
   {
      if (1 == details::strnlength(delimiters,256))
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      str.data(), str.data() + str.size(),
                      std::back_inserter(sequence),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      str.data(), str.data() + str.size(),
                      std::back_inserter(sequence),
                      split_option);
   }

   template <typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t split(const std::string& delimiters,
                            const std::string& str,
                            Sequence<std::pair<const char*, const char*>,Allocator>& sequence,
                            const split_options::type& split_option = split_options::default_mode)
   {
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      str.data(), str.data() + str.size(),
                      std::back_inserter(sequence),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      str.data(), str.data() + str.size(),
                      std::back_inserter(sequence),
                      split_option);
   }

   template <typename DelimiterPredicate,
             typename OutputIterator>
   inline std::size_t split(const DelimiterPredicate& delimiter,
                            const std::string& str,
                            OutputIterator out,
                            const split_options::type& split_option = split_options::default_mode)
   {
      return split(delimiter,
                   str.data(), str.data() + str.size(),
                   out,
                   split_option);
   }

   template <typename DelimiterPredicate,
             typename Iterator,
             typename OutputIterator>
   inline std::size_t split_n(const DelimiterPredicate& delimiter,
                              const Iterator begin,
                              const Iterator end,
                              const std::size_t& token_count,
                              OutputIterator out,
                              const split_options::type& split_option = split_options::default_mode)
   {
      if (0 == token_count) return 0;
      if (begin == end) return 0;
      std::size_t match_count = 0;
      std::pair<Iterator,Iterator> range(begin,begin);
      const bool compress_delimiters = split_options::perform_compress_delimiters(split_option);
      const bool include_1st_delimiter = split_options::perform_include_1st_delimiter(split_option);
      const bool include_all_delimiters = (!include_1st_delimiter) && split_options::perform_include_all_delimiters(split_option);
      const bool include_delimiters = include_1st_delimiter || include_all_delimiters;

      while (end != range.second)
      {
         if (delimiter(*range.second))
         {
            if (include_delimiters)
            {
               ++range.second;
               (*out) = range;
               ++out;
               if (++match_count >= token_count)
                  return match_count;
               if (compress_delimiters)
                  while ((end != range.second) && delimiter(*range.second)) ++range.second;
            }
            else
            {
               (*out) = range;
               ++out;
               if (++match_count >= token_count)
                  return match_count;
               if (compress_delimiters)
                  while ((end != (++range.second)) && delimiter(*range.second)) ;
               else
                  ++range.second;
            }
            range.first = range.second;
         }
         else
            ++range.second;
      }

      if ((range.first != range.second) || delimiter(*(range.second - 1)))
      {
         (*out) = range;
         ++out;
         ++match_count;
      }

      return match_count;
   }

   template <typename OutputIterator>
   inline std::size_t split_n(const char* delimiters,
                              const std::string& str,
                              const std::size_t& token_count,
                              OutputIterator out,
                              const split_options::type& split_option = split_options::default_mode)
   {
      return split_n(multiple_char_delimiter_predicate(delimiters),
                     str.data(), str.data() + str.size(),
                     token_count,
                     out,
                     split_option);
   }

   template <typename OutputIterator>
   inline std::size_t split_n(const std::string& delimiters,
                              const std::string& str,
                              const std::size_t& token_count,
                              OutputIterator out,
                              const split_options::type& split_option = split_options::default_mode)
   {
      if (1 == delimiters.size())
         return split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                        str.data(), str.data() + str.size(),
                        token_count,
                        out,
                        split_option);
      else
         return split_n(multiple_char_delimiter_predicate(delimiters),
                        str.data(), str.data() + str.size(),
                        token_count,
                        out,
                        split_option);
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_n(const std::string& delimiters,
                              const InputIterator begin,
                              const InputIterator end,
                              const std::size_t& token_count,
                              OutputIterator out,
                              const split_options::type& split_option = split_options::default_mode)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                        begin,end,
                        token_count,
                        out,
                        split_option);
      else
         return split_n(multiple_char_delimiter_predicate(delimiters),
                        begin,end,
                        token_count,
                        out,
                        split_option);
   }

   template <typename OutputIterator>
   inline std::size_t split_n(const std::string::value_type delimiter,
                              const std::string& str,
                              const std::size_t& token_count,
                              OutputIterator out,
                              const split_options::type& split_option = split_options::default_mode)
   {
      return split_n(single_delimiter_predicate<std::string::value_type>(delimiter),
                     str.data(),str.data() + str.size(),
                     token_count,
                     out,
                     split_option);
   }

   template <typename DelimiterPredicate,
             typename OutputIterator>
   inline std::size_t split_n(const DelimiterPredicate& delimiter,
                              const std::string& str,
                              const std::size_t& token_count,
                              OutputIterator out,
                              const split_options::type& split_option = split_options::default_mode)
   {
      return split_n(delimiter,
                     str.data(),str.data() + str.size(),
                     token_count,
                     out,
                     split_option);
   }

   #ifdef strtk_enable_regex

   static const std::string uri_expression     ("((https?|ftp)\\://((\\[?(\\d{1,3}\\.){3}\\d{1,3}\\]?)|(([-a-zA-Z0-9]+\\.)+[a-zA-Z]{2,4}))(\\:\\d+)?(/[-a-zA-Z0-9._?,+&amp;%$#=~\\\\]+)*/?)");
   static const std::string email_expression   ("([\\w\\-\\.]+)@((\\[([0-9]{1,3}\\.){3}[0-9]{1,3}\\])|(([\\w\\-]+\\.)+)([a-zA-Z]{2,4}))");
   static const std::string ip_expression      ("(([0-2]*[0-9]+[0-9]+)\\.([0-2]*[0-9]+[0-9]+)\\.([0-2]*[0-9]+[0-9]+)\\.([0-2]*[0-9]+[0-9]+))");
   static const std::string ieee754_expression ("([-+]?((\\.[0-9]+|[0-9]+\\.[0-9]+)([eE][-+][0-9]+)?|[0-9]+))");

   namespace regex_match_mode
   {
      enum type
      {
         match_all = 0,
         match_1   = 1,
         match_2   = 2,
         match_3   = 3,
         match_4   = 4,
         match_5   = 5,
         match_6   = 6,
         match_7   = 7,
         match_8   = 8,
         match_9   = 9
      };
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_regex(const boost::regex& delimiter_expression,
                                  const InputIterator begin,
                                  const InputIterator end,
                                  OutputIterator out,
                                  const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      boost::regex_iterator<InputIterator> itr(begin,end,delimiter_expression);
      boost::regex_iterator<InputIterator> itr_end;
      std::pair<InputIterator,InputIterator> range(begin,begin);
      std::size_t match_count = 0;
      while (itr_end != itr)
      {
         range.first = (*itr)[mode].first;
         range.second = (*itr)[mode].second;
         (*out) = range;
         ++out;
         ++itr;
         ++match_count;
      }
      return match_count;
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_regex(const std::string& delimiter_expression,
                                  const InputIterator begin,
                                  const InputIterator end,
                                  OutputIterator out,
                                  const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      const boost::regex regex_expression(delimiter_expression);
      return split_regex(regex_expression,
                         begin,end,
                         out,
                         mode);
   }

   template <typename OutputIterator>
   inline std::size_t split_regex(const std::string& delimiter_expression,
                                  const std::string& text,
                                  OutputIterator out,
                                  const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      return split_regex(delimiter_expression,
                         text.begin(),text.end(),
                         out,
                         mode);
   }

   template <typename OutputIterator>
   inline std::size_t split_regex(const boost::regex& delimiter_expression,
                                  const std::string& text,
                                  OutputIterator out,
                                  const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      return split_regex(delimiter_expression,
                         text.begin(),text.end(),
                         out,
                         mode);
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_regex_n(const boost::regex& delimiter_expression,
                                    const InputIterator begin,
                                    const InputIterator end,
                                    const std::size_t& token_count,
                                    OutputIterator out,
                                    const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      boost::sregex_iterator itr(begin,end,delimiter_expression);
      const boost::sregex_iterator itr_end;
      std::pair<InputIterator,InputIterator> range(begin,begin);
      std::size_t match_count = 0;
      while (itr_end != itr)
      {
         range.first = (*itr)[mode].first;
         range.second = (*itr)[mode].second;
         (*out) = range;
         ++out;
         ++itr;
         if (++match_count >= token_count)
            return match_count;
      }
      return match_count;
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_regex_n(const std::string& delimiter_expression,
                                    const InputIterator begin,
                                    const InputIterator end,
                                    const std::size_t& token_count,
                                    OutputIterator out,
                                    const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      const boost::regex regex_expression(delimiter_expression);
      return split_regex_n(regex_expression,
                           begin,end,
                           token_count,
                           out,
                           mode);
   }

   template <typename OutputIterator>
   inline std::size_t split_regex_n(const std::string& delimiter_expression,
                                    const std::string& text,
                                    const std::size_t& token_count,
                                    OutputIterator out,
                                    const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      return split_regex_n(delimiter_expression,
                           text.begin(),text.end(),
                           token_count,
                           out,
                           mode);
   }

   template <typename OutputIterator>
   inline std::size_t split_regex_n(const boost::regex& delimiter_expression,
                                    const std::string& text,
                                    const std::size_t& token_count,
                                    OutputIterator out,
                                    const regex_match_mode::type mode = regex_match_mode::match_all)
   {
      return split_regex_n(delimiter_expression,
                           text.begin(),text.end(),
                           token_count,
                           out,
                           mode);
   }

   #endif // strtk_enable_regex

   template <const std::size_t offset_list_size>
   class offset_predicate
   {
   public:

      offset_predicate(const int offset_list[], const bool rotate = false)
      : rotate_(rotate),
        current_index_(0)
      {
         std::copy(offset_list, offset_list + offset_list_size, offset_list_);
         offset_list_[offset_list_size] = 0;
      }

      inline bool operator!() const
      {
         return (0 == offset_list_size);
      }

      inline void reset() const
      {
         current_index_ = 0;
      }

      inline std::size_t size() const
      {
         return offset_list_size;
      }

      inline int next() const
      {
         int result = offset_list_[current_index_++];
         if (rotate_ && (current_index_ >= offset_list_size))
         {
            current_index_ = 0;
         }
         return result;
      }

   private:

      bool rotate_;
      mutable std::size_t current_index_;
      int offset_list_[offset_list_size + 1];
   };

   inline offset_predicate<12> offsets(const int&  v1, const int&  v2, const int&  v3,
                                       const int&  v4, const int&  v5, const int&  v6,
                                       const int&  v7, const int&  v8, const int&  v9,
                                       const int& v10, const int& v11, const int& v12,
                                       const bool& rotate = false)
   {
      const int offset_list[12] = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12 };
      return offset_predicate<12>(offset_list,rotate);
   }

   inline offset_predicate<11> offsets(const int&  v1, const int&  v2, const int& v3,
                                       const int&  v4, const int&  v5, const int& v6,
                                       const int&  v7, const int&  v8, const int& v9,
                                       const int& v10, const int& v11,
                                       const bool& rotate = false)
   {
      const int offset_list[11] = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 };
      return offset_predicate<11>(offset_list,rotate);
   }

   inline offset_predicate<10> offsets(const int& v1, const int& v2, const int& v3,
                                       const int& v4, const int& v5, const int& v6,
                                       const int& v7, const int& v8, const int& v9,
                                       const int& v10, const bool& rotate = false)
   {
      const int offset_list[10] = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 };
      return offset_predicate<10>(offset_list,rotate);
   }

   inline offset_predicate<9> offsets(const int& v1, const int& v2, const int& v3,
                                      const int& v4, const int& v5, const int& v6,
                                      const int& v7, const int& v8, const int& v9,
                                      const bool& rotate = false)
   {
      const int offset_list[9] = { v1, v2, v3, v4, v5, v6, v7, v8, v9 };
      return offset_predicate<9>(offset_list,rotate);
   }

   inline offset_predicate<8> offsets(const int& v1, const int& v2, const int& v3,
                                      const int& v4, const int& v5, const int& v6,
                                      const int& v7, const int& v8, const bool& rotate = false)
   {
      const int offset_list[8] = { v1, v2, v3, v4, v5, v6, v7, v8 };
      return offset_predicate<8>(offset_list,rotate);
   }

   inline offset_predicate<7> offsets(const int& v1, const int& v2, const int& v3,
                                      const int& v4, const int& v5, const int& v6,
                                      const int& v7, const bool& rotate = false)
   {
      const int offset_list[7] = { v1, v2, v3, v4, v5, v6, v7 };
      return offset_predicate<7>(offset_list,rotate);
   }

   inline offset_predicate<6> offsets(const int& v1, const int& v2, const int& v3,
                                      const int& v4, const int& v5, const int& v6,
                                      const bool& rotate = false)
   {
      const int offset_list[6] = { v1, v2, v3, v4, v5, v6 };
      return offset_predicate<6>(offset_list,rotate);
   }

   inline offset_predicate<5> offsets(const int& v1, const int& v2, const int& v3,
                                      const int& v4, const int& v5, const bool& rotate = false)
   {
      const int offset_list[5] = { v1, v2, v3, v4, v5 };
      return offset_predicate<5>(offset_list,rotate);
   }

   inline offset_predicate<4> offsets(const int& v1, const int& v2, const int& v3,
                                      const int& v4, const bool& rotate = false)
   {
      const int offset_list[4] = { v1, v2, v3, v4 };
      return offset_predicate<4>(offset_list,rotate);
   }

   inline offset_predicate<3> offsets(const int& v1, const int& v2, const int& v3,
                                      const bool& rotate = false)
   {
      const int offset_list[3] = { v1, v2, v3 };
      return offset_predicate<3>(offset_list,rotate);
   }

   inline offset_predicate<2> offsets(const int& v1, const int& v2, const bool& rotate = false)
   {
      const int offset_list[2] = { v1, v2 };
      return offset_predicate<2>(offset_list,rotate);
   }

   inline offset_predicate<1> offsets(const int& v1,
                                      const bool& rotate = false)
   {
      const int offset_list[1] = { v1 };
      return offset_predicate<1>(offset_list,rotate);
   }

   template <typename OffsetPredicate,
             typename InputIterator,
             typename OutputIterator>
   inline std::size_t offset_splitter(const InputIterator begin,
                                      const InputIterator end,
                                      const OffsetPredicate& offset,
                                      OutputIterator out)
   {
      std::size_t length = 0;
      if (0 == (length = std::distance(begin,end))) return 0;
      std::pair<InputIterator,InputIterator> range(begin,begin);
      std::size_t match_count = 0;
      int offset_length = 0;
      std::size_t increment_amount = 0;
      while ((end != range.second) && (0 < (offset_length = offset.next())))
      {
         increment_amount = std::min<std::size_t>(length,offset_length);
         range.first = range.second;
         range.second += increment_amount;
         length -= increment_amount;
         (*out) = range;
         ++out;
         ++match_count;
      }
      return match_count;
   }

   template <typename OffsetPredicate,
             typename OutputIterator>
   inline std::size_t offset_splitter(const std::string& str,
                                      const OffsetPredicate& offset,
                                      OutputIterator out)
   {
      return offset_splitter(str.data(),str.data() + str.size(),offset,out);
   }

   template <typename InputIterator,
             typename Predicate,
             typename OutputPair>
   inline bool split_pair(const InputIterator begin,
                          const InputIterator end,
                          const Predicate& delimiter,
                          OutputPair& v1,
                          OutputPair& v2)
   {
      if (0 == std::distance(begin,end)) return false;

      InputIterator itr = begin;

      while (end != itr)
      {
         if (delimiter(*itr))
         {
            v1 = std::make_pair(begin,itr);
            ++itr;
            if (0 != std::distance(itr,end))
            {
               v2 = std::make_pair(itr,end);
               return true;
            }
            else
               return false;
         }
         else
            ++itr;
      }

      return false;
   }

   inline bool split_pair(const std::string::value_type delimiter,
                          const std::string& str,
                          std::pair<const char*,const char*>& v1,
                          std::pair<const char*,const char*>& v2)
   {
      return split_pair(str.data(),
                        str.data() + str.size(),
                        single_delimiter_predicate<std::string::value_type>(delimiter),
                        v1,
                        v2);
   }

   template <typename DelimiterPredicate>
   inline bool split_pair(const DelimiterPredicate& delimiter,
                          const std::string& str,
                          std::pair<const char*,const char*>& v1,
                          std::pair<const char*,const char*>& v2)
   {
      return split_pair(str.data(),
                        str.data() + str.size(),
                        delimiter,
                        v1,
                        v2);
   }

   template <typename Function>
   inline std::size_t for_each_token(const std::string& buffer,
                                     const std::string& delimiters,
                                     Function function)
   {
      return split(delimiters,
                   buffer,
                   strtk::functional_inserter<Function>(function));
   }

   template <typename Function>
   inline std::size_t for_each_token(const std::string& buffer,
                                     const char* delimiters,
                                     Function function)
   {
      return split(delimiters,
                   buffer,
                   strtk::functional_inserter<Function>(function));
   }

   template <typename InputIterator>
   inline std::size_t count_consecutive_duplicates(const InputIterator begin, const InputIterator end)
   {
      if (std::distance(begin,end) < 2) return 0;
      InputIterator prev = begin;
      InputIterator itr = begin;
      std::size_t count = 0;
      while (end != ++itr)
      {
         if ((*prev) == (*itr))
            ++count;
         else
            prev = itr;
      }
      return count;
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline T min_of_cont(const Sequence<T,Allocator>& sequence)
   {
      return (*std::min_element(sequence.begin(),sequence.end()));
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline T min_of_cont(const std::set<T,Comparator,Allocator>& set)
   {
      return (*set.begin());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline T min_of_cont(const std::multiset<T,Comparator,Allocator>& multiset)
   {
      return (*multiset.begin());
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline T max_of_cont(const Sequence<T,Allocator>& sequence)
   {
      return (*std::max_element(sequence.begin(),sequence.end()));
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline T max_of_cont(const std::set<T,Comparator,Allocator>& set)
   {
      return (*set.rbegin());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline T max_of_cont(const std::multiset<T,Comparator,Allocator>& multiset)
   {
      return (*multiset.rbegin());
   }

   template <typename InputIterator>
   inline void min_max_of_range(const InputIterator begin, const InputIterator end,
                                typename std::iterator_traits<InputIterator>::value_type& min_value,
                                typename std::iterator_traits<InputIterator>::value_type& max_value)
   {
      min_value = *begin;
      max_value = *begin;
      InputIterator itr = begin;
      while (end != ++itr)
      {
         if (*itr < min_value)
            min_value = (*itr);
         else if (*itr > max_value)
            max_value = (*itr);
      }
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void min_max_of_cont(const Sequence<T,Allocator>& sequence,
                               T& min_value,
                               T& max_value)
   {
      min_max_of_range(sequence.begin(),sequence.end(),
                       min_value,
                       max_value);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void min_max_of_cont(const std::set<T,Comparator,Allocator>& set,
                               T& min_value,
                               T& max_value)
   {
      min_value = (*set.begin());
      max_value = (*set.rbegin());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void min_max_of_cont(const std::multiset<T,Comparator,Allocator>& multiset,
                               T& min_value,
                               T& max_value)
   {
      min_value = (*multiset.begin());
      max_value = (*multiset.rbegin());
   }

   template <typename Iterator>
   inline void lexicographically_canonicalize(Iterator begin, Iterator end)
   {
      typedef typename std::iterator_traits<Iterator>::value_type type;
      typedef typename std::pair<Iterator,Iterator> iter_type;
      typedef typename std::list<iter_type> itr_list_type;
      itr_list_type itr_list;

      type smallest = (*std::min_element(begin,end));

      for (Iterator itr = begin; itr != end; ++itr)
      {
         if (*itr == smallest) itr_list.push_back(std::make_pair(itr,itr));
      }

      while (itr_list.size() > 1)
      {
         typename itr_list_type::iterator itr = itr_list.begin();
         while (itr_list.end() != itr)
         {
            ++(*itr).first;
            if (end == (*itr).first)
               itr = itr_list.erase(itr);
            else
               ++itr;
         }

         smallest = *(*itr_list.begin()).first;

         for (itr = (++itr_list.begin()); itr != itr_list.end(); ++itr)
         {
            if (*(*itr).first < smallest)
            {
               smallest = *(*itr).first;
            }
         }

         itr = itr_list.begin();
         while (itr_list.end() != itr)
         {
           if (*(*itr).first != smallest)
              itr = itr_list.erase(itr);
           else
              ++itr;
         }

         itr = itr_list.begin();
         while (itr_list.end() != itr)
         {
            if (end == (*itr).first)
               itr = itr_list.erase(itr);
            else
               ++itr;
         }

      }

      std::rotate(begin,(*itr_list.begin()).second,end);
   }

   inline void lexicographically_canonicalize(std::string& str)
   {
      lexicographically_canonicalize(const_cast<char*>(str.data()),
                                     const_cast<char*>(str.data() + str.size()));
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void lexicographically_canonicalize(Sequence<T,Allocator>& sequence)
   {
      lexicographically_canonicalize(sequence.begin(),sequence.end());
   }

   inline const char* first_non_repeated_char(const char* begin, const char* end)
   {
      static const std::size_t lut_size = 256;
      unsigned long long int lut[lut_size];

      std::fill_n(lut,lut_size,std::numeric_limits<unsigned long long int>::max());

      static const unsigned long long int not_yet_encountered = std::numeric_limits<unsigned long long int>::max();
      static const unsigned long long int repeated = not_yet_encountered - 1;

      const char* itr = begin;
      unsigned long long int position = 0;
      while (end != itr)
      {
         unsigned long long int& element = lut[static_cast<unsigned int>(*itr)];
         if (not_yet_encountered == element)
         {
            element = position;
         }
         else if (element < repeated)
         {
            element = repeated;
         }
         ++itr;
         ++position;
      }

      position = repeated;

      for (std::size_t i = 0; i < lut_size; ++i)
      {
         if (lut[i] < position)
            position = lut[i];
      }

      return (repeated != position) ? (begin + position) : end;
   }

   inline const unsigned char* first_non_repeated_char(const unsigned char* begin, const unsigned char* end)
   {
      char * b = reinterpret_cast<char*>(const_cast<unsigned char*>(begin));
      char * e = reinterpret_cast<char*>(const_cast<unsigned char*>(end));
      return const_cast<const unsigned char*>(reinterpret_cast<unsigned char*>(const_cast<char*>(first_non_repeated_char(b,e))));
   }

   inline std::size_t first_non_repeated_char(const std::string& str)
   {
      if (str.empty())
         return static_cast<std::size_t>(std::string::npos);
      const char* itr = first_non_repeated_char(str.data(),str.data() + str.size());
      if ((str.data() + str.size()) != itr)
         return static_cast<std::size_t>(itr - str.data());
      else
         return static_cast<std::size_t>(std::string::npos);
   }

   inline void convert_bin_to_hex(const unsigned char* begin, const unsigned char* end, unsigned char* out)
   {
      static const unsigned short hex_lut[] =
                                  {
                                     0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530, 0x3630, 0x3730,
                                     0x3830, 0x3930, 0x4130, 0x4230, 0x4330, 0x4430, 0x4530, 0x4630,
                                     0x3031, 0x3131, 0x3231, 0x3331, 0x3431, 0x3531, 0x3631, 0x3731,
                                     0x3831, 0x3931, 0x4131, 0x4231, 0x4331, 0x4431, 0x4531, 0x4631,
                                     0x3032, 0x3132, 0x3232, 0x3332, 0x3432, 0x3532, 0x3632, 0x3732,
                                     0x3832, 0x3932, 0x4132, 0x4232, 0x4332, 0x4432, 0x4532, 0x4632,
                                     0x3033, 0x3133, 0x3233, 0x3333, 0x3433, 0x3533, 0x3633, 0x3733,
                                     0x3833, 0x3933, 0x4133, 0x4233, 0x4333, 0x4433, 0x4533, 0x4633,
                                     0x3034, 0x3134, 0x3234, 0x3334, 0x3434, 0x3534, 0x3634, 0x3734,
                                     0x3834, 0x3934, 0x4134, 0x4234, 0x4334, 0x4434, 0x4534, 0x4634,
                                     0x3035, 0x3135, 0x3235, 0x3335, 0x3435, 0x3535, 0x3635, 0x3735,
                                     0x3835, 0x3935, 0x4135, 0x4235, 0x4335, 0x4435, 0x4535, 0x4635,
                                     0x3036, 0x3136, 0x3236, 0x3336, 0x3436, 0x3536, 0x3636, 0x3736,
                                     0x3836, 0x3936, 0x4136, 0x4236, 0x4336, 0x4436, 0x4536, 0x4636,
                                     0x3037, 0x3137, 0x3237, 0x3337, 0x3437, 0x3537, 0x3637, 0x3737,
                                     0x3837, 0x3937, 0x4137, 0x4237, 0x4337, 0x4437, 0x4537, 0x4637,
                                     0x3038, 0x3138, 0x3238, 0x3338, 0x3438, 0x3538, 0x3638, 0x3738,
                                     0x3838, 0x3938, 0x4138, 0x4238, 0x4338, 0x4438, 0x4538, 0x4638,
                                     0x3039, 0x3139, 0x3239, 0x3339, 0x3439, 0x3539, 0x3639, 0x3739,
                                     0x3839, 0x3939, 0x4139, 0x4239, 0x4339, 0x4439, 0x4539, 0x4639,
                                     0x3041, 0x3141, 0x3241, 0x3341, 0x3441, 0x3541, 0x3641, 0x3741,
                                     0x3841, 0x3941, 0x4141, 0x4241, 0x4341, 0x4441, 0x4541, 0x4641,
                                     0x3042, 0x3142, 0x3242, 0x3342, 0x3442, 0x3542, 0x3642, 0x3742,
                                     0x3842, 0x3942, 0x4142, 0x4242, 0x4342, 0x4442, 0x4542, 0x4642,
                                     0x3043, 0x3143, 0x3243, 0x3343, 0x3443, 0x3543, 0x3643, 0x3743,
                                     0x3843, 0x3943, 0x4143, 0x4243, 0x4343, 0x4443, 0x4543, 0x4643,
                                     0x3044, 0x3144, 0x3244, 0x3344, 0x3444, 0x3544, 0x3644, 0x3744,
                                     0x3844, 0x3944, 0x4144, 0x4244, 0x4344, 0x4444, 0x4544, 0x4644,
                                     0x3045, 0x3145, 0x3245, 0x3345, 0x3445, 0x3545, 0x3645, 0x3745,
                                     0x3845, 0x3945, 0x4145, 0x4245, 0x4345, 0x4445, 0x4545, 0x4645,
                                     0x3046, 0x3146, 0x3246, 0x3346, 0x3446, 0x3546, 0x3646, 0x3746,
                                     0x3846, 0x3946, 0x4146, 0x4246, 0x4346, 0x4446, 0x4546, 0x4646
                                  };

      for (const unsigned char* itr = begin; end != itr; ++itr)
      {
         *reinterpret_cast<unsigned short*>(out) = hex_lut[(*itr)];
         out += sizeof(unsigned short);
      }
   }

   inline void convert_bin_to_hex(const char* begin, const char* end, char* out)
   {
      convert_bin_to_hex(reinterpret_cast<const unsigned char*>(begin),
                         reinterpret_cast<const unsigned char*>(end),
                         reinterpret_cast<unsigned char*>(out));
   }

   inline void convert_bin_to_hex(const std::pair<unsigned char*,unsigned char*>& r, unsigned char* out)
   {
      convert_bin_to_hex(r.first,r.second,out);
   }

   inline void convert_bin_to_hex(const std::pair<const unsigned char*,const unsigned char*>& r, unsigned char* out)
   {
      convert_bin_to_hex(r.first,r.second,out);
   }

   inline void convert_bin_to_hex(const std::pair<const char*,const char*>& r, char* out)
   {
      convert_bin_to_hex(r.first,r.second,out);
   }

   inline void convert_bin_to_hex(const std::string& binary_data, std::string& output)
   {
      output.resize(binary_data.size() * 2);
      convert_bin_to_hex(binary_data.data(),
                         binary_data.data() + binary_data.size(),
                         const_cast<char*>(output.data()));
   }

   inline std::string convert_bin_to_hex(const std::string& binary_data)
   {
      std::string output;
      convert_bin_to_hex(binary_data,output);
      return output;
   }

   inline bool convert_hex_to_bin(const unsigned char* begin, const unsigned char* end, unsigned char* out)
   {
      const std::size_t length = std::distance(begin,end);
      if (0 == length)
         return false;
      else if (1 == (length % 2))
         return false;
      static const unsigned char hex_to_bin[] =
                                 {
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00 - 0x07
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x08 - 0x0F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x10 - 0x17
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x18 - 0x1F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x20 - 0x27
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x28 - 0x2F
                                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x30 - 0x37
                                    0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x38 - 0x3F
                                    0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, // 0x40 - 0x47
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x48 - 0x4F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50 - 0x57
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x58 - 0x5F
                                    0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, // 0x60 - 0x67
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x68 - 0x6F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70 - 0x77
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x78 - 0x7F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 - 0x87
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x88 - 0x8F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 - 0x97
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x98 - 0x9F
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0 - 0xA7
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA8 - 0xAF
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0 - 0xB7
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB8 - 0xBF
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0 - 0xC7
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC8 - 0xCF
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0 - 0xD7
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD8 - 0xDF
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0 - 0xE7
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE8 - 0xEF
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xF0 - 0xF7
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xF8 - 0xFF
                                 };

      const unsigned char* itr = begin;
      while (end != itr)
      {
         *reinterpret_cast<unsigned char*>(out) = static_cast<unsigned char>(hex_to_bin[itr[0]] << 4 | hex_to_bin[itr[1]]);
         ++out;
         itr += 2;
      }
      return true;
   }

   inline bool convert_hex_to_bin(const char* begin, const char* end, char* out)
   {
      return convert_hex_to_bin(reinterpret_cast<const unsigned char*>(begin),
                                reinterpret_cast<const unsigned char*>(end),
                                reinterpret_cast<unsigned char*>(out));
   }

   inline bool convert_hex_to_bin(const std::pair<unsigned char*,unsigned char*>& r, unsigned char* out)
   {
      return convert_hex_to_bin(r.first,r.second,out);
   }

   inline bool convert_hex_to_bin(const std::pair<const unsigned char*,const unsigned char*>& r, unsigned char* out)
   {
      return convert_hex_to_bin(r.first,r.second,out);
   }

   inline bool convert_hex_to_bin(const std::pair<char*,char*>& r, char* out)
   {
      return convert_hex_to_bin(r.first,r.second,out);
   }

   inline bool convert_hex_to_bin(const std::pair<const char*,const char*>& r, char* out)
   {
      return convert_hex_to_bin(r.first,r.second,out);
   }

   inline bool convert_hex_to_bin(const std::string& hex_data, std::string& output)
   {
      if (hex_data.empty() || (1 == (hex_data.size() % 2)))
         return false;
      output.resize(hex_data.size() >> 1);
      return convert_hex_to_bin(hex_data.data(),
                                hex_data.data() + hex_data.size(),
                                const_cast<char*>(output.data()));
   }

   inline std::size_t convert_bin_to_base64(const unsigned char* begin, const unsigned char* end, unsigned char* out)
   {
      static const unsigned char bin_to_base64 [] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

      const std::size_t length = std::distance(begin,end);
      std::size_t rounds = length / 3;
      const unsigned char* itr = begin;
      for (std::size_t i = 0; i < rounds; ++i)
      {
         unsigned int block  = *(itr++) << 16;
                      block |= *(itr++) <<  8;
                      block |= *(itr++)      ;
         *(out++) = bin_to_base64[( block >> 18 ) & 0x3F];
         *(out++) = bin_to_base64[( block >> 12 ) & 0x3F];
         *(out++) = bin_to_base64[( block >>  6 ) & 0x3F];
         *(out++) = bin_to_base64[( block       ) & 0x3F];
      }

      if ((rounds = (length % 3)) > 0)
      {
         switch (rounds)
         {
            case 1 : {
                       unsigned int block  = (unsigned char) (*itr) << 16;
                       *(out++) = bin_to_base64[( block >> 18 ) & 0x3F];
                       *(out++) = bin_to_base64[( block >> 12 ) & 0x3F];
                       *(out++) = '=';
                       *(out++) = '=';
                     }
                     break;

            case 2 : {
                        unsigned int block  = *(itr++) << 16;
                                     block |= *(itr++) <<  8;
                        *(out++) = bin_to_base64[( block >> 18 ) & 0x3F];
                        *(out++) = bin_to_base64[( block >> 12 ) & 0x3F];
                        *(out++) = bin_to_base64[( block >>  6 ) & 0x3F];
                        *(out++) = '=';
                     }
                     break;
         }
      }
      return static_cast<std::size_t>((length / 3) * 4) + ((length % 3) > 0 ? 4 : 0);
   }

   inline std::size_t convert_bin_to_base64(const char* begin, const char* end, char* out)
   {
      return convert_bin_to_base64(reinterpret_cast<const unsigned char*>(begin),
                                   reinterpret_cast<const unsigned char*>(end),
                                   reinterpret_cast<unsigned char*>(out));
   }

   inline void convert_bin_to_base64(const std::string& binary_data, std::string& output)
   {
      output.resize(std::max<std::size_t>(4,binary_data.size() << 1));
      std::size_t resize = convert_bin_to_base64(binary_data.data(),
                                                 binary_data.data() + binary_data.size(),
                                                 const_cast<char*>(output.data()));
      output.resize(resize);
   }

   inline std::size_t convert_base64_to_bin(const unsigned char* begin, const unsigned char* end, unsigned char* out)
   {
      static const unsigned char base64_to_bin[] =
                                 {
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x00 - 0x07
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x08 - 0x0F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x10 - 0x17
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x18 - 0x1F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x20 - 0x27
                                    0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F, // 0x28 - 0x2F
                                    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, // 0x30 - 0x37
                                    0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x38 - 0x3F
                                    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // 0x40 - 0x47
                                    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, // 0x48 - 0x4F
                                    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, // 0x50 - 0x57
                                    0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x58 - 0x5F
                                    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, // 0x60 - 0x67
                                    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, // 0x68 - 0x6F
                                    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, // 0x70 - 0x77
                                    0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x78 - 0x7F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x80 - 0x87
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x88 - 0x8F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x90 - 0x97
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x98 - 0x9F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xA0 - 0xA7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xA8 - 0xAF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xB0 - 0xB7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xB8 - 0xBF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xC0 - 0xC7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xC8 - 0xCF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xD0 - 0xD7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xD8 - 0xDF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xE0 - 0xE7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xE8 - 0xEF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xF0 - 0xF7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 0xF8 - 0xFF
                                 };

      const unsigned char* end_itr = end;

      if ('=' == *(end - 2))
         end_itr = end - 2;
      else if ('=' == *(end - 1))
         end_itr = end - 1;

      const std::size_t length = std::distance(begin,end_itr);
      const std::size_t rounds = length / 4;
      const unsigned char* itr = begin;

      for (std::size_t i = 0; i < rounds; ++i)
      {
         unsigned int block  = base64_to_bin[*(itr++)] << 18;
                      block |= base64_to_bin[*(itr++)] << 12;
                      block |= base64_to_bin[*(itr++)] <<  6;
                      block |= base64_to_bin[*(itr++)];

         *(out++) = static_cast<unsigned char>(( block >> 16 ) & 0xFF);
         *(out++) = static_cast<unsigned char>(( block >>  8 ) & 0xFF);
         *(out++) = static_cast<unsigned char>(( block       ) & 0xFF);
      }

      const std::size_t remainder = (length % 4);
      if (remainder > 0)
      {
         switch (remainder)
         {
            case 2 : {
                        unsigned int block  = base64_to_bin[*(itr++)] << 18;
                                     block |= base64_to_bin[*(itr++)] << 12;
                        (*out) = static_cast<unsigned char>(( block >> 16 ) & 0xFF);
                     }
                     break;

            case 3 : {
                        unsigned int block  = base64_to_bin[*(itr++)] << 18;
                                     block |= base64_to_bin[*(itr++)] << 12;
                                     block |= base64_to_bin[*(itr++)] <<  6;
                        *(out++) = static_cast<unsigned char>(( block >> 16 ) & 0xFF);
                        *(out  ) = static_cast<unsigned char>(( block >>  8 ) & 0xFF);
                     }
                     break;
         }
      }

      return static_cast<std::size_t>((3 * length) / 4);
   }

   inline std::size_t convert_base64_to_bin(const char* begin, const char* end, char* out)
   {
      return convert_base64_to_bin(reinterpret_cast<const unsigned char*>(begin),
                                   reinterpret_cast<const unsigned char*>(end),
                                   reinterpret_cast<unsigned char*>(out));
   }

   inline void convert_base64_to_bin(const std::string& binary_data, std::string& output)
   {
      output.resize(binary_data.size());
      std::size_t resize = convert_base64_to_bin(binary_data.data(),
                                                 binary_data.data() + binary_data.size(),
                                                 const_cast<char*>(output.data()));
      output.resize(resize);
   }

   inline void convert_to_printable_chars(unsigned char* begin, unsigned char* end)
   {
      static const unsigned char printable_char_table[] =
                                 {
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x00 - 0x07
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x08 - 0x0F
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x10 - 0x17
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x18 - 0x1F
                                    0x2E, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, // 0x20 - 0x27
                                    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0x28 - 0x2F
                                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, // 0x30 - 0x37
                                    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 0x38 - 0x3F
                                    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, // 0x40 - 0x47
                                    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 0x48 - 0x4F
                                    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, // 0x50 - 0x57
                                    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 0x58 - 0x5F
                                    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, // 0x60 - 0x67
                                    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 0x68 - 0x6F
                                    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, // 0x70 - 0x77
                                    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x2E, // 0x78 - 0x7F
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x80 - 0x87
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x88 - 0x8F
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x90 - 0x97
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0x98 - 0x9F
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xA0 - 0xA7
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xA8 - 0xAF
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xB0 - 0xB7
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xB8 - 0xBF
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xC0 - 0xC7
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xC8 - 0xCF
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xD0 - 0xD7
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xD8 - 0xDF
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xE0 - 0xE7
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xE8 - 0xEF
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, // 0xF0 - 0xF7
                                    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E  // 0xF8 - 0xFF
                                 };
      unsigned char* itr = begin;
      while (end != itr)
      {
         (*itr) = printable_char_table[static_cast<unsigned int>((*itr))];
         ++itr;
      }
   }

   inline void convert_to_printable_chars(char* begin, char* end)
   {
      convert_to_printable_chars(reinterpret_cast<unsigned char*>(begin),
                                 reinterpret_cast<unsigned char*>(end));
   }

   inline void convert_to_printable_chars(std::string& str)
   {
      convert_to_printable_chars(reinterpret_cast<unsigned char*>(const_cast<char*>(str.data())),
                                 reinterpret_cast<unsigned char*>(const_cast<char*>(str.data() + str.size())));
   }

   inline void convert_to_uppercase(unsigned char* begin, unsigned char* end)
   {
      std::transform(begin,end,begin,::toupper);
      /*
      unsigned char* itr = begin;
      while (end != itr)
      {
         //(*itr) = std::toupper((*itr), std::locale::classic());
         (*itr) = static_cast<unsigned char>(::toupper(static_cast<int>(*itr)));
         ++itr;
      }
      */
   }

   inline void convert_to_uppercase(char* begin, char* end)
   {
      convert_to_uppercase(reinterpret_cast<unsigned char*>(begin),
                           reinterpret_cast<unsigned char*>(end));
   }

   inline void convert_to_uppercase(std::string& str)
   {
      convert_to_uppercase(reinterpret_cast<unsigned char*>(const_cast<char*>(str.data())),
                           reinterpret_cast<unsigned char*>(const_cast<char*>(str.data() + str.size())));
   }

   inline void convert_to_lowercase(unsigned char* begin, unsigned char* end)
   {
      std::transform(begin,end,begin,::tolower);
      /*
      unsigned char* itr = begin;
      while (end != itr)
      {
         //(*itr) = std::tolower((*itr), std::locale::classic());
         (*itr) = static_cast<unsigned char>(::tolower(static_cast<int>(*itr)));
         ++itr;
      }
      */
   }

   inline void convert_to_lowercase(char* begin, char* end)
   {
      convert_to_lowercase(reinterpret_cast<unsigned char*>(begin),
                           reinterpret_cast<unsigned char*>(end));
   }

   inline void convert_to_lowercase(const char* begin, const char* end)
   {
      convert_to_lowercase(const_cast<char*>(begin),const_cast<char*>(end));
   }

   inline void convert_to_lowercase(std::string& str)
   {
      convert_to_lowercase(reinterpret_cast<unsigned char*>(const_cast<char*>(str.data())),
                           reinterpret_cast<unsigned char*>(const_cast<char*>(str.data() + str.size())));
   }

   inline std::string as_lowercase(const std::string& str)
   {
      std::string result = str;
      convert_to_lowercase(result);
      return result;
   }

   inline std::string as_uppercase(const std::string& str)
   {
      std::string result = str;
      convert_to_uppercase(result);
      return result;
   }

   inline bool twoway_bitwise_interleave(const unsigned char* begin1, const unsigned char* end1,
                                         const unsigned char* begin2, const unsigned char* end2,
                                         unsigned char* out)
   {
      if (std::distance(begin1,end1) != std::distance(begin2,end2))
      {
         return false;
      }

      static const std::size_t interleave_table_size = 256;
      static const unsigned short interleave_table[interleave_table_size] =
                                  {
                                     0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, // 0x00 - 0x07
                                     0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055, // 0x08 - 0x0F
                                     0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, // 0x10 - 0x17
                                     0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155, // 0x18 - 0x1F
                                     0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, // 0x20 - 0x27
                                     0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455, // 0x28 - 0x2F
                                     0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, // 0x30 - 0x37
                                     0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, // 0x38 - 0x3F
                                     0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, // 0x40 - 0x47
                                     0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, // 0x48 - 0x4F
                                     0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, // 0x50 - 0x57
                                     0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, // 0x58 - 0x5F
                                     0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, // 0x60 - 0x67
                                     0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, // 0x68 - 0x6F
                                     0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, // 0x70 - 0x77
                                     0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, // 0x78 - 0x7F
                                     0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, // 0x80 - 0x87
                                     0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, // 0x88 - 0x8F
                                     0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, // 0x90 - 0x97
                                     0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, // 0x98 - 0x9F
                                     0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, // 0xA0 - 0xA7
                                     0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, // 0xA8 - 0xAF
                                     0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, // 0xB0 - 0xB7
                                     0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, // 0xB8 - 0xBF
                                     0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, // 0xC0 - 0xC7
                                     0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, // 0xC8 - 0xCF
                                     0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, // 0xD0 - 0xD7
                                     0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, // 0xD8 - 0xDF
                                     0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, // 0xE0 - 0xE7
                                     0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, // 0xE8 - 0xEF
                                     0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, // 0xF0 - 0xF7
                                     0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555  // 0xF8 - 0xFF
                                  };

      const unsigned char* itr1 = begin1;
      const unsigned char* itr2 = begin2;
      while (end1 != itr1)
      {
         *(reinterpret_cast<unsigned short*>(out))  = (interleave_table[*(itr2++)] << 1);
         *(reinterpret_cast<unsigned short*>(out)) |=  interleave_table[*(itr1++)];
         out += 2;
      }
      return true;
   }

   inline bool twoway_bitwise_interleave(const char* begin1, const char* end1,
                                         const char* begin2, const char* end2,
                                         char* out)
   {
      return twoway_bitwise_interleave(reinterpret_cast<const unsigned char*>(begin1),
                                       reinterpret_cast<const unsigned char*>(end1),
                                       reinterpret_cast<const unsigned char*>(begin2),
                                       reinterpret_cast<const unsigned char*>(end2),
                                       reinterpret_cast<unsigned char*>(out));
   }

   inline bool twoway_bitwise_interleave(const std::string& str1,
                                         const std::string& str2,
                                         std::string& out)
   {
      if (str1.size() != str2.size())
      {
         return false;
      }
      out.resize(str1.size());
      return twoway_bitwise_interleave(str1.data(),str1.data() + str1.size(),
                                       str2.data(),str2.data() + str2.size(),
                                       const_cast<char*>(out.data()));
   }

   template <std::size_t n>
   struct interleave_ary;

   template<> struct interleave_ary<sizeof(unsigned short)>         { typedef unsigned short         type; };
   template<> struct interleave_ary<sizeof(unsigned int  )>         { typedef unsigned int           type; };
   template<> struct interleave_ary<sizeof(unsigned long long int)> { typedef unsigned long long int type; };

   template <std::size_t n>
   inline void create_nway_interleave_table(typename interleave_ary<n>::type table[256])
   {
      typedef typename interleave_ary<n>::type type;
      const type diff = static_cast<type>(n - 1);
      for (type i = static_cast<type>(0); i < static_cast<type>(256); ++i)
      {
         table[i] = 0x00;
         for (type j = static_cast<type>(0); j < static_cast<type>(8); ++j)
         {
            table[i] |= (i & (1 << j)) << (j * diff);
         }
      }
   }

   namespace bitwise_operation { enum type { eAND, eOR, eXOR }; }

   inline void bitwise_transform(const bitwise_operation::type& operation,
                                 const unsigned char* begin1, const unsigned char* end1,
                                 const unsigned char* begin2,
                                 unsigned char* out)
   {
      const unsigned char* itr1 = begin1;
      const unsigned char* itr2 = begin2;

      switch (operation)
      {
         case bitwise_operation::eAND : while (itr1 != end1) { *(out++) = *(itr1++) & *(itr2++); } return;
         case bitwise_operation::eOR  : while (itr1 != end1) { *(out++) = *(itr1++) | *(itr2++); } return;
         case bitwise_operation::eXOR : while (itr1 != end1) { *(out++) = *(itr1++) ^ *(itr2++); } return;
      }
   }

   inline void bitwise_transform(const bitwise_operation::type& operation,
                                 const char* begin1, const char* end1,
                                 const char* begin2,
                                 char* out)
   {
      bitwise_transform(operation,
                        reinterpret_cast<const unsigned char*>(begin1),
                        reinterpret_cast<const unsigned char*>(end1),
                        reinterpret_cast<const unsigned char*>(begin2),
                        reinterpret_cast<unsigned char*>(out));
   }

   inline void bitwise_transform(const bitwise_operation::type& operation,
                                 const std::string& str1,
                                 const std::string& str2,
                                 std::string& out)
   {
      if (str1.size() != str2.size()) return;
      out.resize(str1.size());
      bitwise_transform(operation,
                        str1.data(),str1.data() + str1.size(),
                        str2.data(),
                        const_cast<char*>(out.data()));
   }

   inline std::size_t high_bit_count(const unsigned char c)
   {
      static const std::size_t high_bits_in_char[256] =
                               {
                                  0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
                                  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                                  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                                  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                                  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                                  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                                  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                                  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                                  1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                                  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                                  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                                  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                                  2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                                  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                                  3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                                  4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
                               };
      return high_bits_in_char[c];
   }

   inline std::size_t high_bit_count(const unsigned short& s)
   {
      const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&s);
      return high_bit_count(*(ptr + 0)) + high_bit_count(*(ptr + 1));
   }

   inline std::size_t high_bit_count(const unsigned int& i)
   {
      const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&i);
      return high_bit_count(*(ptr + 0)) + high_bit_count(*(ptr + 1)) +
             high_bit_count(*(ptr + 2)) + high_bit_count(*(ptr + 3));
   }

   inline std::size_t high_bit_count(const long long int& ll)
   {
      const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&ll);
      return high_bit_count(*(ptr + 0)) + high_bit_count(*(ptr + 1)) +
             high_bit_count(*(ptr + 2)) + high_bit_count(*(ptr + 3)) +
             high_bit_count(*(ptr + 4)) + high_bit_count(*(ptr + 5)) +
             high_bit_count(*(ptr + 6)) + high_bit_count(*(ptr + 7));
   }

   inline std::size_t high_bit_count(const unsigned char* begin, const unsigned char* end)
   {
      std::size_t count = 0;
      const unsigned char* itr = begin;
      while (end != itr)
      {
         count += high_bit_count(*itr++);
      }
      return count;
   }

   inline std::size_t high_bit_count(const char* begin, const char* end)
   {
      return high_bit_count(reinterpret_cast<const unsigned char*>(begin),
                            reinterpret_cast<const unsigned char*>(end));

   }

   inline std::size_t high_bit_count(const std::string& str)
   {
      return high_bit_count(str.data(),str.data() + str.size());
   }

   inline bool bit_state(const std::size_t& index, const unsigned char* ptr)
   {
      static const unsigned char bit_mask[] =
                                 {
                                    0x01, //00000001
                                    0x02, //00000010
                                    0x04, //00000100
                                    0x08, //00001000
                                    0x10, //00010000
                                    0x20, //00100000
                                    0x40, //01000000
                                    0x80  //10000000
                                 };
      return (0 != (ptr[(index >> 3)] & bit_mask[index & 7]));
   }

   inline void set_bit_high(const std::size_t& index, unsigned char* const ptr)
   {
      static const unsigned char bit_mask[] =
                                 {
                                    0x01, //00000001
                                    0x02, //00000010
                                    0x04, //00000100
                                    0x08, //00001000
                                    0x10, //00010000
                                    0x20, //00100000
                                    0x40, //01000000
                                    0x80  //10000000
                                 };
      ptr[(index >> 3)] |= bit_mask[index & 7];
   }

   inline void set_bit_low(const std::size_t& index, unsigned char* const ptr)
   {
      static const unsigned char bit_mask[] =
                                 {
                                    0xFE, //11111110
                                    0xFD, //11111101
                                    0xFB, //11111011
                                    0xF7, //11110111
                                    0xEF, //11101111
                                    0xDF, //11011111
                                    0xBF, //10111111
                                    0x7F  //01111111
                                 };
      ptr[(index >> 3)] &= bit_mask[index & 7];
   }

   inline std::size_t hamming_distance(const unsigned char* begin1, const unsigned char* end1,
                                       const unsigned char* begin2, const unsigned char* end2)
   {
      if (std::distance(begin1,end1) != std::distance(begin2,end2))
      {
         return std::numeric_limits<std::size_t>::max();
      }

      std::size_t distance = 0;
      const unsigned char* itr1 = begin1;
      const unsigned char* itr2 = begin2;

      while (end1 != itr1)
      {
         distance += high_bit_count(static_cast<unsigned char>(((*itr1++) ^ (*itr2++)) & 0xFF));
      }

      return distance;
   }

   inline std::size_t hamming_distance(const char* begin1, const char* end1,
                                       const char* begin2, const char* end2)
   {
      return hamming_distance(reinterpret_cast<const unsigned char*>(begin1),
                              reinterpret_cast<const unsigned char*>(end1),
                              reinterpret_cast<const unsigned char*>(begin2),
                              reinterpret_cast<const unsigned char*>(end2));
   }

   inline std::size_t hamming_distance(const std::string& str1, const std::string& str2)
   {
      return hamming_distance(str1.data(), str1.data() + str1.size(),
                              str2.data(), str2.data() + str2.size());
   }

   template <typename Iterator>
   inline std::size_t hamming_distance_elementwise(const Iterator begin1, const Iterator end1,
                                                   const Iterator begin2, const Iterator end2)
   {
      if (std::distance(begin1,end1) != std::distance(begin2,end2))
      {
         return std::numeric_limits<std::size_t>::max();
      }

      std::size_t distance = 0;
      Iterator itr1 = begin1;
      Iterator itr2 = begin2;

      while (end1 != itr1)
      {
         if ((*itr1) != (*itr2))
            ++distance;
      }

      return distance;
   }

   inline std::size_t hamming_distance_elementwise(const std::string& str1, const std::string& str2)
   {
      return hamming_distance_elementwise(str1.data(), str1.data() + str1.size(),
                                          str2.data(), str2.data() + str2.size());
   }

   class token_grid
   {
   public:

      typedef const unsigned char* iterator_t;
      typedef unsigned int index_t;
      typedef std::pair<iterator_t,iterator_t> range_t;
      typedef std::deque<range_t> token_list_t;
      typedef std::pair<index_t,index_t> row_index_range_t;
      typedef std::deque<row_index_range_t> row_index_t;
      typedef std::pair<index_t,index_t> row_range_t;
      typedef std::pair<index_t,index_t> col_range_t;

   private:

      struct store
      {
         store()
         : max_column(0)
         {}

         token_list_t token_list;
         row_index_t   row_index;
         std::size_t  max_column;

         inline void clear()
         {
            token_list.clear();
            row_index.clear();
         }

         inline range_t operator()(const std::size_t& col, const std::size_t& row) const
         {
            if (row < row_index.size())
            {
               const row_index_range_t& r = row_index[row];
               if (col < (r.second - r.first + 1))
                  return *(token_list.begin() + (r.first + col));
               else
                  return null_range();
            }
            else
               return null_range();
         }

         inline bool remove_row(const std::size_t& row)
         {
            if (row >= row_index.size()) return false;
            row_index_range_t& r = row_index[row];
            std::size_t number_of_tokens = r.second - r.first + 1;
            token_list_t::iterator remove_begin = token_list.begin() + r.first;
            token_list_t::iterator remove_end = token_list.begin() + r.first + number_of_tokens;
            token_list.erase(remove_begin,remove_end);
            row_index.erase(row_index.begin() + row);
            for (std::size_t i = row; i < row_index.size(); ++i)
            {
               row_index_range_t& r = row_index[i];
               r.first  -= static_cast<unsigned int>(number_of_tokens);
               r.second -= static_cast<unsigned int>(number_of_tokens);
            }
            return true;
         }

         inline std::size_t token_count(const row_index_range_t& r) const
         {
            return (r.second - r.first + 1);
         }

         inline std::size_t token_count(const std::size_t& index) const
         {
            return token_count(row_index[index]);
         }

         inline bool remove_row_range(const std::size_t& r0, const std::size_t& r1)
         {
            if (r0 > r1)
               return false;
            else if (r0 >= row_index.size())
               return false;
            else if (r1 >= row_index.size())
               return false;
            std::size_t number_of_tokens = 0;
            for (std::size_t i = r0; i <= r1; ++i)
            {
               row_index_range_t& r = row_index[i];
               number_of_tokens += token_count(r);
            }
            row_index_range_t rr0 = row_index[r0];
            token_list_t::iterator remove_begin = token_list.begin() + rr0.first;
            token_list_t::iterator remove_end = token_list.begin() + rr0.first + number_of_tokens;
            token_list.erase(remove_begin,remove_end);
            row_index.erase(row_index.begin() + r0,row_index.begin() + r0 + (r1 - r0 + 1));
            for (std::size_t i = r0; i < row_index.size(); ++i)
            {
               row_index_range_t& r = row_index[i];
               r.first  -= static_cast<unsigned int>(number_of_tokens);
               r.second -= static_cast<unsigned int>(number_of_tokens);
            }
            return true;
         }

         struct remove_column_impl
         {
            std::size_t column;
            std::size_t counter;
            std::size_t remainder;
            std::size_t current_row;

            inline void update(store& idx)
            {
               current_row++;
               while (current_row < idx.row_index.size())
               {
                  std::size_t number_of_tokens = idx.token_count(current_row);
                  if (number_of_tokens > column)
                     break;
                  counter += number_of_tokens;
                  ++current_row;
               }
               if (current_row < idx.row_index.size())
               {
                  counter += column + remainder;
                  row_index_range_t& r = idx.row_index[current_row];
                  remainder = (r.second - r.first) - column;
               }
               else
                  counter = std::numeric_limits<std::size_t>::max();
            }

            inline void process(store& idx)
            {
               token_list_t::iterator itr1 = idx.token_list.begin();
               token_list_t::iterator itr2 = idx.token_list.begin();
               token_list_t::iterator end  = idx.token_list.end();
               counter = 0;
               remainder = 0;
               current_row = static_cast<std::size_t>(-1);
               update(idx);
               while (end != itr1)
               {
                  while ((end != itr1) && (0 != counter))
                  {
                     if (itr1 != itr2)
                     {
                        (*itr2) = (*itr1);
                     }
                     ++itr1;
                     ++itr2;
                     --counter;
                  }
                  if (0 == counter)
                  {
                     update(idx);
                     ++itr1;
                  }
               }
               std::size_t remove_count = 0;
               idx.max_column = std::numeric_limits<std::size_t>::min();
               for (std::size_t i = 0; i < idx.row_index.size(); ++i)
               {
                  row_index_range_t& r = idx.row_index[i];
                  std::size_t token_count = (r.second - r.first + 1);
                  r.first -= static_cast<unsigned int>(remove_count);
                  if (token_count > column)
                  {
                     ++remove_count;
                  }
                  r.second -= static_cast<unsigned int>(remove_count);
                  token_count = (r.second - r.first + 1);
                  if (token_count > idx.max_column)
                     idx.max_column = token_count;
               }
               idx.token_list.resize(idx.token_list.size() - remove_count);
            }
         };

         inline bool remove_column(const std::size_t& column)
         {
            if (column >= max_column) return false;
            remove_column_impl rc;
            rc.column = column;
            rc.process(*this);
            return true;
         }

         inline static range_t null_range()
         {
            static const range_t null_range_ = range_t(reinterpret_cast<const unsigned char*>(0),
                                                       reinterpret_cast<const unsigned char*>(0));
            return null_range_;
         }

      };

      template <typename DelimiterPredicate = multiple_char_delimiter_predicate>
      struct row_processor
      {
         row_processor(store& idx,
                       DelimiterPredicate& tp,
                       const split_options::type split_mode = split_options::compress_delimiters)
         : idx_(idx),
           row_start_index_(0),
           row_end_index_(0),
           token_predicate_(tp),
           split_mode_(split_mode)
         {
            idx_.max_column = std::numeric_limits<std::size_t>::min();
         }

         inline void operator()(const range_t& range)
         {
            if (0 == std::distance(range.first,range.second))
               return;
            row_start_index_ = static_cast<index_t>(idx_.token_list.size());
            std::size_t token_count = split(token_predicate_,
                                            range.first,range.second,
                                            std::back_inserter(idx_.token_list),
                                            split_mode_);
            row_end_index_ = row_start_index_ + token_count - 1;
            idx_.row_index.push_back(std::make_pair(row_start_index_,row_end_index_));
            if (token_count > idx_.max_column)
               idx_.max_column = token_count;
         }

         row_processor<DelimiterPredicate> operator=(const row_processor<DelimiterPredicate>&);

         store& idx_;
         index_t row_start_index_;
         index_t row_end_index_;
         DelimiterPredicate& token_predicate_;
         split_options::type split_mode_;
      };

   public:

      inline row_range_t range(std::size_t lower_bound,
                                  std::size_t upper_bound = std::numeric_limits<std::size_t>::max()) const
      {
         if (upper_bound == std::numeric_limits<std::size_t>::max())
         {
            upper_bound = dsv_index_.token_list.size();
         }
         else if (upper_bound > dsv_index_.token_list.size())
         {
            return row_range_t(std::numeric_limits<std::size_t>::max(),std::numeric_limits<std::size_t>::max());
         }
         else if (lower_bound > upper_bound)
         {
            return row_range_t(std::numeric_limits<std::size_t>::max(),std::numeric_limits<std::size_t>::max());
         }
         return row_range_t(lower_bound,upper_bound);
      }

      struct options
      {
         options()
         : row_split_option(split_options::compress_delimiters),
           column_split_option(split_options::compress_delimiters),
           row_delimiters("\n\r"),
           column_delimiters(",|;\t "),
           support_dquotes(false),
           trim_dquotes(false)
         {}

         options(split_options::type sro,
                 split_options::type sco,
                 const std::string& rd,
                 const std::string& cd,
                 const bool support_dq = false,
                 const bool trim_dq = false)
         : row_split_option(sro),
           column_split_option(sco),
           row_delimiters(rd),
           column_delimiters(cd),
           support_dquotes(support_dq),
           trim_dquotes(trim_dq)
         {}

         inline options& set_column_split_option(const split_options::type& option)
         {
            column_split_option = option;
            return *this;
         }

         inline options& set_row_split_option(const split_options::type& option)
         {
            row_split_option = option;
            return *this;
         }

         inline options& set_column_delimiters(const std::string& delimiters)
         {
            column_delimiters = delimiters;
            return *this;
         }

         inline options& set_row_delimiters(const std::string& delimiters)
         {
            row_delimiters = delimiters;
            return *this;
         }

         split_options::type row_split_option;
         split_options::type column_split_option;
         std::string row_delimiters;
         std::string column_delimiters;
         bool support_dquotes;
         bool trim_dquotes;
      };

      class row_type
      {
      private:

         typedef std::pair<bool,row_type*> row_pair_type;

      public:

         row_type()
         : index_(std::numeric_limits<std::size_t>::max()),
           size_(0)
         {}

         row_type(const std::size_t& index,
                  const store& dsv_index)
         : index_(index),
           size_ (dsv_index.token_count(index)),
           begin_(dsv_index.token_list.begin() +  dsv_index.row_index[index].first)
         {}

         inline bool is_null(const std::size_t& index) const
         {
            const range_t& range = *(begin_ + index);
            return (0 == std::distance(range.first,range.second));
         }

         template <typename T>
         inline T operator[](const std::size_t& index) const
         {
            const range_t& range = *(begin_ + index);
            return string_to_type_converter<T>(range.first,range.second);
         }

         template <typename T>
         inline T get(const std::size_t& index) const
         {
            return operator[]<T>(index);
         }

         inline col_range_t all_columns() const
         {
            return col_range_t(0,static_cast<index_t>(size()));
         }

         inline range_t range() const
         {
            return range_t((*begin_).first,(*(begin_ + (size_ - 1))).second);
         }

         inline range_t token(const std::size_t& index) const
         {
            return *(begin_ + index);
         }

         inline std::size_t index() const
         {
            return index_;
         }

         inline std::size_t size() const
         {
            return size_;
         }

         inline std::size_t raw_length() const
         {
            std::size_t result = 0;
            token_list_t::const_iterator itr = begin_;
            for (std::size_t i = 0; i < size_; ++i, ++itr)
            {
               const range_t& range = (*itr);
               result += std::distance(range.first,range.second);
            }
            return result;
         }

         inline std::size_t raw_length(const std::size_t& column_index) const
         {
            const range_t& range = *(begin_ + column_index);
            return std::distance(range.first,range.second);
         }

         inline std::string as_string() const
         {
            std::string result;
            result.reserve(std::distance(begin_->first,(begin_ + (size_ - 1))->second));
            token_list_t::const_iterator itr = begin_;
            for (std::size_t i = 0; i < size_; ++i, ++itr)
            {
               const range_t& range = (*itr);
               result.append(range.first,range.second);
            }
            return result;
         }

         inline void as_string(std::string& out) const
         {
            out = as_string();
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6, typename T7,
                   typename T8, typename T9>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      const std::size_t& col4, const std::size_t& col5,
                                      const std::size_t& col6, const std::size_t& col7,
                                      const std::size_t& col8, const std::size_t& col9,
                                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                                      T6& t6, T7& t7, T8& t8, T9& t9) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            if (!process(*(begin_ + col4),t4)) return false;
            if (!process(*(begin_ + col5),t5)) return false;
            if (!process(*(begin_ + col6),t6)) return false;
            if (!process(*(begin_ + col7),t7)) return false;
            if (!process(*(begin_ + col8),t8)) return false;
            if (!process(*(begin_ + col9),t9)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6, typename T7,
                   typename T8>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      const std::size_t& col4, const std::size_t& col5,
                                      const std::size_t& col6, const std::size_t& col7,
                                      const std::size_t& col8,
                                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                                      T6& t6, T7& t7, T8& t8) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            if (!process(*(begin_ + col4),t4)) return false;
            if (!process(*(begin_ + col5),t5)) return false;
            if (!process(*(begin_ + col6),t6)) return false;
            if (!process(*(begin_ + col7),t7)) return false;
            if (!process(*(begin_ + col8),t8)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6, typename T7>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      const std::size_t& col4, const std::size_t& col5,
                                      const std::size_t& col6, const std::size_t& col7,
                                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                                      T6& t6, T7& t7) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            if (!process(*(begin_ + col4),t4)) return false;
            if (!process(*(begin_ + col5),t5)) return false;
            if (!process(*(begin_ + col6),t6)) return false;
            if (!process(*(begin_ + col7),t7)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      const std::size_t& col4, const std::size_t& col5,
                                      const std::size_t& col6,
                                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                                      T6& t6) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            if (!process(*(begin_ + col4),t4)) return false;
            if (!process(*(begin_ + col5),t5)) return false;
            if (!process(*(begin_ + col6),t6)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      const std::size_t& col4, const std::size_t& col5,
                                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            if (!process(*(begin_ + col4),t4)) return false;
            if (!process(*(begin_ + col5),t5)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3, typename T4>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      const std::size_t& col4,
                                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            if (!process(*(begin_ + col4),t4)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2, const std::size_t& col3,
                                      T0& t0, T1& t1, T2& t2, T3& t3) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            if (!process(*(begin_ + col3),t3)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      const std::size_t& col2,
                                      T0& t0, T1& t1, T2& t2) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            if (!process(*(begin_ + col2),t2)) return false;
            return true;
         }

         template <typename T0, typename T1>
         inline bool parse_with_index(const std::size_t& col0, const std::size_t& col1,
                                      T0& t0, T1& t1) const
         {
            if (!process(*(begin_ + col0),t0)) return false;
            if (!process(*(begin_ + col1),t1)) return false;
            return true;
         }

         template <typename T>
         inline bool parse_with_index(const std::size_t& col, T& t) const
         {
            return process(*(begin_ + col),t);
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6, typename T7,
                   typename T8, typename T9 >
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3,
                           T4& t4, T5& t5, T6& t6, T7& t7,
                           T8& t8, T9& t9) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            if (!process(*(begin_ + 4),t4)) return false;
            if (!process(*(begin_ + 5),t5)) return false;
            if (!process(*(begin_ + 6),t6)) return false;
            if (!process(*(begin_ + 7),t7)) return false;
            if (!process(*(begin_ + 8),t8)) return false;
            if (!process(*(begin_ + 9),t9)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6, typename T7,
                   typename T8>
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3,
                           T4& t4, T5& t5, T6& t6, T7& t7,
                           T8& t8) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            if (!process(*(begin_ + 4),t4)) return false;
            if (!process(*(begin_ + 5),t5)) return false;
            if (!process(*(begin_ + 6),t6)) return false;
            if (!process(*(begin_ + 7),t7)) return false;
            if (!process(*(begin_ + 8),t8)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6, typename T7>
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3,
                           T4& t4, T5& t5, T6& t6, T7& t7) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            if (!process(*(begin_ + 4),t4)) return false;
            if (!process(*(begin_ + 5),t5)) return false;
            if (!process(*(begin_ + 6),t6)) return false;
            if (!process(*(begin_ + 7),t7)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5, typename T6>
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3,
                           T4& t4, T5& t5, T6& t6) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            if (!process(*(begin_ + 4),t4)) return false;
            if (!process(*(begin_ + 5),t5)) return false;
            if (!process(*(begin_ + 6),t6)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2, typename T3,
                   typename T4, typename T5>
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3,
                           T4& t4, T5& t5) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            if (!process(*(begin_ + 4),t4)) return false;
            if (!process(*(begin_ + 5),t5)) return false;
            return true;
         }

         template <typename T0, typename T1,
                   typename T2, typename T3,typename T4>
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3, T4& t4) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            if (!process(*(begin_ + 4),t4)) return false;
            return true;
         }

         template <typename T0, typename T1,
                   typename T2, typename T3>
         inline bool parse(T0& t0, T1& t1, T2& t2, T3& t3) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            if (!process(*(begin_ + 3),t3)) return false;
            return true;
         }

         template <typename T0, typename T1, typename T2>
         inline bool parse(T0& t0, T1& t1, T2& t2) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            if (!process(*(begin_ + 2),t2)) return false;
            return true;
         }

         template <typename T0, typename T1>
         inline bool parse(T0& t0, T1& t1) const
         {
            if (!process(*(begin_ + 0),t0)) return false;
            if (!process(*(begin_ + 1),t1)) return false;
            return true;
         }

         template <typename T0>
         inline bool parse(T0& t) const
         {
            return process(*begin_,t);
         }

         template <typename T, typename OutputIterator>
         inline void parse(OutputIterator out) const
         {
            token_list_t::const_iterator itr = begin_;
            const token_list_t::const_iterator end = begin_ + size_;
            while (end != itr)
            {
               const range_t& range = (*itr);
               *(out++) = string_to_type_converter<T>(range.first,range.second);
               ++itr;
            }
         }

         bool validate_column_range(const col_range_t& range) const
         {
            if ((range.first > size()) || (range.second > size()))
               return false;
            else if (range.first > range.second)
               return false;
            else
               return true;
         }

         col_range_t range(const std::size_t& lower_bound,
                           const std::size_t& upper_bound = std::numeric_limits<std::size_t>::max()) const
         {
            if (std::numeric_limits<std::size_t>::max() != upper_bound)
               return col_range_t(lower_bound,upper_bound);
            else
              return col_range_t(lower_bound,static_cast<index_t>(size()));
         }

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         inline bool parse(const col_range_t& range,
                           Sequence<T,Allocator>& sequence) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = (begin_ + range.first);
            token_list_t::const_iterator end = (begin_ + range.second);
            T t;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,t))
                  sequence.push_back(t);
               else
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline bool parse(const col_range_t& range,
                           std::set<T,Comparator,Allocator>& set) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = (begin_ + range.first);
            token_list_t::const_iterator end = (begin_ + range.second);
            T t;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,t))
                  set.insert(t);
               else
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline bool parse(const col_range_t& range,
                           std::multiset<T,Comparator,Allocator>& multiset) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = (begin_ + range.first);
            token_list_t::const_iterator end = (begin_ + range.second);
            T t;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,t))
                  multiset.insert(t);
               else
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T, typename Container>
         inline bool parse(const col_range_t& range,
                           std::queue<T,Container>& queue) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = (begin_ + range.first);
            token_list_t::const_iterator end = (begin_ + range.second);
            T t;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,t))
                  queue.push(t);
               else
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T, typename Container>
         inline bool parse(const col_range_t& range,
                           std::stack<T,Container>& stack) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = (begin_ + range.first);
            token_list_t::const_iterator end = (begin_ + range.second);
            T t;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,t))
                  stack.push(t);
               else
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T,
                   typename Container,
                   typename Comparator>
         inline bool parse(const col_range_t& range,
                           std::priority_queue<T,Container,Comparator>& priority_queue) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = (begin_ + range.first);
            token_list_t::const_iterator end = (begin_ + range.second);
            T t;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,t))
                  priority_queue.push(t);
               else
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         inline bool parse(Sequence<T,Allocator>& sequence) const
         {
            return parse(range(0),sequence);
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline bool parse(std::set<T,Comparator,Allocator>& set) const
         {
            return parse(range(0),set);
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline bool parse(std::multiset<T,Comparator,Allocator>& multiset) const
         {
            return parse(range(0),multiset);
         }

         template <typename T, typename Container>
         inline bool parse(std::queue<T,Container>& queue) const
         {
            return parse(range(0),queue);
         }

         template <typename T, typename Container>
         inline bool parse(std::stack<T,Container>& stack) const
         {
            return parse(range(0),stack);
         }

         template <typename T,
                   typename Container,
                   typename Comparator>
         inline bool parse(std::priority_queue<T,Container,Comparator>& priority_queue) const
         {
            return parse(range(0),priority_queue);
         }

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         inline std::size_t parse_n(const std::size_t& n, Sequence<T,Allocator>& sequence) const
         {
            if (0 == n) return 0;
            T t;
            std::size_t count = 0;
            token_list_t::const_iterator itr = begin_;
            const token_list_t::const_iterator end = begin_ + size_;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (!string_to_type_converter(range.first,range.second,t))
                  return false;
               else
                  sequence.push_back(t);
               if (n == (++count))
                  break;
               ++itr;
            }
            return count;
         }

         template <typename T, typename OutputIterator>
         inline void parse_checked(OutputIterator out) const
         {
            T value;
            token_list_t::const_iterator itr = begin_;
            const token_list_t::const_iterator end = begin_ + size_;
            while (end != itr)
            {
               const range_t& range = (*itr);
               if (string_to_type_converter(range.first,range.second,value))
               {
                  *(out++) = value;
               }
               ++itr;
            }
         }

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         inline void parse_checked(Sequence<T,Allocator>& sequence) const
         {
            parse_checked<T>(std::back_inserter(sequence));
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline void parse_checked(std::set<T,Comparator,Allocator>& set) const
         {
            parse_checked<T>(std::inserter(set,set.end()));
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline void parse_checked(std::multiset<T,Comparator,Allocator>& multiset) const
         {
            parse_checked<T>(std::inserter(multiset,multiset.end()));
         }

         template <typename T, typename Container>
         inline void parse_checked(std::queue<T,Container>& queue) const
         {
            parse_checked<T>(push_inserter(queue));
         }

         template <typename T, typename Container>
         inline void parse_checked(std::stack<T,Container>& stack) const
         {
            parse_checked<T>(push_inserter(stack));
         }

         template <typename T,
                   typename Container,
                   typename Comparator>
         inline void parse_checked(std::priority_queue<T,Container,Comparator>& priority_queue) const
         {
            parse_checked<T>(push_inserter(priority_queue));
         }

         template <typename Function>
         inline std::size_t for_each_column(const col_range_t& range, Function f) const
         {
            if (!validate_column_range(range))
               return false;
            token_list_t::const_iterator itr = begin_ + range.first;
            token_list_t::const_iterator end = begin_ + range.second;
            std::size_t col_count = 0;
            while (end != itr)
            {
               const range_t& range = (*itr);
               f(range);
               ++itr;
               ++col_count;
            }
            return col_count;
         }

         template <typename Function>
         inline std::size_t for_each_column(Function f) const
         {
            return for_each_column(all_columns(),f);
         }

      private:

         template <typename T>
         inline bool process(const range_t& range, T& t) const
         {
            return string_to_type_converter(range.first,range.second,t);
         }

      private:

         std::size_t index_;
         std::size_t size_;
         token_list_t::const_iterator begin_;
      };

      token_grid()
      : file_name_(""),
        buffer_(0),
        buffer_size_(0),
        min_column_count_(0),
        max_column_count_(0),
        load_from_file_(false),
        state_(false)
      {}

      token_grid(const std::string& file_name,
                 const token_grid::options& options)
      : file_name_(file_name),
        buffer_(0),
        buffer_size_(0),
        min_column_count_(0),
        max_column_count_(0),
        options_(options),
        load_from_file_(true),
        state_(load())
      {}

      token_grid(const unsigned char* input_buffer,
                 const std::size_t& input_buffer_size,
                 const token_grid::options& options)
      : file_name_(""),
        buffer_(const_cast<unsigned char*>(input_buffer)),
        buffer_size_(input_buffer_size),
        min_column_count_(0),
        max_column_count_(0),
        options_(options),
        load_from_file_(false),
        state_(load())
      {}

      token_grid(const char* input_buffer,
                 const std::size_t& input_buffer_size,
                 const token_grid::options& options)
      : file_name_(""),
        buffer_(reinterpret_cast<unsigned char*>(const_cast<char*>(input_buffer))),
        buffer_size_(input_buffer_size),
        min_column_count_(0),
        max_column_count_(0),
        options_(options),
        load_from_file_(false),
        state_(load())
      {}

      token_grid(const std::string& input_buffer,
                 const std::size_t& input_buffer_size,
                 const token_grid::options& options)
      : file_name_(""),
        buffer_(reinterpret_cast<unsigned char*>(const_cast<char*>(input_buffer.data()))),
        buffer_size_(input_buffer_size),
        min_column_count_(0),
        max_column_count_(0),
        options_(options),
        load_from_file_(false),
        state_(load())
      {}

      token_grid(const std::string& file_name,
                 const std::string& column_delimiters = ",|;\t",
                 const std::string& row_delimiters = "\n\r")
      : file_name_(file_name),
        buffer_(0),
        buffer_size_(0),
        min_column_count_(0),
        max_column_count_(0),
        options_(split_options::compress_delimiters,
                 split_options::compress_delimiters,
                 row_delimiters,
                 column_delimiters),
        load_from_file_(true),
        state_(load())
      {}

      token_grid(const unsigned char* input_buffer,
                 const std::size_t& input_buffer_size,
                 const std::string& column_delimiters = ",|;\t",
                 const std::string& row_delimiters = "\n\r")
      : file_name_(""),
        buffer_(const_cast<unsigned char*>(input_buffer)),
        buffer_size_(input_buffer_size),
        min_column_count_(0),
        max_column_count_(0),
        options_(split_options::compress_delimiters,
                 split_options::compress_delimiters,
                 row_delimiters,
                 column_delimiters),
        load_from_file_(false),
        state_(load())
      {}

      token_grid(const char* input_buffer,
                 const std::size_t& input_buffer_size,
                 const std::string& column_delimiters = ",|;\t",
                 const std::string& row_delimiters = "\n\r")
      : file_name_(""),
        buffer_(reinterpret_cast<unsigned char*>(const_cast<char*>(input_buffer))),
        buffer_size_(input_buffer_size),
        min_column_count_(0),
        max_column_count_(0),
        options_(split_options::compress_delimiters,
                 split_options::compress_delimiters,
                 row_delimiters,
                 column_delimiters),
        load_from_file_(false),
        state_(load())
      {}

      token_grid(const std::string& input_buffer,
                 const std::size_t& input_buffer_size,
                 const std::string& column_delimiters = ",;|\t ",
                 const std::string& row_delimiters = "\n\r")
      : file_name_(""),
        buffer_(reinterpret_cast<unsigned char*>(const_cast<char*>(input_buffer.data()))),
        buffer_size_(input_buffer_size),
        min_column_count_(0),
        max_column_count_(0),
        options_(split_options::compress_delimiters,
                 split_options::compress_delimiters,
                 row_delimiters,
                 column_delimiters),
        load_from_file_(false),
        state_(load())
      {}

     ~token_grid()
      {
         if ((load_from_file_) && (0 != buffer_))
         {
            delete [] buffer_;
            buffer_ = 0;
         }
      }

      inline bool operator!() const
      {
         return !state_;
      }

      inline std::string source_file() const
      {
         return file_name_;
      }

      inline std::size_t row_count() const
      {
         return dsv_index_.row_index.size();
      }

      inline std::size_t min_column_count() const
      {
         return min_column_count_;
      }

      inline std::size_t max_column_count() const
      {
         return max_column_count_;
      }

      inline range_t token(const unsigned int& row, const std::size_t& col) const
      {
         return dsv_index_(col,row);
      }

      template <typename T>
      inline T get(const unsigned int& row, const std::size_t& col)
      {
         range_t r = token(row,col);
         return string_to_type_converter<T>(r.first,r.second);
      }

      inline row_type row(const unsigned int& row_index) const
      {
         return row_type(row_index,dsv_index_);
      }

      inline row_range_t all_rows() const
      {
         return row_range_t(0,static_cast<index_t>(dsv_index_.row_index.size()));
      }

      template <typename OutputIterator>
      inline bool extract_column_checked(const row_range_t& row_range,
                                         const std::size_t& index,
                                         OutputIterator out) const
      {
         if (index > max_column_count_)
            return false;
         else if (row_range_invalid(row_range))
            return false;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& row = dsv_index_.row_index[i];
            if (index < dsv_index_.token_count(row))
            {
               dsv_index_.token_list.begin() + (row.first + index);
               process_token_checked(*(dsv_index_.token_list.begin() + (row.first + index)),out);
            }
         }
         return true;
      }

      template <typename OutputIterator>
      inline bool extract_column_checked(const std::size_t& index,
                                         OutputIterator out) const
      {
         return extract_column_checked(all_rows(),index,out);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void extract_column_checked(const std::size_t& index,
                                          Sequence<T,Allocator>& sequence) const
      {
         extract_column_checked(index,back_inserter_with_valuetype(sequence));
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline void extract_column_checked(const std::size_t& index,
                                         std::set<T,Comparator,Allocator>& set) const
      {
         extract_column_checked(index,inserter_with_valuetype(set));
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline void extract_column_checked(const std::size_t& index,
                                         std::multiset<T,Comparator,Allocator>& multiset) const
      {
         extract_column_checked(index,inserter_with_valuetype(multiset));
      }

      template <typename OutputIterator>
      inline bool extract_column(const row_range_t& row_range,
                                 const std::size_t& index,
                                 OutputIterator out) const
      {

         if (index > max_column_count_)
            return false;
         else if (row_range_invalid(row_range))
            return false;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& row = dsv_index_.row_index[i];
            if (index < dsv_index_.token_count(row))
            {
               process_token(*(dsv_index_.token_list.begin() + (row.first + index)),out);
            }
         }
         return true;
      }

      template <typename OutputIterator>
      inline bool extract_column(const std::size_t& index,
                                 OutputIterator out) const
      {
         return extract_column(all_rows(),index,out);
      }

      template <typename OutputIterator0, typename OutputIterator1>
      inline bool extract_column(const row_range_t& row_range,
                                 const std::size_t& index0,
                                 const std::size_t& index1,
                                 OutputIterator0 out0,
                                 OutputIterator1 out1) const
      {
         if ((index0 > max_column_count_) ||
             (index1 > max_column_count_))
            return false;
         else if (row_range_invalid(row_range))
            return false;
         std::size_t max_index = std::max(index0,index1);
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& row = dsv_index_.row_index[i];
            if (max_index < dsv_index_.token_count(row))
            {
               process_token(*(dsv_index_.token_list.begin() + (row.first + index0)),out0);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index1)),out1);
            }
         }
         return true;
      }

      template <typename OutputIterator0, typename OutputIterator1, typename OutputIterator2>
      inline bool extract_column(const row_range_t& row_range,
                                 const std::size_t& index0,
                                 const std::size_t& index1,
                                 const std::size_t& index2,
                                 OutputIterator0 out0,
                                 OutputIterator1 out1,
                                 OutputIterator2 out2) const
      {
         if ((index0 > max_column_count_) ||
             (index1 > max_column_count_) ||
             (index2 > max_column_count_))
            return false;
         else if (row_range_invalid(row_range))
            return false;
         std::size_t max_index = std::max(index0,std::max(index1,index2));
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& row = dsv_index_.row_index[i];
            if (max_index < dsv_index_.token_count(row))
            {
               process_token(*(dsv_index_.token_list.begin() + (row.first + index0)),out0);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index1)),out1);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index2)),out2);
            }
         }
         return true;
      }

      template <typename OutputIterator0, typename OutputIterator1,
                typename OutputIterator2, typename OutputIterator3>
      inline bool extract_column(const row_range_t& row_range,
                                 const std::size_t& index0,
                                 const std::size_t& index1,
                                 const std::size_t& index2,
                                 const std::size_t& index3,
                                 OutputIterator0 out0,
                                 OutputIterator1 out1,
                                 OutputIterator2 out2,
                                 OutputIterator3 out3) const
      {
         if ((index0 > max_column_count_) ||
             (index1 > max_column_count_) ||
             (index2 > max_column_count_) ||
             (index3 > max_column_count_))
            return false;
         else if (row_range_invalid(row_range))
            return false;
         std::size_t max_index = std::max(std::max(index0,index1),std::max(index2,index3));
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& row = dsv_index_.row_index[i];
            if (max_index < dsv_index_.token_count(row))
            {
               process_token(*(dsv_index_.token_list.begin() + (row.first + index0)),out0);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index1)),out1);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index2)),out2);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index3)),out3);
            }
         }
         return true;
      }

      template <typename OutputIterator0, typename OutputIterator1,
                typename OutputIterator2, typename OutputIterator3,
                typename OutputIterator4>
      inline bool extract_column(const row_range_t& row_range,
                                 const std::size_t& index0,
                                 const std::size_t& index1,
                                 const std::size_t& index2,
                                 const std::size_t& index3,
                                 const std::size_t& index4,
                                 OutputIterator0 out0,
                                 OutputIterator1 out1,
                                 OutputIterator2 out2,
                                 OutputIterator3 out3,
                                 OutputIterator4 out4) const
      {
         if ((index0 > max_column_count_) ||
             (index1 > max_column_count_) ||
             (index2 > max_column_count_) ||
             (index3 > max_column_count_) ||
             (index4 > max_column_count_))
            return false;
         else if (row_range_invalid(row_range))
            return false;
         std::size_t max_index = std::max(index4,std::max(std::max(index0,index1),std::max(index2,index3)));
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& row = dsv_index_.row_index[i];
            if (max_index < dsv_index_.token_count(row))
            {
               process_token(*(dsv_index_.token_list.begin() + (row.first + index0)),out0);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index1)),out1);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index2)),out2);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index3)),out3);
               process_token(*(dsv_index_.token_list.begin() + (row.first + index4)),out4);
            }
         }
         return true;
      }

      inline void remove_row(const std::size_t& index)
      {
         if (index < dsv_index_.row_index.size())
         {
            dsv_index_.remove_row(index);
         }
      }

      template <typename Predicate>
      inline bool remove_row_if(const row_range_t& row_range, Predicate predicate)
      {
         if (row_range_invalid(row_range))
            return false;
         std::size_t removed_token_count = 0;
         std::deque<std::size_t> remove_token_list;
         std::deque<std::size_t> remove_row_list;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            row_index_range_t& r = dsv_index_.row_index[i];
            std::size_t temp_r_first = r.first - removed_token_count;
            row_type row(i,dsv_index_);
            if (predicate(row))
            {
               remove_row_list.push_back(i);
               for (std::size_t j = r.first; j <= r.second; ++j)
               {
                  remove_token_list.push_back(j);
               }
               removed_token_count += row.size();
            }
            r.first   = static_cast<unsigned int>(temp_r_first);
            r.second -= static_cast<unsigned int>(removed_token_count);
         }
         for (std::size_t i = row_range.second; i < dsv_index_.row_index.size(); ++i)
         {
            row_index_range_t& r = dsv_index_.row_index[i];
            r.first  -= static_cast<unsigned int>(removed_token_count);
            r.second -= static_cast<unsigned int>(removed_token_count);
         }
         if (!remove_row_list.empty())
         {
            remove_inplace(index_remover(remove_row_list),dsv_index_.row_index);
         }
         if (!remove_token_list.empty())
         {
            remove_inplace(index_remover(remove_token_list),dsv_index_.token_list);
         }
         return true;
      }

      template <typename Predicate>
      inline bool remove_row_if(Predicate predicate)
      {
         return remove_row_if(all_rows(),predicate);
      }

      template <typename Predicate>
      inline std::size_t remove_token_if(const row_range_t& row_range, Predicate predicate)
      {
         if (row_range_invalid(row_range))
            return 0;
         std::size_t removed_token_count = 0;
         std::deque<std::size_t> remove_token_list;
         std::deque<std::size_t> remove_row_list;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            row_index_range_t& r = dsv_index_.row_index[i];
            std::size_t temp_r_first = r.first - removed_token_count;
            row_type row(i,dsv_index_);
            for (std::size_t j = 0; j < row.size(); ++j)
            {
               if (predicate(row.token(j)))
               {
                  remove_token_list.push_back(r.first + j);
                  ++removed_token_count;
               }
            }
            r.first   = static_cast<unsigned int>(temp_r_first);
            r.second -= static_cast<unsigned int>(removed_token_count);
            if (0 == dsv_index_.token_count(r))
            {
               remove_row_list.push_back(i);
            }
         }
         for (std::size_t i = row_range.second; i < dsv_index_.row_index.size(); ++i)
         {
            row_index_range_t& r = dsv_index_.row_index[i];
            r.first  -= static_cast<unsigned int>(removed_token_count);
            r.second -= static_cast<unsigned int>(removed_token_count);
         }
         if (!remove_row_list.empty())
         {
            remove_inplace(index_remover(remove_row_list),dsv_index_.row_index);
         }
         if (!remove_token_list.empty())
         {
            remove_inplace(index_remover(remove_token_list),dsv_index_.token_list);
         }
         if (!remove_token_list.empty())
         {
            update_minmax_columns();
         }
         return remove_token_list.size();
      }

      inline std::size_t remove_empty_tokens(const row_range_t& range)
      {
         return remove_token_if(range,is_empty_token());
      }

      inline std::size_t remove_empty_tokens()
      {
         return remove_empty_tokens(all_rows());
      }

      inline void enforce_column_count(const row_range_t& row_range,
                                       const std::size_t& column_count)
      {
         if (row_range_invalid(row_range))
            return;
         remove_row_if(insufficient_number_of_columns(column_count));
         min_column_count_ = column_count;
         max_column_count_ = column_count;
      }

      inline void enforce_column_count(const std::size_t& column_count)
      {
         enforce_column_count(all_rows(),column_count);
      }

      inline void enforce_min_max_column_count(const row_range_t& row_range,
                                               const std::size_t& min_column_count,
                                               const std::size_t& max_column_count)
      {
         if (row_range_invalid(row_range))
            return;
         remove_row_if(insufficient_number_of_minmax_columns(min_column_count,max_column_count));
         min_column_count_ = min_column_count;
         max_column_count_ = max_column_count;
      }

      inline void enforce_min_max_column_count(const std::size_t& min_column_count,
                                               const std::size_t& max_column_count)
      {
         enforce_min_max_column_count(all_rows(),min_column_count,max_column_count);
      }

      inline void clear(const bool force_delete_buffer = false)
      {
         if (load_from_file_ || force_delete_buffer)
            delete[] buffer_;
         buffer_ = 0;
         buffer_size_ = 0;
         dsv_index_.clear();
         min_column_count_ = 0;
         max_column_count_ = 0;
         state_ = false;
         file_name_ = "";
      }

      inline std::size_t column_width(const std::size_t& col,
                                      const row_range_t& row_range) const
      {
         if (col > max_column_count_)
            return 0;
         else if (row_range_invalid(row_range))
            return 0;
         std::size_t result = 0;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& r = dsv_index_.row_index[i];
            if (col < dsv_index_.token_count(r))
            {
               const range_t& range = *(dsv_index_.token_list.begin() + r.first + col);
               result = std::max<std::size_t>(std::distance(range.first,range.second),result);
            }
         }
         return result;
      }

      inline std::size_t column_width(const std::size_t& col) const
      {
         return column_width(col,all_rows());
      }

      template <typename Allocator,
                template <typename,typename> class Sequence>
      inline void get_column_widths(Sequence<std::size_t,Allocator>& columns)
      {
         for (std::size_t c = 0; c < max_column_count(); ++c)
         {
            columns.push_back(column_width(c));
         }
      }

      template <typename Allocator,
                template <typename,typename> class Sequence>
      inline void get_column_widths(Sequence<std::pair<std::size_t,std::size_t>,Allocator>& columns)
      {
         for (std::size_t c = 0; c < max_column_count(); ++c)
         {
            columns.push_back(std::make_pair(c,column_width(c)));
         }
      }

      template <typename T>
      inline std::size_t accumulate_row(const std::size_t& row, T& result) const
      {
         if (row >= dsv_index_.row_index.size())
            return 0;
         const row_index_range_t& r = dsv_index_.row_index[row];
         token_list_t::const_iterator itr = dsv_index_.token_list.begin() + r.first;
         token_list_t::const_iterator end = dsv_index_.token_list.begin() + r.second + 1;
         std::size_t process_count = 0;
         T current_value = T();
         while (end != itr)
         {
            if (string_to_type_converter((*itr).first,(*itr).second,current_value))
            {
              result += current_value;
              ++process_count;
            }
            else
               return 0;
            ++itr;
         }
         return process_count;
      }

      template <typename T>
      inline std::size_t accumulate_column(const std::size_t& col,
                                           const row_range_t& row_range,
                                           T& result) const
      {
         if (col > max_column_count_)
            return 0;
         else if (row_range_invalid(row_range))
            return 0;
         std::size_t process_count = 0;
         T current_value = T();
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& r = dsv_index_.row_index[i];
            if (col < dsv_index_.token_count(r))
            {
               const range_t& range = *(dsv_index_.token_list.begin() + r.first + col);
               if (string_to_type_converter(range.first,range.second,current_value))
                  result += current_value;
               else
                  return 0;
            }
            ++process_count;
         }
         return process_count;
      }

      template <typename T>
      inline std::size_t accumulate_column(const std::size_t& col, T& result) const
      {
         return accumulate_column(col,all_rows(),result);
      }

      template <typename T, typename Predicate>
      inline std::size_t accumulate_column(const std::size_t& col,
                                           const row_range_t& row_range,
                                           Predicate p,
                                           T& result) const
      {
         if (col > max_column_count_)
            return 0;
         else if (row_range_invalid(row_range))
            return 0;
         std::size_t process_count = 0;
         T current_value = T();
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& r = dsv_index_.row_index[i];
            if (col < dsv_index_.token_count(r))
            {
               row_type row = row_type(i,dsv_index_);
               if (p(row))
               {
                  const range_t& range = row.token(col);
                  if (string_to_type_converter(range.first,range.second,current_value))
                  {
                     result += current_value;
                     ++process_count;
                  }
                  else
                     return 0;
               }
            }

         }
         return process_count;
      }

      template <typename T, typename Predicate>
      inline std::size_t accumulate_column(const std::size_t& col,
                                           Predicate p,
                                           T& result) const
      {
         return accumulate_column(col,all_rows(),p,result);
      }

      inline bool join_row(const std::size_t& row,
                           const std::string& delimiter,
                           std::string& result)
      {
         if (row >= dsv_index_.row_index.size())
            return false;
         const row_index_range_t& r = dsv_index_.row_index[row];
         token_list_t::const_iterator itr = dsv_index_.token_list.begin() + r.first;
         token_list_t::const_iterator end = dsv_index_.token_list.begin() + r.second + (row < (dsv_index_.row_index.size() - 1) ? 1 : 0);
         result.reserve(delimiter.size() * dsv_index_.token_count(r) + std::distance(itr->first,end->second));
         bool appended = false;
         while (end != itr)
         {
            if (!delimiter.empty() && appended)
               result.append(delimiter);
            appended = false;
            if ((*itr).first != (*itr).second)
            {
               result.append((*itr).first,(*itr).second);
               appended = true;
            }
            ++itr;
         }
         return true;
      }

      template <typename Predicate>
      inline bool join_row(const std::size_t& row,
                           Predicate predicate,
                           const std::string& delimiter,
                           std::string& result)
      {
         if (row >= dsv_index_.row_index.size())
            return false;
         const row_index_range_t& r = dsv_index_.row_index[row];
         token_list_t::const_iterator itr = (dsv_index_.token_list.begin() + r.first);
         token_list_t::const_iterator end = dsv_index_.token_list.begin() + r.second + (row < (dsv_index_.row_index.size() - 1) ? 1 : 0);
         result.reserve(delimiter.size() * dsv_index_.token_count(r) + std::distance(itr->first,end->second));
         bool appended = false;
         while (end != itr)
         {
            if (!delimiter.empty() && appended)
               result.append(delimiter);
            appended = false;
            if ((*itr).first != (*itr).second)
            {
               if (predicate(*itr))
               {
                  result.append((*itr).first,(*itr).second);
                  appended = true;
               }
            }
            ++itr;
         }
         return true;
      }

      template <typename Predicate>
      inline bool join_row(const std::size_t& row,
                           Predicate predicate,
                           const char* delimiter,
                           std::string& result)
      {
         return join_row(row,predicate,std::string(delimiter),result);
      }

      inline bool join_column(const std::size_t& col,
                              const row_range_t& row_range,
                              const std::string& delimiter,
                              std::string& result) const
      {
         if (col > max_column_count_)
            return false;
         else if (row_range_invalid(row_range))
            return false;
         bool appended = false;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& r = dsv_index_.row_index[i];
            if (col < dsv_index_.token_count(r))
            {
               row_type row = row_type(i,dsv_index_);
               const range_t& range = row.token(col);
               if (!delimiter.empty() && appended)
                  result.append(delimiter);
               appended = false;
               if (range.first != range.second)
               {
                  result.append(range.first,range.second);
                  appended = true;
               }
            }
         }
         return true;
      }

      inline bool join_column(const std::size_t& col,
                              const std::string& delimiter,
                              std::string& result) const
      {
         return join_column(col,all_rows(),delimiter,result);
      }

      template <typename Predicate>
      inline bool join_column(const std::size_t& col,
                              const row_range_t& row_range,
                              Predicate predicate,
                              const std::string& delimiter,
                              std::string& result) const
      {
         if (col > max_column_count_)
            return false;
         else if (row_range_invalid(row_range))
            return false;
         bool appended = false;
         const std::size_t pre_end_index = row_range.second - 1;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            const row_index_range_t& r = dsv_index_.row_index[i];
            if (col < dsv_index_.token_count(r))
            {
               row_type row = row_type(i,dsv_index_);
               const range_t& range = row.token(col);
               if (!delimiter.empty() && appended && (pre_end_index != i))
                  result.append(delimiter);
               appended = false;
               if (range.first != range.second)
               {
                  if (predicate(row))
                  {
                     result.append(range.first,range.second);
                     appended = true;
                  }
               }
            }
         }
         return true;
      }

      template <typename Predicate>
      inline bool join_column(const std::size_t& col,
                              Predicate p,
                              const std::string& delimiter,
                              std::string& result) const
      {
         return join_column(col,all_rows(),p,delimiter,result);
      }

      template <typename TransitionPredicate, typename Function>
      inline bool sequential_partition(const row_range_t& row_range,
                                       TransitionPredicate p,
                                       Function f)
      {
         if (row_range_invalid(row_range))
            return false;
         row_range_t r(row_range.first,row_range.first);
         for (row_range_t::first_type i = row_range.first; i < row_range.second; ++i)
         {
            if (p(row_type(i,dsv_index_)))
            {
               if (r.first != r.second)
               {
                  r.second = i;
                  if (!f(*this,r))
                     return false;
               }
               r.first = r.second;
            }
            else
               r.second = i;
         }
         if (r.first != row_range.second)
         {
            r.second = row_range.second;
            if (!f(*this,r))
               return false;
         }
         return true;
      }

      template <typename TransitionPredicate, typename Function>
      inline bool sequential_partition(TransitionPredicate p, Function f)
      {
         return sequential_partition(all_rows(),p,f);
      }

      static inline token_grid::options default_options()
      {
         return options();
      }

      template <typename Function>
      inline std::size_t for_each_row(const row_range_t& row_range, Function f) const
      {
         if (row_range_invalid(row_range))
            return 0;
         std::size_t row_count = 0;
         for (std::size_t i = row_range.first; i < row_range.second; ++i)
         {
            f(row_type(i,dsv_index_));
            ++row_count;
         }
         return row_count;
      }

      template <typename Function>
      inline std::size_t for_each_row(Function f) const
      {
         return for_each_row(all_rows(),f);
      }

      bool load(const std::string& file_name,
                const token_grid::options& options)
      {
         file_name_ = file_name;
         if ((load_from_file_) && (0 != buffer_))
         {
            delete [] buffer_;
            buffer_ = 0;
         }
         buffer_size_ = 0;
         min_column_count_ = 0;
         max_column_count_ = 0;
         options_ = options;
         load_from_file_ = true;
         state_ = load();
         if (state_)
            return true;
         else
         {
            file_name_ = "";
            if ((load_from_file_) && (0 != buffer_))
            {
               delete [] buffer_;
               buffer_ = 0;
            }
            return false;
         }
      }

      bool load(unsigned char* buffer,
                const std::size_t buffer_size,
                const token_grid::options& options)
      {
         file_name_ = "";
         if ((load_from_file_) && (0 != buffer_))
         {
            delete [] buffer_;
            buffer_ = 0;
         }
         min_column_count_ = 0;
         max_column_count_ = 0;
         options_ = options;
         load_from_file_ = false;
         buffer_ = buffer;
         buffer_size_ = buffer_size;
         state_ = load();
         if (state_)
            return true;
         else
         {
            file_name_ = "";
            if ((load_from_file_) && (0 != buffer_))
            {
               delete [] buffer_;
               buffer_ = 0;
            }
            return false;
         }
      }

   private:

      token_grid(const token_grid& tg);
      token_grid& operator=(const token_grid& tg);

      struct is_empty_token
      {
         inline bool operator()(const range_t& r) const
         {
            return r.first == r.second;
         }
      };

      struct insufficient_number_of_columns
      {
         insufficient_number_of_columns(const std::size_t& noc)
         : num_of_cols(noc)
         {}

         inline bool operator()(const row_type& row) const
         {
            return (num_of_cols != row.size());
         }

         std::size_t num_of_cols;
      };

      struct insufficient_number_of_minmax_columns
      {
         insufficient_number_of_minmax_columns(const std::size_t& min_col, const std::size_t& max_col)
         : min_column_count(min_col),
           max_column_count(max_col)
         {}

         inline bool operator()(const row_type& row) const
         {
            return (row.size() < min_column_count) || (max_column_count < row.size());
         }

         std::size_t min_column_count;
         std::size_t max_column_count;
      };

      class double_quotes_predicate
      {
      public:

         double_quotes_predicate(const std::string& delimiters)
         : in_bracket_range_(false),
           mdp_(delimiters)
         {}

         inline bool operator()(const unsigned char c) const
         {
            if ('"' == c)
            {
               in_bracket_range_ = !in_bracket_range_;
               return false;
            }
            else if (in_bracket_range_)
               return false;
            else
               return mdp_(c);
         }

         inline void reset()
         {
            in_bracket_range_ = false;
         }

      private:

         mutable bool in_bracket_range_;
         mutable strtk::multiple_char_delimiter_predicate mdp_;
      };

      inline bool load()
      {
         if (load_from_file_ && !load_buffer_from_file())
            return false;

         dsv_index_.token_list.clear();
         dsv_index_.row_index.clear();

         multiple_char_delimiter_predicate text_newline_predicate(options_.row_delimiters);

         if (!options_.support_dquotes)
         {
            multiple_char_delimiter_predicate token_predicate(options_.column_delimiters);
            strtk::split(text_newline_predicate,
                         buffer_, buffer_ + buffer_size_,
                         strtk::functional_inserter(
                            row_processor<multiple_char_delimiter_predicate>(dsv_index_,token_predicate,options_.column_split_option)),
                         strtk::split_options::compress_delimiters);
         }
         else
         {
            double_quotes_predicate token_predicate_dblq(options_.column_delimiters);
            strtk::split(text_newline_predicate,
                         buffer_, buffer_ + buffer_size_,
                         strtk::functional_inserter(
                            row_processor<double_quotes_predicate>(dsv_index_,token_predicate_dblq,options_.column_split_option)),
                        strtk::split_options::compress_delimiters);

            if (options_.trim_dquotes)
            {
               for (std::size_t i = 0; i < dsv_index_.token_list.size(); ++i)
               {
                  if (
                      ((*(dsv_index_.token_list[i].first     )) == '"') &&
                      ((*(dsv_index_.token_list[i].second - 1)) == '"')
                     )
                  {
                      ++dsv_index_.token_list[i].first;
                      --dsv_index_.token_list[i].second;
                  }
               }
            }
         }
         update_minmax_columns();
         return true;
      }

      inline bool load_buffer_from_file()
      {
         std::ifstream stream(file_name_.c_str(),std::ios::binary);
         if (!stream)
            return false;
         stream.seekg (0,std::ios::end);
         buffer_size_ = static_cast<std::size_t>(stream.tellg());
         if (0 == buffer_size_)
            return false;
         stream.seekg (0,std::ios::beg);
         buffer_ = new unsigned char[buffer_size_];
         stream.read(reinterpret_cast<char*>(buffer_),static_cast<std::streamsize>(buffer_size_));
         stream.close();
         return true;
      }

      template <typename OutputIterator>
      inline void process_token(const range_t& range, OutputIterator out) const
      {
         typedef typename std::iterator_traits<OutputIterator>::value_type output_type;
         (*out) = string_to_type_converter<output_type>(range.first,range.second);
         ++out;
      }

      template <typename OutputIterator>
      inline void process_token_checked(const range_t& range, OutputIterator out) const
      {
         typedef typename std::iterator_traits<OutputIterator>::value_type output_type;
         output_type value;
         if (string_to_type_converter(range.first,range.second,value))
         {
            (*out) = value;
            ++out;
         }
      }

      inline bool row_range_invalid(const row_range_t& row_range) const
      {
         if (row_range.first > dsv_index_.row_index.size())
            return true;
         else if (row_range.second > dsv_index_.row_index.size())
            return true;
         else if (row_range.first > row_range.second)
            return true;
         else
            return false;
      }

      inline void update_minmax_columns()
      {
         min_column_count_ = std::numeric_limits<std::size_t>::max();
         max_column_count_ = std::numeric_limits<std::size_t>::min();
         for (std::size_t i = 0; i < dsv_index_.row_index.size(); ++i)
         {
            const row_index_range_t& r = dsv_index_.row_index[i];
            const std::size_t number_of_tokens = dsv_index_.token_count(r);
            if (number_of_tokens > max_column_count_)
               max_column_count_ = number_of_tokens;
            if (number_of_tokens < min_column_count_)
               min_column_count_ = number_of_tokens;
         }
      }

   private:

      store dsv_index_;
      std::string file_name_;
      unsigned char* buffer_;
      std::size_t buffer_size_;
      std::size_t min_column_count_;
      std::size_t max_column_count_;
      options options_;
      bool load_from_file_;
      bool state_;
   };

   template <typename T>
   inline bool convert_string_range(const std::pair<std::string::const_iterator,std::string::const_iterator>& range, T& t)
   {
      if (range.first == range.second) return false;
      t = string_to_type_converter<T>(std::string(range.first,range.second));
      return true;
   }

   struct empty_range
   {
   public:

      template <typename InputIterator>
      inline bool operator()(const InputIterator begin, const InputIterator end)
      {
         return (0 == std::distance(begin,end));
      }
   };

   struct nonempty_range
   {
   public:

      template <typename InputIterator>
      inline bool operator()(const InputIterator begin, const InputIterator end)
      {
         return (0 != std::distance(begin,end));
      }
   };

   template <typename OutputIterator>
   struct filter_non_empty_range
   {
   public:

      filter_non_empty_range(OutputIterator out)
      : out_(out)
      {}

      template <typename Iterator>
      inline void operator() (const std::pair<Iterator,Iterator>& range)
      {
         if (range.first != range.second)
         {
            *out_++ = range;
         }
      }

   private:

      OutputIterator out_;
   };

   template <typename OutputPredicate>
   struct filter_on_wildcard_match
   {
   public:

      filter_on_wildcard_match(const std::string& match_pattern,
                               OutputPredicate& predicate,
                               bool allow_through_on_match = true)
      : allow_through_on_match_(allow_through_on_match),
        match_pattern_(match_pattern),
        predicate_(predicate)
      {}

      template <typename Iterator>
      inline void operator() (const std::pair<Iterator,Iterator>& range) const
      {
         if (match(match_pattern_.begin(),match_pattern_.end(),range.first,range.second,'*','?') ^ allow_through_on_match_)
         {
            predicate_(range);
         }
      }

      inline void operator() (const std::string& s) const
      {
         if (match(match_pattern_,s) ^ allow_through_on_match_)
         {
            predicate_(s);
         }
      }

   private:

      filter_on_wildcard_match(const filter_on_wildcard_match& fom);
      filter_on_wildcard_match& operator=(const filter_on_wildcard_match& fom);

      bool allow_through_on_match_;
      std::string match_pattern_;
      OutputPredicate& predicate_;
   };

  template <typename OutputPredicate>
   struct filter_on_match
   {
   public:

     filter_on_match(const std::string* begin, const std::string* end,
                     OutputPredicate predicate,
                     bool case_insensitive,
                     bool allow_through_on_match = true)
     : case_insensitive_(case_insensitive),
       allow_through_on_match_(allow_through_on_match),
       begin_(begin),
       end_(end),
       predicate_(predicate)
     {}

      template <typename Iterator>
      inline void operator() (const std::pair<Iterator,Iterator>& range) const
      {
         for (const std::string* itr = begin_; end_ != itr; ++itr)
         {
            if ((case_insensitive_ &&
               (imatch((*itr).data(),(*itr).data() + (*itr).size(),range.first,range.second))) ||
               (!case_insensitive_ && std::equal((*itr).begin(),(*itr).end(),range.first)))
            {
               if (allow_through_on_match_)
               {
                  predicate_(range);
               }
               return;
            }
         }

         if (!allow_through_on_match_)
         {
            predicate_(range);
            return;
         }
      }

      inline void operator() (const std::string& s) const
      {
         for (const std::string* itr = begin_; end_ != itr; ++itr)
         {
            if ((case_insensitive_ &&
               (imatch((*itr).begin(),(*itr).end(),s.begin(),s.end()))) ||
               (!case_insensitive_ && std::equal((*itr).begin(),(*itr).end(),s.begin())))
            {
               if (allow_through_on_match_)
               {
                  predicate_(s);
                  return;
               }
            }
         }

         if (!allow_through_on_match_)
         {
            predicate_(s);
            return;
         }
      }

   private:

      filter_on_match& operator=(const filter_on_match& fom);

   private:

      bool case_insensitive_;
      bool allow_through_on_match_;
      const std::string* begin_;
      const std::string* end_;
      OutputPredicate predicate_;
   };

   template <typename Iterator, typename MatchPredicate>
   inline void skip_while_matching(Iterator& itr,
                                   const Iterator& end,
                                   const MatchPredicate& predicate)
   {
      while (end != itr)
      {
         if (predicate(*itr))
            ++itr;
         else
            break;
      }
   }

   template <std::size_t length>
   struct size_equal_to
   {
      template <typename Iterator>
      inline bool operator()(const Iterator begin, const Iterator end) const
      {
         return length == std::distance(begin,end);
      }

      template <typename Iterator>
      inline bool operator()(const std::pair<Iterator,Iterator>& range) const
      {
         return length == std::distance(range.first,range.second);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool operator()(const Sequence<T,Allocator>& sequence) const
      {
         return length == sequence.size();
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::set<T,Comparator,Allocator>& set) const
      {
         return length == set.size();
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::multiset<T,Comparator,Allocator>& multiset) const
      {
         return length == multiset.size();
      }

      inline bool operator()(const std::string& str) const
      {
         return length == str.size();
      }
   };

   template <std::size_t length>
   struct size_less_than
   {
      template <typename Iterator>
      inline bool operator()(const Iterator begin, const Iterator end) const
      {
         return std::distance(begin,end) < static_cast<typename std::iterator_traits<Iterator>::difference_type>(length);
      }

      template <typename Iterator>
      inline bool operator()(const std::pair<Iterator,Iterator>& range) const
      {
         return std::distance(range.first,range.second) < static_cast<typename std::iterator_traits<Iterator>::difference_type>(length);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool operator()(const Sequence<T,Allocator>& sequence) const
      {
         return sequence.size() < length;
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::set<T,Comparator,Allocator>& set) const
      {
         return set.size() < length;
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::multiset<T,Comparator,Allocator>& multiset) const
      {
         return multiset.size() < length;
      }

      inline bool operator()(const std::string& str) const
      {
         return str.size() < length;
      }
   };

   template <std::size_t length>
   struct size_greater_than
   {
      template <typename Iterator>
      inline bool operator()(const Iterator begin, const Iterator end) const
      {
         return std::distance(begin,end) > static_cast<typename std::iterator_traits<Iterator>::difference_type>(length);
      }

      template <typename Iterator>
      inline bool operator()(const std::pair<Iterator,Iterator>& range) const
      {
         return std::distance(range.first,range.second) > static_cast<typename std::iterator_traits<Iterator>::difference_type>(length);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool operator()(const Sequence<T,Allocator>& sequence) const
      {
         return sequence.size() > length;
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::set<T,Comparator,Allocator>& set) const
      {
         return set.size() > length;
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::multiset<T,Comparator,Allocator>& multiset) const
      {
         return multiset.size() > length;
      }

      inline bool operator()(const std::string& str) const
      {
         return str.size() > length;
      }
   };

   struct size_is_even
   {
      template <typename Iterator>
      inline bool operator()(const Iterator begin, const Iterator end) const
      {
         return 0 == (std::distance(begin,end) % 2);
      }

      template <typename Iterator>
      inline bool operator()(const std::pair<Iterator,Iterator>& range) const
      {
         return 0 == (std::distance(range.first,range.second) % 2);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool operator()(const Sequence<T,Allocator>& sequence) const
      {
         return 0 == (sequence.size() % 2);
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::set<T,Comparator,Allocator>& set) const
      {
         return 0 == (set.size() % 2);
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::multiset<T,Comparator,Allocator>& multiset) const
      {
         return 0 == (multiset.size() % 2);
      }

      inline bool operator()(const std::string& str) const
      {
         return 0 == (str.size() % 2);
      }
   };

   struct size_is_odd
   {
      template <typename Iterator>
      inline bool operator()(const Iterator begin, const Iterator end) const
      {
         return 0 != (std::distance(begin,end) % 2);
      }

      template <typename Iterator>
      inline bool operator()(const std::pair<Iterator,Iterator>& range) const
      {
         return 0 != (std::distance(range.first,range.second) % 2);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool operator()(const Sequence<T,Allocator>& sequence) const
      {
         return 0 != (sequence.size() % 2);
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::set<T,Comparator,Allocator>& set) const
      {
         return 0 != (set.size() % 2);
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool operator()(const std::multiset<T,Comparator,Allocator>& multiset) const
      {
         return 0 != (multiset.size() % 2);
      }

      inline bool operator()(const std::string& str) const
      {
         return 0 != (str.size() % 2);
      }
   };

   template <typename InputIterator,
             typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11, typename T12>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1,  T2&  t2,  T3&  t3,  T4&  t4,
                     T5& t5,  T6&  t6,  T7&  t7,  T8&  t8,
                     T9& t9, T10& t10, T11& t11, T12& t12)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 12;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second, t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t9)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t10)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t11)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t12)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8,
                     T9& t9, T10& t10, T11& t11)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 11;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second, t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t9)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t10)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t11)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9, typename T10>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8,
                     T9& t9, T10& t10)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 10;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second, t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t9)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t10)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8,
                     T9& t9)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 9;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t9)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 8;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t8)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 7;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t7)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 6;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 5;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 4;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false;
      return true;
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 3;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false;
      return true;
   }

   template <typename InputIterator, typename T1, typename T2>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 2;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false;
      return true;
   }

   template <typename InputIterator, typename T>
   inline bool parse(const InputIterator begin,
                     const InputIterator end,
                     const std::string& delimiters,
                     T& t)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      static const std::size_t token_count = 1;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef iterator_type* iterator_type_ptr;
      iterator_type token_list[token_count];
      const std::size_t parsed_token_count = split_n(delimiters,
                                                     begin,end,
                                                     token_count,
                                                     token_list,
                                                     split_options::compress_delimiters);
      if (token_count > parsed_token_count)
         return false;
      iterator_type_ptr itr = token_list;
      return string_to_type_converter((*itr).first,(*itr).second,t);
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse(const InputIterator begin,
                            const InputIterator end,
                            const std::string& delimiters,
                            Sequence<T,Allocator>& sequence,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      begin,end,
                      range_to_type_back_inserter(sequence),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      begin, end,
                      range_to_type_back_inserter(sequence),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse(const InputIterator begin,
                            const InputIterator end,
                            const std::string& delimiters,
                            std::set<T,Comparator,Allocator>& set,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      begin,end,
                      range_to_type_inserter(set),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      begin,end,
                      range_to_type_inserter(set),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse(const InputIterator begin,
                            const InputIterator end,
                            const std::string& delimiters,
                            std::multiset<T,Comparator,Allocator>& multiset,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      begin,end,
                      range_to_type_inserter(multiset),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      begin,end,
                      range_to_type_inserter(multiset),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse(const InputIterator begin,
                            const InputIterator end,
                            const std::string& delimiters,
                            std::queue<T,Container>& queue,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      begin,end,
                      range_to_type_push_inserter(queue),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      begin,end,
                      range_to_type_push_inserter(queue),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse(const InputIterator begin,
                            const InputIterator end,
                            const std::string& delimiters,
                            std::stack<T,Container>& stack,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      begin,end,
                      range_to_type_push_inserter(stack),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      begin, end,
                      range_to_type_push_inserter(stack),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse(const InputIterator begin,
                            const InputIterator end,
                            const std::string& delimiters,
                            std::priority_queue<T,Container,Comparator>& priority_queue,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      begin,end,
                      range_to_type_push_inserter(priority_queue),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      begin,end,
                      range_to_type_push_inserter(priority_queue),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse(const std::pair<InputIterator,InputIterator>& range,
                            const std::string& delimiters,
                            Sequence<T,Allocator>& sequence,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      range_to_type_back_inserter(sequence),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      range_to_type_back_inserter(sequence),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse(const std::pair<InputIterator,InputIterator>& range,
                            const std::string& delimiters,
                            std::set<T,Comparator,Allocator>& set,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      range_to_type_inserter(set),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      range_to_type_inserter(set),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse(const std::pair<InputIterator,InputIterator>& range,
                            const std::string& delimiters,
                            std::multiset<T,Comparator,Allocator>& multiset,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      range_to_type_inserter(multiset),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      range_to_type_inserter(multiset),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse(const std::pair<InputIterator,InputIterator>& range,
                            const std::string& delimiters,
                            std::queue<T,Container>& queue,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      range_to_type_push_inserter(queue),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      range_to_type_push_inserter(queue),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse(const std::pair<InputIterator,InputIterator>& range,
                            const std::string& delimiters,
                            std::stack<T,Container>& stack,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      range_to_type_push_inserter(stack),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      range_to_type_push_inserter(stack),
                      split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse(const std::pair<InputIterator,InputIterator>& range,
                            const std::string& delimiters,
                            std::priority_queue<T,Container,Comparator>& priority_queue,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         return split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                      range.first,range.second,
                      range_to_type_push_inserter(priority_queue),
                      split_option);
      else
         return split(multiple_char_delimiter_predicate(delimiters),
                      range.first,range.second,
                      range_to_type_push_inserter(priority_queue),
                      split_option);
   }

   namespace details
   {
      class container_adder
      {
      private:

         class container_adder_base
         {
         public:

            typedef const char* itr_type;

            virtual ~container_adder_base(){}

            template <typename InputIterator>
            inline bool add(const InputIterator begin, const InputIterator end) const
            {
               return add_impl(begin,end);
            }

            template <typename InputIterator>
            inline bool add(const std::pair<InputIterator,InputIterator>& range) const
            {
               return add(range.first,range.second);
            }

         protected:

            virtual bool add_impl(const itr_type begin, const itr_type end) const = 0;

         };

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         class sequence_adder_impl : public container_adder_base
         {
         public:

            typedef Sequence<T,Allocator> sequence_t;

            sequence_adder_impl(sequence_t& seq)
            : sequence_(seq)
            {}

         protected:

            bool add_impl(const itr_type begin, const itr_type end) const
            {
               T t;
               if (!string_to_type_converter(begin,end,t)) return false;
               sequence_.push_back(t);
               return true;
            }

         private:

            sequence_adder_impl operator=(const sequence_adder_impl&);

            sequence_t& sequence_;
         };

         template <typename T,
                   typename Comparator,
                   typename Allocator,
                   template <typename,typename,typename> class Set>
         class set_adder_impl : public container_adder_base
         {
         public:

            set_adder_impl(Set<T,Comparator,Allocator>& set)
            : set_(set)
            {}

            bool add_impl(const itr_type begin, const itr_type end) const
            {
               T t;
               if (!string_to_type_converter(begin,end,t)) return false;
               set_.insert(t);
               return true;
            }

         private:

            set_adder_impl operator=(const set_adder_impl&);

            Set<T,Comparator,Allocator>& set_;
         };

         template <typename T,
                   typename Container,
                   typename Comparator>
         class pq_adder_impl : public container_adder_base
         {
         public:

            pq_adder_impl(std::priority_queue<T,Container,Comparator>& pq)
            : pq_(pq)
            {}

            bool add_impl(const itr_type begin, const itr_type end) const
            {
               T t;
               if (!string_to_type_converter(begin,end,t)) return false;
               pq_.push(t);
               return true;
            }

         private:

            pq_adder_impl operator=(const pq_adder_impl&);

            std::priority_queue<T,Container,Comparator>& pq_;
         };

         template <typename T,
                   typename Container,
                   template <typename,typename> class SContainer>
         class stack_queue_adder_impl : public container_adder_base
         {
         public:

            stack_queue_adder_impl(SContainer<T,Container>& container)
            : container_(container)
            {}

            bool add_impl(const itr_type begin, const itr_type end) const
            {
               T t;
               if (!string_to_type_converter(begin,end,t)) return false;
               container_.push(t);
               return true;
            }

         private:

            stack_queue_adder_impl operator=(const stack_queue_adder_impl&);

            SContainer<T,Container>& container_;
         };

      public:

         template <typename T, typename Allocator>
         container_adder(std::vector<T,Allocator>& vec)
         : container_adder_base_(new(buffer)sequence_adder_impl<T,Allocator,std::vector>(vec))
         {}

         template <typename T, typename Allocator>
         container_adder(std::deque<T,Allocator>& deq)
         : container_adder_base_(new(buffer)sequence_adder_impl<T,Allocator,std::deque>(deq))
         {}

         template <typename T, typename Allocator>
         container_adder(std::list<T,Allocator>& list)
         : container_adder_base_(new(buffer)sequence_adder_impl<T,Allocator,std::list>(list))
         {}

         template <typename T, typename Comparator, typename Allocator>
         container_adder(std::set<T,Comparator,Allocator>& set)
         : container_adder_base_(new(buffer)set_adder_impl<T,Comparator,Allocator,std::set>(set))
         {}

         template <typename T, typename Comparator, typename Allocator>
         container_adder(std::multiset<T,Comparator,Allocator>& multiset)
         : container_adder_base_(new(buffer)set_adder_impl<T,Comparator,Allocator,std::multiset>(multiset))
         {}

         template <typename T, typename Container, typename Comparator>
         container_adder(std::priority_queue<T,Container,Comparator>& pq)
         : container_adder_base_(new(buffer)pq_adder_impl<T,Container,Comparator>(pq))
         {}

         template <typename T, typename Container>
         container_adder(std::queue<T,Container>& queue)
         : container_adder_base_(new(buffer)stack_queue_adder_impl<T,Container,std::queue>(queue))
         {}

         template <typename T, typename Container>
         container_adder(std::stack<T,Container>& stack)
         : container_adder_base_(new(buffer)stack_queue_adder_impl<T,Container,std::stack>(stack))
         {}

         template <typename InputIterator>
         inline bool add(InputIterator begin, InputIterator end) const
         {
            return container_adder_base_->add(begin,end);
         }

         template <typename InputIterator>
         inline bool add(std::pair<InputIterator,InputIterator>& range) const
         {
            return add(range.first,range.second);
         }

         template <typename InputIterator>
         inline bool operator()(const InputIterator begin, const InputIterator end)
         {
            InputIterator itr = begin;
            while (end != itr)
            {
               if (!add(*itr)) return false;
               ++itr;
            }
            return true;
         }

      private:

         mutable container_adder_base* container_adder_base_;
         unsigned char buffer[64];
      };

      template <typename T,typename is_stl_container_result>
      struct ca_type { typedef T& type; };

      template <typename T>
      struct ca_type <T,details::yes_t> { typedef  details::container_adder type; };

   }

   template <typename InputIterator,
             typename  T1, typename  T2, typename T3,
             typename  T4, typename  T5, typename T6,
             typename  T7, typename  T8, typename T9,
             typename T10, typename T11>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2,  T3&  t3,  T4&  t4, T5& t5, T6& t6, T7& t7,
                     T8& t8, T9& t9, T10& t10, T11& t11,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 12) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second, t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t9)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t10)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t11)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9,
             typename T10>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8, T9& t9, T10& t10,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 11) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second, t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second, t9)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t10)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8, T9& t9,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 10) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t8)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t9)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6,
             typename T7, typename T8>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7, T8& t8,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 9) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t7)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t8)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6, typename T7>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 8) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t7)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 7) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t6)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3,
             typename T4, typename T5>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 6) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t5)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3, typename T4>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
        split(multiple_char_delimiter_predicate(delimiters),
              begin,end,
              std::back_inserter(token_list),
              split_options::compress_delimiters);
      if (token_list.size() < 5) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t4)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2, typename T3>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 4) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t3)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T1, typename T2>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1, T2& t2,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 3) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      if (!string_to_type_converter((*itr).first,(*itr).second,t2)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator, typename T1>
   inline bool parse(const InputIterator begin, const InputIterator end,
                     const std::string& delimiters,
                     T1& t1,
                     details::container_adder ca)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      typedef std::pair<InputIterator,InputIterator> iterator_type;
      typedef typename std::deque<iterator_type>::iterator iterator_type_ptr;
      details::convert_type_assert<itr_type>();
      std::deque<iterator_type> token_list;
      if (1 == delimiters.size())
         split(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      else
         split(multiple_char_delimiter_predicate(delimiters),
               begin,end,
               std::back_inserter(token_list),
               split_options::compress_delimiters);
      if (token_list.size() < 2) return false;
      iterator_type_ptr itr = token_list.begin();
      if (!string_to_type_converter((*itr).first,(*itr).second,t1)) return false; ++itr;
      return ca(itr,token_list.end());
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              Sequence<T,Allocator>& sequence,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      const std::size_t original_size = sequence.size();
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_type_back_inserter(sequence),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_type_back_inserter(sequence),
                 split_option);
      return sequence.size() - original_size;
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::set<T,Comparator,Allocator>& set,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      const std::size_t original_size = set.size();
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_type_inserter(set),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_type_inserter(set),
                 split_option);
      return set.size() - original_size;
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::multiset<T,Comparator,Allocator>& multiset,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      const std::size_t original_size = multiset.size();
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_type_inserter(multiset),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_type_inserter(multiset),
                 split_option);
      return multiset.size() - original_size;
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::queue<T,Container>& queue,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      const std::size_t original_size = queue.size();
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_type_push_inserter(queue),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_type_push_inserter(queue),
                 split_option);
      return queue.size() - original_size;
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::stack<T,Container>& stack,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      const std::size_t original_size = stack.size();
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_type_push_inserter(stack),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_type_push_inserter(stack),
                 split_option);
      return stack.size() - original_size;
   }

   template <typename InputIterator,
             typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::priority_queue<T,Container,Comparator>& priority_queue,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      const std::size_t original_size = priority_queue.size();
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_type_push_inserter(priority_queue),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_type_push_inserter(priority_queue),
                 split_option);
      return priority_queue.size() - original_size;
   }

   template <typename InputIterator, typename T>
   inline std::size_t parse_n(const InputIterator begin,
                              const InputIterator end,
                              const std::string& delimiters,
                              const std::size_t& n,
                              T* out,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type;
      std::size_t insert_count = 0;
      details::convert_type_assert<itr_type>();
      if (1 == delimiters.size())
         split_n(single_delimiter_predicate<std::string::value_type>(delimiters[0]),
                 begin,end,
                 n,
                 range_to_ptr_type(out,insert_count),
                 split_option);
      else
         split_n(multiple_char_delimiter_predicate(delimiters),
                 begin,end,
                 n,
                 range_to_ptr_type(out,insert_count),
                 split_option);
      return insert_count;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse_n(const std::pair<InputIterator,InputIterator>& range,
                              const std::string& delimiters,
                              const std::size_t& n,
                              Sequence<T,Allocator>& sequence,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(range.first,range.second,delimiters,n,sequence,split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_n(const std::pair<InputIterator,InputIterator>& range,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::set<T,Comparator,Allocator>& set,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(range.first,range.second,delimiters,n,set,split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_n(const std::pair<InputIterator,InputIterator>& range,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::multiset<T,Comparator,Allocator>& multiset,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(range.first,range.second,delimiters,n,multiset,split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse_n(const std::pair<InputIterator,InputIterator>& range,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::queue<T,Container>& queue,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(range.first,range.second,delimiters,n,queue,split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container>
   inline std::size_t parse_n(const std::pair<InputIterator,InputIterator>& range,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::stack<T,Container>& stack,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(range.first,range.second,delimiters,n,stack,split_option);
   }

   template <typename InputIterator,
             typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse_n(const std::pair<InputIterator,InputIterator>& range,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::priority_queue<T,Container,Comparator>& priority_queue,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(range.first,range.second,delimiters,n,priority_queue,split_option);
   }

   template <typename T1, typename T2, typename T3, typename  T4,
             typename T5, typename T6, typename T7, typename  T8,
             typename T9, typename T10, typename T11, typename T12>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1,  T2&  t2,  T3&  t3,  T4&  t4,
                     T5& t5,  T6&  t6,  T7&  t7,  T8&  t8,
                     T9& t9, T10& t10, T11& t11, T12& t12)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,
                   typename details::ca_type<T12, typename details::is_stl_container<T12>::result_t>::type(t12));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9, typename T10, typename T11>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2&   t2,  T3&  t3, T4& t4,
                     T5& t5, T6&   t6,  T7&  t7, T8& t8,
                     T9& t9, T10& t10, T11& t11)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,
                   typename details::ca_type<T11, typename details::is_stl_container<T11>::result_t>::type(t11));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9, typename T10>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8,
                     T9& t9, T10& t10)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,t6,t7,t8,t9,
                   typename details::ca_type<T10, typename details::is_stl_container<T10>::result_t>::type(t10));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8,
                     T9& t9)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,t6,t7,t8,
                   typename details::ca_type<T9, typename details::is_stl_container<T9>::result_t>::type(t9));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7, T8& t8)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,t6,t7,
                   typename details::ca_type<T8, typename details::is_stl_container<T8>::result_t>::type(t8));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6, T7& t7)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,t6,
                   typename details::ca_type<T7, typename details::is_stl_container<T7>::result_t>::type(t7));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5, T6& t6)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,t5,
                   typename details::ca_type<T6,typename details::is_stl_container<T6>::result_t>::type(t6));
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4,
                     T5& t5)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,t4,
                   typename details::ca_type<T5, typename details::is_stl_container<T5>::result_t>::type(t5));
   }

   template <typename T1, typename T2, typename T3, typename T4>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3, T4& t4)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,t3,
                   typename details::ca_type<T4, typename details::is_stl_container<T4>::result_t>::type(t4));
   }

   template <typename T1, typename T2, typename T3>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2, T3& t3)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,t2,
                   typename details::ca_type<T3, typename details::is_stl_container<T3>::result_t>::type(t3));
   }

   template <typename T1, typename T2>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T1& t1, T2& t2)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   t1,
                   typename details::ca_type<T2, typename details::is_stl_container<T2>::result_t>::type(t2));
   }

   template <typename T>
   inline bool parse(const std::string& data,
                     const std::string& delimiters,
                     T& t)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   typename details::ca_type<T,typename details::is_stl_container<T>::result_t>::type(t));
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse(const std::string& data,
                            const std::string& delimiters,
                            Sequence<T,Allocator>& sequence,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   sequence,
                   split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse(const std::string& data,
                            const std::string& delimiters,
                            std::set<T,Comparator,Allocator>& set,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   set,
                   split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse(const std::string& data,
                            const std::string& delimiters,
                            std::multiset<T,Comparator,Allocator>& multiset,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   multiset,
                   split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse(const std::string& data,
                            const std::string& delimiters,
                            std::queue<T,Container>& queue,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   queue,
                   split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse(const std::string& data,
                            const std::string& delimiters,
                            std::stack<T,Container>& stack,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   stack,
                   split_option);
   }

   template <typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse(const std::string& data,
                            const std::string& delimiters,
                            std::priority_queue<T,Container,Comparator>& priority_queue,
                            const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse(data.data(),
                   data.data() + data.size(),
                   delimiters,
                   priority_queue,
                   split_option);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse(const int& argc, char* argv[],
                            Sequence<T,Allocator>& sequence,
                            const bool break_on_fail = true)
   {
      T tmp;
      for (int i = 0; i < argc; ++i)
      {
         if (!string_to_type_converter(std::string(argv[i]),tmp))
         {
            if (break_on_fail)
               return i;
            else
               continue;
         }
         sequence.push_back(tmp);
      }
      return argc;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3, T4& t4,
                            T5& t5, T6& t6, T7& t7, T8& t8,
                            T9& t9)

   {
      if (9 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[3]),t4)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[4]),t5)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[5]),t6)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[6]),t7)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[7]),t8)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[8]),t9)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3, T4& t4,
                            T5& t5, T6& t6, T7& t7, T8& t8)

   {
      if (8 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[3]),t4)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[4]),t5)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[5]),t6)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[6]),t7)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[7]),t8)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3, T4& t4,
                            T5& t5, T6& t6, T7& t7)

   {
      if (7 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[3]),t4)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[4]),t5)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[5]),t6)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[6]),t7)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3, T4& t4,
                            T5& t5, T6& t6)

   {
      if (6 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[3]),t4)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[4]),t5)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[5]),t6)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2, typename T3, typename T4, typename T5>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3, T4& t4, T5& t5)
   {
      if (5 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[3]),t4)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[4]),t5)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2, typename T3, typename T4>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3, T4& t4)
   {
      if (4 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[3]),t4)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2, typename T3>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2, T3& t3)
   {
      if (3 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[2]),t3)) return result; ++result;
      return result;
   }

   template <typename T1, typename T2>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1, T2& t2)
   {
      if (2 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      if (!string_to_type_converter(std::string(argv[1]),t2)) return result; ++result;
      return result;
   }

   template <typename T1>
   inline std::size_t parse(const int& argc, char* argv[],
                            T1& t1)
   {
      if (1 != argc) return 0;
      std::size_t result = 0;
      if (!string_to_type_converter(std::string(argv[0]),t1)) return result; ++result;
      return result;
   }

   #define strtk_parse_begin(Type)                                            \
   namespace strtk {                                                          \
   bool parse(const std::string& data, const std::string& delimiters, Type& t)\
   { return parse(data,delimiters                                             \

   #define strtk_parse_type(T) \
   ,t.T                        \

   #define strtk_parse_hex_type(T) \
   ,t.T                            \

   #define strtk_parse_ignore_token() \
   ,ignore_token()                    \

   #define strtk_parse_end() \
   );}}                      \

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              Sequence<T,Allocator>& sequence,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     sequence,
                     split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::set<T,Comparator,Allocator>& set,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     set,
                     split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::multiset<T,Comparator,Allocator>& multiset,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     multiset,
                     split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::queue<T,Container>& queue,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     queue,
                     split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::stack<T,Container>& stack,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     stack,
                     split_option);
   }

   template <typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              std::priority_queue<T,Container,Comparator>& priority_queue,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     priority_queue,
                     split_option);
   }

   template <typename T>
   inline std::size_t parse_n(const std::string& data,
                              const std::string& delimiters,
                              const std::size_t& n,
                              T* out,
                              const split_options::type& split_option = split_options::compress_delimiters)
   {
      return parse_n(data.data(),
                     data.data() + data.size(),
                     delimiters,
                     n,
                     out,
                     split_option);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11, typename T12>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                          T7& t7, T8& t8, T9& t9, T10& t10, T11& t11, T12& t12)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6,
                          t7,t8,t9,t10,t11,t12);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                          T7& t7, T8& t8, T9& t9, T10& t10, T11& t11)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6,
                          t7,t8,t9,t10,t11);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8,
             typename T9, typename T10>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4&  t4, T5& t5, T6& t6,
                          T7& t7, T8& t8, T9& t9, T10& t10)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6,
                          t7,t8,t9,t10);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8,
             typename T9>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                          T7& t7, T8& t8, T9& t9)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6,
                          t7,t8,t9);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7, typename T8>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                          T7& t7, T8& t8)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6,
                          t7,t8);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6, typename  T7>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                          T7& t7)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6,t7);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5, typename  T6>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5,t6);
   }

   template <typename T1, typename  T2, typename  T3, typename T4,
             typename T5>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4, T5& t5)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4,t5);
   }

   template <typename T1, typename  T2, typename  T3, typename T4>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3, T4& t4)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3,t4);
   }

   template <typename T1, typename  T2, typename  T3>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2, T3& t3)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2,t3);
   }

   template <typename T1, typename  T2>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1, T2& t2)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1,t2);
   }

   template <typename T1>
   inline bool parse_line(std::ifstream& stream,
                          const std::string& delimiters,
                          T1& t1)
   {
      if (!stream)
         return false;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return false;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          t1);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse_line(std::ifstream& stream,
                                 const std::string& delimiters,
                                 Sequence<T,Allocator>& sequence,
                                 const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          sequence,
                          split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_line(std::ifstream& stream,
                                 const std::string& delimiters,
                                 std::set<T,Comparator,Allocator>& set,
                                 const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          set,
                          split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_line(std::ifstream& stream,
                                 const std::string& delimiters,
                                 std::multiset<T,Comparator,Allocator>& multiset,
                                 const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          multiset,
                          split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse_line(std::ifstream& stream,
                                 const std::string& delimiters,
                                 std::queue<T,Container>& queue,
                                 const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          queue,
                          split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse_line(std::ifstream& stream,
                                 const std::string& delimiters,
                                 std::stack<T,Container>& stack,
                                 const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          stack,
                          split_option);
   }

   template <typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse_line(std::ifstream& stream,
                                 const std::string& delimiters,
                                 std::priority_queue<T,Container,Comparator>& priority_queue,
                                 const split_options::type& split_option = split_options::compress_delimiters)

   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return false;
      return strtk::parse(data.data(),
                          data.data() + data.size(),
                          delimiters,
                          priority_queue,
                          split_option);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::size_t parse_line_n(std::ifstream& stream,
                                   const std::string& delimiters,
                                   const std::size_t& n,
                                   Sequence<T,Allocator>& sequence,
                                   const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return 0;
      return strtk::parse_n(data.data(),
                            data.data() + data.size(),
                            delimiters,
                            n,
                            sequence,
                            split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_line_n(std::ifstream& stream,
                                   const std::string& delimiters,
                                   const std::size_t& n,
                                   std::set<T,Comparator,Allocator>& set,
                                   const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return 0;
      return strtk::parse_n(data.data(),
                            data.data() + data.size(),
                            delimiters,
                            n,
                            set,
                            split_option);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::size_t parse_line_n(std::ifstream& stream,
                                   const std::string& delimiters,
                                   const std::size_t& n,
                                   std::multiset<T,Comparator,Allocator>& multiset,
                                   const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return 0;
      return strtk::parse_n(data.data(),
                            data.data() + data.size(),
                            delimiters,
                            n,
                            multiset,
                            split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse_line_n(std::ifstream& stream,
                                   const std::string& delimiters,
                                   const std::size_t& n,
                                   std::queue<T,Container>& queue,
                                   const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return 0;
      return strtk::parse_n(data.data(),
                            data.data() + data.size(),
                            delimiters,
                            n,
                            queue,
                            split_option);
   }

   template <typename T,
             typename Container>
   inline std::size_t parse_line_n(std::ifstream& stream,
                                   const std::string& delimiters,
                                   const std::size_t& n,
                                   std::stack<T,Container>& stack,
                                   const split_options::type& split_option = split_options::compress_delimiters)
   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return 0;
      return strtk::parse_n(data.data(),
                            data.data() + data.size(),
                            delimiters,
                            n,
                            stack,
                            split_option);
   }

   template <typename T,
             typename Container,
             typename Comparator>
   inline std::size_t parse_line_n(std::ifstream& stream,
                                   const std::string& delimiters,
                                   const std::size_t& n,
                                   std::priority_queue<T,Container,Comparator>& priority_queue,
                                   const split_options::type& split_option = split_options::compress_delimiters)

   {
      if (!stream)
         return 0;
      std::string data;
      data.reserve(strtk::one_kilobyte);
      if (!std::getline(stream,data))
         return 0;
      if (data.empty() || delimiters.empty())
         return 0;
      return strtk::parse_n(data.data(),
                            data.data() + data.size(),
                            delimiters,
                            n,
                            priority_queue,
                            split_option);
   }

   template <typename T1, typename T2, typename  T3, typename  T4,
             typename T5, typename T6, typename  T7, typename  T8,
             typename T9, typename T10, typename T11, typename T12>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const  T2&  t2, const  T3&  t3, const  T4&  t4,
                         const T5& t5, const  T6&  t6, const  T7&  t7, const  T8&  t8,
                         const T9& t9, const T10& t10, const T11& t11, const T12& t12)
   {
      output += type_to_string( t1); output += delimiter;
      output += type_to_string( t2); output += delimiter;
      output += type_to_string( t3); output += delimiter;
      output += type_to_string( t4); output += delimiter;
      output += type_to_string( t5); output += delimiter;
      output += type_to_string( t6); output += delimiter;
      output += type_to_string( t7); output += delimiter;
      output += type_to_string( t8); output += delimiter;
      output += type_to_string( t9); output += delimiter;
      output += type_to_string(t10); output += delimiter;
      output += type_to_string(t11); output += delimiter;
      output += type_to_string(t12);
   }

   template <typename T1, typename T2, typename  T3, typename T4,
             typename T5, typename T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const  T2&  t2, const T3& t3, const T4& t4,
                         const T5& t5, const  T6&  t6, const T7& t7, const T8& t8,
                         const T9& t9, const T10& t10, const T11& t11)
   {
      output += type_to_string( t1); output += delimiter;
      output += type_to_string( t2); output += delimiter;
      output += type_to_string( t3); output += delimiter;
      output += type_to_string( t4); output += delimiter;
      output += type_to_string( t5); output += delimiter;
      output += type_to_string( t6); output += delimiter;
      output += type_to_string( t7); output += delimiter;
      output += type_to_string( t8); output += delimiter;
      output += type_to_string( t9); output += delimiter;
      output += type_to_string(t10); output += delimiter;
      output += type_to_string(t11);
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9, typename T10>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                         const T5& t5, const T6& t6, const T7& t7, const T8& t8,
                         const T9& t9, const T10& t10)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4); output += delimiter;
      output += type_to_string(t5); output += delimiter;
      output += type_to_string(t6); output += delimiter;
      output += type_to_string(t7); output += delimiter;
      output += type_to_string(t8); output += delimiter;
      output += type_to_string(t9); output += delimiter;
      output += type_to_string(t10);
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                         const T5& t5, const T6& t6, const T7& t7, const T8& t8,
                         const T9& t9)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4); output += delimiter;
      output += type_to_string(t5); output += delimiter;
      output += type_to_string(t6); output += delimiter;
      output += type_to_string(t7); output += delimiter;
      output += type_to_string(t8); output += delimiter;
      output += type_to_string(t9);
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                         const T5& t5, const T6& t6, const T7& t7, const T8& t8)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4); output += delimiter;
      output += type_to_string(t5); output += delimiter;
      output += type_to_string(t6); output += delimiter;
      output += type_to_string(t7); output += delimiter;
      output += type_to_string(t8);
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                         const T5& t5, const T6& t6, const T7& t7)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4); output += delimiter;
      output += type_to_string(t5); output += delimiter;
      output += type_to_string(t6); output += delimiter;
      output += type_to_string(t7);
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5,typename T6>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                         const T5& t5, const T6& t6)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4); output += delimiter;
      output += type_to_string(t5); output += delimiter;
      output += type_to_string(t6);
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                         const T5& t5)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4); output += delimiter;
      output += type_to_string(t5);
   }

   template <typename T1, typename T2, typename T3, typename T4>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3, const T4& t4)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3); output += delimiter;
      output += type_to_string(t4);
   }

   template <typename T1, typename T2, typename T3>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2, const T3& t3)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2); output += delimiter;
      output += type_to_string(t3);
   }

   template <typename T1, typename T2>
   inline void construct(std::string& output,
                         const std::string& delimiter,
                         const T1& t1, const T2& t2)
   {
      output += type_to_string(t1); output += delimiter;
      output += type_to_string(t2);
   }

   template <typename InputIterator>
   inline void join(std::string& output,
                    const std::string& delimiter,
                    const InputIterator begin,
                    const InputIterator end)
   {
      InputIterator itr = begin;
      while (end != itr)
      {
         output += type_to_string(*itr);
         if (end == (++itr))
            break;
         else
            output += delimiter;
      }
   }

   template <typename InputIterator>
   inline void join(std::string& output,
                    const std::string& delimiter,
                    const std::pair<InputIterator,InputIterator>& range)
   {
      InputIterator itr = range.first;
      while (range.second != itr)
      {
         output += type_to_string(*itr);
         if (range.second == (++itr))
            break;
         else
            output += delimiter;
      }
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void join(std::string& output,
                    const std::string& delimiter,
                    const Sequence<T,Allocator>& sequence)
   {
      join(output,delimiter,sequence.begin(),sequence.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void join(std::string& output,
                    const std::string& delimiter,
                    const std::set<T,Comparator,Allocator>& set)
   {
      join(output,delimiter,set.begin(),set.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void join(std::string& output,
                    const std::string& delimiter,
                    const std::multiset<T,Comparator,Allocator>& multiset)
   {
      join(output,delimiter,multiset.begin(),multiset.end());
   }

   inline void join(std::string& output,
                    const std::string& delimiter,
                    int argc, char* argv[])
   {
      for (int i = 0; i < argc; ++i)
      {
         output += argv[i];
         if (i < (argc - 1))
            output += delimiter;
      }
   }

   template <typename InputIterator>
   inline std::string join(const std::string& delimiter,
                           const InputIterator begin,
                           const InputIterator end)
   {
      std::string output;
      output.reserve(one_kilobyte);
      join(output,delimiter,begin,end);
      return output;
   }

   template <typename InputIterator>
   inline std::string join(const std::string& delimiter,
                           const std::pair<InputIterator,InputIterator>& range)
   {
      std::string output;
      output.reserve(one_kilobyte);
      join(output,delimiter,range.first,range.second);
      return output;
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::string join(const std::string& delimiter,
                           const Sequence<T,Allocator>& sequence)
   {
      if (sequence.empty())
         return "";
      else
         return join(delimiter,sequence.begin(),sequence.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::string join(const std::string& delimiter,
                           const std::set<T,Comparator,Allocator>& set)
   {
      if (set.empty())
         return "";
      else
         return join(delimiter,set.begin(),set.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::string join(const std::string& delimiter,
                           const std::multiset<T,Comparator,Allocator>& multiset)
   {
      if (multiset.empty())
         return "";
      else
         return join(delimiter,multiset.begin(),multiset.end());
   }

   inline std::string join(const std::string& delimiter, int argc, char* argv[])
   {
      std::string result;
      result.reserve(one_kilobyte);
      join(result,delimiter,argc,argv);
      return result;
   }

   template <typename InputIterator, typename Predicate>
   inline void join_if(std::string& output,
                       const std::string& delimiter,
                       Predicate predicate,
                       const InputIterator begin,
                       const InputIterator end)
   {
      InputIterator itr = begin;
      bool first_time = true;
      while (end != itr)
      {
         if (predicate(*itr))
         {
            if (!first_time)
               output += delimiter;
            else
               first_time = false;
            output += type_to_string(*itr);
         }
         if (end == (++itr))
            break;
      }
   }

   template <typename InputIterator, typename Predicate>
   inline void join_if(std::string& output,
                       const std::string& delimiter,
                       Predicate predicate,
                       const std::pair<InputIterator,InputIterator>& range)
   {
      InputIterator itr = range.first;
      bool first_time = true;
      while (range.second != itr)
      {
         if (predicate(*itr))
         {
            if (!first_time)
               output += delimiter;
            else
               first_time = false;
            output += type_to_string(*itr);
         }
         if (range.second == (++itr))
            break;
      }
   }

   template <typename T,
             typename Predicate,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void join_if(std::string& output,
                       const std::string& delimiter,
                       Predicate predicate,
                       const Sequence<T,Allocator>& sequence)
   {
      join_if(output,delimiter,predicate,sequence.begin(),sequence.end());
   }

   template <typename T,
             typename Predicate,
             typename Comparator,
             typename Allocator>
   inline void join_if(std::string& output,
                       const std::string& delimiter,
                       Predicate predicate,
                       const std::set<T,Comparator,Allocator>& set)
   {
      join_if(output,delimiter,predicate,set.begin(),set.end());
   }

   template <typename T,
             typename Predicate,
             typename Comparator,
             typename Allocator>
   inline void join_if(std::string& output,
                       const std::string& delimiter,
                       Predicate predicate,
                       const std::multiset<T,Comparator,Allocator>& multiset)
   {
      join_if(output,delimiter,predicate,multiset.begin(),multiset.end());
   }

   template <typename InputIterator, typename Predicate>
   inline std::string join_if(const std::string& delimiter,
                              Predicate predicate,
                              const InputIterator begin,
                              const InputIterator end)
   {
      std::string output;
      output.reserve(one_kilobyte);
      join_if(output,delimiter,predicate,begin,end);
      return output;
   }

   template <typename InputIterator, typename Predicate>
   inline std::string join_if(const std::string& delimiter,
                              Predicate predicate,
                              const std::pair<InputIterator,InputIterator>& range)
   {
      std::string output;
      output.reserve(one_kilobyte);
      join_if(output,delimiter,predicate,range.first,range.second);
      return output;
   }

   template <typename T,
             typename Predicate,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::string join_if(const std::string& delimiter,
                              Predicate predicate,
                              const Sequence<T,Allocator>& sequence)
   {
      return join(delimiter,predicate,sequence.begin(),sequence.end());
   }

   template <typename T,
             typename Predicate,
             typename Comparator,
             typename Allocator>
   inline std::string join_if(const std::string& delimiter,
                              Predicate predicate,
                              const std::set<T,Comparator,Allocator>& set)
   {
      return join_if(delimiter,predicate,set.begin(),set.end());
   }

   template <typename T,
             typename Predicate,
             typename Comparator,
             typename Allocator>
   inline std::string join_if(const std::string& delimiter,
                              Predicate predicate,
                              const std::multiset<T,Comparator,Allocator>& multiset)
   {
      return join_if(delimiter,predicate,multiset.begin(),multiset.end());
   }

   class build_string
   {
   public:

      build_string(const std::size_t& initial_size = 64)
      {
         data_.reserve(initial_size);
      }

      template <typename T>
      inline build_string& operator << (const T& t)
      {
         data_ += type_to_string(t);
         return (*this);
      }

      inline build_string& operator << (const std::string& s)
      {
         data_ += s;
         return (*this);
      }

      inline std::string to_str() const
      {
         return data_;
      }

      inline operator const char* () const
      {
         return data_.data();
      }

   private:

      std::string data_;
   };

   inline void replicate(const std::size_t& n,
                         const std::string& str,
                         std::string& output)
   {
      if (0 == n) return;
      output.reserve(output.size() + (str.size() * n));
      for (std::size_t i = 0; i < n; ++i)
      {
         output.append(str);
      }
   }

   inline std::string replicate(const std::size_t& n,
                                const std::string& str)
   {
      std::string output;
      replicate(n,str,output);
      return output;
   }

   inline void replicate_inplace(const std::size_t& n,
                                 std::string& str)
   {
      std::string temp_str = str;
      str.reserve(str.size() + (str.size() * n));

      for (std::size_t i = 0; i < n; ++i)
      {
         str.append(temp_str);
      }
   }

   template <typename InputIterator>
   inline void bracketize(std::string& output,
                          const std::string& pre,
                          const std::string& post,
                          const InputIterator begin,
                          const InputIterator end)
   {
      InputIterator itr = begin;
      std::string s;
      s.reserve(one_kilobyte);
      while (end != itr)
      {
         s.clear();
         s.append(pre);
         s.append(type_to_string(*itr));
         s.append(post);
         output.append(s);
         ++itr;
      }
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void bracketize(std::string& output,
                          const std::string& pre,
                          const std::string& post,
                          Sequence<T,Allocator>& sequence)
   {
      bracketize(output,pre,post,sequence.begin(),sequence.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void bracketize(std::string& output,
                          const std::string& pre,
                          const std::string& post,
                          std::set<T,Comparator,Allocator>& set)
   {
      bracketize(output,pre,post,set.begin(),set.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void bracketize(std::string& output,
                          const std::string& pre,
                          const std::string& post,
                          std::multiset<T,Comparator,Allocator>& multiset)
   {
      bracketize(output,pre,post,multiset.begin(),multiset.end());
   }

   template <typename InputIterator>
   inline std::string bracketize(const std::string& pre,
                                 const std::string& post,
                                 const InputIterator begin,
                                 const InputIterator end)
   {
      std::string output;
      output.reserve(one_kilobyte);
      bracketize(output,pre,post,begin,end);
      return output;
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline std::string bracketize(const std::string& pre,
                                 const std::string& post,
                                 Sequence<T,Allocator>& sequence)
   {
      return bracketize(pre,post,sequence.begin(),sequence.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::string bracketize(const std::string& pre,
                                 const std::string& post,
                                 std::set<T,Comparator,Allocator>& set)
   {
      return bracketize(pre,post,set.begin(),set.end());
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline std::string bracketize(const std::string& pre,
                                 const std::string& post,
                                 std::multiset<T,Comparator,Allocator>& multiset)
   {
      return bracketize(pre,post,multiset.begin(),multiset.end());
   }

   template <typename T>
   struct interval_inserter
   {
      typedef T type;

      interval_inserter(const std::size_t& interval, const T& t)
      : count_(0),
        interval_(interval),
        t_(t)
      {}

      inline bool operator()(const type&)
      {
         if (++count_ == interval_)
         {
            count_ = 0;
            return true;
         }
         else
            return false;
      }

      inline T operator()()
      {
         return t_;
      }

   private:

      std::size_t count_;
      std::size_t interval_;
      T t_;
   };

   template <typename Inserter,
             typename InputIterator,
             typename OutputIterator>
   inline std::size_t inserter(Inserter ins,
                               const InputIterator begin, const InputIterator end,
                               OutputIterator out)
   {
      std::size_t size = 0;
      InputIterator itr = begin;
      while (end != itr)
      {
         (*out) = (*itr);
         ++out;
         if (ins(*itr++))
         {
            (*out) = ins();
            ++out;
            size += 2;
         }
         else
            ++size;
      }
      return size;
   }

   template <typename Iterator, typename T>
   inline void iota(Iterator begin, Iterator end, T value)
   {
      Iterator itr = begin;
      while (end != itr)
      {
         (*itr) = value++;
         ++itr;
      }
   }

   template <typename T>
   inline void iota(typename range::adapter<T>& r, T value)
   {
      iota(r.begin(),r.end(),value);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void iota(Sequence<T,Allocator>& sequence, std::size_t count, T value)
   {
      while (count)
      {
         sequence.push_back(value++);
         --count;
      }
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void iota(std::set<T,Comparator,Allocator>& set, std::size_t count, T value)
   {
      while (count)
      {
         set.insert(value++);
         --count;
      }
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void iota(std::multiset<T,Comparator,Allocator>& multiset, std::size_t count, T value)
   {
      while (count)
      {
         multiset.insert(value++);
         --count;
      }
   }

   template <typename OutputIterator, typename T>
   inline void iota(std::size_t count, T value, OutputIterator out)
   {
      while (count)
      {
         (*out) = value++;
         ++out;
         --count;
      }
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void iota(Sequence<T,Allocator>& sequence, const T& value)
   {
      strtk::iota(sequence.begin(),sequence.end(),value);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void iota(std::set<T,Comparator,Allocator>& set, const T& value)
   {
      strtk::iota(set.begin(),set.end(),value);
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline void iota(std::multiset<T,Comparator,Allocator>& multiset, const T& value)
   {
      strtk::iota(multiset.begin(),multiset.end(),value);
   }

   template <typename InputIterator, typename OutputIterator>
   inline void cut(const std::size_t& r0, const std::size_t& r1,
                   const InputIterator begin, InputIterator end,
                   OutputIterator out)
   {
      // static assert: InputIterator must be of type std::string
      InputIterator itr = begin;
      while (end != itr)
      {
         const std::string& s = (*itr);
         ++itr;
         if (s.size() < r0)
            continue;
         (*out++) = s.substr(r0,std::min(r1,s.size()) - r0);
      }
   }

   template <typename Allocator,
             template <typename,typename> class Sequence,
             typename OutputIterator>
   inline void cut(const std::size_t& r0, const std::size_t& r1,
                   const Sequence<std::string, Allocator>& sequence,
                   OutputIterator out)
   {
      cut(r0,r1,sequence.begin(),sequence.end(),out);
   }

   template <typename Iterator>
   inline void cut_inplace(const std::size_t& r0, const std::size_t& r1,
                           const Iterator begin, const Iterator end)
   {
      // static assert: InputIterator must be of type std::string
      Iterator itr = begin;
      while (end != itr)
      {
         if ((*itr).size() >= r0)
         {
            (*itr) = (*itr).substr(r0,std::min(r1,(*itr).size()) - r0);
         }
         ++itr;
      }
   }

   template <typename Allocator,
             template <typename,typename> class Sequence>
   inline void cut(const std::size_t& r0, const std::size_t& r1,
                   const Sequence<std::string, Allocator>& sequence)
   {
      cut(r0,r1,sequence.begin(),sequence.end());
   }

   template <typename Comparator, typename Allocator>
   inline void cut(const std::size_t& r0, const std::size_t& r1,
                   const std::set<std::string, Comparator, Allocator>& set)
   {
      cut(r0,r1,set.begin(),set.end());
   }

   template <typename Comparator, typename Allocator>
   inline void cut(const std::size_t& r0, const std::size_t& r1,
                   const std::multiset<std::string, Comparator, Allocator>& multiset)
   {
      cut(r0,r1,multiset.begin(),multiset.end());
   }

   class translation_table
   {
   public:

      translation_table(const std::string& itable, const std::string& otable)
      {
         if (itable.size() != otable.size())
         {
            throw std::runtime_error("translation_table() - Input/Output table size mismatch.");
         }
         strtk::iota(table_, table_ + 256, static_cast<unsigned char>(0));
         for (std::size_t i = 0; i < itable.size(); ++i)
         {
            table_[static_cast<unsigned int>(itable[i])] = static_cast<unsigned char>(otable[i]);
         }
      }

      inline char operator()(const char c) const
      {
         return static_cast<char>(table_[static_cast<unsigned int>(c)]);
      }

      inline unsigned char operator()(const unsigned char c) const
      {
         return static_cast<unsigned char>(table_[static_cast<unsigned int>(c)]);
      }

   private:

      unsigned char table_[256];
   };

   inline std::string translate(const translation_table& trans_table, const std::string& s)
   {
      std::string result = s;
      std::transform(result.begin(),result.end(),result.begin(),trans_table);
      return result;
   }

   inline void translate_inplace(const translation_table& trans_table, std::string& s)
   {
      std::transform(s.begin(),s.end(),s.begin(),trans_table);
   }

   #ifdef strtk_enable_random
   inline void generate_random_data(unsigned char* data,
                                    std::size_t length,
                                    unsigned int pre_gen_cnt = 0,
                                    unsigned int seed = magic_seed)
   {
      boost::mt19937 rng(static_cast<boost::mt19937::result_type>(seed));
      boost::uniform_int<unsigned int> dist(std::numeric_limits<unsigned int>::min(),std::numeric_limits<unsigned int>::max());
      boost::variate_generator<boost::mt19937&, boost::uniform_int<unsigned int> > rnd(rng,dist);

      if (pre_gen_cnt > 0)
      {
         while (pre_gen_cnt--) rnd();
      }

      unsigned char* itr = data;
      unsigned int* x = 0;
      while (length >= sizeof(unsigned int))
      {
         x = reinterpret_cast<unsigned int*>(itr);
         (*x) = rnd();
         itr += sizeof(unsigned int);
         length -= sizeof(unsigned int);
      }

      if (length > 0)
      {
         itr -= (sizeof(unsigned int) - length);
         x = reinterpret_cast<unsigned int*>(itr);
         (*x) = rnd();
      }
   }

   namespace details
   {
      struct rand_int_type_tag {};
      struct rand_real_type_tag {};

      template <typename T> struct supported_random_type {};

      #define strtk_register_rand_int_type_tag(T) \
      template<> struct supported_random_type<T> { typedef rand_int_type_tag type;  enum { value = true }; };

      #define strtk_register_rand_real_type_tag(T) \
      template<> struct supported_random_type<T> { typedef rand_real_type_tag type;  enum { value = true }; };

      strtk_register_rand_int_type_tag(char)
      strtk_register_rand_int_type_tag(unsigned char)

      strtk_register_rand_int_type_tag(short)
      strtk_register_rand_int_type_tag(int)
      strtk_register_rand_int_type_tag(long)
      strtk_register_rand_int_type_tag(unsigned short)
      strtk_register_rand_int_type_tag(unsigned int)
      strtk_register_rand_int_type_tag(unsigned long)

      strtk_register_rand_real_type_tag(float)
      strtk_register_rand_real_type_tag(double)
      strtk_register_rand_real_type_tag(long double)

      #undef strtk_register_rand_int_type_tag
      #undef strtk_register_rand_real_type_tag

      template <typename T, typename OutputIterator, typename RandomNumberGenerator>
      inline void generate_random_values_impl(const std::size_t& count,
                                              const T& min,
                                              const T& max,
                                              OutputIterator out,
                                              RandomNumberGenerator& rng,
                                              rand_int_type_tag)
      {
         // Note: The implied range will be: [min,max]
         using namespace boost;
         variate_generator<RandomNumberGenerator&,uniform_int<T> > rnd(rng,uniform_int<T>(min,max));
         for (std::size_t i = 0; i < count; ++i, *out++ = rnd()) ;
      }

      template <typename T, typename OutputIterator, typename RandomNumberGenerator>
      inline void generate_random_values_impl(const std::size_t& count,
                                              const T& min,
                                              const T& max,
                                              OutputIterator out,
                                              RandomNumberGenerator& rng,
                                              rand_real_type_tag)
      {
         // Note: The implied range will be: [min,max)
         using namespace boost;
         variate_generator<RandomNumberGenerator&, uniform_real<T> > rnd(rng,uniform_real<T>(min,max));
         for (std::size_t i = 0; i < count; ++i, *out++ = rnd()) ;
      }

   } // namespace details

   class uniform_real_rng
   {
   private:

      typedef boost::mt19937 rng_type;
      typedef boost::variate_generator<rng_type, boost::uniform_real<double> > variate_type;

   public:

      uniform_real_rng(const std::size_t& seed = magic_seed,
                       std::size_t pregen = 0)
      : rng_(static_cast<rng_type::result_type>(seed)),
        rnd_(rng_,boost::uniform_real<double>(0.0,1.0))
      {
         while (pregen--) rng_();
      }

      inline double operator()()
      {
         return rnd_();
      }

   private:

      rng_type rng_;
      variate_type rnd_;
   };

   template <typename T, typename OutputIterator>
   inline void generate_random_values(const std::size_t& count,
                                      const T& min,
                                      const T& max,
                                      OutputIterator out,
                                      const std::size_t& seed = magic_seed,
                                      const std::size_t& pregen = 0)
   {
      typename details::supported_random_type<T>::type type;
      boost::mt19937 rng(static_cast<boost::mt19937::result_type>(seed));
      for (std::size_t i = 0; i++ < pregen; rng()) ;
      generate_random_values_impl(count,min,max,out,rng,type);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void generate_random_values(const std::size_t& count,
                                      const T& min,
                                      const T& max,
                                      Sequence<T,Allocator>& sequence,
                                      const std::size_t& seed = magic_seed,
                                      const std::size_t& pregen = 0)
   {
      typename details::supported_random_type<T>::type type;
      boost::mt19937 rng(static_cast<boost::mt19937::result_type>(seed));
      for (std::size_t i = 0; i++ < pregen; rng()) ;
      generate_random_values_impl(count,min,max,std::back_inserter(sequence),rng,type);
   }

   template <typename Iterator,
             typename RandomNumberGenerator,
             typename OutputIterator>
   inline void random_permutation(const Iterator begin, const Iterator end,
                                  RandomNumberGenerator& rng,
                                  OutputIterator out)
   {
      const std::size_t size = std::distance(begin,end);
      if ((rng. min() < 0.0) || (rng.max() > 1.0)) return;
      std::deque<std::size_t> index;
      for (std::size_t i = 0; i < size; index.push_back(i++)) ;
      while (!index.empty())
      {
         std::size_t idx = static_cast<std::size_t>(index.size() * rng());
         (*out) = *(begin + index[idx]);
         index.erase(index.begin() + idx);
         ++out;
      }
   }

   template <typename Iterator,
             typename OutputIterator>
   inline void random_permutation(const Iterator begin, const Iterator end,
                                  OutputIterator out,
                                  const std::size_t& seed = magic_seed,
                                  const std::size_t& pregen = 0)
   {
      boost::mt19937 rng(static_cast<boost::mt19937::result_type>(seed));
      for (std::size_t i = 0; i++ < pregen; rng()) ;
      boost::uniform_real<double> dist(0.0,1.0);
      boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd(rng,dist);
      random_permutation(begin,end,rnd,out);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence,
             typename OutputIterator>
   inline void random_permutation(const Sequence<T,Allocator>& sequence,
                                  OutputIterator out,
                                  const std::size_t& seed = magic_seed,
                                  const std::size_t& pregen = 0)
   {
      random_permutation(sequence.begin(),sequence.end(),out,seed,pregen);
   }

   template <typename Iterator,
             typename RandomNumberGenerator,
             typename OutputIterator>
   inline bool random_combination(const Iterator begin, const Iterator end,
                                  std::size_t set_size,
                                  RandomNumberGenerator& rng,
                                  OutputIterator out)
   {
      const std::size_t size = std::distance(begin,end);
      if ((size < set_size) || (rng. min() < 0.0) || (rng.max() > 1.0)) return false;
      std::deque<std::size_t> index;
      for (std::size_t i = 0; i < size; index.push_back(i++)) ;
      while (set_size)
      {
         std::size_t idx = static_cast<std::size_t>(index.size() * rng());
         (*out) = *(begin + index[idx]);
         index.erase(index.begin() + idx);
         ++out;
         --set_size;
      }
      return true;
   }

   template <typename Iterator,
             typename OutputIterator>
   inline void random_combination(const Iterator begin, const Iterator end,
                                  const std::size_t& set_size,
                                  OutputIterator out,
                                  const std::size_t& seed = magic_seed,
                                  const std::size_t& pregen = 0)
   {
      boost::mt19937 rng(static_cast<boost::mt19937::result_type>(seed));
      for (std::size_t i = 0; i++ < pregen; rng()) ;
      boost::uniform_real<double> dist(0.0,1.0);
      boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd(rng,dist);
      random_combination(begin,end,set_size,rnd,out);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence,
             typename OutputIterator>
   inline void random_combination(const Sequence<T,Allocator>& sequence,
                                  const std::size_t& set_size,
                                  OutputIterator out,
                                  const std::size_t& seed = magic_seed,
                                  const std::size_t& pregen = 0)
   {
      random_combination(sequence.begin(),sequence.end(),set_size,out,seed,pregen);
   }

   template <typename Iterator,
             typename OutputIterator,
             typename RandomNumberGenerator>
   inline std::size_t select_k_randomly(const Iterator begin, const Iterator end,
                                        const std::size_t k,
                                        OutputIterator out,
                                        RandomNumberGenerator& rng)
   {
      typedef typename std::iterator_traits<Iterator>::value_type T;
      std::vector<T> selection;
      selection.resize(k);
      Iterator itr = begin;
      std::size_t index = 0;
      while ((index < k) && (end != itr))
      {
         selection[index] = (*itr);
         ++index;
         ++itr;
      }
      if (0 == index)
         return 0;
      else if (index < k)
      {
         std::copy(selection.begin(),selection.begin() + index, out);
         return index;
      }
      double n = k + 1;
      while (end != itr)
      {
         if (rng() < (k / n))
         {
            selection[static_cast<std::size_t>(rng() * k)] = (*itr);
         }
         ++itr;
         ++n;
      }
      std::copy(selection.begin(),selection.end(),out);
      return k;
   }

   template <typename Iterator,
             typename OutputIterator,
             typename RandomNumberGenerator>
   inline void select_1_randomly(const Iterator begin, const Iterator end,
                                 OutputIterator out,
                                 RandomNumberGenerator& rng)
   {
      typedef typename std::iterator_traits<Iterator>::value_type T;
      T selection;
      if (begin == end)
         return;
      Iterator itr = begin;
      std::size_t n = 0;
      while (end != itr)
      {
         if (rng() < (1.0 / ++n))
         {
            selection = (*itr);
         }
         ++itr;
      }
      (*out) = selection;
      ++out;
   }
   #endif // strtk_enable_random

   template <typename Iterator>
   inline bool next_combination(const Iterator first, Iterator k, const Iterator last)
   {
      /* Credits: Thomas Draper */
      if ((first == last) || (first == k) || (last == k))
         return false;
      Iterator itr1 = first;
      Iterator itr2 = last;
      ++itr1;
      if (last == itr1)
         return false;
      itr1 = last;
      --itr1;
      itr1 = k;
      --itr2;
      while (first != itr1)
      {
         if (*--itr1 < (*itr2))
         {
            Iterator j = k;
            while (!((*itr1) < (*j))) ++j;
            std::iter_swap(itr1,j);
            ++itr1;
            ++j;
            itr2 = k;
            std::rotate(itr1,j,last);
            while (last != j)
            {
               ++j;
               ++itr2;
            }
            std::rotate(k,itr2,last);
            return true;
         }
      }
      std::rotate(first,k,last);
      return false;
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool next_combination(Sequence<T,Allocator>& sequence, const std::size_t& size)
   {
      return next_combination(sequence.begin(), sequence.begin() + size, sequence.end());
   }

   template <typename Iterator, typename Function>
   inline void for_each_permutation(Iterator begin, Iterator end, Function function)
   {
      do
      {
         function(begin,end);
      }
      while (std::next_permutation(begin,end));
   }

   template <typename Iterator, typename Function>
   inline bool for_each_permutation_conditional(Iterator begin, Iterator end, Function function)
   {
      do
      {
         if (!function(begin,end))
            return false;
      }
      while (std::next_permutation(begin,end));
      return true;
   }

   namespace details
   {
      /*
         Credits:
         (C) Copyright Howard Hinnant 2005-2011.
         Use, modification and distribution are subject to the Boost Software License,
         Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
         http://www.boost.org/LICENSE_1_0.txt).
      */
      template <typename Iterator>
      static inline void rotate_discontinuous(Iterator first1, Iterator last1,
                                              typename std::iterator_traits<Iterator>::difference_type d1,
                                              Iterator first2, Iterator last2,
                                              typename std::iterator_traits<Iterator>::difference_type d2)
      {
         using std::swap;
         if (d1 <= d2)
            std::rotate(first2, std::swap_ranges(first1, last1, first2), last2);
         else
         {
            Iterator i1 = last1;
            while (first2 != last2)
            {
               swap(*--i1,*--last2);
            }
            std::rotate(first1, i1, last1);
         }
      }

      template <typename Iterator, class Function>
      inline void combine_discontinuous(Iterator first1, Iterator last1, typename std::iterator_traits<Iterator>::difference_type d1,
                                        Iterator first2, Iterator last2, typename std::iterator_traits<Iterator>::difference_type d2,
                                        Function& f,
                                        typename std::iterator_traits<Iterator>::difference_type d = 0)
      {
         typedef typename std::iterator_traits<Iterator>::difference_type D;
         using std::swap;
         if ((0 == d1) || (0 == d2))
            return f();
         if (1 == d1)
         {
            Iterator i2 = first2;
            while (i2 != last2)
            {
               f();
               swap(*first1, *i2);
               ++i2;
            }
         }
         else
         {
            Iterator f1p = first1;
            std::advance(f1p,1);
            Iterator i2 = first2;
            D d22 = d2;
            while (i2 != last2)
            {
               combine_discontinuous(f1p, last1, d1 - 1, i2, last2, d22, f, d + 1);
               swap(*first1, *i2);
               ++i2;
               --d22;
            }
         }
         f();
         if (0 != d)
         {
            Iterator f2p = first2;
            std::advance(f2p,1);
            rotate_discontinuous(first1, last1, d1, f2p, last2, d2 - 1);
         }
         else
            rotate_discontinuous(first1, last1, d1, first2, last2, d2);
      }

      template <typename Iterator, class Function>
      inline bool combine_discontinuous_conditional(Iterator first1, Iterator last1, typename std::iterator_traits<Iterator>::difference_type d1,
                                                    Iterator first2, Iterator last2, typename std::iterator_traits<Iterator>::difference_type d2,
                                                    Function& f,
                                                    typename std::iterator_traits<Iterator>::difference_type d = 0)
      {
         typedef typename std::iterator_traits<Iterator>::difference_type D;
         using std::swap;
         if (d1 == 0 || d2 == 0)
            return f();
         if (d1 == 1)
         {
            for (Iterator i2 = first2; i2 != last2; ++i2)
            {
               if (!f())
                  return false;
               swap(*first1, *i2);
            }
         }
         else
         {
            Iterator f1p = first1;
            std::advance(f1p,1);
            Iterator i2 = first2;
            for (D d22 = d2; i2 != last2; ++i2, --d22)
            {
               if (!combine_discontinuous_conditional(f1p, last1, d1-1, i2, last2, d22, f, d + 1))
                  return false;
               swap(*first1, *i2);
            }
         }
         if (!f())
            return false;
         if (d != 0)
         {
            Iterator f2p = first2;
            std::advance(f2p,1);
            rotate_discontinuous(first1, last1, d1, f2p, last2, d2 - 1);
         }
         else
            rotate_discontinuous(first1, last1, d1, first2, last2, d2);
         return true;
      }

      template <class Function, typename Iterator>
      class bound_range
      {
      public:

         bound_range(Function f, Iterator first, Iterator last)
         : f_(f),
           first_(first),
           last_(last)
         {}

         inline void operator()()
         {
            f_(first_,last_);
         }

      private:

         inline bound_range& operator=(const bound_range&);

         Function f_;
         Iterator first_;
         Iterator last_;
      };

      template <class Function, typename Iterator>
      class bound_range_conditional
      {
      public:

         bound_range_conditional(Function f, Iterator first, Iterator last)
         : f_(f),
           first_(first),
           last_(last)
         {}

         inline bool operator()()
         {
            return f_(first_,last_);
         }

      private:

         inline bound_range_conditional& operator=(const bound_range_conditional&);

         Function f_;
         Iterator first_;
         Iterator last_;
      };

   }

   template <typename Iterator, typename Function>
   inline void for_each_combination(Iterator begin, Iterator end,
                                    const std::size_t& size,
                                    Function function)
   {
      if (static_cast<typename std::iterator_traits<Iterator>::difference_type>(size) > std::distance(begin,end))
         return;
      Iterator mid = begin + size;
      details::bound_range<Function&,Iterator> func(function,begin,mid);
      details::combine_discontinuous(begin, mid,
                                     std::distance(begin,mid),
                                     mid, end,
                                     std::distance(mid,end),
                                     func);
   }

   template <typename Iterator, typename Function>
   inline void for_each_combination_conditional(Iterator begin, Iterator end,
                                                const std::size_t& size,
                                                Function function)
   {
      if (static_cast<typename std::iterator_traits<Iterator>::difference_type>(size) > std::distance(begin,end))
         return;
      Iterator mid = begin + size;
      details::bound_range_conditional<Function&,Iterator> func(function,begin,mid);
      details::combine_discontinuous_conditional(begin, mid,
                                                 std::distance(begin,mid),
                                                 mid, end,
                                                 std::distance(mid,end),
                                                 func);
   }

   inline unsigned long long int n_choose_k(const unsigned long long int& n, const unsigned long long int& k)
   {
      if (n  < k) return 0;
      if (0 == n) return 0;
      if (0 == k) return 1;
      if (n == k) return 1;
      if (1 == k) return n;

      typedef unsigned long long int value_type;

      class n_choose_k_impl
      {
      public:

         n_choose_k_impl(value_type* table, const value_type& dimension)
         : table_(table),
           dimension_(dimension / 2)
         {}

         inline value_type& lookup(const value_type& n, const value_type& k)
         {
            const std::size_t difference = static_cast<std::size_t>(n - k);
            return table_[static_cast<std::size_t>((dimension_ * n) + std::min<value_type>(k,difference))];
         }

         inline value_type compute(const value_type& n, const value_type& k)
         {
            // n-Choose-k = (n-1)-Choose-(k-1) + (n-1)-Choose-k
            if ((0 == k) || (k == n))
               return 1;
            value_type v1 = lookup(n - 1,k - 1);
            if (0 == v1)
               v1 = lookup(n - 1,k - 1) = compute(n - 1,k - 1);
            value_type v2 = lookup(n - 1,k);
            if (0 == v2)
               v2 = lookup(n - 1,k) = compute(n - 1,k);
            return v1 + v2;
         }

         value_type* table_;
         const value_type dimension_;

      private:

         inline n_choose_k_impl& operator=(const n_choose_k_impl&)
         {
            return *this;
         }
      };

      static const std::size_t static_table_dim = 100;
      static const std::size_t static_table_size = static_cast<std::size_t>((static_table_dim * static_table_dim) / 2);
      static value_type static_table[static_table_size];
      static bool static_table_initialized = false;

      if (!static_table_initialized && (n <= static_table_dim))
      {
         std::fill_n(static_table,static_table_size,0);
         static_table_initialized = true;
      }

      const std::size_t table_size = static_cast<std::size_t>(n * (n / 2) + (n & 1));

      unsigned long long int dimension = static_table_dim;
      value_type* table = 0;

      if (table_size <= static_table_size)
         table = static_table;
      else
      {
         dimension = n;
         table = new value_type[table_size];
         std::fill_n(table,table_size,0ULL);
      }

      value_type result = n_choose_k_impl(table,dimension).compute(n,k);

      if (table != static_table)
         delete [] table;

      return result;
   }

   inline void initialize_n_choose_k()
   {
      const unsigned long long int max_n = 100ULL;
      for (unsigned long long int n = 0; n < max_n; ++n)
      {
         for (unsigned long long int k = 0; k < max_n; ++k)
         {
            n_choose_k(n,k);
         }
      }
   }

   template <typename OutputIterator>
   inline void nth_combination_sequence(unsigned long long int n,
                                        const std::size_t& r,
                                        const std::size_t& k,
                                        OutputIterator out,
                                        const bool complete_index = true)
   {
      //Compute the indicies for the n'th combination of r-choose-k
      //n must be in the range [0,r-choose-k)
      typedef unsigned long long int value_type;

      std::vector<std::size_t> index_list(k,0);
      value_type j = 0;
      value_type x = 0;
      ++n;

      for (std::size_t i = 1; i <= (k - 1); ++i)
      {
         index_list[i - 1] = 0;
         if (1 < i)
         {
            index_list[i - 1] = index_list[i - 2];
         }

         do
         {
            index_list[i - 1] += 1;
            j = n_choose_k(r - index_list[i - 1], k - i);
            x += j;
         }
         while (n > x);
         x -= j;
      }

      index_list[k - 1] = index_list[k - 2] + static_cast<std::size_t>(n) - static_cast<std::size_t>(x);
      for (std::size_t i = 0; i < index_list.size(); --index_list[i++]);

      std::copy(index_list.begin(),index_list.end(),out);

      if (complete_index)
      {
         std::vector<unsigned int> exist_table(r,0);

         for (std::size_t i = 0; i < index_list.size(); ++i)
         {
            exist_table[index_list[i]] = 1;
         }

         for (std::size_t i = 0; i < exist_table.size(); ++i)
         {
            if (0 == exist_table[i])
            {
               (*out) = i;
               ++out;
            }
         }
      }
   }

   template <typename InputIterator, typename OutputIterator>
   inline void nth_combination_sequence(const std::size_t& n,
                                        const std::size_t& k,
                                        const InputIterator begin,
                                        const InputIterator end,
                                        OutputIterator out,
                                        const bool complete_index = true)
   {
      const std::size_t length = std::distance(begin,end);
      std::vector<std::size_t> index_list;
      nth_combination_sequence(n,length,k,std::back_inserter(index_list),complete_index);
      for (std::size_t i = 0; i < index_list.size(); ++i)
      {
         (*out) = *(begin + index_list[i]);
         ++out;
      }
   }

   template <typename OutputIterator>
   inline void nth_permutation_sequence(std::size_t n, const std::size_t k, OutputIterator out)
   {
      //Note: n in [0,k!)
      std::vector<std::size_t> factorid (k,0);
      std::vector<std::size_t> permutate(k,0);

      factorid[0] = 1;
      for (std::size_t i = 1; i < k; ++i)
      {
         factorid[i] = factorid[i - 1] * i;
      }

      for (std::size_t i = 0; i < k; ++i)
      {
         permutate[i] = n / factorid[k - i - 1];
         n = n % factorid[k - i - 1];
      }

      for (std::size_t i = k - 1; i > 0; --i)
      {
         for (int j = static_cast<int>(i - 1); j >= 0; --j)
         {
            if (permutate[j] <= permutate[i])
            {
               ++permutate[i];
            }
         }
      }

      for (std::size_t i = 0; i < k; ++i)
      {
         *(out++) = permutate[i];
      }
   }

   template <typename InputIterator, typename OutputIterator>
   inline void nth_permutation_sequence(std::size_t n,
                                        const InputIterator begin,
                                        const InputIterator end,
                                        OutputIterator out)
   {
      const std::size_t size = std::distance(begin,end);
      std::vector<std::size_t> index_list(size,0);
      nth_permutation_sequence(n,size,index_list.begin());
      for (std::size_t i = 0; i < size; ++i)
      {
         *(out++) = (begin + index_list[i]);
      }
   }

   inline std::string nth_permutation_sequence(const std::size_t& n, const std::string& s)
   {
      std::vector<std::size_t> index_list(s.size(),0);
      nth_permutation_sequence(n,s.size(),index_list.begin());
      std::string result;
      result.reserve(s.size());
      for (std::size_t i = 0; i < index_list.size(); ++i)
      {
         result += s[index_list[i]];
      }
      return result;
   }

   template <typename Iterator>
   class combination_iterator : public std::iterator<std::forward_iterator_tag,
                                                     std::pair<Iterator,Iterator>,
                                                     void,
                                                     void>
   {
   public:

      typedef Iterator iterator;
      typedef const iterator const_iterator;
      typedef std::pair<Iterator,Iterator> range_type;

      explicit inline combination_iterator(const std::size_t& k,
                                           iterator begin, iterator end,
                                           const bool sorted = true)
      : begin_(begin),
        end_(end),
        middle_(begin + k),
        current_combination_(begin_,middle_)
      {
         if (!sorted)
         {
            std::sort(begin,end);
         }
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      explicit inline combination_iterator(const std::size_t& k,
                                           Sequence<T,Allocator>& seq,
                                           const bool sorted = true)
      : begin_(seq.begin()),
        end_(seq.end()),
        middle_(begin_ + k),
        current_combination_(begin_,middle_)
      {
         if (!sorted)
         {
            std::sort(begin_,end_);
         }
      }

      explicit inline combination_iterator(const std::size_t& k,
                                           std::string& str,
                                           const bool sorted = true)
      : begin_(const_cast<char*>(str.data())),
        end_(const_cast<char*>(str.data() + str.size())),
        middle_(begin_ + k),
        current_combination_(begin_,middle_)
      {
         if (!sorted)
         {
            std::sort(begin_,end_);
         }
      }

      inline combination_iterator(iterator end)
      : begin_(end),
        end_(end),
        middle_(end),
        current_combination_(end,end)
      {}

      inline combination_iterator(const std::string& str)
      : begin_(const_cast<char*>(str.data() + str.size())),
        end_(begin_),
        middle_(end_),
        current_combination_(end_,end_)
      {}

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      explicit inline combination_iterator(Sequence<T,Allocator>& seq)
      : begin_(seq.end()),
        end_(seq.end()),
        middle_(end_),
        current_combination_(end_,end_)
      {}

      inline combination_iterator& operator++()
      {
         if (begin_ != end_)
         {
            if (!next_combination(begin_,middle_,end_))
            {
               begin_ = middle_ = end_;
            }
         }
         return (*this);
      }

      inline combination_iterator operator++(int)
      {
         combination_iterator tmp = *this;
         this->operator++();
         return tmp;
      }

      inline combination_iterator& operator+=(const int inc)
      {
         if (inc > 0)
         {
            for (int i = 0; i < inc; ++i, ++(*this)) ;
         }
         return (*this);
      }

      inline range_type operator*() const
      {
         return current_combination_;
      }

      inline bool operator==(const combination_iterator& itr) const
      {
         return (begin_  == itr.begin_ ) &&
                (end_    == itr.end_   ) &&
                (middle_ == itr.middle_);
      }

      inline bool operator!=(const combination_iterator& itr) const
      {
         return !operator==(itr);
      }

   protected:

      iterator begin_;
      iterator end_;
      iterator middle_;
      range_type current_combination_;
   };

   namespace fast
   {
      /*
        Note: The following routines perform no sanity checks at all
              upon the input data. Hence they should only be used with
              data that is known to be completely 'valid'.
      */
      namespace details
      {

         template <typename Iterator, int N>
         struct all_digits_check_impl
         {
            static inline bool process(Iterator)
            {
               throw std::runtime_error("all_digits_check_impl - unsupported value for N.");
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,19>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      all_digits_check_impl<Iterator,18>::process(itr + 1);
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,18>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      all_digits_check_impl<Iterator,17>::process(itr + 1);
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,17>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      all_digits_check_impl<Iterator,16>::process(itr + 1);
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,16>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 9] - '0') < 10 &&
                      static_cast<unsigned char>(itr[10] - '0') < 10 &&
                      static_cast<unsigned char>(itr[11] - '0') < 10 &&
                      static_cast<unsigned char>(itr[12] - '0') < 10 &&
                      static_cast<unsigned char>(itr[13] - '0') < 10 &&
                      static_cast<unsigned char>(itr[14] - '0') < 10 &&
                      static_cast<unsigned char>(itr[15] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,15>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 9] - '0') < 10 &&
                      static_cast<unsigned char>(itr[10] - '0') < 10 &&
                      static_cast<unsigned char>(itr[11] - '0') < 10 &&
                      static_cast<unsigned char>(itr[12] - '0') < 10 &&
                      static_cast<unsigned char>(itr[13] - '0') < 10 &&
                      static_cast<unsigned char>(itr[14] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,14>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 9] - '0') < 10 &&
                      static_cast<unsigned char>(itr[10] - '0') < 10 &&
                      static_cast<unsigned char>(itr[11] - '0') < 10 &&
                      static_cast<unsigned char>(itr[12] - '0') < 10 &&
                      static_cast<unsigned char>(itr[13] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,13>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 9] - '0') < 10 &&
                      static_cast<unsigned char>(itr[10] - '0') < 10 &&
                      static_cast<unsigned char>(itr[11] - '0') < 10 &&
                      static_cast<unsigned char>(itr[12] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,12>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 9] - '0') < 10 &&
                      static_cast<unsigned char>(itr[10] - '0') < 10 &&
                      static_cast<unsigned char>(itr[11] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,11>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 9] - '0') < 10 &&
                      static_cast<unsigned char>(itr[10] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,10>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[8] - '0') < 10 &&
                      static_cast<unsigned char>(itr[9] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,9>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[7] - '0') < 10 &&
                      static_cast<unsigned char>(itr[8] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,8>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[6] - '0') < 10 &&
                      static_cast<unsigned char>(itr[7] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,7>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[5] - '0') < 10 &&
                      static_cast<unsigned char>(itr[6] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,6>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[4] - '0') < 10 &&
                      static_cast<unsigned char>(itr[5] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,5>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10 &&
                      static_cast<unsigned char>(itr[4] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,4>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[1] - '0') < 10 &&
                      static_cast<unsigned char>(itr[2] - '0') < 10 &&
                      static_cast<unsigned char>(itr[3] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,3>
         {
            static inline bool process(Iterator itr)
            {
               return
                static_cast<unsigned char>(itr[0] - '0') < 10 &&
                static_cast<unsigned char>(itr[1] - '0') < 10 &&
                static_cast<unsigned char>(itr[2] - '0') < 10;
           }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,2>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10 &&
                      static_cast<unsigned char>(itr[ 1] - '0') < 10;
           }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,1>
         {
            static inline bool process(Iterator itr)
            {
               return static_cast<unsigned char>(itr[ 0] - '0') < 10;
            }
         };

         template <typename Iterator>
         struct all_digits_check_impl<Iterator,0>
         {
            static inline bool process(Iterator)
            {
               return false;
            }
         };

         template <typename T, typename Iterator, int N>
         struct numeric_convert_impl
         {
            static inline void process(Iterator, T&)
            { throw std::runtime_error("numeric_convert_impl::process( - unsupported value for N."); }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,19>
         {
            static inline void process(Iterator itr, T& t)
            {
               strtk::fast::details::numeric_convert_impl<T,Iterator,18>::process(itr + 1,t);
               t += static_cast<T>((itr[0] - '0') * 1000000000000000000LL);
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,18>
         {
            static inline void process(Iterator itr, T& t)
            {
               strtk::fast::details::numeric_convert_impl<T,Iterator,17>::process(itr + 1,t);
               t += static_cast<T>((itr[0] - '0') * 100000000000000000LL);
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,17>
         {
            static inline void process(Iterator itr, T& t)
            {
               numeric_convert_impl<T,Iterator,16>::process(itr + 1,t);
               t += static_cast<T>((itr[0] - '0') * 10000000000000000LL);
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,16>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[ 0] - '0') * 1000000000000000LL);
                 x += static_cast<T>((itr[ 1] - '0') *  100000000000000LL);
                 x += static_cast<T>((itr[ 2] - '0') *   10000000000000LL);
                 x += static_cast<T>((itr[ 3] - '0') *    1000000000000LL);
                 x += static_cast<T>((itr[ 4] - '0') *     100000000000LL);
                 x += static_cast<T>((itr[ 5] - '0') *      10000000000LL);
                 x += static_cast<T>((itr[ 6] - '0') *       1000000000LL);
                 x += static_cast<T>((itr[ 7] - '0') *        100000000LL);
                 x += static_cast<T>((itr[ 8] - '0') *         10000000LL);
                 x += static_cast<T>((itr[ 9] - '0') *          1000000LL);
                 x += static_cast<T>((itr[10] - '0') *           100000LL);
                 x += static_cast<T>((itr[11] - '0') *            10000LL);
                 x += static_cast<T>((itr[12] - '0') *             1000LL);
                 x += static_cast<T>((itr[13] - '0') *              100LL);
                 x += static_cast<T>((itr[14] - '0') *               10LL);
                 x += static_cast<T>((itr[15] - '0')                     );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,15>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[ 0] - '0') * 100000000000000LL);
                 x += static_cast<T>((itr[ 1] - '0') *  10000000000000LL);
                 x += static_cast<T>((itr[ 2] - '0') *   1000000000000LL);
                 x += static_cast<T>((itr[ 3] - '0') *    100000000000LL);
                 x += static_cast<T>((itr[ 4] - '0') *     10000000000LL);
                 x += static_cast<T>((itr[ 5] - '0') *      1000000000LL);
                 x += static_cast<T>((itr[ 6] - '0') *       100000000LL);
                 x += static_cast<T>((itr[ 7] - '0') *        10000000LL);
                 x += static_cast<T>((itr[ 8] - '0') *         1000000LL);
                 x += static_cast<T>((itr[ 9] - '0') *          100000LL);
                 x += static_cast<T>((itr[10] - '0') *           10000LL);
                 x += static_cast<T>((itr[11] - '0') *            1000LL);
                 x += static_cast<T>((itr[12] - '0') *             100LL);
                 x += static_cast<T>((itr[13] - '0') *              10LL);
                 x += static_cast<T>((itr[14] - '0')                    );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,14>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[ 0] - '0') * 10000000000000LL);
                 x += static_cast<T>((itr[ 1] - '0') *  1000000000000LL);
                 x += static_cast<T>((itr[ 2] - '0') *   100000000000LL);
                 x += static_cast<T>((itr[ 3] - '0') *    10000000000LL);
                 x += static_cast<T>((itr[ 4] - '0') *     1000000000LL);
                 x += static_cast<T>((itr[ 5] - '0') *      100000000LL);
                 x += static_cast<T>((itr[ 6] - '0') *       10000000LL);
                 x += static_cast<T>((itr[ 7] - '0') *        1000000LL);
                 x += static_cast<T>((itr[ 8] - '0') *         100000LL);
                 x += static_cast<T>((itr[ 9] - '0') *          10000LL);
                 x += static_cast<T>((itr[10] - '0') *           1000LL);
                 x += static_cast<T>((itr[11] - '0') *            100LL);
                 x += static_cast<T>((itr[12] - '0') *             10LL);
                 x += static_cast<T>((itr[13] - '0')                   );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,13>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[ 0] - '0') * 1000000000000LL);
                 x += static_cast<T>((itr[ 1] - '0') *  100000000000LL);
                 x += static_cast<T>((itr[ 2] - '0') *   10000000000LL);
                 x += static_cast<T>((itr[ 3] - '0') *    1000000000LL);
                 x += static_cast<T>((itr[ 4] - '0') *     100000000LL);
                 x += static_cast<T>((itr[ 5] - '0') *      10000000LL);
                 x += static_cast<T>((itr[ 6] - '0') *       1000000LL);
                 x += static_cast<T>((itr[ 7] - '0') *        100000LL);
                 x += static_cast<T>((itr[ 8] - '0') *         10000LL);
                 x += static_cast<T>((itr[ 9] - '0') *          1000LL);
                 x += static_cast<T>((itr[10] - '0') *           100LL);
                 x += static_cast<T>((itr[11] - '0') *            10LL);
                 x += static_cast<T>((itr[12] - '0')                  );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,12>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[ 0] - '0') * 100000000000LL);
                 x += static_cast<T>((itr[ 1] - '0') *  10000000000LL);
                 x += static_cast<T>((itr[ 2] - '0') *   1000000000LL);
                 x += static_cast<T>((itr[ 3] - '0') *    100000000LL);
                 x += static_cast<T>((itr[ 4] - '0') *     10000000LL);
                 x += static_cast<T>((itr[ 5] - '0') *      1000000LL);
                 x += static_cast<T>((itr[ 6] - '0') *       100000LL);
                 x += static_cast<T>((itr[ 7] - '0') *        10000LL);
                 x += static_cast<T>((itr[ 8] - '0') *         1000LL);
                 x += static_cast<T>((itr[ 9] - '0') *          100LL);
                 x += static_cast<T>((itr[10] - '0') *           10LL);
                 x += static_cast<T>((itr[11] - '0')                 );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,11>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[ 0] - '0') * 10000000000LL);
                 x += static_cast<T>((itr[ 1] - '0') *  1000000000LL);
                 x += static_cast<T>((itr[ 2] - '0') *   100000000LL);
                 x += static_cast<T>((itr[ 3] - '0') *    10000000LL);
                 x += static_cast<T>((itr[ 4] - '0') *     1000000LL);
                 x += static_cast<T>((itr[ 5] - '0') *      100000LL);
                 x += static_cast<T>((itr[ 6] - '0') *       10000LL);
                 x += static_cast<T>((itr[ 7] - '0') *        1000LL);
                 x += static_cast<T>((itr[ 8] - '0') *         100LL);
                 x += static_cast<T>((itr[ 9] - '0') *          10LL);
                 x += static_cast<T>((itr[10] - '0')                );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,10>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 1000000000);
                 x += static_cast<T>((itr[1] - '0') *  100000000);
                 x += static_cast<T>((itr[2] - '0') *   10000000);
                 x += static_cast<T>((itr[3] - '0') *    1000000);
                 x += static_cast<T>((itr[4] - '0') *     100000);
                 x += static_cast<T>((itr[5] - '0') *      10000);
                 x += static_cast<T>((itr[6] - '0') *       1000);
                 x += static_cast<T>((itr[7] - '0') *        100);
                 x += static_cast<T>((itr[8] - '0') *         10);
                 x += static_cast<T>((itr[9] - '0')             );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,9>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 100000000);
                 x += static_cast<T>((itr[1] - '0') *  10000000);
                 x += static_cast<T>((itr[2] - '0') *   1000000);
                 x += static_cast<T>((itr[3] - '0') *    100000);
                 x += static_cast<T>((itr[4] - '0') *     10000);
                 x += static_cast<T>((itr[5] - '0') *      1000);
                 x += static_cast<T>((itr[6] - '0') *       100);
                 x += static_cast<T>((itr[7] - '0') *        10);
                 x += static_cast<T>((itr[8] - '0')            );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,8>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 10000000);
                 x += static_cast<T>((itr[1] - '0') *  1000000);
                 x += static_cast<T>((itr[2] - '0') *   100000);
                 x += static_cast<T>((itr[3] - '0') *    10000);
                 x += static_cast<T>((itr[4] - '0') *     1000);
                 x += static_cast<T>((itr[5] - '0') *      100);
                 x += static_cast<T>((itr[6] - '0') *       10);
                 x += static_cast<T>((itr[7] - '0')           );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,7>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 1000000);
                 x += static_cast<T>((itr[1] - '0') *  100000);
                 x += static_cast<T>((itr[2] - '0') *   10000);
                 x += static_cast<T>((itr[3] - '0') *    1000);
                 x += static_cast<T>((itr[4] - '0') *     100);
                 x += static_cast<T>((itr[5] - '0') *      10);
                 x += static_cast<T>((itr[6] - '0')          );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,6>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 100000);
                 x += static_cast<T>((itr[1] - '0') *  10000);
                 x += static_cast<T>((itr[2] - '0') *   1000);
                 x += static_cast<T>((itr[3] - '0') *    100);
                 x += static_cast<T>((itr[4] - '0') *     10);
                 x += static_cast<T>((itr[5] - '0')         );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,5>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 10000);
                 x += static_cast<T>((itr[1] - '0') *  1000);
                 x += static_cast<T>((itr[2] - '0') *   100);
                 x += static_cast<T>((itr[3] - '0') *    10);
                 x += static_cast<T>((itr[4] - '0')        );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,4>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 1000);
                 x += static_cast<T>((itr[1] - '0') *  100);
                 x += static_cast<T>((itr[2] - '0') *   10);
                 x += static_cast<T>((itr[3] - '0')       );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,3>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 100);
                 x += static_cast<T>((itr[1] - '0') *  10);
                 x += static_cast<T>((itr[2] - '0')      );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,2>
         {
            static inline void process(Iterator itr, T& t)
            {
               T x  = static_cast<T>((itr[0] - '0') * 10);
                 x += static_cast<T>((itr[1] - '0')     );
               t = x;
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,1>
         {
            static inline void process(Iterator itr, T& t)
            {
               t = static_cast<T>((itr[0] - '0'));
            }
         };

         template <typename T, typename Iterator>
         struct numeric_convert_impl<T,Iterator,0>
         {
            static inline void process(Iterator, T& t)
            {
               t = 0;
            }
         };

         template <typename T, typename NoneSignedTag>
         inline bool negate(T&, NoneSignedTag)
         {
            return false;
         }

         template <typename T>
         inline bool negate(T& t, strtk::details::signed_type_tag)
         {
            t = -t;
            return true;
         }

      } // namespace details

      template <std::size_t N, typename Iterator>
      inline bool all_digits_check(Iterator itr)
      {
         typedef typename strtk::details::is_valid_iterator<Iterator>::type itr_type;
         strtk::details::convert_type_assert<itr_type>();
         return details::all_digits_check_impl<Iterator,N>::process(itr);
      }

      template <std::size_t N>
      inline bool all_digits_check(const std::string& s)
      {
         return all_digits_check<N,const char*>(s.data());
      }

      template <typename Iterator>
      inline bool all_digits_check(const std::size_t& n, Iterator itr)
      {
         switch (n)
         {
            case  0 : return details::all_digits_check_impl<Iterator, 0>::process(itr);
            case  1 : return details::all_digits_check_impl<Iterator, 1>::process(itr);
            case  2 : return details::all_digits_check_impl<Iterator, 2>::process(itr);
            case  3 : return details::all_digits_check_impl<Iterator, 3>::process(itr);
            case  4 : return details::all_digits_check_impl<Iterator, 4>::process(itr);
            case  5 : return details::all_digits_check_impl<Iterator, 5>::process(itr);
            case  6 : return details::all_digits_check_impl<Iterator, 6>::process(itr);
            case  7 : return details::all_digits_check_impl<Iterator, 7>::process(itr);
            case  8 : return details::all_digits_check_impl<Iterator, 8>::process(itr);
            case  9 : return details::all_digits_check_impl<Iterator, 9>::process(itr);
            case 10 : return details::all_digits_check_impl<Iterator,10>::process(itr);
            case 11 : return details::all_digits_check_impl<Iterator,11>::process(itr);
            case 12 : return details::all_digits_check_impl<Iterator,12>::process(itr);
            case 13 : return details::all_digits_check_impl<Iterator,13>::process(itr);
            case 14 : return details::all_digits_check_impl<Iterator,14>::process(itr);
            case 15 : return details::all_digits_check_impl<Iterator,15>::process(itr);
            case 16 : return details::all_digits_check_impl<Iterator,16>::process(itr);
            case 17 : return details::all_digits_check_impl<Iterator,17>::process(itr);
            case 18 : return details::all_digits_check_impl<Iterator,18>::process(itr);
            case 19 : return details::all_digits_check_impl<Iterator,19>::process(itr);
            default : return false;
         }
      }

      template <typename Iterator>
      inline bool all_digits_check(Iterator begin, Iterator end)
      {
         return all_digits_check(std::distance(begin,end),begin);
      }

      inline bool all_digits_check(const std::string& s)
      {
         return all_digits_check(s.size(),s.data());
      }

      template <std::size_t N, typename Iterator>
      inline bool signed_all_digits_check(Iterator itr)
      {
         if (('-' == (*itr)) || ('+' == (*itr)))
            return all_digits_check<Iterator,N - 1>((itr + 1));
         else
            return all_digits_check<Iterator,N>(itr);
      }

      template <typename Iterator>
      inline bool signed_all_digits_check(const std::size_t& n, Iterator itr)
      {
         if (('-' == (*itr)) || ('+' == (*itr)))
            return all_digits_check(n - 1,(itr + 1));
         else
            return all_digits_check(n,itr);
      }

      template <std::size_t N>
      inline bool signed_all_digits_check(const std::string& s)
      {
         return signed_all_digits_check<N,const char*>(s.data());
      }

      template <typename Iterator>
      inline bool signed_all_digits_check(Iterator begin, Iterator end)
      {
         return signed_all_digits_check(std::distance(begin,end),begin);
      }

      inline bool signed_all_digits_check(const std::string& s)
      {
         return signed_all_digits_check(s.size(),s.data());
      }

      template <std::size_t N, typename T, typename Iterator>
      inline void numeric_convert(Iterator itr, T& t, const bool digit_check = false)
      {
         typedef typename strtk::details::is_valid_iterator<Iterator>::type itr_type;
         strtk::details::convert_type_assert<itr_type>();
         if (digit_check)
         {
             if (!all_digits_check<N,Iterator>(itr))
             {
                t = 0;
                return;
             }
         }

         details::numeric_convert_impl<T,Iterator,N>::process(itr,t);
      }

      template <std::size_t N, typename T>
      inline void numeric_convert(const std::string& s, T& t, const bool digit_check = false)
      {
         numeric_convert<N,T,const char*>(s.data(),t,digit_check);
      }

      template <typename T, typename Iterator>
      inline bool numeric_convert(const std::size_t& n,
                                  Iterator itr, T& t,
                                  const bool digit_check = false)
      {
         if (digit_check)
         {
             if (!all_digits_check(n,itr))
             {
                return false;
             }
         }

         switch (n)
         {
            case  0 : details::numeric_convert_impl<T,Iterator, 0>::process(itr,t); return true;
            case  1 : details::numeric_convert_impl<T,Iterator, 1>::process(itr,t); return true;
            case  2 : details::numeric_convert_impl<T,Iterator, 2>::process(itr,t); return true;
            case  3 : details::numeric_convert_impl<T,Iterator, 3>::process(itr,t); return true;
            case  4 : details::numeric_convert_impl<T,Iterator, 4>::process(itr,t); return true;
            case  5 : details::numeric_convert_impl<T,Iterator, 5>::process(itr,t); return true;
            case  6 : details::numeric_convert_impl<T,Iterator, 6>::process(itr,t); return true;
            case  7 : details::numeric_convert_impl<T,Iterator, 7>::process(itr,t); return true;
            case  8 : details::numeric_convert_impl<T,Iterator, 8>::process(itr,t); return true;
            case  9 : details::numeric_convert_impl<T,Iterator, 9>::process(itr,t); return true;
            case 10 : details::numeric_convert_impl<T,Iterator,10>::process(itr,t); return true;
            case 11 : details::numeric_convert_impl<T,Iterator,11>::process(itr,t); return true;
            case 12 : details::numeric_convert_impl<T,Iterator,12>::process(itr,t); return true;
            case 13 : details::numeric_convert_impl<T,Iterator,13>::process(itr,t); return true;
            case 14 : details::numeric_convert_impl<T,Iterator,14>::process(itr,t); return true;
            case 15 : details::numeric_convert_impl<T,Iterator,15>::process(itr,t); return true;
            case 16 : details::numeric_convert_impl<T,Iterator,16>::process(itr,t); return true;
            case 17 : details::numeric_convert_impl<T,Iterator,17>::process(itr,t); return true;
            case 18 : details::numeric_convert_impl<T,Iterator,18>::process(itr,t); return true;
            case 19 : details::numeric_convert_impl<T,Iterator,19>::process(itr,t); return true;
            default : return false;
         }
      }

      template <typename T>
      inline void numeric_convert(const std::string& s, T& t, const bool digit_check = false)
      {
         numeric_convert(s.size(),s.data(),t,digit_check);
      }

      template <std::size_t N, typename T, typename Iterator>
      inline void signed_numeric_convert(Iterator itr, T& t, const bool digit_check = false)
      {
         if ('-' == (*itr))
         {
            numeric_convert<N - 1,T,Iterator>((itr + 1),t,digit_check);
            typename strtk::details::supported_conversion_to_type<T>::type type;
            details::negate(t,type);
         }
         else if ('+' == (*itr))
         {
            numeric_convert<N - 1,T,Iterator>((itr + 1),t,digit_check);
         }
         else
            numeric_convert<N,T,Iterator>(itr,t,digit_check);
      }

      template <typename T, typename Iterator>
      inline bool signed_numeric_convert(const std::size_t& n,
                                         Iterator itr,
                                         T& t,
                                         const bool digit_check = false)
      {
         if ('-' == (*itr))
         {
            bool result = numeric_convert((n - 1),(itr + 1),t,digit_check);
            typename strtk::details::supported_conversion_to_type<T>::type type;
            return details::negate<T>(t,type) && result;
         }
         else if ('+' == (*itr))
         {
            return numeric_convert((n - 1),(itr + 1),t,digit_check);
         }
         else
            return numeric_convert(n,itr,t,digit_check);
      }

      template <std::size_t N, typename T>
      inline void signed_numeric_convert(const std::string& s,
                                         T& t,
                                         const bool digit_check = false)
      {
         signed_numeric_convert<N,T,const char*>(s.data(),t,digit_check);
      }

      template <typename T>
      inline bool signed_numeric_convert(const std::string& s,
                                         T& t,
                                         const bool digit_check = false)
      {
         return signed_numeric_convert<T,const char*>(s.size(),s.data(),t,digit_check);
      }

   } // namespace fast

   namespace binary
   {

      namespace details
      {
         namespace details_endian
         {
            #if (defined(__LITTLE_ENDIAN__)) ||\
                (defined(WIN32)) ||\
                (defined(__MINGW32_VERSION)) ||\
                (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
               static const bool __le_result = true;
               static const bool __be_result = false;
            #else
               static const bool __le_result = false;
               static const bool __be_result = true;
            #endif
         }

         static inline bool is_little_endian()
         {
            //Is the current architecture/platform little-endian?
            return details_endian::__le_result;
         }

         static inline bool is_big_endian()
         {
            return details_endian::__be_result;
         }

         static inline unsigned short convert(const unsigned short v)
         {
            //static_assert(2 == sizeof(v),"");
            return ((v >> 8) & 0x00FF) | ((v << 8) & 0xFFFF);
         }

         static inline unsigned int convert(const unsigned int v)
         {
            //static_assert(4 == sizeof(v),"");
            return ((v >> 24) & 0x000000FF) | ((v << 24) & 0x0000FF00) |
                   ((v <<  8) & 0x00FF0000) | ((v >>  8) & 0xFF000000);
         }

         static inline unsigned long long int convert(const unsigned long long int v)
         {
            //static_assert(8 == sizeof(v),"");
            return ((v >> 56) & 0x00000000000000FFLL) | ((v << 56) & 0xFF00000000000000LL) |
                   ((v >> 40) & 0x000000000000FF00LL) | ((v << 40) & 0x00FF000000000000LL) |
                   ((v >> 24) & 0x0000000000FF0000LL) | ((v << 24) & 0x0000FF0000000000LL) |
                   ((v >>  8) & 0x00000000FF000000LL) | ((v <<  8) & 0x000000FF00000000LL) ;
         }

         static inline short convert(const short v)
         {
            return static_cast<short>(convert(static_cast<unsigned short>(v)));
         }

         static inline int convert(const int v)
         {
            return static_cast<int>(convert(static_cast<unsigned int>(v)));
         }

         static inline long long int convert(const long long int v)
         {
            return static_cast<long long>(convert(static_cast<unsigned long long int>(v)));
         }

         static inline unsigned short convert_to_be(const unsigned short v)
         {
            return (is_little_endian()) ? convert(v) : v;
         }

         static inline unsigned int convert_to_be(const unsigned int v)
         {
            return (is_little_endian()) ? convert(v) : v;
         }

         static inline unsigned long long int convert_to_be(const unsigned long long int v)
         {
            return (is_little_endian()) ? convert(v) : v;
         }

         static inline short convert_to_be(const short v)
         {
            return (is_little_endian()) ? convert(v) : v;
         }

         static inline int convert_to_be(const int v)
         {
            return (is_little_endian()) ? convert(v) : v;
         }

         static inline long long int convert_to_be(const long long int v)
         {
            return (is_little_endian()) ? convert(v) : v;
         }

         static inline unsigned short convert_to_le(const unsigned short v)
         {
            return (is_big_endian()) ? convert(v) : v;
         }

         static inline unsigned int convert_to_le(const unsigned int v)
         {
            return (is_big_endian()) ? convert(v) : v;
         }

         static inline unsigned long long int convert_to_le(const unsigned long long int v)
         {
            return (is_big_endian()) ? convert(v) : v;
         }

         static inline short convert_to_le(const short v)
         {
            return (is_big_endian()) ? convert(v) : v;
         }

         static inline int convert_to_le(const int v)
         {
            return (is_big_endian()) ? convert(v) : v;
         }

         static inline unsigned long long int convert_to_le(const long long int v)
         {
            return (is_big_endian()) ? convert(v) : v;
         }

         class marker
         {
         private:

            typedef std::pair<std::size_t,char*> mark_type;

         public:

            inline bool reset(std::size_t& v1, char*& v2)
            {
               if (stack_.empty())
                  return false;
               v1 = stack_.top().first;
               v2 = stack_.top().second;
               stack_.pop();
               return true;
            }

            inline void mark(const std::size_t& v1,char* v2)
            {
               stack_.push(std::make_pair(v1,v2));
            }

         private:

            std::stack<mark_type> stack_;
         };

      }

      class reader
      {
      public:

         // should be sourced from cstdint
         typedef unsigned int uint32_t;
         typedef unsigned short uint16_t;
         typedef unsigned char uint8_t;
         typedef unsigned long long int uint64_t;

         template <typename T>
         reader(T* buffer,
                const std::size_t& buffer_length)
         : original_buffer_(reinterpret_cast<char*>(buffer)),
           buffer_(reinterpret_cast<char*>(buffer)),
           buffer_length_(buffer_length * sizeof(T)),
           amount_read_sofar_(0)
         {}

         inline bool operator!() const
         {
            return (0 == buffer_length_)   ||
                   (0 == original_buffer_) ||
                   (0 == buffer_);
         }

         inline void reset(const bool clear_buffer = false)
         {
            amount_read_sofar_ = 0;
            buffer_ = original_buffer_;
            if (clear_buffer)
               clear();
         }

         inline std::size_t position() const
         {
            return amount_read_sofar_;
         }

         inline const char* position_ptr() const
         {
            return buffer_ ;
         }

         inline std::size_t amount_read()
         {
            return amount_read_sofar_;
         }

         inline bool rewind(const std::size_t& n_bytes)
         {
            if (n_bytes <= amount_read_sofar_)
            {
               amount_read_sofar_ -= n_bytes;
               buffer_ -= n_bytes;
               return true;
            }
            else
               return false;
         }

         inline bool seek(const int& n_bytes)
         {
            if (n_bytes < 0)
               return rewind(-n_bytes);
            else if (n_bytes > 0)
            {
               if ((amount_read_sofar_ + n_bytes) <= buffer_length_)
               {
                  amount_read_sofar_ += n_bytes;
                  buffer_ += n_bytes;
                  return true;
               }
               else
                  return false;
            }
            else
               return true;
         }

         inline void clear()
         {
            reset();
            std::memset(buffer_,0x00,buffer_length_);
         }

         template <typename T>
         inline bool operator()(T*& data, uint32_t& length, const bool read_length = true)
         {
            if (read_length && !operator()(length))
               return false;

            const std::size_t raw_size = length * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            if (read_length)
            {
               data = new T[length];
            }
            std::copy(buffer_, buffer_ + raw_size, reinterpret_cast<char*>(data));
            buffer_ += raw_size;
            amount_read_sofar_ += raw_size;
            return true;
         }

         template <typename T>
         inline bool operator()(T*& data, uint64_t& length, const bool read_length = true)
         {
            uint32_t l = 0;
            if (read_length)
               l = static_cast<uint32_t>(length);
            if (!operator()(data,l,read_length))
               return false;
            if (read_length)
               length = l;
            return true;
         }

         inline bool operator()(std::string& output)
         {
            uint32_t length = 0;
            if (!operator()(length))
               return false;

            if (!buffer_capacity_ok(length))
               return false;

            output.resize(length);
            std::copy(buffer_,
                      buffer_ + length,
                      const_cast<char*>(output.data()));
            buffer_ += length;
            amount_read_sofar_ += length;
            return true;
         }

         template <typename T1, typename T2>
         inline bool operator()(std::pair<T1,T2>& p)
         {
            if (!operator()(p.first))
               return false;
            if (!operator()(p.second))
               return false;
            return true;
         }

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         inline bool operator()(Sequence<T,Allocator>& seq)
         {
            uint32_t size = 0;
            if (!read_pod(size))
               return false;

            const std::size_t raw_size = size * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            T t = T();

            for (std::size_t i = 0; i < size; ++i)
            {
               if (operator()(t))
                  seq.push_back(t);
               else
                  return false;
            }
            return true;
         }

         template <typename T, typename Allocator>
         inline bool operator()(std::vector<T,Allocator>& vec)
         {
            uint32_t size = 0;
            if (!read_pod(size))
               return false;
            const std::size_t raw_size = size * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;
            vec.resize(size);
            return selector<T>::type::batch_vector_read(*this,size,vec,false);
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline bool operator()(std::set<T,Comparator,Allocator>& set)
         {
            uint32_t size = 0;
            if (!read_pod(size))
               return false;

            const std::size_t raw_size = size * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            T t;
            for (std::size_t i = 0; i < size; ++i)
            {
               if (!operator()(t))
                  return false;
               set.insert(t);
            }

            return true;
         }

         template <typename T,
                   typename Allocator,
                   typename Comparator>
         inline bool operator()(std::multiset<T,Allocator,Comparator>& multiset)
         {
            uint32_t size = 0;
            if (!read_pod(size))
               return false;

            const std::size_t raw_size = size * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            T t;
            for (std::size_t i = 0; i < size; ++i)
            {
               if (!operator()(t))
                  return false;
               multiset.insert(t);
            }

            return true;
         }

         inline bool operator()(std::ifstream& stream, const std::size_t& length)
         {
            if (length > buffer_length_) return false;
            stream.read(original_buffer_,static_cast<std::streamsize>(length));
            return true;
         }

         inline bool operator()(std::ifstream& stream)
         {
            if (0 == amount_read_sofar_) return false;
            stream.read(original_buffer_,static_cast<std::streamsize>(amount_read_sofar_));
            return true;
         }

         template <typename T>
         inline bool operator()(T& output)
         {
            return selector<T>::type::run(*this,output);
         }

         template <typename T>
         inline bool operator()(const T& output)
         {
            return selector<T>::type::run(*this,const_cast<T&>(output));
         }

         template <typename T>
         inline bool be_to_native(T& output)
         {
            //From big-endian to native
            if (details::is_little_endian())
            {
               if (!operator()<T>(output)) return false;
               output = details::convert(output);
               return true;
            }
            else
               return operator()(output);
         }

         template <typename T>
         inline bool le_to_native(T& output)
         {
            //From little-endian to native
            if (details::is_little_endian())
               return operator()(output);
            else
            {
               if (!operator()<T>(output)) return false;
               output = details::convert(output);
               return true;
            }
         }

         template <typename T, std::size_t N>
         inline bool operator()(T (&output)[N])
         {
            const std::size_t raw_size = N * sizeof(T);
            if (buffer_capacity_ok(raw_size))
            {
               std::copy(buffer_,
                         buffer_ + raw_size,
                         reinterpret_cast<char*>(output));
               buffer_ += raw_size;
               amount_read_sofar_ += raw_size;
               return true;
            }
            else
               return false;
         }

         template <typename T>
         inline bool operator()(T& output, const std::size_t& size)
         {
            if (buffer_capacity_ok(size))
            {
               bool result = strtk::string_to_type_converter<char*,T>(buffer_,buffer_ + size,output);
               buffer_ += size;
               amount_read_sofar_ += size;
               return result;
            }
            else
               return false;
         }

         inline void mark()
         {
            marker_.mark(amount_read_sofar_,buffer_);
         }

         inline bool reset_to_mark()
         {
            return marker_.reset(amount_read_sofar_,buffer_);
         }

      private:

         reader();
         reader(const reader& s);
         reader& operator=(const reader& s);

         inline bool buffer_capacity_ok(const std::size_t& required_read_qty)
         {
            return ((required_read_qty + amount_read_sofar_) <= buffer_length_);
         }

         template <typename Type>
         struct selector
         {
         private:

            template <typename T, typename IsPOD>
            struct selector_impl
            {
               template <typename Reader>
               static inline bool run(Reader& r, T& t)
               {
                  return t(r);
               }

               template <typename Reader,
                         typename Allocator>
               static inline bool batch_vector_read(Reader& r,
                                                    const std::size_t& size,
                                                    std::vector<T,Allocator>& v,
                                                    const bool)
               {
                  T t;
                  for (std::size_t i = 0; i < size; ++i)
                  {
                     if (r.operator()(t))
                        v[i] = t;
                     else
                        return false;
                  }
                  return true;
               }
            };

            template <typename T>
            struct selector_impl<T,strtk::details::yes_t>
            {
               template <typename Reader>
               static inline bool run(Reader& r,
                                      T& t,
                                      const bool perform_buffer_capacity_check = true)
               {
                  return r.read_pod(t,perform_buffer_capacity_check);
               }

               template <typename Reader,
                         typename Allocator>
               static inline bool batch_vector_read(Reader& r,
                                                    const std::size_t& size,
                                                    std::vector<T,Allocator>& v,
                                                    const bool)
               {
                  const std::size_t raw_size = sizeof(T) * size;
                  char* ptr = const_cast<char*>(reinterpret_cast<const char*>(&v[0]));
                  std::copy(r.buffer_, r.buffer_ + raw_size, ptr);
                  r.buffer_ += raw_size;
                  r.amount_read_sofar_ += raw_size;
                  return true;
               }
            };

         public:

            typedef selector_impl<Type,typename strtk::details::is_pod<Type>::result_t> type;
         };

         template <typename T>
         inline bool read_pod(T& data, const bool perform_buffer_capacity_check = true)
         {
            static const std::size_t data_length = sizeof(T);
            if (perform_buffer_capacity_check)
            {
               if (!buffer_capacity_ok(data_length))
                  return false;
            }
            data = (*reinterpret_cast<T*>(buffer_));
            buffer_ += data_length;
            amount_read_sofar_ += data_length;
            return true;
         }

         char* const original_buffer_;
         char* buffer_;
         std::size_t buffer_length_;
         std::size_t amount_read_sofar_;
         details::marker marker_;
      };

      class writer
      {
      public:

         // should be sourced from cstdint
         typedef unsigned int   uint32_t;
         typedef unsigned short uint16_t;
         typedef unsigned char  uint8_t;
         typedef unsigned long long int uint64_t;

         template <typename T>
         writer(T* buffer, const std::size_t& buffer_length)
         : original_buffer_(reinterpret_cast<char*>(buffer)),
           buffer_(reinterpret_cast<char*>(buffer)),
           buffer_length_(buffer_length * sizeof(T)),
           amount_written_sofar_(0)
         {}

         inline bool operator!() const
         {
            return (0 == buffer_length_)   ||
                   (0 == original_buffer_) ||
                   (0 == buffer_);
         }

         inline void reset(const bool clear_buffer = false)
         {
            amount_written_sofar_ = 0;
            buffer_ = original_buffer_;
            if (clear_buffer)
               clear();
         }

         inline std::size_t position() const
         {
            return amount_written_sofar_;
         }

         inline const char* position_ptr() const
         {
            return buffer_ ;
         }

         inline std::size_t amount_written() const
         {
            return amount_written_sofar_;
         }

         inline void clear()
         {
            reset();
            std::memset(buffer_,0x00,buffer_length_);
         }

         template <typename T, std::size_t N>
         inline bool operator()(const T (&data)[N], const bool write_length = false)
         {
            if (write_length && !operator()(N))
               return false;

            const std::size_t raw_size = N * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            const char* ptr = reinterpret_cast<const char*>(data);
            std::copy(ptr, ptr + raw_size, buffer_);
            buffer_ += raw_size;
            amount_written_sofar_ += raw_size;
            return true;
         }

         template <typename T>
         inline bool operator()(const T* data, const uint32_t& length, const bool write_length = true)
         {
            if (write_length && !operator()(length))
               return false;

            const std::size_t raw_size = length * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            const char* ptr = reinterpret_cast<const char*>(data);
            std::copy(ptr, ptr + raw_size, buffer_);
            buffer_ += raw_size;
            amount_written_sofar_ += raw_size;
            return true;
         }

         template <typename T>
         inline bool operator()(const T* data, const uint64_t& length, const bool write_length = true)
         {
            return operator()(data,static_cast<uint32_t>(length),write_length);
         }

         template <typename T>
         inline bool operator()(const T* data, const uint16_t& length, const bool write_length = true)
         {
            return operator()(data,static_cast<uint32_t>(length),write_length);
         }

         template <typename T>
         inline bool operator()(const T* data, const uint8_t& length, const bool write_length = true)
         {
            return operator()(data,static_cast<uint32_t>(length),write_length);
         }

         template <typename T1, typename T2>
         inline bool operator()(const std::pair<T1,T2>& p)
         {
            if (!operator()(p.first))
               return false;
            if (!operator()(p.second))
               return false;
            return true;
         }

         inline bool operator()(const std::string& input)
         {
            return operator()<const char>(input.data(),static_cast<uint32_t>(input.size()));
         }

         template <typename T,
                   typename Allocator,
                   template <typename,typename> class Sequence>
         inline bool operator()(const Sequence<T,Allocator>& seq)
         {
            const uint32_t size = static_cast<uint32_t>(seq.size());
            if (!operator()(size))
               return false;

            typename Sequence<T,Allocator>::const_iterator itr = seq.begin();
            typename Sequence<T,Allocator>::const_iterator end = seq.end();
            while (end != itr)
            {
               if (!operator()(*itr))
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T,
                   typename Allocator>
         inline bool operator()(const std::vector<T,Allocator>& vec)
         {
            const uint32_t size = static_cast<uint32_t>(vec.size());
            const std::size_t raw_size = (size * sizeof(T));
            if (!buffer_capacity_ok(raw_size + sizeof(size)))
               return false;
            if (!operator()(size))
               return false;
            return selector<T>::type::batch_vector_writer(*this,raw_size,vec);
         }

         template <typename T,
                   typename Comparator,
                   typename Allocator>
         inline bool operator()(const std::set<T,Comparator,Allocator>& set)
         {
            const uint32_t size = static_cast<uint32_t>(set.size());
            if (!operator()(size))
               return false;

            const std::size_t raw_size = size * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            typename std::set<T,Comparator,Allocator>::const_iterator itr = set.begin();
            typename std::set<T,Comparator,Allocator>::const_iterator end = set.end();
            while (end != itr)
            {
               if (!operator()(*itr))
                  return false;
               ++itr;
            }
            return true;
         }

         template <typename T,
                   typename Allocator,
                   typename Comparator>
         inline bool operator()(const std::multiset<T,Allocator,Comparator>& multiset)
         {
            const uint32_t size = static_cast<uint32_t>(multiset.size());
            if (!operator()(size))
               return false;

            const std::size_t raw_size = size * sizeof(T);
            if (!buffer_capacity_ok(raw_size))
               return false;

            typename std::multiset<T,Allocator,Comparator>::const_iterator itr = multiset.begin();
            typename std::multiset<T,Allocator,Comparator>::const_iterator end = multiset.end();
            while (end != itr)
            {
               if (!operator()(*itr))
                  return false;
               ++itr;
            }
            return true;
         }

         inline std::size_t operator()(std::ofstream& stream)
         {
            stream.write(original_buffer_,static_cast<std::streamsize>(amount_written_sofar_));
            return amount_written_sofar_;
         }

         template <typename T>
         inline bool operator()(const T& input)
         {
            return selector<T>::type::run(*this,input);
         }

         template <typename T>
         inline bool native_to_be(const T& input)
         {
            //From native to big-endian
            if (details::is_little_endian())
            {
               return operator()<T>(details::convert(input));
            }
            else
               return operator()<T>(input);
         }

         template <typename T>
         inline bool native_to_le(const T& input)
         {
            //From native to little-endian
            if (details::is_little_endian())
               return operator()<T>(input);
            else
               return operator()<T>(details::convert(input));
         }

         enum padding_mode
         {
            right_padding = 0,
            left_padding  = 1
         };

         template <typename T>
         inline bool operator()(const T& input,
                                const std::size_t& size,
                                const padding_mode pmode,
                                const char padding = ' ')
         {
            if (amount_written_sofar_ + size <= buffer_length_)
            {
               std::string s;
               s.reserve(size);
               if (!strtk::type_to_string<T>(input,s))
                  return false;
               else if (s.size() > size)
                  return false;
               else if (s.size() < size)
               {
                  if (right_padding == pmode)
                     s.resize(size - s.size(),padding);
                  else
                     s = std::string(size - s.size(),padding) + s;
               }
               return operator()<const char>(s.data(),static_cast<uint32_t>(size),false);
            }
            else
               return false;
         }

         inline void mark()
         {
            marker_.mark(amount_written_sofar_,buffer_);
         }

         inline bool reset_to_mark()
         {
            return marker_.reset(amount_written_sofar_,buffer_);
         }

      private:

         writer();
         writer(const writer& s);
         writer& operator=(const writer& s);

         inline bool buffer_capacity_ok(const std::size_t& required_write_qty)
         {
            return ((required_write_qty + amount_written_sofar_) <= buffer_length_);
         }

         template <typename Type>
         struct selector
         {
         private:

            template <typename T, typename IsPOD>
            struct selector_impl
            {
               template <typename Writer>
               static inline bool run(Writer& w, const T& t)
               {
                  return t(w);
               }

               template <typename Writer,
                         typename Allocator>
               static inline bool batch_vector_writer(Writer& w,
                                                      const std::size_t&,
                                                      const std::vector<T,Allocator>& v)
               {
                  for (std::size_t i = 0; i < v.size(); ++i)
                  {
                     if (w.operator()(v[i]))
                        continue;
                     else
                        return false;
                  }
                  return true;
               }
            };

            template <typename T>
            struct selector_impl<T,strtk::details::yes_t>
            {
               template <typename Writer>
               static inline bool run(Writer& w, const T& t)
               {
                  return w.write_pod(t);
               }

               template <typename Writer,
                         typename Allocator>
               static inline bool batch_vector_writer(Writer& w,
                                                      const std::size_t& raw_size,
                                                      const std::vector<T,Allocator>& v)
               {
                  const char* ptr = reinterpret_cast<const char*>(&v[0]);
                  std::copy(ptr, ptr + raw_size, w.buffer_);
                  w.buffer_ += raw_size;
                  w.amount_written_sofar_ += raw_size;
                  return true;
               }
            };

         public:

            typedef selector_impl<Type,typename strtk::details::is_pod<Type>::result_t> type;
         };

         template <typename T>
         inline bool write_pod(const T& data, const bool perform_buffer_capacity_check = true)
         {
            static const std::size_t data_length = sizeof(T);
            if (perform_buffer_capacity_check)
            {
               if ((data_length + amount_written_sofar_) > buffer_length_)
               {
                  return false;
               }
            }
            *(reinterpret_cast<T*>(buffer_)) = data;
            buffer_ += data_length;
            amount_written_sofar_ += data_length;
            return true;
         }

         char* const original_buffer_;
         char* buffer_;
         std::size_t buffer_length_;
         std::size_t amount_written_sofar_;
         details::marker marker_;
      };

      #define strtk_binary_reader_begin()           \
      bool operator()(strtk::binary::reader& reader)\
      { return true                                 \

      #define strtk_binary_reader(T) \
      && reader(T)                   \

      #define strtk_binary_reader_end() \
      ;}                                \

      #define strtk_binary_writer_begin()                 \
      bool operator()(strtk::binary::writer& writer) const\
      { return true                                       \

      #define strtk_binary_writer(T) \
      && writer(T)                   \

      #define strtk_binary_writer_end() \
      ;}                                \

      namespace details
      {
         template <typename size_type>
         class short_string_impl
         {
         public:

            short_string_impl()
            : s(0)
            {}

            short_string_impl(std::string& str)
            : s(&str)
            {}

            inline void clear()
            {
               s = 0;
            }

            inline short_string_impl<size_type>& set(std::string& str)
            {
               s = &str;
               return *this;
            }

            inline bool operator()(reader& r)
            {
               if (0 == s)
                  return false;
              size_type size = 0;
              if (!r(size))
                 return false;
               s->resize(size);
               char* ptr = const_cast<char*>(s->data());
               strtk::binary::reader::uint32_t length = size;
               if (!r(ptr,length,false))
                  return false;
               return true;
            }

            inline bool operator()(writer& w) const
            {
               if (0 == s)
                  return false;
               if (s->size() > std::numeric_limits<size_type>::max())
                  return false;
               const size_type size = static_cast<size_type>(s->size());
               if (!w(size))
                  return false;
               if (!w(s->data(),size, false))
                  return false;
               return true;
            }

         private:

            short_string_impl& operator=(const short_string_impl&);
            mutable std::string* s;
         };
      }

      typedef details::short_string_impl<reader::uint16_t> short_string;
      typedef details::short_string_impl<reader::uint8_t> pascal_string;

   } // namespace binary

   class ignore_token
   {
   public:

      template <typename InputIterator>
      inline ignore_token& operator=(const std::pair<InputIterator,InputIterator>&)
      {
         return (*this);
      }

      inline ignore_token& operator=(const std::string&)
      {
         return (*this);
      }
   };

   template <typename T>
   class hex_to_number_sink
   {
      // static_assert for T either int or unsigned int and alike (could use a concept)
   private:

      struct hex_value_check
      {
         inline bool operator()(const unsigned char c) const
         {
            return (('0' <= c) && (c <= '9')) ||
                   (('A' <= c) && (c <= 'F')) ||
                   (('a' <= c) && (c <= 'f'));
         }

         inline bool operator()(const char c) const
         {
            return (*this)(static_cast<unsigned char>(c));
         }
      };

   public:

      hex_to_number_sink(T& t)
      : valid_(false),
        t_(&t)
      {}

      hex_to_number_sink(const hex_to_number_sink& hns)
      : valid_(hns.valid),
        t_(hns.t_)
      {}

      inline hex_to_number_sink& operator=(const hex_to_number_sink& hns)
      {
         valid_ = hns.valid_;
         t_ = hns.t_;
         return (*this);
      }

      template <typename InputIterator>
      inline hex_to_number_sink& operator=(const std::pair<InputIterator,InputIterator>& s)
      {
         std::size_t offset = 0;
         const std::size_t size = std::distance(s.first,s.second);
         if ((size > 2) && ((*s.first) == '0') && (((*(s.first + 1)) == 'x') || ((*(s.first + 1)) == 'X')))
            offset = 2;
         if ((size - offset) > (2 * sizeof(T)))
            return (*this);

         const std::size_t buffer_size = 2 * sizeof(T);
         const std::size_t buffer_offset = ((size - offset) % 2);
         char buffer[buffer_size] = { '0' };
         if (!range_only_contains(hex_value_check(),s.first + offset,s.first + size))
         {
            valid_ = false;
            return (*this);
         }

         std::copy(s.first + offset, s.first + size, buffer + buffer_offset);
         (*t_) = 0;
         valid_= convert_hex_to_bin(buffer,
                                    buffer + (size - offset) + buffer_offset,
                                    reinterpret_cast<char*>(t_));
         reverse_bytes();
         return (*this);
      }

      inline hex_to_number_sink& operator=(const std::string& s)
      {
         return this->operator =(std::make_pair(s.data(),s.data() + s.size()));
      }

      inline bool valid() const
      {
         return valid_;
      }

   private:

      inline void reverse_bytes()
      {
         unsigned char* itr1 = reinterpret_cast<unsigned char*>(t_);
         unsigned char* itr2 = itr1 + (sizeof(T) - 1);
         while (itr1 < itr2)
         {
            std::swap(*itr1,*itr2);
            ++itr1;
            --itr2;
         }
      }

   private:

      bool valid_;
      T* t_;
   };

   template <typename T>
   class base64_to_number_sink
   {
      // static_assert for T either int or unsigned int and alike (could use a concept)
   private:

      struct base64_value_check
      {
         inline bool operator()(const unsigned char c) const
         {
            return (('0' <= c) && (c <= '9')) ||
                   (('A' <= c) && (c <= 'Z')) ||
                   (('a' <= c) && (c <= 'z')) ||
                    ('+' == c) ||
                    ('/' == c) ||
                    ('=' == c);
         }

         inline bool operator()(const char c) const
         {
            return (*this)(static_cast<unsigned char>(c));
         }
      };

   public:

      base64_to_number_sink(T& t)
      : valid_(false),
        t_(&t)
      {}

      base64_to_number_sink(const base64_to_number_sink& bns)
      : valid_(bns.valid),
        t_(bns.t_)
      {}

      inline base64_to_number_sink& operator=(const base64_to_number_sink& bns)
      {
         valid_ = bns.valid_;
         t_ = bns.t_;
         return (*this);
      }

      inline base64_to_number_sink& operator=(const std::string& s)
      {
         if (!range_only_contains(base64_value_check(),s.data(),s.data() + s.size()))
         {
            valid_ = false;
            return (*this);
         }

         (*t_) = T(0);
         convert_base64_to_bin(s.data(),
                               s.data() + s.size(),
                               reinterpret_cast<char*>(t_));
         reverse_bytes();
         return (*this);
      }

      template <typename InputIterator>
      inline base64_to_number_sink& operator=(const std::pair<InputIterator,InputIterator>& s)
      {
         if (!range_only_contains(base64_value_check(),s.first,s.second))
         {
            valid_ = false;
            return (*this);
         }

         (*t_) = T(0);
         convert_base64_to_bin(s.first, s.second,reinterpret_cast<char*>(t_));
         reverse_bytes();
         return (*this);
      }

      inline bool valid() const
      {
         return valid_;
      }

   private:

      inline void reverse_bytes()
      {
         unsigned char* itr1 = reinterpret_cast<unsigned char*>(t_);
         unsigned char* itr2 = itr1 + (sizeof(T) - 1);
         while (itr1 < itr2)
         {
            std::swap(*itr1,*itr2);
            ++itr1;
            --itr2;
         }
      }

   private:

      bool valid_;
      T* t_;
   };

   class hex_to_string_sink
   {
   public:

      hex_to_string_sink(std::string& s)
      : valid_(false),
        s_(s)
      {}

      hex_to_string_sink(const hex_to_string_sink& hss)
      : valid_(hss.valid_),
        s_(hss.s_)
      {}

      inline hex_to_string_sink& operator=(const hex_to_string_sink& hss)
      {
         valid_ = hss.valid_;
         s_ = hss.s_;
         return (*this);
      }

      template <typename InputIterator>
      inline hex_to_string_sink& operator=(const std::pair<InputIterator,InputIterator>& s)
      {
         const std::size_t size = std::distance(s.first,s.second);
         std::size_t offset = 0;
         if ((size > 2) && ((*s.first) == '0') && (((*(s.first + 1)) == 'x') || ((*(s.first + 1)) == 'X')))
            offset = 2;
         if ((size - offset) < 2)
         {
            valid_ = false;
            return (*this);
         }
         s_.resize((size - offset) / 2);
         valid_ = convert_hex_to_bin(s.first + offset,
                                     s.second,
                                     const_cast<char*>(s_.data()));
         return (*this);
      }

      inline hex_to_string_sink& operator=(const std::string& s)
      {
         return this->operator=(std::make_pair<char*>(const_cast<char*>(s.data()),
                                                      const_cast<char*>(s.data() + s.size())));
      }

      inline bool valid() const
      {
         return valid_;
      }

   private:

      bool valid_;
      std::string& s_;
   };

   template <typename T>
   class truncated_int
   {
   public:

      truncated_int()
      : t_(0),
        fractional_size_(std::numeric_limits<std::size_t>::max())
      {}

      truncated_int& fractional_size(const std::size_t& size)
      {
         fractional_size_ = size;
         return *this;
      }

      truncated_int& fractional_unknown_size()
      {
         fractional_size_ = std::numeric_limits<std::size_t>::max();
         return *this;
      }

      truncated_int& operator()(T& t)
      {
         t_ = &t;
         return *this;
      }

      template <typename InputIterator>
      inline bool operator()(InputIterator begin, InputIterator end)
      {
         if (0 == t_)
            return false;

         const std::size_t size = std::distance(begin,end);

         if (std::numeric_limits<std::size_t>::max() != fractional_size_)
         {
            if (size < (fractional_size_ + 1))
               return false;
            else
               return strtk::string_to_type_converter(begin, begin + (size - (fractional_size_ + 1)),(*t_));
         }

         typedef typename std::iterator_traits<InputIterator>::value_type value_type;
         const value_type fullstop = value_type('.');
         InputIterator new_end = std::find(begin,end,fullstop);
         return strtk::string_to_type_converter(begin,new_end,(*t_));
      }

   private:

      T* t_;
      std::size_t fractional_size_;
   };

   namespace details
   {

      template <typename InputIterator,
                typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline std::size_t parse_stl_container_proxy(const InputIterator begin,
                                                   const InputIterator end,
                                                   const std::string& delimiters,
                                                   Sequence<T,Allocator>& sequence,
                                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse(begin,end,delimiters,sequence,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Comparator,
                typename Allocator>
      inline std::size_t parse_stl_container_proxy(const InputIterator begin,
                                                   const InputIterator end,
                                                   const std::string& delimiters,
                                                   std::set<T,Comparator,Allocator>& set,
                                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse(begin,end,delimiters,set,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Comparator,
                typename Allocator>
      inline std::size_t parse_stl_container_proxy(const InputIterator begin,
                                                   const InputIterator end,
                                                   const std::string& delimiters,
                                                   std::multiset<T,Comparator,Allocator>& multiset,
                                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse(begin,end,delimiters,multiset,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Container>
      inline std::size_t parse_stl_container_proxy(const InputIterator begin,
                                                   const InputIterator end,
                                                   const std::string& delimiters,
                                                   std::queue<T,Container>& queue,
                                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse(begin,end,delimiters,queue,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Container>
      inline std::size_t parse_stl_container_proxy(const InputIterator begin,
                                                   const InputIterator end,
                                                   const std::string& delimiters,
                                                   std::stack<T,Container>& stack,
                                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse(begin,end,delimiters,stack,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Container,
                typename Comparator>
      inline std::size_t parse_stl_container_proxy(const InputIterator begin,
                                                   const InputIterator end,
                                                   const std::string& delimiters,
                                                   std::priority_queue<T,Container,Comparator>& priority_queue,
                                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse(begin,end,delimiters,priority_queue,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline std::size_t parse_n_stl_container_proxy(const InputIterator begin,
                                                     const InputIterator end,
                                                     const std::string& delimiters,
                                                     const std::size_t& n,
                                                     Sequence<T,Allocator>& sequence,
                                                     const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse_n(begin,end,delimiters,n,sequence,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Comparator,
                typename Allocator>
      inline std::size_t parse_n_stl_container_proxy(const InputIterator begin,
                                                     const InputIterator end,
                                                     const std::string& delimiters,
                                                     const std::size_t& n,
                                                     std::set<T,Comparator,Allocator>& set,
                                                     const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse_n(begin,end,delimiters,n,set,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Comparator,
                typename Allocator>
      inline std::size_t parse_n_stl_container_proxy(const InputIterator begin,
                                                     const InputIterator end,
                                                     const std::string& delimiters,
                                                     const std::size_t& n,
                                                     std::multiset<T,Comparator,Allocator>& multiset,
                                                     const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse_n(begin,end,delimiters,n,multiset,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Container>
      inline std::size_t parse_n_stl_container_proxy(const InputIterator begin,
                                                     const InputIterator end,
                                                     const std::string& delimiters,
                                                     const std::size_t& n,
                                                     std::queue<T,Container>& queue,
                                                     const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse_n(begin,end,delimiters,n,queue,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Container>
      inline std::size_t parse_n_stl_container_proxy(const InputIterator begin,
                                                     const InputIterator end,
                                                     const std::string& delimiters,
                                                     const std::size_t& n,
                                                     std::stack<T,Container>& stack,
                                                     const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse_n(begin,end,delimiters,n,stack,split_option);
      }

      template <typename InputIterator,
                typename T,
                typename Container,
                typename Comparator>
      inline std::size_t parse_n_stl_container_proxy(const InputIterator begin,
                                                     const InputIterator end,
                                                     const std::string& delimiters,
                                                     const std::size_t& n,
                                                     std::priority_queue<T,Container,Comparator>& priority_queue,
                                                     const split_options::type& split_option = split_options::compress_delimiters)
      {
         return parse_n(begin,end,delimiters,n,priority_queue,split_option);
      }

   } // namespace details

   template <typename Container>
   class sink_type
   {
   public:

      typedef typename Container::value_type value_type;

      inline sink_type(const std::string& delimiters,
                       const split_options::type& split_option = split_options::compress_delimiters)
      : delimiters_(delimiters),
        split_option_(split_option),
        container_(0),
        element_count_(std::numeric_limits<std::size_t>::max())
      {}

      inline sink_type(Container& container,
                       const std::string& delimiters,
                       const split_options::type& split_option = split_options::compress_delimiters)
      : delimiters_(delimiters),
        split_option_(split_option),
        container_(&container)
      {}

      inline sink_type& count(const std::size_t& element_count = std::numeric_limits<std::size_t>::max())
      {
         element_count_ = element_count;
         return (*this);
      }

      inline sink_type& operator()(Container& container,
                                   const std::string& delimiters = "",
                                   const split_options::type& split_option = split_options::compress_delimiters)
      {
         container_ = (&container);
         if (!delimiters.empty())
            delimiters_ = delimiters;
         split_option_ = split_option;
         return (*this);
      }

      template <typename InputIterator>
      inline bool parse(InputIterator begin, InputIterator end)
      {
         if (container_)
         {
            if (std::numeric_limits<std::size_t>::max() == element_count_)
               return (details::parse_stl_container_proxy
                         (begin,end,delimiters_,(*container_),split_option_) > 0);
            else
               return (details::parse_n_stl_container_proxy
                         (begin,end,delimiters_,element_count_,(*container_),split_option_) == element_count_);
         }
         else
            return false;
      }

      sink_type<Container>& reference()
      {
         return *this;
      }

   private:

      std::string delimiters_;
      split_options::type split_option_;
      Container* container_;
      std::size_t element_count_;
   };

   template <typename T> struct vector_sink   { typedef sink_type<std::vector<T> >   type; };
   template <typename T> struct deque_sink    { typedef sink_type<std::deque<T> >    type; };
   template <typename T> struct list_sink     { typedef sink_type<std::list<T> >     type; };
   template <typename T> struct set_sink      { typedef sink_type<std::set<T> >      type; };
   template <typename T> struct multiset_sink { typedef sink_type<std::multiset<T> > type; };
   template <typename T> struct queue_sink    { typedef sink_type<std::queue<T> >    type; };
   template <typename T> struct stack_sink    { typedef sink_type<std::stack<T> >    type; };
   template <typename T> struct priority_queue_sink { typedef sink_type<std::priority_queue<T> > type; };

   namespace text
   {
      inline std::string center(const std::size_t& width,
                                const std::string::value_type& pad,
                                const std::string& str)
      {
         if (str.size() >= width) return str;
         const std::size_t pad_count = width - str.size();
         const std::size_t pad_count_2 = (pad_count >> 1) + (pad_count & 1);
         return std::string(pad_count >> 1,pad) + str + std::string(pad_count_2,pad);
      }

      inline std::string right_align(const std::size_t& width,
                                     const std::string::value_type& pad,
                                     const std::string& str)
      {
         if (str.size() >= width) return str;
         return std::string(width - str.size(),pad) + str;
      }

      inline std::string left_align(const std::size_t& width,
                                    const std::string::value_type& pad,
                                    const std::string& str)
      {
         if (str.size() >= width) return str;
         return str + std::string(width - str.size(),pad);
      }

      template <typename T>
      inline std::string center(const std::size_t& width,
                                const std::string::value_type& pad,
                                const T& t)
      {
         return center(width,pad,type_to_string(t));
      }

      template <typename T>
      inline std::string right_align(const std::size_t& width,
                                     const std::string::value_type& pad,
                                     const T& t)
      {
         return right_align(width,pad,type_to_string(t));
      }

      template <typename T>
      inline std::string left_align(const std::size_t& width,
                                    const std::string::value_type& pad,
                                    const T& t)
      {
         return left_align(width,pad,type_to_string(t));
      }

      template <typename T>
      inline std::string center(const std::size_t& width, const T& t)
      {
         return center(width,' ',type_to_string(t));
      }

      template <typename T>
      inline std::string right_align(const std::size_t& width, const T& t)
      {
         return right_align(width,' ',type_to_string(t));
      }

      template <typename T>
      inline std::string left_align(const std::size_t& width, const T& t)
      {
         return left_align(width,' ',type_to_string(t));
      }

      inline std::string remaining_string(const std::size_t& index,
                                          const std::string& str)
      {
         return (index < str.size()) ? str.substr(index,str.size() - index) : str;
      }

      inline void remaining_string(const std::size_t& index,
                                   const std::string& str,
                                   std::string& result)
      {
         result = (index < str.size()) ? str.substr(index,str.size() - index) : str;
      }

      inline bool is_letter(const char c)
      {
         return (('A' <= c) && ( c <= 'Z')) || (('a' <= c) && ( c <= 'z'));
      }

      inline bool is_lowercase_letter(const char c)
      {
         return (('a' <= c) && ( c <= 'z'));
      }

      inline bool is_uppercase_letter(const char c)
      {
         return (('A' <= c) && ( c <= 'Z'));
      }

      inline bool is_digit(const char c)
      {
         return (('0' <= c) && ( c <= '9'));
      }

      inline bool is_hex_digit(const char c)
      {
         return (('0' <= c) && (c <= '9')) ||
                (('A' <= c) && (c <= 'F')) ||
                (('a' <= c) && (c <= 'f'));
      }

      inline bool is_letter_or_digit(const char c)
      {
         return (is_letter(c) || is_digit(c));
      }

      inline bool is_all_letters(const std::string& s)
      {
         for (std::size_t i = 0; i < s.size(); ++i)
         {
            if (!is_letter(s[i]))
               return false;
         }
         return true;
      }

      inline bool is_all_digits(const std::string& s)
      {
         for (std::size_t i = 0; i < s.size(); ++i)
         {
            if (!is_digit(s[i]))
               return false;
         }
         return true;
      }

      inline void swap_inplace(std::string& s, const std::size_t& i0, const std::size_t& i1)
      {
         if (i0 >= s.size()) return;
         if (i1 >= s.size()) return;
         std::swap(s[i0],s[i1]);
      }

      inline std::string swap(const std::string& s, const std::size_t& i0, const std::size_t& i1)
      {
         std::string result = s;
         swap_inplace(result,i0,i1);
         return result;
      }

      inline void remove_inplace(std::string& s, const std::size_t& index)
      {
         if (index >= s.size())
            return;
         std::memcpy(const_cast<char*>(s.data() + index), const_cast<char*>(s.data() + (index + 1)), s.size() - index);
         s.resize(s.size() - 1);
      }

      inline std::string remove(const std::string& s, const std::size_t& index)
      {
         std::string result = s;
         remove_inplace(result,index);
         return result;
      }

      inline void insert_inplace(std::string& s, const std::size_t& index, const char c)
      {
         s.resize(s.size() + 1);
         std::memcpy(const_cast<char*>(s.data() + index + 1), const_cast<char*>(s.data() + (index)), s.size() - index);
         s[index] = c;
      }

      inline std::string insert(const std::string& s, const std::size_t& index, const char c)
      {
         std::string result = s;
         insert_inplace(result,index,c);
         return result;
      }

   } // namespace text

   namespace find_mode
   {
      enum type
      {
         exactly_n,
         atleast_n
      };
   }

   namespace find_type
   {
      enum type
      {
         digits,
         letters,
         lowercase_letters,
         uppercase_letters,
         letters_digits
      };
   }

   namespace details
   {
      template <typename Iterator>
      struct range_type
      {
         typedef typename std::pair<Iterator,Iterator> type;
      };

      template <typename Iterator, typename Predicate>
      inline typename range_type<Iterator>::type find_exactly_n_consecutive_values(const std::size_t n,
                                                                                   Predicate p,
                                                                                   Iterator itr,
                                                                                   const Iterator end,
                                                                                   const bool stateful_predicate = false)
      {
         if (static_cast<unsigned int>(std::distance(itr,end)) < n)
            return typename range_type<Iterator>::type(end,end);
         std::size_t count = n;
         while (end != itr)
         {
            if (p(*itr))
            {
               if (0 != --count)
                  ++itr;
               else
               {
                  std::advance(itr,1 - n);
                  return typename range_type<Iterator>::type(itr,itr + n);
               }
            }
            else
            {
               ++itr;
               while ((end != itr) && !p(*itr))
                  ++itr;
               if (!stateful_predicate)
                  count = n;
               else
               {
                  --count;
                  ++itr;
               }
            }
         }
         return typename range_type<Iterator>::type(end,end);
      }

      template <typename Iterator, typename Predicate>
      inline typename range_type<Iterator>::type find_atleast_n_consecutive_values(const std::size_t n,
                                                                                   Predicate p,
                                                                                   Iterator itr,
                                                                                   const Iterator end)
      {
         if (static_cast<unsigned int>(std::distance(itr,end)) < n)
            return typename range_type<Iterator>::type(end,end);
         std::size_t count = 0;
         while (end != itr)
         {
            if (p(*itr))
            {
               ++count;
               ++itr;
            }
            else
            {
               if (count >= n)
               {
                  std::advance(itr,-static_cast<int>(count));
                  return typename range_type<Iterator>::type(itr,itr + count);
               }
               while ((end != itr) && !p(*itr))
                  ++itr;
               count = 0;
            }
         }
         if (count >= n)
         {
            std::advance(itr,-static_cast<int>(count));
            return typename range_type<Iterator>::type(itr,itr + count);
         }
         else
            return typename range_type<Iterator>::type(end,end);
      }

      template <typename Iterator, typename Predicate>
      inline typename range_type<Iterator>::type find_exactly_n_consecutive_values(const std::size_t n,
                                                                                   Predicate p,
                                                                                   typename details::range_type<Iterator>::type range,
                                                                                   const bool stateful_predicate = false)
      {
         return find_exactly_n_consecutive_values(n,p,range.first,range.second,stateful_predicate);
      }

      template <typename Iterator, typename Predicate>
      inline typename range_type<Iterator>::type find_atleast_n_consecutive_values(const std::size_t n,
                                                                                   Predicate p,
                                                                                   typename details::range_type<Iterator>::type range)
      {
         return find_atleast_n_consecutive_values(n,p,range.first,range.second);
      }

      template <typename Iterator, typename Predicate>
      inline typename range_type<Iterator>::type find_n_consecutive_values(const std::size_t n,
                                                                           find_mode::type mode,
                                                                           Predicate p,
                                                                           Iterator itr,
                                                                           const Iterator end)
      {
         switch (mode)
         {
            case find_mode::exactly_n : return find_exactly_n_consecutive_values(n,p,itr,end);
            case find_mode::atleast_n : return find_atleast_n_consecutive_values(n,p,itr,end);
            default                   : return typename range_type<Iterator>::type(end,end);
         }
      }

      template <typename Iterator,typename Predicate>
      inline bool match_exactly_n_consecutive_values(const std::size_t n,
                                                     Predicate p,
                                                     Iterator itr,
                                                     const Iterator end)
      {
         if (static_cast<unsigned int>(std::distance(itr,end)) < n)
            return false;
         std::size_t count = n;
         while (end != itr)
         {
            if (p(*itr))
            {
               if (0 == --count)
                  return true;
               else
                  ++itr;
            }
            else
               return false;
         }
         return false;
      }

      template <typename Iterator,typename Predicate>
      inline bool match_atleast_n_consecutive_values(const std::size_t n,
                                                     Predicate p,
                                                     Iterator itr,
                                                     const Iterator end)
      {
         if (static_cast<unsigned int>(std::distance(itr,end)) < n)
            return false;
         std::size_t count = 0;
         while (end != itr)
         {
            if (p(*itr))
            {
               ++count;
               ++itr;
            }
            else if (count >= n)
               return true;
            else
               return false;
         }
         return false;
      }

      template <typename Iterator,typename Predicate>
      inline bool match_n_consecutive_values(const std::size_t n,
                                             find_mode::type mode,
                                             Predicate p,
                                             Iterator itr,
                                             const Iterator end)
      {
         switch (mode)
         {
            case find_mode::exactly_n : return match_exactly_n_consecutive_values(n,p,itr,end);
            case find_mode::atleast_n : return match_atleast_n_consecutive_values(n,p,itr,end);
            default                   : return false;
         }
      }

   }

   template <typename Iterator>
   inline typename details::range_type<Iterator>::type find_n_consecutive(const std::size_t n,
                                                                          find_type::type type,
                                                                          find_mode::type mode,
                                                                          typename details::range_type<Iterator>::type range)
   {
      switch (type)
      {
         case find_type::digits  : return details::find_n_consecutive_values<Iterator>(n,
                                                                                       mode,
                                                                                       strtk::text::is_digit,
                                                                                       range.first,range.second);

         case find_type::letters : return details::find_n_consecutive_values<Iterator>(n,
                                                                                       mode,
                                                                                       strtk::text::is_letter,
                                                                                       range.first,range.second);

         case find_type::lowercase_letters : return details::find_n_consecutive_values<Iterator>(n,
                                                                                                 mode,
                                                                                                 strtk::text::is_lowercase_letter,
                                                                                                 range.first,range.second);

         case find_type::uppercase_letters : return details::find_n_consecutive_values<Iterator>(n,
                                                                                                 mode,
                                                                                                 strtk::text::is_uppercase_letter,
                                                                                                 range.first,range.second);

         case find_type::letters_digits    : return details::find_n_consecutive_values<Iterator>(n,
                                                                                                 mode,
                                                                                                 strtk::text::is_letter_or_digit,
                                                                                                 range.first,range.second);

         default : return typename details::range_type<Iterator>::type(range.second,range.second);
      }
   }

   template <typename Iterator>
   inline bool match_n_consecutive(const std::size_t n,
                                   find_type::type type,
                                   find_mode::type mode,
                                   typename details::range_type<Iterator>::type range)
   {
      switch (type)
      {
         case find_type::digits  : return details::match_n_consecutive_values<Iterator>(n,
                                                                                       mode,
                                                                                       strtk::text::is_digit,
                                                                                       range.first,range.second);

         case find_type::letters : return details::match_n_consecutive_values<Iterator>(n,
                                                                                        mode,
                                                                                        strtk::text::is_letter,
                                                                                        range.first,range.second);

         case find_type::lowercase_letters : return details::match_n_consecutive_values<Iterator>(n,
                                                                                                  mode,
                                                                                                  strtk::text::is_lowercase_letter,
                                                                                                  range.first,range.second);

         case find_type::uppercase_letters : return details::match_n_consecutive_values<Iterator>(n,
                                                                                                  mode,
                                                                                                  strtk::text::is_uppercase_letter,
                                                                                                  range.first,range.second);

         case find_type::letters_digits    : return details::match_n_consecutive_values<Iterator>(n,
                                                                                                  mode,
                                                                                                  strtk::text::is_letter_or_digit,
                                                                                                  range.first,range.second);

         default : return false;
      }
   }

   template <typename Predicate,
             typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t n,
                                           Predicate p,
                                           char* begin,
                                           char* end,
                                           OutputIterator out,
                                           const bool stateful_predicate = false)
   {
      if (0 == n) return 0;
      typedef char* iterator_type;
      typedef details::range_type<iterator_type>::type range_type;
      range_type itr_range(begin,end);
      std::size_t match_count = 0;
      while (end != itr_range.first)
      {
         range_type found_itr =
            details::find_exactly_n_consecutive_values<iterator_type,Predicate>(n,
                                                                                p,
                                                                                itr_range,
                                                                                stateful_predicate);

         if ((end == found_itr.first) && (found_itr.first == found_itr.second))
         {
            break;
         }
         else
         {
            (*out) = found_itr;
            ++out;
            ++match_count;
            itr_range.first = found_itr.second;
         }
      }
      return match_count;
   }

   template <typename Predicate,
             typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t n,
                                           const std::size_t m,
                                           Predicate p,
                                           char* begin,
                                           char* end,
                                           OutputIterator out)
   {
      if (0 == n) return 0;
      typedef char* iterator_type;
      typedef details::range_type<iterator_type>::type range_type;
      range_type itr_range(begin,end);
      std::size_t match_count = 0;
      while ((end != itr_range.first) && (match_count <= n))
      {
         range_type found_itr = details::find_exactly_n_consecutive_values(m,p,itr_range);
         if ((end == found_itr.first) && (found_itr.first == found_itr.second))
         {
            break;
         }
         else
         {
            (*out) = found_itr;
            ++out;
            ++match_count;
            itr_range.first = found_itr.second;
         }
      }
      return match_count;
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           const find_type::type type,
                                           const find_mode::type mode,
                                           char* begin,
                                           char* end,
                                           OutputIterator out)
   {
      if (0 == n) return 0;
      typedef char* iterator_type;
      typedef details::range_type<iterator_type>::type range_type;
      range_type itr_range(begin,end);
      std::size_t match_count = 0;
      while (end != itr_range.first)
      {
         range_type found_itr = find_n_consecutive<iterator_type>(n,type,mode,itr_range);
         if ((end == found_itr.first) && (found_itr.first == found_itr.second))
         {
            break;
         }
         else
         {
            (*out) = found_itr;
            ++out;
            ++match_count;
            itr_range.first = found_itr.second;
         }
      }
      return match_count;
   }

   template <typename InputIterator, typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             const find_type::type type,
                                             const find_mode::type mode,
                                             char* begin,
                                             char* end,
                                             OutputIterator out)
   {
      if (0 == n) return 0;
      typedef char* iterator_type;
      typedef details::range_type<iterator_type>::type range_type;
      range_type itr_range(begin,end);
      std::size_t match_count = 0;
      while ((end != itr_range.first) && (match_count <= n))
      {
         range_type found_itr = find_n_consecutive<iterator_type>(m,type,mode,itr_range);
         if ((end == found_itr.first) && (found_itr.first == found_itr.second))
         {
            break;
         }
         else
         {
            (*out) = found_itr;
            ++out;
            ++match_count;
            itr_range.first = found_itr.second;
         }
      }
      return match_count;
   }

   template <typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           const find_type::type type,
                                           const find_mode::type mode,
                                           const char* begin,
                                           const char* end,
                                           OutputIterator out)
   {
      return split_on_consecutive<char*,OutputIterator>(n,
                                                        type,
                                                        mode,
                                                        const_cast<char*>(begin),
                                                        const_cast<char*>(end),
                                                        out);
   }

   template <typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           const find_type::type type,
                                           const find_mode::type mode,
                                           const unsigned char* begin,
                                           const unsigned char* end,
                                           OutputIterator out)
   {
      return split_on_consecutive<OutputIterator>(n,
                                                  type,
                                                  mode,
                                                  reinterpret_cast<const char*>(begin),
                                                  reinterpret_cast<const char*>(end),
                                                  out);
   }

   template <typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           const find_type::type type,
                                           const find_mode::type mode,
                                           const std::string& str,
                                           OutputIterator out)
   {
      return split_on_consecutive<OutputIterator>(n,
                                                  type,
                                                  mode,
                                                  str.data(),
                                                  str.data() + str.size(),
                                                  out);
   }

   template <typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             const find_type::type type,
                                             const find_mode::type mode,
                                             const char* begin,
                                             const char* end,
                                             OutputIterator out)
   {
      return split_on_consecutive_n<char*,OutputIterator>(n,
                                                          m,
                                                          type,
                                                          mode,
                                                          const_cast<char*>(begin),
                                                          const_cast<char*>(end),
                                                          out);
   }

   template <typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             const find_type::type type,
                                             const find_mode::type mode,
                                             const unsigned char* begin,
                                             const unsigned char* end,
                                             OutputIterator out)
   {
      return split_on_consecutive_n<OutputIterator>(n,
                                                    m,
                                                    type,
                                                    mode,
                                                    reinterpret_cast<const char*>(begin),
                                                    reinterpret_cast<const char*>(end),
                                                    out);
   }

   template <typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             const find_type::type type,
                                             const find_mode::type mode,
                                             const std::string& str,
                                             OutputIterator out)
   {
      return split_on_consecutive_n<OutputIterator>(n,
                                                    m,
                                                    type,
                                                    mode,
                                                    str.data(),
                                                    str.data() + str.size(),
                                                    out);
   }

   template <typename Predicate, typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           Predicate p,
                                           const char* begin,
                                           const char* end,
                                           OutputIterator out,
                                           const bool stateful_predicate = false)
   {
      return split_on_consecutive<Predicate,
                                  OutputIterator>(n,
                                                  p,
                                                  const_cast<char*>(begin),
                                                  const_cast<char*>(end),
                                                  out,
                                                  stateful_predicate);
   }

   template <typename Predicate, typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           Predicate p,
                                           const unsigned char* begin,
                                           const unsigned char* end,
                                           OutputIterator out,
                                           const bool stateful_predicate = false)
   {
      return split_on_consecutive<Predicate,
                                  OutputIterator>(n,
                                                  p,
                                                  reinterpret_cast<const char*>(begin),
                                                  reinterpret_cast<const char*>(end),
                                                  out,
                                                  stateful_predicate);
   }

   template <typename Predicate, typename OutputIterator>
   inline std::size_t split_on_consecutive(const std::size_t& n,
                                           Predicate p,
                                           const std::string& str,
                                           OutputIterator out,
                                           const bool stateful_predicate = false)
   {
      return split_on_consecutive<Predicate,
                                  OutputIterator>(n,
                                                  p,
                                                  str.data(),
                                                  str.data() + str.size(),
                                                  out,
                                                  stateful_predicate);
   }

   template <typename Predicate, typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             Predicate p,
                                             const char* begin,
                                             const char* end,
                                             OutputIterator out)
   {
      return split_on_consecutive_n<Predicate,
                                    char*,
                                    OutputIterator>(n,
                                                    m,
                                                    p,
                                                    const_cast<char*>(begin),
                                                    const_cast<char*>(end),
                                                    out);
   }

   template <typename Predicate, typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             Predicate p,
                                             const unsigned char* begin,
                                             const unsigned char* end,
                                             OutputIterator out)
   {
      return split_on_consecutive_n<Predicate,
                                    OutputIterator>(n,
                                                    m,
                                                    p,
                                                    reinterpret_cast<const char*>(begin),
                                                    reinterpret_cast<const char*>(end),
                                                    out);
   }

   template <typename Predicate, typename OutputIterator>
   inline std::size_t split_on_consecutive_n(const std::size_t& n,
                                             const std::size_t& m,
                                             Predicate p,
                                             const std::string& str,
                                             OutputIterator out)
   {
      return split_on_consecutive_n<Predicate,
                                    OutputIterator>(n,
                                                    m,
                                                    p,
                                                    str.data(),
                                                    str.data() + str.size(),
                                                    out);
   }

   // Required for broken versions of GCC pre 4.5
   namespace util { class value; }

   namespace details
   {

      class expect_impl
      {
      public:

         expect_impl(const std::string& s)
         : s_(s)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            if (static_cast<std::size_t>(std::distance(begin,end)) != s_.size())
               return false;
            else
               return std::equal(s_.data(),s_.data() + s_.size(),begin);
         }

         inline expect_impl& ref()
         {
            return (*this);
         }

         inline void set_value(const std::string& s)
         {
            s_ = s;
         }

      private:

         std::string s_;
      };

      class iexpect_impl
      {
      public:

         iexpect_impl(const std::string& s)
         : s_(s)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            if (static_cast<std::size_t>(std::distance(begin,end)) != s_.size())
               return false;
            else
               return std::equal(s_.data(),s_.data() + s_.size(),begin,imatch_char);
         }

         inline iexpect_impl& ref()
         {
            return (*this);
         }

         inline void set_value(const std::string& s)
         {
            s_ = s;
         }

      private:

         std::string s_;
      };

      class like_impl
      {
      public:

         like_impl(const std::string& s)
         : s_(s)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            typedef typename std::iterator_traits<InputIterator>::value_type value_type;
            static const value_type zero_or_more = value_type('*');
            static const value_type zero_or_one  = value_type('?');
            return strtk::match(s_.data(),s_.data() + s_.size(),begin,end,zero_or_more,zero_or_one);
         }

         inline like_impl& ref()
         {
            return (*this);
         }

         inline void set_pattern(const std::string& s)
         {
            s_ = s;
         }

      private:

         std::string s_;
      };

      template <typename T>
      class inrange_impl
      {
      public:

         inrange_impl(T& t, const T& low, const T& hi)
         : t_(&t),
           low_(low),
           hi_(hi)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            T temp;
            if (!strtk::string_to_type_converter(begin,end,temp))
               return false;
            else if (temp < low_)
               return false;
            else if (temp > hi_)
               return false;
            (*t_) = temp;
            return true;
         }

         inline inrange_impl<T>& ref()
         {
            return (*this);
         }

         inline void set_low_hi(const T& low, const T& hi)
         {
            low_ = low;
            hi_  = hi;
         }

      private:

         T* t_;
         T low_;
         T hi_;
      };

      namespace trim_details
      {
         template <typename Type>
         struct convert_impl
         {
            template <typename InputIterator>
            static bool execute(InputIterator begin, InputIterator end,
                                const std::string& rem_chars,
                                std::size_t mode,
                                Type& t)
            {
               std::string s;
               if (!strtk::string_to_type_converter(begin,end,s))
                  return false;
               switch (mode)
               {
                  case 0  : remove_leading_trailing(rem_chars,s); break;
                  case 1  : remove_leading         (rem_chars,s); break;
                  case 2  : remove_trailing        (rem_chars,s); break;
                  default : return false;
               }
               return strtk::string_to_type_converter(s,t);
            }
         };

         template <>
         struct convert_impl <std::string>
         {
            template <typename InputIterator>
            static bool execute(InputIterator begin, InputIterator end,
                                const std::string& rem_chars,
                                std::size_t mode,
                                std::string& t)
            {
               if (!strtk::string_to_type_converter(begin,end,t))
                  return false;
               switch (mode)
               {
                  case 0  : remove_leading_trailing(rem_chars,t); break;
                  case 1  : remove_leading         (rem_chars,t); break;
                  case 2  : remove_trailing        (rem_chars,t); break;
                  default : return false;
               }
               return true;
            }
         };
      }

      template <typename T>
      class trim_impl
      {
      public:

         trim_impl(const std::size_t mode,
                   T& t,
                   const std::string& rem_chars = " ")
         : mode_(mode),
           t_(&t),
           rem_chars_(rem_chars)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            return trim_details::convert_impl<T>::execute(begin,end,rem_chars_,mode_,(*t_));
         }

         inline trim_impl<T>& ref()
         {
            return (*this);
         }

      private:

         std::size_t  mode_;
         T* t_;
         std::string  rem_chars_;
      };

      class conv_to_lcase_impl
      {
      public:

         conv_to_lcase_impl(std::string& s)
         : s_(&s)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            std::string& s = (*s_);
            s.assign(begin,end);
            convert_to_lowercase(s);
            return true;
         }

         inline conv_to_lcase_impl& ref()
         {
            return (*this);
         }

      private:

         std::string* s_;
      };

      class conv_to_ucase_impl
      {
      public:

         conv_to_ucase_impl(std::string& s)
         : s_(&s)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            std::string& s = (*s_);
            s.assign(begin,end);
            convert_to_uppercase(s);
            return true;
         }

         inline conv_to_ucase_impl& ref()
         {
            return (*this);
         }

      private:

         std::string* s_;
      };

      class fill_array_impl
      {
      public:

         fill_array_impl(unsigned char* data, const std::size_t& size)
         : data_(data),
           size_(size)
         {}

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end)
         {
            const std::size_t range_size = static_cast<std::size_t>(std::distance(begin,end));
            if (range_size != size_)
               return false;
            std::memcpy(data_,begin,range_size);
            return true;
         }

         inline fill_array_impl& ref()
         {
            return (*this);
         }

         inline fill_array_impl& set(unsigned char* data, const std::size_t& size)
         {
            data_ = data;
            size_ = size;
            return (*this);
         }

         inline fill_array_impl& set(char* data, const std::size_t& size)
         {
            data_ = reinterpret_cast<unsigned char*>(data);
            size_ = size;
            return (*this);
         }

         inline fill_array_impl& set_data(unsigned char* data)
         {
            data_ = data;
            return (*this);
         }

         inline fill_array_impl& set_data(char* data)
         {
            data_ = reinterpret_cast<unsigned char*>(data);
            return (*this);
         }

         inline fill_array_impl& set_size(const std::size_t& size)
         {
            size_ = size;
            return (*this);
         }

      private:

         unsigned char* data_;
         std::size_t size_;
      };
   }

   inline details::expect_impl expect(const std::string& s)
   {
      return details::expect_impl(s);
   }

   inline details::iexpect_impl iexpect(const std::string& s)
   {
      return details::iexpect_impl(s);
   }

   inline details::like_impl like(const std::string& s)
   {
      return details::like_impl(s);
   }

   template <typename T, typename T0, typename T1>
   inline details::inrange_impl<T> inrange(T& t, const T0& low, const T1& hi)
   {
      return details::inrange_impl<T>(t,T(low),T(hi));
   }

   template <typename T>
   inline details::trim_impl<T> trim(const std::string& rem_chars, T& t)
   {
      return details::trim_impl<T>(0,t,rem_chars);
   }

   template <typename T>
   inline details::trim_impl<T> trim_leading(const std::string& rem_chars, T& t)
   {
      return details::trim_impl<T>(1,t,rem_chars);
   }

   template <typename T>
   inline details::trim_impl<T> trim_trailing(const std::string& rem_chars, T& t)
   {
      return details::trim_impl<T>(2,t,rem_chars);
   }

   inline details::conv_to_lcase_impl as_lcase(std::string& s)
   {
      return details::conv_to_lcase_impl(s);
   }

   inline details::conv_to_ucase_impl as_ucase(std::string& s)
   {
      return details::conv_to_ucase_impl(s);
   }

   inline details::fill_array_impl fill_array(unsigned char* data, const std::size_t& size)
   {
      return details::fill_array_impl(data,size);
   }

   inline details::fill_array_impl fill_array(char* data, const std::size_t& size)
   {
      return details::fill_array_impl(reinterpret_cast<unsigned char*>(data),size);
   }

   template <std::size_t N>
   inline details::fill_array_impl fill_array(unsigned char (&data)[N])
   {
      return details::fill_array_impl(data,N);
   }

   template <std::size_t N>
   inline details::fill_array_impl fill_array(char (&data)[N])
   {
      return details::fill_array_impl(reinterpret_cast<unsigned char*>(data),N);
   }

   inline details::fill_array_impl fill_array(std::string& data, const std::size_t& size)
   {
      return fill_array(const_cast<char*>(data.data()),size);
   }

   inline details::fill_array_impl fill_array(std::string& data)
   {
      return fill_array(const_cast<char*>(data.data()),data.size());
   }

   namespace details
   {
      static const unsigned char digit_table[] =
                                 {
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFF - 0x07
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x08 - 0x0F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x10 - 0x17
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x18 - 0x1F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x20 - 0x27
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x28 - 0x2F
                                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x30 - 0x37
                                    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x38 - 0x3F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x40 - 0x47
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x48 - 0x4F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x50 - 0x57
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x58 - 0x5F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x60 - 0x67
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x68 - 0x6F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x70 - 0x77
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x78 - 0x7F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x80 - 0x87
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x88 - 0x8F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x90 - 0x97
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x98 - 0x9F
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xA0 - 0xA7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xA8 - 0xAF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xB0 - 0xB7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xB8 - 0xBF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xC0 - 0xC7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xC8 - 0xCF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xD0 - 0xD7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xD8 - 0xDF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xE0 - 0xE7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xE8 - 0xEF
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xF0 - 0xF7
                                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 0xF8 - 0xFF
                                 };

      static const std::size_t digit_table_size = sizeof(digit_table) / sizeof(unsigned char);

      template <typename T>
      static inline bool is_invalid_digit(const T& t)
      {
         static const unsigned int invalid_digit = 0xFF;
         return (static_cast<T>(invalid_digit) == t);
      }

      template <typename T>
      static inline bool is_valid_digit(const T& t)
      {
         static const unsigned int invalid_digit = 0xFF;
         return (static_cast<T>(invalid_digit) != t);
      }

      static const unsigned char digitr[] =
                                 {
                                    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 };

      static const unsigned char rev_3digit_lut[] =
                                 {
                                    "000001002003004005006007008009010011012013014015016017018019020021022023024"
                                    "025026027028029030031032033034035036037038039040041042043044045046047048049"
                                    "050051052053054055056057058059060061062063064065066067068069070071072073074"
                                    "075076077078079080081082083084085086087088089090091092093094095096097098099"
                                    "100101102103104105106107108109110111112113114115116117118119120121122123124"
                                    "125126127128129130131132133134135136137138139140141142143144145146147148149"
                                    "150151152153154155156157158159160161162163164165166167168169170171172173174"
                                    "175176177178179180181182183184185186187188189190191192193194195196197198199"
                                    "200201202203204205206207208209210211212213214215216217218219220221222223224"
                                    "225226227228229230231232233234235236237238239240241242243244245246247248249"
                                    "250251252253254255256257258259260261262263264265266267268269270271272273274"
                                    "275276277278279280281282283284285286287288289290291292293294295296297298299"
                                    "300301302303304305306307308309310311312313314315316317318319320321322323324"
                                    "325326327328329330331332333334335336337338339340341342343344345346347348349"
                                    "350351352353354355356357358359360361362363364365366367368369370371372373374"
                                    "375376377378379380381382383384385386387388389390391392393394395396397398399"
                                    "400401402403404405406407408409410411412413414415416417418419420421422423424"
                                    "425426427428429430431432433434435436437438439440441442443444445446447448449"
                                    "450451452453454455456457458459460461462463464465466467468469470471472473474"
                                    "475476477478479480481482483484485486487488489490491492493494495496497498499"
                                    "500501502503504505506507508509510511512513514515516517518519520521522523524"
                                    "525526527528529530531532533534535536537538539540541542543544545546547548549"
                                    "550551552553554555556557558559560561562563564565566567568569570571572573574"
                                    "575576577578579580581582583584585586587588589590591592593594595596597598599"
                                    "600601602603604605606607608609610611612613614615616617618619620621622623624"
                                    "625626627628629630631632633634635636637638639640641642643644645646647648649"
                                    "650651652653654655656657658659660661662663664665666667668669670671672673674"
                                    "675676677678679680681682683684685686687688689690691692693694695696697698699"
                                    "700701702703704705706707708709710711712713714715716717718719720721722723724"
                                    "725726727728729730731732733734735736737738739740741742743744745746747748749"
                                    "750751752753754755756757758759760761762763764765766767768769770771772773774"
                                    "775776777778779780781782783784785786787788789790791792793794795796797798799"
                                    "800801802803804805806807808809810811812813814815816817818819820821822823824"
                                    "825826827828829830831832833834835836837838839840841842843844845846847848849"
                                    "850851852853854855856857858859860861862863864865866867868869870871872873874"
                                    "875876877878879880881882883884885886887888889890891892893894895896897898899"
                                    "900901902903904905906907908909910911912913914915916917918919920921922923924"
                                    "925926927928929930931932933934935936937938939940941942943944945946947948949"
                                    "950951952953954955956957958959960961962963964965966967968969970971972973974"
                                    "975976977978979980981982983984985986987988989990991992993994995996997998999"
                                    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                 };

      static const unsigned char rev_2digit_lut[] =
                                 {
                                    "0001020304050607080910111213141516171819"
                                    "2021222324252627282930313233343536373839"
                                    "4041424344454647484950515253545556575859"
                                    "6061626364656667686970717273747576777879"
                                    "8081828384858687888990919293949596979899"
                                    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                 };

      #define strtk_register_pod_type(T)                                                           \
      template<> struct is_pod<T>{ typedef yes_t result_t; enum {result = true }; };               \
      template<> struct is_pod<const T>{ typedef yes_t result_t; enum {result = true }; };         \
      template<> struct is_pod<volatile T>{ typedef yes_t result_t; enum {result = true }; };      \
      template<> struct is_pod<const volatile T>{ typedef yes_t result_t; enum {result = true }; };\

      strtk_register_pod_type(bool)
      strtk_register_pod_type(signed char)
      strtk_register_pod_type(char)
      strtk_register_pod_type(short)
      strtk_register_pod_type(int)
      strtk_register_pod_type(long int)
      strtk_register_pod_type(long long int)
      strtk_register_pod_type(unsigned char)
      strtk_register_pod_type(unsigned short)
      strtk_register_pod_type(unsigned int)
      strtk_register_pod_type(unsigned long int)
      strtk_register_pod_type(unsigned long long int)
      strtk_register_pod_type(float)
      strtk_register_pod_type(double)
      strtk_register_pod_type(long double)

      #undef strtk_register_pod_type

      template <typename>
      struct numeric { enum { length = 0, size = 32, bound_length = 0, min_exp = 0, max_exp = 0 }; };

      template<> struct numeric<short>                  { enum { length =  5, size = 16, bound_length =  4}; };
      template<> struct numeric<unsigned short>         { enum { length =  5, size = 16, bound_length =  4}; };

      template<> struct numeric<int>                    { enum { length = 10, size = 16, bound_length =  9}; };
      template<> struct numeric<unsigned int>           { enum { length = 10, size = 16, bound_length =  9}; };

      template<> struct numeric<long>                   { enum { length = 10, size = 16, bound_length =  9}; };
      template<> struct numeric<unsigned long>          { enum { length = 10, size = 16, bound_length =  9}; };

      template<> struct numeric<long long>              { enum { length = 19, size = 24, bound_length = 18}; };
      template<> struct numeric<unsigned long long int> { enum { length = 20, size = 24, bound_length = 19}; };

      template<> struct numeric<float>                  { enum { min_exp =  -37, max_exp =  +38, precision = 10}; };
      template<> struct numeric<double>                 { enum { min_exp = -307, max_exp = +308, precision = 15}; };

      template <typename ld, std::size_t size> struct ldt {};
      template <> struct ldt <long double,     sizeof(double)> { enum { i =  -308, a =  +308, p = 15}; }; //64-bit
      template <> struct ldt <long double,                 10> { enum { i = -4931, a = +4931, p = 18}; }; //80-bit
      template <> struct ldt <long double,                 12> { enum { i = -4931, a = +4931, p = 22}; }; //96-bit
      template <> struct ldt <long double, 2 * sizeof(double)> { enum { i = -4931, a = +4931, p = 34}; }; //128-bit

      template<>
      struct numeric<long double>
      {
         typedef ldt<long double, sizeof(long double)> ld;
         enum { min_exp = ld::i, max_exp = ld::a, precision = ld::p};
      };

      #define strtk_register_unsigned_type_tag(T)                                             \
      template<> struct supported_conversion_to_type<T> { typedef unsigned_type_tag type; };  \
      template<> struct supported_conversion_from_type<T> { typedef unsigned_type_tag type; };\

      #define strtk_register_signed_type_tag(T)                                               \
      template<> struct supported_conversion_to_type<T>{ typedef signed_type_tag type; };     \
      template<> struct supported_conversion_from_type<T> { typedef signed_type_tag type; };  \

      #define strtk_register_real_type_tag(T) \
      template<> struct supported_conversion_to_type<T>{ typedef real_type_tag type; };

      #define strtk_register_byte_type_tag(T)                                             \
      template<> struct supported_conversion_to_type<T>{ typedef byte_type_tag type; };   \
      template<> struct supported_conversion_from_type<T> { typedef byte_type_tag type; };\

      #define strtk_register_hex_number_type_tag(T) \
      template<> struct supported_conversion_to_type<T >{ typedef hex_number_type_tag type; };

      template<> struct supported_conversion_to_type<hex_to_string_sink>{ typedef hex_string_type_tag type; };

      #define strtk_register_base64_type_tag(T) \
      template<> struct supported_conversion_to_type<T >{ typedef base64_type_tag type; };

      #define strtk_register_supported_iterator_type(T) \
      template<> struct supported_iterator_type<T> { enum { value = true }; };

      template<> struct supported_conversion_to_type<bool> { typedef bool_type_tag type; };
      template<> struct supported_iterator_type<bool> { enum { value = true }; };

      template<> struct supported_conversion_to_type<std::string> { typedef stdstring_type_tag type; };
      template<> struct supported_iterator_type<std::string> { enum { value = true }; };

      template<> struct supported_conversion_to_type<strtk::util::value> { typedef value_type_tag type; };
      template<> struct supported_conversion_from_type<strtk::util::value> { typedef value_type_tag type; };
      template<> struct supported_iterator_type<strtk::util::value> { enum { value = true }; };

      template<> struct supported_conversion_to_type<strtk::details::expect_impl> { typedef expect_type_tag type; };
      template<> struct supported_iterator_type<strtk::details::expect_impl>      { enum { value = true };        };

      template<> struct supported_conversion_to_type<strtk::details::iexpect_impl> { typedef expect_type_tag type; };
      template<> struct supported_iterator_type<strtk::details::iexpect_impl>      { enum { value = true }; };

      template<> struct supported_conversion_to_type<strtk::details::like_impl> { typedef like_type_tag type; };
      template<> struct supported_iterator_type<strtk::details::like_impl>      { enum { value = true }; };

      template<> struct supported_conversion_to_type<strtk::details::fill_array_impl> { typedef fillchararray_type_tag type; };
      template<> struct supported_iterator_type<strtk::details::fill_array_impl>      { enum { value = true }; };

      template<> struct supported_conversion_to_type<strtk::details::conv_to_lcase_impl> { typedef lcase_type_tag type; };
      template<> struct supported_iterator_type<strtk::details::conv_to_lcase_impl>      { enum { value = true }; };

      template<> struct supported_conversion_to_type<strtk::details::conv_to_ucase_impl> { typedef ucase_type_tag type; };
      template<> struct supported_iterator_type<strtk::details::conv_to_ucase_impl>      { enum { value = true }; };

      #define strtk_register_truncint_type_tag(T) \
      template<> struct supported_conversion_to_type<strtk::truncated_int<T> > { typedef truncint_type_tag type; };\
      template<> struct supported_iterator_type<strtk::truncated_int<T> >      { enum { value = true }; };

      #define strtk_register_inrange_type_tag(T) \
      template<> struct supported_conversion_to_type<strtk::details::inrange_impl<T> >   { typedef inrange_type_tag type; };\
      template<> struct supported_iterator_type<strtk::details::inrange_impl<T> >        { enum { value = true }; };

      #define strtk_register_trim_type_tag(T) \
      template<> struct supported_conversion_to_type<strtk::details::trim_impl<T> >   { typedef trim_type_tag type; };\
      template<> struct supported_iterator_type<strtk::details::trim_impl<T> >        { enum { value = true }; };

      #define strtk_register_stdstring_range_type_tag(T) \
      template<> struct supported_conversion_to_type< std::pair<T,T> >{ typedef stdstring_range_type_tag type; };

      #define strtk_register_sink_type_tag(T)                                                                               \
      template<> struct supported_conversion_to_type<sink_type<std::vector<T> > > { typedef sink_type_tag type; };          \
      template<> struct supported_conversion_to_type<sink_type<std::deque<T> > > { typedef sink_type_tag type; };           \
      template<> struct supported_conversion_to_type<sink_type<std::list<T> > > { typedef sink_type_tag type; };            \
      template<> struct supported_conversion_to_type<sink_type<std::set<T> > > { typedef sink_type_tag type; };             \
      template<> struct supported_conversion_to_type<sink_type<std::multiset<T> > > { typedef sink_type_tag type; };        \
      template<> struct supported_conversion_to_type<sink_type<std::queue<T> > > { typedef sink_type_tag type; };           \
      template<> struct supported_conversion_to_type<sink_type<std::stack<T> > > { typedef sink_type_tag type; };           \
      template<> struct supported_conversion_to_type<sink_type<std::priority_queue<T> > > { typedef sink_type_tag type; };  \
      template<> struct supported_conversion_from_type<sink_type<std::vector<T> > > { typedef sink_type_tag type; };        \
      template<> struct supported_conversion_from_type<sink_type<std::deque<T> > > { typedef sink_type_tag type; };         \
      template<> struct supported_conversion_from_type<sink_type<std::list<T> > > { typedef sink_type_tag type; };          \
      template<> struct supported_conversion_from_type<sink_type<std::set<T> > > { typedef sink_type_tag type; };           \
      template<> struct supported_conversion_from_type<sink_type<std::multiset<T> > > { typedef sink_type_tag type; };      \
      template<> struct supported_conversion_from_type<sink_type<std::queue<T> > > { typedef sink_type_tag type; };         \
      template<> struct supported_conversion_from_type<sink_type<std::stack<T> > > { typedef sink_type_tag type; };         \
      template<> struct supported_conversion_from_type<sink_type<std::priority_queue<T> > > { typedef sink_type_tag type; };\

      #define strtk_register_stl_container_to_string_conv_type_tag(T)                                              \
      template<> struct supported_conversion_from_type<std::vector<T> > { typedef stl_seq_type_tag type; };        \
      template<> struct supported_conversion_from_type<std::deque<T> > { typedef stl_seq_type_tag type; };         \
      template<> struct supported_conversion_from_type<std::list<T> > { typedef stl_seq_type_tag type; };          \
      template<> struct supported_conversion_from_type<std::set<T> > { typedef stl_seq_type_tag type; };           \
      template<> struct supported_conversion_from_type<std::multiset<T> > { typedef stl_seq_type_tag type; };      \
      template<> struct supported_conversion_from_type<std::queue<T> > { typedef stl_seq_type_tag type; };         \
      template<> struct supported_conversion_from_type<std::stack<T> > { typedef stl_seq_type_tag type; };         \
      template<> struct supported_conversion_from_type<std::priority_queue<T> > { typedef stl_seq_type_tag type; };\

      template<> struct supported_conversion_to_type<ignore_token>{ typedef ignore_token_type_tag type; };

      #define strtk_register_sequence_iterator_type(sequence)                        \
      strtk_register_supported_iterator_type(sequence<char>::iterator)               \
      strtk_register_supported_iterator_type(sequence<char>::const_iterator)         \
      strtk_register_supported_iterator_type(sequence<unsigned char>::iterator)      \
      strtk_register_supported_iterator_type(sequence<unsigned char>::const_iterator)\

      strtk_register_unsigned_type_tag(unsigned short)
      strtk_register_unsigned_type_tag(unsigned int)
      strtk_register_unsigned_type_tag(unsigned long)
      strtk_register_unsigned_type_tag(unsigned long long int)

      strtk_register_signed_type_tag(short)
      strtk_register_signed_type_tag(int)
      strtk_register_signed_type_tag(long)
      strtk_register_signed_type_tag(long long)

      strtk_register_real_type_tag(float)
      strtk_register_real_type_tag(double)
      strtk_register_real_type_tag(long double)

      strtk_register_byte_type_tag(unsigned char)
      strtk_register_byte_type_tag(signed char)
      strtk_register_byte_type_tag(char)

      strtk_register_hex_number_type_tag(hex_to_number_sink<short>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<int>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<long>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<long long>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<unsigned short>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<unsigned int>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<unsigned long>)
      strtk_register_hex_number_type_tag(hex_to_number_sink<unsigned long long int>)

      strtk_register_base64_type_tag(base64_to_number_sink<short>)
      strtk_register_base64_type_tag(base64_to_number_sink<int>)
      strtk_register_base64_type_tag(base64_to_number_sink<long>)
      strtk_register_base64_type_tag(base64_to_number_sink<long long>)
      strtk_register_base64_type_tag(base64_to_number_sink<unsigned short>)
      strtk_register_base64_type_tag(base64_to_number_sink<unsigned int>)
      strtk_register_base64_type_tag(base64_to_number_sink<unsigned long>)
      strtk_register_base64_type_tag(base64_to_number_sink<unsigned long long int>)

      strtk_register_stdstring_range_type_tag(std::string::iterator)
      strtk_register_stdstring_range_type_tag(std::string::const_iterator)
      strtk_register_stdstring_range_type_tag(char*)
      strtk_register_stdstring_range_type_tag(signed char*)
      strtk_register_stdstring_range_type_tag(unsigned char*)
      strtk_register_stdstring_range_type_tag(const char*)
      strtk_register_stdstring_range_type_tag(const unsigned char*)

      strtk_register_supported_iterator_type(char*)
      strtk_register_supported_iterator_type(signed char*)
      strtk_register_supported_iterator_type(unsigned char*)
      strtk_register_supported_iterator_type(const char*)
      strtk_register_supported_iterator_type(const signed char*)
      strtk_register_supported_iterator_type(const unsigned char*)
      strtk_register_supported_iterator_type(std::string::iterator)
      strtk_register_supported_iterator_type(std::string::const_iterator)

      strtk_register_sequence_iterator_type(std::vector)
      strtk_register_sequence_iterator_type(std::deque)

      strtk_register_sink_type_tag(float)
      strtk_register_sink_type_tag(double)
      strtk_register_sink_type_tag(long double)
      strtk_register_sink_type_tag(signed char)
      strtk_register_sink_type_tag(char)
      strtk_register_sink_type_tag(short)
      strtk_register_sink_type_tag(int)
      strtk_register_sink_type_tag(long)
      strtk_register_sink_type_tag(long long)
      strtk_register_sink_type_tag(unsigned char)
      strtk_register_sink_type_tag(unsigned short)
      strtk_register_sink_type_tag(unsigned int)
      strtk_register_sink_type_tag(unsigned long)
      strtk_register_sink_type_tag(unsigned long long int)
      strtk_register_sink_type_tag(std::string)

      strtk_register_stl_container_to_string_conv_type_tag(float)
      strtk_register_stl_container_to_string_conv_type_tag(double)
      strtk_register_stl_container_to_string_conv_type_tag(long double)
      strtk_register_stl_container_to_string_conv_type_tag(signed char)
      strtk_register_stl_container_to_string_conv_type_tag(char)
      strtk_register_stl_container_to_string_conv_type_tag(short)
      strtk_register_stl_container_to_string_conv_type_tag(int)
      strtk_register_stl_container_to_string_conv_type_tag(long)
      strtk_register_stl_container_to_string_conv_type_tag(long long)
      strtk_register_stl_container_to_string_conv_type_tag(unsigned char)
      strtk_register_stl_container_to_string_conv_type_tag(unsigned short)
      strtk_register_stl_container_to_string_conv_type_tag(unsigned int)
      strtk_register_stl_container_to_string_conv_type_tag(unsigned long)
      strtk_register_stl_container_to_string_conv_type_tag(unsigned long long int)
      strtk_register_stl_container_to_string_conv_type_tag(std::string)

      strtk_register_inrange_type_tag(float)
      strtk_register_inrange_type_tag(double)
      strtk_register_inrange_type_tag(long double)
      strtk_register_inrange_type_tag(signed char)
      strtk_register_inrange_type_tag(char)
      strtk_register_inrange_type_tag(short)
      strtk_register_inrange_type_tag(int)
      strtk_register_inrange_type_tag(long)
      strtk_register_inrange_type_tag(long long)
      strtk_register_inrange_type_tag(unsigned char)
      strtk_register_inrange_type_tag(unsigned short)
      strtk_register_inrange_type_tag(unsigned int)
      strtk_register_inrange_type_tag(unsigned long)
      strtk_register_inrange_type_tag(unsigned long long int)
      strtk_register_inrange_type_tag(std::string)

      strtk_register_trim_type_tag(float)
      strtk_register_trim_type_tag(double)
      strtk_register_trim_type_tag(long double)
      strtk_register_trim_type_tag(signed char)
      strtk_register_trim_type_tag(char)
      strtk_register_trim_type_tag(short)
      strtk_register_trim_type_tag(int)
      strtk_register_trim_type_tag(long)
      strtk_register_trim_type_tag(long long)
      strtk_register_trim_type_tag(unsigned char)
      strtk_register_trim_type_tag(unsigned short)
      strtk_register_trim_type_tag(unsigned int)
      strtk_register_trim_type_tag(unsigned long)
      strtk_register_trim_type_tag(unsigned long long int)
      strtk_register_trim_type_tag(std::string)

      strtk_register_trim_type_tag(truncated_int<short>)
      strtk_register_trim_type_tag(truncated_int<int>)
      strtk_register_trim_type_tag(truncated_int<long>)
      strtk_register_trim_type_tag(truncated_int<long long>)
      strtk_register_trim_type_tag(truncated_int<unsigned char>)
      strtk_register_trim_type_tag(truncated_int<unsigned short>)
      strtk_register_trim_type_tag(truncated_int<unsigned int>)
      strtk_register_trim_type_tag(truncated_int<unsigned long long int>)

      strtk_register_truncint_type_tag(short)
      strtk_register_truncint_type_tag(int)
      strtk_register_truncint_type_tag(long)
      strtk_register_truncint_type_tag(long long)
      strtk_register_truncint_type_tag(unsigned short)
      strtk_register_truncint_type_tag(unsigned int)
      strtk_register_truncint_type_tag(unsigned long)
      strtk_register_truncint_type_tag(unsigned long long int)

      #define strtk_register_userdef_type_sink(T) \
      namespace strtk { namespace details { strtk_register_sink_type_tag(T) }}

      #undef strtk_register_unsigned_type_tag
      #undef strtk_register_signed_type_tag
      #undef strtk_register_real_type_tag
      #undef strtk_register_byte_type_tag
      #undef strtk_register_hex_number_type_tag
      #undef strtk_register_base64_type_tag
      #undef strtk_register_supported_iterator_type
      #undef strtk_register_stdstring_range_type_tag
      #undef strtk_register_sequence_iterator_type
      #undef strtk_register_stl_container_to_string_conv_type_tag
      #undef strtk_register_inrange_type_tag
      #undef strtk_register_trim_type_tag
      #undef strtk_register_truncint_type_tag

      template <typename T>
      struct precision
      {  static void set(std::iostream&) {}  };

      #define strtk_register_iostream_precision(T) \
      template<> struct precision<T> { static void set(std::iostream& s, const std::streamsize& p = 10) { s.precision(p);} };

      strtk_register_iostream_precision(float)
      strtk_register_iostream_precision(double)
      strtk_register_iostream_precision(long double)

      #undef strtk_register_iostream_precision

      template <typename Iterator, typename T, typename Tag>
      inline bool string_to_type_converter_impl(Iterator& begin, const Iterator end, T& t, not_supported_type_tag)
      {
         #ifdef strtk_enable_lexical_cast
            try
            {
               t = boost::lexical_cast<T>(std::string(begin,end));
            }
            catch (const boost::bad_lexical_cast&)
            {
               return false;
            }
            begin = end;
            return true;
         #else
            try
            {
               std::stringstream ss(std::string(begin,end));
               ss >> t;
            }
            catch (const std::exception&)
            {
               return false;
            }
            begin = end;
            return true;
         #endif
      }

      template <typename Iterator>
      inline bool string_to_type_converter_impl(Iterator& begin, const Iterator end, strtk::util::value& v, value_type_tag)
      {
         return v(begin,end);
      }

      template <typename Iterator>
      inline bool string_to_type_converter_impl(Iterator& begin, const Iterator end, std::string& t, stdstring_type_tag)
      {
         t.assign(begin,end);
         begin = end;
         return true;
      }

      template <typename Iterator, typename Expect>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, Expect& t, expect_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename Like>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, Like& t, like_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename InRange>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, InRange& t, inrange_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename TrimToken>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, TrimToken& t, trim_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename CaseToken>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, CaseToken& t, lcase_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename CaseToken>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, CaseToken& t, ucase_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename Array>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, Array& t, fillchararray_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename TruncatedInt>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, TruncatedInt& t, truncint_type_tag)
      {
         if (!t(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename T>
      inline bool string_to_type_converter_impl(Iterator& itr_external, const Iterator end, T& result, unsigned_type_tag)
      {
         if (end == itr_external) return false;

         Iterator itr = itr_external;

         if ('+' == (*itr))
            ++itr;

         if (end == itr)
            return false;

         while ((end != itr) && ('0' == (*itr))) ++itr;
         const std::size_t length = std::distance(itr,end);

         if (length > numeric<T>::length)
            return false;

         static const std::size_t bound_length = numeric<T>::bound_length;
         T t  = 0;

         if (0 != length)
         {
            std::size_t interim_length = std::min<std::size_t>(bound_length,length);
            const Iterator interim_end = itr + interim_length;
            unsigned int digit[8];
            T t0 = 0;
            T t1 = 0;
            T t2 = 0;
            T t3 = 0;

            //Needed for incompetent and broken msvc compiler versions
            #ifdef _MSC_VER
            #pragma warning(push)
            #pragma warning(disable: 4127)
            #endif

            while (interim_length > 7)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9) ||
                   ((digit[2] = (itr[2] - '0')) > 9) ||
                   ((digit[3] = (itr[3] - '0')) > 9) ||
                   ((digit[4] = (itr[4] - '0')) > 9) ||
                   ((digit[5] = (itr[5] - '0')) > 9) ||
                   ((digit[6] = (itr[6] - '0')) > 9) ||
                   ((digit[7] = (itr[7] - '0')) > 9))
                  return false;
               else
               {
                  t0 = static_cast<T>(digit[0] * 10000000 + digit[1] * 1000000);
                  t1 = static_cast<T>(digit[2] * 100000   + digit[3] *   10000);
                  t2 = static_cast<T>(digit[4] * 1000     + digit[5] *     100);
                  t3 = static_cast<T>(digit[6] * 10       + digit[7]          );
                  t  = t0 + t1 + t2 + t3 + static_cast<T>(t * 100000000);
                  itr += 8;
                  interim_length -= 8;
               }
            }

            while (interim_length > 3)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9) ||
                   ((digit[2] = (itr[2] - '0')) > 9) ||
                   ((digit[3] = (itr[3] - '0')) > 9))
                  return false;
               else
               {
                  t1 = static_cast<T>(digit[0] * 1000 + digit[1] * 100);
                  t2 = static_cast<T>(digit[2] * 10   + digit[3]      );
                  t3 = static_cast<T>(t * 10000                       );
                  t  = t1 + t2 + t3;
                  itr += 4;
                  interim_length -= 4;
               }
            }

            while (interim_length > 1)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9))
                  return false;
               else
               {
                  t1 = static_cast<T>(digit[0] * 10 + digit[1]);
                  t2 = static_cast<T>(t * 100                 );
                  t  = t1 + t2;
                  itr += 2;
                  interim_length -= 2;
               }
            }

            //Needed for incompetent and broken msvc compiler versions.
            #ifdef _MSC_VER
            #pragma warning(pop)
            #endif

            if (interim_length)
            {
               if ((digit[0] = (itr[0] - '0')) < 10)
               {
                  t = static_cast<T>(digit[0] + t * 10);
                  ++itr;
               }
               else
                  return false;
            }

            if (interim_end != end)
            {
               if (1 == std::distance(interim_end,end))
               {
                  typedef unsigned long long int num_type;
                  static const num_type max               = static_cast<num_type>(std::numeric_limits<T>::max());
                  static const num_type penultimate_bound = static_cast<num_type>(max / 10);
                  static const num_type final_digit       = static_cast<num_type>(max % 10);

                  digit[0] = static_cast<unsigned int>(*itr - '0');
                  if (digit[0] <= 9)
                  {
                     if (t > penultimate_bound)
                        return false;
                     else if ((penultimate_bound == t) && (final_digit < digit[0]))
                        return false;
                     t = static_cast<T>(digit[0] + t * 10);
                  }
                  else
                     return false;
               }
               else
                  return false;
            }
         }

         result = static_cast<T>(t);
         return true;
      }

      template <typename Iterator, typename T>
      inline bool string_to_type_converter_impl(Iterator& itr_external, const Iterator end, T& result, signed_type_tag)
      {
         if (end == itr_external) return false;

         Iterator itr = itr_external;

         bool negative = false;

         if ('+' == (*itr))
            ++itr;
         else if ('-' == (*itr))
         {
            ++itr;
            negative = true;
         }

         if (end == itr) return false;

         while ((end != itr) && ('0' == (*itr))) ++itr;

         const std::size_t length = std::distance(itr,end);

         if (length > numeric<T>::length)
            return false;

         static const std::size_t bound_length = numeric<T>::bound_length;
         T t  = 0;

         if (0 != length)
         {
            std::size_t interim_length = std::min<std::size_t>(bound_length,length);
            const Iterator interim_end = itr + interim_length;
            unsigned int digit[8];
            T t0 = 0;
            T t1 = 0;
            T t2 = 0;
            T t3 = 0;

            //Needed for incompetent and broken msvc compiler versions
            #ifdef _MSC_VER
            #pragma warning(push)
            #pragma warning(disable: 4127)
            #endif

            while (interim_length > 7)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9) ||
                   ((digit[2] = (itr[2] - '0')) > 9) ||
                   ((digit[3] = (itr[3] - '0')) > 9) ||
                   ((digit[4] = (itr[4] - '0')) > 9) ||
                   ((digit[5] = (itr[5] - '0')) > 9) ||
                   ((digit[6] = (itr[6] - '0')) > 9) ||
                   ((digit[7] = (itr[7] - '0')) > 9) )
                  return false;
               else
               {
                  t0 = static_cast<T>(digit[0] * 10000000 + digit[1] * 1000000);
                  t1 = static_cast<T>(digit[2] * 100000   + digit[3] *   10000);
                  t2 = static_cast<T>(digit[4] * 1000     + digit[5] *     100);
                  t3 = static_cast<T>(digit[6] * 10       + digit[7]          );
                  t  = t0 + t1 + t2 + t3 + static_cast<T>(t * 100000000);
                  itr += 8;
                  interim_length -= 8;
               }
            }

            while (interim_length > 3)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9) ||
                   ((digit[2] = (itr[2] - '0')) > 9) ||
                   ((digit[3] = (itr[3] - '0')) > 9) )
                  return false;
               else
               {
                  t0 = static_cast<T>(digit[0] * 1000 + digit[1] * 100);
                  t1 = static_cast<T>(digit[2] * 10   + digit[3]      );
                  t  = t0 + t1 + static_cast<T>(t * 10000);
                  itr += 4;
                  interim_length -= 4;
               }
            }

            while (interim_length > 2)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9) ||
                   ((digit[2] = (itr[2] - '0')) > 9))
                  return false;
               else
               {
                  t0 = static_cast<T>(digit[0] * 100 + digit[1] * 10);
                  t1 = static_cast<T>(t * 1000 + digit[2]           );
                  t  = t0 + t1;
                  itr += 3;
                  interim_length -= 3;
               }
            }

            while (interim_length > 1)
            {
               if (((digit[0] = (itr[0] - '0')) > 9) ||
                   ((digit[1] = (itr[1] - '0')) > 9))
                  return false;
               else
               {
                  t0 = static_cast<T>(digit[0] * 10 + digit[1]);
                  t  = t0 + static_cast<T>(t * 100);
                  itr += 2;
                  interim_length -= 2;
               }
            }

            //Needed for incompetent and broken msvc compiler versions.
            #ifdef _MSC_VER
            #pragma warning(pop)
            #endif

            if (interim_length)
            {
               if ((digit[0] = (itr[0] - '0')) < 10)
               {
                  t = static_cast<T>(digit[0] + t * 10);
                  ++itr;
               }
               else
                  return false;
            }

            if (interim_end != end)
            {
               if (1 == std::distance(interim_end,end))
               {
                  typedef unsigned long long int num_type;
                  static const num_type max = static_cast<num_type>(std::numeric_limits<T>::max());
                  static const num_type min = static_cast<num_type>(static_cast<long long>(-1) * std::numeric_limits<T>::min());
                  static const num_type positive_penultimate_bound = static_cast<num_type>(max / 10);
                  static const num_type negative_penultimate_bound = static_cast<num_type>(min / 10);
                  static const num_type positive_final_digit = static_cast<num_type>(max % 10);
                  static const num_type negative_final_digit = static_cast<num_type>(min % 10);

                  digit[0] = static_cast<unsigned int>(*itr - '0');

                  if (digit[0] < 10)
                  {
                     if (negative)
                     {
                        if (static_cast<num_type>(t) > negative_penultimate_bound)
                           return false;
                        else if (
                                 (negative_penultimate_bound == static_cast<num_type>(t)) &&
                                 (negative_final_digit < digit[0])
                                )
                           return false;
                     }
                     else
                     {
                        if (static_cast<num_type>(t) > positive_penultimate_bound)
                           return false;
                        else if (
                                 (positive_penultimate_bound == static_cast<num_type>(t)) &&
                                 (positive_final_digit < digit[0])
                                )
                           return false;
                     }
                     t = static_cast<T>(digit[0] + t * 10);
                  }
                  else
                     return false;
               }
               else
                  return false;
            }
         }
         itr_external = itr;
         result = static_cast<T>((negative) ? -t : t);
         return true;
      }

      template <typename Iterator, typename T>
      inline bool string_to_type_converter_impl_ref(Iterator& itr, const Iterator end, T& result, signed_type_tag)
      {
         if (end == itr) return false;

         T t = 0;
         bool negative = false;

         if ('+' == (*itr))
            ++itr;
         else if ('-' == (*itr))
         {
            ++itr;
            negative = true;
         }

         if (end == itr)
            return false;

         unsigned int digit_count = 0;
         while ((end != itr) && ('0' == (*itr))) ++itr;

         bool return_result = true;
         while (end != itr)
         {
            const unsigned char digit = (*itr - '0');
            if (digit > 9)
            {
               return_result = false;
               break;
            }

            if ((++digit_count) <= numeric<T>::bound_length)
            {
               t *= 10;
               t += digit;
            }
            else
            {
               typedef unsigned long long int base_type;
               static const base_type max_limit = +std::numeric_limits<T>::max();
               static const base_type min_limit = -std::numeric_limits<T>::min();
               base_type tmp = static_cast<base_type>(t) * 10 + digit;
               if (negative && static_cast<base_type>(tmp) > min_limit)
                  return_result = false;
               else if (static_cast<base_type>(tmp) > max_limit)
                  return_result = false;
               t = static_cast<T>(tmp);
            }
            ++itr;
         }

         result = static_cast<T>((negative) ? -t : t);
         return return_result;
      }

      template <typename Iterator, typename T>
      inline bool parse_nan(Iterator& itr, const Iterator end, T& t)
      {
         typedef typename std::iterator_traits<Iterator>::value_type type;
         static const std::size_t nan_length = 3;
         if (std::distance(itr,end) != static_cast<int>(nan_length))
            return false;
         if (static_cast<type>('n') == (*itr))
         {
            if ((static_cast<type>('a') != *(itr + 1)) || (static_cast<type>('n') != *(itr + 2)))
            {
               return false;
            }
         }
         else if ((static_cast<type>('A') != *(itr + 1)) || (static_cast<type>('N') != *(itr + 2)))
         {
            return false;
         }
         t = std::numeric_limits<T>::quiet_NaN();
         return true;
      }

      template <typename Iterator, typename T>
      inline bool parse_inf(Iterator& itr, const Iterator end, T& t, bool negative)
      {
         static const char inf_uc[] = "INFINITY";
         static const char inf_lc[] = "infinity";
         static const std::size_t inf_length = 8;
         const std::size_t length = std::distance(itr,end);
         if ((3 != length) && (inf_length != length))
            return false;
         const char* inf_itr = ('i' == (*itr)) ? inf_lc : inf_uc;
         while (end != itr)
         {
            if (*inf_itr == static_cast<char>(*itr))
            {
               ++itr;
               ++inf_itr;
               continue;
            }
            else
               return false;
         }
         if (negative)
            t = -std::numeric_limits<T>::infinity();
         else
            t =  std::numeric_limits<T>::infinity();
         return true;
      }

      template <typename RealType> struct real_type {};
      template <> struct real_type<float>       { typedef double type;      };
      template <> struct real_type<double>      { typedef double type;      };
      template <> struct real_type<long double> { typedef long double type; };

      template <typename Iterator, typename T>
      inline bool string_to_type_converter_impl(Iterator& itr_external, const Iterator end, T& t, real_type_tag)
      {
         typedef typename real_type<T>::type real_t;
         if (end == itr_external) return false;
         Iterator itr = itr_external;
         real_t d = real_t(0);
         bool negative = false;
         if ('+' == (*itr))
            ++itr;
         else if ('-' == (*itr))
         {
            ++itr;
            negative = true;
         }

         if (end == itr)
            return false;

         if (('I' <= (*itr)) && ((*itr) <= 'n'))
         {
            if (('i' == (*itr)) || ('I' == (*itr)))
            {
               return parse_inf(itr,end,t,negative);
            }
            else if (('n' == (*itr)) || ('N' == (*itr)))
            {
               return parse_nan(itr,end,t);
            }
            else
               return false;
         }

         bool instate = false;
         int pre_decimal = 0;

         if ('.' != (*itr))
         {
            const Iterator curr = itr;
            while ((end != itr) && ('0' == (*itr))) ++itr;
            const Iterator post_zero_cull_itr = itr;
            unsigned char digit = 0;

            #define parse_digit_1 \
            if ((digit = static_cast<unsigned char>((*itr) - '0')) < 10) { d *= real_t(10); d += digit; } else break; if (end == ++itr) break; \

            #define parse_digit_2 \
            if ((digit = static_cast<unsigned char>((*itr) - '0')) < 10) { d *= real_t(10); d += digit; } else break; ++itr;\

            while (end != itr)
            {
               parse_digit_1
               parse_digit_1
               parse_digit_1
               parse_digit_1
               parse_digit_1
               parse_digit_1
               parse_digit_1
               parse_digit_2
            }
            #undef parse_digit_1
            #undef parse_digit_2
            if (curr != itr) instate = true;
            pre_decimal = static_cast<int>(std::distance(post_zero_cull_itr,itr));
         }

         int exponent = 0;

         if (end != itr)
         {
            if ('.' == (*itr))
            {
               ++itr;
               const Iterator curr = itr;
               unsigned char digit = 0;

               #define parse_digit_1 \
               if ((digit = static_cast<unsigned char>((*itr) - '0')) < 10) { d *= real_t(10); d += digit; } else break; if (end == ++itr) break; \

               #define parse_digit_2 \
               if ((digit = static_cast<unsigned char>((*itr) - '0')) < 10) { d *= real_t(10); d += digit; } else break; ++itr;\

               while (end != itr)
               {
                  parse_digit_1
                  parse_digit_1
                  parse_digit_1
                  parse_digit_1
                  parse_digit_1
                  parse_digit_1
                  parse_digit_1
                  parse_digit_2
               }
               #undef parse_digit_1
               #undef parse_digit_2
               if (curr != itr) instate = true;
               exponent -= static_cast<int>(std::distance(curr,itr));
            }

            if (end != itr)
            {
               typename std::iterator_traits<Iterator>::value_type c = (*itr);

               if (('e' == c) || ('E' == c))
               {
                  ++itr;
                  int exp = 0;
                  if (!details::string_to_type_converter_impl_ref(itr,end,exp,details::signed_type_tag()))
                  {
                     if (end == itr)
                        return false;
                     else
                        c = (*itr);
                  }

                  if ((exp < numeric<T>::min_exp) || (numeric<T>::max_exp < exp))
                     return false;

                  exponent += exp;
               }

               if (('f' == c) || ('F' == c) || ('l' == c) || ('L' == c))
                  ++itr;
               else if ('#' == c)
               {
                  ++itr;
                  if (end == itr)
                     return false;
                  if ((10.0 != d) || (exponent != -1))
                     return false;
                  if (('I' <= (*itr)) && ((*itr) <= 'n'))
                  {
                     if (('i' == (*itr)) || ('I' == (*itr)))
                     {
                        return parse_inf(itr,end,t,negative);
                     }
                     else if (('n' == (*itr)) || ('N' == (*itr)))
                     {
                        return parse_nan(itr,end,t);
                     }
                     else
                        return false;
                  }
                  return false;
               }
            }
         }

         if ((end != itr) || (!instate))
            return false;

         if (0 != exponent)
         {
            if (
                 (std::numeric_limits<T>::max_exponent10 < (exponent + pre_decimal)) ||
                 (std::numeric_limits<T>::min_exponent10 > (exponent + pre_decimal))
               )
            {
               return false;
            }

            const int e = std::abs(exponent);
            static const double fract10[] =
                         {
                            0.0,
                            1.0E+001, 1.0E+002, 1.0E+003, 1.0E+004, 1.0E+005, 1.0E+006, 1.0E+007, 1.0E+008, 1.0E+009, 1.0E+010,
                            1.0E+011, 1.0E+012, 1.0E+013, 1.0E+014, 1.0E+015, 1.0E+016, 1.0E+017, 1.0E+018, 1.0E+019, 1.0E+020,
                            1.0E+021, 1.0E+022, 1.0E+023, 1.0E+024, 1.0E+025, 1.0E+026, 1.0E+027, 1.0E+028, 1.0E+029, 1.0E+030,
                            1.0E+031, 1.0E+032, 1.0E+033, 1.0E+034, 1.0E+035, 1.0E+036, 1.0E+037, 1.0E+038, 1.0E+039, 1.0E+040,
                            1.0E+041, 1.0E+042, 1.0E+043, 1.0E+044, 1.0E+045, 1.0E+046, 1.0E+047, 1.0E+048, 1.0E+049, 1.0E+050,
                            1.0E+051, 1.0E+052, 1.0E+053, 1.0E+054, 1.0E+055, 1.0E+056, 1.0E+057, 1.0E+058, 1.0E+059, 1.0E+060,
                            1.0E+061, 1.0E+062, 1.0E+063, 1.0E+064, 1.0E+065, 1.0E+066, 1.0E+067, 1.0E+068, 1.0E+069, 1.0E+070,
                            1.0E+071, 1.0E+072, 1.0E+073, 1.0E+074, 1.0E+075, 1.0E+076, 1.0E+077, 1.0E+078, 1.0E+079, 1.0E+080,
                            1.0E+081, 1.0E+082, 1.0E+083, 1.0E+084, 1.0E+085, 1.0E+086, 1.0E+087, 1.0E+088, 1.0E+089, 1.0E+090,
                            1.0E+091, 1.0E+092, 1.0E+093, 1.0E+094, 1.0E+095, 1.0E+096, 1.0E+097, 1.0E+098, 1.0E+099, 1.0E+100,
                            1.0E+101, 1.0E+102, 1.0E+103, 1.0E+104, 1.0E+105, 1.0E+106, 1.0E+107, 1.0E+108, 1.0E+109, 1.0E+110,
                            1.0E+111, 1.0E+112, 1.0E+113, 1.0E+114, 1.0E+115, 1.0E+116, 1.0E+117, 1.0E+118, 1.0E+119, 1.0E+120,
                            1.0E+121, 1.0E+122, 1.0E+123, 1.0E+124, 1.0E+125, 1.0E+126, 1.0E+127, 1.0E+128, 1.0E+129, 1.0E+130,
                            1.0E+131, 1.0E+132, 1.0E+133, 1.0E+134, 1.0E+135, 1.0E+136, 1.0E+137, 1.0E+138, 1.0E+139, 1.0E+140,
                            1.0E+141, 1.0E+142, 1.0E+143, 1.0E+144, 1.0E+145, 1.0E+146, 1.0E+147, 1.0E+148, 1.0E+149, 1.0E+150,
                            1.0E+151, 1.0E+152, 1.0E+153, 1.0E+154, 1.0E+155, 1.0E+156, 1.0E+157, 1.0E+158, 1.0E+159, 1.0E+160,
                            1.0E+161, 1.0E+162, 1.0E+163, 1.0E+164, 1.0E+165, 1.0E+166, 1.0E+167, 1.0E+168, 1.0E+169, 1.0E+170,
                            1.0E+171, 1.0E+172, 1.0E+173, 1.0E+174, 1.0E+175, 1.0E+176, 1.0E+177, 1.0E+178, 1.0E+179, 1.0E+180,
                            1.0E+181, 1.0E+182, 1.0E+183, 1.0E+184, 1.0E+185, 1.0E+186, 1.0E+187, 1.0E+188, 1.0E+189, 1.0E+190,
                            1.0E+191, 1.0E+192, 1.0E+193, 1.0E+194, 1.0E+195, 1.0E+196, 1.0E+197, 1.0E+198, 1.0E+199, 1.0E+200,
                            1.0E+201, 1.0E+202, 1.0E+203, 1.0E+204, 1.0E+205, 1.0E+206, 1.0E+207, 1.0E+208, 1.0E+209, 1.0E+210,
                            1.0E+211, 1.0E+212, 1.0E+213, 1.0E+214, 1.0E+215, 1.0E+216, 1.0E+217, 1.0E+218, 1.0E+219, 1.0E+220,
                            1.0E+221, 1.0E+222, 1.0E+223, 1.0E+224, 1.0E+225, 1.0E+226, 1.0E+227, 1.0E+228, 1.0E+229, 1.0E+230,
                            1.0E+231, 1.0E+232, 1.0E+233, 1.0E+234, 1.0E+235, 1.0E+236, 1.0E+237, 1.0E+238, 1.0E+239, 1.0E+240,
                            1.0E+241, 1.0E+242, 1.0E+243, 1.0E+244, 1.0E+245, 1.0E+246, 1.0E+247, 1.0E+248, 1.0E+249, 1.0E+250,
                            1.0E+251, 1.0E+252, 1.0E+253, 1.0E+254, 1.0E+255, 1.0E+256, 1.0E+257, 1.0E+258, 1.0E+259, 1.0E+260,
                            1.0E+261, 1.0E+262, 1.0E+263, 1.0E+264, 1.0E+265, 1.0E+266, 1.0E+267, 1.0E+268, 1.0E+269, 1.0E+270,
                            1.0E+271, 1.0E+272, 1.0E+273, 1.0E+274, 1.0E+275, 1.0E+276, 1.0E+277, 1.0E+278, 1.0E+279, 1.0E+280,
                            1.0E+281, 1.0E+282, 1.0E+283, 1.0E+284, 1.0E+285, 1.0E+286, 1.0E+287, 1.0E+288, 1.0E+289, 1.0E+290,
                            1.0E+291, 1.0E+292, 1.0E+293, 1.0E+294, 1.0E+295, 1.0E+296, 1.0E+297, 1.0E+298, 1.0E+299, 1.0E+300,
                            1.0E+301, 1.0E+302, 1.0E+303, 1.0E+304, 1.0E+305, 1.0E+306, 1.0E+307, 1.0E+308
                         };

            static const std::size_t fract10_size = sizeof(fract10) / sizeof(double);

            if (d != real_t(0))
            {
               if (static_cast<std::size_t>(e) < fract10_size)
               {
                  if (exponent > 0)
                     d *= fract10[e];
                  else
                     d /= fract10[e];
               }
               else
                  d *= std::pow(real_t(10), real_t(10) * exponent);
            }
         }

         t = static_cast<T>((negative) ? -d : d);
         return true;
      }

      template <typename Iterator, typename T>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, T& t, byte_type_tag)
      {
         if (1 != std::distance(itr,end))
            return false;
         t = static_cast<T>(*itr);
         itr = end;
         return true;
      }

      template <typename Iterator>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, bool& t, bool_type_tag)
      {
         if (1 != std::distance(itr,end))
            return false;
         t = (('0' == (*itr)) ? false : true);
         itr = end;
         return true;
      }

      template <typename Iterator, typename IgnoreTokenType>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, IgnoreTokenType&, ignore_token_type_tag)
      {
         itr = end;
         return true;
      }

      template <typename Iterator, typename HexSinkType>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, HexSinkType& t, hex_number_type_tag)
      {
         t = std::pair<Iterator,Iterator>(itr,end);
         if (!t.valid())
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename HexSinkType>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, HexSinkType& t, hex_string_type_tag)
      {
         t = std::pair<Iterator,Iterator>(itr,end);
         if (!t.valid())
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename Base64SinkType>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, Base64SinkType& t, base64_type_tag)
      {
         t = std::pair<Iterator,Iterator>(itr,end);
         if (!t.valid())
            return false;
         itr = end;
         return true;
      }

      template <typename Iterator, typename SinkType>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, SinkType& t, sink_type_tag)
      {
         if (!t.parse(itr,end))
            return false;
         itr = end;
         return true;
      }

      template <typename T>
      inline bool type_to_string_converter_impl(const T& t, std::string& s, not_supported_type_tag)
      {
         #ifdef strtk_enable_lexical_cast
            try
            {
               s = boost::lexical_cast<std::string>(t);
            }
            catch (const boost::bad_lexical_cast&)
            {
               return false;
            }
         #else
            try
            {
               std::stringstream ss;
               precision<T>::set(ss);
               ss << t;
               s = ss.str();
            }
            catch (const std::exception&)
            {
               return false;
            }
         #endif
         return true;
      }

      template <typename T>
      inline bool type_to_string_converter_impl(T value, std::string& result, unsigned_type_tag)
      {
         static const std::size_t radix = 10;
         static const std::size_t radix_sqr = radix * radix;
         static const std::size_t radix_cube = radix * radix * radix;
         static const std::size_t buffer_size = ((strtk::details::numeric<T>::size < 16) ? 16 : 32);
         unsigned char buffer[buffer_size];
         unsigned char* itr = buffer + buffer_size;

         if (value)
         {
            while (value >= static_cast<T>(radix_sqr))
            {
               itr -= 3;
               T temp_v = value / radix_cube;
               memcpy(itr,&details::rev_3digit_lut[3 * (value - (temp_v * radix_cube))],3);
               value = temp_v;
            }

            while (value >= static_cast<T>(radix))
            {
               itr -= 2;
               T temp_v = value / radix_sqr;
               memcpy(itr,&details::rev_2digit_lut[2 * (value - (temp_v * radix_sqr))],2);
               value = temp_v;
            }

            if (value)
            {
               *(--itr) = static_cast<unsigned char>('0' + value);
            }
         }
         else
            *(--itr) = '0';

         result.assign(reinterpret_cast<char*>(itr), (buffer + buffer_size) - itr);
         return true;
      }

      template <typename T>
      struct tsci_type {};

      #define define_tsci_type(Type,ReType) \
      template <>                           \
      struct tsci_type<Type>                \
      {                                     \
         typedef ReType  type;              \
      };                                    \

      define_tsci_type(short    ,unsigned short    )
      define_tsci_type(int      ,unsigned int      )
      define_tsci_type(long     ,unsigned long     )
      define_tsci_type(long long,unsigned long long)

      #undef define_tsci_type

      template <typename T>
      inline bool type_to_string_converter_impl(T valuex, std::string& result, signed_type_tag)
      {
         static const std::size_t radix = 10;
         static const std::size_t radix_sqr   = radix * radix;
         static const std::size_t radix_cube  = radix * radix * radix;
         static const std::size_t buffer_size = ((strtk::details::numeric<T>::size < 16) ? 16 : 32);
         unsigned char buffer[buffer_size];
         unsigned char* itr = buffer + buffer_size;
         bool negative = (valuex < 0);
         typedef typename tsci_type<T>::type TT;
         TT value = (negative) ? -valuex : valuex;

         if (value)
         {
            while (value >= static_cast<T>(radix_sqr))
            {
               itr -= 3;
               T temp_v = value / radix_cube;
               memcpy(itr,&details::rev_3digit_lut[3 * (value - (temp_v * radix_cube))],3);
               value = temp_v;
            }

            while (value >= static_cast<T>(radix))
            {
               itr -= 2;
               T temp_v = value / radix_sqr;
               memcpy(itr,&details::rev_2digit_lut[2 * (value - (temp_v * radix_sqr))],2);
               value = temp_v;
            }

            if (value)
            {
               *(--itr) = static_cast<unsigned char>('0' + value);
            }

            if (negative)
            {
               *(--itr) = '-';
            }
         }
         else
            *(--itr) = '0';

         result.assign(reinterpret_cast<char*>(itr), (buffer + buffer_size) - itr);
         return true;
      }

      template <typename T>
      inline bool type_to_string_converter_impl(const T& value, std::string& result, byte_type_tag)
      {
         result.resize(1);
         result[0] = static_cast<char>(value);
         return true;
      }

      inline bool type_to_string_converter_impl(const bool& value, std::string& result, bool_type_tag)
      {
         result.resize(1);
         result[0] = value ? '1' : '0';
         return true;
      }

      inline bool type_to_string_converter_impl(const std::string& value, std::string& result, stdstring_type_tag)
      {
         result = value;
         return true;
      }

      template <typename Iterator>
      inline bool type_to_string_converter_impl(const std::pair<Iterator,Iterator>& range, std::string& result, stdstring_range_type_tag)
      {
         result.assign(range.first,range.second);
         return true;
      }

      template <typename SinkType>
      inline bool type_to_string_converter_impl(const SinkType&, std::string&, sink_type_tag)
      {
         //Generic conversion not supported for sinks. Use joins or custom converters.
         return false;
      }

      template <typename STLContainerType>
      inline bool type_to_string_converter_impl(const STLContainerType&, std::string&, stl_seq_type_tag)
      {
         //Generic conversion not supported for stl containers. Use joins or custom converters.
         return false;
      }

      template <typename T>
      inline std::string type_name()
      {
         static const std::string s("Unknown");
         return s;
      }

      #define strtk_register_type_name(Type) \
      template <> inline std::string type_name<Type>() { static const std::string s(#Type); return s; }

      strtk_register_type_name(signed char)
      strtk_register_type_name(unsigned char)
      strtk_register_type_name(short)
      strtk_register_type_name(int)
      strtk_register_type_name(long)
      strtk_register_type_name(long long)
      strtk_register_type_name(unsigned short)
      strtk_register_type_name(unsigned int)
      strtk_register_type_name(unsigned long)
      strtk_register_type_name(unsigned long long int)
      strtk_register_type_name(double)
      strtk_register_type_name(float)
      strtk_register_type_name(long double)
      strtk_register_type_name(std::string)

      #undef strtk_register_type_name

      template <typename T>
      inline std::string type_name(const T&)
      {
         static const std::string s = type_name<T>();
         return s;
      }

      template <typename T1, typename T2>
      inline std::string type_name(const std::pair<T1,T2>& p)
      {
         static const std::string s = std::string("std::pair<" +
                                                  type_name(p.first) +
                                                  "," +
                                                  type_name(p.second) +
                                                  ">");
         return s;
      }

      template <typename T>
      inline std::size_t type_length()
      {
         return numeric<T>::length;
      }

      template <typename T>
      inline std::size_t type_length(const T&)
      {
         return type_length<T>();
      }

      inline std::size_t type_length(const std::string& s)
      {
         return s.size();
      }

      template <typename T1, typename T2>
      inline std::size_t type_length(const std::pair<T1,T2>&)
      {
         return type_length<T1>() + type_length<T2>();
      }

   } // namespace details

   template <typename T>
   inline std::string type_name(const T& t)
   {
      static const std::string s = details::type_name<T>(t);
      return s;
   }

   template <typename T, std::size_t N>
   inline std::string type_name(const T(&)[N])
   {
      static const std::string s = details::type_name<T>() +
                                   std::string("[") + type_to_string(N) + std::string("]");
      return s;
   }

   template <typename T1, typename T2>
   inline std::string type_name(const std::pair<T1,T2>& p)
   {
      static const std::string s = std::string("std::pair<" +
                                               type_name(p.first) +
                                               "," +
                                               type_name(p.second) +
                                               ">");
      return s;
   }

   #define strtk_register_sequence_type_name(Type)                                                       \
   template <typename T, typename Allocator>                                                             \
   inline std::string type_name(const Type<T,Allocator>&)                                                \
   {                                                                                                     \
      static const std::string s = std::string(#Type) + std::string("<" + details::type_name<T>() + ">");\
      return s;                                                                                          \
   }                                                                                                     \

   #define strtk_register_set_type_name(Type)                                                            \
   template <typename T, typename Comparator, typename Allocator>                                        \
   inline std::string type_name(const Type<T,Comparator,Allocator>&)                                     \
   {                                                                                                     \
      static const std::string s = std::string(#Type) + std::string("<" + details::type_name<T>() + ">");\
      return s;                                                                                          \
   }                                                                                                     \

   strtk_register_sequence_type_name(std::vector)
   strtk_register_sequence_type_name(std::deque)
   strtk_register_sequence_type_name(std::list)
   strtk_register_set_type_name(std::set)
   strtk_register_set_type_name(std::multiset)

   template <typename T>
   inline std::size_t type_length()
   {
      return details::type_length<T>();
   }

   template <typename T>
   inline std::size_t type_length(const T&)
   {
      return type_length<T>();
   }

   class ext_string
   {
   public:

      explicit ext_string()
      {}

      explicit ext_string(const std::string& s)
      : s_(s)
      {}

      explicit ext_string(const char* s)
      : s_(s)
      {}

      explicit ext_string(const range::adapter<char>& r)
      : s_(r.begin(),r.end())
      {}

      ext_string(const ext_string& es)
      : s_(es.s_)
      {}

      template <typename T>
      inline ext_string& operator << (const T& t)
      {
         s_ += type_to_string(t);
         return (*this);
      }

      inline operator std::string () const
      {
         return s_;
      }

      inline std::string clone() const
      {
         return s_;
      }

      inline const std::string& as_string() const
      {
         return s_;
      }

      inline std::string& as_string()
      {
         return s_;
      }

      template <typename T>
      inline T as_type() const
      {
         return string_to_type_converter<T>(s_);
      }

      template <typename T>
      inline bool as_type(T& t) const
      {
         return string_to_type_converter(s_,t);
      }

      inline bool imatch(const std::string& s) const
      {
         return strtk::imatch(s_,s);
      }

      inline bool imatch(const ext_string& es) const
      {
         return strtk::imatch(s_,es.s_);
      }

      inline ext_string& to_lowercase()
      {
         convert_to_lowercase(s_);
         return (*this);
      }

      inline ext_string& to_uppercase()
      {
         convert_to_uppercase(s_);
         return (*this);
      }

      template <typename Predicate>
      inline ext_string& remove_leading(const Predicate& p)
      {
         if (s_.empty()) return (*this);
         strtk::remove_leading(p,s_);
         return (*this);
      }

      inline ext_string& remove_leading(const std::string& removal_set)
      {
         if (removal_set.empty())
            return (*this);
         else if (1 == removal_set.size())
            strtk::remove_leading(single_delimiter_predicate<std::string::value_type>(removal_set[0]),s_);
         else
            strtk::remove_leading(multiple_char_delimiter_predicate(removal_set),s_);
         return (*this);
      }

      template <typename Predicate>
      inline ext_string& remove_trailing(const Predicate& p)
      {
         if (s_.empty()) return (*this);
         strtk::remove_trailing(p,s_);
         return (*this);
      }

      inline ext_string& remove_trailing(const std::string& removal_set)
      {
         if (removal_set.empty())
            return (*this);
         else if (1 == removal_set.size())
            strtk::remove_trailing(single_delimiter_predicate<std::string::value_type>(removal_set[0]),s_);
         else
            strtk::remove_trailing(multiple_char_delimiter_predicate(removal_set),s_);
         return (*this);
      }

      template <typename T>
      inline ext_string& operator += (const T& t)
      {
         s_.append(type_to_string(t));
         return (*this);
      }

      inline ext_string& operator -= (const std::string& pattern)
      {
         replace(pattern,"");
         return (*this);
      }

      inline ext_string& operator *= (const std::size_t& n)
      {
         strtk::replicate_inplace(n,s_);
         return (*this);
      }

      inline void replace(const std::string& pattern, const std::string& replace_pattern)
      {
         std::string result;
         result.reserve(s_.size());
         strtk::replace_pattern(s_,pattern,replace_pattern,result);
         s_.assign(result);
      }

      template <typename DelimiterPredicate, typename OutputIterator>
      inline std::size_t split(const DelimiterPredicate& p,
                               OutputIterator out,
                               const split_options::type split_option = split_options::default_mode) const
      {
         return strtk::split(p,s_,out,split_option);
      }

      template <typename DelimiterPredicate,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline std::size_t split(const DelimiterPredicate& p,
                               Sequence<std::string,Allocator>& seq,
                               const split_options::type split_option = split_options::default_mode) const
      {
         return strtk::split(p,s_,range_to_type_back_inserter(seq),split_option);
      }

      template <typename DelimiterPredicate, typename OutputIterator>
      inline std::size_t split_n(const DelimiterPredicate& p,
                                 const std::size_t& n,
                                 OutputIterator out,
                                 const split_options::type split_option = split_options::default_mode) const
      {
         return strtk::split_n(p,s_,n,out,split_option);
      }

      template <typename DelimiterPredicate,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline std::size_t split_n(const DelimiterPredicate& p,
                                 const std::size_t& n,
                                 Sequence<std::string,Allocator>& seq,
                                 const split_options::type split_option = split_options::default_mode) const
      {
         return strtk::split_n(p,s_,n,range_to_type_back_inserter(seq),split_option);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline std::size_t parse(const std::string& delimiters, Sequence<T,Allocator>& seq) const
      {
         return strtk::parse(s_,delimiters,seq);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline std::size_t parse(const char* delimiters, Sequence<T,Allocator>& seq) const
      {
         return parse(std::string(delimiters),seq);
      }

      friend inline ext_string operator * (const std::size_t& n, const ext_string& s);
      friend inline ext_string operator * (const ext_string& s, const std::size_t& n);

      template <typename T>
      friend inline ext_string operator + (const ext_string& s, const T& t);

      template <typename T>
      friend inline ext_string operator + (const T& t, const ext_string& s);

      friend inline ext_string operator - (const ext_string& s, const std::string& pattern);
      friend inline ext_string operator - (const ext_string& s, const char* pattern);
      friend inline ext_string operator - (const ext_string& s, const ext_string& pattern);

      static inline ext_string all_digits()
      {
         static const ext_string digits("0123456789");
         return digits;
      }

      static inline ext_string all_letters()
      {
         static const ext_string letters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
         return letters;
      }

      static inline ext_string all_lowercase_letters()
      {
         static const ext_string letters("abcdefghijklmnopqrstuvwxyz");
         return letters;
      }

      static inline ext_string all_uppercase_letters()
      {
         static const ext_string letters("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
         return letters;
      }

      static inline ext_string all_chars()
      {
         ext_string s;
         s.as_string().resize(256);
         strtk::iota(s.as_string().begin(),
                     s.as_string().end(),
                     static_cast<std::string::value_type>(0x00));
         return s;
      }

   private:

      std::string s_;
   };

   inline ext_string operator * (const std::size_t& n, const ext_string& s)
   {
      return ext_string(replicate(n, s.s_));
   }

   inline ext_string operator * (const ext_string& s, const std::size_t& n)
   {
      return ext_string(replicate(n, s.s_));
   }

   template <typename T>
   inline ext_string operator + (const ext_string& s, const T& t)
   {
      return ext_string(s.s_ + type_to_string(t));
   }

   template <typename T>
   inline ext_string operator + (const T& t, const ext_string& s)
   {
      return ext_string(type_to_string(t) + s.s_);
   }

   inline ext_string operator - (const ext_string& s, const std::string& pattern)
   {
      std::string tmp;
      tmp.reserve(s.s_.size());
      remove_pattern(s,pattern,tmp);
      return ext_string(tmp);
   }

   inline ext_string operator - (const ext_string& s, const char* pattern)
   {
      return s - std::string(pattern);
   }

   inline ext_string operator - (const ext_string& s, const ext_string& pattern)
   {
      return s - std::string(pattern.as_string());
   }

   static inline std::ostream& operator<<(std::ostream& os, const ext_string& es)
   {
      return (os << es.as_string());
   }

   namespace fileio
   {

      inline bool file_exists(const std::string& file_name)
      {
         std::ifstream file(file_name.c_str(), std::ios::binary);
         return ((!file) ? false : true);
      }

      inline std::size_t file_size(const std::string& file_name)
      {
         std::ifstream file(file_name.c_str(),std::ios::binary);
         if (!file) return 0;
         file.seekg (0, std::ios::end);
         return static_cast<std::size_t>(file.tellg());
      }

      inline bool load_file(const std::string& file_name, char* buffer, std::size_t buffer_size)
      {
         std::ifstream in_stream(file_name.c_str(),std::ios::binary);
         if (!in_stream) return false;
         in_stream.read(buffer,static_cast<std::streamsize>(buffer_size));
         in_stream.close();
         return true;
      }

      inline bool load_file(const std::string& file_name, std::string& buffer)
      {
         buffer.resize(file_size(file_name));
         return load_file(file_name,const_cast<char*>(buffer.data()),buffer.size());
      }

      inline bool write_file(const std::string& file_name, char* buffer, const std::size_t& buffer_size)
      {
         std::ofstream out_stream(file_name.c_str(),std::ios::binary);
         if (!out_stream) return false;
         out_stream.write(buffer,static_cast<std::streamsize>(buffer_size));
         out_stream.close();
         return true;
      }

      inline bool write_file(const std::string& file_name, const std::string& buffer)
      {
         return write_file(file_name,const_cast<char*>(buffer.data()),buffer.size());
      }

      inline bool copy_file(const std::string& src_file_name, const std::string& dest_file_name)
      {
         std::ifstream src_file(src_file_name.c_str(),std::ios::binary);
         std::ofstream dest_file(dest_file_name.c_str(),std::ios::binary);
         if (!src_file) return false;
         if (!dest_file) return false;

         static const std::size_t block_size = 16 * one_kilobyte;
         char buffer[block_size];

         std::size_t remaining_bytes = file_size(src_file_name);

         while (remaining_bytes >= block_size)
         {
            src_file.read(&buffer[0],static_cast<std::streamsize>(block_size));
            dest_file.write(&buffer[0],static_cast<std::streamsize>(block_size));
            remaining_bytes -= block_size;
         }

         if (remaining_bytes > 0)
         {
            src_file.read(&buffer[0],static_cast<std::streamsize>(remaining_bytes));
            dest_file.write(&buffer[0],static_cast<std::streamsize>(remaining_bytes));
            remaining_bytes = 0;
         }

         src_file.close();
         dest_file.close();
         return true;
      }

      inline bool concatenate(const std::string& file_name1,
                              const std::string& file_name2,
                              const std::string& output_file_name)
      {
         std::ifstream file1(file_name1.c_str(),std::ios::binary);
         std::ifstream file2(file_name2.c_str(),std::ios::binary);
         std::ofstream out_file(output_file_name.c_str(),std::ios::binary);

         if (!file1 || !file2 || !out_file) return false;

         static const std::size_t block_size = 16 * one_kilobyte;
         char buffer[block_size];
         unsigned int round = 0;
         std::size_t remaining_bytes = 0;

         while (round < 2)
         {
            std::ifstream& input_stream = ((0 == round) ? file1 : file2);
            remaining_bytes = ((0 == round) ? file_size(file_name1) : file_size(file_name2));

            while (remaining_bytes >= block_size)
            {
               input_stream.read(&buffer[0],static_cast<std::streamsize>(block_size));
               out_file.write(&buffer[0],static_cast<std::streamsize>(block_size));
               remaining_bytes -= block_size;
            }

            if (remaining_bytes > 0)
            {
               input_stream.read(&buffer[0],static_cast<std::streamsize>(remaining_bytes));
               out_file.write(&buffer[0],static_cast<std::streamsize>(remaining_bytes));
               remaining_bytes = 0;
            }

            input_stream.close();
            ++round;
         }
         out_file.close();
         return true;
      }

      inline bool files_identical(const std::string& file_name1, const std::string& file_name2)
      {
         std::ifstream file1(file_name1.c_str(),std::ios::binary);
         std::ifstream file2(file_name2.c_str(),std::ios::binary);
         if (!file1) return false;
         if (!file2) return false;
         if (file_size(file_name1) != file_size(file_name2)) return false;

         static const std::size_t block_size = 16 * one_kilobyte;
         char buffer1[block_size];
         char buffer2[block_size];

         std::size_t remaining_bytes = file_size(file_name1);

         while (remaining_bytes >= block_size)
         {
            file1.read(&buffer1[0],static_cast<std::streamsize>(block_size));
            file2.read(&buffer2[0],static_cast<std::streamsize>(block_size));
            if (0 != std::memcmp(buffer1,buffer2,block_size))
               return false;
            remaining_bytes -= block_size;
         }

         if (remaining_bytes > 0)
         {
            file1.read(&buffer1[0],static_cast<std::streamsize>(remaining_bytes));
            file2.read(&buffer2[0],static_cast<std::streamsize>(remaining_bytes));
            if (0 != std::memcmp(buffer1,buffer2,remaining_bytes))
               return false;
            remaining_bytes = 0;
         }

         file1.close();
         file2.close();

         return true;
      }

      namespace details
      {
         template <typename T>
         inline bool read_pod_proxy(std::ifstream& stream, T& t)
         {
            return (false == stream.read(reinterpret_cast<char*>(&t),
                                         static_cast<std::streamsize>(sizeof(T))).fail());
         }

         template <typename T>
         inline bool write_pod_proxy(std::ofstream& stream, const T& t)
         {
            return (false == stream.write(reinterpret_cast<char*>(&t),
                                          static_cast<std::streamsize>(sizeof(T))).fail());
         }
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8,
                typename T9, typename T10>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4,
                           T5& t5, T6& t6, T7& t7, T8& t8,
                           T9& t9, T10& t10)
      {
         return details::read_pod_proxy(stream, t1) &&
                details::read_pod_proxy(stream, t2) &&
                details::read_pod_proxy(stream, t3) &&
                details::read_pod_proxy(stream, t4) &&
                details::read_pod_proxy(stream, t5) &&
                details::read_pod_proxy(stream, t6) &&
                details::read_pod_proxy(stream, t7) &&
                details::read_pod_proxy(stream, t8) &&
                details::read_pod_proxy(stream, t9) &&
                details::read_pod_proxy(stream,t10);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8,
                typename T9>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4,
                           T5& t5, T6& t6, T7& t7, T8& t8,
                           T9& t9)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3) &&
                details::read_pod_proxy(stream,t4) &&
                details::read_pod_proxy(stream,t5) &&
                details::read_pod_proxy(stream,t6) &&
                details::read_pod_proxy(stream,t7) &&
                details::read_pod_proxy(stream,t8) &&
                details::read_pod_proxy(stream,t9);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4,
                           T5& t5, T6& t6, T7& t7, T8& t8)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3) &&
                details::read_pod_proxy(stream,t4) &&
                details::read_pod_proxy(stream,t5) &&
                details::read_pod_proxy(stream,t6) &&
                details::read_pod_proxy(stream,t7) &&
                details::read_pod_proxy(stream,t8);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4,
                           T5& t5, T6& t6, T7& t7)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3) &&
                details::read_pod_proxy(stream,t4) &&
                details::read_pod_proxy(stream,t5) &&
                details::read_pod_proxy(stream,t6) &&
                details::read_pod_proxy(stream,t7);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4,
                           T5& t5, T6& t6)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3) &&
                details::read_pod_proxy(stream,t4) &&
                details::read_pod_proxy(stream,t5) &&
                details::read_pod_proxy(stream,t6);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4,
                           T5& t5)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3) &&
                details::read_pod_proxy(stream,t4) &&
                details::read_pod_proxy(stream,t5);
      }

      template <typename T1, typename T2, typename T3, typename T4>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3, T4& t4)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3) &&
                details::read_pod_proxy(stream,t4);
      }

      template <typename T1, typename T2, typename T3>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2, T3& t3)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2) &&
                details::read_pod_proxy(stream,t3);
      }

      template <typename T1, typename T2>
      inline bool read_pod(std::ifstream& stream,
                           T1& t1, T2& t2)
      {
         return details::read_pod_proxy(stream,t1) &&
                details::read_pod_proxy(stream,t2);
      }

      template <typename T>
      inline bool read_pod(std::ifstream& stream, T& t)
      {
         return details::read_pod_proxy(stream,t);
      }

      template <typename T, std::size_t N>
      inline bool read_pod(std::ifstream& stream, T (&t)[N])
      {
         return (false != stream.read(reinterpret_cast<char*>(&t[0]),sizeof(T) * N).fail());
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool read_pod(std::ifstream& stream,
                           const std::size_t& count,
                           Sequence<T,Allocator>& sequence)
      {
         T t;
         for (std::size_t i = 0; i < count; ++i)
         {
            if (details::read_pod_proxy(stream,t))
               sequence.push_back(t);
            else
               return false;
         }
         return true;
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool read_pod(std::ifstream& stream,
                           const std::size_t& count,
                           std::set<T,Comparator,Allocator>& set)
      {
         T t;
         for (std::size_t i = 0; i < count; ++i)
         {
            if (details::read_pod_proxy(stream,t))
               set.insert(t);
            else
               return false;
         }
         return true;
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool read_pod(std::ifstream& stream,
                           const std::size_t& count,
                           std::multiset<T,Comparator,Allocator>& multiset)
      {
         T t;
         for (std::size_t i = 0; i < count; ++i)
         {
            if (details::read_pod_proxy(stream,t))
               multiset.insert(t);
            else
               return false;
         }
         return true;
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8,
                typename T9, typename T10>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                            const T5& t5, const T6& t6, const T7& t7, const T8& t8,
                            const T9& t9, const T10& t10)
      {
         return details::write_pod_proxy(stream, t1) &&
                details::write_pod_proxy(stream, t2) &&
                details::write_pod_proxy(stream, t3) &&
                details::write_pod_proxy(stream, t4) &&
                details::write_pod_proxy(stream, t5) &&
                details::write_pod_proxy(stream, t6) &&
                details::write_pod_proxy(stream, t7) &&
                details::write_pod_proxy(stream, t8) &&
                details::write_pod_proxy(stream, t9) &&
                details::write_pod_proxy(stream,t10);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8,
                typename T9>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                            const T5& t5, const T6& t6, const T7& t7, const T8& t8,
                            const T9& t9)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3) &&
                details::write_pod_proxy(stream,t4) &&
                details::write_pod_proxy(stream,t5) &&
                details::write_pod_proxy(stream,t6) &&
                details::write_pod_proxy(stream,t7) &&
                details::write_pod_proxy(stream,t8) &&
                details::write_pod_proxy(stream,t9);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                            const T5& t5, const T6& t6, const T7& t7, const T8& t8)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3) &&
                details::write_pod_proxy(stream,t4) &&
                details::write_pod_proxy(stream,t5) &&
                details::write_pod_proxy(stream,t6) &&
                details::write_pod_proxy(stream,t7) &&
                details::write_pod_proxy(stream,t8);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                            const T5& t5, const T6& t6, const T7& t7)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3) &&
                details::write_pod_proxy(stream,t4) &&
                details::write_pod_proxy(stream,t5) &&
                details::write_pod_proxy(stream,t6) &&
                details::write_pod_proxy(stream,t7);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                            const T5& t5, const T6& t6)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3) &&
                details::write_pod_proxy(stream,t4) &&
                details::write_pod_proxy(stream,t5) &&
                details::write_pod_proxy(stream,t6);
      }

      template <typename T1, typename T2, typename T3, typename T4,
                typename T5>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                            const T5& t5)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3) &&
                details::write_pod_proxy(stream,t4) &&
                details::write_pod_proxy(stream,t5);
      }

      template <typename T1, typename T2, typename T3, typename T4>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3, const T4& t4)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3) &&
                details::write_pod_proxy(stream,t4);
      }

      template <typename T1, typename T2, typename T3>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2, const T3& t3)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2) &&
                details::write_pod_proxy(stream,t3);
      }

      template <typename T1, typename T2>
      inline bool write_pod(std::ofstream& stream,
                            const T1& t1, const T2& t2)
      {
         return details::write_pod_proxy(stream,t1) &&
                details::write_pod_proxy(stream,t2);
      }

      template <typename T>
      inline bool write_pod(std::ofstream& stream, const T& t)
      {
         return details::write_pod_proxy(stream,t);
      }

      template <typename T, std::size_t N>
      inline bool write_pod(std::ofstream& stream, T (&t)[N])
      {
         return (false != stream.write(reinterpret_cast<char*>(&t[0]),sizeof(T) * N).fail());
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline bool write_pod(std::ofstream& stream,
                            const Sequence<T,Allocator>& sequence)
      {
         typename Sequence<T,Allocator>::iterator itr = sequence.begin();
         typename Sequence<T,Allocator>::iterator end = sequence.end();
         while (end != itr)
         {
            if (details::write_pod_proxy(stream,*itr))
               ++itr;
            else
               return false;
         }
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool write_pod(std::ofstream& stream,
                            const std::set<T,Comparator,Allocator>& set)
      {
         typename std::set<T,Comparator,Allocator>::iterator itr = set.begin();
         typename std::set<T,Comparator,Allocator>::iterator end = set.end();
         while (end != itr)
         {
            if (details::write_pod_proxy(stream,*itr))
               ++itr;
            else
               return false;
         }
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline bool write_pod(std::ofstream& stream,
                            const std::multiset<T,Comparator,Allocator>& multiset)
      {
         typename std::multiset<T,Comparator,Allocator>::iterator itr = multiset.begin();
         typename std::multiset<T,Comparator,Allocator>::iterator end = multiset.end();
         while (end != itr)
         {
            if (details::write_pod_proxy(stream,*itr))
               ++itr;
            else
               return false;
         }
      }

      inline bool read_at_offset(std::ifstream& stream,
                                 const std::size_t& offset,
                                 char* buffer,
                                 const std::size_t& buffer_size)
      {
         if (!stream) return false;
         stream.seekg(static_cast<std::ifstream::off_type>(offset),std::ios_base::beg);
         if (stream.fail()) return false;
         stream.read(buffer,static_cast<std::streamsize>(buffer_size));
         if (stream.fail()) return false;
         stream.close();
         return true;
      }

      inline bool read_at_offset(const std::string& file_name,
                                 const std::size_t& offset,
                                 char* buffer,
                                 const std::size_t& buffer_size)
      {
         std::ifstream stream(file_name.c_str(), std::ios::in | std::ios::binary);
         if (!stream) return false;
         return read_at_offset(stream,offset,buffer,buffer_size);
      }

      inline bool read_at_offset(const std::string& file_name,
                                 const std::size_t& offset,
                                 std::string& buffer,
                                 const std::size_t& buffer_size)
      {
         std::ifstream stream(file_name.c_str(), std::ios::in | std::ios::binary);
         if (!stream) return false;
         buffer.resize(buffer_size);
         return read_at_offset(stream,
                               offset,
                               const_cast<char*>(buffer.data()),
                               buffer_size);
      }

   } // namespace fileio

   template <typename T1,  typename T2, typename  T3, typename  T4,
             typename T5,  typename T6, typename  T7, typename  T8,
             typename T9, typename T10, typename T11, typename T12>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2&   t2, T3&   t3, T4&   t4,
                                  T5& t5, T6&   t6, T7&   t7, T8&   t8,
                                  T9& t9, T10& t10, T11& t11, T12& t12)
   {
      t1 = (*reinterpret_cast< T1*>(data)); data += sizeof( T1);
      t2 = (*reinterpret_cast< T2*>(data)); data += sizeof( T2);
      t3 = (*reinterpret_cast< T3*>(data)); data += sizeof( T3);
      t4 = (*reinterpret_cast< T4*>(data)); data += sizeof( T4);
      t5 = (*reinterpret_cast< T5*>(data)); data += sizeof( T5);
      t6 = (*reinterpret_cast< T6*>(data)); data += sizeof( T6);
      t7 = (*reinterpret_cast< T7*>(data)); data += sizeof( T7);
      t8 = (*reinterpret_cast< T8*>(data)); data += sizeof( T8);
      t9 = (*reinterpret_cast< T9*>(data)); data += sizeof( T9);
     t10 = (*reinterpret_cast<T10*>(data)); data += sizeof(T10);
     t11 = (*reinterpret_cast<T11*>(data)); data += sizeof(T11);
     t12 = (*reinterpret_cast<T12*>(data)); data += sizeof(T12);
     return data;
   }

   template <typename T1,  typename T2, typename  T3, typename T4,
             typename T5,  typename T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2&   t2, T3&   t3, T4& t4,
                                  T5& t5, T6&   t6, T7&   t7, T8& t8,
                                  T9& t9, T10& t10, T11& t11)
   {
      t1 = (*reinterpret_cast< T1*>(data)); data += sizeof( T1);
      t2 = (*reinterpret_cast< T2*>(data)); data += sizeof( T2);
      t3 = (*reinterpret_cast< T3*>(data)); data += sizeof( T3);
      t4 = (*reinterpret_cast< T4*>(data)); data += sizeof( T4);
      t5 = (*reinterpret_cast< T5*>(data)); data += sizeof( T5);
      t6 = (*reinterpret_cast< T6*>(data)); data += sizeof( T6);
      t7 = (*reinterpret_cast< T7*>(data)); data += sizeof( T7);
      t8 = (*reinterpret_cast< T8*>(data)); data += sizeof( T8);
      t9 = (*reinterpret_cast< T9*>(data)); data += sizeof( T9);
     t10 = (*reinterpret_cast<T10*>(data)); data += sizeof(T10);
     t11 = (*reinterpret_cast<T11*>(data)); data += sizeof(T11);
     return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9, typename T10>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4,
                                  T5& t5, T6& t6, T7& t7, T8& t8,
                                  T9& t9, T10& t10)
   {
       t1 = (*reinterpret_cast< T1*>(data)); data += sizeof( T1);
       t2 = (*reinterpret_cast< T2*>(data)); data += sizeof( T2);
       t3 = (*reinterpret_cast< T3*>(data)); data += sizeof( T3);
       t4 = (*reinterpret_cast< T4*>(data)); data += sizeof( T4);
       t5 = (*reinterpret_cast< T5*>(data)); data += sizeof( T5);
       t6 = (*reinterpret_cast< T6*>(data)); data += sizeof( T6);
       t7 = (*reinterpret_cast< T7*>(data)); data += sizeof( T7);
       t8 = (*reinterpret_cast< T8*>(data)); data += sizeof( T8);
       t9 = (*reinterpret_cast< T9*>(data)); data += sizeof( T9);
      t10 = (*reinterpret_cast<T10*>(data)); data += sizeof(T10);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4,
                                  T5& t5, T6& t6, T7& t7, T8& t8,
                                  T9& t9)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      t4 = (*reinterpret_cast<T4*>(data)); data += sizeof(T4);
      t5 = (*reinterpret_cast<T5*>(data)); data += sizeof(T5);
      t6 = (*reinterpret_cast<T6*>(data)); data += sizeof(T6);
      t7 = (*reinterpret_cast<T7*>(data)); data += sizeof(T7);
      t8 = (*reinterpret_cast<T8*>(data)); data += sizeof(T8);
      t9 = (*reinterpret_cast<T9*>(data)); data += sizeof(T9);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4,
                                  T5& t5, T6& t6, T7& t7, T8& t8)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      t4 = (*reinterpret_cast<T4*>(data)); data += sizeof(T4);
      t5 = (*reinterpret_cast<T5*>(data)); data += sizeof(T5);
      t6 = (*reinterpret_cast<T6*>(data)); data += sizeof(T6);
      t7 = (*reinterpret_cast<T7*>(data)); data += sizeof(T7);
      t8 = (*reinterpret_cast<T8*>(data)); data += sizeof(T8);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4,
                                  T5& t5, T6& t6, T7& t7)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      t4 = (*reinterpret_cast<T4*>(data)); data += sizeof(T4);
      t5 = (*reinterpret_cast<T5*>(data)); data += sizeof(T5);
      t6 = (*reinterpret_cast<T6*>(data)); data += sizeof(T6);
      t7 = (*reinterpret_cast<T7*>(data)); data += sizeof(T7);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4,
                                  T5& t5, T6& t6)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      t4 = (*reinterpret_cast<T4*>(data)); data += sizeof(T4);
      t5 = (*reinterpret_cast<T5*>(data)); data += sizeof(T5);
      t6 = (*reinterpret_cast<T6*>(data)); data += sizeof(T6);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4,
                                  T5& t5)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      t4 = (*reinterpret_cast<T4*>(data)); data += sizeof(T4);
      t5 = (*reinterpret_cast<T5*>(data)); data += sizeof(T5);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3, T4& t4)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      t4 = (*reinterpret_cast<T4*>(data)); data += sizeof(T4);
      return data;
   }

   template <typename T1, typename T2, typename T3>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2, T3& t3)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      t3 = (*reinterpret_cast<T3*>(data)); data += sizeof(T3);
      return data;
   }

   template <typename T1, typename T2>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1, T2& t2)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      t2 = (*reinterpret_cast<T2*>(data)); data += sizeof(T2);
      return data;
   }

   template <typename T1>
   inline unsigned char* read_pod(unsigned char* data,
                                  T1& t1)
   {
      t1 = (*reinterpret_cast<T1*>(data)); data += sizeof(T1);
      return data;
   }

   template <typename T, std::size_t N>
   inline unsigned char* read_pod(unsigned char* data, T (&t)[N])
   {
      T* begin = reinterpret_cast<T*>(data);
      T* end = begin + N;
      std::copy(begin,end,&t[0]);
      return data + (N * sizeof(T));
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline unsigned char* read_pod(unsigned char* data,
                                  const std::size_t& n,
                                  const Sequence<T,Allocator>& sequence)
   {
      T* ptr = reinterpret_cast<T>(data);
      std::copy(ptr, ptr + n, std::back_inserter(sequence));
      return data + (sequence.size() * sizeof(T));
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline unsigned char* read_pod(unsigned char* data,
                                  const std::size_t& n,
                                  const std::set<T,Comparator,Allocator>& set)
   {
      T* ptr = reinterpret_cast<T>(data);
      std::copy(ptr, ptr + n, std::inserter(set,set.begin()));
      return data + (set.size() * sizeof(T));
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline unsigned char* read_pod(unsigned char* data,
                                  const std::size_t& n,
                                  const std::multiset<T,Comparator,Allocator>& multiset)
   {
      T* ptr = reinterpret_cast<T>(data);
      std::copy(ptr, ptr + n, std::inserter(multiset,multiset.begin()));
      return data + (multiset.size() * sizeof(T));
   }

   template <typename T1,  typename T2, typename  T3, typename  T4,
             typename T5,  typename T6, typename  T7, typename  T8,
             typename T9, typename T10, typename T11, typename T12>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2&   t2, const T3&   t3, const T4& t4,
                                   const T5& t5, const T6&   t6, const T7&   t7, const T8& t8,
                                   const T9& t9, const T10& t10, const T11& t11, const T12& t12)
   {
      (*reinterpret_cast< T1*>(data)) =  t1; data += sizeof( T1);
      (*reinterpret_cast< T2*>(data)) =  t2; data += sizeof( T2);
      (*reinterpret_cast< T3*>(data)) =  t3; data += sizeof( T3);
      (*reinterpret_cast< T4*>(data)) =  t4; data += sizeof( T4);
      (*reinterpret_cast< T5*>(data)) =  t5; data += sizeof( T5);
      (*reinterpret_cast< T6*>(data)) =  t6; data += sizeof( T6);
      (*reinterpret_cast< T7*>(data)) =  t7; data += sizeof( T7);
      (*reinterpret_cast< T8*>(data)) =  t8; data += sizeof( T8);
      (*reinterpret_cast< T9*>(data)) =  t9; data += sizeof( T9);
      (*reinterpret_cast<T10*>(data)) = t10; data += sizeof(T10);
      (*reinterpret_cast<T11*>(data)) = t11; data += sizeof(T11);
      (*reinterpret_cast<T12*>(data)) = t12; data += sizeof(T12);
      return data;
   }

   template <typename T1,  typename T2, typename  T3, typename T4,
             typename T5,  typename T6, typename  T7, typename T8,
             typename T9, typename T10, typename T11>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2&   t2, const T3&   t3, const T4& t4,
                                   const T5& t5, const T6&   t6, const T7&   t7, const T8& t8,
                                   const T9& t9, const T10& t10, const T11& t11)
   {
      (*reinterpret_cast< T1*>(data)) =  t1; data += sizeof( T1);
      (*reinterpret_cast< T2*>(data)) =  t2; data += sizeof( T2);
      (*reinterpret_cast< T3*>(data)) =  t3; data += sizeof( T3);
      (*reinterpret_cast< T4*>(data)) =  t4; data += sizeof( T4);
      (*reinterpret_cast< T5*>(data)) =  t5; data += sizeof( T5);
      (*reinterpret_cast< T6*>(data)) =  t6; data += sizeof( T6);
      (*reinterpret_cast< T7*>(data)) =  t7; data += sizeof( T7);
      (*reinterpret_cast< T8*>(data)) =  t8; data += sizeof( T8);
      (*reinterpret_cast< T9*>(data)) =  t9; data += sizeof( T9);
      (*reinterpret_cast<T10*>(data)) = t10; data += sizeof(T10);
      (*reinterpret_cast<T11*>(data)) = t11; data += sizeof(T11);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9, typename T10>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                                   const T5& t5, const T6& t6, const T7& t7, const T8& t8,
                                   const T9& t9, const T10& t10)
   {
      (*reinterpret_cast< T1*>(data)) =  t1; data += sizeof( T1);
      (*reinterpret_cast< T2*>(data)) =  t2; data += sizeof( T2);
      (*reinterpret_cast< T3*>(data)) =  t3; data += sizeof( T3);
      (*reinterpret_cast< T4*>(data)) =  t4; data += sizeof( T4);
      (*reinterpret_cast< T5*>(data)) =  t5; data += sizeof( T5);
      (*reinterpret_cast< T6*>(data)) =  t6; data += sizeof( T6);
      (*reinterpret_cast< T7*>(data)) =  t7; data += sizeof( T7);
      (*reinterpret_cast< T8*>(data)) =  t8; data += sizeof( T8);
      (*reinterpret_cast< T9*>(data)) =  t9; data += sizeof( T9);
      (*reinterpret_cast<T10*>(data)) = t10; data += sizeof(T10);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8,
             typename T9>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                                   const T5& t5, const T6& t6, const T7& t7, const T8& t8,
                                   const T9& t9)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      (*reinterpret_cast<T4*>(data)) = t4; data += sizeof(T4);
      (*reinterpret_cast<T5*>(data)) = t5; data += sizeof(T5);
      (*reinterpret_cast<T6*>(data)) = t6; data += sizeof(T6);
      (*reinterpret_cast<T7*>(data)) = t7; data += sizeof(T7);
      (*reinterpret_cast<T8*>(data)) = t8; data += sizeof(T8);
      (*reinterpret_cast<T9*>(data)) = t9; data += sizeof(T9);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                                   const T5& t5, const T6& t6, const T7& t7, const T8& t8)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      (*reinterpret_cast<T4*>(data)) = t4; data += sizeof(T4);
      (*reinterpret_cast<T5*>(data)) = t5; data += sizeof(T5);
      (*reinterpret_cast<T6*>(data)) = t6; data += sizeof(T6);
      (*reinterpret_cast<T7*>(data)) = t7; data += sizeof(T7);
      (*reinterpret_cast<T8*>(data)) = t8; data += sizeof(T8);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                                   const T5& t5, const T6& t6, const T7& t7)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      (*reinterpret_cast<T4*>(data)) = t4; data += sizeof(T4);
      (*reinterpret_cast<T5*>(data)) = t5; data += sizeof(T5);
      (*reinterpret_cast<T6*>(data)) = t6; data += sizeof(T6);
      (*reinterpret_cast<T7*>(data)) = t7; data += sizeof(T7);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                                   const T5& t5, const T6& t6)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      (*reinterpret_cast<T4*>(data)) = t4; data += sizeof(T4);
      (*reinterpret_cast<T5*>(data)) = t5; data += sizeof(T5);
      (*reinterpret_cast<T6*>(data)) = t6; data += sizeof(T6);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4,
                                   const T5& t5)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      (*reinterpret_cast<T4*>(data)) = t4; data += sizeof(T4);
      (*reinterpret_cast<T5*>(data)) = t5; data += sizeof(T5);
      return data;
   }

   template <typename T1, typename T2, typename T3, typename T4>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3, const T4& t4)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      (*reinterpret_cast<T4*>(data)) = t4; data += sizeof(T4);
      return data;
   }

   template <typename T1, typename T2, typename T3>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2, const T3& t3)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      (*reinterpret_cast<T3*>(data)) = t3; data += sizeof(T3);
      return data;
   }

   template <typename T1, typename T2>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1, const T2& t2)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      (*reinterpret_cast<T2*>(data)) = t2; data += sizeof(T2);
      return data;
   }

   template <typename T1>
   inline unsigned char* write_pod(unsigned char* data,
                                   const T1& t1)
   {
      (*reinterpret_cast<T1*>(data)) = t1; data += sizeof(T1);
      return data;
   }

   template <typename T, std::size_t N>
   inline unsigned char* write_pod(unsigned char* data, const T (&t)[N])
   {
      T* ptr = reinterpret_cast<T*>(data);
      std::copy(t,t + N,ptr);
      return data + (N * sizeof(T));
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline unsigned char* write_pod(unsigned char* data,
                                   const Sequence<T,Allocator>& sequence)
   {
      T* ptr = reinterpret_cast<T>(data);
      std::copy(sequence.begin(),sequence.end(),ptr);
      return data + (sequence.size() * sizeof(T));
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline unsigned char* write_pod(unsigned char* data,
                                   const std::set<T,Comparator,Allocator>& set)
   {
      T* ptr = reinterpret_cast<T>(data);
      std::copy(set.begin(),set.end(),ptr);
      return data + (set.size() * sizeof(T));
   }

   template <typename T,
             typename Comparator,
             typename Allocator>
   inline unsigned char* write_pod(unsigned char* data,
                                   const std::multiset<T,Comparator,Allocator>& multiset)
   {
      T* ptr = reinterpret_cast<T>(data);
      std::copy(multiset.begin(),multiset.end(),ptr);
      return data + (multiset.size() * sizeof(T));
   }

   class string_condition
   {
   private:

      typedef const unsigned char* itr_type;

      inline bool condition_equal(const itr_type begin, const itr_type end) const
      {
         if (s.size() == static_cast<std::size_t>(std::distance(begin,end)))
         {
            return std::equal(s_begin,s_end,begin);
         }
         else
            return false;
      }

      inline bool condition_notequal(const itr_type begin, const itr_type end) const
      {
         if (s.size() == static_cast<std::size_t>(std::distance(begin,end)))
         {
            return !std::equal(s_begin,s_end,begin);
         }
         else
            return true;
      }

      inline bool condition_like(const itr_type begin, const itr_type end) const
      {
         return match(s_begin,s_end,begin,end,(unsigned char)'*',(unsigned char)'?');
      }

      inline bool condition_begins_with(const itr_type begin, const itr_type end) const
      {
         if (s.size() == static_cast<std::size_t>(std::distance(begin,end)))
         {
            return strtk::begins_with(s_begin,s_end,begin,end);
         }
         else
            return false;
      }

      inline bool condition_ends_with(const itr_type begin, const itr_type end) const
      {
         if (s.size() == static_cast<std::size_t>(std::distance(begin,end)))
         {
            return strtk::ends_with(s_begin,s_end,begin,end);
         }
         else
            return false;
      }

      inline bool condition_within(const itr_type begin, const itr_type end) const
      {
         if (s.size() <= static_cast<std::size_t>(std::distance(begin,end)))
         {
            return (end != std::search(begin,end,s_begin,s_end));
         }
         else
            return false;
      }

      inline bool condition_notwithin(const itr_type begin, const itr_type end) const
      {
         if (s.size() <= static_cast<std::size_t>(std::distance(begin,end)))
         {
            return (end == std::search(begin,end,s_begin,s_end));
         }
         else
            return true;
      }

      typedef bool (string_condition::*condition_method)(const itr_type begin, const itr_type end) const;

   public:

      enum condition_type
      {
         equal       =  0,
         notequal    =  1,
         like        =  2,
         begins_with =  4,
         ends_with   =  8,
         within      = 16,
         notwithin   = 32
      };

      inline explicit string_condition(condition_type cond_type, const std::string& str)
      : cond_type_(cond_type),
        s(str),
        s_begin(reinterpret_cast<const unsigned char*>(s.data())),
        s_end(reinterpret_cast<const unsigned char*>(s.data() + str.size())),
        condition_method_(0)
      {
         switch (cond_type_)
         {
            case equal       : condition_method_ = &string_condition::condition_equal;
                               break;
            case notequal    : condition_method_ = &string_condition::condition_notequal;
                               break;
            case like        : condition_method_ = &string_condition::condition_like;
                               break;
            case begins_with : condition_method_ = &string_condition::condition_begins_with;
                               break;
            case ends_with   : condition_method_ = &string_condition::condition_ends_with;
                               break;
            case within      : condition_method_ = &string_condition::condition_within;
                               break;
            case notwithin   : condition_method_ = &string_condition::condition_notwithin;
                               break;
         }
      }

      template <typename Iterator>
      inline bool operator()(const Iterator begin, const Iterator end)
      {
         return ((*this).*condition_method_)(begin,end);
      }

      inline bool operator()(const std::string& str)
      {
         return operator()(reinterpret_cast<const unsigned char*>(str.data()),
                           reinterpret_cast<const unsigned char*>(str.data() + str.size()));
      }

   private:

      condition_type cond_type_;
      std::string s;
      const unsigned char* s_begin;
      const unsigned char* s_end;
      condition_method condition_method_;
   };

   namespace trie
   {
      template <typename KeyIterator, typename ValueType>
      class prefix
      {

         template <typename Iterator,
                   typename Value,
                   typename KeyValue = typename std::iterator_traits<Iterator>::value_type>
         struct node
         {
         public:

            typedef KeyValue key_value_t;
            typedef Value value_t;

            typedef node<Iterator,Value,KeyValue> node_t;
            typedef node_t* node_ptr;
            typedef const node_ptr const_node_ptr;

            typedef std::vector<node_ptr> node_list_t;
            typedef typename node_list_t::const_iterator node_list_iterator;

            explicit node(const key_value_t& key_value)
            : key_value_(key_value),
              value_holder_(false)
            {}

            node(const key_value_t& key_value, const value_t& v)
            : key_value_(key_value),
              value_holder_(true),
              value_(v)
            {}

           ~node()
            {
               if (!node_list_.empty())
               {
                  node_list_iterator itr = node_list_.begin();
                  node_list_iterator end = node_list_.end();
                  while (end != itr)
                  {
                     delete (*itr);
                     ++itr;
                  }
               }
            }

            inline node_ptr get_node(const key_value_t& key_value)
            {
               if (node_list_.empty())
                  return 0;
               node_list_iterator itr = node_list_.begin();
               const node_list_iterator end = node_list_.end();
               while (end != itr)
               {
                  if (key_value == (*itr)->key_value_)
                     return (*itr);
                  else
                     ++itr;
               }
               return 0;
            }

            inline void assign_value(const value_t& v)
            {
               value_ = v;
               value_holder_ = true;
            }

            inline void add_node(node_ptr n)
            {
               node_list_.push_back(n);
            }

            inline bool value_holder() const
            {
               return value_holder_;
            }

            inline const value_t& value() const
            {
               return value_;
            }

            inline const key_value_t& key() const
            {
               return key_value_;
            }

         private:

            node(const node_t& n);
            node_t& operator=(const node_t& n);

            key_value_t key_value_;
            bool value_holder_;
            value_t value_;
            node_list_t node_list_;
         };

      public:

         //typedef KeyIterator key_iterator_t;
         typedef typename std::iterator_traits<KeyIterator>::value_type key_value_t;
         typedef ValueType value_t;

         typedef node<KeyIterator,value_t> node_t;
         typedef node_t* node_ptr;

         prefix()
         : head_(0)
         {}

         template <typename key_iterator_t>
         inline void insert(const key_iterator_t begin,
                            const key_iterator_t end,
                            const value_t& v)
         {
            if (0 == std::distance(begin,end))
               return;

            key_iterator_t itr = begin;
            key_value_t key    = (*itr);
            node_ptr parent    = 0;
            node_ptr next_node = 0;
            node_ptr n = head_ = ((0 == head_) ? new node_t(*itr) : head_);

            while (end != itr)
            {
               key = (*itr);
               if (0 == (next_node = n->get_node(key)))
               {
                  n->add_node(next_node = new node_t(key));
               }
               parent = n;
               n = next_node;
               ++itr;
            }

            parent->assign_value(v);
         }

         template <typename key_iterator_t>
         inline bool find(const key_iterator_t begin,
                          const key_iterator_t end,
                          value_t& v) const
         {
            if ((0 == head_) || (0 == std::distance(begin,end)))
               return false;
            key_iterator_t itr = begin;
            node_ptr parent = head_;
            node_ptr n = head_;
            while (end != itr)
            {
               node_ptr next_node = n->get_node(*itr);
               if (0 == next_node)
                  return false;
               parent = n;
               n = next_node;
               ++itr;
            }
            if (!parent->value_holder())
               return false;
             v = parent->value();
             return true;
         }

         template <typename key_iterator_t>
         inline bool find_prefix(const key_iterator_t begin, const key_iterator_t end) const
         {
            if ((0 == head_) || (0 == std::distance(begin,end)))
               return false;

            key_iterator_t itr = begin;
            node_ptr n = head_;

            while (end != itr)
            {
               if (0 == (n = n->get_node(*itr)))
                  return false;
               ++itr;
            }

            return true;
         }

        ~prefix()
         {
            delete head_;
         }

      private:

         node_ptr head_;
      };

      template <typename Value>
      inline void insert(prefix<std::string::const_iterator,Value>& trie,
                         const std::string& key,
                         const Value& value = Value(0))
      {
         trie.insert(key.begin(),key.end(),value);
      }

      template <typename Value>
      inline void insert(prefix<std::string::iterator,Value>& trie,
                         const char* key,
                         const Value& value = Value(0))
      {
         trie.insert(std::string(key),value);
      }

      template <typename Value>
      inline bool find(prefix<std::string::const_iterator,Value>& trie,
                       const std::string& key,
                       Value& v)
      {
         return trie.find(key.begin(),key.end(),v);
      }

      template <typename Value>
      inline bool find(prefix<std::string::const_iterator,Value>& trie,
                       const char* key,
                       Value& v)
      {
         return trie.find_prefix(trie,std::string(key),v);
      }

      template <typename Value>
      inline bool find_prefix(prefix<std::string::const_iterator,Value>& trie,
                              const std::string& key)
      {
         return trie.find_prefix(key.begin(),key.end());
      }

      template <typename Value>
      inline bool find_prefix(prefix<std::string::const_iterator,Value>& trie,
                              const char* key)
      {
         return trie.find_prefix(trie,std::string(key));
      }

   } // namespace trie

   template <typename ValueType, typename KeyIterator = std::string::const_iterator>
   struct prefix_trie
   {
      typedef trie::prefix<KeyIterator,ValueType> type;
      typedef trie::prefix<KeyIterator,ValueType> std_string;
      typedef trie::prefix<char*,ValueType> char_ptr;
      typedef trie::prefix<const char*,ValueType> const_char_ptr;
      typedef trie::prefix<unsigned char*,ValueType> uchar_ptr;
      typedef trie::prefix<const unsigned char*,ValueType> const_uchar_ptr;
   };

   namespace bloom
   {

      static const std::size_t bits_per_char = 0x08;    // 8 bits in 1 char(unsigned)
      static const unsigned char bit_mask[bits_per_char] = {
                                                             0x01,  //00000001
                                                             0x02,  //00000010
                                                             0x04,  //00000100
                                                             0x08,  //00001000
                                                             0x10,  //00010000
                                                             0x20,  //00100000
                                                             0x40,  //01000000
                                                             0x80   //10000000
                                                           };

      class parameters
      {
      public:

         parameters()
         : minimum_size(1),
           maximum_size(std::numeric_limits<unsigned long long int>::max()),
           minimum_number_of_hashes(1),
           maximum_number_of_hashes(std::numeric_limits<unsigned int>::max()),
           projected_element_count(10000),
           false_positive_probability(1.0 / projected_element_count),
           random_seed(0xA5A5A5A55A5A5A5AULL)
         {}

         virtual ~parameters()
         {}

         inline bool operator!()
         {
            return (minimum_size > maximum_size)      ||
                   (minimum_number_of_hashes > maximum_number_of_hashes) ||
                   (minimum_number_of_hashes < 1)     ||
                   (0 == maximum_number_of_hashes)    ||
                   (0 == projected_element_count)     ||
                   (false_positive_probability < 0.0) ||
                   (std::numeric_limits<double>::infinity() == std::abs(false_positive_probability)) ||
                   (0 == random_seed)                 ||
                   (0xFFFFFFFFFFFFFFFFULL == random_seed);
         }

         //Allowed min/max size of the bloom filter in bits
         unsigned long long int minimum_size;
         unsigned long long int maximum_size;

         //Allowed min/max number of hash functions
         unsigned int minimum_number_of_hashes;
         unsigned int maximum_number_of_hashes;

         //The approximate number of elements to be inserted
         //into the bloom filter, should be within one order
         //of magnitude. The default is 10000.
         unsigned long long int projected_element_count;

         //The approximate false positive probability expected
         //from the bloom filter. The default is the reciprocal
         //of the projected_element_count.
         double false_positive_probability;

         unsigned long long int random_seed;

         inline bool operator()(strtk::binary::reader& reader)
         {
            return reader(minimum_size) &&
                   reader(maximum_size) &&
                   reader(minimum_number_of_hashes) &&
                   reader(maximum_number_of_hashes) &&
                   reader(projected_element_count) &&
                   reader(false_positive_probability) &&
                   reader(random_seed);
         }

         inline bool operator()(strtk::binary::writer& writer)
         {
            return writer(minimum_size) &&
                   writer(maximum_size) &&
                   writer(minimum_number_of_hashes) &&
                   writer(maximum_number_of_hashes) &&
                   writer(projected_element_count) &&
                   writer(false_positive_probability) &&
                   writer(random_seed);
         }

         struct optimal_parameters_t
         {
            optimal_parameters_t()
            : number_of_hashes(0),
              table_size(0)
            {}

            unsigned int number_of_hashes;
            unsigned long long int table_size;
         };

         optimal_parameters_t optimal_parameters;

         virtual bool compute_optimal_parameters()
         {
            /*
              Note:
              The following will attempt to find the number of hash functions
              and minimum amount of storage bits required to construct a bloom
              filter consistent with the user defined false positive probability
              and estimated element insertion count.
            */

            if (!(*this))
               return false;

            double min_m  = std::numeric_limits<double>::infinity();
            double min_k  = 0.0;
            double curr_m = 0.0;
            double k      = 1.0;

            while (k < 1000.0)
            {
               double numerator   = -k * projected_element_count;
               double denominator = std::log(1.0 - std::pow(false_positive_probability, 1.0 / k));
               curr_m = numerator / denominator;
               if (curr_m < min_m)
               {
                  min_m = curr_m;
                  min_k = k;
               }
               k += 1.0;
            }

            optimal_parameters_t& optp = optimal_parameters;

            optp.number_of_hashes = static_cast<unsigned int>(min_k);
            optp.table_size = static_cast<unsigned long long int>(min_m);
            optp.table_size += (((optp.table_size % bits_per_char) != 0) ? (bits_per_char - (optp.table_size % bits_per_char)) : 0);

            if (optp.number_of_hashes < minimum_number_of_hashes)
               optp.number_of_hashes = minimum_number_of_hashes;
            else if (optp.number_of_hashes > maximum_number_of_hashes)
               optp.number_of_hashes = maximum_number_of_hashes;

            if (optp.table_size < minimum_size)
               optp.table_size = minimum_size;
            else if (optp.table_size > maximum_size)
               optp.table_size = maximum_size;

            return true;
         }

      };

      class filter
      {
      protected:

         typedef unsigned int bloom_type;
         typedef unsigned char cell_type;

      public:

         filter()
         : bit_table_(0),
           salt_count_(0),
           table_size_(0),
           raw_table_size_(0),
           projected_element_count_(0),
           inserted_element_count_(0),
           random_seed_(0),
           desired_false_positive_probability_(0.0)
         {}

         filter(const parameters& p)
         : bit_table_(0),
           projected_element_count_(p.projected_element_count),
           inserted_element_count_(0),
           random_seed_((p.random_seed * 0xA5A5A5A5) + 1),
           desired_false_positive_probability_(p.false_positive_probability)
         {
            salt_count_ = p.optimal_parameters.number_of_hashes;
            table_size_ = p.optimal_parameters.table_size;
            generate_unique_salt();
            raw_table_size_ = table_size_ / bits_per_char;
            bit_table_ = new cell_type[static_cast<std::size_t>(raw_table_size_)];
            std::fill_n(bit_table_,raw_table_size_,0x00);
         }

         filter(const filter& filter)
         {
            this->operator=(filter);
         }

         inline bool operator == (const filter& f) const
         {
            if (this != &f)
            {
               return
                  (salt_count_                         == f.salt_count_)                         &&
                  (table_size_                         == f.table_size_)                         &&
                  (raw_table_size_                     == f.raw_table_size_)                     &&
                  (projected_element_count_            == f.projected_element_count_)            &&
                  (inserted_element_count_             == f.inserted_element_count_)             &&
                  (random_seed_                        == f.random_seed_)                        &&
                  (desired_false_positive_probability_ == f.desired_false_positive_probability_) &&
                  (salt_                               == f.salt_)                               &&
                  std::equal(f.bit_table_,f.bit_table_ + raw_table_size_,bit_table_);
            }
            else
               return true;
         }

         inline bool operator != (const filter& f) const
         {
            return !operator==(f);
         }

         inline filter& operator = (const filter& f)
         {
            if (this != &f)
            {
               salt_count_              = f.salt_count_;
               table_size_              = f.table_size_;
               raw_table_size_          = f.raw_table_size_;
               projected_element_count_ = f.projected_element_count_;
               inserted_element_count_  = f.inserted_element_count_;
               random_seed_             = f.random_seed_;
               desired_false_positive_probability_ = f.desired_false_positive_probability_;
               delete[] bit_table_;
               bit_table_               = new cell_type[static_cast<std::size_t>(raw_table_size_)];
               std::copy(f.bit_table_,f.bit_table_ + raw_table_size_,bit_table_);
               salt_                    = f.salt_;
            }
            return *this;
         }

         virtual ~filter()
         {
            delete[] bit_table_;
         }

         inline bool operator!() const
         {
            return (0 == table_size_);
         }

         inline void clear()
         {
            std::fill_n(bit_table_,raw_table_size_,0x00);
            inserted_element_count_ = 0;
         }

         inline void insert(const unsigned char* key_begin, const std::size_t& length)
         {
            std::size_t bit_index = 0;
            std::size_t bit = 0;
            for (std::size_t i = 0; i < salt_.size(); ++i)
            {
               compute_indices(hash_ap(key_begin,length,salt_[i]),bit_index,bit);
               bit_table_[bit_index / bits_per_char] |= bit_mask[bit];
            }
            ++inserted_element_count_;
         }

         template <typename T>
         inline void insert(const T& t)
         {
            // Note: T must be a C++ POD type.
            insert(reinterpret_cast<const unsigned char*>(&t),sizeof(T));
         }

         inline void insert(const std::string& key)
         {
            insert(reinterpret_cast<const unsigned char*>(key.data()),key.size());
         }

         inline void insert(const char* data, const std::size_t& length)
         {
            insert(reinterpret_cast<const unsigned char*>(data),length);
         }

         template <typename InputIterator>
         inline void insert(const InputIterator begin, const InputIterator end)
         {
            InputIterator itr = begin;
            while (end != itr)
            {
               insert(*(itr++));
            }
         }

         inline virtual bool contains(const unsigned char* key_begin, const std::size_t length) const
         {
            std::size_t bit_index = 0;
            std::size_t bit = 0;
            for (std::size_t i = 0; i < salt_.size(); ++i)
            {
               compute_indices(hash_ap(key_begin,length,salt_[i]),bit_index,bit);
               if ((bit_table_[bit_index / bits_per_char] & bit_mask[bit]) != bit_mask[bit])
               {
                  return false;
               }
            }
            return true;
         }

         template <typename T>
         inline bool contains(const T& t) const
         {
            return contains(reinterpret_cast<const unsigned char*>(&t),static_cast<std::size_t>(sizeof(T)));
         }

         inline bool contains(const std::string& key) const
         {
            return contains(reinterpret_cast<const unsigned char*>(key.data()),key.size());
         }

         inline bool contains(const char* data, const std::size_t& length) const
         {
            return contains(reinterpret_cast<const unsigned char*>(data),length);
         }

         template <typename InputIterator>
         inline InputIterator contains_all(const InputIterator begin, const InputIterator end) const
         {
            InputIterator itr = begin;
            while (end != itr)
            {
               if (!contains(*itr))
               {
                  return itr;
               }
               ++itr;
            }
            return end;
         }

         template <typename InputIterator>
         inline InputIterator contains_none(const InputIterator begin, const InputIterator end) const
         {
            InputIterator itr = begin;
            while (end != itr)
            {
               if (contains(*itr))
               {
                  return itr;
               }
               ++itr;
            }
            return end;
         }

         inline virtual unsigned long long int size() const
         {
            return table_size_;
         }

         inline std::size_t element_count() const
         {
            return inserted_element_count_;
         }

         inline double effective_fpp() const
         {
            /*
              Note:
              The effective false positive probability is calculated using the
              designated table size and hash function count in conjunction with
              the current number of inserted elements - not the user defined
              predicated/expected number of inserted elements.
            */
            return std::pow(1.0 - std::exp(-1.0 * salt_.size() * inserted_element_count_ / size()), 1.0 * salt_.size());
         }

         inline filter& operator &= (const filter& f)
         {
            /* intersection */
            if (
                (salt_count_  == f.salt_count_) &&
                (table_size_  == f.table_size_) &&
                (random_seed_ == f.random_seed_)
               )
            {
               for (std::size_t i = 0; i < raw_table_size_; ++i)
               {
                  bit_table_[i] &= f.bit_table_[i];
               }
            }
            return *this;
         }

         inline filter& operator |= (const filter& f)
         {
            /* union */
            if (
                (salt_count_  == f.salt_count_) &&
                (table_size_  == f.table_size_) &&
                (random_seed_ == f.random_seed_)
               )
            {
               for (std::size_t i = 0; i < raw_table_size_; ++i)
               {
                  bit_table_[i] |= f.bit_table_[i];
               }
            }
            return *this;
         }

         inline filter& operator ^= (const filter& f)
         {
            /* difference */
            if (
                (salt_count_  == f.salt_count_) &&
                (table_size_  == f.table_size_) &&
                (random_seed_ == f.random_seed_)
               )
            {
               for (std::size_t i = 0; i < raw_table_size_; ++i)
               {
                  bit_table_[i] ^= f.bit_table_[i];
               }
            }
            return *this;
         }

         inline const cell_type* table() const
         {
            return bit_table_;
         }

         inline bool write_to_file(const std::string& file_name) const
         {
            if (0 == table_size_)
               return false;
            const std::size_t buffer_size = sizeof(                        salt_count_) +
                                            sizeof(                        table_size_) +
                                            sizeof(                    raw_table_size_) +
                                            sizeof(           projected_element_count_) +
                                            sizeof(            inserted_element_count_) +
                                            sizeof(                       random_seed_) +
                                            sizeof(desired_false_positive_probability_) +
                                            salt_count_ * sizeof(           bloom_type) +
                                              static_cast<std::size_t>(raw_table_size_) *
                                                                      sizeof(cell_type) +
                                            64; // handle array sizes etc.
            std::ofstream ostream(file_name.c_str(),std::ios::binary);
            if (!ostream)
               return false;
            unsigned char* buffer = new unsigned char[buffer_size];
            strtk::binary::writer writer(buffer,buffer_size);
            writer.reset(true);
            bool result = writer(salt_count_)                         &&
                          writer(table_size_)                         &&
                          writer(raw_table_size_)                     &&
                          writer(projected_element_count_)            &&
                          writer(inserted_element_count_)             &&
                          writer(random_seed_)                        &&
                          writer(desired_false_positive_probability_) &&
                          writer(salt_)                               &&
                          writer(bit_table_,raw_table_size_);
            if (result)
            {
               writer(ostream);
            }
            ostream.close();
            delete[] buffer;
            return result;
         }

         inline bool read_from_file(const std::string& file_name)
         {
            std::ifstream istream(file_name.c_str(),std::ios::binary);
            if (!istream)
               return false;
            salt_count_                         = 0;
            table_size_                         = 0;
            raw_table_size_                     = 0;
            projected_element_count_            = 0;
            inserted_element_count_             = 0;
            random_seed_                        = 0;
            desired_false_positive_probability_ = 0.0;
            salt_.clear();
            if (0 != bit_table_)
               delete [] bit_table_;
            bit_table_= 0;
            const std::size_t buffer_size = strtk::fileio::file_size(file_name);
            unsigned char* buffer = new unsigned char[buffer_size];
            strtk::binary::reader reader(buffer,buffer_size);
            reader.reset(true);
            reader(istream,buffer_size);
            istream.close();
            bool result = reader(salt_count_)                         &&
                          reader(table_size_)                         &&
                          reader(raw_table_size_)                     &&
                          reader(projected_element_count_)            &&
                          reader(inserted_element_count_)             &&
                          reader(random_seed_)                        &&
                          reader(desired_false_positive_probability_) &&
                          reader(salt_)                               &&
                          reader(bit_table_,raw_table_size_);
            delete[] buffer;
            return result;
         }

         inline std::size_t hash_count()
         {
            return salt_.size();
         }

      protected:

         inline virtual void compute_indices(const bloom_type& hash, std::size_t& bit_index, std::size_t& bit) const
         {
            bit_index = static_cast<std::size_t>(hash % table_size_);
            bit = bit_index % bits_per_char;
         }

         void generate_unique_salt()
         {
            /*
              Note:
              A distinct hash function need not be implementation-wise
              distinct. In the current implementation "seeding" a common
              hash function with different values seems to be adequate.
            */
            const unsigned int predef_salt_count = 128;
            static const bloom_type predef_salt[predef_salt_count] =
                                       {
                                          0xAAAAAAAA, 0x55555555, 0x33333333, 0xCCCCCCCC,
                                          0x66666666, 0x99999999, 0xB5B5B5B5, 0x4B4B4B4B,
                                          0xAA55AA55, 0x55335533, 0x33CC33CC, 0xCC66CC66,
                                          0x66996699, 0x99B599B5, 0xB54BB54B, 0x4BAA4BAA,
                                          0xAA33AA33, 0x55CC55CC, 0x33663366, 0xCC99CC99,
                                          0x66B566B5, 0x994B994B, 0xB5AAB5AA, 0xAAAAAA33,
                                          0x555555CC, 0x33333366, 0xCCCCCC99, 0x666666B5,
                                          0x9999994B, 0xB5B5B5AA, 0xFFFFFFFF, 0xFFFF0000,
                                          0xB823D5EB, 0xC1191CDF, 0xF623AEB3, 0xDB58499F,
                                          0xC8D42E70, 0xB173F616, 0xA91A5967, 0xDA427D63,
                                          0xB1E8A2EA, 0xF6C0D155, 0x4909FEA3, 0xA68CC6A7,
                                          0xC395E782, 0xA26057EB, 0x0CD5DA28, 0x467C5492,
                                          0xF15E6982, 0x61C6FAD3, 0x9615E352, 0x6E9E355A,
                                          0x689B563E, 0x0C9831A8, 0x6753C18B, 0xA622689B,
                                          0x8CA63C47, 0x42CC2884, 0x8E89919B, 0x6EDBD7D3,
                                          0x15B6796C, 0x1D6FDFE4, 0x63FF9092, 0xE7401432,
                                          0xEFFE9412, 0xAEAEDF79, 0x9F245A31, 0x83C136FC,
                                          0xC3DA4A8C, 0xA5112C8C, 0x5271F491, 0x9A948DAB,
                                          0xCEE59A8D, 0xB5F525AB, 0x59D13217, 0x24E7C331,
                                          0x697C2103, 0x84B0A460, 0x86156DA9, 0xAEF2AC68,
                                          0x23243DA5, 0x3F649643, 0x5FA495A8, 0x67710DF8,
                                          0x9A6C499E, 0xDCFB0227, 0x46A43433, 0x1832B07A,
                                          0xC46AFF3C, 0xB9C8FFF0, 0xC9500467, 0x34431BDF,
                                          0xB652432B, 0xE367F12B, 0x427F4C1B, 0x224C006E,
                                          0x2E7E5A89, 0x96F99AA5, 0x0BEB452A, 0x2FD87C39,
                                          0x74B2E1FB, 0x222EFD24, 0xF357F60C, 0x440FCB1E,
                                          0x8BBE030F, 0x6704DC29, 0x1144D12F, 0x948B1355,
                                          0x6D8FD7E9, 0x1C11A014, 0xADD1592F, 0xFB3C712E,
                                          0xFC77642F, 0xF9C4CE8C, 0x31312FB9, 0x08B0DD79,
                                          0x318FA6E7, 0xC040D23D, 0xC0589AA7, 0x0CA5C075,
                                          0xF874B172, 0x0CF914D5, 0x784D3280, 0x4E8CFEBC,
                                          0xC569F575, 0xCDB2A091, 0x2CC016B4, 0x5C5F4421
                                       };

            if (salt_count_ <= predef_salt_count)
            {
               std::copy(predef_salt,
                         predef_salt + salt_count_,
                         std::back_inserter(salt_));
                for (unsigned int i = 0; i < salt_.size(); ++i)
                {
                  /*
                    Note:
                    This is done to integrate the user defined random seed,
                    so as to allow for the generation of unique bloom filter
                    instances.
                  */
                  salt_[i] = salt_[i] * salt_[(i + 3) % salt_.size()] + static_cast<bloom_type>(random_seed_);
                }
            }
            else
            {
               std::copy(predef_salt,predef_salt + predef_salt_count,std::back_inserter(salt_));
               srand(static_cast<unsigned int>(random_seed_));
               while (salt_.size() < salt_count_)
               {
                  bloom_type current_salt = static_cast<bloom_type>(rand()) * static_cast<bloom_type>(rand());
                  if (0 == current_salt) continue;
                  if (salt_.end() == std::find(salt_.begin(), salt_.end(), current_salt))
                  {
                     salt_.push_back(current_salt);
                  }
               }
            }
         }

         inline bloom_type hash_ap(const unsigned char* begin, std::size_t remaining_length, bloom_type hash) const
         {
            const unsigned char* itr = begin;
            unsigned int loop = 0;
            while (remaining_length >= 8)
            {
               const unsigned int& i1 = *(reinterpret_cast<const unsigned int*>(itr)); itr += sizeof(unsigned int);
               const unsigned int& i2 = *(reinterpret_cast<const unsigned int*>(itr)); itr += sizeof(unsigned int);
               hash ^= (hash <<  7) ^  i1 * (hash >> 3) ^
                    (~((hash << 11) + (i2 ^ (hash >> 5))));
               remaining_length -= 8;
            }
            if (remaining_length)
            {
               if (remaining_length >= 4)
               {
                  const unsigned int& i = *(reinterpret_cast<const unsigned int*>(itr));
                  if (loop & 0x01)
                     hash ^=    (hash <<  7) ^  i * (hash >> 3);
                  else
                     hash ^= (~((hash << 11) + (i ^ (hash >> 5))));
                  ++loop;
                  remaining_length -= 4;
                  itr += sizeof(unsigned int);
               }
               if (remaining_length >= 2)
               {
                  const unsigned short& i = *(reinterpret_cast<const unsigned short*>(itr));
                  if (loop & 0x01)
                     hash ^=    (hash <<  7) ^  i * (hash >> 3);
                  else
                     hash ^= (~((hash << 11) + (i ^ (hash >> 5))));
                  ++loop;
                  remaining_length -= 2;
                  itr += sizeof(unsigned short);
               }
               if (remaining_length)
               {
                  hash += ((*itr) ^ (hash * 0xA5A5A5A5)) + loop;
               }
            }
            return hash;
         }

         std::vector<bloom_type> salt_;
         unsigned char*          bit_table_;
         unsigned int            salt_count_;
         unsigned long long int  table_size_;
         unsigned long long int  raw_table_size_;
         unsigned long long int  projected_element_count_;
         unsigned int            inserted_element_count_;
         unsigned long long int  random_seed_;
         double                  desired_false_positive_probability_;
      };

      inline filter operator & (const filter& a, const filter& b)
      {
         filter result = a;
         result &= b;
         return result;
      }

      inline filter operator | (const filter& a, const filter& b)
      {
         filter result = a;
         result |= b;
         return result;
      }

      inline filter operator ^ (const filter& a, const filter& b)
      {
         filter result = a;
         result ^= b;
         return result;
      }

      class compressible_filter : public filter
      {
      public:

         compressible_filter(const parameters& p)
         : filter(p)
         {
            size_list.push_back(table_size_);
         }

         inline virtual unsigned long long int size() const
         {
            return size_list.back();
         }

         inline bool compress(const double& percentage)
         {
            if ((0.0 >= percentage) || (percentage >= 100.0))
            {
               return false;
            }

            unsigned long long int original_table_size = size_list.back();
            unsigned long long int new_table_size = static_cast<unsigned long long int>((size_list.back() * (1.0 - (percentage / 100.0))));
            new_table_size -= (((new_table_size % bits_per_char) != 0) ? (new_table_size % bits_per_char) : 0);

            if ((bits_per_char > new_table_size) || (new_table_size >= original_table_size))
            {
               return false;
            }

            desired_false_positive_probability_ = effective_fpp();
            cell_type* tmp = new cell_type[static_cast<std::size_t>(new_table_size / bits_per_char)];
            std::copy(bit_table_, bit_table_ + (new_table_size / bits_per_char), tmp);
            cell_type* itr = bit_table_ + (new_table_size / bits_per_char);
            cell_type* end = bit_table_ + (original_table_size / bits_per_char);
            cell_type* itr_tmp = tmp;

            while (end != itr)
            {
               *(itr_tmp++) |= (*itr++);
            }

            delete[] bit_table_;
            bit_table_ = tmp;
            size_list.push_back(new_table_size);

            return true;
         }

      private:

         inline virtual void compute_indices(const bloom_type& hash, std::size_t& bit_index, std::size_t& bit) const
         {
            bit_index = hash;
            for (std::size_t i = 0; i < size_list.size(); ++i)
            {
               bit_index %= size_list[i];
            }
            bit = bit_index % bits_per_char;
         }

         std::vector<unsigned long long int> size_list;
      };

   }

   namespace details
   {

      inline void compute_pod_hash(const char data[], unsigned int& hash)
      {
         hash ^=  ((hash <<  7) ^  data[0] * (hash >> 3));
         hash ^= ~((hash << 11) + (data[1] ^ (hash >> 5)));
      }

      inline void compute_pod_hash(const unsigned char data[], unsigned int& hash)
      {
         hash ^=  ((hash <<  7) ^  data[0] * (hash >> 3));
         hash ^= ~((hash << 11) + (data[1] ^ (hash >> 5)));
      }

      inline void compute_pod_hash(const int& data, unsigned int& hash)
      {
         const unsigned char* itr = reinterpret_cast<const unsigned char*>(&data);
         hash ^=  ((hash <<  7) ^  itr[0] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[1] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[2] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[3] ^ (hash >> 5)));
      }

      inline void compute_pod_hash(const unsigned int& data, unsigned int& hash)
      {
         compute_pod_hash(static_cast<int>(data),hash);
      }

      inline void compute_pod_hash(const unsigned long long int& data, unsigned int& hash)
      {
         const unsigned char* itr = reinterpret_cast<const unsigned char*>(&data);
         hash ^=  ((hash <<  7) ^  itr[0] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[1] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[2] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[3] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[4] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[5] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[6] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[7] ^ (hash >> 5)));
      }

      inline void compute_pod_hash(const double& data, unsigned int& hash)
      {
         const unsigned char* itr = reinterpret_cast<const unsigned char*>(&data);
         hash ^=  ((hash <<  7) ^  itr[0] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[1] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[2] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[3] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[4] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[5] ^ (hash >> 5)));
         hash ^=  ((hash <<  7) ^  itr[6] * (hash >> 3));
         hash ^= ~((hash << 11) + (itr[7] ^ (hash >> 5)));
      }

      template <std::size_t block_size, typename Iterator>
      inline void compute_block(Iterator itr, std::size_t& length, unsigned int& hash)
      {
         while (length >= block_size)
         {
            for (std::size_t i = 0; i < block_size; ++i, ++itr)
            {
               compute_pod_hash((*itr),hash);
            }
            length -= block_size;
         }
      }

      template <std::size_t block_size>
      inline void compute_block(unsigned char* itr, std::size_t& length, unsigned int& hash)
      {
         unsigned int local_hash = hash;
         while (length >= block_size)
         {
            for (std::size_t i = 0; i < block_size; ++i, ++itr)
            {
               compute_pod_hash((*itr),local_hash);
            }
            length -= block_size;
         }
         hash = local_hash;
      }

      template <std::size_t block_size>
      inline void compute_block(char* itr, std::size_t& length, unsigned int& hash)
      {
         compute_block<block_size>(reinterpret_cast<unsigned char*>(itr),length,hash);
      }

      static const unsigned int hash_seed = 0xAAAAAAAA;

      template <typename Iterator>
      inline void hash(const Iterator itr, std::size_t length, unsigned int& hash_value)
      {
         if (length >= 64) compute_block<64>(itr,length,hash_value);
         if (length >= 32) compute_block<32>(itr,length,hash_value);
         if (length >= 16) compute_block<16>(itr,length,hash_value);
         if (length >=  8) compute_block< 8>(itr,length,hash_value);
         if (length >=  4) compute_block< 4>(itr,length,hash_value);
         if (length >=  2) compute_block< 2>(itr,length,hash_value);
         if (length ==  0) compute_block< 1>(itr,length,hash_value);
      }

   } // namespace details

   template <typename Iterator>
   inline unsigned int hash(const Iterator itr,
                            std::size_t length,
                            unsigned int seed = details::hash_seed)
   {
      unsigned int hash_value = seed;
      details::hash(itr,length,hash_value);
      return hash_value;
   }

   inline unsigned int hash(const std::string& s, unsigned int seed = details::hash_seed)
   {
      unsigned int hash_value = seed;
      return hash(s.begin(),s.size(),hash_value);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline unsigned int hash(const Sequence<T,Allocator>& sequence, unsigned int seed = details::hash_seed)
   {
      unsigned int hash_value = seed;
      return hash(sequence.begin(),sequence.size(),hash_value);
   }

   namespace util
   {
      template <typename T>
      class scoped_restore
      {
      public:

         scoped_restore(T& t, const bool restore = true)
         : restore_(restore),
           reference_(t),
           copy_(t)
         {}

        ~scoped_restore()
         {
            if (restore_)
            {
               reference_ = copy_;
            }
         }

         inline bool& restore()
         {
            return restore_;
         }

      private:

         scoped_restore(const scoped_restore&);
         scoped_restore& operator=(const scoped_restore&);

         bool restore_;
         T& reference_;
         T copy_;
      };

      template <typename T>
      class attribute
      {
      public:

         attribute()
         : initialised_(false)
         {}

         attribute(const T& t)
         {
            assign(t);
            prev_t_ = t;
         }

         inline attribute& operator=(const T& t)
         {
            prev_t_ = t_;
            assign(t);
            return *this;
         }

         inline bool operator==(const T& t)
         {
            return initialised_ && (t_ == t);
         }

         template <typename TConvertibleType>
         inline bool operator!=(const TConvertibleType& t)
         {
            return !(operator==(t));
         }

         inline T& operator()()
         {
            return t_;
         }

         inline const T& operator()() const
         {
            return t_;
         }

         inline operator T() const
         {
            return t_;
         }

         inline operator T()
         {
            return t_;
         }

         inline bool initialised() const
         {
            return initialised_;
         }

         inline bool& initialised()
         {
            return initialised_;
         }

         inline bool changed() const
         {
            return (initialised_ && (t_ != prev_t_));
         }

         inline const T& value() const
         {
            return t_;
         }

         inline T& value()
         {
            return t_;
         }

         inline const T& previous() const
         {
            return prev_t_;
         }

         inline T& previous()
         {
            return prev_t_;
         }

      private:

         inline void assign(const T& t)
         {
            t_ = t;
            initialised_ = true;
         }

         T t_;
         T prev_t_;
         bool initialised_;
      };

      inline bool operator==(const char* s, const attribute<std::string>& attrib)
      {
         return attrib.value() == s;
      }

      inline bool operator!=(const char* s, const attribute<std::string>& attrib)
      {
         return !(s == attrib.value());
      }

      template <typename T>
      static inline std::ostream& operator<<(std::ostream& os, const attribute<T>& attrib)
      {
         return (os << attrib.value());
      }

      class semantic_action_impl
      {
      private:

         class function_holder_base
         {
         public:

            typedef const unsigned char* itr_type;

            virtual ~function_holder_base(){}

            virtual bool operator()(itr_type begin, itr_type end) const = 0;

            inline bool operator()(const char* begin, const char* end) const
            {
               return operator()(reinterpret_cast<itr_type>(begin),
                                 reinterpret_cast<itr_type>(end));
            }

            template <typename Iterator>
            inline bool operator()(const std::pair<Iterator,Iterator>& p) const
            {
               return operator()(p.first,p.second);
            }
         };

         template <typename Function>
         class function_holder : public function_holder_base
         {
         public:

            explicit function_holder(Function& f)
            : function_(&f)
            {}

            inline virtual bool operator()(itr_type begin, itr_type end) const
            {
               return (*function_)(begin,end);
            }

         private:

            Function* function_;
         };

      public:

         semantic_action_impl()
         : function_holder_(0)
         {
            std::fill_n(function_holder_buffer_,sizeof(function_holder_buffer_),0x00);
         }

         template <typename Function>
         inline explicit semantic_action_impl(const Function& f)
         {
            std::fill_n(function_holder_buffer_,sizeof(function_holder_buffer_),0x00);
            assign(f);
         }

         inline bool operator!() const
         {
            return (0 == function_holder_);
         }

         inline bool operator==(const semantic_action_impl& sa) const
         {
            return (0 !=    function_holder_) &&
                   (0 != sa.function_holder_) &&
                   (function_holder_ == sa.function_holder_);
         }

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end) const
         {
            if (0 != function_holder_)
               return (*function_holder_).operator()(begin,end);
            else
               return false;
         }

         template <typename InputIterator>
         inline bool operator()(const std::pair<InputIterator,InputIterator>& r) const
         {
            return operator()(r.first,r.second);
         }

         inline bool operator()(const std::string& s) const
         {
            return operator()(s.data(),s.data() + s.size());
         }

         template <typename Function>
         inline void assign(Function& f)
         {
            static const std::size_t type_size = sizeof(function_holder<Function>(f));
            function_holder_ = construct<Function,type_size <= function_holder_buffer_size>::type(f,function_holder_buffer_);
         }

         inline semantic_action_impl& ref()
         {
            return (*this);
         }

      private:

         typedef function_holder_base* function_holder_ptr;

         inline semantic_action_impl& operator=(const semantic_action_impl&);

         template <typename Function, bool b>
         struct construct
         {
            inline static function_holder_ptr type(Function&, unsigned char*)
            {
               return reinterpret_cast<function_holder_ptr>(0);
            }
         };

         template <typename Function>
         struct construct<Function,true>
         {
            inline static function_holder_ptr type(Function& f, unsigned char* buffer)
            {
               return new(buffer)function_holder<Function>(f);
            }
         };

         function_holder_ptr function_holder_;
         enum { function_holder_buffer_size = 64 };
         unsigned char function_holder_buffer_[function_holder_buffer_size];
      };

      template <typename Function>
      inline semantic_action_impl semantic_action(Function& f)
      {
         return semantic_action_impl(f);
      }

   } // namespace util

   namespace details
   {
      #define strtk_register_attribute_type_tag(T)                                                                       \
      template<> struct supported_conversion_to_type< strtk::util::attribute<T> >{ typedef attribute_type_tag type; };   \
      template<> struct supported_conversion_from_type< strtk::util::attribute<T> > { typedef attribute_type_tag type; };\

      strtk_register_attribute_type_tag(unsigned short)
      strtk_register_attribute_type_tag(unsigned int)
      strtk_register_attribute_type_tag(unsigned long)
      strtk_register_attribute_type_tag(unsigned long long int)
      strtk_register_attribute_type_tag(short)
      strtk_register_attribute_type_tag(int)
      strtk_register_attribute_type_tag(long)
      strtk_register_attribute_type_tag(long long)
      strtk_register_attribute_type_tag(float)
      strtk_register_attribute_type_tag(double)
      strtk_register_attribute_type_tag(long double)
      strtk_register_attribute_type_tag(unsigned char)
      strtk_register_attribute_type_tag(signed char)
      strtk_register_attribute_type_tag(char)
      strtk_register_attribute_type_tag(std::string)

      template <typename Iterator, typename T>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, strtk::util::attribute<T>& result, attribute_type_tag)
      {
         if (strtk::string_to_type_converter(itr,end,result.value()))
         {
            result.initialised() = true;
            return true;
         }
         else
            return false;
      }

      template <typename T>
      inline bool type_to_string_converter_impl(const strtk::util::attribute<T>& attrib, std::string& result, attribute_type_tag)
      {
         if (!attrib.initialised())
            return false;
         return strtk::type_to_string(attrib.value(),result);
      }

      #undef strtk_register_attribute_type_tag

      template<> struct supported_conversion_to_type  < strtk::util::semantic_action_impl > { typedef semantic_action_type_tag type; };
      template<> struct supported_conversion_from_type< strtk::util::semantic_action_impl > { typedef semantic_action_type_tag type; };

      template <typename Iterator>
      inline bool string_to_type_converter_impl(Iterator& itr, const Iterator end, strtk::util::semantic_action_impl& result, semantic_action_type_tag)
      {
         return result(itr,end);
      }

      inline bool type_to_string_converter_impl(const strtk::util::semantic_action_impl&, std::string& result, semantic_action_type_tag)
      {
         static std::string result_str = "semantic_action";
         result = result_str;
         return true;
      }

   } // namespace details

   namespace util
   {
      class value
      {
      private:

         class type_holder_base
         {
         public:

            typedef const unsigned char* itr_type;

            virtual ~type_holder_base(){}

            virtual bool operator()(itr_type begin, itr_type end) const = 0;

            virtual bool to_string(std::string& s) const = 0;

            inline bool operator()(const char* begin, const char* end) const
            {
               return operator()(reinterpret_cast<itr_type>(begin),
                                 reinterpret_cast<itr_type>(end));
            }

            template <typename Iterator>
            inline bool operator()(const std::pair<Iterator,Iterator>& p) const
            {
               return operator()(p.first,p.second);
            }
         };

         template <typename T>
         class type_holder : public type_holder_base
         {
         public:

            typedef T* type_ptr;

            explicit type_holder(T& t)
            : value_ptr_(&t)
            {}

            inline virtual bool operator()(itr_type begin, itr_type end) const
            {
               return strtk::string_to_type_converter(begin,end,(*value_ptr_));
            }

            inline virtual bool to_string(std::string& s) const
            {
               return strtk::type_to_string((*value_ptr_),s);
            }

            inline operator T() const
            {
               return (*value_ptr_);
            }

         private:

            type_ptr value_ptr_;
         };

      public:

         value()
         : type_holder_(0)
         {
            std::fill_n(type_holder_buffer_,sizeof(type_holder_buffer_),0x00);
         }

         template <typename T>
         inline explicit value(T& t)
         {
            std::fill_n(type_holder_buffer_,sizeof(type_holder_buffer_),0x00);
            assign(t);
         }

         inline bool operator!() const
         {
            return (0 == type_holder_);
         }

         inline bool operator==(const value& v) const
         {
            return (0 !=   type_holder_) &&
                   (0 != v.type_holder_) &&
                   (type_holder_ == v.type_holder_);
         }

         inline value& operator=(const value& v)
         {
            if (&v != this)
            {
               if (0 != v.type_holder_)
               {
                  std::copy(v.type_holder_buffer_,
                            v.type_holder_buffer_ + type_holder_buffer_size,
                            type_holder_buffer_);
                  type_holder_ = reinterpret_cast<type_holder_base*>(type_holder_buffer_);
               }
            }
            return *this;
         }

         template <typename InputIterator>
         inline bool operator()(InputIterator begin, InputIterator end) const
         {
            if (0 != type_holder_)
               return (*type_holder_).operator()(begin,end);
            else
               return false;
         }

         template <typename InputIterator>
         inline bool operator()(const std::pair<InputIterator,InputIterator>& r) const
         {
            return operator()(r.first,r.second);
         }

         inline bool operator()(const std::string& s) const
         {
            return operator()(s.data(),s.data() + s.size());
         }

         template <typename T>
         inline void assign(T& t)
         {
            static const std::size_t type_size = sizeof(type_holder<T>(t));
            type_holder_ = construct<T,type_size <= type_holder_buffer_size>::type(t,type_holder_buffer_);
         }

         inline bool to_string(std::string& s) const
         {
            if (0 != type_holder_)
               return (*type_holder_).to_string(s);
            else
               return false;
         }

         template <typename T>
         inline operator T() const
         {
            if (0 != type_holder_)
               return (*type_holder_);
            else
               return T();
         }

      private:

         typedef type_holder_base* type_holder_ptr;

         template <typename T, bool b>
         struct construct
         {
            inline static type_holder_ptr type(T&, unsigned char*)
            {
               return reinterpret_cast<type_holder_ptr>(0);
            }
         };

         template <typename T>
         struct construct<T,true>
         {
            inline static type_holder_ptr type(T& t, unsigned char* buffer)
            {
               return new(buffer)type_holder<T>(t);
            }
         };

         type_holder_ptr type_holder_;
         enum { type_holder_buffer_size = 2 * sizeof(type_holder<unsigned long long int>) };
         unsigned char type_holder_buffer_[type_holder_buffer_size];
      };

      template <typename Key,
                typename T,
                typename Comparator,
                typename MapAllocator,
                typename OutputIterator>
      inline void make_key_list(const std::map<Key,T,Comparator,MapAllocator>& map,
                                OutputIterator out)
      {
         if (map.empty()) return;
         typedef typename std::map<Key,T,Comparator,MapAllocator> map_type;
         typename map_type::const_iterator itr = map.begin();
         typename map_type::const_iterator end = map.end();
         while (end != itr)
         {
            *out++ = (itr++)->first;
         }
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename MapAllocator,
                typename SetAllocator>
      inline void make_key_list(const std::map<Key,T,Comparator,MapAllocator>& map,
                                std::set<Key,Comparator,SetAllocator>& set)
      {
         make_key_list(map,std::inserter(set,set.begin()));
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename MapAllocator,
                typename SetAllocator>
      inline void make_key_list(const std::map<Key,T,Comparator,MapAllocator>& map,
                                std::multiset<Key,Comparator,SetAllocator>& multiset)
      {
         make_key_list(map,std::inserter(multiset,multiset.begin()));
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename MapAllocator,
                typename SequenceAllocator,
                template <typename,typename> class Sequence>
      inline void make_key_list(const std::map<Key,T,Comparator,MapAllocator>& map,
                                Sequence<Key,SequenceAllocator>& sequence)
      {
         make_key_list(map,std::back_inserter(sequence));
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename MapAllocator,
                typename OutputIterator>
      inline void make_value_list(const std::multimap<Key,T,Comparator,MapAllocator>& map,
                                  const Key& key,
                                  OutputIterator out)
      {
         if (map.empty()) return;
         typedef typename std::multimap<Key,T,Comparator,MapAllocator> map_type;
         typename map_type::const_iterator itr = map.find(key);
         typename map_type::const_iterator end = map.end();
         while ((end != itr) && (key == itr->first))
         {
            *out++ = (itr++)->second;
         }
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename MapAllocator,
                typename SequenceAllocator,
                template <typename,typename> class Sequence>
      inline void make_value_list(const std::multimap<Key,T,Comparator,MapAllocator>& map,
                                  const Key& key,
                                  Sequence<T,SequenceAllocator>& sequence)
      {
         make_value_list(map,key,std::back_inserter(sequence));
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void delete_all(Sequence<T*,Allocator>& sequence)
      {
         typename Sequence<T*,Allocator>::iterator itr = sequence.begin();
         typename Sequence<T*,Allocator>::iterator end = sequence.end();
         while (end != itr)
         {
            delete (*itr);
            ++itr;
         }
         sequence.clear();
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_all(std::map<Key,T*,Comparator,Allocator>& cont)
      {
         typename std::map<Key,T*,Comparator,Allocator>::iterator itr = cont.begin();
         typename std::map<Key,T*,Comparator,Allocator>::iterator end = cont.end();
         while (end != itr)
         {
            delete (*itr).second;
            ++itr;
         }
         cont.clear();
      }

      template <typename Key,
                typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_all(std::multimap<Key,T*,Comparator,Allocator>& cont)
      {
         typename std::multimap<Key,T*,Comparator,Allocator>::iterator itr = cont.begin();
         typename std::multimap<Key,T*,Comparator,Allocator>::iterator end = cont.end();
         while (end != itr)
         {
            delete (*itr).second;
            ++itr;
         }
         cont.clear();
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_all(std::set<T*,Comparator,Allocator>& cont)
      {
         typename std::set<T*,Comparator,Allocator>::iterator itr = cont.begin();
         typename std::set<T*,Comparator,Allocator>::iterator end = cont.end();
         while (end != itr)
         {
            delete (*itr);
            ++itr;
         }
         cont.clear();
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_all(std::multiset<T*,Comparator,Allocator>& cont)
      {
         typename std::multiset<T*,Comparator,Allocator>::iterator itr = cont.begin();
         typename std::multiset<T*,Comparator,Allocator>::iterator end = cont.end();
         while (end != itr)
         {
            delete (*itr);
            ++itr;
         }
         cont.clear();
      }

      template <typename Predicate,
                typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void delete_if(const Predicate& p,
                            Sequence<T*,Allocator>& sequence)
      {
         typename Sequence<T*,Allocator>::iterator itr = sequence.begin();
         while (sequence.end() != itr)
         {
            if (p(*itr))
            {
               delete (*itr);
               itr = sequence.erase(itr);
            }
            else
               ++itr;
         }
      }

      template <typename Predicate,
                typename Key,
                typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_if(const Predicate& p,
                            std::map<Key,T*,Comparator,Allocator>& cont)
      {
         typename std::map<Key,T*,Comparator,Allocator>::iterator itr = cont.begin();
         while (cont.end() != itr)
         {
            if (p(*itr))
            {
               delete (*itr).second;
               itr = cont.erase(itr);
            }
            else
               ++itr;
         }
      }

      template <typename Predicate,
                typename Key,
                typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_if(const Predicate& p,
                            std::multimap<Key,T*,Comparator,Allocator>& cont)
      {
         typename std::multimap<Key,T*,Comparator,Allocator>::iterator itr = cont.begin();
         while (cont.end() != itr)
         {
            if (p(*itr))
            {
               delete (*itr).second;
               itr = cont.erase(itr);
            }
            else
               ++itr;
         }
      }

      template <typename Predicate,
                typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_if(const Predicate& p,
                            std::set<T*,Comparator,Allocator>& cont)
      {
         typename std::set<T*,Comparator,Allocator>::iterator itr = cont.begin();
         while (cont.end() != itr)
         {
            if (p(*itr))
            {
               delete (*itr).second;
               itr = cont.erase(itr);
            }
            else
               ++itr;
         }
      }

      template <typename Predicate,
                typename T,
                typename Comparator,
                typename Allocator>
      inline void delete_if(const Predicate& p,
                            std::multiset<T*,Comparator,Allocator>& cont)
      {
         typename std::multiset<T*,Comparator,Allocator>::iterator itr = cont.begin();
         while (cont.end() != itr)
         {
            if (p(*itr))
            {
               delete (*itr).second;
               itr = cont.erase(itr);
            }
            else
               ++itr;
         }
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T&  v2, const T&  v3, const T& v4,
                            const T& v5, const T&  v6, const T&  v7, const T& v8,
                            const T& v9, const T& v10, const T& v11)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);  sequence.push_back(v6);
         sequence.push_back(v7);  sequence.push_back(v8);
         sequence.push_back(v9); sequence.push_back(v10);
         sequence.push_back(v11);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8,
                            const T& v9, const T& v10)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);  sequence.push_back(v6);
         sequence.push_back(v7);  sequence.push_back(v8);
         sequence.push_back(v9); sequence.push_back(v10);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8,
                            const T& v9)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);  sequence.push_back(v6);
         sequence.push_back(v7);  sequence.push_back(v8);
         sequence.push_back(v9);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);  sequence.push_back(v6);
         sequence.push_back(v7);  sequence.push_back(v8);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);  sequence.push_back(v6);
         sequence.push_back(v7);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);  sequence.push_back(v6);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
         sequence.push_back(v5);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3, const T& v4)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);  sequence.push_back(v4);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2, const T& v3)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
         sequence.push_back(v3);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1, const T& v2)
      {
         sequence.push_back(v1);  sequence.push_back(v2);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void push_back(Sequence<T,Allocator>& sequence,
                            const T& v1)
      {
         sequence.push_back(v1);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8,
                            const T& v9, const T& v10)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);  set.insert(v8);
         set.insert(v9); set.insert(v10);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8,
                            const T& v9)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);  set.insert(v8);
         set.insert(v9);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);  set.insert(v8);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2)
      {
         set.insert(v1);  set.insert(v2);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::set<T,Comparator,Allocator>& set,
                            const T& v1)
      {
         set.insert(v1);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8,
                            const T& v9, const T& v10)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);  set.insert(v8);
         set.insert(v9); set.insert(v10);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8,
                            const T& v9)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);  set.insert(v8);
         set.insert(v9);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7, const T& v8)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);  set.insert(v8);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6, const T& v7)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
         set.insert(v7);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5, const T& v6)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);  set.insert(v6);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4,
                            const T& v5)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
         set.insert(v5);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3, const T& v4)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);  set.insert(v4);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2, const T& v3)
      {
         set.insert(v1);  set.insert(v2);
         set.insert(v3);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1, const T& v2)
      {
         set.insert(v1);  set.insert(v2);
      }

      template <typename T, typename Comparator, typename Allocator>
      inline void push_back(std::multiset<T,Comparator,Allocator>& set,
                            const T& v1)
      {
         set.insert(v1);
      }

      template <typename T,
                typename Allocator,
                template <typename,typename> class Sequence>
      inline void clear(Sequence<T,Allocator>& sequence)
      {
         sequence.clear();
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline void clear(std::set<T,Comparator,Allocator>& set)
      {
         std::set<T> null_set;
         std::swap(set,null_set);
      }

      template <typename T,
                typename Comparator,
                typename Allocator>
      inline void clear(std::multiset<T,Comparator,Allocator>& multiset)
      {
         std::multiset<T> null_set;
         std::swap(multiset,null_set);
      }

      template <typename T, typename Container>
      inline void clear(std::queue<T,Container>& queue)
      {
         std::queue<T> null_que;
         std::swap(queue,null_que);
      }

      template <typename T, typename Container>
      inline void clear(std::stack<T,Container>& stack)
      {
         std::stack<T> null_stack;
         std::swap(stack,null_stack);
      }

      template <typename T,
                typename Container,
                typename Comparator>
      inline void clear(std::priority_queue<T,Container,Comparator>& priority_queue)
      {
         std::priority_queue<T> null_pqueue;
         std::swap(priority_queue,null_pqueue);
      }

   } // namespace util

   namespace details
   {
      template <std::size_t N>
      struct column_list_impl
      {
         enum { size = N };
         std::size_t index_list[N];
      };

      template <typename Cli, std::size_t N>
      class column_selector_base
      {
      public:

         typedef column_selector_base<Cli,N> csb_t;
         typedef column_list_impl<N> column_list_t;

         column_selector_base(const column_list_t& column_list)
         : column_list_(column_list),
           current_index_(0),
           target_index_(column_list_.index_list[0]),
           col_list_index_(0),
           error_count_(0)
         {}

         inline csb_t& operator*()
         {
            return (*this);
         }

         inline csb_t& operator++()
         {
            return (*this);
         }

         inline csb_t operator++(int)
         {
            return (*this);
         }

         template <typename Iterator>
         inline csb_t& operator=(const std::pair<Iterator,Iterator>& r)
         {
            process(r);
            return (*this);
         }

         void reset()
         {
            current_index_  = 0;
            col_list_index_ = 0;
            target_index_   = column_list_.index_list[0];
            error_count_    = 0;
         }

      protected:

         class colsel_value_list
         {
         public:

            typedef std::pair<strtk::util::value,bool> value_t;

            colsel_value_list()
            : current_index(0)
            {
               static const value_t null_value(strtk::util::value(),false);
               std::fill_n(value_list,N,null_value);
            }

            template <typename T>
            inline void register_value(T& t)
            {
               if (current_index < N)
               {
                  value_list[current_index].first.assign(t);
                  value_list[current_index].second = false;
                  ++current_index;
               }
            }

            std::size_t current_index;
            value_t value_list[N];
         };

         template <typename Iterator>
         inline void process(const std::pair<Iterator,Iterator>& r)
         {
            if (current_index_ >  target_index_)
               return;
            else if (current_index_ == target_index_)
            {
               typename colsel_value_list::value_t& v = cvl_.value_list[col_list_index_];
               if (true != (v.second = v.first(r.first,r.second)))
               {
                  ++error_count_;
               }
               ++col_list_index_;
               if (col_list_index_ < column_list_t::size)
                  target_index_ = column_list_.index_list[col_list_index_];
               else
                  target_index_ = std::numeric_limits<std::size_t>::max();
            }
            ++current_index_;
         }

         inline colsel_value_list& cvl()
         {
            return cvl_;
         }

         const column_list_t& column_list_;
         std::size_t current_index_;
         std::size_t target_index_;
         std::size_t col_list_index_;
         std::size_t error_count_;
         colsel_value_list cvl_;

      private:

         csb_t& operator=(const csb_t& csb);
      };

      template <typename T0  = void, typename T1  = void, typename T2 = void, typename T3 = void, typename T4 = void,
                typename T5  = void, typename T6  = void, typename T7 = void, typename T8 = void, typename T9 = void,
                typename T10 = void, typename T11 = void>
      class column_selector_impl
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>,12>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>,12> csb_t;
         typedef column_list_impl<12> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                              T5& t5, T6& t6, T7& t7, T8& t8, T9& t9,
                              T10& t10, T11& t11)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value( t0); csb_t::cvl().register_value( t1);
            csb_t::cvl().register_value( t2); csb_t::cvl().register_value( t3);
            csb_t::cvl().register_value( t4); csb_t::cvl().register_value( t5);
            csb_t::cvl().register_value( t6); csb_t::cvl().register_value( t7);
            csb_t::cvl().register_value( t8); csb_t::cvl().register_value( t9);
            csb_t::cvl().register_value(t10); csb_t::cvl().register_value(t11);
         }
      };

      template <typename T0, typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8, typename T9,
                typename T10>
      class column_selector_impl <T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>,11>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>,11> csb_t;
         typedef column_list_impl<11> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                              T5& t5, T6& t6, T7& t7, T8& t8, T9& t9,
                              T10& t10)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value( t0); csb_t::cvl().register_value( t1);
            csb_t::cvl().register_value( t2); csb_t::cvl().register_value( t3);
            csb_t::cvl().register_value( t4); csb_t::cvl().register_value( t5);
            csb_t::cvl().register_value( t6); csb_t::cvl().register_value( t7);
            csb_t::cvl().register_value( t8); csb_t::cvl().register_value( t9);
            csb_t::cvl().register_value(t10);
         }
      };

      template <typename T0, typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8, typename T9>
      class column_selector_impl <T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>,10>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>,10> csb_t;
         typedef column_list_impl<10> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                              T5& t5, T6& t6, T7& t7, T8& t8, T9& t9)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
            csb_t::cvl().register_value(t4); csb_t::cvl().register_value(t5);
            csb_t::cvl().register_value(t6); csb_t::cvl().register_value(t7);
            csb_t::cvl().register_value(t8); csb_t::cvl().register_value(t9);
         }
      };

      template <typename T0, typename T1, typename T2, typename T3, typename T4,
                typename T5, typename T6, typename T7, typename T8>
      class column_selector_impl <T0,T1,T2,T3,T4,T5,T6,T7,T8>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8>,9>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8>,9> csb_t;
         typedef column_list_impl<9> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                              T5& t5, T6& t6, T7& t7, T8& t8)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
            csb_t::cvl().register_value(t4); csb_t::cvl().register_value(t5);
            csb_t::cvl().register_value(t6); csb_t::cvl().register_value(t7);
            csb_t::cvl().register_value(t8);
         }
      };

      template <typename T0, typename T1, typename T2, typename T3,
                typename T4, typename T5, typename T6, typename T7>
      class column_selector_impl <T0,T1,T2,T3,T4,T5,T6,T7>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7>,8>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7>,8> csb_t;
         typedef column_list_impl<8> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2, T3& t3,
                              T4& t4, T5& t5, T6& t6, T7& t7)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
            csb_t::cvl().register_value(t4); csb_t::cvl().register_value(t5);
            csb_t::cvl().register_value(t6); csb_t::cvl().register_value(t7);
         }
      };

      template <typename T0, typename T1, typename T2, typename T3,
                typename T4, typename T5, typename T6>
      class column_selector_impl <T0,T1,T2,T3,T4,T5,T6>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6>,7>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5,T6>,7> csb_t;
         typedef column_list_impl<7> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2, T3& t3,
                              T4& t4, T5& t5, T6& t6)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
            csb_t::cvl().register_value(t4); csb_t::cvl().register_value(t5);
            csb_t::cvl().register_value(t6);
         }
      };

      template <typename T0, typename T1, typename T2,
                typename T3, typename T4, typename T5>
      class column_selector_impl <T0,T1,T2,T3,T4,T5>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5>,6>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4,T5>,6> csb_t;
         typedef column_list_impl<6> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2,
                              T3& t3, T4& t4, T5& t5)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
            csb_t::cvl().register_value(t4); csb_t::cvl().register_value(t5);
         }
      };

      template <typename T0, typename T1, typename T2,
                typename T3, typename T4>
      class column_selector_impl <T0,T1,T2,T3,T4>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3,T4>,5>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3,T4>,5> csb_t;
         typedef column_list_impl<5> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2,
                              T3& t3, T4& t4)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
            csb_t::cvl().register_value(t4);
         }
      };

      template <typename T0, typename T1, typename T2, typename T3>
      class column_selector_impl <T0,T1,T2,T3>
            : public column_selector_base<column_selector_impl<T0,T1,T2,T3>,4>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2,T3>,4> csb_t;
         typedef column_list_impl<4> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2, T3& t3)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2); csb_t::cvl().register_value(t3);
         }
      };

      template <typename T0, typename T1, typename T2>
      class column_selector_impl <T0,T1,T2>
            : public column_selector_base<column_selector_impl<T0,T1,T2>,3>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1,T2>,3> csb_t;
         typedef column_list_impl<3> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1, T2& t2)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
            csb_t::cvl().register_value(t2);
         }
      };

      template <typename T0, typename T1>
      class column_selector_impl <T0,T1>
            : public column_selector_base<column_selector_impl<T0,T1>,2>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0,T1>,2> csb_t;
         typedef column_list_impl<2> column_list_t;

         column_selector_impl(const column_list_t& column_list,
                              T1& t0, T1& t1)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0); csb_t::cvl().register_value(t1);
         }
      };

      template <typename T0>
      class column_selector_impl <T0>
            : public column_selector_base<column_selector_impl<T0>,1>
      {
      public:

         typedef column_selector_base<column_selector_impl<T0>,1> csb_t;
         typedef column_list_impl<1> column_list_t;

         column_selector_impl(const column_list_t& column_list, T0& t0)
         : csb_t(column_list)
         {
            csb_t::cvl().register_value(t0);
         }
      };

   }

   inline details::column_list_impl<12>
      column_list(const std::size_t&  idx0, const std::size_t&  idx1,
                  const std::size_t&  idx2, const std::size_t&  idx3,
                  const std::size_t&  idx4, const std::size_t&  idx5,
                  const std::size_t&  idx6, const std::size_t&  idx7,
                  const std::size_t&  idx8, const std::size_t&  idx9,
                  const std::size_t& idx10, const std::size_t& idx11)
   {
      details::column_list_impl<12> cli;
      cli.index_list[ 0] =  idx0; cli.index_list[ 1] = idx1;
      cli.index_list[ 2] =  idx2; cli.index_list[ 3] = idx3;
      cli.index_list[ 4] =  idx4; cli.index_list[ 5] = idx5;
      cli.index_list[ 6] =  idx6; cli.index_list[ 7] = idx7;
      cli.index_list[ 8] =  idx8; cli.index_list[ 9] = idx9;
      cli.index_list[10] = idx10; cli.index_list[11] = idx11;
      return cli;
   }

   inline details::column_list_impl<11>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4, const std::size_t& idx5,
                  const std::size_t& idx6, const std::size_t& idx7,
                  const std::size_t& idx8, const std::size_t& idx9,
                  const std::size_t& idx10)
   {
      details::column_list_impl<11> cli;
      cli.index_list[ 0] =  idx0; cli.index_list[1] = idx1;
      cli.index_list[ 2] =  idx2; cli.index_list[3] = idx3;
      cli.index_list[ 4] =  idx4; cli.index_list[5] = idx5;
      cli.index_list[ 6] =  idx6; cli.index_list[7] = idx7;
      cli.index_list[ 8] =  idx8; cli.index_list[9] = idx9;
      cli.index_list[10] = idx10;
      return cli;
   }

   inline details::column_list_impl<10>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4, const std::size_t& idx5,
                  const std::size_t& idx6, const std::size_t& idx7,
                  const std::size_t& idx8, const std::size_t& idx9)
   {
      details::column_list_impl<10> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      cli.index_list[4] = idx4; cli.index_list[5] = idx5;
      cli.index_list[6] = idx6; cli.index_list[7] = idx7;
      cli.index_list[8] = idx8; cli.index_list[9] = idx9;
      return cli;
   }

   inline details::column_list_impl<9>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4, const std::size_t& idx5,
                  const std::size_t& idx6, const std::size_t& idx7,
                  const std::size_t& idx8)
   {
      details::column_list_impl<9> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      cli.index_list[4] = idx4; cli.index_list[5] = idx5;
      cli.index_list[6] = idx6; cli.index_list[7] = idx7;
      cli.index_list[8] = idx8;
      return cli;
   }

   inline details::column_list_impl<8>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4, const std::size_t& idx5,
                  const std::size_t& idx6, const std::size_t& idx7)
   {
      details::column_list_impl<8> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      cli.index_list[4] = idx4; cli.index_list[5] = idx5;
      cli.index_list[6] = idx6; cli.index_list[7] = idx7;
      return cli;
   }

   inline details::column_list_impl<7>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4, const std::size_t& idx5,
                  const std::size_t& idx6)
   {
      details::column_list_impl<7> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      cli.index_list[4] = idx4; cli.index_list[5] = idx5;
      cli.index_list[6] = idx6;
      return cli;
   }

   inline details::column_list_impl<6>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4, const std::size_t& idx5)
   {
      details::column_list_impl<6> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      cli.index_list[4] = idx4; cli.index_list[5] = idx5;
      return cli;
   }

   inline details::column_list_impl<5>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3,
                  const std::size_t& idx4)
   {
      details::column_list_impl<5> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      cli.index_list[4] = idx4;
      return cli;
   }

   inline details::column_list_impl<4>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2, const std::size_t& idx3)
   {
      details::column_list_impl<4> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2; cli.index_list[3] = idx3;
      return cli;
   }

   inline details::column_list_impl<3>
      column_list(const std::size_t& idx0, const std::size_t& idx1,
                  const std::size_t& idx2)
   {
      details::column_list_impl<3> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      cli.index_list[2] = idx2;
      return cli;
   }

   inline details::column_list_impl<2>
      column_list(const std::size_t& idx0, const std::size_t& idx1)
   {
      details::column_list_impl<2> cli;
      cli.index_list[0] = idx0; cli.index_list[1] = idx1;
      return cli;
   }

   inline details::column_list_impl<1>
      column_list(const std::size_t& idx0)
   {
      details::column_list_impl<1> cli;
      cli.index_list[0] = idx0;
      return cli;
   }

   inline details::column_list_impl<12> column_list(const std::size_t (&idx)[12])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5],
                         idx[6],idx[7],idx[8],idx[9],idx[10],idx[11]);
   }

   inline details::column_list_impl<11> column_list(const std::size_t (&idx)[11])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5],
                         idx[6],idx[7],idx[8],idx[9],idx[10]);
   }

   inline details::column_list_impl<10> column_list(const std::size_t (&idx)[10])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5],
                         idx[6],idx[7],idx[8],idx[9]);
   }

   inline details::column_list_impl<9> column_list(const std::size_t (&idx)[9])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5],
                         idx[6],idx[7],idx[8]);
   }

   inline details::column_list_impl<8> column_list(const std::size_t (&idx)[8])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5],
                         idx[6],idx[7]);
   }

   inline details::column_list_impl<7> column_list(const std::size_t (&idx)[7])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5],idx[6]);
   }

   inline details::column_list_impl<6> column_list(const std::size_t (&idx)[6])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4],idx[5]);
   }

   inline details::column_list_impl<5> column_list(const std::size_t (&idx)[5])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3],idx[4]);
   }

   inline details::column_list_impl<4> column_list(const std::size_t (&idx)[4])
   {
      return column_list(idx[0],idx[1],idx[2],idx[3]);
   }

   inline details::column_list_impl<3> column_list(const std::size_t (&idx)[3])
   {
      return column_list(idx[0],idx[1],idx[2]);
   }

   inline details::column_list_impl<2> column_list(const std::size_t (&idx)[2])
   {
      return column_list(idx[0],idx[1]);
   }

   inline details::column_list_impl<1> column_list(const std::size_t (&idx)[1])
   {
      return column_list(idx[0]);
   }

   template <typename T0, typename T1, typename  T2, typename T3,
             typename T4, typename T5, typename  T6, typename T7,
             typename T8, typename T9, typename T10, typename T11>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>
      column_selector(const details::column_list_impl<11>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3,  T4&  t4,  T5& t5,
                      T6& t6, T7& t7, T8& t8, T9& t9, T10& t10, T11& t11)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>
               (col_list,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11);
   }

   template <typename T0, typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6, typename T7,
             typename T8, typename T9, typename T10>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>
      column_selector(const details::column_list_impl<11>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                      T6& t6, T7& t7, T8& t8, T9& t9, T10& t10)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10>
               (col_list,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10);
   }

   template <typename T0, typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6, typename T7,
             typename T8, typename T9>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>
      column_selector(const details::column_list_impl<10>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                      T6& t6, T7& t7, T8& t8, T9& t9)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>
               (col_list,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9);
   }

   template <typename T0, typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6, typename T7,
             typename T8>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7,T8>
      column_selector(const details::column_list_impl<9>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                      T6& t6, T7& t7, T8& t8)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5,T6,T7,T8>
               (col_list,t0,t1,t2,t3,t4,t5,t6,t7,t8);
   }

   template <typename T0, typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6, typename T7>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5,T6,T7>
      column_selector(const details::column_list_impl<8>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                      T6& t6, T7& t7)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5,T6,T7>
               (col_list,t0,t1,t2,t3,t4,t5,t6,t7);
   }

   template <typename T0, typename T1, typename T2, typename T3,
             typename T4, typename T5, typename T6>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5,T6>
      column_selector(const details::column_list_impl<7>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5,T6>
               (col_list,t0,t1,t2,t3,t4,t5,t6);
   }

   template <typename T0, typename T1, typename T2, typename T3,
             typename T4, typename T5>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4,T5>
      column_selector(const details::column_list_impl<6>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4,T5>
               (col_list,t0,t1,t2,t3,t4,t5);
   }

   template <typename T0, typename T1, typename T2,
             typename T3, typename T4>
   inline typename details::column_selector_impl<T0,T1,T2,T3,T4>
      column_selector(const details::column_list_impl<5>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3, T4& t4)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3,T4>
               (col_list,t0,t1,t2,t3,t4);
   }

   template <typename T0, typename T1, typename T2, typename T3>
   inline typename details::column_selector_impl<T0,T1,T2,T3>
      column_selector(const details::column_list_impl<4>& col_list,
                      T0& t0, T1& t1, T2& t2, T3& t3)
   {
      return
         details::column_selector_impl
            <T0,T1,T2,T3>
               (col_list,t0,t1,t2,t3);
   }

   template <typename T0, typename T1, typename T2>
   inline typename details::column_selector_impl<T0,T1,T2>
      column_selector(const details::column_list_impl<3>& col_list,
                      T0& t0, T1& t1, T2& t2)
   {
      return
         details::column_selector_impl
            <T0,T1,T2>
               (col_list,t0,t1,t2);
   }

   template <typename T0, typename T1>
   inline typename details::column_selector_impl<T0,T1>
      column_selector(const details::column_list_impl<2>& col_list,
                      T0& t0, T1& t1)
   {
      return
         details::column_selector_impl
            <T0,T1>
               (col_list,t0,t1);
   }

   template <typename T0>
   inline typename details::column_selector_impl<T0>
      column_selector(const details::column_list_impl<1>& col_list, T0& t0)
   {
      return
         details::column_selector_impl
            <T0>
               (col_list,t0);
   }

   namespace details
   {
      template <typename Iterator>
      inline Iterator inc(Iterator itr, const std::size_t& n)
      {
         std::advance(itr,n);
         return itr;
      }

      //Single type column selectors
      template <typename T, std::size_t N>
      struct compose_st_selector_impl
      {};

      template <typename T>
      struct compose_st_selector_impl <T,1>
      {
         typedef column_selector_impl<T> type;
         typedef column_list_impl<1> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,2>
      {
         typedef column_selector_impl<T,T> type;
         typedef column_list_impl<2> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,3>
      {
         typedef column_selector_impl<T,T,T> type;
         typedef column_list_impl<3> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,4>
      {
         typedef column_selector_impl<T,T,T,T> type;
         typedef column_list_impl<4> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,5>
      {
         typedef column_selector_impl<T,T,T,T,T> type;
         typedef column_list_impl<5> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3],seq[4]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3),*inc(b,4));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,6>
      {
         typedef column_selector_impl<T,T,T,T,T,T> type;
         typedef column_list_impl<6> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3],seq[4],seq[5]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3),*inc(b,4),*inc(b,5));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,7>
      {
         typedef column_selector_impl<T,T,T,T,T,T,T> type;
         typedef column_list_impl<7> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3],seq[4],seq[5],seq[6]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3),*inc(b,4),*inc(b,5),*inc(b,6));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,8>
      {
         typedef column_selector_impl<T,T,T,T,T,T,T,T> type;
         typedef column_list_impl<8> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3],seq[4],seq[5],seq[6],seq[7]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3),*inc(b,4),*inc(b,5),*inc(b,6),*inc(b,7));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,9>
      {
         typedef column_selector_impl<T,T,T,T,T,T,T,T,T> type;
         typedef column_list_impl<9> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3],seq[4],seq[5],seq[6],seq[7],seq[8]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3),*inc(b,4),*inc(b,5),*inc(b,6),*inc(b,7),*inc(b,8));
         }
      };

      template <typename T>
      struct compose_st_selector_impl <T,10>
      {
         typedef column_selector_impl<T,T,T,T,T,T,T,T,T,T> type;
         typedef column_list_impl<10> column_list_t;

         template <typename Allocator,
                   template <typename,typename> class Sequence>
         static inline type create(const column_list_t& col_list, Sequence<T,Allocator>& seq)
         {
            return type(col_list,seq[0],seq[1],seq[2],seq[3],seq[4],seq[5],seq[6],seq[7],seq[8],seq[9]);
         }

         template <typename Allocator>
         static inline type create(const column_list_t& col_list, std::list<T,Allocator>& list)
         {
            typename std::list<T,Allocator>::iterator b = list.begin();
            return type(col_list,*(b),*inc(b,1),*inc(b,2),*inc(b,3),*inc(b,4),*inc(b,5),*inc(b,6),*inc(b,7),*inc(b,8),*inc(b,9));
         }
      };

   }

   template <std::size_t N,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline typename details::compose_st_selector_impl<T,N>::type
      column_selector(const details::column_list_impl<N>& col_list, Sequence<T,Allocator>& seq)
   {
      if (seq.size() >= N)
      {
         typedef typename details::compose_st_selector_impl<T,N> composer_t;
         return composer_t::create(col_list,seq);
      }
      else
         throw std::invalid_argument("column_selector(sequence/list) - size < N!");
   }

   namespace details
   {

      template <typename InputIterator, std::size_t N>
      class column_selector_iterator_impl
      {
      public:

         typedef column_selector_iterator_impl<InputIterator,N> csii_t;
         typedef details::column_list_impl<N> column_list_t;
         typedef std::pair<InputIterator,InputIterator> iterator_type;
         typedef iterator_type* iterator_type_ptr;

         column_selector_iterator_impl(const details::column_list_impl<N>& column_list,iterator_type (&token_list)[N])
         : column_list_(column_list),
           token_list_(token_list),
           current_index_(0),
           target_index_(column_list_.index_list[0]),
           col_list_index_(0)
         {}

         inline csii_t& operator*()
         {
            return (*this);
         }

         inline csii_t& operator++()
         {
            return (*this);
         }

         inline csii_t operator++(int)
         {
            return (*this);
         }

         template <typename Iterator>
         inline csii_t& operator=(const std::pair<Iterator,Iterator>& r)
         {
            if (current_index_ == target_index_)
            {
               token_list_[col_list_index_] = r;
               ++col_list_index_;
               if (col_list_index_ < column_list_t::size)
                  target_index_ = column_list_.index_list[col_list_index_];
               else
                  target_index_ = std::numeric_limits<std::size_t>::max();
            }
            ++current_index_;
            return (*this);
         }

      private:

         csii_t& operator=(const csii_t& csb);

         const column_list_t& column_list_;
         iterator_type_ptr token_list_;
         std::size_t current_index_;
         std::size_t target_index_;
         std::size_t col_list_index_;
      };

   }

   #define strtk_parse_col_token(Index) \
      if (!string_to_type_converter(token_list[Index].first,token_list[Index].second,t##Index)) return false;

   #define strtk_parse_col_token_seq(Index) \
      if (!string_to_type_converter(token_list[Index].first,token_list[Index].second,seq[Index])) return false;

   #define strtk_parse_columns_impl(NN)                                          \
      static const std::size_t N = NN;                                           \
      typedef typename details::is_valid_iterator<InputIterator>::type itr_type; \
      typedef std::pair<InputIterator,InputIterator> iterator_type;              \
      typedef details::column_selector_iterator_impl<InputIterator,N> csii_t;    \
      const std::size_t token_count = (column_list.index_list[N - 1] + 1);       \
      details::convert_type_assert<itr_type>();                                  \
      iterator_type token_list[N];                                               \
      csii_t csii(column_list,token_list);                                       \
      const std::size_t parsed_token_count = split_n<InputIterator,csii_t&>      \
      (delimiters,begin,end,token_count,csii,split_options::compress_delimiters);\
      if (token_count > parsed_token_count) return false;                        \

   #define strk_parse_col_seq \
      return parse_columns(data.data(),data.data() + data.size(),delimiters,column_list,seq);

   template <typename InputIterator,
             typename  T0, typename T1, typename T2, typename T3, typename T4,
             typename  T5, typename T6, typename T7, typename T8, typename T9,
             typename T10, typename T11>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<12>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                             T6& t6, T7& t7, T8& t8, T9& t9, T10& t10, T11& t11)
   {
      strtk_parse_columns_impl(12)
      strtk_parse_col_token( 0) strtk_parse_col_token( 1)
      strtk_parse_col_token( 2) strtk_parse_col_token( 3)
      strtk_parse_col_token( 4) strtk_parse_col_token( 5)
      strtk_parse_col_token( 6) strtk_parse_col_token( 7)
      strtk_parse_col_token( 8) strtk_parse_col_token( 9)
      strtk_parse_col_token(10) strtk_parse_col_token(11)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8, typename T9,
             typename T10>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<11>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                             T6& t6, T7& t7, T8& t8, T9& t9, T10& t10)
   {
      strtk_parse_columns_impl(11)
      strtk_parse_col_token( 0) strtk_parse_col_token(1)
      strtk_parse_col_token( 2) strtk_parse_col_token(3)
      strtk_parse_col_token( 4) strtk_parse_col_token(5)
      strtk_parse_col_token( 6) strtk_parse_col_token(7)
      strtk_parse_col_token( 8) strtk_parse_col_token(9)
      strtk_parse_col_token(10)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8, typename T9>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<10>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5,
                             T6& t6, T7& t7, T8& t8, T9& t9)
   {
      strtk_parse_columns_impl(10)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      strtk_parse_col_token(4) strtk_parse_col_token(5)
      strtk_parse_col_token(6) strtk_parse_col_token(7)
      strtk_parse_col_token(8) strtk_parse_col_token(9)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<9>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6,
                             T7& t7, T8& t8)
   {
      strtk_parse_columns_impl(9)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      strtk_parse_col_token(4) strtk_parse_col_token(5)
      strtk_parse_col_token(6) strtk_parse_col_token(7)
      strtk_parse_col_token(8)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<8>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6, T7& t7)
   {
      strtk_parse_columns_impl(8)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      strtk_parse_col_token(4) strtk_parse_col_token(5)
      strtk_parse_col_token(6) strtk_parse_col_token(7)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<7>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5, T6& t6)
   {
      strtk_parse_columns_impl(7)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      strtk_parse_col_token(4) strtk_parse_col_token(5)
      strtk_parse_col_token(6)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<6>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4, T5& t5)
   {
      strtk_parse_columns_impl(6)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      strtk_parse_col_token(4) strtk_parse_col_token(5)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3, typename T4>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<5>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4)
   {
      strtk_parse_columns_impl(5)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      strtk_parse_col_token(4)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2, typename T3>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<4>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3)
   {
      strtk_parse_columns_impl(4)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2) strtk_parse_col_token(3)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1, typename T2>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<3>& column_list,
                             T0& t0, T1& t1, T2& t2)
   {
      strtk_parse_columns_impl(3)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      strtk_parse_col_token(2)
      return true;
   }

   template <typename InputIterator,
             typename T0, typename T1>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<2>& column_list,
                             T0& t0, T1& t1)
   {
      strtk_parse_columns_impl(2)
      strtk_parse_col_token(0) strtk_parse_col_token(1)
      return true;
   }

   template <typename InputIterator,
             typename T0>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<1>& column_list,
                             T0& t0)
   {
      strtk_parse_columns_impl(1)
      strtk_parse_col_token(0)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<12>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(12)
      strtk_parse_col_token_seq( 0) strtk_parse_col_token_seq( 1)
      strtk_parse_col_token_seq( 2) strtk_parse_col_token_seq( 3)
      strtk_parse_col_token_seq( 4) strtk_parse_col_token_seq( 5)
      strtk_parse_col_token_seq( 6) strtk_parse_col_token_seq( 7)
      strtk_parse_col_token_seq( 8) strtk_parse_col_token_seq( 9)
      strtk_parse_col_token_seq(10) strtk_parse_col_token_seq(11)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<11>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(11)
      strtk_parse_col_token_seq( 0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq( 2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq( 4) strtk_parse_col_token_seq(5)
      strtk_parse_col_token_seq( 6) strtk_parse_col_token_seq(7)
      strtk_parse_col_token_seq( 8) strtk_parse_col_token_seq(9)
      strtk_parse_col_token_seq(10)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<10>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(10)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq(4) strtk_parse_col_token_seq(5)
      strtk_parse_col_token_seq(6) strtk_parse_col_token_seq(7)
      strtk_parse_col_token_seq(8) strtk_parse_col_token_seq(9)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<9>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(9)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq(4) strtk_parse_col_token_seq(5)
      strtk_parse_col_token_seq(6) strtk_parse_col_token_seq(7)
      strtk_parse_col_token_seq(8)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<8>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(8)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq(4) strtk_parse_col_token_seq(5)
      strtk_parse_col_token_seq(6) strtk_parse_col_token_seq(7)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<7>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(7)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq(4) strtk_parse_col_token_seq(5)
      strtk_parse_col_token_seq(6)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<6>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(6)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq(4) strtk_parse_col_token_seq(5)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<5>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(5)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      strtk_parse_col_token_seq(4)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<4>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(4)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2) strtk_parse_col_token_seq(3)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<3>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(3)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      strtk_parse_col_token_seq(2)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<2>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(2)
      strtk_parse_col_token_seq(0) strtk_parse_col_token_seq(1)
      return true;
   }

   template <typename InputIterator,
             typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const InputIterator begin,
                             const InputIterator end,
                             const std::string& delimiters,
                             const details::column_list_impl<1>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strtk_parse_columns_impl(1)
      strtk_parse_col_token_seq(0)
      return true;
   }

   #undef strtk_parse_col_token
   #undef strtk_parse_col_token_seq
   #undef strtk_parse_columns_impl

   #define strtk_parse_col_begin()                              \
      return parse_columns(data.data(),                         \
                           data.data() + data.size(),delimiters,\
                           column_list,                         \

   #define strtk_parse_col_end() );

   template <typename  T0, typename  T1, typename T2, typename T3, typename T4,
             typename  T5, typename  T6, typename T7, typename T8, typename T9,
             typename T10, typename T11>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<12>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5, T6& t6, T7& t7, T8& t8, T9& t9,
                             T10& t10, T11& t11)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11
      strtk_parse_col_end()
   }

   template <typename  T0, typename T1, typename T2, typename T3, typename T4,
             typename  T5, typename T6, typename T7, typename T8, typename T9,
             typename T10>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<11>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5, T6& t6, T7& t7, T8& t8, T9& t9,
                             T10& t10)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8, typename T9>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<10>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5, T6& t6, T7& t7, T8& t8,
                             T9& t9)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5,t6,t7,t8,t9
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7, typename T8>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<9>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5, T6& t6, T7& t7, T8& t8)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5,t6,t7,t8
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<8>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5, T6& t6, T7& t7)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5,t6,t7
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<7>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5, T6& t6)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5,t6
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3, typename T4,
             typename T5>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<6>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4,
                             T5& t5)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4,t5
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<5>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3, T4& t4)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3,t4
      strtk_parse_col_end()
   }

   template <typename T0, typename T1, typename T2, typename T3>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<4>& column_list,
                             T0& t0, T1& t1, T2& t2, T3& t3)
   {
      strtk_parse_col_begin()
         t0,t1,t2,t3
      strtk_parse_col_end();
   }

   template <typename T0, typename T1, typename T2>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<3>& column_list,
                             T0& t0, T1& t1, T2& t2)
   {
      strtk_parse_col_begin()
         t0,t1,t2
      strtk_parse_col_end()
   }

   template <typename T0, typename T1>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<2>& column_list,
                             T0& t0, T1& t1)
   {
      strtk_parse_col_begin()
         t0,t1
      strtk_parse_col_end()
   }

   template <typename T>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<1>& column_list,
                             T& t)
   {
      strtk_parse_col_begin()
         t
      strtk_parse_col_end()
   }

   #undef strtk_parse_col_begin
   #undef strtk_parse_col_end

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<12>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<11>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<10>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<9>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<8>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<7>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<6>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<5>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<4>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<3>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<2>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline bool parse_columns(const std::string& data,
                             const std::string& delimiters,
                             const details::column_list_impl<1>& column_list,
                             Sequence<T,Allocator>& seq)
   {
      strk_parse_col_seq
   }

   #undef strk_parse_col_seq

   namespace details
   {
      typedef const unsigned char* ptr;

      template <typename T>
      bool cmpimpl(ptr c1, ptr c2) { return (*reinterpret_cast<T>(c1)) == (*reinterpret_cast<T>(c2)); }

      template <std::size_t K>
      struct size_impl { static inline bool cmp(ptr,ptr) { return true; } };

      template <>
      struct size_impl<8> { static inline bool cmp(ptr c1, ptr c2) { return cmpimpl<const unsigned long long*>(c1,c2); } };

      template <>
      struct size_impl<4> { static inline bool cmp(ptr c1, ptr c2) { return cmpimpl<const unsigned int*>(c1,c2); } };

      template <>
      struct size_impl<2> { static inline bool cmp(ptr c1, ptr c2) { return cmpimpl<const unsigned short*>(c1,c2); } };

      template <>
      struct size_impl<1> { static inline bool cmp(ptr c1, ptr c2) { return cmpimpl<const unsigned char*>(c1,c2); } };

      template <std::size_t N>
      struct next_size { enum { size = (N >= 8) ? 8 : ((N >= 4) ? 4 : ((N >= 2) ? 2 : 1)) }; };

      template <std::size_t N>
      struct memcmp_n_impl
      {
         static inline bool process(details::ptr c1, details::ptr c2)
         {
            static const std::size_t size = details::next_size<N>::size;
            return details::size_impl<size>::cmp(c1,c2) && memcmp_n_impl<N - size>::process(c1 + size, c2 + size);
         }

         static inline bool process(const char* c1, const char* c2)
         {
            return memcmp_n_impl<N>::process(reinterpret_cast<details::ptr>(c1),reinterpret_cast<details::ptr>(c2));
         }

         template <std::size_t K1, std::size_t K2>
         static inline bool process(const unsigned char (&c1)[K1], const unsigned char (&c2)[K2])
         {
            return memcmp_n_impl<N>::process(static_cast<ptr>(c1),static_cast<ptr>(c2));
         }
      };

      template<> struct memcmp_n_impl<0> { static inline bool process(ptr,ptr) { return true; } };
   }

   template <std::size_t N>
   inline bool memcmp_n(details::ptr c1, details::ptr c2)
   {
      return details::memcmp_n_impl<N>::process(c1,c2);
   }

   template <std::size_t N>
   inline bool memcmp_n(const char* c1, const char* c2)
   {
      return details::memcmp_n_impl<N>::process(c1,c2);
   }

   template <std::size_t N,std::size_t K1, std::size_t K2>
   inline bool memcmp_n(const unsigned char (&c1)[K1], const unsigned char (&c2)[K2])
   {
      return details::memcmp_n_impl<N>::process(c1,c2);
   }

   namespace details
   {
      inline bool type_to_string_converter_impl(const strtk::util::value& v, std::string& result, value_type_tag)
      {
         return v.to_string(result);
      }
   }

   template <typename Iterator>
   inline std::size_t distance(const std::pair<Iterator,Iterator>& p)
   {
      return std::distance(p.first,p.second);
   }

   template <typename Iterator1, typename Iterator2>
   inline std::pair<Iterator1,Iterator2> make_pair(const std::string& s)
   {
      return std::make_pair<Iterator1,Iterator2>(
             reinterpret_cast<Iterator1>(const_cast<char*>(s.data())),
             reinterpret_cast<Iterator2>(const_cast<char*>(s.data() + s.size())));
   }

   template <typename Iterator1, typename Iterator2>
   inline std::pair<Iterator1,Iterator2> make_pair(const std::pair<const char*, const char*> p)
   {
      return std::make_pair<Iterator1,Iterator2>(
             reinterpret_cast<Iterator1>(const_cast<char*>(p.first)),
             reinterpret_cast<Iterator2>(const_cast<char*>(p.second)));
   }

   template <typename Iterator>
   inline std::pair<Iterator,Iterator> make_pair(const std::string& s)
   {
      return make_pair<Iterator,Iterator>(s);
   }

   template <typename Iterator>
   inline std::pair<Iterator,Iterator> make_pair(const std::pair<const char*, const char*>& p)
   {
      return make_pair<Iterator,Iterator>(p);
   }

   template <typename Iterator1, typename Iterator2>
   inline std::pair<Iterator1,Iterator2> make_pair(const strtk::range::string& range)
   {
      return std::make_pair<Iterator1,Iterator2>(
             reinterpret_cast<Iterator1>(const_cast<char*>(range.begin())),
             reinterpret_cast<Iterator2>(const_cast<char*>(range.end())));
   }

   template <std::size_t N>
   inline std::string make_string(const unsigned char (&s)[N], const std::size_t& length = N)
   {
      static const std::string null_string;
      if (N < length)
         return null_string;
      else
         return std::string(&s[0],&s[0] + length);
   }

   template <std::size_t N>
   inline std::string make_string(const char (&s)[N], const std::size_t& length = N)
   {
      static const std::string null_string;
      if (N < length)
         return null_string;
      else
         return std::string(&s[0],&s[0] + length);
   }

   inline std::string make_string(const std::pair<const char*,const char*>& range)
   {
      return std::string(range.first,range.second);
   }

   template <typename T, std::size_t N>
   inline bool clear_array(T (&a)[N], const T& t, const std::size_t& length = N)
   {
      if (N < length)
         return false;
      else
         std::fill_n(&a[0],length,t);
      return true;
   }

   template <std::size_t N>
   inline bool set_array(unsigned char (&a)[N],
                         const std::string& s,
                         const bool pad = false,
                         const unsigned char padding = '0')
   {
      if (N < s.size())
         return false;
      std::copy(s.data(),s.data() + s.size(), &a[0]);
      if ((s.size() < N) && pad)
         std::fill_n(&a[s.size()],N - s.size(),padding);
      return true;
   }

   template <std::size_t N, std::size_t M>
   inline bool set_array(unsigned char (&dest)[N],
                         unsigned char (&src)[M],
                         const bool pad = false,
                         const unsigned char padding = '0')
   {
      if (N < M)
         return false;
      std::copy(src,src + N, &dest[0]);
      if ((M < N) && pad)
         std::fill_n(&dest[M],N - M,padding);
      return true;
   }

   inline void reverse(const std_string::iterator_type& range)
   {
      char* begin = const_cast<char*>(range.first);
      char* end = const_cast<char*>(range.second);
      std::reverse(begin,end);
   }

   template <typename T>
   inline void reverse(const range::adapter<T>& r)
   {
      T* begin = const_cast<T*>(r.begin());
      T* end = const_cast<T*>(r.end());
      std::reverse(begin,end);
   }

   template <typename T>
   inline void reverse(const range::adapter<const T>& r)
   {
      T* begin = const_cast<T*>(r.begin());
      T* end = const_cast<T*>(r.end());
      std::reverse(begin,end);
   }

   inline void reverse(std::string& s)
   {
      std::reverse(s.begin(),s.end());
   }

   inline void fill(std::string& s, const std::string::value_type v)
   {
      std::fill(const_cast<char*>(s.data()),const_cast<char*>(s.data() + s.size()), v);
   }

   inline void fill(const std::pair<const char*,const char*>& range, char v)
   {
      char* begin = const_cast<char*>(range.first);
      char* end = const_cast<char*>(range.second);
      std::fill(begin,end,v);
   }

   template <typename T>
   inline void fill(const range::adapter<const T>& r, const typename range::adapter<const T>::value_type& v)
   {
      char* begin = const_cast<char*>(r.begin());
      char* end = const_cast<char*>(r.end());
      std::fill(begin,end,v);
   }

   inline void fill(const std_string::iterator_type& range, const std::string::value_type& v)
   {
      char* begin = const_cast<char*>(range.first);
      char* end = const_cast<char*>(range.second);
      std::fill(begin,end,v);
   }

   template <typename T,
             typename Allocator,
             template <typename,typename> class Sequence>
   inline void fill(Sequence<T,Allocator>& seq, const T& t)
   {
      if (seq.empty())
         return;
      std::fill_n(seq.begin(),seq.size(),t);
   }

   namespace keyvalue
   {
      template <typename CharType>
      struct options
      {
         typedef CharType char_type;

         options()
         : pair_block_delimiter(0),
           pair_delimiter(0)
         {}

         char_type pair_block_delimiter;
         char_type pair_delimiter;
      };

      template <typename KeyValueMap>
      class parser
      {
      public:

         typedef unsigned char char_type;
         typedef std::pair<char_type*,char_type*> range_type;

         template <typename Options>
         parser(const Options& opts)
         : options_(opts),
           kv_map_(opts),
           pair_block_sdp_(options_.pair_block_delimiter),
           pair_delimiter_sdp_(options_.pair_delimiter)
         {
            const std::size_t pair_list_default_size = 32;
            pair_list_.reserve(pair_list_default_size);
         }

         template <typename T>
         inline bool register_keyvalue(const typename KeyValueMap::key_type& key, T& t)
         {
            return kv_map_.register_keyvalue(key,t);
         }

         inline bool operator()(const range_type& data, const bool ignore_failures = false)
         {
            if (!ignore_failures)
            {
               pair_list_.clear();
               const std::size_t pair_count = split(pair_block_sdp_,
                                                    data.first,
                                                    data.second,
                                                    std::back_inserter(pair_list_));

               if (0 == pair_count)
                  return false;

               range_type key_range;
               range_type value_range;

               for (std::size_t i = 0; i < pair_count; ++i)
               {
                  const range_type& r = pair_list_[i];
                  if (0 == std::distance(r.first,r.second))
                     continue;
                  else if (!split_pair(r.first,r.second,
                                       pair_delimiter_sdp_,
                                       key_range,value_range))
                     return false;
                  else if (!kv_map_(key_range,value_range))
                     return false;
               }
               return true;
            }
            else
            {
                parse_failures_ = 0;
                pair_token_processor processor(*this);
                split(pair_block_sdp_,
                      data.first,
                      data.second,
                      strtk::functional_inserter(processor));
                return true;
            }
         }

         inline bool operator()(const std::string& s, const bool ignore_failures = false)
         {
            return operator()(strtk::make_pair<range_type::first_type>(s),ignore_failures);
         }

         inline std::size_t failure_count() const
         {
            return parse_failures_;
         }

      private:

         class pair_token_processor
         {
         public:

            pair_token_processor(parser<KeyValueMap>& p)
            : parser_(p)
            {}

            inline void operator()(const range_type& r)
            {
               if (r.first == r.second)
                  return;
               if (split_pair(r.first,r.second,
                              parser_.pair_delimiter_sdp_,
                              key_range,
                              value_range))
               {
                  if (parser_.kv_map_(key_range,value_range))
                     return;
               }
               ++parser_.parse_failures_;
            }

         private:

            pair_token_processor operator=(const pair_token_processor&);

            parser<KeyValueMap>& parser_;
            range_type key_range;
            range_type value_range;
         };

         options<char_type> options_;
         std::size_t parse_failures_;
         KeyValueMap kv_map_;
         single_delimiter_predicate<char_type> pair_block_sdp_;
         single_delimiter_predicate<char_type> pair_delimiter_sdp_;
         std::vector<range_type> pair_list_;
      };

      class uintkey_map
      {
      private:

         typedef unsigned char char_type;
         typedef strtk::keyvalue::options<char_type> general_options;

      public:

         typedef unsigned int key_type;

         struct options : public general_options
         {
            options()
            : general_options(),
              key_count(0)
            {}

            std::size_t key_count;
         };

         template <typename Options>
         uintkey_map(const Options& options)
         {
            value_lut_.resize(options.key_count,strtk::util::value());
         }

         virtual ~uintkey_map()
         {}

         template <typename Range>
         inline bool operator()(const Range& key_range, const Range& value_range)
         {
            std::size_t key = 0;
            if (!fast::numeric_convert(distance(key_range),key_range.first,key,true))
               return false;
            if (key >= value_lut_.size())
               return false;
            const strtk::util::value& v = value_lut_[key];
            if (!v)
               return false;
            else
               return v(value_range);
         }

         template <typename T>
         inline bool register_keyvalue(const key_type& key, T& t)
         {
            if (key < value_lut_.size())
            {
               strtk::util::value& v = value_lut_[key];
               if (!v)
                  v = strtk::util::value(t);
               else
                  v.assign(t);
               return true;
            }
            else
               return false;
         }

      private:

         std::vector<strtk::util::value> value_lut_;
      };

      namespace details
      {
         template <typename Range,typename KType>
         struct keygen
         {
            static inline KType transform(const Range&)
            {
               return KType();
            }
         };

         template <typename Range>
         struct keygen<Range,std::string>
         {
            static inline std::string transform(const Range& key_range)
            {
               return std::string(key_range.first,key_range.second);
            }
         };

         template <typename Range>
         struct keygen<Range,unsigned int>
         {
            static inline unsigned int transform(const Range& key_range)
            {
               unsigned int result = 0;
               if (strtk::fast::numeric_convert(std::distance(key_range.first,key_range.second),key_range.first,result,true))
                  return result;
               else
                  return std::numeric_limits<unsigned int>::max();
            }
         };

         struct no_op_validator
         {
            template <typename Range>
            inline bool operator()(const Range&)
            {
               return true;
            }
         };
      }

      template <typename KeyType,
                typename MapType = std::map<KeyType,strtk::util::value>,
                typename KeyValidator = details::no_op_validator,
                typename ValueValidator = details::no_op_validator>
      class key_map
      {
      public:

         typedef KeyType key_type;
         typedef MapType map_type;
         typedef KeyValidator key_validator_type;
         typedef ValueValidator value_validator_type;

         template <typename Options>
         key_map(const Options&)
         {}

         virtual ~key_map()
         {}

         template <typename Range>
         inline bool operator()(const Range& key_range, const Range& value_range)
         {
            if (!key_validator_(key_range))
               return false;
            if (!val_validator_(value_range))
               return false;
            typename map_type::iterator itr = value_map_.find(details::keygen<Range,key_type>::transform(key_range));
            if (value_map_.end() == itr)
               return false;
            const util::value& v = (*itr).second;
            if (!v)
               return false;
            else
               return v(value_range);
         }

         template <typename T>
         inline bool register_keyvalue(const key_type& key, T& t)
         {
            strtk::util::value& v = value_map_[key];
            if (!v)
               v = strtk::util::value(t);
            else
               v.assign(t);
            return true;
         }

      private:

         map_type value_map_;
         key_validator_type key_validator_;
         value_validator_type val_validator_;
      };

      typedef key_map<std::string> stringkey_map;

   }
} // namespace strtk

namespace
{

   static inline std::ostream& operator<<(std::ostream& os,
                                          const strtk::std_string::tokenizer<strtk::single_delimiter_predicate<char> >::type::iterator& range)
   {
      os << std::string((*range).first,(*range).second);
      return os;
   }

   static inline std::ostream& operator<<(std::ostream& os,
                                          const strtk::std_string::tokenizer<strtk::single_delimiter_predicate<unsigned char> >::type::iterator& range)
   {
      os << std::string((*range).first,(*range).second);
      return os;
   }

   static inline std::ostream& operator<<(std::ostream& os,
                                          const strtk::std_string::tokenizer<strtk::multiple_char_delimiter_predicate>::type::iterator& range)
   {
      os << std::string((*range).first,(*range).second);
      return os;
   }

   #define strtk_register_pair_to_ostream(Iterator)                                                   \
   static inline std::ostream& operator<<(std::ostream& os, const std::pair<Iterator,Iterator>& range)\
   { os << std::string(range.first,range.second); return os; }                                        \
   static inline std::ostream& operator<<(std::ostream& os, std::pair<Iterator,Iterator>& range)      \
   { os << std::string(range.first,range.second); return os; }                                        \

   strtk_register_pair_to_ostream(char*)
   strtk_register_pair_to_ostream(unsigned char*)
   strtk_register_pair_to_ostream(const char*)
   strtk_register_pair_to_ostream(const unsigned char*)
   strtk_register_pair_to_ostream(std::string::iterator)
   strtk_register_pair_to_ostream(std::string::const_iterator)
   strtk_register_pair_to_ostream(const std::string::iterator)
   strtk_register_pair_to_ostream(const std::string::const_iterator)

   #undef strtk_register_pair_to_ostream

} // namespace anonymous

#if 0

#ifdef WIN32
   #ifndef NOMINMAX
      #define NOMINMAX
   #endif
   #ifndef WIN32_LEAN_AND_MEAN
      #define WIN32_LEAN_AND_MEAN
   #endif
   #include <windows.h>
#else
   #include <sys/time.h>
   #include <sys/types.h>
#endif
namespace strtk
{
   namespace util
   {
      class timer
      {
      public:

         #ifdef WIN32
            timer()
            : in_use_(false)
            {
               QueryPerformanceFrequency(&clock_frequency_);
            }

            inline void start()
            {
               in_use_ = true;
               QueryPerformanceCounter(&start_time_);
            }

            inline void stop()
            {
               QueryPerformanceCounter(&stop_time_);
               in_use_ = false;
            }

            inline double time() const
            {
               return (1.0 * (stop_time_.QuadPart - start_time_.QuadPart)) / (1.0 * clock_frequency_.QuadPart);
            }

         #else

            timer()
            : in_use_(false)
            {
               start_time_.tv_sec  = 0;
               start_time_.tv_usec = 0;
               stop_time_.tv_sec   = 0;
               stop_time_.tv_usec  = 0;
            }

            inline void start()
            {
               in_use_ = true;
               gettimeofday(&start_time_,0);
            }

            inline void stop()
            {
               gettimeofday(&stop_time_, 0);
               in_use_ = false;
            }

            inline unsigned long long int usec_time() const
            {
               if (!in_use_)
               {
                  if (stop_time_.tv_sec >= start_time_.tv_sec)
                  {
                     return 1000000 * (stop_time_.tv_sec  - start_time_.tv_sec ) +
                                      (stop_time_.tv_usec - start_time_.tv_usec);
                  }
                  else
                     return std::numeric_limits<unsigned long long int>::max();
               }
               else
                  return std::numeric_limits<unsigned long long int>::max();
            }

            inline double time() const
            {
               return usec_time() * 0.000001;
            }

         #endif

            inline bool in_use() const
            {
               return in_use_;
            }

      private:

            bool in_use_;

         #ifdef WIN32
            LARGE_INTEGER start_time_;
            LARGE_INTEGER stop_time_;
            LARGE_INTEGER clock_frequency_;
         #else
            struct timeval start_time_;
            struct timeval stop_time_;
         #endif
      };

      class scoped_timer
      {
      public:

         scoped_timer(double& time_value)
         : time_value_(time_value)
         {
            t_.start();
         }

        ~scoped_timer()
         {
            t_.stop();
            time_value_ = t_.time();
         }

      private:

         scoped_timer(const scoped_timer&);
         scoped_timer& operator=(const scoped_timer&);

         double& time_value_;
         timer t_;
      };

   } // namespace util

   namespace information
   {
      static const char* library = "String Toolkit";
      static const char* version = "2.718281828459045235360287471352662497757247093699959574";
      static const char* date    = "20140118";

      static inline std::string data()
      {
         static const std::string info_str = std::string(library) +
                                             std::string(" v") + std::string(version) +
                                             std::string(" (") + date + std::string(")");
         return info_str;
      }

   } // namespace information

} // namespace strtk

#endif

#endif