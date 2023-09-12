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
 *   \brief PaceMakerDev device software implementation
 *   \author Oleh Sharuda
 */

#include "pacemakerdev_common.hpp"
#include "pacemakerdev.hpp"
#include "ekit_firmware.hpp"
#include <math.h>

PaceMakerDev::PaceMakerDev(std::shared_ptr<EKitBus>& ebus, const PaceMakerDevConfig* cfg) :
    super(ebus, cfg->dev_id, cfg->dev_name),
    config(cfg) {
}

PaceMakerDev::~PaceMakerDev() {
}
