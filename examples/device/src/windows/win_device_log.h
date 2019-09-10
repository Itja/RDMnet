/******************************************************************************
 * Copyright 2019 ETC Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************
 * This file is a part of RDMnet. For more information, go to:
 * https://github.com/ETCLabs/RDMnet
 *****************************************************************************/

#ifndef _WIN_DEVICE_LOG_H_
#define _WIN_DEVICE_LOG_H_

#include "etcpal/bool.h"
#include "etcpal/log.h"

void device_log_init(const char* file_name);
const EtcPalLogParams* device_get_log_params();
void device_log_deinit();

#endif /* _WIN_DEVICE_LOG_H_ */
