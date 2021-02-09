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
 *   \brief Generated include header of firmware part for interfacing desk device
 *   \author Oleh Sharuda
 *   \warning This is generated file. All changes made may be overwritten by the following code generation. It is not
 *            intended for editing. In order to fix issues, corresponding template file should be changed.
 */

#pragma once
#define DESKDEV_DEVICE_ENABLED 1

/// \addtogroup group_desk_dev
/// @{{

{__DESKDEV_SHARED_HEADER__}

#define BUTTON_UP_PORT         {__BUTTON_UP_PORT__}
#define BUTTON_UP_PIN          {__BUTTON_UP_PIN__}
#define BUTTON_UP_PIN_MASK     {__BUTTON_UP_PIN_MASK__}
#define BUTTON_UP_EXTICR       {__BUTTON_UP_EXTICR__}

#define BUTTON_DOWN_PORT         {__BUTTON_DOWN_PORT__}
#define BUTTON_DOWN_PIN          {__BUTTON_DOWN_PIN__}
#define BUTTON_DOWN_PIN_MASK     {__BUTTON_DOWN_PIN_MASK__}
#define BUTTON_DOWN_EXTICR       {__BUTTON_DOWN_EXTICR__}

#define BUTTON_LEFT_PORT         {__BUTTON_LEFT_PORT__}
#define BUTTON_LEFT_PIN          {__BUTTON_LEFT_PIN__}
#define BUTTON_LEFT_PIN_MASK     {__BUTTON_LEFT_PIN_MASK__}
#define BUTTON_LEFT_EXTICR       {__BUTTON_LEFT_EXTICR__}

#define BUTTON_RIGHT_PORT         {__BUTTON_RIGHT_PORT__}
#define BUTTON_RIGHT_PIN          {__BUTTON_RIGHT_PIN__}
#define BUTTON_RIGHT_PIN_MASK     {__BUTTON_RIGHT_PIN_MASK__}
#define BUTTON_RIGHT_EXTICR       {__BUTTON_RIGHT_EXTICR__}

#define ENCODER_A_PORT            {__ENCODER_A_PORT__}
#define ENCODER_A_PIN_MASK        {__ENCODER_A_PIN_MASK__}
#define ENCODER_A_PIN             {__ENCODER_A_PIN__}
#define ENCODER_A_EXTICR          {__ENCODER_A_EXTICR__}

#define ENCODER_B_PORT            {__ENCODER_B_PORT__}
#define ENCODER_B_PIN_MASK        {__ENCODER_B_PIN_MASK__}
#define ENCODER_B_PIN             {__ENCODER_B_PIN__}
#define ENCODER_B_EXTICR          {__ENCODER_B_EXTICR__}

/// @}}
