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
 *   \brief Text tools utilities
 *   \author Oleh Sharuda
 */

#include "texttools.hpp"
#include <memory>
#include <cassert>
#include <unicode/errorcode.h>
#include <cstdarg>

namespace tools {

    // special array for faster decoding from text to buffer, 255 is not hex character, otherwise actual value
    const uint8_t SpecialCharacterTables::hex_val[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255,
                                                       255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

    const char SpecialCharacterTables::hex_upcase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    const char SpecialCharacterTables::hex_lwcase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};



	ICUHelper::ICUHelper () {
		UErrorCode status = U_ZERO_ERROR;

		conv_utf16_le = ucnv_open("UTF-16LE", &status);
		assert(U_SUCCESS(status));
		conv_utf16_be = ucnv_open("UTF-16BE", &status);
		assert(U_SUCCESS(status));
		conv_utf8 = ucnv_open("UTF8", &status);
		assert(U_SUCCESS(status));
		conv_default = ucnv_open(nullptr, &status);
		assert(U_SUCCESS(status));

		date_formatter = DateFormat::createDateTimeInstance();
	}

	ICUHelper::~ICUHelper() {
		ucnv_close(conv_utf16_le);
		ucnv_close(conv_utf16_be);
		ucnv_close(conv_utf8);
		delete date_formatter;
	}


	icu::UnicodeString ICUHelper::make_unicode_string(const std::string& src, UErrorCode& err) {
		return icu::UnicodeString(src.c_str(), -1, conv_default, err);
	}

	icu::UnicodeString ICUHelper::make_unicode_string(const tools::u16_string& src, UConverter* convertor, UErrorCode& err) {
		icu::UnicodeString res = icu::UnicodeString((char*)src.c_str(), src.length()*sizeof(char16_t), convertor, err);
		return  res;
	}

	void ICUHelper::make_string(icu::UnicodeString& us, std::string& dst, UErrorCode& err) {
		size_t len = count_chars((const char16_t*)us.getTerminatedBuffer(), conv_utf8, err);
		if (!U_SUCCESS(err)) {
			return;
		}

		std::unique_ptr<char[]> buffer(new char[len+1]);

		us.extract(buffer.get(), len, conv_utf8, err);
		if (!U_SUCCESS(err)) {
			return;
		} else if (err==U_STRING_NOT_TERMINATED_WARNING){
			err = U_ZERO_ERROR;
		}

		// extract is not required to make string null terminated, terminate it
		buffer.get()[len] = 0;

		dst = (const char*)buffer.get();
	}

	void ICUHelper::make_u16string(icu::UnicodeString& us, tools::u16_string& dst, UConverter* convertor, UErrorCode& err) {
		size_t len = count_chars((const char16_t*)us.getTerminatedBuffer(), convertor, err) / sizeof(char16_t);
		if (err!=U_ZERO_ERROR) {
			return;
		}

		std::unique_ptr<char16_t[]> buffer(new char16_t[len+1]);

		// convert to UTF16
		us.extract((char*)buffer.get(), len*sizeof(char16_t), convertor, err);
		if (!U_SUCCESS(err)) {
			return;
		} else if (err==U_STRING_NOT_TERMINATED_WARNING){
			err = U_ZERO_ERROR;
		}

		// extract is not required to make string null terminated, terminate it
		buffer.get()[len] = 0;

		dst = (const char16_t*)buffer.get();
	}


	size_t ICUHelper::count_chars(const char16_t* s, UConverter* convertor, UErrorCode& err) {
		size_t res = ucnv_fromUChars(convertor, NULL, 0, (const UChar*)s, -1, &err);
		
		if(err==U_BUFFER_OVERFLOW_ERROR) {
	    	err=U_ZERO_ERROR;
	    } else {
	    	res = 0;
	    }

	    return res;
	}

	bool ICUHelper::utf8_to_utf16(const std::string& src, u16_string& dst, bool little_endian) {
		UConverter* conv = little_endian ? conv_utf16_le : conv_utf16_be;
		UErrorCode err = U_ZERO_ERROR;
		icu::UnicodeString us(make_unicode_string(src, err));
		make_u16string(us, dst, conv, err);
		return U_SUCCESS(err);
	}

	bool ICUHelper::utf16_to_utf8(const u16_string& src, std::string& dst, bool little_endian) {
		UConverter* conv = little_endian ? conv_utf16_le : conv_utf16_be;
		UErrorCode err = U_ZERO_ERROR;
		icu::UnicodeString us(make_unicode_string(src, conv, err));
		make_string(us, dst, err);
		return U_SUCCESS(err);
	}

	bool ICUHelper::to_case(std::string& s, bool lowcase) {
		UErrorCode err = U_ZERO_ERROR;
		icu::UnicodeString us(make_unicode_string(s, err));
		if (err!=U_ZERO_ERROR) {
			return false;
		}

		if (lowcase) {
			us.toLower();
		} else {
			us.toUpper();
		}

		make_string(us, s, err);
		return err==U_ZERO_ERROR;
	}

	bool ICUHelper::to_case(u16_string& s, bool lowcase, bool little_endian) {
		UConverter* conv = little_endian ? conv_utf16_le : conv_utf16_be;
		UErrorCode err = U_ZERO_ERROR;
		icu::UnicodeString us(make_unicode_string(s, conv, err));

		if (err!=U_ZERO_ERROR) {
			return false;
		}	

		if (lowcase) {
			us.toLower();
		} else {
			us.toUpper();
		}	

		make_u16string(us, s, conv, err);
		return err==U_ZERO_ERROR;
	}

	void ICUHelper::utf16_to_wide(const u16_string& src, std::wstring& dst) {
		size_t slen = src.length();
		if (slen)
		{
			dst = std::wstring(slen, L' ');
			for (size_t i = 0; i<slen; ++i) {
				dst[i] = (wchar_t)src[i];
			}
		} else {
			dst.clear();
		}
	}
	void ICUHelper::wide_to_utf16(const std::wstring& src, u16_string& dst) {
		size_t slen = src.length();
		if (slen)
		{
			dst = u16_string(slen, L' ');
			for (size_t i = 0; i<slen; ++i) {
				dst[i] = (char16_t)src[i];
			}
		} else {
			dst.clear();
		}
	}

	std::wstring utf8_to_wstr(const std::string& s) {
		u16_string u16s;
		std::wstring res;
		g_unicode_ts.utf8_to_utf16(s, u16s, true);
		g_unicode_ts.utf16_to_wide(u16s, res);
		return res;
	}

	std::string wstr_to_utf8(const std::wstring& s) {
		u16_string u16s;
		std::string res;
		g_unicode_ts.wide_to_utf16(s, u16s);
		g_unicode_ts.utf16_to_utf8(u16s, res, true);
		return res;
	}

	std::unique_ptr<RegexPattern> ICUHelper::regex_pattern(const std::string& pattern, uint32_t flags) {
		UErrorCode err = U_ZERO_ERROR;
		UParseError pe;
		std::unique_ptr<RegexPattern> rpattern;

		icu::UnicodeString upattern(make_unicode_string(pattern, err));
		if (err!=U_ZERO_ERROR) {
			return rpattern;
		}		

		// Construct pattern
		rpattern.reset(RegexPattern::compile(upattern, flags, pe, err));


		if (err!=U_ZERO_ERROR) {
			rpattern.reset(nullptr);
		}

		return rpattern;
	}

	bool ICUHelper::regex_groups(const RegexPattern& pattern, const std::string& s, std::vector<std::string>& groups) {
		UErrorCode err = U_ZERO_ERROR;

		icu::UnicodeString us(make_unicode_string(s, err));
		if (err!=U_ZERO_ERROR) {
			return false;
		}

		// Create matcher
		std::unique_ptr<RegexMatcher> rm(pattern.matcher(us, err));
		if (!rm || err!=U_ZERO_ERROR) {
			return false;
		}

		int32_t n_groups = rm->groupCount()+1;

		if (rm->matches(0, err)) {
			for (int32_t i=0; i<n_groups; ++i) {
				UnicodeString ug = rm->group(i, err);
				
				if (!U_SUCCESS(err)) {
					return false;
				}

				std::string g;
				make_string(ug, g, err);

				if (!U_SUCCESS(err)) {
					return false;
				}

				groups.push_back(g);
			}

			return true;			
		}

		return false;
	}

    bool ICUHelper::regex_match(const RegexPattern& pattern, const std::string& s) {
        UErrorCode err = U_ZERO_ERROR;

        icu::UnicodeString us(make_unicode_string(s, err));
        if (err!=U_ZERO_ERROR) {
            return false;
        }

        // Create matcher
        std::unique_ptr<RegexMatcher> rm(pattern.matcher(us, err));
        if (!rm || err!=U_ZERO_ERROR) {
            return false;
        }

        return rm->matches(err);
	}


	bool ICUHelper::is_ascii(const std::string& s) {
		size_t len = s.length();
		for (size_t i=0; i<len; ++i) {
			if ((uint8_t)s[i] > 0x7f) {
				return false;
			}
		}
		return true;
	}

	std::string ICUHelper::dtime_to_utf8(std::time_t t) {
		UErrorCode err = U_ZERO_ERROR;
		UnicodeString t_string;
		std::string res;
		t_string.remove();
		date_formatter->format(static_cast<UDate>(t)*1000, t_string, NULL, err);
		assert(U_SUCCESS(err));

		make_string(t_string, res, err);
		assert(U_SUCCESS(err));

		return res;
	}

    template <class _Ch>
    std::string dump_string_hex(const std::basic_string<_Ch>& s, const std::string& separator) {
        size_t n_char = s.length();
        size_t n_sep = separator.length();
        size_t res_len = n_char>1 ? n_sep*n_char-1 : 0;
        res_len += (n_char+1)*sizeof(_Ch)*2;
        static const char digits[]  = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        std::string res;
        res.reserve(res_len);

        uint8_t* pbuf = (uint8_t*)s.c_str();
        size_t buflen = n_char*sizeof(_Ch);

        for (size_t i=0; i<buflen; ++i) {
            if (i!=0 && i%sizeof(_Ch)==0) {
                res+=separator;
            }

            uint8_t b = pbuf[i];
            char c0 = digits[b >> 4];
            char c1 = digits[b & 0x0F];

            res+=c0;
            res+=c1;
        }

        return res;
    }

	std::string buffer_to_hex(const uint8_t* buffer, size_t len, bool lwrcase, const char* separator) {
	    const char* digit_set = lwrcase ? SpecialCharacterTables::hex_lwcase : SpecialCharacterTables::hex_upcase;
	    size_t sep_len = separator!=nullptr ? strlen(separator) : 0;
	    size_t res_len = len>1 ? sep_len*(len-1)+(len+1)*2: (len+1)*2;	// length of separators

	    std::string res;
	    res.reserve(res_len);

	    for (size_t i=0; i<len; ++i) {
	        if (sep_len!=0 && i!=0) {
	            res+=separator;
	        }

	        uint8_t b = buffer[i];
	        char c0 = digit_set[b >> 4];
	        char c1 = digit_set[b & 0x0F];

	        res+=c0;
	        res+=c1;
	    }	    

	    return res;
	}

	std::vector<uint8_t> buffer_from_hex(const std::string& hex) {
	    size_t slen = hex.length();
	    std::vector<uint8_t> result;

	    if (slen%2!=0) {
	    	throw std::length_error("Hex buffer description must have even number of characters");
	    }

	    result.resize(slen/2);

	    for (size_t i=0;i<slen;) {

	        uint8_t b0 = SpecialCharacterTables::hex_val[hex[i++]];
	        uint8_t b1 = SpecialCharacterTables::hex_val[hex[i++]];

	        if (b0>0x0F || b1>0x0F) {
	        	throw std::out_of_range("Is not hex character");
	        }

	        result[(i/2)-1] = (b0 << 4) + b1;
	    }

	    return result;
	}

    std::string format_buffer(size_t bytes_per_line,
                              const uint8_t* buffer,
                              size_t buffer_len,
                              const std::string& line_prefix,
                              const std::string& text_separator) {
        size_t line_count = buffer_len/bytes_per_line;
        size_t tail_bytes = buffer_len%bytes_per_line;
        line_count+= (tail_bytes!=0);
        std::list<std::string> slist;
        size_t offset_length = 0;
        size_t t = buffer_len;
        for (size_t i=0; i<sizeof(size_t) && t!=0; i++, offset_length++) {
            t = t >> 8;
        }
        offset_length*=2;

        // get maximum width of the offset field

        // line has the following format (for bytes_per_line==4)
        // <prefix>XX XX XX XX XX<text_separator>....

        for (size_t l=0; l<line_count; l++) {
            const uint8_t* start = buffer + l*bytes_per_line;
            size_t len = (l==line_count-1) ? tail_bytes : bytes_per_line;
            size_t pad = bytes_per_line - len;
            size_t offset = l*bytes_per_line;

            std::string offset_prefix = str_format("%*X",offset_length, offset);

            std::string hex =buffer_to_hex(start, len, true, " ");
            hex += std::string(pad*3, ' ');

            std::string ascii = buffer_to_ascii(start, len, '.');
            ascii += std::string(pad, ' ');

            slist.push_back(line_prefix + offset_prefix + text_separator + hex + text_separator + ascii);
        }

        return join_strings(slist, "\n");
    }


    std::string buffer_to_ascii(const uint8_t* buffer, size_t len, char unprintable_char) {
	    std::string res;
	    res.reserve(len);

	    for (size_t i=0; i<len; i++) {
	        uint8_t b = buffer[i];
	        res += (b>=0x20 && b<0x7F) ? (char)b : unprintable_char;
	    }

        return res;
	}

    std::string str_format(const char *format, ... ) {
        std::vector<char> buffer;
        char* buf_ptr;

        va_list args1;
        va_start(args1, format);

        va_list args2;
        va_copy(args2, args1);


        int buflen = std::vsnprintf( nullptr, 0, format, args1 );
        va_end(args1);

        if( buflen < 0 ) {
            throw std::runtime_error( "bad format" );
        }
        buflen++;
        buffer.resize(buflen);
        buf_ptr = buffer.data();
        std::vsnprintf( buf_ptr, buflen, format, args2 );
        va_end(args2);

        // Terminate string
        buf_ptr[buflen-1] = '\0';
        return std::string(buf_ptr);
    }

}
