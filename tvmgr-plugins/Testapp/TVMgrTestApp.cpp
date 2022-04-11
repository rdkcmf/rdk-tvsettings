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
#include "TVMgrTestApp.h"

using namespace WPEFramework;

#define SYSSRV_CALLSIGN "org.rdk.tv.ControlSettings"
#define SYSSRV_CALLSIGN_V1 "org.rdk.tv.ControlSettings.1"
#define SYSSRV_CALLSIGN_V2 "org.rdk.tv.ControlSettings.2"
#define SERVER_DETAILS  "127.0.0.1:9998"

/* For version 1 */
JSONRPC::LinkType<Core::JSON::IElement> * remoteObject = NULL;
/* For version 2 */
JSONRPC::LinkType<Core::JSON::IElement> * secondaryObject = NULL;

/* Declare module name */
MODULE_NAME_DECLARATION(BUILD_REFERENCE)

	bool bNeedExtraLineRead = false;

	/* This section can be used for API validation logic. */
void showMenu()
{
	std::cout<<"\n\nTVMgr Methods: \n";
	std::cout<<"1.getBrightness\n";
	std::cout<<"2.setBrightness\n";
	std::cout<<"3.resetBrightness\n";
	std::cout<<"4.getBacklight\n";
	std::cout<<"5.setBacklight\n";
	std::cout<<"6.resetBacklight\n";
	std::cout<<"7.getContrast\n";
	std::cout<<"8.setContrast\n";
	std::cout<<"9.resetContrast\n";
	std::cout<<"10.getSaturation\n";
	std::cout<<"11.setSaturation\n";
	std::cout<<"12.resetSaturation\n";
	std::cout<<"13.getSharpness\n";
	std::cout<<"14.setSharpness\n";
	std::cout<<"15.resetSharpness\n";
	std::cout<<"16.getHue\n";
	std::cout<<"17.setHue\n";
	std::cout<<"18.resetHue\n";
	std::cout<<"19.getColorTemperature\n";
	std::cout<<"20.setColorTemperature\n";
	std::cout<<"21.resetColorTemperature\n";
	std::cout<<"22.getAspectRatio\n";
	std::cout<<"23.setAspectRatio\n";
	std::cout<<"24.resetAspectRatio\n";
	std::cout<<"25.getBacklightDimmingMode\n";
	std::cout<<"26.setBacklightDimmingMode\n";
	std::cout<<"27.resetBacklightDimmingMode\n";
	std::cout<<"28.getComponentSaturation\n";
	std::cout<<"29.setComponentSaturation\n";
	std::cout<<"30.resetComponentSaturation\n";
	std::cout<<"31.getComponentHue\n";
	std::cout<<"32.setComponentHue\n";
	std::cout<<"33.resetComponentHue\n";
	std::cout<<"34.getComponentLuma\n";
	std::cout<<"35.setComponentLuma\n";
	std::cout<<"36.resetComponentLuma\n";
	std::cout<<"37.getDolbyVisionMode\n";
	std::cout<<"38.setDolbyVisionMode\n";
	std::cout<<"39.resetDolbyVisionMode\n";
	std::cout<<"40.getAutoBacklightControl\n";
	std::cout<<"41.setAutoBacklightControl\n";
	std::cout<<"42.resetAutoBacklight\n";
	std::cout<<"43.getDynamicContrast\n";
	std::cout<<"44.setDynamicContrast\n";
	std::cout<<"47.getPictureMode\n";
	std::cout<<"48.setPictureMode\n";
	std::cout<<"49.getVideoResolution\n";
	std::cout<<"50.getVideoFrameRate\n";
	std::cout<<"51.getVideoFormat\n";
	std::cout<<"52.getSupportedPictureModes\n";
	std::cout<<"53.getSupportedDolbyVisionModes\n";
	std::cout<<"54.getComponentColorInfo\n";
	std::cout<<"55.getAllBacklightDimmingModes\n";
	std::cout<<"56.enableWBMode\n";
	std::cout<<"57.getWBInfo\n";
	std::cout<<"58.getWBCtrl\n";
	std::cout<<"59.setWBCtrl\n";
	std::cout<<"60.commitWB\n";
	std::cout<<"61.setWakeupConfiguration\n";  
	std::cout<<"\nEnter your choice: ";
}

/* For displaying the result in Json format */
void formatForDisplay(std::string& str)
{
	const int indent = 4;
	int level = 0;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == ',')
		{
			str.insert((i+1), 1, '\n'); // insert after
			if (level > 0)
			{
				str.insert((i+2), (level * indent), ' ');
			}
		}
		else if (str[i] == '}')
		{
			level--;
			if (level < 0) level = 0;
			str.insert((i), 1, '\n');   // insert before
			if (level > 0)
			{
				// after the newline, but before the curly brace
				str.insert((i+1), (level * indent), ' ');
			}
			i += (level * indent) + 1;    // put i back on the curly brace
		}
		else if (str[i] == '{')
		{
			level++;
			str.insert((i+1), 1, '\n'); // insert after
			if (level > 0)
			{
				str.insert((i+2), (level * indent), ' ');
			}
		}
	}
}

/* This section is related to the event handler implementation for ControlService Plugin Events. */
namespace Handlers {
	/* Event Handlers */
	static void tvVideoFormatChangeHandler(const Core::JSON::String& parameters) {
		std::string message;
		parameters.ToString(message);
		std::cout << "\n[TVMgrEvt] " << __FUNCTION__ << ": " << message << std::endl;
	}
	static void tvVideoResolutionChangeHandler(const Core::JSON::String& parameters) {
		std::string message;
		parameters.ToString(message);
		std::cout << "\n[TVMgrEvt] " << __FUNCTION__ << ": " << message << std::endl;
	}
	static void tvVideoFrameRateChangeHandler(const Core::JSON::String& parameters) {
		std::string message;
		parameters.ToString(message);
		std::cout << "\n[TVMgrEvt] " << __FUNCTION__ << ": " << message << std::endl;
	}
}

int getBrightness()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getBrightness"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TMVgr getBrightness call - Success!\n";
	} else {
		std::cout<<"TMVgr getBrightness call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setBrightness()
{
	JsonObject params;
	JsonObject result;
	string res;
	bNeedExtraLineRead = true;

	std::cout<<"Enter value ";
	int value;
	std::cout<<"(e.g., 0 thru 100 ) --> ";
	std::cin>> value;
	params["brightness"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setBrightness"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setBrightness: success\n";
	} else {
		std::cout<<"TVMgr setBrightness: failure\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetBrightness()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetBrightness"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetBrightness call - Success!\n";
	} else {
		std::cout<<"TVMgr resetBrightness call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getBacklight()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getBacklight"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getBacklight call - Success!\n";
	} else {
		std::cout<<"TVMgr getBacklight call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setBacklight()
{
	JsonObject params;
	JsonObject result;
	string res;
	bNeedExtraLineRead = true;

	std::cout<<"Enter value ";
	int value;
	std::cout<<"(e.g., 0 thru 100 ) --> ";
	std::cin>> value;
	params["backlight"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setBacklight"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setBacklight: success\n";
	} else {
		std::cout<<"TVMgr setBacklight: failure\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetBacklight()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetBacklight"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetBacklight call - Success!\n";
	} else {
		std::cout<<"TVMgr resetBacklight call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n"; 
	return 0;
}

int getContrast()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getContrast"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getContrast call - Success!\n";
	} else {
		std::cout<<"TVMgr getContrast call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setContrast()
{
	JsonObject params;
	JsonObject result;
	string res;
	bNeedExtraLineRead = true;

	std::cout<<"Enter value ";
	int value;
	std::cout<<"(e.g., 0 thru 100 ) --> ";
	std::cin>> value;
	params["contrast"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setContrast"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setContrast: success\n";
	} else {
		std::cout<<"TVMgr setContrast: failure\n";
	}
	std::cout<<"result : "<<res<<"\n"; 
	return 0;
}

int resetContrast()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetContrast"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetContrast call - Success!\n";
	} else {
		std::cout<<"TVMgr resetContrast call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";  
	return 0;
}

int getSaturation()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getSaturation"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getSaturation call - Success!\n";
	} else {
		std::cout<<"TVMgr getSaturation call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";   
	return 0;
}

int setSaturation()
{
	JsonObject params;
	JsonObject result;
	string res;
	bNeedExtraLineRead = true;

	std::cout<<"Enter value ";
	int value;
	std::cout<<"(e.g., 0 thru 100 ) --> ";
	std::cin>> value;
	params["saturation"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setSaturation"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setSaturation: success\n";
	} else {
		std::cout<<"TVMgr setSaturation: failure\n";
	}
	std::cout<<"result : "<<res<<"\n"; 
	return 0;
}

int resetSaturation()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetSaturation"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetSaturation call - Success!\n";
	} else {
		std::cout<<"TVMgr resetSaturation call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";   
	return 0;
}

int getSharpness()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getSharpness"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getSharpness call - Success!\n";
	} else {
		std::cout<<"TVMgr getSharpness call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";   
	return 0;
}

int setSharpness()
{
	JsonObject params;
	JsonObject result;
	string res;
	bNeedExtraLineRead = true;

	std::cout<<"Enter value ";
	int value;
	std::cout<<"(e.g., 0 thru 100 ) --> ";
	std::cin>> value;
	params["sharpness"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setSharpness"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setSharpness: success\n";
	} else {
		std::cout<<"TVMgr setSharpness: failure\n";
	}
	std::cout<<"result : "<<res<<"\n";    
	return 0;
}

int resetSharpness()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetSharpness"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetSharpness call - Success!\n";
	} else {
		std::cout<<"TVMgr resetSharpness call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";   
	return 0;
}

int getHue()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getHue"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getHue call - Success!\n";
	} else {
		std::cout<<"TVMgr getHue call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";   
	return 0;
}

int setHue()
{
	JsonObject params;
	JsonObject result;
	string res;
	bNeedExtraLineRead = true;

	std::cout<<"Enter value ";
	int value;
	std::cout<<"(e.g., 0 thru 100 ) --> ";
	std::cin>> value;
	params["hue"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setHue"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setHue: success\n";
	} else {
		std::cout<<"TVMgr setHue: failure\n";
	}
	std::cout<<"result : "<<res<<"\n"; 
	return 0;
}

int resetHue()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetHue"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetHue call - Success!\n";
	} else {
		std::cout<<"TVMgr resetHue call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getColorTemperature()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getColorTemperature"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getColorTemperature call - Success!\n";
	} else {
		std::cout<<"TVMgr getColorTemperature call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setColorTemperature()
{
	JsonObject params;
	JsonObject result;
	string res;
	string colorTemp;
	bNeedExtraLineRead = true;

	std::cout<<"\nEnter colorTemp string(Standard, Cold, Warm, User Defined) :";  

	std::cin>> colorTemp;
	params["colorTemp"] = colorTemp;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setColorTemperature"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setColorTemperature: success\n";
	} else {
		std::cout<<"TVMgr setColorTemperature: failure\n";
	}
	std::cout<<"result : "<<res<<"\n";   
	return 0;
}

int resetColorTemperature()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetColorTemperature"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetColorTemperature call - Success!\n";
	} else {
		std::cout<<"TVMgr resetColorTemperature call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getAspectRatio()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getAspectRatio"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getAspectRatio call - Success!\n";
	} else {
		std::cout<<"TVMgr getAspectRatio call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setAspectRatio()
{
	JsonObject params;
	JsonObject result;
	string res;

	string value;
	std::cout<<"Enter string(TV NORMAL, TV DIRECT, TV AUTO) --> ";
	std::cin>> value;
	params["aspectRatio"] = value;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setAspectRatio"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setAspectRatio: success\n";
	} else {
		std::cout<<"TVMgr setAspectRatio: failure\n";
	}
	std::cout<<"result : "<<res<<"\n"; 
	return 0;
}

int resetAspectRatio()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetAspectRatio"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetAspectRatio call - Success!\n";
	} else {
		std::cout<<"TVMgr resetAspectRatio call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getBacklightDimmingMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getBacklightDimmingMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getBacklightDimmingMode call - Success!\n";
	} else {
		std::cout<<"TVMgr getBacklightDimmingMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setBacklightDimmingMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	string DimmingMode;

	std::cout<<"\nEnter DimmingMode (fixed) :";    
	std::cin>> DimmingMode;
	bNeedExtraLineRead = true;
	params["DimmingMode"] = DimmingMode;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setBacklightDimmingMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setBacklightDimmingMode call - Success!\n";
	} else {
		std::cout<<"TVMgr setBacklightDimmingMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetBacklightDimmingMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetBacklightDimmingMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetBacklightDimmingMode call - Success!\n";
	} else {
		std::cout<<"TVMgr resetBacklightDimmingMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getComponentSaturation()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;

	std::cout<<"\nEnter color string(none, red, green, blue, yellow, cyan, magenta) :";  
	std::cin>> color;
	bNeedExtraLineRead = true;
	params["color"] = color;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getComponentSaturation"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getComponentSaturation call - Success!\n";
	} else {
		std::cout<<"TVMgr getComponentSaturation call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setComponentSaturation()
{
	JsonObject params;
	JsonObject result;
	string res;
	string value;

	std::cout<<"\nEnter color string(none, red, green, blue, yellow, cyan, magenta) :"; 
	std::cin>> value;
	params["color"] = value;
	bNeedExtraLineRead = true;

	std::cout<<"\nEnter saturation value:";   
	std::cin>> value;									
	params["saturation"] = value;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setComponentSaturation"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setComponentSaturation call - Success!\n";
	} else {
		std::cout<<"TVMgr setComponentSaturation call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetComponentSaturation()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetComponentSaturation"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetComponentSaturation call - Success!\n";
	} else {
		std::cout<<"TVMgr resetComponentSaturation call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getComponentHue()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;

	std::cout<<"\nEnter color string(none, red, green, blue, yellow, cyan, magenta) :";   
	std::cin>> color;
	bNeedExtraLineRead = true;
	params["color"] = color;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getComponentHue"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getComponentHue call - Success!\n";
	} else {
		std::cout<<"TVMgr getComponentHue call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setComponentHue()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;
	int hue;

	std::cout<<"\nEnter color string(none, red, green, blue, yellow, cyan, magenta) :";  
	std::cin>> color;
	bNeedExtraLineRead = true;
	std::cout<<"\nEnter component hue value:";     
	std::cin>> hue;									

	params["color"] = color;
	params["hue"] = hue;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setComponentHue"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setComponentHue call - Success!\n";
	} else {
		std::cout<<"TVMgr setComponentHue call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetComponentHue()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetComponentHue"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetComponentHue call - Success!\n";
	} else {
		std::cout<<"TVMgr resetComponentHue call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getComponentLuma()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;

	std::cout<<"\nEnter color string(none, red, green, blue, yellow, cyan, magenta) :"; 
	std::cin>> color;
	bNeedExtraLineRead = true;
	params["color"] = color;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getComponentLuma"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getComponentLuma call - Success!\n";
	} else {
		std::cout<<"TVMgr getComponentLuma call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setComponentLuma()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;
	int luma;

	std::cout<<"\nEnter color string(none, red, green, blue, yellow, cyan, magenta) :"; 
	std::cin>> color;
	bNeedExtraLineRead = true;
	std::cout<<"\nEnter component luma value:";     
	std::cin>> luma;									
	params["color"] = color;
	params["luma"] = luma;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setComponentLuma"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setComponentLuma call - Success!\n";
	} else {
		std::cout<<"TVMgr setComponentLuma call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetComponentLuma()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetComponentLuma"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetComponentLuma call - Success!\n";
	} else {
		std::cout<<"TVMgr resetComponentLuma call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getDolbyVisionMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getDolbyVisionMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getDolbyVisionMode call - Success!\n";
	} else {
		std::cout<<"TVMgr getDolbyVisionMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setDolbyVisionMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	string DolbyVisionMode;

	std::cout<<"\nEnter DolbyVisionMode (bright)) :";      
	std::cin>> DolbyVisionMode;
	bNeedExtraLineRead = true;
	params["DolbyVisionMode"] = DolbyVisionMode;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setDolbyVisionMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setDolbyVisionMode call - Success!\n";
	} else {
		std::cout<<"TVMgr setDolbyVisionMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetDolbyVisionMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetDolbyVisionMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetDolbyVisionMode call - Success!\n";
	} else {
		std::cout<<"TVMgr resetDolbyVisionMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getAutoBacklightControl()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getAutoBacklightControl"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getAutoBacklightControl call - Success!\n";
	} else {
		std::cout<<"TVMgr getAutoBacklightControl call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setAutoBacklightControl()
{
	JsonObject params;
	JsonObject result;
	string res;
	string mode;

	std::cout<<"\nEnter mode (none, manual, ambient, eco) :"; 
	std::cin>> mode;
	bNeedExtraLineRead = true;
	params["mode"] = mode;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setAutoBacklightControl"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setAutoBacklightControl call - Success!\n";
	} else {
		std::cout<<"TVMgr setAutoBacklightControl call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int resetAutoBacklight()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("resetAutoBacklight"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr resetAutoBacklight call - Success!\n";
	} else {
		std::cout<<"TVMgr resetAutoBacklight call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getDynamicContrast()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getDynamicContrast"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getDynamicContrast call - Success!\n";
	} else {
		std::cout<<"TVMgr getDynamicContrast call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setDynamicContrast()
{
	JsonObject params;
	JsonObject result;
	string res;
	string DynamicContrast;

	std::cout<<"\nEnter DynamicContrast (enabled) :";   
	std::cin>> DynamicContrast;
	bNeedExtraLineRead = true;
	params["DynamicContrast"] = DynamicContrast;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setDynamicContrast"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setDynamicContrast call - Success!\n";
	} else {
		std::cout<<"TVMgr setDynamicContrast call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getPictureMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getPictureMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getPictureMode call - Success!\n";
	} else {
		std::cout<<"TVMgr getPictureMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setPictureMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	string pictureMode;

	std::cout<<"\nEnter pictureMode (standard, vivid, colorful, game) :";   
	std::cin>> pictureMode;
	bNeedExtraLineRead = true;
	params["pictureMode"] = pictureMode;

	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setPictureMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setPictureMode call - Success!\n";
	} else {
		std::cout<<"TVMgr setPictureMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}


int getVideoResolution()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getVideoResolution"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getVideoResolution call - Success!\n";
	} else {
		std::cout<<"TVMgr getVideoResolution call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getVideoFrameRate()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getVideoFrameRate"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getVideoFrameRate call - Success!\n";
	} else {
		std::cout<<"TVMgr getVideoFrameRate call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getVideoFormat()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getVideoFormat"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getVideoFormat call - Success!\n";
	} else {
		std::cout<<"TVMgr getVideoFormat call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getSupportedPictureModes()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getSupportedPictureModes"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getSupportedPictureModes call - Success!\n";
	} else {
		std::cout<<"TVMgr getSupportedPictureModes call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getSupportedDolbyVisionModes()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getSupportedDolbyVisionModes"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getSupportedDolbyVisionModes call - Success!\n";
	} else {
		std::cout<<"TVMgr getSupportedDolbyVisionModes call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getComponentColorInfo()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getComponentColorInfo"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getComponentColorInfo call - Success!\n";
	} else {
		std::cout<<"TVMgr getComponentColorInfo call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getAllBacklightDimmingModes()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = secondaryObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getAllBacklightDimmingModes"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getAllBacklightDimmingModes call - Success!\n";
	} else {
		std::cout<<"TVMgr getAllBacklightDimmingModes call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int enableWBMode()
{
	JsonObject params;
	JsonObject result;
	string res;
	string mode;

	std::cout<<"\nEnter mode (true, false) :";      
	std::cin>> mode;
	bNeedExtraLineRead = true;
	params["mode"] = mode;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("enableWBMode"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr enableWBMode call - Success!\n";
	} else {
		std::cout<<"TVMgr enableWBMode call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getWBInfo()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getWBInfo"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getWBInfo call - Success!\n";
	} else {
		std::cout<<"TVMgr getWBInfo call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int getWBCtrl()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;
	string ctrl;
	string applies;

	std::cout<<"\nEnter color string(black, red, green) :";
	std::cin>> color;
	bNeedExtraLineRead = true;
	std::cout<<"\nEnter ctrl string(gain) :";     
	std::cin>> ctrl;
	params["applies"] = "[{\"selector\" : \"color temp\", \"index\" : \"normal\"},{\"selector\":\"input\",\"index\" : \"HDMI1\"}]";
	params["color"] = color;
	params["ctrl"] = ctrl;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("getWBCtrl"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr getWBCtrl call - Success!\n";
	} else {
		std::cout<<"TVMgr getWBCtrl call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setWBCtrl()
{
	JsonObject params;
	JsonObject result;
	string res;
	string color;
	string ctrl;
	string value;
	string applies;

	std::cout<<"\nEnter color string(black, red) :";   
	std::cin>> color;
	bNeedExtraLineRead = true;
	std::cout<<"\nEnter ctrl string(gain) :";      
	std::cin>> ctrl;
	std::cout<<"\nEnter value string :";      
	std::cin>> value;
	params["applies"] = "[{\"selector\" : \"color temp\", \"index\" : \"normal\"},{\"selector\":\"input\",\"index\" : \"HDMI1\"}]";
	params["color"] = color;
	params["ctrl"] = ctrl;
	params["value"] = value;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setWBCtrl"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setWBCtrl call - Success!\n";
	} else {
		std::cout<<"TVMgr setWBCtrl call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int commitWB()
{
	JsonObject params;
	JsonObject result;
	string res;
	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("commitWB"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr commitWB call - Success!\n";
	} else {
		std::cout<<"TVMgr commitWB call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

int setWakeupConfiguration()
{
	JsonObject params;
	JsonObject result;
	string res;
	string wakeupSrc;
	string config;

	std::cout<<"\nEnter wakeupSrc :";      
	std::cin>> wakeupSrc;
	bNeedExtraLineRead = true;
	std::cout<<"\nEnter config value :";      
	std::cin>> config;
	params["wakeupSrc"] = wakeupSrc;	
	params["config"] = config;

	uint32_t ret = remoteObject->Invoke<JsonObject, JsonObject>(1000,
			_T("setWakeupConfiguration"), params, result);
	std::cout<<"TVMgr Invoke ret : "<< ret <<"\n";
	result.ToString(res);
	formatForDisplay(res);
	if (result["success"].Boolean()) {
		std::cout<<"TVMgr setWakeupConfiguration call - Success!\n";
	} else {
		std::cout<<"TVMgr setWakeupConfiguration call - failed!\n";
	}
	std::cout<<"result : "<<res<<"\n";
	return 0;
}

/* Main Function */
int main(int argc, char** argv)
{
	int choice;
	string cmd;
	string lastCmd;

	Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));

	// Security Token
	std::cout << "Retrieving security token" << std::endl;
	std::string sToken;

	FILE *pSecurity = popen("/usr/bin/WPEFrameworkSecurityUtility", "r");
	if(pSecurity) {
		JsonObject pSecurityJson;
		std::string pSecurityOutput;
		int         pSecurityOutputTrimIndex;
		std::array<char, 256> pSecurityBuffer;

		while(fgets(pSecurityBuffer.data(), 256, pSecurity) != NULL) {
			pSecurityOutput += pSecurityBuffer.data();
		}
		pclose(pSecurity);

		pSecurityOutputTrimIndex = pSecurityOutput.find('{');
		if((size_t)pSecurityOutputTrimIndex == std::string::npos) {
			std::cout << "Security Utility returned unexpected output" << std::endl;
		} else {
			if(pSecurityOutputTrimIndex > 0) {
				std::cout << "Trimming output from Security Utility" << std::endl;
				pSecurityOutput = pSecurityOutput.substr(pSecurityOutputTrimIndex);
			}
			pSecurityJson.FromString(pSecurityOutput);
			if(pSecurityJson["success"].Boolean() == true) {
				std::cout << "Security Token retrieved successfully!" << std::endl;
				sToken = "token=" + pSecurityJson["token"].String();
			} else {
				std::cout << "Security Token retrieval failed!" << std::endl;
			}
		}
	} else {
		std::cout << "Failed to open security utility" << std::endl;
	}
	// End Security Token

	std::cout << "Using callsign: " << SYSSRV_CALLSIGN << std::endl;

	if ((NULL == remoteObject) && (NULL == secondaryObject)){
		remoteObject = new WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>(_T(SYSSRV_CALLSIGN_V1), _T(""), false, sToken); 
		secondaryObject = new WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>(_T(SYSSRV_CALLSIGN_V2), _T(""), false, sToken);

		if ((NULL == remoteObject) && (NULL == secondaryObject)) {
			std::cout << "JSONRPC::Client initialization failed" << std::endl;
		} else {
			{  
				// Create a controller client
				static auto& controllerClient = *new WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>("", "", false, sToken);
				// In case the plugin isn't activated already, try to start it, BEFORE registering for the events!
				string strres;
				JsonObject params;
				params["callsign"] = SYSSRV_CALLSIGN_V1;
				JsonObject result;
				uint32_t ret = controllerClient.Invoke<JsonObject, Core::JSON::VariantContainer>(2000, "activate", params, result);
				std::cout<<"\nTVMgr activate Invoke ret : "<< ret <<"\n";
				result.ToString(strres);
				std::cout<<"startup result : "<< strres <<"\n";
			}

			/* Register handlers for Event reception. */
			std::cout << "\nSubscribing to event handlers\n" << std::endl;
			if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("tvVideoFormatChangeHandler"),
						&Handlers::tvVideoFormatChangeHandler) == Core::ERROR_NONE) {
				std::cout << "Subscribed to : tvVideoFormatChangeHandler" << std::endl;
			} else {
				std::cout << "Failed to Subscribe notification handler : tvVideoFormatChangeHandler" << std::endl;
			}
			if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("tvVideoResolutionChangeHandler"),
						&Handlers::tvVideoResolutionChangeHandler) == Core::ERROR_NONE) {
				std::cout << "Subscribed to : tvVideoResolutionChangeHandler" << std::endl;
			} else {
				std::cout << "Failed to Subscribe notification handler : tvVideoResolutionChangeHandler" << std::endl;
			}
			if (remoteObject->Subscribe<Core::JSON::String>(1000, _T("tvVideoFrameRateChangeHandler"),
						&Handlers::tvVideoFrameRateChangeHandler) == Core::ERROR_NONE) {
				std::cout << "Subscribed to : tvVideoFrameRateChangeHandler" << std::endl;
			} else {
				std::cout << "Failed to Subscribe notification handler : tvVideoFrameRateChangeHandler" << std::endl;
			}

			/* API Validation Logic. */
			while (true) {
				while (cmd.empty())
				{
					showMenu();
					std::getline(std::cin, cmd);
					lastCmd = cmd;
				}
				if ((cmd[0] >= '0') && (cmd <= "61"))
				{
					choice = stoi(cmd);
				}
				else{
					return 0;
				} 

				{
					switch (choice) {
						case 1:
							getBrightness();
							break;

						case 2:
							setBrightness();
							break;

						case 3:
							resetBrightness();
							break;

						case 4:
							getBacklight();
							break;

						case 5:
							setBacklight();
							break;

						case 6:
							resetBacklight();
							break;

						case 7:
							getContrast();
							break;

						case 8:
							setContrast();
							break;

						case 9:
							resetContrast();
							break;					

						case 10:
							getSaturation();
							break;

						case 11:
							setSaturation();
							break;

						case 12:
							resetSaturation();
							break;

						case 13:
							getSharpness();
							break;

						case 14:
							setSharpness();
							break;

						case 15:
							resetSharpness();
							break;

						case 16:
							getHue();
							break;

						case 17:
							setHue();
							break;

						case 18:
							resetHue();
							break;

						case 19:
							getColorTemperature();
							break;

						case 20:
							setColorTemperature();
							break;

						case 21:
							resetColorTemperature();
							break;

						case 22:
							getAspectRatio();
							break;

						case 23:
							setAspectRatio();
							break;

						case 24:
							resetAspectRatio();
							break;

						case 25:
							getBacklightDimmingMode();
							break;

						case 26:
							setBacklightDimmingMode();
							break;

						case 27:
							resetBacklightDimmingMode();
							break;	

						case 28:
							getComponentSaturation();
							break;

						case 29:
							setComponentSaturation();
							break;

						case 30:
							resetComponentSaturation();
							break;    

						case 31:
							getComponentHue();
							break;

						case 32:
							setComponentHue();
							break;

						case 33:
							resetComponentHue();
							break;       

						case 34:
							getComponentLuma();
							break;

						case 35:
							setComponentLuma();
							break;

						case 36:
							resetComponentLuma();
							break;                                                                               

						case 37:
							getDolbyVisionMode();
							break;

						case 38:
							setDolbyVisionMode();
							break;

						case 39:
							resetDolbyVisionMode();
							break;

						case 40:
							getAutoBacklightControl();
							break;

						case 41:
							setAutoBacklightControl();
							break;

						case 42:
							resetAutoBacklight();
							break;                                                                                  

						case 43:
							getDynamicContrast();
							break;		

						case 44:
							setDynamicContrast();
							break;

						case 47:
							getPictureMode();
							break;		

						case 48:
							setPictureMode();
							break; 

						case 49:
							getVideoResolution();
							break;		

						case 50:
							getVideoFrameRate();
							break; 

						case 51:
							getVideoFormat();
							break;		

						case 52:
							getSupportedPictureModes();
							break; 

						case 53:
							getSupportedDolbyVisionModes();
							break;		

						case 54:
							getComponentColorInfo();
							break;

						case 55:
							getAllBacklightDimmingModes();
							break; 

						case 56:
							enableWBMode();
							break;		

						case 57:
							getWBInfo();
							break; 

						case 58:
							getWBCtrl();
							break;		

						case 59:
							setWBCtrl();
							break;    

						case 60:
							commitWB();
							break;		

						case 61:
							setWakeupConfiguration();
							break;                                                      

						default:
							std::cout<<"Entry not recognized!\n";
							break;

					}
				}

				std::cout<<"\n\nTo continue press ENTER; To quit press any other key --> ";

				if(bNeedExtraLineRead)
				{
					if (std::cin.peek() == '\n')
					{
						std::getline(std::cin, cmd);
					}
					bNeedExtraLineRead = false;
				}
				else
					cmd.clear();

				std::getline(std::cin, cmd);
				if (cmd.empty())
				{
					cmd.clear();
					lastCmd.clear();
				}
				else if ((cmd[0] == 'r') || (cmd[0] == 'R'))
					cmd = lastCmd;
				else if ((cmd[0] >= '0') && (cmd <= "61"))
				{
					choice = stoi(cmd);
					lastCmd = cmd;
				}
				else
					break;
			}
		}
	}

	if((NULL != remoteObject) && (NULL != secondaryObject))
	{
		delete remoteObject;
		delete secondaryObject;
		remoteObject = NULL;
		secondaryObject = NULL;
	}
        (void)argc;
        (void)argv;
	return 0;
}
