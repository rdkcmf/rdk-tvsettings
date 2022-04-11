#!/bin/bash

testdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
url="\'http://127.0.0.1:9998/jsonrpc\'"
t=`/usr/bin/WPEFrameworkSecurityUtility`; t=${t%\",*}; t=${t#*:\"}
token="Authorization:Bearer $t"
content="\"Content-Type:application/json\""
curl_opts="--silent -H \"${token}\" -d"
container_id="TestContainer"



get_brightness()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getBrightness"}')
	echo ${ret}
	echo ""
}

get_brightness2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getBrightness"}')
	echo ${ret}
	echo ""
}

set_brightness()
{
	param=$1
	#    echo $param
	#    urlCommand='{"jsonrpc": "2.0","id": 4,"method": ""org.rdk.tv.ControlSettings.1.setBrightness", "params": {"brightness":"$1"}}'
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setBrightness", "params": {"brightness":'$param'}}')
	echo ${ret}
	echo ""
}

get_backlight()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getBacklight"}')
	echo ${ret}
	echo ""
}

get_backlight2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getBacklight"}')
	echo ${ret}
	echo ""
}

get_contrast()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getContrast"}')
	echo ${ret}
	echo ""
}

get_contrast2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getContrast"}')
	echo ${ret}
	echo ""
}

get_saturation()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getSaturation"}')
	echo ${ret}
	echo ""
}

get_saturation2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getSaturation"}')
	echo ${ret}
	echo ""
}


get_sharpness()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getSharpness"}')
	echo ${ret}
	echo ""
}

get_sharpness2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getSharpness"}')
	echo ${ret}
	echo ""
}

get_hue()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getHue"}')
	echo ${ret}
	echo ""
}

get_hue2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getHue"}')
	echo ${ret}
	echo ""
}

get_colortemperature()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getColorTemperature"}')
	echo ${ret}
	echo ""
}

get_colortemperature2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getColorTemperature"}')
	echo ${ret}
	echo ""
}

get_aspectratio()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getAspectRatio"}')
	echo ${ret}
	echo ""
}

get_aspectratio2()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getAspectRatio"}')
	echo ${ret}
	echo ""
}

get_videoresolution()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getVideoResolution"}')
	echo ${ret}
	echo ""
}

set_backlight()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setBacklight", "params": {"backlight":'$param'}}')
	echo ${ret}
	echo ""
}

set_backlightfade()
{
	param1=$1
	param2=$2
	param3=$3
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.3.setBacklightFade", "params": {"from":'$param1', "to":'$param2', "duration":'$param3'}}')
	echo ${ret}
	echo ""
}

set_contrast()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setContrast", "params": {"contrast":'$param'}}')
	echo ${ret}
	echo ""
}

set_saturation()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setSaturation", "params": {"saturation":'$param'}}')
	echo ${ret}
	echo ""
}

set_sharpness()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setSharpness", "params": {"sharpness":'$param'}}')
	echo ${ret}
	echo ""
}

set_hue()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setHue", "params": {"hue":'$param'}}')
	echo ${ret}
	echo ""
}

set_colortemperature()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setColorTemperature", "params": {"colorTemp":'$param'}}')
	echo ${ret}
	echo ""
}

set_aspectratio()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setAspectRatio", "params": {"aspectRatio":'$param'}}')
	echo ${ret}
	echo ""
}

set_aspectratio2()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setAspectRatio", "params": {"aspectRatio":'$param'}}')
	echo ${ret}
	echo ""
}

get_videoframerate()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getVideoFrameRate"}')
	echo ${ret}
	echo ""
}

commit_wb()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.commitWB"}')
	echo ${ret}
	echo ""
}

get_wbinfo()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getWBInfo"}')
	echo ${ret}
	echo ""
}

enable_wbmode()
{
	param=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.enableWBMode", "params": {"mode":'$param'}}')
	echo ${ret}
	echo ""
}

set_wakeupconfiguration()
{
	param1=$1
	param2=$2
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setWakeupConfiguration", "params": {"wakeupSrc":'$param1',"config":'$param2'}}')
	echo ${ret}
	echo ""
}

get_wbcontrol()
{
	param1=$1
	param2=$2
	param3=$3
	param4=$4
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.getWBCtrl", "params": {"applies":[{"selector":"color temp","index":'$param3'},{"selector":"input","index":'$param2'}],"color":'$param1',"ctrl":'$param4'}}')
	echo ${ret}
	echo ""
}

set_wbcontrol()
{
	param1=$1
	param2=$2
	param3=$3
	param4=$4
	param5=$5
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.1.setWBCtrl", "params": {"applies":[{"selector":"color temp","index":'$param3'},{"selector":"input","index":'$param2'}],"color":'$param1',"ctrl":'$param4',"value":'$param5'}}')
	echo ${ret}
	echo ""
}

get_dynamiccontrast()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getDynamicContrast"}')
	echo ${ret}
	echo ""
}

get_componentcolorinfo()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getComponentColorInfo"}')
	echo ${ret}
	echo ""
}

get_allbacklightdimmingmodes()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getAllBacklightDimmingModes"}')
	echo ${ret}
	echo ""
}

get_SupportedHDR10Modes()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getSupportedHDR10Modes"}')
	echo ${ret}
	echo ""
}

get_SupportedHLGModes()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getSupportedHLGModes"}')
	echo ${ret}
	echo ""
}

get_backlightdimmingmode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getBacklightDimmingMode"}')
	echo ${ret}
	echo ""
}

get_HDR10Mode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getHDR10Mode"}')
	echo ${ret}
	echo ""
}

get_HLGMode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getHLGMode"}')
	echo ${ret}
	echo ""
}

get_globalbacklightfactor()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getGlobalBacklightFactor"}')
	echo ${ret}
	echo ""
}

get_componentsaturation()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getComponentSaturation", "params": {"color":'$param1'}}')
	echo ${ret}
	echo ""
}

get_componenthue()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getComponentHue", "params": {"color":'$param1'}}')
	echo ${ret}
	echo ""
}

get_componentluma()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getComponentLuma", "params": {"color":'$param1'}}')
	echo ${ret}
	echo ""
}

set_componentsaturation()
{
	param1=$1
	param2=$2
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setComponentSaturation", "params": {"color":'$param1',"saturation":'$param2'}}')
	echo ${ret}
	echo ""
}

set_componenthue()
{
	param1=$1
	param2=$2
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setComponentHue", "params": {"color":'$param1',"hue":'$param2'}}')
	echo ${ret}
	echo ""
}

set_componentluma()
{
	param1=$1
	param2=$2
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setComponentLuma", "params": {"color":'$param1',"luma":'$param2'}}')
	echo ${ret}
	echo ""
}

set_backlightdimmingmode()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setBacklightDimmingMode", "params": {"DimmingMode":'$param1'}}')
	echo ${ret}
	echo ""
}

set_HDR10Mode()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setHDR10Mode", "params": {"HDR10Mode":'$param1'}}')
	echo ${ret}
	echo ""
}

set_HLGMode()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setHLGMode", "params": {"HLGMode":'$param1'}}')
	echo ${ret}
	echo ""
}

set_picturemode()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setPictureMode", "params": {"pictureMode":'$param1'}}')
	echo ${ret}
	echo ""
}

set_dolbyvisionmode()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setDolbyVisionMode", "params": {"DolbyVisionMode":'$param1'}}')
	echo ${ret}
	echo ""
}

set_dynamiccontrast()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setDynamicContrast", "params": {"DynamicContrast":'$param1'}}')
	echo ${ret}
	echo ""
}

set_globalbacklightfactor()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setGlobalBacklightFactor", "params": {"BacklightFactor":'$param1'}}')
	echo ${ret}
	echo ""
}

set_autobacklightcontrol()
{
	param1=$1
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.setAutoBacklightControl", "params": {"mode":'$param1'}}')
	echo ${ret}
	echo ""
}

get_supportedpicturemodes()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getSupportedPictureModes"}')
	echo ${ret}
	echo ""
}

get_videoformat()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getVideoFormat"}')
	echo ${ret}
	echo ""
}

get_autobacklightcontrol()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getAutoBacklightControl"}')
	echo ${ret}
	echo ""
}

reset_brightness()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetBrightness"}')
	echo ${ret}
	echo ""
}

reset_hue()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetHue"}')
	echo ${ret}
	echo ""
}

reset_saturation()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetSaturation"}')
	echo ${ret}
	echo ""
}

reset_sharpness()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetSharpness"}')
	echo ${ret}
	echo ""
}

reset_backlight()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetBacklight"}')
	echo ${ret}
	echo ""
}

reset_colortemperature()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetColorTemperature"}')
	echo ${ret}
	echo ""
}

reset_dolbyvisionmode()
{
	echo ""
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetDolbyVisionMode"}')
	echo ${ret}
	echo ""
}

reset_autobacklight()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetAutoBacklight"}')
	echo ${ret}
	echo ""
}

reset_contrast()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetContrast"}')
	echo ${ret}
	echo ""
}

reset_WBCtrl()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetWBCtrl"}')
	echo ${ret}
	echo ""
}

reset_aspectratio()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetAspectRatio"}')
	echo ${ret}
	echo ""
}

reset_componentsaturation()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetComponentSaturation"}')
	echo ${ret}
	echo ""
}

reset_componentluma()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetComponentLuma"}')
	echo ${ret}
	echo ""
}

reset_componenthue()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetComponentHue"}')
	echo ${ret}
	echo ""
}

reset_backlightdimmingmode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetBacklightDimmingMode"}')
	echo ${ret}
	echo ""
}

reset_HDR10Mode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetHDR10Mode"}')
	echo ${ret}
	echo ""
}

reset_HLGMode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.resetHLGMode"}')
	echo ${ret}
	echo ""
}

get_picturemode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getPictureMode"}')
	echo ${ret}
	echo ""
}

get_supporteddolbyvisionmodes()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getSupportedDolbyVisionModes"}')
	echo ${ret}
	echo ""
}

get_dolbyvisionmode()
{
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "org.rdk.tv.ControlSettings.2.getDolbyVisionMode"}')
	echo ${ret}
	echo ""
}


tvmgr_help()
{
	echo ""
	echo ""
	echo "Methods:"
	echo ""
	echo "1.getBrightness"
	echo "2.setBrightness"
	echo "3.getBacklight"
	echo "4.getContrast"
	echo "5.getSaturation"
	echo "6.getSharpness"
	echo "7.getHue"
	echo "8.getColorTemperature"
	echo "9.getAspectRatio"
	echo "10.setBacklight"
	echo "11.setContrast"
	echo "12.setSaturation"
	echo "13.setSharpness"
	echo "14.setHue"
	echo "15.setColorTemperature"
	echo "16.setAspectRatio"
	echo "17.getVideoResolution"
	echo "18.getVideoFrameRate"
	echo "19.getWBInfo"
	echo "20.enableWBMode"
	echo "21.setWakeupConfiguration"
	echo "22.getWBCtrl"
	echo "23.setWBCtrl"
	echo "24.getDynamicContrast"
	echo "25.getComponentColorInfo"
	echo "26.getAllBacklightDimmingModes"
	echo "27.getBacklightDimmingMode"
	echo "28.getGlobalBacklightFactor"
	echo "29.getComponentSaturation"
	echo "30.getComponentHue"
	echo "31.getComponentLuma"
	echo "32.setComponentSaturation"
	echo "33.setComponentHue"
	echo "34.setComponentLuma"
	echo "35.setBacklightDimmingMode"
	echo "36.setPictureMode"
	echo "37.setDolbyVisionMode"
	echo "38.setDynamicContrast"
	echo "39.setGlobalBacklightFactor"
	echo "40.setAutoBacklightControl"
	echo "41.getSupportedPictureModes"
	echo "42.getVideoFormat"
	echo "43.getAutoBacklightControl"
	echo "44.resetBrightness"
	echo "45.resetSharpness"
	echo "46.resetSaturation"
	echo "47.resetHue"
	echo "48.resetBacklight"
	echo "49.resetColorTemperature"
	echo "50.resetDolbyVisionMode"
	echo "51.resetAutoBacklight"
	echo "52.resetContrast"
	echo "53.resetAspectRatio"
	echo "54.resetComponentSaturation"
	echo "55.resetComponentLuma"
	echo "56.resetComponentHue"
	echo "57.resetBacklightDimmingMode"
	echo "58.getPictureMode"
	echo "59.getSupportedDolbyVisionModes"
	echo "60.getDolbyVisionMode"
	echo "61.setAspectRatio2"
	echo "62.getAspectRatio2"
	echo "63.getBacklight2"
	echo "64.getBrightness2"
	echo "65.getColorTemperature2"
	echo "66.getHue2"
	echo "67.getSharpness2"
	echo "68.getContrast2"
	echo "69.getSaturation2"
	echo "70.setHDR10Mode"
	echo "71.getHDR10Mode"
	echo "72.resetHDR10Mode"
	echo "73.getSupportedHLGModes"
	echo "74.setHLGMode"
	echo "75.getHLGMode"
	echo "76.resetHLGMode"
	echo "77.getSupportedHLGModes"
	echo "78.setBacklightFade"
	echo "79.resetWBCtrl"
	echo "h  -help to show command input"
	echo "x - exit the script."
	echo ""
}

retval=1
check_plugin()
{
	sub='deactivated'
	sub1='null'
	ret=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "Controller.1.status@org.rdk.tv.ControlSettings"}')
	# echo ${ret}

	if [[ "$ret" == *"$sub"* ]]; then
		rtrn=$(curl 'http://127.0.0.1:9998/jsonrpc' --silent -H "$token" -d '{"jsonrpc": "2.0","id": 4,"method": "Controller.1.activate", "params":{"callsign":"org.rdk.tv.ControlSettings"}}')

		if [[ "$rtrn" == *"$sub1"* ]]; then
			echo "Plugin is activated."
			retval=1
		elif [[ "$rtrn" == "ERROR_UNKNOWN_KEY" ]]; then
			echo "Plugin is not available"
			retval=0
		else
			retval=1
		fi
	fi
}

tvmgr_case()
{
	# Read user input and create an args array out of it
	read -p "Command:" input
	read -a args <<< ${input}

	case ${args[0]} in
		1)
			get_brightness ;;
		2)
			read -p "Brightness(0-100): " brightness
			set_brightness $brightness ;;
		3)
			get_backlight ;;
		4)
			get_contrast ;;
		5)
			get_saturation ;;
		6)
			get_sharpness ;;
		7)
			get_hue ;;
		8)
			get_colortemperature ;;
		9)
			get_aspectratio ;;
		10)
			read -p "Backlight(0-100): " backlight
			set_backlight $backlight ;;
		11)
			read -p "Contrast(0-100): " contrast
			set_contrast $contrast ;;
		12)
			read -p "Saturation(0-100): " saturation
			set_saturation $saturation ;;
		13)
			read -p "Sharpness(0-100): " sharpness
			set_sharpness $sharpness ;;
		14)
			read -p "Hue(0-100): " hue
			set_hue $hue ;;
		15)
			read -p "colorTemp(Standard, Cold, Warm, User): " colorTemp
			set_colortemperature $colorTemp ;;
		16)
			read -p "Aspect ratio(0-4): " aspectratio
			set_aspectratio $aspectratio ;;
		17)
			get_videoresolution ;;
		18)
			get_videoframerate ;;
		19)
			get_wbinfo ;;
		20)
			read -p "Enable WB(true/false): " wb
			enable_wbmode $wb ;;
		21)
			read -p "Wake-up source(0-8): " src
			read -p "Config("1"): " cfg
			set_wakeupconfiguration $src $cfg ;;
		22)
			read -p "color temp(cool,normal,warm): " colortemp
			read -p "ctrl(gain/offset): " ctrl
			read -p "Color(red/green/blue): " color
			read -p "index(HDMI, TV, AV): " id
			get_wbcontrol $color $id $colortemp $ctrl ;;
		23)
			read -p "color temp((cool,normal,warm): " colortemp
			read -p "ctrl(gain/offset): " ctrl
			read -p "value((gain)0-2047/(offset)-1024-+1023): " val
			read -p "Color(red/green/blue): " color
			read -p "index(HDMI, TV, AV): " id
			set_wbcontrol $color $id $colortemp $ctrl $val ;;
		24)
			get_dynamiccontrast ;;
		25)
			get_componentcolorinfo ;;
		26)
			get_allbacklightdimmingmodes ;;
		27)
			get_backlightdimmingmode ;;
		28)
			get_globalbacklightfactor ;;
		29)
			read -p "Color(none, red, green, blue, yellow, cyan, magenta): " color
			get_componentsaturation $color ;;
		30)
			read -p "Color(none, red, green, blue, yellow, cyan, magenta): " color
			get_componenthue $color ;;
		31)
			read -p "Color(none, red, green, blue, yellow, cyan, magenta): " color
			get_componentluma $color ;;
		32)
			read -p "Color(none, red, green, blue, yellow, cyan, magenta): " color
			read -p "Saturation(0-100): " sat
			set_componentsaturation $color $sat ;;
		33)
			read -p "Color(none, red, green, blue, yellow, cyan, magenta): " color
			read -p "Hue(0-100): " hue
			set_componenthue $color $hue ;;
		34)
			read -p "Color(none, red, green, blue, yellow, cyan, magenta): " color
			read -p "Luma(0-30): " luma
			set_componentluma $color $luma ;;
		35)
			read -p "Dimming Mode(fixed, global): " dm
			set_backlightdimmingmode $dm ;;
		36)
			read -p "Picture Mode(standard, vivid, colorful, game): " pm
			set_picturemode $pm ;;
		37)
			read -p "Dolby Vision mode(bright, dark): " dv
			set_dolbyvisionmode $dv ;;
		38)
			read -p "Dynamic Contrast(enabled/disabled): " dc
			set_dynamiccontrast $dc ;;
		39)
			read -p "Global Backlight Factor(0-255): " gbf
			set_globalbacklightfactor $gbf ;;
		40)
			read -p "Auto Backlight mode(none, manual, ambient, eco): " abc
			set_autobacklightcontrol $abc ;;
		41)
			get_supportedpicturemodes ;;
		42)
			get_videoformat ;;
		43)
			get_autobacklightcontrol ;;
		44)
			reset_brightness ;;
		45)
			reset_sharpness ;;
		46)
			reset_saturation ;;
		47)
			reset_hue ;;
		48)
			reset_backlight ;;
		49)
			reset_colortemperature ;;
		50)
			reset_dolbyvisionmode ;;
		51)
			reset_autobacklight ;;
		52)
			reset_contrast ;;
		53)
			reset_aspectratio ;;
		54)
			reset_componentsaturation ;;
		55)
			reset_componentluma ;;
		56)
			reset_componenthue ;;
		57)
			reset_backlightdimmingmode ;;
		58)
			get_picturemode ;;
		59)
			get_supporteddolbyvisionmodes ;;
		60)
			get_dolbyvisionmode ;;
		61)
			read -p "Aspect ratio2(0-4): " aspectratio
			set_aspectratio2 $aspectratio ;;
		62)
			get_aspectratio2 ;;
		63)
			get_backlight2 ;;
		64)
			get_brightness2 ;;
		65)
			get_colortemperature2 ;;
		66)
			get_hue2 ;;
		67)
			get_sharpness2 ;;
		68)
			get_contrast2 ;;
		69)
			get_saturation2 ;;
		70)
			read -p "HDR10Mode(fixed, global): " hdrm
			set_HDR10Mode $hdrm ;;
		71)
			get_HDR10Mode ;;
		72)
			reset_HDR10Mode ;;
		73)
			get_SupportedHDR10Modes ;;
		74)
			read -p "HLGMode(fixed, global): " hlgm
			set_HLGMode $hlgm ;;
		75)
			get_HLGMode ;;
		76)
			reset_HLGMode ;;
		77)
			get_SupportedHLGModes ;;
		78)
			read -p "from (0-100): " from
			read -p "to (0-100): " to
			read -p "duration (0-60): " duration
			set_backlightfade $from $to $duration ;;
		79)
			reset_WBCtrl ;;
		h)
			tvmgr_help ;;    
		x)
			exit ;;
		X)
			exit ;;
		*)
			echo "Invalid selection."
			echo ""
			;;
	esac
	main
}

#This function is used in command line, e.g. tvmgr_test.sh -h
help()
{
	echo "          TV Manager test application help:"
	echo "--------------------------------------------------------"
	echo "brightness -							get the brightness value"
	echo "brightness <0-100> -						set the brightness"
	echo "brightness reset -						reset the brightness value"
	echo "backlight - 							get the backlight value"
	echo "backlight <0-100> - 						set the backlight"
	echo "backlight reset - 						reset the backlight value"
	echo "sharpness - 							get the sharpness value"
	echo "sharpness <0-100> - 						set the sharpness"
	echo "sharpness reset - 						reset the sharpness value"
	echo "saturation - 							get the saturation value"
	echo "saturation <0-100> - 						set the saturation"
	echo "saturation reset -						reset the saturation value"
	echo "contrast - 							get the contrast value"
	echo "contrast <0-100> - 						set the contrast"
	echo "contrast reset - 						reset the contrast value"
	echo "hue -								get the Hue value"
	echo "hue <0-100> -							set the Hue"
	echo "hue reset -							reset the Hue value"
	echo "colortemp - 							get the Color Temperature value"
	echo "colortemp <Standard/Cold/Warm/User> -				set the Color Temperature"
	echo "colortemp reset - 						reset the Color Temperature value"
	echo "aspectratio -							get the Aspect Ratio"
	echo "aspectratio <0-4> -						set the Aspect Ratio"
	echo "aspectratio reset -						reset the Aspect Ratio"
	echo "videores -							get the Video Resolution"
	echo "videoframerate -						get the Video FrameRate"
	echo "videoformat -							get the Video Format"
	echo "wb -								get the White balance information"
	echo "wb <true/false> -						Enable/Disable White balance"
	echo "dynamiccontrast -						get the Dynamic Contrast"
	echo "dynamiccontrast <enabled/disabled> -				Enable/Disable Dynamic Contrast"
	echo "globalbacklightfactor -						get the global Backlight factor"
	echo "globalbacklightfactor <0-255> -					set the global Backlight factor"
	echo "backlightdimmingmode -						get the Backlight dimming mode"
	echo "backlightdimmingmode all -					get all the Backlight dimming mode"
	echo "backlightdimmingmode <fixed/global> -				set the Backlight dimming mode"
	echo "backlightdimmingmode reset -					reset the Backlight dimming mode"
	echo "componentsaturation <none/red/green/blue/"
	echo "yellow/cyan/magenta> - 				                get the Component Saturation values"
	echo "componentsaturation <none/red/green/blue/"
	echo "     yellow/cyan/magenta> <0-100> - 				set the Component Saturation values"
	echo "componentsaturation reset - 					reset the Component Saturation values"
	echo "componentluma <none/red/green/blue/"
	echo "             yellow/cyan/magenta> - 				get the Component Luma values"
	echo "componentluma <none/red/green/blue/"
	echo "     yellow/cyan/magenta> <0-30> - 				set the Component Luma values"
	echo "componentluma reset -						reset the Component Luma values"
	echo "componenthue <none/red/green/blue/"
	echo "             yellow/cyan/magenta> - 				get the Component Hue values"
	echo "componenthue <none/red/green/blue/"
	echo "     yellow/cyan/magenta> <0-100> - 				set the Component Hue values"
	echo "componenthue reset - 						reset the Component Hue values"
	echo "autobacklight -							get the Auto Backlight control"
	echo "autobacklight <none/manual/ambient/eco> -			set the Auto Backlight control"
	echo "autobacklight reset -						reset the Auto Backlight control"
	echo "dolbyvisionmode -						get the Dolby Vision mode"
	echo "dolbyvisionmode <bright/dark> -					set the Dolby Vision mode"
	echo "dolbyvisionmode reset -						reset the Dolby Vision mode"
	echo "dolbyvisionmode all -						get all the supported Dolby Vision modes"
	echo "picturemode -							get the Picture mode"
	echo "picturemode <standard/vivid/sports/"
	echo "         game/enerysaving/custom/theater> -			set the Picture mode"
	echo "picturemode all -						get all supported Picture modes"
	echo "wakeupconfig <0-8> <1> -					set the Wake-up source configuration"
	echo "wbcontrol <color-red/green/blue> <Input-HDMI/TV/AV> "
	echo "       <color temp-cool/normal/warm> "
	echo "       <control-gain/offset> -					get the value of White balance control"
	echo "wbcontrol <color-red/green/blue> <Input-HDMI/TV/AV> "
	echo "    <color temp-cool/normal/warm> <control-gain/offset>"
	echo "    <0-2047(gain)/-1024 to 1023(offset)> -			set the White balance control"
	echo "aspectratio2 <0-4> -						set the Aspect Ratio2"
	echo "aspectratio2 -							get the Aspect Ratio2"
	echo "backlight2 - 							get the backlight2 value"
	echo "brightness2 -							get the brightness2 value"
	echo "colortemp2 - 							get the Color Temperature2 value"
	echo "hue2 -								get the Hue2 value"
	echo "sharpness2 - 							get the sharpness2 value"
	echo "contrast2 - 							get the contrast2 value"
	echo "saturation2 - 							get the saturation2 value"
	echo "HDR10Mode -						get the HDR10Mode"
	echo "HDR10Mode all -					get all the HDR10Mode"
	echo "HDR10Mode <fixed/global> -				set the HDR10Mode"
	echo "HDR10Mode reset -					reset the HDR10Mode"
	echo "HLGMode -						get the HLGMode"
	echo "HLGMode all -					get all the HLGMode"
	echo "HLGMode <fixed/global> -				set the HLGMode"
	echo "HLGMode reset -					reset the HLGMode"
	echo "backlightfade <0-100> <0-100> <0-60> - 						set the backlightfade"
	echo "wbcontrol reset -						reset the WBCtrl"
	echo "--------------------------------------------------------"
}

main()
{
	# Check whether plugin is activated or not
	check_plugin
	if [[ $retval == 0 ]]; then    
		echo "Plugin not supported"
		exit 1
	fi

	if [ $# -eq 0 ]; then
		tvmgr_help
		tvmgr_case
	elif [[ $1 == "brightness" ]]; then
		if [ $# -eq 1 ]; then
			get_brightness
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_brightness
			else
				set_brightness $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "brightness2" ]]; then
		if [ $# -eq 1 ]; then
			get_brightness2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "backlight" ]]; then
		if [ $# -eq 1 ]; then
			get_backlight
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_backlight
			else
				set_backlight $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "backlightfade" ]]; then
		if [ $# -eq 3 ]; then
			set_backlightfade $1 $2 $3
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "backlight2" ]]; then
		if [ $# -eq 1 ]; then
			get_backlight2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "sharpness" ]]; then
		if [ $# -eq 1 ]; then
			get_sharpness
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_sharpness
			else
				set_sharpness $2
			fi	    
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "sharpness2" ]]; then
		if [ $# -eq 1 ]; then
			get_sharpness2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "saturation" ]]; then
		if [ $# -eq 1 ]; then
			get_saturation
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_saturation
			else
				set_saturation $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "saturation2" ]]; then
		if [ $# -eq 1 ]; then
			get_saturation2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "contrast" ]]; then
		if [ $# -eq 1 ]; then
			get_contrast
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_contrast
			else
				set_contrast $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "contrast2" ]]; then
		if [ $# -eq 1 ]; then
			get_contrast2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "hue" ]]; then
		if [ $# -eq 1 ]; then
			get_hue
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_hue
			else
				set_hue $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "hue2" ]]; then
		if [ $# -eq 1 ]; then
			get_hue2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "colortemp" ]]; then
		if [ $# -eq 1 ]; then
			get_colortemperature
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_colortemperature
			else
				set_colortemperature $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "colortemp2" ]]; then
		if [ $# -eq 1 ]; then
			get_colortemperature2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "aspectratio" ]]; then
		if [ $# -eq 1 ]; then
			get_aspectratio
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_aspectratio
			else
				set_aspectratio $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "aspectratio2" ]]; then
		if [ $# -eq 1 ]; then
			get_aspectratio2
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_aspectratio
			else
				set_aspectratio2 $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "videores" ]]; then
		if [ $# -eq 1 ]; then
			get_videoresolution
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "videoframerate" ]]; then
		if [ $# -eq 1 ]; then
			get_videoframerate
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "videoformat" ]]; then
		if [ $# -eq 1 ]; then
			get_videoformat
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "wb" ]]; then
		if [ $# -eq 1 ]; then
			get_wbinfo
		elif [ $# -eq 2 ]; then
			if [ $2 == "true" ] || [ $2 == "false" ]; then
				enable_wbmode $2
			else
				echo "Invalid argument"
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "dynamiccontrast" ]]; then
		if [ $# -eq 1 ]; then
			get_dynamiccontrast
		elif [ $# -eq 2 ]; then
			set_dynamiccontrast $2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "globalbacklightfactor" ]]; then
		if [ $# -eq 1 ]; then
			get_globalbacklightfactor
		elif [ $# -eq 2 ]; then
			set_globalbacklightfactor $2
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "backlightdimmingmode" ]]; then
		if [ $# -eq 1 ]; then
			get_backlightdimmingmode
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_backlightdimmingmode
			elif [[ $2 == "all" ]]; then
				get_allbacklightdimmingmodes
			else
				set_backlightdimmingmode $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "HDRl0Mode" ]]; then
		if [ $# -eq 1 ]; then
			get_HDR10Mode
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_HDR10Mode
			elif [[ $2 == "all" ]]; then
				get_SupportedHDR10Modes
			else
				set_HDR10Mode $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "HLGMode" ]]; then
		if [ $# -eq 1 ]; then
			get_HLGMode
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_HLGMode
			elif [[ $2 == "all" ]]; then
				get_SupportedHLGModes
			else
				set_HLGMode $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "componentsaturation" ]]; then
		if [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_componentsaturation
			else
				get_componentsaturation $2
			fi
		elif [ $# -eq 3 ]; then
			set_componentsaturation $2 $3
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "componentluma" ]]; then
		if [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_componentluma
			else
				get_componentluma $2
			fi
		elif [ $# -eq 3 ]; then
			set_componentluma $2 $3
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "componenthue" ]]; then
		if [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_componenthue
			else
				get_componenthue $2
			fi
		elif [ $# -eq 3 ]; then
			set_componenthue $2 $3
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "autobacklight" ]]; then
		if [ $# -eq 1 ]; then
			get_autobacklightcontrol
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_autobacklight
			else
				set_autobacklightcontrol $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "dolbyvisionmode" ]]; then
		if [ $# -eq 1 ]; then
			get_dolbyvisionmode
		elif [ $# -eq 2 ]; then
			if [[ $2 == "reset" ]]; then
				reset_dolbyvisionmode
			elif [[ $2 == "all" ]]; then
				get_supporteddolbyvisionmodes
			else
				set_dolbyvisionmode $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "picturemode" ]]; then
		if [ $# -eq 1 ]; then
			get_picturemode
		elif [ $# -eq 2 ]; then
			if [[ $2 == "all" ]]; then
				get_supportedpicturemodes
			else
				set_picturemode $2
			fi
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "wakeupconfig" ]]; then
		if [ $# -eq 3 ]; then
			set_wakeupconfiguration $2 $3
		else
			echo "Invalid arguments"
		fi
	elif [[ $1 == "wbcontrol" ]]; then
		if [ $# -eq 5 ]; then
			get_wbcontrol $2 $3 $4 $5
		elif [ $# -eq 6 ]; then
			set_wbcontrol $2 $3 $4 $5 $6
		else
			echo "Invalid arguments"
		fi
	else
		help
	fi   
}


echo "TV Manager test tool"
echo "---------------------"
main $@
