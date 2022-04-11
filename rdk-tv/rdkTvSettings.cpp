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

/**
 * @file rdkTvsettings.cpp
 * @brief This file contains implementation of TV settings class methods,
 * variable assignments and support functions to manage the TV setting types.
 */

/**
 @defgroup TV TVSettings Manager
 @{
 @defgroup rdktvsettings
 @{
 **/

 

#include "rdkTvSettings.hpp"
#include "tvError.h"
#include "tvLog.h"

#include "Module.h"
#include <WPEFramework/core/core.h>
#include <WPEFramework/websocket/websocket.h>
#include <WPEFramework/securityagent/SecurityTokenUtil.h>


#define MAX_LENGTH 1024

using namespace std;
using namespace WPEFramework;

JSONRPC::LinkType<Core::JSON::IElement>* remoteObject = NULL;
JSONRPC::LinkType<Core::JSON::IElement>* remoteObjectV2 = NULL;

unsigned char buffer[MAX_LENGTH] = {0};

/**
 * @fn TVSettings::TVSettings()
 * @brief This function provides the information on TvMgrClient Initialization.
 *
 * @param[in] none
 *
 * @return
 **/

TVSettings::TVSettings() {
  INFO("<<<<< Tvmgr Thunder Plugin used >>>>>>>>\r\n");

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));

    int ret = GetSecurityToken(MAX_LENGTH,buffer);
    if(ret > 0)
    {
        string sToken = (char*)buffer;
        string query = "token=" + sToken;
        //ßprintf(" token %s \n",query.c_str());

        if(NULL == remoteObject)
            remoteObject = new JSONRPC::LinkType<Core::JSON::IElement>(_T("org.rdk.tv.ControlSettings.1"), _T(""),false,query);
        else
            ERROR("TVSettings: %s Null remote object \n",__func__);
        
        if(NULL == remoteObjectV2)
            remoteObjectV2 = new JSONRPC::LinkType<Core::JSON::IElement>(_T("org.rdk.tv.ControlSettings.2"), _T(""),false,query);
        else
            ERROR("TVSettings: %s Null remote object V2 \n",__func__);
    }
    else
        ERROR("TVSettings: %s failed to retrieve security token %d \n",__func__,ret);
}

/**
 * @fn TVSettings::~TVSettings()
 * @brief This function provides the information on TvMgrClient Termination
 *
 * @param[in] none
 *
 * @return
 **/

TVSettings::~TVSettings() {
    INFO("TvMgrClient Terminating\n");
    JsonObject result;
    JsonObject param;
    string value;

    if(NULL != remoteObject) {
        delete remoteObject;
        remoteObject = NULL;
    }
    if(NULL != remoteObjectV2) {
    delete remoteObjectV2;
    remoteObjectV2 = NULL;
}
}

/**
 * @fn TVSettings& TVSettings::getInstance()
 * @brief This function gets the instance of TVSettings
 *
 * @param[in] none
 *
 * @return eturns an instance of TVSettings
 */

TVSettings& TVSettings::getInstance() {
    static TVSettings _instance;
    return _instance;
}

/**
 * @fn tvPictureMode_t TVSettings::getPictureMode()
 * @brief This function gets the picture mode of TV for ex:STANDARD,GAME,SPORTS, * etc.,
 *
 * @param[in] none
 *
 * @return Returns the mode of the picture or throws an error indicating failure during retriving picture mode
 */

int TVSettings::getPictureMode(std::string &mode){

    if(NULL == remoteObjectV2) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("getPictureMode"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("pictureMode") ? result["pictureMode"].String() : "";

        if(!value.empty())
            mode = value;
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }

    return 0;
}

 /**
  * @fn TVSettings::getBacklight()
  * @brief This function gets the Back light of TV which value is in the range
  * of 0-100
  *
  * @param[in] none
  *
  * @return Returns the back light  or throws an error indicating failure during retriving back light
  */

int TVSettings::getBacklight() {
    int backlight = 0;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getBacklight"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("backlight") ? result["backlight"].String() : "";

        if(!value.empty())
           backlight = stoi(value);
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }

    return backlight;
}

/**
 * @fn TVSettings::getBrightness()
 * @brief This function gets the brightness of TV which is in the range of 0-100
 *
 * @param[in] none
 *
 * @return Returns the brightness of TV or throws an error indicating failure during retriving brightness
 */

int TVSettings::getBrightness() {
    int brightness = 0;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getBrightness"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("brightness") ? result["brightness"].String() : "";

        if(!value.empty())
           brightness = stoi(value);

    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return brightness;
}

/**
 * @fn TVSettings::getContrast()
 * @brief This function provides gets the contrast of TV which is in the range
 * of 0-100
 *
 * @param[in] none
 *
 * @return Returns the contrast of TV or throws an error indicating failure during retriving contrast
 */

int TVSettings::getContrast() {
    int contrast = 0;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getContrast"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("contrast") ? result["contrast"].String() : "";

        if(!value.empty())
           contrast  = stoi(value);

    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return contrast;
}

/**
 * @fn TVSettings::getSaturation()
 * @brief This function provides gets the saturation of TV which is in the range *  of 0-100
 *
 * @param[in] none
 *
 * @return Returns the saturation of TV  or throws an error indicating failure during retriving saturation
 */

int TVSettings::getSaturation() {
    int saturation = 0;
    if(NULL == remoteObject)  {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getSaturation"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("saturation") ? result["saturation"].String() : "";

        if(!value.empty())
           saturation = stoi(value);
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return saturation;
}

/**
 * @fn TVSettings::getSharpness()
 * @brief This function provides gets the sharpness of Tv which is in the range  *  of 0-100
 *
 * @param[in] none
 *
 * @return Returns the sharpness of TV or throws an error indicating failure during retriving sharpness
 */

int TVSettings::getSharpness() {
    int sharpness = 0;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getSharpness"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("sharpness") ? result["sharpness"].String() : "";

        if(!value.empty())
           sharpness = stoi(value);
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return sharpness;
}

/**
 * @fn TVSettings::getHue()
 * @brief This function provides gets the hue of TV which is in the range of
 * 0-100
 *
 * @param[in] none
 *
 * @return Returns the hue of TV  or throws an error indicating failure during retriving hue
 */

int TVSettings::getHue() {
    int hue = 0;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getHue"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("hue") ? result["hue"].String() : "";

        if(!value.empty())
           hue = stoi(value);
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return hue;
}

/**
 * @fn  tvColorTemp_t TVSettings::getColorTemperature()
 * @brief This function provides gets the temperature of the color for ex: COLD, * WARM,STANDARD,USER,MAX
 *
 * @param[in] none
 *
 * @return Returns the color temperature  or throws an error indicating failure during retriving color temperature
 */

tvColorTemp_t TVSettings::getColorTemperature() {
    tvColorTemp_t colorTemp = tvColorTemp_MAX;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return tvColorTemp_MAX;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getColorTemperature"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("colorTemp") ? result["colorTemp"].String() : "";

        if(!value.compare("Standard")) {
            colorTemp = tvColorTemp_STANDARD;
        }
        else if (!value.compare("Warm")){
            colorTemp = tvColorTemp_WARM;
        }
        else if (!value.compare("Cold")){
            colorTemp = tvColorTemp_COLD;
        }
        else if (!value.compare("User")){
            colorTemp = tvColorTemp_USER;
        }
        else {
            colorTemp = tvColorTemp_STANDARD;
        }
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return colorTemp;
}

/**
 * @fn tvDisplayMode_t TVSettings::getAspectRatio()
 * @brief This function provides gets the aspect ratio of TV for ex: FULL,NORMAL * ,4x3,16x9,MAX
 *
 * @param[in] none
 *
 * @return Returns the aspect ratio of TV  or throws an error indicating failure during retriving aspect ratio
 */

tvDisplayMode_t TVSettings::getAspectRatio() {
    tvDisplayMode_t displayMode = tvDisplayMode_MAX;
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return tvDisplayMode_MAX;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("getAspectRatio"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("aspectRatio") ? result["aspectRatio"].String() : "";

        if(!value.empty())
           displayMode = tvDisplayMode_NORMAL;
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }
    return displayMode;
}

/**
 * @fn TVSettings::setPictureMode
 * @brief This function provides sets the picture mode of TV for ex: STANDARD,
 * GAME,SPORTS etc.,
 *
 * @param[in] tvPictureMode_t mode indicates the mode of the picture to be set
 *
 * @return Returns the set status  or throws an error indicating failure during setting the mode
 */

int TVSettings::setPictureMode(std::string mode) {

    if(NULL == remoteObjectV2) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }
    JsonObject result;
    JsonObject param;
    string value1;
    param.Set(_T("pictureMode"),mode);

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("setPictureMode"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value1.c_str());
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setBacklight
 * @brief This function provides sets the back light of TV which is in the range * of 0-100
 *
 * @param[in] value  indicates the back light to be set
 *
 * @return Returns the set status or throws an error indicating failure during setting the back light
 */

int TVSettings::setBacklight(const int value) {
    if(NULL == remoteObject) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    if((value < 0) || (value > 100)) {
        ERROR("TvMgrClient: %s Invalid Param\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value1;

    param.Set(_T("backlight"),to_string(value));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setBacklight"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value1.c_str());
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setBrightness
 * @brief This function provides sets the brightness of TV which is in the range * of 0-100
 *
 * @param[in] value indicates the brightness to be set
 *
 * @return Returns the set status  or throws an error indicating failure during setting the brightness
 */

int TVSettings::setBrightness(const int value) {
    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    if((value < 0) || (value > 100)) {
        ERROR("TvMgrClient: %s Invalid Param\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value1;

    param.Set(_T("brightness"),to_string(value));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setBrightness"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value1.c_str());
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setContrast
 * @brief This function provides sets the contrast of TV which is in the range
 * of 0-100
 *
 * @param[in] value  indicates the contrast to be set
 *
 * @return Returns the set status or throws an error indicating failure during setting the contrast
 */

int TVSettings::setContrast(const int value) {
    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    if((value < 0) || (value > 100)) {
        ERROR("TvMgrClient: %s Invalid Param\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value1;

    param.Set(_T("contrast"),to_string(value));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setContrast"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value1.c_str());
        return -1;
    }

    return 0;
}

/**
 * @fn TVSettings::setSaturation
 * @brief This function sets the saturation of TV which is in the range of 0-100
 *
 * @param[in] value  indicates the saturation  to be set
 *
 * @return Returns the set status  or throws an error indicating failure during setting the saturation of Tv
 * */

int TVSettings::setSaturation(const int value) {
    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    if((value < 0) || (value > 100)) {
        ERROR("TvMgrClient: %s Invalid Param\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value1;

    param.Set(_T("saturation"),to_string(value));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setSaturation"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: \n",__FUNCTION__, ret);
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setSharpness
 * @brief This function sets the sharpness of TV which is in the range of 0-100
 *
 * @param[in] value indicates the sharpness  to be set
 *
 * @return Returns the set status or throws an error indicating failure during setting the sharpness
 * */

int TVSettings::setSharpness(const int value) {
    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    if((value < 0) || (value > 100)) {
        ERROR("TvMgrClient: %s Invalid Param\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value1;

    param.Set(_T("sharpness"),to_string(value));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setSharpness"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: \n",__FUNCTION__, ret);
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setHue
 * @brief This function sets the Hue which is in the range of 0-100
 *
 * @param[in] value indicates the hue to be set
 *
 * @return Returns the set status or throws an error indicating failure during setting the hue
 * */

int TVSettings::setHue(const int value) {
    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    if((value < 0) || (value > 100)) {
        ERROR("TvMgrClient: %s Invalid Param\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value1;

    param.Set(_T("hue"),to_string(value));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setHue"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: \n",__FUNCTION__, ret);
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setColorTemperature
 * @brief This function sets the color temperature of TV for ex: COLD,WARM,
 * STANDARD,USER,MAX
 *
 * @param[in]tvColorTemp_t ct indicates the color temperature to be set
 *
 * @return Returns the set status or throws an error indicating failure during setting the temperature of color
 * */

int TVSettings::setColorTemperature(const tvColorTemp_t ct) {
    int param_err = 0;

    JsonObject result;
    JsonObject param;
    string value;

    switch(ct) {
        case tvColorTemp_STANDARD:
            param.Set(_T("colorTemp"),"Standard");
            break;
        case tvColorTemp_WARM:
            param.Set(_T("colorTemp"),"Warm");
            break;
        case tvColorTemp_COLD:
            param.Set(_T("colorTemp"),"Cold");
            break;
        case tvColorTemp_USER:
            param.Set(_T("colorTemp"),"User");
            break;
        case tvColorTemp_MAX:
            param.Set(_T("colorTemp"),"Standard");
            break;

       default:
           ERROR("TvMgrClient: %s Invalid Param\n",__func__);
           param_err = 1;
    }

    if(param_err == 1) {
        return -1;
    }

    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setColorTemperature"), param, result);
    if (!result["success"].Boolean()) {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
        return -1;
    }
    return 0;
}

/**
 * @fn TVSettings::setAspectRatio
 * @brief This function sets the aspect ratio of TV for ex: FULL,NORMAL,4x3,
 * 16x9,MAX
 * @param[in]tvDisplayMode_t mode indicates the display mode to be set
 *
 * @return Returns the set status or throws an error indicating failure during setting the aspect ratio
 * */

int TVSettings::setAspectRatio(const tvDisplayMode_t mode) {
    tvDisplayMode_t dispMode;
    int param_err = 0;

    switch(mode) {
        case tvDisplayMode_4x3:
        case tvDisplayMode_16x9:
        case tvDisplayMode_FULL:
        case tvDisplayMode_NORMAL:
        case tvDisplayMode_MAX:
            dispMode = mode;
            break;

       default:
           param_err = 1;
           ERROR("TvMgrClient: %s Invalid Param\n",__func__);
    }

    if(param_err == 1) {
        return -1;
    }

    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value;

    param.Set(_T("aspectRatio"),to_string((int)dispMode));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setAspectRatio"), param, result);
    if (!result["success"].Boolean()) {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
        return -1;
    }
    return 0;
}


/* Enable/disable particular wakeup source in order to wakeup from  deepsleep
 * param[in]: src_type, wakeup source type value could be VOICE, PRESENCE, IR etc
 * param[in]: value TRUE for enable FALSE for disable
 */
int TVSettings::setWakeupConfig(const tvWakeupSrcType_t src_type, const bool value) {
    tvWakeupSrcType_t wakeupSrc;
    bool config = 0;
    int param_err = 0;

    switch(src_type) {
        case tvWakeupSrc_VOICE:
        case tvWakeupSrc_PRESENCE_DETECTION:
        case tvWakeupSrc_BLUETOOTH:
        case tvWakeupSrc_WIFI:
        case tvWakeupSrc_IR:
        case tvWakeupSrc_POWER_KEY:
        case tvWakeupSrc_TIMER:
        case tvWakeupSrc_CEC:
        case tvWakeupSrc_LAN:
        case tvWakeupSrc_MAX:
            wakeupSrc = src_type;
            config = value;
            break;

       default:
           param_err = 1;
           ERROR("TvMgrClient: %s Invalid Param\n",__func__);
    }

    if(param_err == 1) {
        return -1;
    }

    if(NULL == remoteObject) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string svalue;

    param.Set(_T("wakeupSrc"),to_string((int)wakeupSrc));
    param.Set(_T("config"),to_string((int)config));
    uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000, _T("setWakeupConfiguration"), param, result);
    if (!result["success"].Boolean()) {
        svalue = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,svalue.c_str());
        return -1;
    }

    return 0;
}

int TVSettings::getSupportedPictureModes(std::vector<string> &picModes){
    if(NULL == remoteObjectV2) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }
    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("getSupportedPictureModes"), param, result);
    if (result["success"].Boolean()) {
        JsonArray modes;
        if(result.HasLabel("SupportedPicmodes"))
        {
            modes=result["SupportedPicmodes"].Array();
        }     

        for(int i=0;i<modes.Length();i++)
        {
            picModes.push_back(modes[i].String());
        }
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
    }

    return 0;       
}


int TVSettings::getSupportedDVModes(std::vector<string> &picModes){
    if(NULL == remoteObjectV2) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }
    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("getSupportedDolbyVisionModes"), param, result);
    if (result["success"].Boolean()) {
        JsonArray modes;
        if(result.HasLabel("SupportedDVModes"))
        {
            modes=result["SupportedDVModes"].Array();
        }     

        for(int i=0;i<modes.Length();i++)
        {
            picModes.push_back(modes[i].String());
        }
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
        return -1;
    }

    return 0;       
}

int TVSettings::getDVMode(std::string &mode){

    if(NULL == remoteObjectV2) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("getDolbyVisionMode"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("DolbyVisionMode") ? result["DolbyVisionMode"].String() : "";

        if(!value.empty())
            mode = value;
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
        return -1;
    }

    return 0;
}

int TVSettings::setDVMode(std::string mode) {
    if(NULL == remoteObjectV2) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }
    JsonObject result;
    JsonObject param;
    string value1;
    param.Set(_T("DolbyVisionMode"),mode);

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("setDolbyVisionMode"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value1.c_str());
        return -1;
    }
    return 0;
}

int TVSettings::getDCMode(std::string &mode)
{
    if(NULL == remoteObjectV2) {
        ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }

    JsonObject result;
    JsonObject param;
    string value = "";

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("getDynamicContrast"), param, result);
    if (result["success"].Boolean()) {
        value = result.HasLabel("DynamicContrast") ? result["DynamicContrast"].String() : "";

        if(!value.empty())
            mode = value;
    }
    else {
        value = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value.c_str());
        return -1;
    }

    return 0;
}

int TVSettings::setDCMode(std::string mode)
{
    if(NULL == remoteObjectV2) {
         ERROR("TvMgrClient: %s Invalid State\n",__func__);
        return -1;
    }
    JsonObject result;
    JsonObject param;
    string value1;
    param.Set(_T("DynamicContrast"),mode);

    uint32_t ret = remoteObjectV2->Invoke<JsonObject, JsonObject>(1000, _T("setDynamicContrast"), param, result);
    if (!result["success"].Boolean()) {
        value1 = result["error_message"].String();
        ERROR("%s failed!!! Plugin returned: %d Response: %s\n",__FUNCTION__, ret,value1.c_str());
        return -1;
    }

    return 0;
}
