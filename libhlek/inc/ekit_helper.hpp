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
 *   \brief EKit helper utilities
 *   \author Oleh Sharuda
 */

#pragma once
#include "ekit_error.hpp"

namespace EkitHelper {

    /// \class ioctl_request
    /// \brief This is a template wrapper around ioctl call, to provide handling EINTR and other specific error codes.
    /// \tparam P - type of the value to pass into ioctl
    /// \return EKIT_ERROR error code.
    template<class P>
    EKIT_ERROR ioctl_request(int fd, unsigned long int request, P param) {
        int err, ern;
        do {
            err = ioctl(fd, request, param);
            ern = errno;
        } while ((err < 0) && (ern == EINTR || ern == EAGAIN || ern == EWOULDBLOCK));

        return (err >= 0) ? EKIT_OK : EKIT_IOCTL_FAILED;
    }
}