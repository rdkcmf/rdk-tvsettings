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

#ifndef TVMGR_TVMGR_H
#define TVMGR_TVMGR_H

#include "string.h"
#include <pthread.h>
#include "Module.h"
#include "utils.h"
#include "tvTypes.h"
#include "tvError.h"
#include "AbstractPlugin.h"

#define PRODUCT_CONFIG "tvproduct_config.ini"
#define DECLARE_JSON_RPC_METHOD(method) \
uint32_t method(const JsonObject& parameters, JsonObject& response);

namespace WPEFramework {
namespace Plugin {

    class TVMgr : public AbstractPlugin {

    private:
        TVMgr(const TVMgr&) = delete;
        TVMgr& operator=(const TVMgr&) = delete;


        DECLARE_JSON_RPC_METHOD(getBacklight)
        DECLARE_JSON_RPC_METHOD(setBacklight)
        DECLARE_JSON_RPC_METHOD(getBrightness)
        DECLARE_JSON_RPC_METHOD(setBrightness)
        DECLARE_JSON_RPC_METHOD(getContrast)
        DECLARE_JSON_RPC_METHOD(setContrast)
        DECLARE_JSON_RPC_METHOD(getSharpness)
        DECLARE_JSON_RPC_METHOD(setSharpness)
        DECLARE_JSON_RPC_METHOD(getSaturation)
        DECLARE_JSON_RPC_METHOD(setSaturation)
        DECLARE_JSON_RPC_METHOD(setHue)
        DECLARE_JSON_RPC_METHOD(getHue)
        DECLARE_JSON_RPC_METHOD(getAspectRatio)
        DECLARE_JSON_RPC_METHOD(setAspectRatio)
        DECLARE_JSON_RPC_METHOD(getColorTemperature)
        DECLARE_JSON_RPC_METHOD(setColorTemperature)
        DECLARE_JSON_RPC_METHOD(setWakeupConfiguration)

	DECLARE_JSON_RPC_METHOD(getWBInfo)
        DECLARE_JSON_RPC_METHOD(setWBCtrl)
        DECLARE_JSON_RPC_METHOD(getWBCtrl)
	DECLARE_JSON_RPC_METHOD(enableWBMode)
        DECLARE_JSON_RPC_METHOD(getVideoResolution)
        DECLARE_JSON_RPC_METHOD(getVideoFrameRate)


//org.rdk.tv.ControlSettings.2
        DECLARE_JSON_RPC_METHOD(getPictureMode2)
        DECLARE_JSON_RPC_METHOD(getBacklight2)
        DECLARE_JSON_RPC_METHOD(getBrightness2)
        DECLARE_JSON_RPC_METHOD(getContrast2)
        DECLARE_JSON_RPC_METHOD(getSharpness2)
        DECLARE_JSON_RPC_METHOD(getSaturation2)
        DECLARE_JSON_RPC_METHOD(getHue2)
        DECLARE_JSON_RPC_METHOD(getAspectRatio2)
        DECLARE_JSON_RPC_METHOD(getColorTemperature2)
        DECLARE_JSON_RPC_METHOD(setAspectRatio2)
        DECLARE_JSON_RPC_METHOD(getAutoBacklightControl)
        DECLARE_JSON_RPC_METHOD(setAutoBacklightControl)
        DECLARE_JSON_RPC_METHOD(getVideoFormat)
        DECLARE_JSON_RPC_METHOD(getSupportedPictureModes)
        DECLARE_JSON_RPC_METHOD(getPictureMode)
        DECLARE_JSON_RPC_METHOD(setPictureMode)
        DECLARE_JSON_RPC_METHOD(getSupportedDolbyVisionModes)
        DECLARE_JSON_RPC_METHOD(getDolbyVisionMode)
        DECLARE_JSON_RPC_METHOD(setDolbyVisionMode)
        DECLARE_JSON_RPC_METHOD(setDynamicContrast)
        DECLARE_JSON_RPC_METHOD(getDynamicContrast)
        DECLARE_JSON_RPC_METHOD(getComponentColorInfo)
        DECLARE_JSON_RPC_METHOD(setComponentSaturation)
        DECLARE_JSON_RPC_METHOD(getComponentSaturation)

        DECLARE_JSON_RPC_METHOD(getAllBacklightDimmingModes)
        DECLARE_JSON_RPC_METHOD(getBacklightDimmingMode)
        DECLARE_JSON_RPC_METHOD(setBacklightDimmingMode)
        DECLARE_JSON_RPC_METHOD(resetBrightness)
        DECLARE_JSON_RPC_METHOD(resetSharpness)
        DECLARE_JSON_RPC_METHOD(resetSaturation)
        DECLARE_JSON_RPC_METHOD(resetHue)
        DECLARE_JSON_RPC_METHOD(resetBacklight)
        DECLARE_JSON_RPC_METHOD(resetColorTemperature)
        DECLARE_JSON_RPC_METHOD(resetDolbyVisionMode)
        DECLARE_JSON_RPC_METHOD(resetAutoBacklight)
        DECLARE_JSON_RPC_METHOD(resetContrast)
        DECLARE_JSON_RPC_METHOD(resetAspectRatio)
        DECLARE_JSON_RPC_METHOD(setComponentHue)
        DECLARE_JSON_RPC_METHOD(getComponentHue)
        DECLARE_JSON_RPC_METHOD(setComponentLuma)
        DECLARE_JSON_RPC_METHOD(getComponentLuma)
        DECLARE_JSON_RPC_METHOD(resetComponentSaturation)
        DECLARE_JSON_RPC_METHOD(resetComponentLuma)
        DECLARE_JSON_RPC_METHOD(resetComponentHue)
        DECLARE_JSON_RPC_METHOD(resetBacklightDimmingMode)
        DECLARE_JSON_RPC_METHOD(resetWBCtrl)
        DECLARE_JSON_RPC_METHOD(resetPictureMode)
        DECLARE_JSON_RPC_METHOD(getHLGMode)
        DECLARE_JSON_RPC_METHOD(setHLGMode)
        DECLARE_JSON_RPC_METHOD(getHDR10Mode)
        DECLARE_JSON_RPC_METHOD(setHDR10Mode)
        DECLARE_JSON_RPC_METHOD(resetHDR10Mode)
        DECLARE_JSON_RPC_METHOD(resetHLGMode)
        DECLARE_JSON_RPC_METHOD(getSupportedHLGModes)
        DECLARE_JSON_RPC_METHOD(getSupportedHDR10Modes)
	DECLARE_JSON_RPC_METHOD(setBacklightFade)
    public:
        TVMgr();
        virtual ~TVMgr();
        static TVMgr* _instance;

        BEGIN_INTERFACE_MAP(TVMgr)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP

    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const std::string Initialize(PluginHost::IShell* service);
        virtual void Deinitialize(PluginHost::IShell* service);
        virtual std::string Information() const;
        void NotifyVideoFormatChange(tvVideoHDRFormat_t format);
        void NotifyVideoResolutionChange(tvResolutionParam_t resolution);
        void NotifyVideoFrameRateChange(tvVideoFrameRate_t frameRate);
    	int getCurrentPictureMode(char *picMode);


    private:
        uint8_t _skipURL;
        int m_currentHdmiInResoluton;
        int m_videoZoomMode;
        bool m_isDisabledHdmiIn4KZoom;

	std::string getErrorString (tvError_t eReturn);
        std::string numberToString (int number);
        int stringToNumber (std::string text);
        uint32_t generateStorageIdentifier(std::string &key,const char * forParam,uint32_t contentFormat);
        uint32_t generateStorageIdentifierDirty(std::string &key,const char * forParam,uint32_t contentFormat, int pqmode);
        uint32_t getContentFormatIndex(tvVideoHDRFormat_t formatToConvert);
        bool isSaveforAllContentFormats(void);
        tvError_t setDefaultAspectRatio(void);
        bool isBacklightUsingGlobalBacklightFactor(void);
        tvError_t FillGlobalBacklightStructure(int backlight);
        tvError_t SetBacklightAtInitAndSaveForBLUsingGBF(void);
	tvError_t saveBacklightToLocalStoreForGBF(const char* key, const char* value);
	int ReadBacklightFromTable(char* panelId);
	int UpdatePQParamsToCache(const char *action,const char *pqParamName ,const char *tr181ParamName,tvPQParameterIndex_t pqParamIndex,int params[]);
	int GetTheSaveConfigMap(tvSaveFormatsConfig_t formatConfig, tvSavePicModesConfig_t picmodeConfig, tvSourceSaveConfig_t sourceConfig, std::vector<int> &sources,std::vector<int> &picturemodes, std::vector<int> &formats);
        int ReadConifgEntryValues(const char* action, const char *param,unsigned int &forSource,unsigned int  &forFormat,unsigned int &forPicMode );
	int GetDimmingModeIndex(const char* mode);
        int GetPQParamsToSync (const char *forParam,int pqmode,int format, int& value,bool cms=false,int tunnel_type=0);
	int GetDolbyParamToSync(int& value);
	int SyncCMSParams(const char *configParam, const char *pqParam,tvCMS_tunel_t tunnel_type);
	int GetLDIMParamsToSync(int &value,int mode);
	int InitializeAndSyncLDIM(void);
	int InitializeSDRHDRBacklight(void);
	void LocatePQSettingsFile(void);
        tvError_t UpdatePQParamToLocalCache(const char* forParam, int pqmode, int format, int value,bool setNotDelete);
        tvDataComponentColor_t GetComponentColorEnum(std::string colorName);
        tvError_t SyncPQParamsToDriverCache();
        tvError_t SyncCMSParamsToDriverCache();
    void GetColorTempStringFromEnum(int value, std::string &toStore);
    void SyncWBparams(void);
    tvError_t SyncCustomWBValuesToDriverCache(tvDataColor_t WBValues,bool setDuringSync);
    int GetGainOffsetValue(const char* color,const char*ctrl,tvDataColor_t WBValues,int &rgbType);
    tvError_t GetCustomWBValuesFromLocalCache(tvDataColor_t &WBValues);
    tvError_t GetLastSetBacklightForGBF(int &backlight);
    tvError_t SaveSDRHDRBacklightAtInitToDrv(int backlightSDR,int backlightHDR);
    int GetHDR10ParamToSync(int& value);
    int GetHLGParamToSync(int& value);
    bool isIARMConnected();
    bool IARMinit();
    static void dsHdmiVideoModeEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
    static void dsHdmiStatusEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
    void InitializeIARM();
    void DeinitializeIARM();
    tvError_t setAspectRatioZoomSettings(tvDisplayMode_t mode);
    tvError_t getUserSelectedAspectRatio(tvDisplayMode_t* mode);

    };

} // Namespace Plugin.
}
#endif //  TVMGR_TVMGR_H
