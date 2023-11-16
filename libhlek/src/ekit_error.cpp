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
 *   \brief EKitException class implementation
 *   \author Oleh Sharuda
 */

#include <errno.h>
#include <sstream>
#include <string>
#include <string.h>
#include "ekit_error.hpp"
#include <texttools.hpp>

#define CASE_ERROR_NAME(_name) case (_name): return #_name; break;
constexpr size_t EKIT_MAX_ERROR_DESCR_NAME = 256;
thread_local char last_error_descr[EKIT_MAX_ERROR_DESCR_NAME];
const char* errname(const EKIT_ERROR err) {
    switch (err) {
        CASE_ERROR_NAME(EKIT_OK);
        CASE_ERROR_NAME(EKIT_FAIL);
        CASE_ERROR_NAME(EKIT_CANT_CONNECT);
        CASE_ERROR_NAME(EKIT_ALREADY_CONNECTED);
        CASE_ERROR_NAME(EKIT_DISCONNECTED);
        CASE_ERROR_NAME(EKIT_NO_DATA);
        CASE_ERROR_NAME(EKIT_OPEN_FAILED);
        CASE_ERROR_NAME(EKIT_IOCTL_FAILED);
        CASE_ERROR_NAME(EKIT_DEVCTL_FAILED);
        CASE_ERROR_NAME(EKIT_READ_FAILED);
        CASE_ERROR_NAME(EKIT_CRC_ERROR);
        CASE_ERROR_NAME(EKIT_WRONG_DEVICE);
        CASE_ERROR_NAME(EKIT_COMMAND_FAILED);
        CASE_ERROR_NAME(EKIT_DEVICE_BUSY);
        CASE_ERROR_NAME(EKIT_NOT_COMPLETE);
        CASE_ERROR_NAME(EKIT_TIMEOUT);
        CASE_ERROR_NAME(EKIT_SUSPENDED);
        CASE_ERROR_NAME(EKIT_NOT_SUSPENDED);
        CASE_ERROR_NAME(EKIT_NOT_OPENED);
        CASE_ERROR_NAME(EKIT_WRITE_FAILED);
        CASE_ERROR_NAME(EKIT_BAD_PARAM);
        CASE_ERROR_NAME(EKIT_LOCKED);
        CASE_ERROR_NAME(EKIT_UNLOCKED);
        CASE_ERROR_NAME(EKIT_NOT_SUPPORTED);
        CASE_ERROR_NAME(EKIT_OVERFLOW);
        CASE_ERROR_NAME(EKIT_OUT_OF_RANGE);
        CASE_ERROR_NAME(EKIT_PROTOCOL);
        CASE_ERROR_NAME(EKIT_PARITY);
        CASE_ERROR_NAME(EKIT_COLLISION);
        CASE_ERROR_NAME(EKIT_UNALIGNED);
        CASE_ERROR_NAME(EKIT_TOO_FAST);
        CASE_ERROR_NAME(EKIT_NOT_STARTED);
        CASE_ERROR_NAME(EKIT_NOT_STOPPED);
        default:
            if (err < 0) {
                // Errno is used instead of EKIT_ERROR codes
                int errn = err * -1;
#if (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE
                if (strerror_r(errn, last_error_descr, EKIT_MAX_ERROR_DESCR_NAME)!=0) {
                    std::string err_str = tools::str_format("errno=%i)", errn);
                    strncpy(last_error_descr, err_str.c_str(), EKIT_MAX_ERROR_DESCR_NAME);
                }
#else
                const char* resbuf = strerror_r(errn, last_error_descr, EKIT_MAX_ERROR_DESCR_NAME);
                if (resbuf != last_error_descr) {
                    return resbuf;
                }
#endif
                return last_error_descr;
            } else {
                return "UNKNOWN";
            }
    }
}

EKitException::EKitException(const std::string& func_info, EKIT_ERROR e) :
    std::runtime_error(format_exception(func_info, e, "EKitException")),
    ekit_error(e) {}

EKitException::EKitException(const std::string& func_info, EKIT_ERROR e, const std::string& descr) :
    std::runtime_error(format_exception(func_info, e, descr)),
    ekit_error(e) {}

EKitException::~EKitException() {}

std::string EKitException::format_exception(const std::string& func_info, EKIT_ERROR e, const std::string& description) {
	std::stringstream ss;
	ss << description << "; (" <<  errname(e) << "); errcode=" << e << "; thrown from: " << func_info;
	return ss.str();
}
