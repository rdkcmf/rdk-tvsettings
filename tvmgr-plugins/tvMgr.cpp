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

#include <string>
#include "libIARM.h"
#include "libIBusDaemon.h"
#include "libIBus.h"
#include "iarmUtil.h"

#include "dsError.h"
#include "dsMgr.h"
#include "hdmiIn.hpp"

#include "tvMgr.h"
#include "bl_table.h"
#include "als_bl_iniparser.h"
#include <iostream>

#include "tvTypes.h"
#include "tvLog.h"
#include "tvSettings.h"

#include "tr181api.h"
#include <sys/stat.h>
#include <vector>


#define PLUGIN_Lock(lock) pthread_mutex_lock(&lock)
#define PLUGIN_Unlock(lock) pthread_mutex_unlock(&lock)
#define SAVE_FOR_ALL_SOURCES    (-1)
#define BUFFER_SIZE     (128)
#define VIDEO_DESCRIPTION_MAX (25)
#define VIDEO_DESCRIPTION_NAME_SIZE (25)

static pthread_mutex_t tvLock = PTHREAD_MUTEX_INITIALIZER;
static int numberModesSupported=0;
int pic_mode_index[PIC_MODES_SUPPORTED_MAX]={ };
static char videoDescBuffer[VIDEO_DESCRIPTION_MAX*VIDEO_DESCRIPTION_NAME_SIZE] = {0};

#define returnResponse(return_status, error_log) \
    {response["success"] = return_status; \
    if(!return_status) \
        response["error_message"] = _T(error_log); \
    PLUGIN_Unlock(tvLock); \
    return (Core::ERROR_NONE);}

#define returnIfParamNotFound(param)\
    if(param.empty())\
    {\
        LOGERR("missing parameter %s\n",#param);\
        returnResponse(false,"missing parameter");\
    }

#define IARM_CHECK(FUNC) { \
    if ((res = FUNC) != IARM_RESULT_SUCCESS) { \
        LOGINFO("IARM %s: %s", #FUNC, \
            res == IARM_RESULT_INVALID_PARAM ? "invalid param" : ( \
            res == IARM_RESULT_INVALID_STATE ? "invalid state" : ( \
            res == IARM_RESULT_IPCCORE_FAIL ? "ipcore fail" : ( \
            res == IARM_RESULT_OOM ? "oom" : "unknown")))); \
    } \
    else \
    { \
        LOGINFO("IARM %s: success", #FUNC); \
    } \
}



#define TVSETTINGS_RFC_CALLERID        "tvsettings"
#define TVSETTINGS_PICTUREMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.PictureMode"
#define TVSETTINGS_ASPECTRATIO_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.AspectRatio"
#define TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM  "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.AutoBacklightMode"
#define TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.DolbyVisionMode"
#define TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.PictureModeString"
#define TVSETTINGS_DYNAMICCONTRASTMODE_STRING_RFC_PARAM    "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.DynamicContrastModeString"
#define TVSETTINGS_GENERIC_STRING_RFC_PARAM    "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings."
#define TVSETTINGS_DIMMING_MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.DimmingMode"
#define TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.SDR.Backlight"
#define TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HDR.Backlight"
#define TVSETTINGS_BACKLIGHT_FACTOR_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.BacklightFactor"
#define TVSETTINGS_BACKLIGHT_FACTOR_MID_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.BacklightFactorRangeMid"
#define TVSETTINGS_SAVE_FOR_ALL_CONTENT_FORMAT_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.PQSaveAcrossFormats"
#define TVSETTINGS_RESET_FOR_ALL_CONTENT_FORMAT_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.PQResetAcrossFormats"
#define TVSETTINGS_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.UseGBFForBacklightControl"
#define TVSETTINGS_WB_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.WB"
#define TVSETTINGS_HLGMODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HLGMode"
#define TVSETTINGS_HDR10MODE_RFC_PARAM      "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.TvSettings.HDR10Mode"

#define STRING_DIRTY  ".Dirty."
#define STRING_PICMODE  "PicMode."
#define STRING_FORMAT  "Format."
#define STRING_DEFAULT  "Default"
#define CREATE_DIRTY(__X__) (__X__+=STRING_DIRTY)

#define PIC_MODE_MAX 32
#define RFC_BUFF_MAX 100
#define GBF_MID_POINT_DEFAULT 100
static bool appUsesGlobalBackLightFactor = false;
const char* PLUGIN_IARM_BUS_NAME = "Thunder_Plugins";

char rfc_caller_id[RFC_BUFF_MAX]={0};

const char *component_color[] = {
    [COLOR_ENABLE] = "enable",
    [COLOR_RED] = "red",
    [COLOR_GREEN] = "green",
    [COLOR_BLUE] = "blue",
    [COLOR_CYAN] = "cyan",
    [COLOR_MAGENTA] = "magenta",
    [COLOR_YELLOW] = "yellow"
};

static JsonArray getSupportedVideoFormat(void)
{
    JsonArray supportedHdrFormat;
    tvError_t ret = tvERROR_NONE;
    char buffer[EXPECTED_FILE_SIZE] = {0};
    unsigned short totalVideoFormats = 0;

    ret = ReadAllModeConfigfile(GENERIC_MODE_CONFIG_FILE,  buffer, "videoformat");
    if(ret == tvERROR_NONE){
       splitstringsfrombuffer(buffer, videoDescBuffer, &totalVideoFormats);

        for(int index=0; index<totalVideoFormats; index++){
            supportedHdrFormat.Add(videoDescBuffer+(index*VIDEO_DESCRIPTION_NAME_SIZE));
        }
    }else{
        printf("%s : Reading supported video format failed %d", __FUNCTION__, ret);
    }

    return supportedHdrFormat;
}

static const char *getVideoFormatTypeToString(tvVideoHDRFormat_t format)
{
    const char *strValue = "NONE";
    switch(format) {
        case tvVideoHDRFormat_SDR:
            printf("Video Format: SDR\n");
            strValue = "SDR";
            break;
        case tvVideoHDRFormat_HDR10:
            printf("Video Format: HDR10\n");
            strValue = "HDR10";
            break;
        case tvVideoHDRFormat_HDR10PLUS:
            printf("Video Format: HDR10PLUS\n");
            strValue = "HDR10PLUS";
            break;
        case tvVideoHDRFormat_HLG:
            printf("Video Format: HLG\n");
            strValue = "HLG";
            break;
        case tvVideoHDRFormat_DV:
            printf("Video Format: DV\n");
            strValue = "DV";
            break;
        default:
            printf("Video Format:: NONE\n");
            strValue = "NONE";
            break;
    }
    return strValue;
}

static JsonArray getSupportedVideoResolution(void)
{
    JsonArray supportedResolution;
    tvError_t ret = tvERROR_NONE;
    char buffer[EXPECTED_FILE_SIZE] = {0};
    unsigned short totalVideoResolutions = 0;

    ret = ReadAllModeConfigfile(GENERIC_MODE_CONFIG_FILE,  buffer, "videoresolution");
    if(ret == tvERROR_NONE){
       splitstringsfrombuffer(buffer, videoDescBuffer, &totalVideoResolutions);

        for(int index=0; index < totalVideoResolutions; index++){
            supportedResolution.Add(videoDescBuffer+(index*VIDEO_DESCRIPTION_NAME_SIZE));
        }
    }else{
        printf("%s : Reading supported video resolutions failed %d", __FUNCTION__, ret);
    }

    return supportedResolution;
}

static std::string getVideoResolutionTypeToString(tvResolutionParam_t resolution)
{
    std::string strValue = "NONE";
    std::string interlaceValue = (resolution.isInterlaced) ? "i" : "p";
    if ( resolution.resolutionValue != tvVideoResolution_NONE ) {
        strValue = std::to_string(resolution.frameWidth) + "*" + std::to_string(resolution.frameHeight) + interlaceValue;
    }
    printf("Video Resolution:[%s]\n", strValue.c_str());
    return strValue;
}

static JsonArray getSupportedVideoFrameRate(void)
{
    JsonArray supportedFrameRate;
    tvError_t ret = tvERROR_NONE;
    char buffer[EXPECTED_FILE_SIZE] = {0};
    unsigned short totalFrameRates = 0;

    ret = ReadAllModeConfigfile(GENERIC_MODE_CONFIG_FILE,  buffer, "videoframerate");
    if(ret == tvERROR_NONE){
       splitstringsfrombuffer(buffer, videoDescBuffer, &totalFrameRates);

        for(int index=0; index<totalFrameRates; index++){
            supportedFrameRate.Add(videoDescBuffer+(index*VIDEO_DESCRIPTION_NAME_SIZE));
        }
    }else{
        printf("%s : Reading supported video frame rate failed %d", __FUNCTION__, ret);
    }

    return supportedFrameRate;
}

static const char *getVideoFrameRateTypeToString(tvVideoFrameRate_t frameRate)
{
    const char *strValue = "NONE";
    switch(frameRate) {
        case tvVideoFrameRate_24:
            printf("Video FrameRate: 24\n");
            strValue = "24";
            break;
        case tvVideoFrameRate_25:
            printf("Video FrameRate: 25\n");
            strValue = "25";
            break;
        case tvVideoFrameRate_30:
            printf("Video FrameRate: 30\n");
            strValue = "30";
            break;
        case tvVideoFrameRate_50:
            printf("Video FrameRate: 50\n");
            strValue = "50";
            break;
        case tvVideoFrameRate_60:
            printf("Video FrameRate: 60\n");
            strValue = "60";
            break;
        case tvVideoFrameRate_23dot98:
            printf("Video FrameRate: 23.98\n");
            strValue = "23.98";
            break;
        case tvVideoFrameRate_29dot97:
            printf("Video FrameRate: 29.97\n");
            strValue = "29.97";
            break;
        case tvVideoFrameRate_59dot94:
            printf("Video FrameRate: 59.94\n");
            strValue = "59.94";
            break;
        default:
            printf("Video FrameRate: NONE\n");
            strValue = "NONE";
            break;

    }
    return strValue;
}


namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(TVMgr,1, 0);
    TVMgr* TVMgr::_instance = nullptr;

    static void tvVideoFormatChangeHandler(tvVideoHDRFormat_t format, void *userData)
    {
        printf("tvVideoFormatChangeHandler format:%d \n",format);
        TVMgr *obj = (TVMgr *)userData;
        if(obj)obj->NotifyVideoFormatChange(format);
    }

    static void tvVideoResolutionChangeHandler(tvResolutionParam_t resolution, void *userData)
    {
        printf("tvVideoResolutionChangeHandler resolution:%d\n",resolution.resolutionValue);
        TVMgr *obj = (TVMgr *)userData;
        if(obj)obj->NotifyVideoResolutionChange(resolution);
    }

    static void tvVideoFrameRateChangeHandler(tvVideoFrameRate_t frameRate, void *userData)
    {
        printf("tvVideoFrameRateChangeHandler format:%d \n",frameRate);
        TVMgr *obj = (TVMgr *)userData;
        if(obj)obj->NotifyVideoFrameRateChange(frameRate);
    }

    TVMgr::TVMgr()
               : AbstractPlugin(3)
               , _skipURL(0)
               , m_currentHdmiInResoluton (dsVIDEO_PIXELRES_1920x1080)
               , m_videoZoomMode (tvDisplayMode_NORMAL)
               , m_isDisabledHdmiIn4KZoom (false)
    {
        LOGINFO();
        TVMgr::_instance = this;
        InitializeIARM();

        registerMethod("getBacklight", &TVMgr::getBacklight, this, {1});
        registerMethod("getBrightness", &TVMgr::getBrightness, this, {1});
        registerMethod("getContrast", &TVMgr::getContrast, this, {1});
        registerMethod("getSaturation", &TVMgr::getSaturation, this, {1});
        registerMethod("getSharpness", &TVMgr::getSharpness, this, {1});
        registerMethod("getHue", &TVMgr::getHue, this, {1});
        registerMethod("getColorTemperature", &TVMgr::getColorTemperature, this, {1});
        registerMethod("getAspectRatio", &TVMgr::getAspectRatio, this, {1});
        registerMethod("setBacklight", &TVMgr::setBacklight, this);
        registerMethod("setBrightness",&TVMgr::setBrightness, this);
        registerMethod("setContrast", &TVMgr::setContrast, this);
        registerMethod("setSaturation", &TVMgr::setSaturation, this);
        registerMethod("setSharpness", &TVMgr::setSharpness, this);
        registerMethod("setHue", &TVMgr::setHue, this);
        registerMethod("setColorTemperature", &TVMgr::setColorTemperature, this);
        registerMethod("setAspectRatio", &TVMgr::setAspectRatio, this, {1});
        registerMethod("setWakeupConfiguration", &TVMgr::setWakeupConfiguration, this);
        registerMethod("getWBInfo", &TVMgr::getWBInfo, this);
        registerMethod("setWBCtrl", &TVMgr::setWBCtrl, this);
        registerMethod("getWBCtrl", &TVMgr::getWBCtrl, this);
        registerMethod("enableWBMode", &TVMgr::enableWBMode, this);
        registerMethod("getVideoResolution", &TVMgr::getVideoResolution, this, {1});
        registerMethod("getVideoFrameRate", &TVMgr::getVideoFrameRate, this, {1});

        registerMethod("getBacklight", &TVMgr::getBacklight2, this, {2});
        registerMethod("getBrightness", &TVMgr::getBrightness2, this, {2});
        registerMethod("getContrast", &TVMgr::getContrast2, this, {2});
        registerMethod("getSaturation", &TVMgr::getSaturation2, this, {2});
        registerMethod("getSharpness", &TVMgr::getSharpness2, this, {2});
        registerMethod("getHue", &TVMgr::getHue2, this, {2});
        registerMethod("getColorTemperature", &TVMgr::getColorTemperature2, this, {2});
        registerMethod("getAspectRatio", &TVMgr::getAspectRatio2, this, {2});
        registerMethod("setAspectRatio", &TVMgr::setAspectRatio2, this, {2});
        registerMethod("getAutoBacklightControl", &TVMgr::getAutoBacklightControl, this, {2});
        registerMethod("setAutoBacklightControl", &TVMgr::setAutoBacklightControl, this, {2});
        registerMethod("getVideoFormat", &TVMgr::getVideoFormat, this, {2});
        registerMethod("getSupportedPictureModes", &TVMgr::getSupportedPictureModes, this, {2});
        registerMethod("getPictureMode", &TVMgr::getPictureMode, this, {2});
        registerMethod("setPictureMode", &TVMgr::setPictureMode, this, {2});

        registerMethod("getSupportedDolbyVisionModes", &TVMgr::getSupportedDolbyVisionModes, this, {2});
        registerMethod("getDolbyVisionMode", &TVMgr::getDolbyVisionMode, this, {2});
        registerMethod("setDolbyVisionMode", &TVMgr::setDolbyVisionMode, this, {2});

        registerMethod("setDynamicContrast", &TVMgr::setDynamicContrast, this, {2});
        registerMethod("getDynamicContrast", &TVMgr::getDynamicContrast, this, {2});

        registerMethod("getComponentColorInfo", &TVMgr::getComponentColorInfo, this, {2});
        registerMethod("setComponentSaturation", &TVMgr::setComponentSaturation, this, {2});
        registerMethod("getComponentSaturation", &TVMgr::getComponentSaturation, this, {2});
        registerMethod("setComponentHue", &TVMgr::setComponentHue, this, {2});
        registerMethod("getComponentHue", &TVMgr::getComponentHue, this, {2});
        registerMethod("setComponentLuma", &TVMgr::setComponentLuma, this, {2});
        registerMethod("getComponentLuma", &TVMgr::getComponentLuma, this, {2});
        registerMethod("getAllBacklightDimmingModes", &TVMgr::getAllBacklightDimmingModes, this, {2});
        registerMethod("getBacklightDimmingMode", &TVMgr::getBacklightDimmingMode, this, {2});
        registerMethod("setBacklightDimmingMode", &TVMgr::setBacklightDimmingMode, this, {2});
        registerMethod("setBacklightFade", &TVMgr::setBacklightFade, this, {3});

        registerMethod("resetBrightness", &TVMgr::resetBrightness, this, {2});
        registerMethod("resetSharpness", &TVMgr::resetSharpness, this, {2});
        registerMethod("resetSaturation", &TVMgr::resetSaturation, this, {2});
        registerMethod("resetHue", &TVMgr::resetHue, this, {2});
        registerMethod("resetBacklight", &TVMgr::resetBacklight, this, {2});
        registerMethod("resetColorTemperature", &TVMgr::resetColorTemperature, this, {2});
        registerMethod("resetDolbyVisionMode", &TVMgr::resetDolbyVisionMode, this, {2});
        registerMethod("resetAutoBacklight", &TVMgr::resetAutoBacklight, this, {2});
        registerMethod("resetContrast", &TVMgr::resetContrast, this, {2});
        registerMethod("resetAspectRatio", &TVMgr::resetAspectRatio, this, {2});
        registerMethod("resetComponentSaturation", &TVMgr::resetComponentSaturation, this, {2});
        registerMethod("resetComponentLuma", &TVMgr::resetComponentLuma, this, {2});
        registerMethod("resetComponentHue", &TVMgr::resetComponentHue, this, {2});
        registerMethod("resetBacklightDimmingMode", &TVMgr::resetBacklightDimmingMode, this, {2});

        registerMethod("resetWBCtrl", &TVMgr::resetWBCtrl, this, {2});
        registerMethod("resetPictureMode", &TVMgr::resetPictureMode, this, {2});
        registerMethod("getHLGMode", &TVMgr::getHLGMode, this, {2});
        registerMethod("setHLGMode", &TVMgr::setHLGMode, this, {2});
        registerMethod("getHDR10Mode", &TVMgr::getHDR10Mode, this, {2});
        registerMethod("setHDR10Mode", &TVMgr::setHDR10Mode, this, {2});
        registerMethod("resetHDR10Mode", &TVMgr::resetHDR10Mode, this, {2});
        registerMethod("resetHLGMode", &TVMgr::resetHLGMode, this, {2});
        registerMethod("getSupportedHLGModes", &TVMgr::getSupportedHLGModes, this, {2});
        registerMethod("getSupportedHDR10Modes", &TVMgr::getSupportedHDR10Modes, this, {2});
    }

    TVMgr::~TVMgr()
    {
        LOGINFO();
    }

    bool TVMgr::isIARMConnected() {
        IARM_Result_t res;
        int isRegistered = 0;
        res = IARM_Bus_IsConnected(PLUGIN_IARM_BUS_NAME, &isRegistered);
        LOGINFO("tvmgrplugin: IARM_Bus_IsConnected: %d (%d)", res, isRegistered);

        return (isRegistered == 1);
    }

    bool TVMgr::IARMinit() {
        IARM_Result_t res;
        bool result = false;

        if (isIARMConnected()) {
            LOGINFO("tvmgrplugin: IARM already connected");
            result = true;
        } else {
            res = IARM_Bus_Init(PLUGIN_IARM_BUS_NAME);
            LOGINFO("tvmgrplugin: IARM_Bus_Init: %d", res);
            if (res == IARM_RESULT_SUCCESS ||
                res == IARM_RESULT_INVALID_STATE /* already inited or connected */) {

                res = IARM_Bus_Connect();
                LOGINFO("tvmgrplugin: IARM_Bus_Connect: %d", res);
                if (res == IARM_RESULT_SUCCESS ||
                    res == IARM_RESULT_INVALID_STATE /* already connected or not inited */) {

                    result = isIARMConnected();
                } else {
                    LOGERR("tvmgrplugin: IARM_Bus_Connect failure: %d", res);
                }
            } else {
                LOGERR("tvmgrplugin: IARM_Bus_Init failure: %d", res);
            }
        }

        return result;
    }

    void TVMgr::dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!TVMgr::_instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_video_mode.port;
            dsVideoPortResolution_t resolution;
            TVMgr::_instance->m_currentHdmiInResoluton = eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.pixelResolution =  eventData->data.hdmi_in_video_mode.resolution.pixelResolution;
            resolution.interlaced =  eventData->data.hdmi_in_video_mode.resolution.interlaced;
            resolution.frameRate =  eventData->data.hdmi_in_video_mode.resolution.frameRate;
            LOGWARN("tvmgrplugin: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE  event  port: %d, pixelResolution: %d, interlaced : %d, frameRate: %d \n", hdmi_in_port,resolution.pixelResolution, resolution.interlaced, resolution.frameRate);
            if (TVMgr::_instance->m_isDisabledHdmiIn4KZoom) {
                tvError_t ret = tvERROR_NONE;
                if (TVMgr::_instance->m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                    (dsVIDEO_PIXELRES_MAX == TVMgr::_instance->m_currentHdmiInResoluton)){
                    LOGWARN("tvmgrplugin: Setting %d zoom mode for below 4K", TVMgr::_instance->m_videoZoomMode);
                    ret = SetAspectRatio((tvDisplayMode_t)TVMgr::_instance->m_videoZoomMode);
                }else {
                    LOGWARN("tvmgrplugin: Setting auto zoom mode for 4K and above");
                    ret = SetAspectRatio(tvDisplayMode_AUTO);
                }
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            } else {
                LOGWARN("tvmgrplugin: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, TVMgr::_instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }

    void TVMgr::dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
    {
        if(!TVMgr::_instance)
            return;

        if (IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS == eventId)
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_in_port = eventData->data.hdmi_in_status.port;
            bool hdmi_in_status = eventData->data.hdmi_in_status.isPresented;
            LOGWARN("tvmgrplugin: Received IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS  event  port: %d, started: %d", hdmi_in_port,hdmi_in_status);
            if (!hdmi_in_status){
                tvError_t ret = tvERROR_NONE;
                TVMgr::_instance->m_isDisabledHdmiIn4KZoom = false;
                LOGWARN("tvmgrplugin: Hdmi streaming stopped here reapply the global zoom settings:%d here. m_isDisabledHdmiIn4KZoom: %d", TVMgr::_instance->m_videoZoomMode, TVMgr::_instance->m_isDisabledHdmiIn4KZoom);
                ret = SetAspectRatio((tvDisplayMode_t)TVMgr::_instance->m_videoZoomMode);
                if (ret != tvERROR_NONE) {
                    LOGWARN("SetAspectRatio set Failed");
                }
            }else {
                TVMgr::_instance->m_isDisabledHdmiIn4KZoom = true;
                LOGWARN("tvmgrplugin: m_isDisabledHdmiIn4KZoom: %d", TVMgr::_instance->m_isDisabledHdmiIn4KZoom);
            }
        }
    }


    void TVMgr::InitializeIARM()
    {
#if !defined (HDMIIN_4K_ZOOM)
        if (IARMinit())
        {
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS, dsHdmiStatusEventHandler) );
            IARM_CHECK( IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE, dsHdmiVideoModeEventHandler) );
        }
#endif
    }

    void TVMgr::DeinitializeIARM()
    {
#if !defined (HDMIIN_4K_ZOOM)
        if (isIARMConnected())
        {
            IARM_Result_t res;
            IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS) );
            IARM_CHECK( IARM_Bus_UnRegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE) );
        }
#endif
    }

    tvError_t TVMgr::setAspectRatioZoomSettings(tvDisplayMode_t mode)
    {
        tvError_t ret = tvERROR_GENERAL;
	LOGERR("tvmgrplugin: %s mode selected is: %d", __FUNCTION__, m_videoZoomMode);
#if !defined (HDMIIN_4K_ZOOM)
        if (TVMgr::_instance->m_isDisabledHdmiIn4KZoom) {
            if (m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
                (dsVIDEO_PIXELRES_MAX == m_currentHdmiInResoluton)){
                LOGWARN("tvmgrplugin: %s: Setting %d zoom mode for below 4K", __FUNCTION__, m_videoZoomMode);
#endif
                ret = SetAspectRatio(mode);
#if !defined (HDMIIN_4K_ZOOM)
            }else {
                LOGWARN("tvmgrplugin: %s: Setting auto zoom mode for 4K and above", __FUNCTION__);
                ret = SetAspectRatio(tvDisplayMode_AUTO);
            }
        } else {
            LOGWARN("tvmgrplugin: %s: HdmiInput is not started yet. m_isDisabledHdmiIn4KZoom: %d", __FUNCTION__, TVMgr::_instance->m_isDisabledHdmiIn4KZoom);
            ret = SetAspectRatio((tvDisplayMode_t)m_videoZoomMode);
        }
#endif
        return ret;
    }

    tvError_t TVMgr::setDefaultAspectRatio(void)
    {
        tvDisplayMode_t mode = tvDisplayMode_MAX;
        TR181_ParamData_t param;
        tvError_t ret = tvERROR_NONE;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_ASPECTRATIO_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, param.value);

            if(!std::string(param.value).compare("16:9")) {
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("4:3")){
                mode = tvDisplayMode_4x3;
            }
            else if (!std::string(param.value).compare("Full")){
                mode = tvDisplayMode_FULL;
            }
            else if (!std::string(param.value).compare("Normal")){
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV AUTO")){
                mode = tvDisplayMode_AUTO;
            }
            else if (!std::string(param.value).compare("TV DIRECT")){
                mode = tvDisplayMode_DIRECT;
            }
            else if (!std::string(param.value).compare("TV NORMAL")){
                mode = tvDisplayMode_NORMAL;
            }
            else if (!std::string(param.value).compare("TV ZOOM")){
                mode = tvDisplayMode_ZOOM;
            }
            else if (!std::string(param.value).compare("TV 16X9 STRETCH")){
                mode = tvDisplayMode_16x9;
            }
            else if (!std::string(param.value).compare("TV 4X3 PILLARBOX")){
                mode = tvDisplayMode_4x3;
            }
            else {
                mode = tvDisplayMode_AUTO;
            }

            m_videoZoomMode = mode;
            tvError_t ret = setAspectRatioZoomSettings (mode);

            if(ret != tvERROR_NONE) {
                LOGWARN("AspectRatio  set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                //Save DisplayMode to ssm_data
                int params[3]={0};
                params[0]=mode;
                int retval=UpdatePQParamsToCache("set","aspectratio","AspectRatio",PQ_PARAM_ASPECT_RATIO,params);

                if(retval != 0) {
                    LOGWARN("Failed to Save DisplayMode to ssm_data\n");
                }
                LOGINFO("Aspect Ratio initialized successfully, value: %s\n", param.value);
            }

        }
        else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        return ret;
    }

    /* virtual */ const std::string TVMgr::Initialize(PluginHost::IShell* service)
    {
        LOGINFO();
        try {
            dsVideoPortResolution_t vidResolution;
            device::HdmiInput::getInstance().getCurrentVideoModeObj(vidResolution);
            m_currentHdmiInResoluton = vidResolution.pixelResolution;
        } catch (...){
            LOGWARN("tvmgrplugin: getCurrentVideoModeObj failed");
        }
        LOGWARN("tvmgrplugin: TVMgr Initialize m_currentHdmiInResoluton:%d m_mod:%d", m_currentHdmiInResoluton, m_videoZoomMode);

        ASSERT(service != nullptr);
        _skipURL = static_cast<uint8_t>(service->WebPrefix().length());

        tvError_t ret = tvERROR_NONE;

        ret = tvInit();

        if(ret != tvERROR_NONE) {
            LOGERR("Platform Init failed, ret: %s \n", getErrorString(ret).c_str());

        }
        else{
            LOGINFO("Platform Init successful...\n");
            ret = tvSD3toCriSyncInit();
            if(ret != tvERROR_NONE) {
                LOGERR(" SD3 <->cri_data sync failed, ret: %s \n", getErrorString(ret).c_str());
            }
            else {
                LOGERR(" SD3 <->cri_data sync success, ret: %s \n", getErrorString(ret).c_str());
            }

        }
        tvVideoFormatCallbackData callbackData = {this,tvVideoFormatChangeHandler};
        RegisterVideoFormatChangeCB(callbackData);

        tvVideoResolutionCallbackData RescallbackData = {this,tvVideoResolutionChangeHandler};
        RegisterVideoResolutionChangeCB(RescallbackData);

        tvVideoFrameRateCallbackData FpscallbackData = {this,tvVideoFrameRateChangeHandler};
        RegisterVideoFrameRateChangeCB(FpscallbackData);

        //Initialize all settings
        TR181_ParamData_t param;
        memset(&param, 0, sizeof(param));

        //Get number of pqmode supported
        numberModesSupported=GetNumberOfModesupported();

        //UpdatePicModeIndex
        GetAllSupportedPicModeIndex(pic_mode_index);

        //LocateDefaultPQSettingsFile
        LocatePQSettingsFile();
        appUsesGlobalBackLightFactor = isBacklightUsingGlobalBacklightFactor();

        SyncPQParamsToDriverCache();

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, param.value);
            ret = SetTVPictureMode(param.value);

            if(ret != tvERROR_NONE) {
                LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                LOGINFO("Picture Mode initialized successfully, value: %s\n", param.value);
            }
        }
        else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
        }


        //Initialize Backlight
        if(appUsesGlobalBackLightFactor)
        {
            if(!InitializeSDRHDRBacklight())
                LOGINFO("InitializeSDRHDRBacklight() : Success\n");
            else
                LOGWARN("InitializeSDRHDRBacklight() : Failed\n");
        }

        tvBacklightMode_t blMode = tvBacklightMode_NONE;
        memset(&param, 0, sizeof(param));
        err = getLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, param.value);

            if(!std::string(param.value).compare("none")) {
                blMode = tvBacklightMode_NONE;
            }
            else if (!std::string(param.value).compare("manual")){
                blMode = tvBacklightMode_MANUAL;
            }
            else if (!std::string(param.value).compare("ambient")){
                blMode = tvBacklightMode_AMBIENT;
            }
            else if (!std::string(param.value).compare("eco")){
                blMode = tvBacklightMode_ECO;
            }
            else {
                blMode = tvBacklightMode_NONE;
            }
            ret = SetCurrentBacklightMode(blMode);

            if(ret != tvERROR_NONE) {
                LOGWARN("Auto Backlight Mode set failed: %s\n",getErrorString(ret).c_str());
            }
            else {
                LOGINFO("Auto Backlight Mode initialized successfully, value: %s\n", param.value);
            }

        }
        else
        {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
        }

        setDefaultAspectRatio();

        LOGINFO("Initialize : Default tvsettings file : %s\n",rfc_caller_id);

        return (service != nullptr ? _T("") : _T("No service."));
    }


    /* virtual */ void TVMgr::Deinitialize(PluginHost::IShell* service)
    {
        tvError_t ret = tvERROR_NONE;
        ret = tvTerm();

        if(ret != tvERROR_NONE) {
            LOGERR("Platform De-Init failed, ret: %s \n", getErrorString(ret).c_str());
        }
        else{
            LOGINFO("Platform De-Init successful... \n");
        }
        TVMgr::_instance = nullptr;
        DeinitializeIARM();
    }


    /* virtual */ std::string TVMgr::Information() const
    {
        // No additional info to report.
        return (std::string());
    }

    uint32_t TVMgr::getPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        // tvPictureMode_t mode;
        char mode[PIC_MODE_NAME_MAX]={0};
        std::string picturemode;
        tvError_t ret = GetTVPictureMode(mode);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            std::string s;
            s+=mode;
            response["pictureMode"] = s;
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::setPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;

        value = parameters.HasLabel("pictureMode") ? parameters["pictureMode"].String() : "";
        returnIfParamNotFound(value);

        tvError_t ret = SetTVPictureMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : Value : %s \n",value.c_str());
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::getBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int backlight;
        tvError_t ret = tvERROR_NONE;

        if(appUsesGlobalBackLightFactor){
            ret = GetLastSetBacklightForGBF(backlight);
        }else{
            ret = GetBacklight(&backlight);
        }
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["backlight"] = std::to_string(backlight);
            LOGINFO("Backlight Value: %d\n", backlight);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getBacklight2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int backlight;
        JsonObject range;
        JsonObject backlightObj;
        range["From"] = 0;
        range["To"] = 100;
        backlightObj["Range"] = range;
        tvError_t ret = tvERROR_NONE;

        if(appUsesGlobalBackLightFactor){
            ret = GetLastSetBacklightForGBF(backlight);
        }else{
            ret = GetBacklight(&backlight);
        }
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            backlightObj["Setting"] = std::to_string(backlight);
            response["Backlight"] = backlightObj;
            LOGINFO("Exit : Backlight Value: %d\n", backlight);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        std::string value;
        int backlight = 0;
        tvError_t ret  = tvERROR_NONE;

        value = parameters.HasLabel("backlight") ? parameters["backlight"].String() : "";
        returnIfParamNotFound(value);
        backlight = std::stoi(value);

        if(appUsesGlobalBackLightFactor){
            int convertedBL = GetDriverEquivalentBLForCurrentFmt(backlight);// 100scale/255
            backlight = convertedBL;
        }
        ret = SetBacklight(backlight);
        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set backlight\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=backlight;
            int retval = UpdatePQParamsToCache("set","backlight","Backlight",PQ_PARAM_BACKLIGHT,params);
            if(retval != 0) {
                LOGWARN("Failed to Save Backlight to ssm_data\n");
            }
            if(appUsesGlobalBackLightFactor)
                saveBacklightToLocalStoreForGBF("Backlight",value.c_str());
            LOGINFO("Exit : setBacklight successful to value: %d\n",backlight);
            returnResponse(true, "success");
        }

    }


    uint32_t TVMgr::getAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        tvBacklightMode_t blMode;
        JsonArray supportedBLModes;
        int blModes = tvBacklightMode_NONE;
        tvError_t ret = GetSupportedBacklightModes(&blModes);
        if(ret != tvERROR_NONE) {
            supportedBLModes.Add("none");
        }
        else {
            if(!blModes)supportedBLModes.Add("none");
            if(blModes & tvBacklightMode_MANUAL)supportedBLModes.Add("manual");
            if(blModes & tvBacklightMode_AMBIENT)supportedBLModes.Add("ambient");
            if(blModes & tvBacklightMode_ECO)supportedBLModes.Add("eco");
        }

        JsonObject backlightCtrlObj;

        ret = GetCurrentBacklightMode(&blMode);
        if(ret != tvERROR_NONE) {
            response["mode"] = "none";
            response["supportedModes"] = supportedBLModes;
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            switch(blMode) {

                case tvBacklightMode_NONE:
                    LOGINFO("Auto Backlight Control Mode: none\n");
                    response["mode"] = "none";
                    break;

                case tvBacklightMode_MANUAL:
                    LOGINFO("Auto Backlight Control Mode: manual\n");
                    response["mode"] = "manual";
                    break;

                case tvBacklightMode_AMBIENT:
                    LOGINFO("Auto Backlight Control Mode: ambient\n");
                    response["mode"] = "ambient";
                    break;

                case tvBacklightMode_ECO:
                    LOGINFO("Auto Backlight Control Mode: eco\n");
                    response["mode"] = "eco";
                    break;

                default:
                    LOGINFO("Auto Backlight Control Mode: none\n");
                    response["mode"] = "none";
                    break;
            }

            response["supportedModes"] = supportedBLModes;
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::setAutoBacklightControl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvBacklightMode_t blMode = tvBacklightMode_NONE;

        value = parameters.HasLabel("mode") ? parameters["mode"].String() : "";
        returnIfParamNotFound(value);

        if(!value.compare("none")) {
            blMode = tvBacklightMode_NONE;
        }
        else if (!value.compare("manual")){
            blMode = tvBacklightMode_MANUAL;
        }
        else if (!value.compare("ambient")){
            blMode = tvBacklightMode_AMBIENT;
        }
        else if (!value.compare("eco")){
            blMode = tvBacklightMode_ECO;
        }
        else {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }


        tvError_t ret = SetCurrentBacklightMode(blMode);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : value :%s\n",value.c_str());
            returnResponse(true, "success");
        }

    }
    void TVMgr::NotifyVideoFormatChange(tvVideoHDRFormat_t format)
    {
        JsonObject response;
        response["currentVideoFormat"] = getVideoFormatTypeToString(format);
        response["supportedVideoFormat"] = getSupportedVideoFormat();
        sendNotify("videoFormatChanged", response);
    }

    uint32_t TVMgr::getVideoFormat(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        tvVideoHDRFormat_t videoFormat;
        tvError_t ret = GetCurrentVideoFormat(&videoFormat);
        response["supportedVideoFormat"] = getSupportedVideoFormat();
        if(ret != tvERROR_NONE) {
            response["currentVideoFormat"] = "NONE";
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["currentVideoFormat"] = getVideoFormatTypeToString(videoFormat);
            LOGINFO(" getVideoFormat :%d   success \n",videoFormat);
            returnResponse(true, "success");
        }
    }

    void TVMgr::NotifyVideoResolutionChange(tvResolutionParam_t resolution)
    {
        JsonObject response;
        response["currentVideoResolution"] = getVideoResolutionTypeToString(resolution);
        response["supportedVideoResolution"] = getSupportedVideoResolution();
        sendNotify("videoResolutionChanged", response);
    }

    uint32_t TVMgr::getVideoResolution(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        tvResolutionParam_t videoResolution;
        tvError_t ret = GetVideoResolution(&videoResolution);
        response["supportedVideoResolution"] = getSupportedVideoResolution();
        if(ret != tvERROR_NONE) {
            response["currentVideoResolution"] = "NONE";
            returnResponse(false, getErrorString(ret));
        }
        else {
            response["currentVideoResolution"] = getVideoResolutionTypeToString(videoResolution);
            LOGINFO(" getVideoResolution :%d   success \n",videoResolution.resolutionValue);
            returnResponse(true, "success");
        }
    }

    void TVMgr::NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate)
    {
        JsonObject response;
        response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(frameRate);
        response["supportedVideoFrameRate"] = getSupportedVideoFrameRate();
        sendNotify("videoFrameRateChanged", response);
    }

    uint32_t TVMgr::getVideoFrameRate(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        tvVideoFrameRate_t videoFramerate;
        tvError_t ret = GetVideoFrameRate(&videoFramerate);
        response["supportedFrameRate"] = getSupportedVideoFrameRate();
        if(ret != tvERROR_NONE) {
            response["currentVideoFrameRate"] = "NONE";
            returnResponse(false, getErrorString(ret));
        }
        else {
            response["currentVideoFrameRate"] = getVideoFrameRateTypeToString(videoFramerate);
            LOGINFO(" videoFramerate :%d   success \n",videoFramerate);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int brightness;

        tvError_t ret = GetBrightness(&brightness);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["brightness"] = std::to_string(brightness);
            LOGINFO("Exit : Brightness Value: %d\n", brightness);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getBrightness2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int brightness;
        JsonObject range;
        JsonObject brightnessObj;
        range["From"] = 0;
        range["To"] = 100;
        brightnessObj["Range"] = range;

        tvError_t ret = GetBrightness(&brightness);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            brightnessObj["Setting"] = std::to_string(brightness);
            response["Brightness"] = brightnessObj;
            LOGINFO("Exit : Brightness Value: %d\n", brightness);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        std::string value;
        int brightness = 0;

        value = parameters.HasLabel("brightness") ? parameters["brightness"].String() : "";
        returnIfParamNotFound(value);
        brightness = std::stoi(value);

        tvError_t ret = SetBrightness(brightness);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Brightness\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save Brightness to localstore and ssm_data
            int params[3]={0};
            params[0]=brightness;
            int retval=UpdatePQParamsToCache("set","brightness","Brightness",PQ_PARAM_BRIGHTNESS,params);
            if(retval != 0 ) {
                LOGWARN("Failed to Save Brightness to ssm_data\n");
            }
            LOGINFO("Exit : setBrightness successful to value: %d\n", brightness);
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::getContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        int contrast;
        tvError_t ret = GetContrast(&contrast);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["contrast"] = std::to_string(contrast);
            LOGINFO("Exit : Contrast Value: %d\n", contrast);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getContrast2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        int contrast;
        JsonObject range;
        JsonObject contrastObj;
        range["From"] = 0;
        range["To"] = 100;
        contrastObj["Range"] = range;

        tvError_t ret = GetContrast(&contrast);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            contrastObj["Setting"] = std::to_string(contrast);
            response["Contrast"] = contrastObj;
            LOGINFO("Exit : Contrast Value: %d\n", contrast);
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::setContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        std::string value;
        int contrast = 0;

        value = parameters.HasLabel("contrast") ? parameters["contrast"].String() : "";
        returnIfParamNotFound(value);
        contrast = std::stoi(value);

        tvError_t ret = SetContrast(contrast);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set contrast\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save Contrast to localstore and ssm_data
            int params[3]={0};
            params[0]=contrast;

            int retval=UpdatePQParamsToCache("set","contrast","Contrast",PQ_PARAM_CONTRAST,params);
            if(retval != 0) {
                LOGWARN("Failed to Save Contrast to ssm_data\n");
            }
            LOGINFO("Exit : setContrast successful to value: %d\n",contrast);
            returnResponse(true, "success");
        }

    }


    uint32_t TVMgr::getSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int sharpness;
        tvError_t ret = GetSharpness(&sharpness);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["sharpness"] = std::to_string(sharpness);
            LOGINFO("Exit : Sharpness Value: %d\n", sharpness);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getSharpness2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int sharpness;
        JsonObject range;
        JsonObject sharpnessObj;
        range["From"] = 0;
        range["To"] = 100;
        sharpnessObj["Range"] = range;

        tvError_t ret = GetSharpness(&sharpness);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            sharpnessObj["Setting"] = std::to_string(sharpness);
            response["Sharpness"] = sharpnessObj;
            LOGINFO("Exit : Sharpness Value: %d\n", sharpness);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        int sharpness = 0;

        value = parameters.HasLabel("sharpness") ? parameters["sharpness"].String() : "";
        returnIfParamNotFound(value);
        sharpness = std::stoi(value);

        tvError_t ret = SetSharpness(sharpness);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set sharpness\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save Sharpness to localstore and ssm_data
            int params[3]={0};
            params[0]=sharpness;

            int retval=UpdatePQParamsToCache("set","sharpness","Sharpness",PQ_PARAM_SHARPNESS,params);
            if(retval != 0) {
                LOGWARN("Failed to Save Sharpness to ssm_data\n");
            }
            LOGINFO("Exit : setSharpness successful to value: %d\n", sharpness);
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::getSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int saturation;
        tvError_t ret = GetSaturation(&saturation);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["saturation"] = std::to_string(saturation);
            LOGINFO("Exit : Saturation Value: %d\n", saturation);
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::getSaturation2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int saturation;
        JsonObject range;
        JsonObject saturationObj;
        range["From"] = 0;
        range["To"] = 100;
        saturationObj["Range"] = range;

        tvError_t ret = GetSaturation(&saturation);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            saturationObj["Setting"] = std::to_string(saturation);
            response["Saturation"] = saturationObj;
            LOGINFO("Exit : Saturation Value: %d\n", saturation);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry \n");
        PLUGIN_Lock(tvLock);

        std::string value;
        int saturation = 0;

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(value);
        saturation = std::stoi(value);

        tvError_t ret = SetSaturation(saturation);
        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set saturation\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save Saturation to localstore and ssm_data
            int params[3]={0};
            params[0]=saturation;
            int retval=UpdatePQParamsToCache("set","saturation","Saturation",PQ_PARAM_SATURATION,params);
            if(retval != 0) {
                LOGWARN("Failed to Save Saturation to ssm_data\n");
            }
            LOGINFO("Exit : setSaturation successful to value: %d\n",saturation);
            returnResponse(true, "success");
        }

    }


    uint32_t TVMgr::setHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        std::string value;
        int hue = 0;

        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(value);
        hue = std::stoi(value);

        tvError_t ret = SetHue(hue);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set hue\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save Sharpness to localstore and ssm_data
            int params[3]={0};
            params[0]=hue;
            int retval=UpdatePQParamsToCache("set","hue","Hue",PQ_PARAM_HUE,params);

            if(retval != 0) {
                LOGWARN("Failed to Save Hue to ssm_data\n");
            }
            LOGINFO("Exit : setHue successful to value: %d\n",hue);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        int hue;
        tvError_t ret = GetHue(&hue);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["hue"] = std::to_string(hue);
            LOGINFO("Exit : Hue Value: %d\n", hue);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getHue2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        int hue;
        JsonObject range;
        JsonObject hueObj;
        range["From"] = 0;
        range["To"] = 100;
        hueObj["Range"] = range;

        tvError_t ret = GetHue(&hue);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            hueObj["Setting"] = std::to_string(hue);
            response["Hue"] = hueObj;
            LOGINFO("Exit : Hue Value: %d\n", hue);
            returnResponse(true, "success");
        }
    }

    tvError_t TVMgr::getUserSelectedAspectRatio (tvDisplayMode_t* mode)
    {
        tvError_t ret = tvERROR_GENERAL;
#if !defined (HDMIIN_4K_ZOOM)
        LOGERR("tvmgrplugin: %s mode selected is: %d", __FUNCTION__, m_videoZoomMode);
        if (m_isDisabledHdmiIn4KZoom) {
            if (!(m_currentHdmiInResoluton<dsVIDEO_PIXELRES_3840x2160 ||
               (dsVIDEO_PIXELRES_MAX == m_currentHdmiInResoluton))){
                *mode = (tvDisplayMode_t)m_videoZoomMode;
                LOGWARN("tvmgrplugin: %s: Getting zoom mode %d for display, for 4K and above", __FUNCTION__, *mode);
                return tvERROR_NONE;
            }
        }
#endif
        ret = GetAspectRatio(mode);
        return ret;
    }

    uint32_t TVMgr::getAspectRatio(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        tvDisplayMode_t mode;
        tvError_t ret = getUserSelectedAspectRatio (&mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            response["aspectRatio"] = std::to_string(mode);
            LOGINFO("Exit : Aspect Ratio Value: %d\n", mode);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getAspectRatio2(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        tvDisplayMode_t mode;
        JsonObject dispModeObj;
        JsonArray dispOptions ;
        dispOptions.Add("TV AUTO"); dispOptions.Add("TV DIRECT"); dispOptions.Add("TV NORMAL");
        dispOptions.Add("TV 16X9 STRETCH"); dispOptions.Add("TV 4X3 PILLARBOX"); dispOptions.Add("TV ZOOM");
        dispModeObj["Selected"] = "TV AUTO";
        dispModeObj["Options"] = dispOptions;

        tvError_t ret = getUserSelectedAspectRatio (&mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            switch(mode) {

                case tvDisplayMode_16x9:
                    LOGINFO("Aspect Ratio: TV 16X9 STRETCH\n");
                    dispModeObj["Selected"] = "TV 16X9 STRETCH";
                    break;

                case tvDisplayMode_4x3:
                    LOGINFO("Aspect Ratio: TV 4X3 PILLARBOX\n");
                    dispModeObj["Selected"] = "TV 4X3 PILLARBOX";
                    break;

                case tvDisplayMode_NORMAL:
                    LOGINFO("Aspect Ratio: TV Normal\n");
                    dispModeObj["Selected"] = "TV NORMAL";
                    break;

                case tvDisplayMode_AUTO:
                    LOGINFO("Aspect Ratio: TV AUTO\n");
                    dispModeObj["Selected"] = "TV AUTO";
                    break;

                case tvDisplayMode_DIRECT:
                    LOGINFO("Aspect Ratio: TV DIRECT\n");
                    dispModeObj["Selected"] = "TV DIRECT";
                    break;

                case tvDisplayMode_ZOOM:
                    LOGINFO("Aspect Ratio: TV ZOOM\n");
                    dispModeObj["Selected"] = "TV ZOOM";
                    break;

                default:
                    LOGINFO("Aspect Ratio: TV AUTO\n");
                    break;
            }
            response["AspectRatio"] = dispModeObj;
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setAspectRatio(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDisplayMode_t mode = tvDisplayMode_16x9;
        std::string sMode = "16:9";

        value = parameters.HasLabel("aspectRatio") ? parameters["aspectRatio"].String() : "";
        returnIfParamNotFound(value);
        mode = (tvDisplayMode_t)std::stoi(value);

        switch(mode) {

            case tvDisplayMode_16x9:
                sMode = "16:9";
                break;

            case tvDisplayMode_4x3:
                sMode = "4:3";
                break;

            case tvDisplayMode_FULL:
                sMode = "Full";
                break;

            case tvDisplayMode_NORMAL:
                sMode = "Normal";
                break;

            default:
                break;
        }

        m_videoZoomMode = mode;
	LOGERR("tvmgrplugin: %s mode selected is: %d", __FUNCTION__,m_videoZoomMode);
        tvError_t ret = SetAspectRatio(mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save DisplayMode to localstore and ssm_data
            int params[3]={0};
            params[0]=mode;
            int retval=UpdatePQParamsToCache("set","aspectratio","AspectRatio",PQ_PARAM_ASPECT_RATIO,params);

            if(retval != 0) {
                LOGWARN("Failed to Save DisplayMode to ssm_data\n");
            }

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_ASPECTRATIO_RFC_PARAM, sMode.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, sMode.c_str());
            }
            LOGINFO("Exit : SetAspectRatio() value : %s\n",sMode.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setAspectRatio2(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDisplayMode_t mode = tvDisplayMode_16x9;

        value = parameters.HasLabel("aspectRatio") ? parameters["aspectRatio"].String() : "";
        returnIfParamNotFound(value);

        if(!value.compare("TV 16X9 STRETCH")) {
            mode = tvDisplayMode_16x9;
        }
        else if (!value.compare("TV 4X3 PILLARBOX")){
            mode = tvDisplayMode_4x3;
        }
        else if (!value.compare("TV NORMAL")){
            mode = tvDisplayMode_NORMAL;
        }
        else if (!value.compare("TV DIRECT")){
            mode = tvDisplayMode_DIRECT;
        }
        else if (!value.compare("TV AUTO")){
            mode = tvDisplayMode_AUTO;
        }
        else if (!value.compare("TV ZOOM")){
            mode = tvDisplayMode_ZOOM;
        }
        else {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        m_videoZoomMode = mode;
        tvError_t ret = setAspectRatioZoomSettings (mode);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save DisplayMode to localstore and ssm_data
            int params[3]={0};
            params[0]=mode;
            int retval=UpdatePQParamsToCache("set","aspectratio","AspectRatio",PQ_PARAM_ASPECT_RATIO,params);

            if(retval != 0) {
                LOGWARN("Failed to Save DisplayMode to ssm_data\n");
            }

            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_ASPECTRATIO_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, value.c_str());
            }
            LOGINFO("Exit : SetAspectRatio2() value : %s\n",value.c_str());
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::getColorTemperature(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        tvColorTemp_t ct;
        tvError_t ret = GetColorTemperature(&ct);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            switch(ct) {

                case tvColorTemp_STANDARD:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemp"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Color Temp Value: Warm\n");
                    response["colorTemp"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Color Temp Value: Cold\n");
                    response["colorTemp"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Color Temp Value: User Defined\n");
                    response["colorTemp"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    response["colorTemp"] = "Standard";
                    break;
            }
            LOGINFO("Exit : getColorTemperature() : %d\n",ct);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getColorTemperature2(const JsonObject& parameters, JsonObject& response)
    {

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);

        tvColorTemp_t ct;
        JsonObject ctObj;
        JsonArray ctOptions;
        ctOptions.Add("Standard"); ctOptions.Add("Warm"); ctOptions.Add("Cold"); ctOptions.Add("User Defined");
        ctObj["Selected"] = "Standard";
        ctObj["Options"] = ctOptions;

        tvError_t ret = GetColorTemperature(&ct);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            switch(ct) {

                case tvColorTemp_STANDARD:
                    LOGINFO("Exit : Color Temp Value: Standard\n");
                    ctObj["Selected"] = "Standard";
                    break;

                case tvColorTemp_WARM:
                    LOGINFO("Exit : Color Temp Value: Warm\n");
                    ctObj["Selected"] = "Warm";
                    break;

                case tvColorTemp_COLD:
                    LOGINFO("Exit : Color Temp Value: Cold\n");
                    ctObj["Selected"] = "Cold";
                    break;

                case tvColorTemp_USER:
                    LOGINFO("Exit : Color Temp Value: User Defined\n");
                    ctObj["Selected"] = "User Defined";
                    break;

                default:
                    LOGINFO("Color Temp Value: Standard\n");
                    break;
            }
            response["ColorTemperature"] = ctObj;
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::setColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvColorTemp_t ct = tvColorTemp_STANDARD;

        value = parameters.HasLabel("colorTemp") ? parameters["colorTemp"].String() : "";
        returnIfParamNotFound(value);

        if(!value.compare("Standard")) {
            ct = tvColorTemp_STANDARD;
        }
        else if (!value.compare("Warm")){
            ct = tvColorTemp_WARM;
        }
        else if (!value.compare("Cold")){
            ct = tvColorTemp_COLD;
        }
        else if (!value.compare("User Defined")){
            ct = tvColorTemp_USER;
        }
        else {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        tvError_t ret = SetColorTemperature(ct);

        if(ret != tvERROR_NONE) {
            LOGWARN("Failed to set Color Temperature\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            //Save ColorTemperature to localstore and ssm_data
            int params[3]={0};
            params[0]=ct;

            int retval=UpdatePQParamsToCache("set","colortemp","ColorTemp",PQ_PARAM_COLOR_TEMPERATURE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save Color Temperature to ssm_data\n");
            }

            LOGINFO("Exit : setColorTemperature successful to value: %d\n",ct);
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::setWakeupConfiguration(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        std::string value;
        tvWakeupSrcType_t src_type = tvWakeupSrc_MAX; //default
        int config = 0;

        value = parameters.HasLabel("wakeupSrc") ? parameters["wakeupSrc"].String() : "";
        returnIfParamNotFound(value);
        src_type = (tvWakeupSrcType_t)std::stoi(value);
        value.clear();

        value = parameters.HasLabel("config") ? parameters["config"].String() : "";
        returnIfParamNotFound(value);
        config = std::stoi(value);
        value.clear();

        tvError_t ret = setWakeupConfig(src_type, (bool)config);

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {

            returnResponse(true, "success");
        }

    }


    std::string TVMgr::getErrorString (tvError_t eReturn)
    {
        switch (eReturn)
        {
            case tvERROR_NONE:
                return "TV API SUCCESS";
            case tvERROR_GENERAL:
                return "TV API FAILED";
            case tvERROR_OPERATION_NOT_SUPPORTED:
                return "TV OPERATION NOT SUPPORTED ERROR";
            case tvERROR_INVALID_PARAM:
                return "TV INVALID PARAM ERROR";
            case tvERROR_INVALID_STATE:
                return "TV INVALID STATE ERROR";
        }
        return "TV UNKNOWN ERROR";
    }

    uint32_t TVMgr::getSupportedPictureModes(const JsonObject& parameters, JsonObject& response)
    {//sample response: {"success":true,"SupportedPicmodes":["standard","vivid","colorful","game"]}
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        pic_modes_t *pictureModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedPictureModes(&pictureModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedPicModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedPicModes.Add(pictureModes[count].name);
                // printf("Added Mode %s %s \n",pictureModes[count].name,SupportedPicModes[count].String().c_str());
            }

            response["SupportedPicmodes"] = SupportedPicModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::getSupportedDolbyVisionModes(const JsonObject& parameters, JsonObject& response)
    {//sample response: {"success":true,”SupportedDolbyModes": ["bright","dark"]}

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        pic_modes_t *dvModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedDVModes(&dvModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedDVModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedDVModes.Add(dvModes[count].name);
            }

            response["SupportedDVModes"] = SupportedDVModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }

    }
    uint32_t TVMgr::getDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["DolbyVisionMode"] = s;
            LOGINFO("Exit getDolbyVisionMode(): %s\n",s.c_str());
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::setDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;

        value = parameters.HasLabel("DolbyVisionMode") ? parameters["DolbyVisionMode"].String() : "";
        returnIfParamNotFound(value);

        tvError_t ret = SetTVDolbyVisionMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=GetDolbyModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","dvmode","DolbyVisionMode",PQ_PARAM_DOLBY_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save DolbyVisionMode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setDynamicContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;

        value = parameters.HasLabel("DynamicContrast") ? parameters["DynamicContrast"].String() : "";
        returnIfParamNotFound(value);

        tvError_t ret = SetDynamicContrast(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_DYNAMICCONTRASTMODE_STRING_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_DYNAMICCONTRASTMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_DYNAMICCONTRASTMODE_STRING_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit setDynamicContrast() Value : %s\n",value.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getDynamicContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        char mode[PIC_MODE_NAME_MAX]={0};
        tvError_t ret =  GetDynamicContrast(mode);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            std::string s;
            s+=mode;
            response["DynamicContrast"] = s;
            LOGINFO("Exit : getDynamicContrast() : %s\n",s.c_str());
            returnResponse(true, "success");
        }

    }

    int TVMgr::getCurrentPictureMode(char *picMode){
        TR181_ParamData_t param;
        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
        if ( err == tr181Success ) {
            strncpy(picMode, param.value, strlen(param.value)+1);
            LOGINFO("getLocalParam success, mode = %s\n", picMode);
            return 1;
        }
        else {
            LOGWARN("getLocalParam failed");
            return 0;
        }
    }

    uint32_t TVMgr::generateStorageIdentifierDirty(std::string &key,const char * forParam,uint32_t contentFormat, int pqmode)
    {
        key+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
        key+=STRING_PICMODE+std::to_string(pqmode)+std::string(".")+std::string(STRING_FORMAT)+std::to_string(contentFormat);
        CREATE_DIRTY(key)+=forParam;

        return tvERROR_NONE;
    }


    uint32_t TVMgr::enableWBMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        std::string value;
        tvError_t ret = tvERROR_NONE;
        bool mode = 0;
        if(parameters.HasLabel("mode")) {
            value = parameters["mode"].String();
            if(value == "true") {
                mode = 1;
            } else if(value == "false") {
                mode = 0;
            }
            ret = enableWBmode(mode);
            if(ret != tvERROR_NONE) {
                LOGERR("enableWBmode failed\n");
                returnResponse(false, getErrorString(ret).c_str());
            }
            else{
                LOGINFO("enableWBmode successful... \n");
                returnResponse(true, "Success");
            }

        } else {
            returnResponse(false, "Parameter missing");
        }
    }


    /***
     *@brief setting the WB gain or offset the specific selector can be used * to target gain&offset value for the specified combination
     * @param1[in] : {"jsonrpc":"2.0", "id":3,
     * "method":"org.rdk.tv.ControlSettings.1.setWBCtrl", "params":{
     * "applies" : [{"selector":"color temp","index":"cool"},
     * {"selector":"input","index":"HDMI1"}]"color":"red", "ctrl":"gain",
     * "value":129 }}
     * @param2[out] : {"jsonrpc":"2.0","id":3,"result":{"success":true}}
     */
    uint32_t TVMgr::setWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        JsonArray applies;
        JsonObject obj;
        std::vector<std::string> selector;
        std::vector<std::string> index;
        std::string val;
        std::string color;
        std::string ctrl;
        int value;
        if(parameters.HasLabel("applies") && parameters.HasLabel("color") && parameters.HasLabel("ctrl") && parameters.HasLabel("value"))
        {
            color = parameters["color"].String();
            ctrl = parameters["ctrl"].String();
            val = parameters["value"].String();
            value = std::stoi(val);
            applies = parameters["applies"].Array();
            for(int i=0; i<applies.Length(); ++i)
            {
                obj = applies[i].Object();
                selector.push_back(obj["selector"].String());
                index.push_back(obj["index"].String());
            }
            std::string temp = "color temp";
            std::string input = "input";
            std::string colorTemp;
            std::string inputSrc;
            int pos;
            auto found = find(selector.begin(), selector.end(), temp);
            auto found1 = find(selector.begin(), selector.end(), input);
            if(found1 == selector.end()) {
                LOGINFO("Input source not passed");
                inputSrc = "";
            } else {
                pos = found1 - selector.begin();
                inputSrc = index.at(pos);
            }
            if(found != selector.end())
            {
                pos = found - selector.begin();
                colorTemp = index.at(pos);
                LOGINFO("input source : %s\ncolortemp : %s\ncolor : %s\nctrl : %s\nvalue : %d\n", inputSrc.c_str(), colorTemp.c_str(), color.c_str(), ctrl.c_str(), value);
                tvError_t ret = setWBctrl (const_cast <char *> (inputSrc.c_str()),const_cast <char *> (colorTemp.c_str()), const_cast <char *> (color.c_str()), const_cast <char *> (ctrl.c_str()), value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("setWBCtrl failed");
                    returnResponse(false, getErrorString(ret).c_str());
                } else {
                    //set it to local cache
                    std::string identifier = TVSETTINGS_GENERIC_STRING_RFC_PARAM;
                    identifier+=std::string("wb")+std::string(STRING_DIRTY)+color+"."+ctrl;
                    tr181ErrorCode_t err = setLocalParam(rfc_caller_id, identifier.c_str(), val.c_str());
                    if ( err != tr181Success ) {
                        LOGWARN("setLocalParam for %s Failed : %s\n", identifier.c_str(), getTR181ErrorString(err));
                    }
                    else {
                        LOGINFO("setLocalParam for %s Successful, Value: %d\n", identifier.c_str(), value);
                    }
                    LOGINFO("setWBCtrl success");
                    returnResponse(true, "success");
                }
            } else {
                returnResponse(false, "color temp or input not available");
            }
        }
        else{
            returnResponse(false,"not valid parameter");
        }
    }

    /***
     * @brief Get the white balance ctrl value for the color specified, and *the specified selector.
     *When getting a gain or offset value all selectors must be specified.
     * @param1[in]  : {"jsonrpc":"2.0", "id":3, "method":"org.rdk.tv.ControlSettings.1.getWBCtrl","params":{"applies":
     * [{"selector":"color temp","index":"normal"},{"selector":"input",
     * "index":"HDMI1"}]"color":"red","ctrl":"gain" }}
     * @param2[out] : {"jsonrpc":"2.0","id":3,"result":{"success":true}}
     */
    uint32_t TVMgr::getWBCtrl(const JsonObject& parameters,JsonObject& response)
    {
        LOGINFOMETHOD();
        JsonArray applies;
        JsonObject obj;
        std::vector<std::string> selector;
        std::vector<std::string> index;
        std::string color;
        std::string ctrl;
        int value;
        if ((parameters.HasLabel("applies")) && (parameters.HasLabel("color")) && (parameters.HasLabel("ctrl")))
        {
            color = parameters["color"].String();
            ctrl = parameters["ctrl"].String();
            applies = parameters["applies"].Array();
            for (int i = 0; i < applies.Length(); ++i)
            {
                obj = applies[i].Object();
                selector.push_back(obj["selector"].String());
                index.push_back(obj["index"].String());
            }
            std::string colorTemp;
            std::string inputSrc;
            int pos;
            std::string temp = "color temp";
            std::string input = "input";
            auto found = find(selector.begin(), selector.end(), temp);
            auto found1 = find(selector.begin(), selector.end(), input);
            if(found1 == selector.end()) {
                LOGINFO("Input source not passed");
                inputSrc = "";
            } else {
                pos = found1 - selector.begin();
                inputSrc = index.at(pos);
            }
            if(found != selector.end())
            {
                pos = found - selector.begin();
                colorTemp = index.at(pos);

                LOGINFO("input source : %s\ncolortemp : %s\ncolor : %s\nctrl : %s\n", inputSrc.c_str(), colorTemp.c_str(), color.c_str(), ctrl.c_str());
                // Call to HAL API to getWBctrl
                tvError_t ret = getWBctrl (const_cast <char *> (inputSrc.c_str()), const_cast <char *> (colorTemp.c_str()), const_cast <char *> (color.c_str()), const_cast <char *> (ctrl.c_str()), &value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("getWBCtrl failed");
                    returnResponse(false, getErrorString(ret).c_str());
                } else {
                    LOGINFO("getWBCtrl success");
                    response["value"] = value;
                    returnResponse(true, "success");
                }

            } else {
                returnResponse(false, "wrong selector value");
            }
        } else {
            returnResponse(false, "Missing Parameter");
        }
    }

    uint32_t TVMgr::getWBInfo(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();
        JsonArray supportedWBSelectors;
        JsonObject supportedWBSelectorInfo;
        JsonArray colorTemp;
        JsonArray inputs;
        JsonArray supportedWBCtrls;
        JsonObject supportedWBCtrlInfo;
        JsonObject gain;
        JsonObject offset;
        tvError_t ret;
        unsigned int index = 0;
        getWBInfo_t wbInfo;
        std::vector<std::string> wbSelector;
        std::vector<std::string> wbColorTmp;
        std::vector<std::string> wbInput;

        /* HAL call for getWBInfo */
        ret = getWbInfo(&wbInfo, wbSelector, wbColorTmp, wbInput);
        if(ret != tvERROR_NONE) {
            LOGWARN("getWbInfo() Failed!!! \n");
        }
        for (index = 0; index < wbSelector.size(); index++) {
            supportedWBSelectors.Add(wbSelector[index]);
        }
        response["supportedWBSelectors"] = supportedWBSelectors;

        for (index = 0; index < wbColorTmp.size(); index++) {
            colorTemp.Add(wbColorTmp[index]);
        }

        for (index = 0; index < wbInput.size(); index++) {
            inputs.Add(wbInput[index]);
        }
        if (wbColorTmp.size() != 0) {
            supportedWBSelectorInfo[wbSelector[0].c_str()] = colorTemp;
        }
        if (wbInput.size() != 0) {
            supportedWBSelectorInfo[wbSelector[1].c_str()] = inputs;
        }
        response["supportedWBSelectorInfo"] = supportedWBSelectorInfo;

        for (index = 0; index < 2; index++) {
            supportedWBCtrls.Add(wbInfo.wbControls[index]);
        }
        response["supportedWBCtrls"] = supportedWBCtrls;
        gain["min"] = std::string(wbInfo.wbGain[0]);
        gain["max"] = std::string(wbInfo.wbGain[1]);
        supportedWBCtrlInfo[wbInfo.wbControls[0]] = gain;
        offset["min"] = std::string(wbInfo.wbOffset[0]);
        offset["max"] = std::string(wbInfo.wbOffset[1]);
        supportedWBCtrlInfo[wbInfo.wbControls[1]] = offset;
        response["supportedWBCtrlInfo"] = supportedWBCtrlInfo;
        returnResponse(true, "success");

    }

    uint32_t TVMgr::getComponentColorInfo(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int blComponentColor;
        JsonArray supportedComponentColor;

        tvError_t ret = GetSupportedComponentColor(&blComponentColor);
        if(ret != tvERROR_NONE) {
            supportedComponentColor.Add("none");
            response["colors"] = supportedComponentColor;
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            if(blComponentColor & tvDataColor_RED)supportedComponentColor.Add("red");
            if(blComponentColor & tvDataColor_GREEN)supportedComponentColor.Add("green");
            if(blComponentColor & tvDataColor_BLUE)supportedComponentColor.Add("blue");
            if(blComponentColor & tvDataColor_YELLOW)supportedComponentColor.Add("yellow");
            if(blComponentColor & tvDataColor_CYAN)supportedComponentColor.Add("cyan");
            if(blComponentColor & tvDataColor_MAGENTA)supportedComponentColor.Add("magneta");
            response["colors"] = supportedComponentColor;
            LOGINFO("Exit : supportedComponentColor Value: %d\n",blComponentColor );
            returnResponse(true, "success");

        }

    }

    uint32_t TVMgr::setComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDataComponentColor_t blSaturationColor;
        int saturation;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blSaturationColor =  GetComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        value = parameters.HasLabel("saturation") ? parameters["saturation"].String() : "";
        returnIfParamNotFound(value);
        saturation = std::stoi(value);

        //Enable CMS
        tvError_t ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","compsat","cms.enable",PQ_PARAM_CMS,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        ret = SetCurrentComponentSaturation(blSaturationColor, saturation);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=COLOR_SATURATION;
            params[1]=ConvertTVColorToVendorColor(blSaturationColor);
            params[2]=saturation;

            char format[64]={0};
            snprintf(format,sizeof(format),"saturation.%s",component_color[ConvertTVColorToVendorColor(blSaturationColor)]);
            int retval=UpdatePQParamsToCache("set","compsat",format,PQ_PARAM_CMS,params);

            if(retval != 0) {
                LOGWARN("Failed to Save component saturation to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentSaturation() %s : %s\n",format,value.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDataComponentColor_t blSaturationColor;
        int saturation;
        JsonObject range;
        JsonObject saturationColorObj;
        range["From"] = 0;
        range["To"] = 100;
        saturationColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blSaturationColor =  GetComponentColorEnum(value);
        if(blSaturationColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }
        tvError_t ret = GetCurrentComponentSaturation(blSaturationColor, &saturation);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            saturationColorObj["Setting"] = std::to_string(saturation);
            response["saturation"] = saturationColorObj;
            LOGINFO("Exit : Component Saturation for color: %s Value: %d\n", value.c_str(),saturation);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value_mode;
        tvError_t ret = tvERROR_NONE;

        value_mode = parameters.HasLabel("DimmingMode") ? parameters["DimmingMode"].String() : "";
        returnIfParamNotFound(value_mode);

        ret = SetTVDimmingMode(value_mode.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret));
        }
        else {
            //Save Dimming Mode to localstore and ssm_data
            char format[64]={0};
            int params[3]={0};
            params[0]=GetDimmingModeIndex(value_mode.c_str());
            int retval=UpdatePQParamsToCache("set","ldim","DimmingMode",PQ_PARAM_LDIM,params);

            if(retval != 0) {
                LOGWARN("Failed to Save Dimmingmode to ssm_data\n");
            }

            //Save To New Format
            snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,GetCurrentPQIndex(),"DimmingMode");
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, format, value_mode.c_str());
            if ( err != tr181Success )
            {
                LOGWARN("setLocalParam for %s Failed : %s\n", format, getTR181ErrorString(err));
                ret=tvERROR_GENERAL;
            }
            else
                LOGINFO("setLocalParam for %s Successful(New Format), Value: %s for pqmode:%d\n", format,value_mode.c_str(),GetCurrentPQIndex());

            //Also Save To Old Format
            err = setLocalParam(rfc_caller_id,TVSETTINGS_DIMMING_MODE_RFC_PARAM,value_mode.c_str());
            if ( err != tr181Success )
            {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_DIMMING_MODE_RFC_PARAM, getTR181ErrorString(err));
                ret=tvERROR_GENERAL;
            }
            else
                LOGINFO("setLocalParam for %s Successful(Old Format), Value: %s for pqmode:%d\n", format,value_mode.c_str(),GetCurrentPQIndex());
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : setBacklightDimmingMode() Value : %s\n",value_mode.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        TR181_ParamData_t param;
        char format[BUFFER_SIZE]={0};

        snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,GetCurrentPQIndex(),"DimmingMode");
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, format, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["DimmingMode"] = s;
            LOGINFO("Exit : getBacklightDimmingMode : %s\n",s.c_str());
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getAllBacklightDimmingModes(const JsonObject& parameters, JsonObject& response)
    {//sample response: {"success":true,”SupportedDimmingModes": ["local","global","fixed"]}

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        char *DimmingModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedDimmingModes(&DimmingModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret));
        }
        else {
            JsonArray SupportedDimmingModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedDimmingModes.Add(DimmingModes+(count*DIMMING_MODE_NAME_SIZE));
            }

            response["SupportedDimmingModes"] = SupportedDimmingModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::resetBrightness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param={0};
        string identifier;
        int brightness=0;
        int params[3]={0};

        int retval=UpdatePQParamsToCache("reset","brightness","Brightness",PQ_PARAM_BRIGHTNESS,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Brightness\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            generateStorageIdentifier(identifier,"Brightness",getContentFormatIndex(GetCurrentContentFormat()));
            if(identifier.empty()){
                LOGWARN("generateStorageIdentifier failed\n");
            }
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
            if ( err == tr181Success ) {
                brightness=std::stoi(param.value);
                ret = SetBrightness(brightness);
            }
            else {
                LOGWARN("resetBrightness Failed unable to fetch default value from localstore\n");
                ret = tvERROR_GENERAL;
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetBrightness Successful to value : %d \n",brightness);
            returnResponse(true, "success");
        }
    }
    uint32_t TVMgr::resetSharpness(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param;
        string identifier;
        int sharpness=0;
        int params[3]={0};

        memset(&param, 0, sizeof(param));

        int retval=UpdatePQParamsToCache("reset","sharpness","Sharpness",PQ_PARAM_SHARPNESS,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Sharpness\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            generateStorageIdentifier(identifier,"Sharpness",getContentFormatIndex(GetCurrentContentFormat()));
            if(identifier.empty()){
                LOGWARN("generateStorageIdentifier failed\n");
            }
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
            if ( err == tr181Success ) {
                sharpness=std::stoi(param.value);
                ret = SetSharpness(sharpness);
            }
            else {
                LOGWARN("resetSharpness Failed unable to fetch default value from localstore\n");
                ret = tvERROR_GENERAL;
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetSharpness Successful to value:%d\n",sharpness);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param;
        string identifier;
        int saturation=0;
        int params[3]={0};

        memset(&param, 0, sizeof(param));

        int retval=UpdatePQParamsToCache("reset","saturation","Saturation",PQ_PARAM_SATURATION,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Saturation\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            generateStorageIdentifier(identifier,"Saturation",getContentFormatIndex(GetCurrentContentFormat()));
            if(identifier.empty()){
                LOGWARN("generateStorageIdentifier failed\n");
            }
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
            if ( err == tr181Success ) {
                saturation=std::stoi(param.value);
                ret = SetSaturation(saturation);
            }
            else {
                LOGWARN("resetSaturation Failed unable to fetch default value from localstore\n");
                ret = tvERROR_GENERAL;
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetSaturation Successful to value: %d \n",saturation);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param;
        string identifier;
        int hue=0;
        int params[3]={0};

        memset(&param, 0, sizeof(param));

        int retval=UpdatePQParamsToCache("reset","hue","Hue",PQ_PARAM_HUE,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Hue\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            generateStorageIdentifier(identifier,"Hue",getContentFormatIndex(GetCurrentContentFormat()));
            if(identifier.empty()){
                LOGWARN("generateStorageIdentifier failed\n");
            }
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
            if ( err == tr181Success ) {
                hue=std::stoi(param.value);
                ret = SetHue(hue);
            }
            else {
                LOGWARN("resetHue Failed unable to fetch default value from localstore\n");
                ret = tvERROR_GENERAL;
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetHue Successful to value : %d\n",hue);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        int backlight=0;

        if(appUsesGlobalBackLightFactor){
            tr181ErrorCode_t err = clearLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM);
            if ( err != tr181Success ) {
                LOGWARN("ClearLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM, getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
            else {
                LOGINFO("ClearLocalParam for %s Successful\n", TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM);
            }
            err = clearLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM);
            if ( err != tr181Success ) {
                LOGWARN("ClearLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM, getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
        }

        /* non backlight factor path */
        {
            TR181_ParamData_t param;
            string identifier;
            int params[3]={0};

            memset(&param, 0, sizeof(param));

            int retval=UpdatePQParamsToCache("reset","backlight","Backlight",PQ_PARAM_BACKLIGHT,params);
            if(retval != 0 ) {
                LOGWARN("Failed to reset Backlight\n");
                ret = tvERROR_GENERAL;
            }
            else
            {
                /* this default is same for the GBF path and non GBF path so
                 * no need to read separately for GBF path
                 */
                generateStorageIdentifier(identifier,"Backlight",getContentFormatIndex(GetCurrentContentFormat()));
                if(identifier.empty()){
                    LOGWARN("generateStorageIdentifier failed\n");
                }
                tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                if ( err == tr181Success ) {
                    backlight=std::stoi(param.value);
                    ret = SetBacklight(backlight);
                }
                else {
                    LOGWARN("resetBacklight Failed unable to fetch default value from localstore\n");
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetBacklight Successful to value : %d\n",backlight);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetColorTemperature(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param;
        string identifier;
        int colortemp=0;
        int params[3]={0};

        memset(&param, 0, sizeof(param));

        int retval=UpdatePQParamsToCache("reset","colortemp","ColorTemp",PQ_PARAM_COLOR_TEMPERATURE,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset ColorTemp\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            generateStorageIdentifier(identifier,"ColorTemp",getContentFormatIndex(GetCurrentContentFormat()));
            if(identifier.empty()){
                LOGWARN("generateStorageIdentifier failed\n");
            }
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
            if ( err == tr181Success ) {
                int colortemp=std::stoi(param.value);
                ret = SetColorTemperature((tvColorTemp_t)colortemp);
            }
            else {
                LOGWARN("resetColorTemp Failed unable to fetch default value from localstore\n");
                ret = tvERROR_GENERAL;
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetColorTemperature Successful to value : %d\n",colortemp);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetDolbyVisionMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);

                ret = SetTVDolbyVisionMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("DV Mode initialized successfully value %s\n",param.value);
                    //Save DolbyVisionMode to ssm_data
                    int params[3]={0};
                    params[0]=GetDolbyModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","dvmode","DolbyVisionMode",PQ_PARAM_DOLBY_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save DolbyVisionMode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetDolbyVisionMode() \n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetAutoBacklight(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_AUTO_BACKLIGHT_MODE_RFC_PARAM);

                tvBacklightMode_t blMode = tvBacklightMode_NONE;

                if(!std::string(param.value).compare("none")) {
                    blMode = tvBacklightMode_NONE;
                }
                else if (!std::string(param.value).compare("manual")){
                    blMode = tvBacklightMode_MANUAL;
                }
                else if (!std::string(param.value).compare("ambient")){
                    blMode = tvBacklightMode_AMBIENT;
                }
                else if (!std::string(param.value).compare("eco")){
                    blMode = tvBacklightMode_ECO;
                }
                else {
                    blMode = tvBacklightMode_NONE;
                }
                ret = SetCurrentBacklightMode(blMode);
                if(ret != tvERROR_NONE) {
                    LOGWARN("Autobacklight Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {
                    LOGINFO("Exit : Autobacklight Mode set successfully, value: %s\n", param.value);
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetContrast(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param;
        string identifier;
        int contrast=0;
        int params[3]={0};

        memset(&param, 0, sizeof(param));

        int retval=UpdatePQParamsToCache("reset","contrast","Contrast",PQ_PARAM_CONTRAST,params);
        if(retval != 0 ) {
            LOGWARN("Failed to reset Contrast\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            generateStorageIdentifier(identifier,"Contrast",getContentFormatIndex(GetCurrentContentFormat()));
            if(identifier.empty()){
                LOGWARN("generateStorageIdentifier failed\n");
            }
            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
            if ( err == tr181Success ) {
                contrast=std::stoi(param.value);
                ret = SetContrast(contrast);
            }
            else {
                LOGWARN("resetContrast Failed unable to fetch default value from localstore\n");
                ret = tvERROR_GENERAL;
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetContrast Successful to value %d\n",contrast);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getContentFormatIndex(tvVideoHDRFormat_t formatToConvert)
    {
        /* default to SDR always*/
        tvContentFormatType_t ret = tvContentFormatType_NONE;
        switch(formatToConvert)
        {
            case tvVideoHDRFormat_HLG:
                ret = tvContentFormatType_HLG;
                break;

            case tvVideoHDRFormat_HDR10:
                ret = tvContentFormatType_HDR10;
                break;

            case tvVideoHDRFormat_HDR10PLUS:
                ret =  tvContentFormatType_HDR10PLUS;
                break;

            case tvVideoHDRFormat_DV:
                ret = tvContentFormatType_DOVI;
                break;

            case tvVideoHDRFormat_SDR:
            case tvVideoHDRFormat_NONE:
            default:
                ret  = tvContentFormatType_SDR;
                break;
        }
        return ret;
    }

    bool TVMgr::isSaveforAllContentFormats(void)
    {
        TR181_ParamData_t param;
        bool ret  =false;

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_SAVE_FOR_ALL_CONTENT_FORMAT_RFC_PARAM,&param);
        if ( err != tr181Success ) {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_SAVE_FOR_ALL_CONTENT_FORMAT_RFC_PARAM, getTR181ErrorString(err));
        }
        else {
            if(!std::string(param.value).compare("true"))
            {
                ret = true;
            }
        }
        return ret;
    }

    uint32_t TVMgr::resetAspectRatio(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_ASPECTRATIO_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_ASPECTRATIO_RFC_PARAM);
            ret = setDefaultAspectRatio();
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetDefaultAspectRatio()\n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetBacklightDimmingMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        char format[BUFFER_SIZE]={0};
        TR181_ParamData_t param;
        tr181ErrorCode_t err;

        //Check old format of dimming mode in local store and remove it.
        err = clearLocalParam(rfc_caller_id,TVSETTINGS_DIMMING_MODE_RFC_PARAM);
        if ( err != tr181Success )
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_DIMMING_MODE_RFC_PARAM, getTR181ErrorString(err));
        else
            LOGINFO("clearLocalParam for %s Successful\n",TVSETTINGS_DIMMING_MODE_RFC_PARAM);

        //Check new format of Dimming  mode in local store and remove it.
        for (int mode=0;mode<numberModesSupported;mode++)
        {
            memset(&format, 0, sizeof(format));
            snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,pic_mode_index[mode],"DimmingMode");
            err = clearLocalParam(rfc_caller_id,format);

            if ( err != tr181Success ) {
                LOGWARN("clearLocalParam for %s Failed : %s\n", format, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
                break;
            }
            else
                LOGINFO("clearLocalParam for %s Successful\n", format);
        }
        //set and save
        if( ret == tvERROR_NONE)
        {
            memset(&format, 0, sizeof(format));
            memset(&param, 0, sizeof(param));
            snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,GetCurrentPQIndex(),"DimmingMode");
            err = getLocalParam(rfc_caller_id, format, &param);
            if ( tr181Success == err ) {
                ret=SetTVDimmingMode(param.value);
            }

            if(ret == tvERROR_NONE) {
                int params[3]={0};
                params[0]=GetDimmingModeIndex(param.value);
                int retval=UpdatePQParamsToCache("reset","ldim","DimmingMode",PQ_PARAM_LDIM,params);
                if(retval != 0 ) {
                    LOGWARN("Failed to Save ldim to ssm_data\n");
                    ret = tvERROR_GENERAL;
                }
            }
        }

        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : ResetDimmingMode() to value %s\n",param.value);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDataComponentColor_t blHueColor;
        int hue;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blHueColor =  GetComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        value = parameters.HasLabel("hue") ? parameters["hue"].String() : "";
        returnIfParamNotFound(value);
        hue = std::stoi(value);

        //Enable CMS
        tvError_t ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","comphue","cms.enable",PQ_PARAM_CMS,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        ret = SetCurrentComponentHue(blHueColor, hue);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=COLOR_HUE;
            params[1]=ConvertTVColorToVendorColor(blHueColor);
            params[2]=hue;
            char format[64]={0};
            snprintf(format,sizeof(format),"hue.%s",component_color[ConvertTVColorToVendorColor(blHueColor)]);
            int retval=UpdatePQParamsToCache("set","comphue",format,PQ_PARAM_CMS,params);

            if(retval != 0) {
                LOGWARN("Failed to Save hue to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentHue() color : %s value:%d\n",format,hue);
            returnResponse(true, "success");
        }
    }


    uint32_t TVMgr::getComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDataComponentColor_t blHueColor;
        int hue;
        JsonObject range;
        JsonObject hueColorObj;
        range["From"] = 0;
        range["To"] = 100;
        hueColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blHueColor =  GetComponentColorEnum(value);
        if(blHueColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }
        tvError_t ret = GetCurrentComponentHue(blHueColor, &hue);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            hueColorObj["Setting"] = std::to_string(hue);
            response["hue"] = hueColorObj;
            LOGINFO("Exit : Component Hue for Color: %s Value: %d\n", value.c_str(),hue);
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDataComponentColor_t blLumaColor;
        int luma;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blLumaColor =  GetComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }

        value = parameters.HasLabel("luma") ? parameters["luma"].String() : "";
        returnIfParamNotFound(value);
        luma = std::stoi(value);

        //Enable CMS
        tvError_t ret = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_ENABLE);
        if(ret != tvERROR_NONE) {
            LOGWARN("CMS enable failed\n");
            returnResponse(false, getErrorString(ret).c_str());
        }
        else
        {
            int cms_params[3]={0};
            cms_params[0]=COLOR_STATE;
            cms_params[1]=COLOR_ENABLE;
            cms_params[2]=COMPONENT_ENABLE;
            int retval = UpdatePQParamsToCache("set","compluma","cms.enable",PQ_PARAM_CMS,cms_params);
            if(retval != 0) {
                LOGWARN("Failed to Save enableflag to ssm_data\n");
            }
            LOGINFO("CMS Enable Success to localstore\n");
        }

        ret = SetCurrentComponentLuma(blLumaColor, luma);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {


            int params[3]={0};
            params[0]=COLOR_LUMA;
            params[1]=ConvertTVColorToVendorColor(blLumaColor);
            params[2]=luma;

            char format[BUFFER_SIZE]={0};
            snprintf(format,sizeof(format),"luma.%s",component_color[ConvertTVColorToVendorColor(blLumaColor)]);
            int retval=UpdatePQParamsToCache("set","compluma",format,PQ_PARAM_CMS,params);

            if(retval != 0) {
                LOGWARN("Failed to Save luma to ssm_data\n");
            }
            LOGINFO("Exit : SetComponentLuma() color : %s value:%d\n",format,luma);
            returnResponse(true, "success");
        }
    }



    uint32_t TVMgr::getComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Ëntry\n");
        PLUGIN_Lock(tvLock);
        std::string value;
        tvDataComponentColor_t blLumaColor;
        int luma;
        JsonObject range;
        JsonObject lumaColorObj;
        range["From"] = 0;
        range["To"] = 30;
        lumaColorObj["Range"] = range;

        value = parameters.HasLabel("color") ? parameters["color"].String() : "";
        returnIfParamNotFound(value);

        blLumaColor =  GetComponentColorEnum(value);
        if(blLumaColor ==tvDataColor_MAX)
        {
            returnResponse(false, getErrorString(tvERROR_INVALID_PARAM));
        }
        tvError_t ret = GetCurrentComponentLuma(blLumaColor, &luma);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            lumaColorObj["Setting"] = std::to_string(luma);
            response["luma"] = lumaColorObj;
            LOGINFO("Exit : Component Luma for Color: %s Value: %d\n", value.c_str(),luma);
            returnResponse(true, "success");
        }
    }

    bool TVMgr::isBacklightUsingGlobalBacklightFactor(void)
    {
        TR181_ParamData_t param;
        bool ret  =false;

        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM,&param);
        if ( err != tr181Success ) {
            LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_BACKLIGHT_CONTROL_USE_GBF_RFC_PARAM, getTR181ErrorString(err));
        }
        else {
            if(!std::string(param.value).compare("true"))
            {
                ret = true;
            }
        }
        return ret;
    }

    tvError_t TVMgr::SetBacklightAtInitAndSaveForBLUsingGBF(void)
    {
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param={0};
        int backlightSDR=0;
        int backlightHDR=0;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            backlightSDR=GetDriverEquivalentBLForCurrentFmt(std::stoi(param.value));
        }
        else
        {
            LOGERR("Default BL for SDR can't be read \n");
            ret = tvERROR_GENERAL;
        }

        if(ret == tvERROR_NONE)
        {
            memset(&param, 0, sizeof(param));
            err = getLocalParam(rfc_caller_id, TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM, &param);
            if ( tr181Success == err )
            {
                backlightHDR=GetDriverEquivalentBLForCurrentFmt(std::stoi(param.value));
            }
            else
            {
                LOGERR("Default BL for HDR can't be read \n");
                ret = tvERROR_GENERAL;
            }
        }

        if(ret == tvERROR_NONE)
        {
            int backlight = isCurrentHDRTypeIsSDR()?backlightSDR:backlightHDR;
            ret = SetBacklight(backlight);
            if(ret != tvERROR_NONE) {
                LOGERR("Failed to set backlight at init \n");
            }
            else
            {
                ret = SaveSDRHDRBacklightAtInitToDrv(backlightSDR,backlightHDR);
            }
        }
        return ret;
    }

    tvError_t TVMgr::saveBacklightToLocalStoreForGBF(const char* key, const char* value)
    {
        tvError_t ret = tvERROR_NONE;
        tr181ErrorCode_t err;
        const char * paramToSet = isCurrentHDRTypeIsSDR()?TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM:TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM;

        err = setLocalParam(rfc_caller_id, paramToSet, value);
        if ( err != tr181Success ) {
            LOGWARN("setLocalParam for %s Failed : %s\n", paramToSet, getTR181ErrorString(err));
            ret = tvERROR_GENERAL;
        }
        else {
            LOGINFO("setLocalParam %s Successful, Value: %s\n", paramToSet,value);
        }
        return ret;
    }

    int TVMgr::ReadBacklightFromTable(char *panelId)
    {
        tvBacklightInfo_t  blInfo = {0};

        std::string temp_panelid=std::string(panelId);
        temp_panelid=std::string(temp_panelid.rbegin(),temp_panelid.rend());//reverse
        std::string delimiter="_";
        temp_panelid=temp_panelid.erase(0, temp_panelid.find(delimiter) + delimiter.length());//remove the first _ token
        temp_panelid=std::string(temp_panelid.rbegin(),temp_panelid.rend());  //reverse again

        LOGINFO("%s: Looking at %s / %s for BLT \n",__FUNCTION__,panelId,temp_panelid.c_str());

        try
        {
            CIniFile inFile(BACKLIGHT_FILE_NAME);

            for(int i = 0 ; i < BACKLIGHT_CURVE_MAX_INDEX; i++ )
            {
                std::string s;
                s = temp_panelid+ "_" + std::string("SDR") + ".bl_" + std::to_string(i);
                blInfo.sdrBLCurve[i] = inFile.Get<int>(s);
                LOGINFO("bl_table_sdr[%d] = %u\n", i, blInfo.sdrBLCurve[i] );
            }

            for(int j = 0 ; j < BACKLIGHT_CURVE_MAX_INDEX; j++ )
            {
                std::string s;
                s = temp_panelid+ "_" + std::string("HDR") + ".bl_" + std::to_string(j);
                blInfo.hdrBLCurve[j] = inFile.Get<int>(s);
                LOGINFO("bl_table_hdr[%d] = %u\n", j, blInfo.hdrBLCurve[j] );
            }
        }

        catch(const boost::property_tree::ptree_error &e)
        {
            LOGERR("%s: error %s::config table entry not found in ini file\n",__FUNCTION__,e.what());
            return -1;
        }

        {
            blInfo.defaultBLSDR = defaultSDR;
            blInfo.defaultBLHDR = defaultHDR;
            SetBacklightInfo(blInfo);
        }
        return 0;
    }

    int TVMgr::ReadConifgEntryValues(const char* action, const char *param,unsigned int &forSource,unsigned int  &forFormat,unsigned int &forPicMode )
    {
        int ret = 0;
        try {
            CIniFile inFile(PRODUCT_CONFIG);
            string ConfigString;
            ConfigString=string(action)+"."+string(param)+"_"+string("format");
            forFormat = inFile.Get<unsigned int>(ConfigString);
            ConfigString.clear();
            ConfigString=string(action)+"."+string(param)+"_"+string("mode");
            forPicMode = inFile.Get<unsigned int>(ConfigString);
            forSource = SOURCE_SAVE_FOR_ALL; //for now save for all sources

        }
        catch(const boost::property_tree::ptree_error &e)
        {
            LOGERR("%s: error %s::config table entry not found in product_config.ini file\n",__FUNCTION__,e.what());
            ret = -1;
        }
        return ret ;
    }

    int TVMgr::GetTheSaveConfigMap(tvSaveFormatsConfig_t formatConfig, tvSavePicModesConfig_t picmodeConfig, tvSourceSaveConfig_t sourceConfig,
            std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats)
    {
        int ret  = -1;
        //get the sources
        switch(sourceConfig)
        {
            case SOURCE_SAVE_FOR_ALL:
                sources.push_back(SAVE_FOR_ALL_SOURCES);
                ret = 0;
                break;
            default:
                LOGERR("%s: error Unsupported source config %d \n",__FUNCTION__,sourceConfig);
                break;
        }

        if(!ret)
        {
            switch(picmodeConfig)
            {
                case PIC_MODE_SAVE_FOR_ALL: {
                                                int lCount = 0;
                                                for(;lCount<numberModesSupported;lCount++)
                                                {
                                                    picturemodes.push_back(pic_mode_index[lCount]);
                                                }
                                                break;
                                            }
                case PIC_MODE_FORMAT_SAVE_FOR_CURRENT: {
                                                           char picMode[PIC_MODE_NAME_MAX]={0};
                                                           if(!getCurrentPictureMode(picMode))
                                                           {
                                                               LOGWARN("Failed to get the current picture mode\n");
                                                               ret =-1;
                                                           }
                                                           else
                                                               picturemodes.push_back(GetTVPictureModeIndex(picMode));
                                                           break;
                                                       }
                default: {
                             LOGERR("%s: error Unsupported picmode config %d \n",__FUNCTION__,picmodeConfig);
                             ret=-1;
                             break;
                         }
            }

            if(!ret)
            {
                unsigned int contentFormats=0;
                unsigned short numberOfSupportedFormats =  0;
                GetSupportedContentFormats(&contentFormats,&numberOfSupportedFormats);
                //get the formats
                switch(formatConfig)
                {
                    case CONTENT_FORMAT_SAVE_FOR_ALL:
                    case CONTENT_FORMAT_SAVE_FOR_NON_DV:
                        {
                            unsigned int lcount=0;
                            for(;(lcount<sizeof(uint32_t)*8 && numberOfSupportedFormats);lcount++)
                            {
                                tvhdr_type_t formatToStore = (tvhdr_type_t)ConvertVideoFormatToHDRFormat((tvVideoHDRFormat_t)(contentFormats&(1<<lcount)));
                                if(formatToStore!= HDR_TYPE_NONE)
                                {
                                    numberOfSupportedFormats--;
                                    if((formatConfig==CONTENT_FORMAT_SAVE_FOR_NON_DV) && (formatToStore == HDR_TYPE_DOVI))
                                        continue;
                                    formats.push_back(formatToStore);
                                }
                            }
                        }
                        break;
                    case CONTENT_FORMAT_SAVE_FOR_CURRENT:
                        if( HDR_TYPE_NONE == ConvertVideoFormatToHDRFormat(GetCurrentContentFormat()))
                            formats.push_back(HDR_TYPE_SDR);//Save  To SDR if format is HDR_TYPE_NONE
                        else
                            formats.push_back(ConvertVideoFormatToHDRFormat(GetCurrentContentFormat()));
                        break;

                    case CONTENT_FORMAT_SAVE_FOR_GROUPED:
                        if( isCurrentHDRTypeIsSDR() )
                        {
                            formats.push_back(HDR_TYPE_SDR);
                        }
                        else
                        {
                            //getSupported HDR Group -
                            formats.push_back(HDR_TYPE_HDR10);
                            formats.push_back(HDR_TYPE_HLG);
                            formats.push_back(HDR_TYPE_DOVI);
                        }
                        break;

                    case CONTENT_FORMAT_SAVE_FOR_DV_ONLY:
                        formats.push_back(HDR_TYPE_DOVI);
                        break;
                    
                    case CONTENT_FORMAT_SAVE_FOR_HDR10_ONLY:
                        formats.push_back(HDR_TYPE_HDR10);
                        break;

                    case CONTENT_FORMAT_SAVE_FOR_HLG_ONLY:
                        formats.push_back(HDR_TYPE_HLG);
                        break;

                    default:
                        LOGERR("%s: error Unsupported Format config %d \n",__FUNCTION__,formatConfig);
                        ret=-1;
                        break;
                }

            }
        }
        return ret;
    }

    int TVMgr :: UpdatePQParamsToCache(const char *action,const char *pqParamName ,const char *tr181ParamName,tvPQParameterIndex_t pqParamIndex,int * params)
    {
        unsigned int sourceConfig,formatConfig, picmodeConfig;
        std::vector<int> sources;
        std::vector<int> picturemodes;
        std::vector<int> formats;
        bool sync = !strncmp(action,"sync",strlen("sync"));
        bool reset = !strncmp(action,"reset",strlen("reset"));
        bool set = !strncmp(action,"set",strlen("set"));
        int ret=0;
        ret = ReadConifgEntryValues(action, pqParamName,sourceConfig,formatConfig,picmodeConfig);
        if(0 == ret)
        {
            ret = GetTheSaveConfigMap((tvSaveFormatsConfig_t)formatConfig, (tvSavePicModesConfig_t) picmodeConfig, (tvSourceSaveConfig_t) sourceConfig,
                            sources,picturemodes,formats);
            if(0 == ret)
            {
                for(int source: sources)
                {
                    for(int mode : picturemodes)
                    {
                        for(int format : formats)
                        {

                            switch(pqParamIndex)
                            {
                                case PQ_PARAM_BRIGHTNESS:
                                case PQ_PARAM_CONTRAST:
                                case PQ_PARAM_BACKLIGHT:
                                case PQ_PARAM_SATURATION:
                                case PQ_PARAM_SHARPNESS:
                                case PQ_PARAM_HUE:
                                case PQ_PARAM_COLOR_TEMPERATURE:

                                    if(reset)
                                        ret |= UpdatePQParamToLocalCache(tr181ParamName,mode, format,0,false);

                                    if(sync || reset)
                                    {
                                        int value=0;
                                        if(!GetPQParamsToSync(tr181ParamName,mode,format,value))
                                            LOGINFO("Found param from tr181 %s pqmode : %d format:%d value:%d\n",tr181ParamName,mode,format,value);
                                        else
                                            LOGINFO("value not found in tr181 %s pqmode : %d format:%d value:%d\n",tr181ParamName,mode,format,value);

                                        params[0]=value;
                                    }

                                    if(set)
                                        ret |= UpdatePQParamToLocalCache(tr181ParamName,mode, format, params[0],true);

                                    break;
                                default :
                                    break;

                            }

                            switch(pqParamIndex)
                            {
                                case PQ_PARAM_BRIGHTNESS:
                                    ret |= SaveBrightness(source, mode,format,params[0]);
                                    break;
                                case PQ_PARAM_CONTRAST:
                                    ret |= SaveContrast(source, mode,format,params[0]);
                                    break;
                                case PQ_PARAM_SHARPNESS:
                                    ret |= SaveSharpness(source, mode,format,params[0]);
                                    break;
                                case PQ_PARAM_SATURATION:
                                    ret |= SaveSaturation(source, mode,format,params[0]);
                                    break;
                                case PQ_PARAM_BACKLIGHT:
                                    ret |= SaveBacklight(source, mode,format,params[0]);
                                    break;
                                case PQ_PARAM_HUE:
                                    ret |= SaveHue(source, mode,format,params[0]);
                                    break;
                                case PQ_PARAM_COLOR_TEMPERATURE:
                                    ret |= SaveColorTemperature(source, mode,format,params[0]);
                                    break;

                                case PQ_PARAM_DOLBY_MODE:
                                    if(sync)
                                    {
                                        int value=0;
                                        if( !GetDolbyParamToSync(value) )
                                            LOGINFO("Found param from tr181 dvmode pqmode : %d format:%d value:%d\n",mode,format,value);
                                        else
                                            LOGINFO("value not found in tr181 dvmode pqmode : %d format:%d value:%d\n",mode,format,value);

                                        params[0]=value;
                                    }
                                    ret |= SaveDolbyMode(source, mode,format,params[0]);
                                    break;

                                case PQ_PARAM_CMS:
                                    if(reset)
                                        ret |= UpdatePQParamToLocalCache(tr181ParamName,mode, format,0,false);
                                    if(sync || reset)
                                    {
                                        int value=0;
                                        if(!GetPQParamsToSync(tr181ParamName,mode,format,value,true,params[0]))
                                            LOGINFO("Found param from tr181 CMS pqmode : %d format:%d value:%d\n",mode,format,value);
                                        else{
                                            if(sync) /*block default cms sync to save tvsettings init time*/
                                                break;
                                        }
                                        params[2]=value;
                                    }
                                    ret |= SaveCMS(source, mode,format,params[0],params[1],params[2]);
                                    if(set)
                                        ret |= UpdatePQParamToLocalCache(tr181ParamName,mode, format, params[2],true);
                                    break;
                                case PQ_PARAM_ASPECT_RATIO:
                                    ret |= SaveDisplayMode(source,mode,format,params[0]);
                                    break;
                                case PQ_PARAM_LDIM:
                                    if(sync)
                                    {
                                        int value=0;
                                        if(!GetLDIMParamsToSync(value,mode))
                                            LOGINFO("Found param from tr181 ldim pqmode : %d format:%d value:%d\n",mode,format,value);
                                        else
                                            LOGINFO("value not found in tr181 ldim pqmode : %d format:%d value:%d\n",mode,format,value);

                                        params[0]=value;
                                    }
                                    ret |= SaveDynamicBacklight(source,mode,format,params[0]);
                                    break;

                                case PQ_PARAM_HDR10_MODE:
                                    if(sync){
                                        int value=0;
                                        if( !GetHDR10ParamToSync(value) )
                                            LOGINFO("Found param from tr181 hdr10mode pqmode : %d format:%d value:%d\n",mode,format,value);
                                        else
                                            LOGINFO("value not found in tr181 hdr10mode pqmode : %d format:%d value:%d\n",mode,format,value);

                                        params[0]=value;
                                    }
                                    //SaveDolbymode api is support for HLG and HDR10 also, difference is only 4th parameter
                                    ret |= SaveDolbyMode(source, mode,format,params[0]);
                                    break;

                                case PQ_PARAM_HLG_MODE:
                                    if(sync){
                                        int value=0;
                                        if( !GetHLGParamToSync(value) )
                                            LOGINFO("Found param from tr181 hlgmode pqmode : %d format:%d value:%d\n",mode,format,value);
                                        else
                                            LOGINFO("value not found in tr181 hlgmode pqmode : %d format:%d value:%d\n",mode,format,value);

                                        params[0]=value;
                                    }
                                    //SaveDolbymode api is support for HLG and HDR10 also, difference is only 4th parameter
                                    ret |= SaveDolbyMode(source, mode,format,params[0]);
                                    break;
                            }
                        }
                    }
                }
            }
        }
        return ret;
    }

    uint32_t TVMgr::resetComponentSaturation(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        int ret = 0;
        tvError_t err=tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        if(err == tvERROR_NONE)
        {
	    int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            ret=UpdatePQParamsToCache("reset","compsat","cms.enable",PQ_PARAM_CMS,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_SATURATION;
                params[1]=color;
                snprintf(param,sizeof(param),"saturation.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset","compsat",param,PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentSaturation Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }
        }
        if(err != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(err));
        }
        else
        {
            LOGINFO("Exit : resetComponentSaturation()\n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetComponentHue(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        int ret = 0;
        tvError_t err=tvERROR_NONE;
        char param[BUFFER_SIZE]={0};

        err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
	if(err == tvERROR_NONE)
        {
            int params[3]={0};
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            ret=UpdatePQParamsToCache("reset","comphue","cms.enable",PQ_PARAM_CMS,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_HUE;
                params[1]=color;
                snprintf(param,sizeof(param),"hue.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset","comphue",param,PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                LOGWARN("resetComponentHue Failed couldn't remove from localstore error %d \n", ret);
                err=tvERROR_GENERAL;
            }    
        }
        if(err != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(err));
        }
        else
        {
            LOGINFO("Exit : resetComponentHue()\n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetComponentLuma(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO();
        PLUGIN_Lock(tvLock);
        int ret = 0;
        tvError_t err=tvERROR_NONE;
        char param[BUFFER_SIZE]={0};
        int params[3]={0};
   
        err = SetCMSState(COLOR_STATE,COLOR_ENABLE,COMPONENT_DISABLE);
        if( err == tvERROR_NONE )
        {
            params[0]=COLOR_STATE;
            params[1]=COLOR_ENABLE;
            ret=UpdatePQParamsToCache("reset","compluma","cms.enable",PQ_PARAM_CMS,params);
            if(ret != 0) {
                LOGWARN("Failed to Save enable flag to ssm_data\n");
            }

            for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
            {
                params[0]=COLOR_LUMA;
                params[1]=color;
                snprintf(param,sizeof(param),"luma.%s",component_color[color]);
                ret |= UpdatePQParamsToCache("reset","compluma",param,PQ_PARAM_CMS,params);
                memset(&param, 0, sizeof(param));
            }

            if(ret != 0)
            {
                err=tvERROR_GENERAL;
                LOGWARN("resetComponentluma Failed couldn't remove from localstore error %d \n", ret);
            }
        }

        if(err != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(err));
        }
        else
        {
            LOGINFO("Exit : resetComponentLuma()\n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::generateStorageIdentifier(string &key,const char * forParam,uint32_t contentFormat)
    {
        TR181_ParamData_t param;
        memset(&param, 0, sizeof(param));

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
        if ( err == tr181Success ) {
            key+=string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
            key+=STRING_PICMODE+std::to_string(GetTVPictureModeIndex(param.value))+string(".")+string(STRING_FORMAT)+std::to_string(contentFormat);
            key+=string(".")+forParam;
        }
        else
        {
            LOGWARN("getLocalParam failed %s \n",TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
            key="";
        }
        return tvERROR_NONE;
    }

    int TVMgr::GetDimmingModeIndex(const char* mode)
    {
        unsigned short index = 1;

        if(strncmp(mode,"local",strlen("local"))==0)
            index=tvDimmingMode_Local;
        else if(strncmp(mode,"fixed",strlen("fixed"))==0)
            index=tvDimmingMode_Fixed;
        else if(strncmp(mode,"global",strlen("global"))==0)
            index=tvDimmingMode_Global;
        else
            LOGWARN("Return Default Dimmingmode:%d!!!\n",index);

        return index;

    }

    int TVMgr::GetPQParamsToSync (const char *forParam,int pqmode,int format, int& value,bool cms,int tunnel_type)
    {
        int ret=0;
        TR181_ParamData_t param={0};
        string key;
        tr181ErrorCode_t err;

        format=ConvertHDRFormatToContentFormat((tvhdr_type_t)format);
        key.clear();
        generateStorageIdentifierDirty(key,forParam,format,pqmode);

        err=getLocalParam(rfc_caller_id, key.c_str(), &param);
        if ( tr181Success == err )
        {
            if(strncmp(forParam,"ColorTemp",strlen(forParam))==0)
            {
                if (strncmp(param.value, "Standard", strlen(param.value))==0)
                    value=tvColorTemp_STANDARD;
                else if (strncmp(param.value, "Warm", strlen(param.value))==0)
                    value=tvColorTemp_WARM;
                else if (strncmp(param.value, "Cold", strlen(param.value))==0)
                    value=tvColorTemp_COLD;
                else if (strncmp(param.value, "User Defined", strlen(param.value))==0)
                    value=tvColorTemp_USER;
                else
                    value=tvColorTemp_STANDARD;
            }
            else
                value=std::stoi(param.value);
        }
        else
        {
            if(cms)
            {
                value=GetCMSDefault((tvCMS_tunel_t)tunnel_type);
                ret =  -1;/*block default cms sync and allow default values during reset*/
            }
            else
            {
                key.clear();
                memset(&param, 0, sizeof(param));
                key+=string(TVSETTINGS_GENERIC_STRING_RFC_PARAM);
                key+=STRING_PICMODE+std::to_string(pqmode)+string(".")+string(STRING_FORMAT)+std::to_string(format);
                key+=string(".")+forParam;
                err=getLocalParam(rfc_caller_id, key.c_str(), &param);
                if ( tr181Success == err ) {
                    value=std::stoi(param.value);
                    LOGINFO("GetPQParamsToSync : found default %d \n",value);
                }
                else
                {
                    LOGWARN("Default not found %s \n",key.c_str());
                    ret = -1;
                }
            }

        }
        return ret;
    }

    int TVMgr::SyncCMSParams(const char *configParam, const char *pqParam,tvCMS_tunel_t tunnel_type)
    {
        int ret=0;
        char cms_param[BUFFER_SIZE]={0};
        int params[3]={0};

        for(int color=COLOR_RED;color<=COLOR_YELLOW;color++)
        {
            memset(&cms_param, 0, sizeof(cms_param));
            snprintf(cms_param,sizeof(cms_param),"%s.%s",pqParam,component_color[color]);
            params[0]=tunnel_type;//tunnel_type
            params[1]=color;//color_type
            params[2]=0;//value

            if(!UpdatePQParamsToCache("sync",configParam,cms_param,PQ_PARAM_CMS,params))
                ret |= 0;
            else
                ret |= 1;
        }
        return ret;
    }

    int TVMgr::GetDolbyParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=GetDolbyModeIndex(param.value);
        }
        else
        {
            LOGWARN("Unable to fetch %s from localstore\n",TVSETTINGS_DOLBYVISIONMODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    int TVMgr::GetLDIMParamsToSync(int &value,int mode)
    {
        int ret=0;
        TR181_ParamData_t param;
        char format[BUFFER_SIZE]={0};

        memset(&param, 0, sizeof(param));
        snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,mode,"DimmingMode");
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, format,&param);
        if ( tr181Success == err )
        {
            value=GetDimmingModeIndex(param.value);
        }
        else
        {
            LOGWARN("Unable to fetch %s from localstore\n",format);
            ret=-1;
        }
        return ret;
    }

    int TVMgr::InitializeAndSyncLDIM()
    {
        int ret=0;
        char format[BUFFER_SIZE]={0};
        int params[3]={0};//Set param array members to zero
        TR181_ParamData_t param;

        //MigrationCode starts
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_DIMMING_MODE_RFC_PARAM, &param);

        if ( tr181Success == err )
        {
            ret = tvERROR_NONE;
            LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_DIMMING_MODE_RFC_PARAM, param.value);

            //Set Old format LDIM values to New Format (Set it for Custom Picture Mode)
            snprintf(format,sizeof(format),"%s%d.%s",TVSETTINGS_GENERIC_STRING_RFC_PARAM,GetCustomPQModeIndex(),"DimmingMode");
            err = setLocalParam(rfc_caller_id, format, param.value);
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", format, getTR181ErrorString(err));
            }
            else
                LOGINFO("setLocalParam copied old dimming mode value to new formt for custom mode(migration)\n");
        }
        else
        {
            LOGWARN("getLocalParam for %s Not available in localstore\n", TVSETTINGS_DIMMING_MODE_RFC_PARAM);
        }
        //End of Migration code for LDIM

        //Sync ldim to Driver cache
        if( !UpdatePQParamsToCache("sync","ldim","DimmingMode",PQ_PARAM_LDIM,params))
            LOGINFO("ldim Successfully Synced to Drive Cache\n");
        else
        {
            ret=-1;
            LOGWARN("ldim Sync to cache Failed !!!\n");
        }
        return ret;
    }

    int TVMgr::InitializeSDRHDRBacklight()
    {
        char panelId[20] = {0};
        int val=GetPanelID(panelId);
        int ret=0;

        if (val != 0)
        {
            LOGERR("Failed to read panel id!!! Set 55 panel as default\n");
            memset(panelId,0,sizeof(panelId));
            GetDefaultPanelID(panelId);
            LOGINFO("Panel ID : %s \n",panelId);
        }
        else
            LOGINFO("Read panel id ok [%s] \n", panelId);

        if(strncmp(panelId,"0_0_00",strlen("0_0_00"))==0)
        {
            memset(panelId,0,sizeof(panelId));
            GetDefaultPanelID(panelId);
            LOGINFO("Load 55 panel values as default panel ID : %s\n",panelId);
        }

        val=ReadBacklightFromTable(panelId);
        if(val == 0)
            LOGINFO("Backlight read success from backlight_default.ini\n");
        else
            LOGWARN("Backlight read failed from backlight_default.ini\n");

        ret=SetBacklightAtInitAndSaveForBLUsingGBF();
        if(ret == tvERROR_NONE)
        {
            LOGINFO("SetBacklightAtInitAndSaveForBLUsingGBF: success\n");
        }
        else
        {
            ret=-1;
            LOGWARN("SetBacklightFromLocalStore(): Failed\n");
        }
        return ret;
    }

    void TVMgr::LocatePQSettingsFile()
    {
        LOGINFO("Entry\n");
        char panelId[20] = {0};
        std::string PQFileName = TVSETTINGS_RFC_CALLERID;
        std::string FilePath = "/etc/rfcdefaults/";

        int val=GetPanelID(panelId);
        if(val==0)
        {
            LOGINFO("%s : panel id read is : %s\n",__FUNCTION__,panelId);
            if(strncmp(panelId,"0_0_00",strlen("0_0_00"))!=0)
            {
                struct stat tmp_st;

                PQFileName+=std::string("_")+panelId;
                LOGINFO("%s: Looking for %s.ini \n",__FUNCTION__,PQFileName.c_str());
                if(stat((FilePath+PQFileName+std::string(".ini")).c_str(), &tmp_st)!=0)
                {
                    //fall back
                    LOGINFO("%s not available in %s Fall back to default\n",PQFileName.c_str(),FilePath.c_str());
                    PQFileName =std::string(TVSETTINGS_RFC_CALLERID);
                }
            }
        }
        else
            LOGINFO("%s : GetPanelID failed : %d\n",__FUNCTION__,val);

        strncpy(rfc_caller_id,PQFileName.c_str(),PQFileName.size());
        LOGINFO("%s : Default tvsettings file : %s\n",__FUNCTION__,rfc_caller_id);
    }

    tvError_t TVMgr::UpdatePQParamToLocalCache(const char* forParam, int pqmode, int format, int value,bool setNotDelete)
    {
        tvError_t ret = tvERROR_NONE;
        std::string key;

        if((!strncmp(forParam,"Backlight",strlen("Backlight")))&&
            appUsesGlobalBackLightFactor)
        {
            /* Do nothing this global BLF using single values for SDR and HDR
             * stored only once per format and handled separately
             */
            return ret;
        }

        format=ConvertHDRFormatToContentFormat((tvhdr_type_t)format);
        key.clear();
        generateStorageIdentifierDirty(key,forParam,format,pqmode);
        if(key.empty())
        {
            LOGWARN("generateStorageIdentifierDirty failed\n");
            ret = tvERROR_GENERAL;
        }
        else
        {
            tr181ErrorCode_t err  = tr181Success;
            if(setNotDelete)
            {
                std::string toStore = std::to_string(value);
                if (!strncmp(forParam,"ColorTemp",strlen("ColorTemp")))
                {
                    GetColorTempStringFromEnum(value, toStore);
                }
                err = setLocalParam(rfc_caller_id, key.c_str(),toStore.c_str());

            }
            else
                err = clearLocalParam(rfc_caller_id, key.c_str());

            if ( err != tr181Success ) {
                LOGWARN("%s for %s Failed : %s\n", setNotDelete?"Set":"Delete", key.c_str(), getTR181ErrorString(err));
                ret = tvERROR_GENERAL;
            }
            else {
                LOGINFO("%s for %s Successful \n", setNotDelete?"Set":"Delete",key.c_str());
            }
        }
        return ret;
    }

    tvDataComponentColor_t TVMgr::GetComponentColorEnum(std::string colorName)
    {
        tvDataComponentColor_t CompColorEnum = tvDataColor_MAX;

        if(!colorName.compare("none")) {
            CompColorEnum = tvDataColor_NONE;
        }
        else if (!colorName.compare("red")){
            CompColorEnum = tvDataColor_RED;
        }
        else if (!colorName.compare("green")){
            CompColorEnum = tvDataColor_GREEN;
        }
        else if (!colorName.compare("blue")){
            CompColorEnum = tvDataColor_BLUE;
        }
        else if (!colorName.compare("yellow")){
            CompColorEnum = tvDataColor_YELLOW;
        }
        else if (!colorName.compare("cyan")){
            CompColorEnum = tvDataColor_CYAN;
        }
        else if (!colorName.compare("magenta")){
            CompColorEnum = tvDataColor_MAGENTA;
        }
        return CompColorEnum;
    }

    tvError_t TVMgr::SyncPQParamsToDriverCache()
    {
        int params[3]={0};

        if( !UpdatePQParamsToCache("sync","brightness","Brightness",PQ_PARAM_BRIGHTNESS,params))
            LOGINFO("Brightness Successfully sync to Drive Cache\n");
        else
            LOGWARN("Brightness Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","contrast","Contrast",PQ_PARAM_CONTRAST,params))
            LOGINFO("Contrast Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Contrast Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","sharpness","Sharpness",PQ_PARAM_SHARPNESS,params))
            LOGINFO("Sharpness Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Sharpness Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","saturation","Saturation",PQ_PARAM_SATURATION,params))
            LOGINFO("Saturation Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Saturation Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","hue","Hue",PQ_PARAM_HUE,params))
            LOGINFO("Hue Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Hue Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","colortemp","ColorTemp",PQ_PARAM_COLOR_TEMPERATURE,params))
            LOGINFO("ColorTemp Successfully Synced to Drive Cache\n");
        else
            LOGWARN("ColorTemp Sync to cache Failed !!!\n");

        if( !UpdatePQParamsToCache("sync","dvmode","DolbyVisionMode",PQ_PARAM_DOLBY_MODE,params))
            LOGINFO("dvmode Successfully Synced to Drive Cache\n");
        else
            LOGWARN("dvmode Sync to cache Failed !!!\n");

        if(appUsesGlobalBackLightFactor){
            if( !UpdatePQParamsToCache("sync","hlgmode","HLGMode",PQ_PARAM_HLG_MODE,params))
                LOGINFO("hlgmode Successfully Synced to Drive Cache\n");
            else
                LOGWARN("hlgmode Sync to cache Failed !!!\n");

            if( !UpdatePQParamsToCache("sync","hdr10mode","HDR10Mode",PQ_PARAM_HDR10_MODE,params))
                LOGINFO("hdr10mode Successfully Synced to Drive Cache\n");
            else
                LOGWARN("hdr10mode Sync to cache Failed !!!\n");
        }

        if( !UpdatePQParamsToCache("sync","backlight","Backlight",PQ_PARAM_BACKLIGHT,params) )
            LOGINFO("Backlight Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Backlight Sync to cache Failed !!!\n");
            
        if(!InitializeAndSyncLDIM())
                LOGINFO("InitializeAndSyncLDIM() : Success\n");
        else
                LOGWARN("InitializeAndSyncLDIM() : Failed!!!\n");

        if(appUsesGlobalBackLightFactor)
        {
            SyncCMSParamsToDriverCache();

            SyncWBparams();
        }
        return tvERROR_NONE;
    }

    tvError_t TVMgr::SyncCMSParamsToDriverCache()
    {
        int cms_enable[3]={0};
        cms_enable[0]=COLOR_STATE;//tunel_type
        cms_enable[1]=COLOR_ENABLE;//color_type
        cms_enable[2]=0;//value

        if(! UpdatePQParamsToCache("sync","compsat","cms.enable",PQ_PARAM_CMS,cms_enable))
            LOGINFO("CMS Enable Flag  Successfully Synced to Drive Cache\n");
        else
            LOGWARN("CMS Enable Flag Sync to cache Failed !!!\n");

        if( !SyncCMSParams("compsat","saturation",COLOR_SATURATION))
            LOGINFO("Component saturation Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Component saturation Sync to cache Failed !!!\n");

        if( !SyncCMSParams("comphue","hue",COLOR_HUE))
            LOGINFO("Component hue Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Component hue to cache Failed !!!\n");

        if( !SyncCMSParams("compluma","luma",COLOR_LUMA))
            LOGINFO("Component Luma Successfully Synced to Drive Cache\n");
        else
            LOGWARN("Component Luma Sync to cache Failed !!!\n");

        return tvERROR_NONE;
    }

    void TVMgr::GetColorTempStringFromEnum(int value, std::string &toStore)
    {
        const char *color_temp_string[] = {
                    [tvColorTemp_STANDARD] = "Standard",
                    [tvColorTemp_WARM] = "Warm",
                    [tvColorTemp_COLD] = "Cold",
                    [tvColorTemp_USER] = "User Defined"
                };
        toStore.clear();
        toStore+=color_temp_string[value];
    }
    uint32_t TVMgr::resetWBCtrl(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;
        string identifier=(std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb")+std::string(STRING_DIRTY));

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,identifier.c_str());
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n",identifier.c_str(),getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }else{
            tvDataColor_t WBValues={0};
            //get the defaults
            ret = GetCustomWBValuesFromLocalCache(WBValues);
            if(ret == tvERROR_NONE)
            {
                //set the defaults and save
                ret = SyncCustomWBValuesToDriverCache(WBValues,true);
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : Successfully reset user WB Entries \n");
            returnResponse(true, "success");
        }
    }

    tvError_t TVMgr::GetCustomWBValuesFromLocalCache(tvDataColor_t &WBValues)
    {
        TR181_ParamData_t param={0};
        tvError_t ret = tvERROR_NONE;
        std::vector<std::string> allCtrls{ "gain", "offset"};
        std::vector<std::string> allColors{ "red", "green","blue"};

        LOGINFO("Entry");
        for(std::string color : allColors)
        {
            for(std::string ctrl : allCtrls)
            {
                string identifier=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb");
                int value =0;
                tr181ErrorCode_t err;
                identifier+=std::string(STRING_DIRTY)+color+"."+ctrl;
                err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                if ( tr181Success == err )
                {
                    value = std::stoi(param.value);
                    LOGINFO("%s  : %d\n",identifier.c_str(),value);
                }
                else
                {
                    LOGINFO("%s  Failed trying dirty\n",identifier.c_str());
                    identifier.clear();
                    identifier+=std::string(TVSETTINGS_GENERIC_STRING_RFC_PARAM)+std::string("wb.")+color+"."+ctrl;
                    err = getLocalParam(rfc_caller_id, identifier.c_str(), &param);
                    if ( tr181Success == err )
                    {
                        value = std::stoi(param.value);
                        LOGINFO("%s  : %d\n",identifier.c_str(),value);

                    }
                    else
                    {
                        LOGWARN("Not finding entry for %s  : %s\n",identifier.c_str(),getTR181ErrorString(err));
                        ret = tvERROR_GENERAL;
                    }
                }

                if(tr181Success == err )
                {
                    switch(GetWBRgbType(color.c_str(),ctrl.c_str()))
                    {
                        case R_GAIN:
                            WBValues.r_gain = value;
                            break;
                        case G_GAIN:
                            WBValues.g_gain = value;
                            break;
                        case B_GAIN:
                            WBValues.b_gain = value;
                           break;
                        case R_POST_OFFSET:
                            WBValues.r_offset = value;
                            break;
                        case G_POST_OFFSET:
                            WBValues.g_offset = value;
                            break;
                        case B_POST_OFFSET:
                            WBValues.b_offset = value;
                            break;
                    }
                }
            }
        }
        LOGINFO("r.gain %d,r.offset %d,g.gain %d,g.offset %d,b.gain %d,b.offset %d",WBValues.r_gain,WBValues.r_offset,
        WBValues.g_gain,WBValues.g_offset,WBValues.b_gain,WBValues.b_offset);
        LOGINFO("Exit");
        return ret;
    }

    int TVMgr::GetGainOffsetValue(const char* color,const char*ctrl,tvDataColor_t WBValues,int &rgbType)
    {
        int value = 0;
        rgbType = GetWBRgbType(color,ctrl);
        switch(rgbType)
        {
            case R_GAIN:
                value = WBValues.r_gain;
                break;
            case G_GAIN:
                value = WBValues.g_gain;
                break;
            case B_GAIN:
                value = WBValues.b_gain;
                break;
            case R_POST_OFFSET:
                value = WBValues.r_offset;
                break;
            case G_POST_OFFSET:
                value = WBValues.g_offset;
                break;
            case B_POST_OFFSET:
                value = WBValues.b_offset;
                break;
        }
        return value;
    }

    tvError_t TVMgr::SyncCustomWBValuesToDriverCache(tvDataColor_t WBValues,bool setDuringSync)
    {
        tvError_t ret = tvERROR_NONE;
        std::vector<std::string> allCtrls{ "gain", "offset"};
        std::vector<std::string> allColors{ "red", "green","blue"};

        for(std::string color : allColors)
        {
            for(std::string ctrl : allCtrls)
            {
                int rgbType;
                int value = GetGainOffsetValue(color.c_str(),ctrl.c_str(),WBValues,rgbType);

                if(setDuringSync)
                    ret  = SetColorTemperatureUser(rgbType,value);

                if(tvERROR_NONE == ret)
                    ret  = SaveColorTemperatureUser(rgbType,value);

                if(tvERROR_NONE!= ret)
                {
                    LOGWARN("WB Entry for %s.%s fail to save to driver\n",color.c_str(),ctrl.c_str());
                }
                else
                    LOGINFO("WB Entry for %s.%s saved to driver\n",color.c_str(),ctrl.c_str());
            }
        }
        return ret;
    }

    void TVMgr::SyncWBparams(void)
    {
        tvDataColor_t WbValuesFromCache={0};
        tvDataColor_t WbValueFromDrv={0};
        tvDataColor_t WbValueAllZero={0};
        bool WbvalueFromDrvIsDefault = false;
        bool WbvalueInLocalCacheIsDefault = false;

        GetCustomWBValuesFromLocalCache(WbValuesFromCache);
        LOGINFO("cached:r.gain %d,r.offset %d,g.gain %d,g.offset %d,b.gain %d,b.offset %d",WbValuesFromCache.r_gain,WbValuesFromCache.r_offset,
        WbValuesFromCache.g_gain,WbValuesFromCache.g_offset,WbValuesFromCache.b_gain,WbValuesFromCache.b_offset);
        WbValueFromDrv = GetUSerWBValueOnInit();
        LOGINFO("cached:r.gain %d,r.offset %d,g.gain %d,g.offset %d,b.gain %d,b.offset %d",WbValueFromDrv.r_gain,WbValueFromDrv.r_offset,
        WbValueFromDrv.g_gain,WbValueFromDrv.g_offset,WbValueFromDrv.b_gain,WbValueFromDrv.b_offset);
        WbvalueFromDrvIsDefault = isWBUserDfault(WbValueFromDrv);
        WbvalueInLocalCacheIsDefault = isWBUserDfault(WbValuesFromCache);
        LOGINFO("Drv WB deafult:%s , tr181 WB default:%s\n",WbvalueFromDrvIsDefault?"Yes":"No",
            WbvalueInLocalCacheIsDefault?"Yes":"No");
        if(!WbvalueInLocalCacheIsDefault &&
            !areEqual(WbValuesFromCache,WbValueFromDrv) &&
            !areEqual(WbValuesFromCache,WbValueAllZero) )
        {
            //RDK->Driver
            LOGINFO("RDK->Driver\n");
            SyncCustomWBValuesToDriverCache(WbValuesFromCache,false);

        }
        else 
        {
            LOGINFO("No need to sync WB params\n");
        }

    }

    tvError_t TVMgr::GetLastSetBacklightForGBF(int &backlight)
    {
        tvError_t ret = tvERROR_NONE;
        TR181_ParamData_t param={0};

        const char * paramToRead = isCurrentHDRTypeIsSDR()?TVSETTINGS_BACKLIGHT_SDR_RFC_PARAM:TVSETTINGS_BACKLIGHT_HDR_RFC_PARAM;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, paramToRead, &param);
        if ( tr181Success == err )
        {
            backlight = std::stoi(param.value);
        }
        else
        {
            LOGWARN("Reading %s fails \n",paramToRead);
            ret = tvERROR_GENERAL;
        }

        return ret;
    }

    tvError_t TVMgr::SaveSDRHDRBacklightAtInitToDrv(int backlightSDR,int backlightHDR)
    {
        tvError_t ret = tvERROR_NONE;

        for(int lCount=0;lCount<numberModesSupported;lCount++)
        {
            if(tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_SDR,backlightSDR)||
                tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_HDR10,backlightHDR) ||
                tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_HLG,backlightHDR) ||
                tvERROR_NONE != SaveBacklight(SAVE_FOR_ALL_SOURCES, pic_mode_index[lCount],(int)HDR_TYPE_DOVI,backlightHDR))
            {
                ret = tvERROR_GENERAL;
                LOGWARN("BL update failed for picmode %d\n",pic_mode_index[lCount]);
                break;
            }
        }
        return ret;
    }

    uint32_t TVMgr::resetPictureMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tr181ErrorCode_t err = tr181Success;

        err = clearLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, getTR181ErrorString(err));
            returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            TR181_ParamData_t param = {0};
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
            err = getLocalParam(rfc_caller_id, TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, &param);
            if ( tr181Success == err )
            {
                LOGINFO("getLocalParam for %s is %s\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM, param.value);
                tvError_t ret = SetTVPictureMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("Picture Mode set failed: %s\n",getErrorString(ret).c_str());
                    returnResponse(false, getErrorString(ret).c_str());
                }
                else {
                    LOGINFO("Picture Mode reset successfully, value: %s\n", param.value);
                    returnResponse(true, "success");
                }
            }
            else {
                LOGWARN("getLocalParam for %s failed\n", TVSETTINGS_PICTUREMODE_STRING_RFC_PARAM);
                returnResponse(false, getErrorString(tvERROR_GENERAL).c_str());
            }
        }
    }

    uint32_t TVMgr::getHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["HLGMode"] = s;
            LOGINFO("Exit getHLGMode(): %s\n",s.c_str());
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::getHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        TR181_ParamData_t param;

        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, &param);
        if (err!= tr181Success) {
            returnResponse(false,getErrorString(tvERROR_GENERAL).c_str());
        }
        else {
            std::string s;
            s+=param.value;
            response["HDR10Mode"] = s;
            LOGINFO("Exit getHDR10Mode(): %s\n",s.c_str());
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::setHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;

        value = parameters.HasLabel("HLGMode") ? parameters["HLGMode"].String() : "";
        returnIfParamNotFound(value);

        tvError_t ret = SetTVHLGMode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=GetHLGModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","hlgmode","HLGMode",PQ_PARAM_HLG_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HLG Mode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::setHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string value;

        value = parameters.HasLabel("HDR10Mode") ? parameters["HDR10Mode"].String() : "";
        returnIfParamNotFound(value);

        tvError_t ret = SetTVHDR10Mode(value.c_str());

        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            int params[3]={0};
            params[0]=GetHDR10ModeIndex(value.c_str());
            int retval=UpdatePQParamsToCache("set","hdr10mode","HDR10Mode",PQ_PARAM_HDR10_MODE,params);
            if(retval != 0) {
                LOGWARN("Failed to Save HDR10 Mode to ssm_data\n");
            }
            tr181ErrorCode_t err = setLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, value.c_str());
            if ( err != tr181Success ) {
                LOGWARN("setLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            }
            else {
                LOGINFO("setLocalParam for %s Successful, Value: %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, value.c_str());
            }

            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }
    }

    int TVMgr::GetHDR10ParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=GetHDR10ModeIndex(param.value);
        }
        else 
        {
            LOGWARN("Unable to fetch %s from localstore\n",TVSETTINGS_HDR10MODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    int TVMgr::GetHLGParamToSync(int& value)
    {
        int ret=0;
        TR181_ParamData_t param;

        memset(&param, 0, sizeof(param));
        tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM, &param);
        if ( tr181Success == err )
        {
            value=GetHLGModeIndex(param.value);
        }
        else 
        {
            LOGWARN("Unable to fetch %s from localstore\n",TVSETTINGS_HLGMODE_RFC_PARAM);
            ret=-1;
        }
        return ret;
    }

    uint32_t TVMgr::resetHDR10Mode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_HDR10MODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_HDR10MODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HDR10MODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_HDR10MODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_HDR10MODE_RFC_PARAM);

                ret = SetTVHDR10Mode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("HDR10 Mode initialized successfully value %s\n",param.value);
                    //Save HDR10Mode to ssm_data
                    int params[3]={0};
                    params[0]=GetHDR10ModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","hdr10mode","HDR10Mode",PQ_PARAM_HDR10_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save HDR10Mode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE){
            returnResponse(false, getErrorString(ret));
        }else{
            LOGINFO("Exit : resetHDR10Mode() \n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::resetHLGMode(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        tvError_t ret = tvERROR_NONE;

        tr181ErrorCode_t err = clearLocalParam(rfc_caller_id,TVSETTINGS_HLGMODE_RFC_PARAM);
        if ( err != tr181Success ) {
            LOGWARN("clearLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
            ret  = tvERROR_GENERAL;
        }
        else {
            LOGINFO("clearLocalParam for %s Successful\n", TVSETTINGS_HLGMODE_RFC_PARAM);

            TR181_ParamData_t param;
            memset(&param, 0, sizeof(param));

            tr181ErrorCode_t err = getLocalParam(rfc_caller_id, TVSETTINGS_HLGMODE_RFC_PARAM,&param);
            if ( err != tr181Success ) {
                LOGWARN("getLocalParam for %s Failed : %s\n", TVSETTINGS_HLGMODE_RFC_PARAM, getTR181ErrorString(err));
                ret  = tvERROR_GENERAL;
            }
            else {
                LOGINFO("getLocalParam for %s Successful\n", TVSETTINGS_HLGMODE_RFC_PARAM);

                ret = SetTVHLGMode(param.value);
                if(ret != tvERROR_NONE) {
                    LOGWARN("DV Mode set failed: %s\n",getErrorString(ret).c_str());
                }
                else {

                    LOGINFO("HLG Mode initialized successfully value %s\n",param.value);
                    //Save HLGMode to ssm_data
                    int params[3]={0};
                    params[0]=GetHLGModeIndex(param.value);
                    int retval=UpdatePQParamsToCache("reset","hlgmode","HLGMode",PQ_PARAM_HLG_MODE,params);

                    if(retval != 0) {
                        LOGWARN("Failed to Save HLGMode to ssm_data\n");
                        ret=tvERROR_GENERAL;
                    }
                }
            }
        }
        if(ret != tvERROR_NONE)
        {
            returnResponse(false, getErrorString(ret));
        }
        else
        {
            LOGINFO("Exit : resetHLGMode() \n");
            returnResponse(true, "success");
        }
    }

    uint32_t TVMgr::getSupportedHLGModes(const JsonObject& parameters, JsonObject& response)
    {//sample response: {"success":true,”SupportedHLGModes": ["bright","dark"]}

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        pic_modes_t *hlgModes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedHLGModes(&hlgModes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedHLGModes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedHLGModes.Add(hlgModes[count].name);
            }

            response["SupportedHLGModes"] = SupportedHLGModes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::getSupportedHDR10Modes(const JsonObject& parameters, JsonObject& response)
    {//sample response: {"success":true,”SupportedHDR10Modes": ["bright","dark"]}

        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        pic_modes_t *hdr10Modes;
        unsigned short totalAvailable = 0;
        tvError_t ret = GetTVSupportedHDR10Modes(&hdr10Modes,&totalAvailable);
        if(ret != tvERROR_NONE) {
            returnResponse(false, getErrorString(ret).c_str());
        }
        else {
            JsonArray SupportedHDR10Modes;

            for(int count = 0;count <totalAvailable;count++ )
            {
                SupportedHDR10Modes.Add(hdr10Modes[count].name);
            }

            response["SupportedHDR10Modes"] = SupportedHDR10Modes;
            LOGINFO("Exit\n");
            returnResponse(true, "success");
        }

    }

    uint32_t TVMgr::setBacklightFade(const JsonObject& parameters, JsonObject& response)
    { 
        LOGINFO("Entry\n");
        PLUGIN_Lock(tvLock);
        std::string from,to,duration;
        int fromValue = 0,toValue = 0,durationValue = 0;

        if( !appUsesGlobalBackLightFactor )
        {
            LOGINFO("Exit : backlightFade Not supported \n");
            returnResponse(false, getErrorString(tvERROR_OPERATION_NOT_SUPPORTED));
        }
        from = parameters.HasLabel("from") ? parameters["from"].String() : "";
        if(from.empty())
            fromValue = 100;
        else
            fromValue = std::stoi(from);

        to = parameters.HasLabel("to") ? parameters["to"].String() : "";
        if(to.empty())
            toValue = 0;
        else
            toValue = std::stoi(to);

        duration = parameters.HasLabel("duration") ? parameters["duration"].String() : "";
        if(duration.empty())
            durationValue = 0;
        else
            durationValue = std::stoi(duration);

        LOGINFO("from = %d to = %d d = %d\n" ,fromValue,toValue,durationValue);
        tvError_t ret = SetBacklightFade(fromValue,toValue,durationValue);

        if(ret != tvERROR_NONE) {
           LOGWARN("Failed to set BacklightFade \n");
           returnResponse(false, getErrorString(ret).c_str());
        }
        else {
           LOGINFO("Exit : backlightFade Success \n");
           returnResponse(true, "success");
        }
    }


} //namespace WPEFramework

} //namespace Plugin
