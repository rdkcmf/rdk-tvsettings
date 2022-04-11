/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2022 Sky UK
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef _RDK_TV_SETTINGS_H
#define _RDK_TV_SETTINGS_H

#include <iostream>
#include <string>
#include <vector>

#include "tvError.h"
#include "tvTypes.h"


class TVSettings {
public:
    TVSettings();
    ~TVSettings();
    static TVSettings& getInstance(void);

    int getPictureMode(std::string &mode);
    int getBacklight();
    int getBrightness();
    int getContrast();
    int getSaturation();
    int getSharpness();
    int getHue();
    tvColorTemp_t getColorTemperature();
    tvDisplayMode_t getAspectRatio();

    int setPictureMode(std::string mode);
    int setBacklight(const int value);
    int setBrightness(const int value);
    int setContrast(const int value);
    int setSaturation(const int value);
    int setSharpness(const int value);
    int setHue(const int value);
    int setColorTemperature(const tvColorTemp_t ct);
    int setAspectRatio(const tvDisplayMode_t mode);

    int setWakeupConfig(const tvWakeupSrcType_t src_type, const bool value);
    int getSupportedPictureModes(std::vector<std::string> &picModes);

    int getSupportedDVModes(std::vector<std::string> &dvModes);
    int getDVMode(std::string &mode);
    int setDVMode(std::string mode);

    int getDCMode(std::string &mode);
    int setDCMode(std::string mode);

private:

};


#endif //_RDK_TV_SETTINGS_H
