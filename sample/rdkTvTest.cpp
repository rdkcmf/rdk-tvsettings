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

#include <iostream>
#include "rdkTvSettings.hpp"

std::vector<std::string> allPicModes;
std::vector<std::string> allDVModes;


void printHelp() {
    printf("############################################## \n");
    printf("############# TVSettings-hal API ############# \n");
    printf("############################################## \n");

    printf("  1  GetBrightness\n");
    printf("  2  GetContrast\n");
    printf("  3  GetSaturation\n");
    printf("  4  GetBacklight\n");
    printf("  5  GetPictureMode\n");
    printf("  6  GetSharpness\n");
    printf("  7  GetHue\n");
    printf("  8  GetColorTemperature\n");
    printf("  9  GetAspectRatio\n\n");
    printf("  a  SetBrightness\n");
    printf("  b  SetContrast\n");
    printf("  c  SetSaturation\n");
    printf("  d  SetSharpness\n");
    printf("  e  SetBacklight\n");
    printf("  f  SetPictureMode\n");
    printf("  g  SetHue\n");
    printf("  h  SetColorTemperature\n");
    printf("  i  SetAspectRatio\n");
    printf("  j  SetDynamicContrast\n");
    printf("  k  GetDynamicContrast\n");
    printf("  w  SetWakeupSrcConfig\n");
    printf("  x  getDVMode\n");
    printf("  y  setDVMode\n");
    printf("  q  stop\n");
    printf("############################################## \n");

}

void printAllPicModes(void)
{
    for(unsigned int i=0;i<allPicModes.size();i++)
    {
        printf(" %u : %s \n",i,allPicModes[i].c_str());
    }
}

void printAllDVModes(void)
{
    for(unsigned int i=0;i<allDVModes.size();i++)
    {
        printf(" %u : %s \n",i,allDVModes[i].c_str());
    }
}

int main(int argc, char *argv[]) {

    printHelp();

    char Command[1];
    int run = 1;
    int setValue = 0;
    int configVal = 0;

    TVSettings tv = TVSettings::getInstance();  
    tv.getSupportedPictureModes(allPicModes);
    tv.getSupportedDVModes(allDVModes);
    while (run) {
        scanf("%s", Command);
        switch (Command[0]) {
          case 'q': {
            run = 0;
            break;
          }
          case '1': {
              printf("Brightness Level: %d\n",tv.getBrightness());
              break;
          }
         case '2': {
              printf("Contrast Level: %d\n",tv.getContrast());
              break;
          }
          case '3': {
              printf("Saturation Level: %d\n",tv.getSaturation());
              break;
          }
          case '4': {
              printf("Backlight Level: %d\n",tv.getBacklight());
              break;
          }
          case '5': {
              std::string picmode="";
              tv.getPictureMode(picmode);
              printf("Picture Mode: %s\n",picmode.c_str());
              break;
          }
          case '6': {
              printf("Sharpness Level: %d\n",tv.getSharpness());
              break;
          }
          case '7': {
              printf("Hue Level: %d\n",tv.getHue());
              break;
          }
          case '8': {
              printf("Color Temperature: %d\n",tv.getColorTemperature());
              break;
          }
          case '9': {
              printf("Aspect Ratio: %d\n",tv.getAspectRatio());
              break;
          }
          case 'a': {
              printf("Enter Brightness value 0-100\n");
              scanf("%d",&setValue);
              tv.setBrightness(setValue);
              break;
          }
          case 'b': {
              printf("Enter Contrast value 0-100\n");
              scanf("%d",&setValue);
              tv.setContrast(setValue);
              break;
          }
          case 'c': {
              printf("Enter Saturation value 0-100\n");
              scanf("%d",&setValue);
              tv.setSaturation(setValue);
              break;
          }
          case 'd': {
              printf("Enter Sharpness value 0-100\n");
              scanf("%d",&setValue);
              tv.setSharpness(setValue);
              break;
          }
          case 'e': {
              printf("Enter Backlight value 0-100\n");
              scanf("%d",&setValue);
              tv.setBacklight(setValue);
              break;
          }
          case 'f': {
            if(allPicModes.size())
            {
                printAllPicModes();
                scanf("%d",&setValue);
                if((size_t)setValue <= allPicModes.size())
                    tv.setPictureMode(allPicModes[setValue]);
            }  
            else
                printf("Sorry No pic modes available, check config \n");
              break;
          }
          case 'g': {
              printf("Enter Hue value 0-100\n");
              scanf("%d",&setValue);
              tv.setHue(setValue);
              break;
          }
          case 'h': {
              printf("Enter Color Temperature value:\ntvColorTemp_STANDARD --> 0\ntvColorTemp_WARM --> 1\ntvColorTemp_COLD --> 2\ntvColorTemp_USER --> 3\n");
              scanf("%d",&setValue);
              tv.setColorTemperature((tvColorTemp_t)setValue);
              break;
          }
          case 'i': {
              printf("Enter Aspect Ratio value\ntvDisplayMode_4x3 --> 0\ntvDisplayMode_16x9 --> 1\ntvDisplayMode_FULL --> 2\ntvDisplayMode_NORMAL -->3\n");
              scanf("%d",&setValue);
              tv.setAspectRatio((tvDisplayMode_t)setValue);
              break;
          }
          case 'j': {
              printf("Enter set Dynamic Contrast Mode 1-enabled/0-disabled\n");
              scanf("%d",&setValue);
              if (setValue == 0)
                  tv.setDCMode("disabled");
              else if (setValue == 1)
                  tv.setDCMode("enabled");
              break;
          }
          case 'k': {
              printf("Enter get Dynamic Contrast Mode\n");
              std::string dcmode="";
              tv.getDCMode(dcmode);
              printf("Dynamic Contrast Mode: %s\n", dcmode.c_str());
              break;
          }
          case 'w': {
              printf("Enter Wakeup source configuration type: \ntvWakepusrc_VOICE --> 0\ntvWakeupSrc_PRESENCE_DETECTION --> 1\n \
			tvWakeupSrc_BLUEOTH --> 2\n tvWakeupSrc_WIFI --> 3\n tvWakeupSrc_IR 4\n tvWakeupSrc_POWER_KEY --> 5\n tvWakeupSrc_TIMER --> 6\n tvWakeupSrc_CEC --> 7\n ");
	      scanf("%d", &setValue);
	      printf("Enter enable/disable wakeup source confiuration\n Enable --> 1\n Disable --> 0");
	      scanf("%d", &configVal);
	      tv.setWakeupConfig((tvWakeupSrcType_t)setValue, (bool)configVal);
	      break;
          }
        case 'x': {
              std::string dvmode="";
              tv.getDVMode(dvmode);
              printf("DolbyVision Mode: %s\n",dvmode.c_str());
              break;
          }
        case 'y': {
            if(allDVModes.size())
            {
                printAllDVModes();
                scanf("%d",&setValue);
                if((size_t)setValue <= allDVModes.size())
                    tv.setDVMode(allDVModes[setValue]);
            }  
            else
                printf("Sorry No DV modes available, check config \n");
              break;
        }    
          default: {
              printHelp();
              break;
          }
        }
        fflush (stdout);
}
    (void)argc;
    (void)argv;
    return 0;
}
