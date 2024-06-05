/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief Text tools utilities header
 *   \author Oleh Sharuda
 */

#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <memory>
#include <regex>
#include <list>
#include <utility>
#include <map>

#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/regex.h>
#include <unicode/datefmt.h>

using namespace icu;

/// \addtogroup group_tools
/// @{
/// \defgroup group_text_tools Text tools
/// \brief Text and string utilities
/// @{
/// \page page_text_tools
/// \tableofcontents
///
/// \section sect_text_tools_01 Text tools
/// Text tools are used to perform miscellaneous operations on strings and text. In order to work with Unicode ICU4C library
/// is used. As well known C++ support of unicode is deprecated. It is still work in clang, and never appeared in GCC,
/// therefore it seems lke ICU4C is the only reliable (and official) way to support Unicode. Some ICU4C functionalities
/// are not thread safe, therefore tools#ICUHelper class must be allocated with thread local specification. This will
/// avoid race conditions during work with Unicode.
///
/// Most of the features here may be implemented in newer C++ standards, possibly there are some ways to do the same with
/// existing STL features, and I missed them.
///


namespace tools {

    /// \typedef u16_string
    /// \brief Typedef for two byte character string
	typedef std::basic_string<char16_t> u16_string;


	/// \class ICUHelper
	/// \brief This class is a wrapper for some of ICU4C features.
	class ICUHelper final {

		UConverter* conv_utf16_le = nullptr;    ///< Convertor for UTF-16 LE.
		UConverter* conv_utf16_be = nullptr;    ///< Convertor for UTF-16 BE.
		UConverter* conv_utf8 = nullptr;        ///< Convertor for UTF-8.
		UConverter* conv_default = nullptr;     ///< Default unicode convertor.
		DateFormat* date_formatter = nullptr;   ///< Default date and time formatter.

		/// \brief Creates icu::UnicodeString from std::string.
		/// \param src - Source string.
        /// \param err - Reference to output error code.
		/// \return Unicode string.
        icu::UnicodeString make_unicode_string(const std::string& src, UErrorCode& err);

        /// \brief Creates icu::UnicodeString from two byte string.
        /// \param src - Source string.
        /// \param convertor - Convertor to be used.
        /// \param err - Reference to output error code.
        /// \return Unicode string.
        icu::UnicodeString make_unicode_string(const u16_string& src, UConverter* convertor, UErrorCode& err);

        /// \brief Creates std::string from icu::UnicodeString.
        /// \param us - Input unicode string.
        /// \param dst - Reference to output string.
        /// \param err - Reference to output error code.
        void make_string(icu::UnicodeString& us, std::string& dst, UErrorCode& err);

        /// \brief Creates two byte string from icu::UnicodeString.
        /// \param us - Input unicode string.
        /// \param dst - Reference to output string.
        /// \param convertor - Convertor to be used.
        /// \param err - Reference to output error code.
        void make_u16string(icu::UnicodeString& us, u16_string& dst, UConverter* convertor, UErrorCode& err);

        /// \brief Counts amount of character in two byte string.
        /// \param s - Null terminated string to be measured.
        /// \param convertor - Convertor to be used.
        /// \param err - Reference to output error code.
        /// \return Size of the string in characters.
        size_t count_chars(const char16_t* s, UConverter* convertor, UErrorCode& err);

		public:
	    /// \brief Constructor
		ICUHelper();

	    /// \brief Destructor
		~ICUHelper();

		/// \brief Checks if string consist of ASCII characters.
		/// \param s - string to test.
		/// \return true if ASCII string, otherwise false.
		bool is_ascii(const std::string& s);

		/// \brief Converts UTF-8 string to UTF-16 string.
		/// \param src - source UTF-8 string.
		/// \param dst - destination UTF-16 string.
		/// \param little_endian - true if UTF-16 LE is required, false for UTF-16 BE.
		/// \return true if success, otherwise false.
		bool utf8_to_utf16(const std::string& src, u16_string& dst, bool little_endian);

		/// \brief Converts from UTF-16 string to UTF-8 string.
		/// \param src - source UTF-16 string.
		/// \param dst - destination UTF-8 string.
        /// \param little_endian - true if UTF-16 LE is provided, false for UTF-16 BE.
        /// \return true if success, otherwise false.
		bool utf16_to_utf8(const u16_string& src, std::string& dst, bool little_endian);

		/// \brief Converts from UTF-16 string to wide character string.
        /// \param src - source UTF-16 string.
        /// \param dst - destination wide character string.
		void utf16_to_wide(const u16_string& src, std::wstring& dst);

		/// \brief Converts from wide character string to UTF-16 string.
		/// \param src - source wide character string.
		/// \param dst - destination UTF-16 string.
		void wide_to_utf16(const std::wstring& src, u16_string& dst);

		/// \brief Converts UTF-8 string to some case.
		/// \param s - reference to input/output string to be converted.
		/// \param lowcase - true - to convert to lower case, false to convert to upper case.
        /// \return true if success, otherwise false.
		bool to_case(std::string& s, bool lowcase);

		/// \brief Converts UTF-16 string to some case.
		/// \param s - reference to input/output string to be converted.
		/// \param lowcase - true - to convert to lower case, false to convert to upper case.
		/// \param little_endian - true if UTF-16 LE is provided, false for UTF-16 BE.
        /// \return true if success, otherwise false.
		bool to_case(u16_string& s, bool lowcase, bool little_endian);

		/// \brief Creates regular expression pattern.
		/// \param pattern - regular expression pattern.
		/// \param flags - flags for regular expression pattern.
		/// \return std::unique_ptr with regular expression.
		std::unique_ptr<RegexPattern> regex_pattern(const std::string& pattern, uint32_t flags);

		/// \brief Match groups for regular expression.
		/// \param pattern - regular expression pattern.
		/// \param s - string to be matched.
		/// \param groups - output array with groups. Groups are pushed to the end of the vector.
		/// \return true if match, otherwise false.
		bool regex_groups(const RegexPattern& pattern, const std::string& s, std::vector<std::string>& groups);

		/// \brief Matches regular expression.
		/// \param pattern - regular expression pattern.
        /// \param s - string to be matched.
        /// \return true if match, otherwise false.
        bool regex_match(const RegexPattern& pattern, const std::string& s);

        /// \brief Converts std::time to string representation using default date and time formatter.
        /// \param t - time to convert.
        /// \return string representation of time.
		std::string dtime_to_utf8(std::time_t t);
	};

	/// \brief Wrapper for conversion from UTF-8 to wide character string.
	/// \param s - input string.
	/// \return wide character string.
	std::wstring utf8_to_wstr(const std::string& s);

    /// \brief Wrapper for conversion from wide character to UTF-8 string.
    /// \param s - input string.
    /// \return wide character string.
	std::string wstr_to_utf8(const std::wstring& s);

	/// \class SpecialCharacterTables
	/// \brief This class contains special character tables that may improve some string operations
	class SpecialCharacterTables final {
    public:

		static const uint8_t hex_val[]; ///< Converts ASCII character to HEX value. 255 means invalid hex character.

		static const char hex_upcase[]; ///< Converts hex digit value into hex upper case character.

		static const char hex_lwcase[]; ///< Converts hex digit value into hex lower case character.
    };

	/// \brief Keeps ICUHelper object for current thread. Object is created on the first usage of the variable.
	static thread_local ICUHelper g_unicode_ts;

	/// \brief Trims string from the both ends.
	/// \tparam ChT - Character type
	/// \tparam UnaryPredicate - type of the unary predicated which returns true if whitespace character is passed.
	/// \param s - reference to input/output string to trim.
	/// \param trim_char_pred - unary predicate to test characters for white space.
	/// \return new string length
	template<class ChT, class UnaryPredicate>
	size_t trim_string(std::basic_string<ChT>& s, UnaryPredicate trim_char_pred) {
	    typename std::basic_string<ChT>::const_iterator end = s.end();	    
	    typename std::basic_string<ChT>::const_iterator start = s.begin();		
		start = std::find_if_not(start, end, trim_char_pred);
		if (start==end) {
			s.clear();
			return 0;
		} else {
			end = std::find_if_not(s.crbegin(), s.crend(), trim_char_pred).base();
			s = std::basic_string<ChT>(start, end);
			return s.length();
		}
	}

	/// \brief Splits and trim input string
	/// \tparam ChT - type of the characters being used.
	/// \tparam SplitPredicate - type of the unary predicated which returns true if split character is passed.
	/// \tparam TrimPredicate - type of the unary predicated which returns true if whitespace character is passed.
	/// \param text - text to be processed
	/// \param split_pred - split unary predicate which returns true if split character is passed.
	/// \param trim_pred - split unary predicate which returns true if whitespace character is passed.
	/// \return vector of strings
	template<class ChT, class SplitPredicate, class TrimPredicate>
	std::vector<std::basic_string<ChT>> split_and_trim(const std::basic_string<ChT>& text, SplitPredicate split_pred, TrimPredicate trim_pred) {
	    typename std::vector<std::string> result;
	    typename std::basic_string<ChT> s;
	    typename std::basic_string<ChT>::const_iterator start = text.begin();
	    typename std::basic_string<ChT>::const_iterator end = text.end();	    
	    typename std::basic_string<ChT>::const_iterator next_pos;

	    for (start = std::find_if_not(start, end, split_pred); start!=end; start = std::find_if_not(next_pos, end, split_pred))
	    {
	        next_pos = std::find_if(start, end, split_pred);
	        if (next_pos==end) {
	            s = std::basic_string<ChT>(start, end);
	            if (trim_string(s, trim_pred)) result.push_back(s);
	            break;
	        } else {
	            s = std::basic_string<ChT>(start, next_pos);
	            if (trim_string(s, trim_pred) ) {
	                result.push_back(s);    // Do not add empty strings
	            }

	            ++next_pos;
	            if (next_pos==end) {
	                break;
	            }
	        }
	    }

	    return result;
	}

	/// \brief Checks if string has specified prefix
	/// \tparam ChT - type of the character being used
	/// \param s - const reference to string to test
	/// \param p - const reference to prefix
	/// \return true if string s is prefixed by p.
	template<class ChT>
	bool check_prefix(const std::basic_string<ChT>& s, const std::basic_string<ChT>& p) {
	    size_t s_len = s.length();
	    size_t p_len = p.length();

	    if (s_len > p_len) {
	        return s.substr(0, p_len).compare(p)==0;
	    } else if (p_len==s_len) {
	        return s.compare(p)==0;
	    }

	    return false;
	}

    /// \brief Checks if string has specified prefix
    /// \tparam ChT - type of the character being used
    /// \param s - const pointer to null terminated string to test.
    /// \param p - const pointer to null terminated prefix.
    /// \return true if string s is prefixed by p.
	template<class ChT>
	bool check_prefix(const ChT* s, const ChT* p) {
		return tools::check_prefix(std::basic_string<ChT>(s), std::basic_string<ChT>(p));
	}

    /// \brief Checks if string has specified prefix
    /// \tparam ChT - type of the character being used
    /// \param s - const reference to string to test
    /// \param p - const pointer to null terminated prefix.
    /// \return true if string s is prefixed by p.
	template<class ChT>
	bool check_prefix(const std::basic_string<ChT>& s, const ChT* p) {
		return tools::check_prefix(s, std::basic_string<ChT>(p));
	}

	/// \brief Parses command arguments with the following format "arg1, arg2, argN"
    /// \tparam ChT - type of the character being used
    /// \param s - const reference to input string.
	/// \param args - reference to output argument parameter. Parsed arguments are appended to the args. First argument
	///        appended is trimmed s string.
	/// \return Number of arguments parsed.
	/// \note  Quotes are not escaped, but are taken into account
	template <class ChT>
	int parse_args(const std::basic_string<ChT>& s, std::vector<std::basic_string<ChT>>& args) {
	    std::basic_string<ChT> s_args = s;
	    std::basic_string<ChT> a;
	    size_t arg_c = 0;	    
	    static const char no_quote = '\0';
	    char quote = no_quote;	    
	    tools::trim_string(s_args, [](char b){return std::isspace(b);});
	    size_t len = s_args.length();
	    a.reserve(len);

	    args.push_back(s_args);

	    for (size_t pos=0; pos<len; ++pos) {
	        char c = s_args[pos];

	        if (c==quote) {
	            // close quotation
	            quote = no_quote;
	            a+=c;
	        } else if (quote!=no_quote) {
	            // continue quotation
	            a+=c;
	        } else if (c=='\'' or c=='\"') {
	            // start quotation
	            quote=c;
	            a+=c;
	        } else if (c==',') {
	            // start next argument
	            tools::trim_string(a, [](ChT c){return std::isspace(c);});
	            args.push_back(a);
	            arg_c++;
	            a.clear();
	        } else {
	            // accumulate argument
	            a+=c;
	        }
	    }

	    tools::trim_string(a, [](ChT c){return std::isspace(c);});
	    if (arg_c!=0 || a.length()!=0) {
	        args.push_back(a);
	        arg_c++;
	    }

	    if (quote!=no_quote) {
	    	args.clear();
	    	return -1;
	    } else {
	    	return arg_c;
	    }		
	}

	/// \brief Converts buffer to hex string
	/// \param buffer - pointer to the buffer memory
	/// \param len - length of the buffer
	/// \param lwrcase - if true all hex characters will be in lower case, otherwise they will be in upper case.
	/// \param separator - string that separates every byte of the sequence.
	/// \return string that represent hex buffer.
	std::string buffer_to_hex(const uint8_t* buffer, size_t len, bool lwrcase, const char* separator);

	/// \brief Creates buffer from hex.
	/// \param hex - string with hex buffer.
	/// \return std::vector holding allocated buffer.
	std::vector<uint8_t> buffer_from_hex(const std::string& hex);

	/// \brief Takes a buffer and format it as multiline text.
	/// \param bytes_per_line - how many bytes to put in every line.
	/// \param buffer - pointer to the memory buffer.
	/// \param buffer_len buffer length.
	/// \param line_prefix - prefix for every line.
	/// \param text_separator - string that separates hex data from text
	/// \return string that represent memory buffer
    std::string format_buffer(size_t bytes_per_line,
                              const uint8_t* buffer,
                              size_t buffer_len,
                              const std::string& line_prefix,
                              const std::string& text_separator);

    /// \brief Converts memory buffer into ASCII text, with unprintable characters substituted by some character
    /// \param buffer - pointer to the memory buffer.
    /// \param len - length of the buffer.
    /// \param unprintable_char - character to be used instead of unprintable characters.
    /// \return string that represent memory buffer.
    /// \details Printable characters are those in range between 0x20 and 0x7F ASCII codes.
    std::string buffer_to_ascii(const uint8_t* buffer, size_t len, char unprintable_char);

    /// \brief Calculates length of the string for constexpr string.
    /// \param str - string to be measured.
    /// \return size of the string (constexpr).
    /// \details Calculates string length recursively on compilation stage, may be used for constant strings known
    ///          during compilation.
	size_t constexpr const_strlen(const char* str)
	{
	    return *str ? const_strlen(str + 1) + 1 : 0;
	}

	/// \brief Format string.
	/// \tparam ChT - Type of the character being used.
	/// \param format - Format string.
	/// \return Resulting string.
	/// \details This call is used as template recursion terminator.
	template <typename ChT>
	std::basic_string<ChT> format_string(const ChT* format) {
		return std::basic_string<ChT>(format);
	}

    /// \brief Format string.
    /// \tparam ChT - Type of the character being used.
    /// \param format - Format string.
    /// \return Resulting string.
	template <typename ChT>
	std::basic_string<ChT> format_string(const std::basic_string<ChT>& format) {
		return format;
	}

    /// \brief Format string.
    /// \tparam ChT - Type of the character being used.
	/// \tparam T - Type of the first object in queue to format.
	/// \tparam Args - Type template parameter pack
    /// \param format - Format string.
    /// \param val - Reference to the first value.
	/// \param args - Parameter pack.
    /// \return Resulting string.
	/// \details This variadic template uses recursion in order to format a string.
	template <typename ChT, typename T, typename... Args>
	std::basic_string<ChT> format_string(const std::basic_string<ChT>& format, const T& val, Args... args) {
		return format_string(format.c_str(), val, args...);
	}

    /// \brief Format string.
    /// \tparam ChT - Type of the character being used.
    /// \tparam T - Type of the first object in queue to format.
    /// \tparam Args - Type template parameter pack
    /// \param format - Format string.
    /// \param val - Reference to the first value.
    /// \param args - Parameter pack.
    /// \return Resulting string.
    /// \details This variadic template uses recursion in order to format a string.
	template <typename ChT, typename T, typename... Args>
	std::basic_string<ChT> format_string(const ChT* format, const T& val, Args... args) {
		ChT* fmt = const_cast<ChT*>(format);
		std::basic_stringstream<ChT> result;

  		while (*fmt) { 
		    if (*fmt == '%' && *(++fmt) != '%') {
			    // seems like specifier
		    	result << val;
			    result << format_string(++fmt, args...);
			    return result.str();
		    }

		    // add single character
		    result << *(fmt++);
	  	}

	  	throw std::runtime_error("format specification requires less arguments");
	}

	/// \brief Wrapper for std::vsnprintf call.
	/// \param format string
	/// \param ... - other arguments
	/// \return resulting string
	std::string str_format(const char *format, ... );

    /// \brief Joins strings in container into single string and using a separator.
    /// \tparam ChT - Type of the character being used.
    /// \tparam T - Type of the container.
	/// \param container - const reference to input container with strings.
	/// \param separator - string to separate concatenated strings.
	/// \return Concatenated string.
	template <typename ChT, typename T>
	std::basic_string<ChT> join_strings(const T& container, const std::basic_string<ChT>& separator) {
		size_t el_count = container.size();
		size_t sep_len = separator.length();
		size_t datalen = 0;
		std::basic_string<ChT> res;
		typename T::const_iterator i;

		if (container.empty()) {
			goto done;
		}

		// calculate amount of data required in order to reserve memory for result string.
		for (i=container.begin(); i!=container.end(); ++i) {
			datalen+=i->length();
		}

		// append separators
		datalen += el_count>1 ? (el_count - 1)*sep_len : 0;
		res.reserve(datalen+1);

		i=container.begin();
		do {
			res+= *i;

			++i;
			if (i==container.end())
				break;

			res += separator;

		} while (true);

	done:		
		return res;
	}

    /// \brief Joins strings in container into single string and using a separator.
    /// \tparam ChT - Type of the character being used.
    /// \tparam T - Type of the container.
    /// \param container - const reference to input container with strings.
    /// \param separator - pointer to null terminated string to separate concatenated strings.
    /// \return Concatenated string.

	template <typename ChT, typename T>
	std::basic_string<ChT> join_strings(const T& container, const ChT* separator) {
		return join_strings(container, std::basic_string<ChT>(separator));
	}

	/// \brief Return text representation of flags set in the value
    /// \tparam ChT - Type of the character being used.
	/// \tparam V - Type of the value with flags.
	/// \param flags - value containing flags set and cleared.
	/// \param names - std::map with key equal to flag, and value - a pair of name of the flag if it is set and name of the
	///                flag if it is cleared. Empty names are not printed and ignored.
	/// \param separator - pointer to null terminated string to separate flags.
	/// \return Resulting string with flags description.
    template <typename ChT, typename V>
    std::basic_string<ChT> flags_to_string(V flags,
                                           const std::map<V, std::pair< std::basic_string<ChT>, std::basic_string<ChT>>>& names, const ChT* separator) {
        size_t nbits = sizeof(V)*8;
        std::list<std::basic_string<ChT>> vallist;
        std::basic_string<ChT> s;
        for (size_t i=0; i<nbits; i++) {
            V f = 1 << i;
            auto kvp = names.find(f);
            if (kvp==names.end()) continue;
            s = f & flags ? kvp->second.first : kvp->second.second;

            if (!s.empty()) {
                vallist.push_front(s);
            }
        }

        return join_strings(vallist, separator);
    }

}

/// @}
/// @}
