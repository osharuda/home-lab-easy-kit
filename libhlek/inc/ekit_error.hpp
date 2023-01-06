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
 *   \brief EKitException class header
 *   \author Oleh Sharuda
 */

#pragma once

#include <stdexcept>
#include <string>

/// \addtogroup group_communication
/// @{

/// \defgroup group_communication_error EKitException
/// \brief Exceptions and errors
/// @{
/// \page page_communication_error
/// \tableofcontents
///
/// \section sect_communication_error_01 Errors and exceptions
/// Home Lab Easy Kit project uses both error codes and exceptions. Internally error codes are mostly used, however,
/// public interfaces throw exceptions.
///

/// \typedef EKIT_ERROR
/// \brief Allias for error codes.
typedef int EKIT_ERROR;

#define ERRNO_TO_EKIT_ERROR(x) ((x)*-1)

extern thread_local char last_error_descr[];

#define EKIT_OK                   0 ///< OK.
#define EKIT_FAIL                 1 ///< Failure.
#define EKIT_CANT_CONNECT         2 ///< Can not connect.
#define EKIT_ALREADY_CONNECTED    3 ///< Already connected.
#define EKIT_DISCONNECTED         4 ///< Disconnected.
#define EKIT_NO_DATA              5 ///< No data available.
#define EKIT_OPEN_FAILED          6 ///< Open failure.
#define EKIT_IOCTL_FAILED         7 ///< ioctl() failure.
#define EKIT_DEVCTL_FAILED        8 ///< devctl() failure.
#define EKIT_READ_FAILED          9 ///< read() failure.
#define EKIT_CRC_ERROR            10 ///< CRC failure;
#define EKIT_WRONG_DEVICE         11 ///< Wrong device specified.
#define EKIT_COMMAND_FAILED       12 ///< Command failure.
#define EKIT_DEVICE_BUSY          13 ///< Device busy.
#define EKIT_NOT_COMPLETE         14 ///< Operation not completed.
#define EKIT_TIMEOUT              15 ///< Timeout expired.
#define EKIT_SUSPENDED            16 ///< Bus suspended.
#define EKIT_NOT_SUSPENDED        17 ///< Bus not suspended.
#define EKIT_NOT_OPENED           18 ///< Bus is not opened.
#define EKIT_WRITE_FAILED         19 ///< Write failed.
#define EKIT_BAD_PARAM            20 ///< Bad parameter or argument.
#define EKIT_LOCKED               21 ///< Bus is locked.
#define EKIT_UNLOCKED             22 ///< Bus is unlocked.
#define EKIT_NOT_SUPPORTED        23 ///< Operation is not supported.
#define EKIT_OVERFLOW             24 ///< Buffer overflow occurred.
#define EKIT_OUT_OF_RANGE         25 ///< Index out of range.
#define EKIT_PROTOCOL             26 ///< Protocol error.
#define EKIT_PARITY               27 ///< Parity error.
#define EKIT_COLLISION            28 ///< Collision detected.
#define EKIT_UNALIGNED            29 ///< Data is not aligned.

/// \brief Translates EKIT_ERROR to string for error message formatting purposes.
/// \param err - error code
/// \return error code text representation.
const char* errname(const EKIT_ERROR err);

/// \class EKitException
/// \brief Exception class used to deliver errors.
class EKitException : public std::runtime_error {

	/// \brief Helper function that formats message
	/// \param func_info - name of the function where message is thrown from.
	/// \param e - error code
	/// \param description - text description
    std::string format_exception(const std::string& func_info, EKIT_ERROR e, const std::string& description);
public:

    /// \brief Assignment is forbidden
    EKitException& operator=(const EKitException&) = delete;

    /// \brief Constructor to create exception by name of the function and error code.
    /// \param func_info - name of the function exception is being thrown from.
    /// \param e - error code.
	EKitException(const std::string& func_info, EKIT_ERROR e);


    /// \brief Constructor to create exception by name of the function, error code and description.
    /// \param func_info - name of the function exception is being thrown from.
    /// \param e - error code.
    /// \param description - error text description.
    EKitException(const std::string& func_info, EKIT_ERROR e, const std::string& description);

    EKIT_ERROR ekit_error;

    /// \brief Destructor (virtual)
	~EKitException() override;
};

/// @}
/// @}
