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
#include "persistencestorage.h"
#include "psError.h"

using namespace std;

#define PLUGIN_Lock(lock) pthread_mutex_lock(&lock)
#define PLUGIN_Unlock(lock) pthread_mutex_unlock(&lock)

static pthread_mutex_t psLock = PTHREAD_MUTEX_INITIALIZER;


#define returnResponse(return_status, error_log) \
	    {response["success"] = return_status; \
		        if(!return_status) \
		            response["error_message"] = _T(error_log); \
		        PLUGIN_Unlock(psLock); \
		        return (Core::ERROR_NONE);}

#define returnIfParamNotFound(param)\
	    if(param.empty())\
    {\
	            printf("method %s missing parameter %s\n", __FUNCTION__, #param);\
	            returnResponse(false,"missing parameter");\
	        }

namespace WPEFramework {
namespace Plugin {

    SERVICE_REGISTRATION(PersistenceStorage,1, 0);

    PersistenceStorage::PersistenceStorage()
               : PluginHost::JSONRPC()
               , _skipURL(0)
    {
        printf("PersistenceStorage ctor\n");
        Register("load", &PersistenceStorage::load, this);
        Register("setPluginParamSettings", &PersistenceStorage::setPluginParamSettings, this);
        Register("clearPluginParamSettings", &PersistenceStorage::clearPluginParamSettings, this);
    }

    PersistenceStorage::~PersistenceStorage()
    {
        printf("PersistenceStorage dtor\n");

        Unregister("load");
        Unregister("setPluginParamSettings");
        Unregister("clearPluginParamSettings");

    }
/* virtual */ const string PersistenceStorage::Initialize(PluginHost::IShell* service)
{
	printf("PersistenceStorage Initialize called\n");
	ASSERT(service != nullptr);
	_skipURL = static_cast<uint8_t>(service->WebPrefix().length());

	return (service != nullptr ? _T("") : _T("No service."));
}


/* virtual */ void PersistenceStorage::Deinitialize(PluginHost::IShell* service)
{
	//Nothing to do at the moment
}


#if 0 
PersistenceStorage::PersistenceStorage(const std::string &storeFileName)
{
    printf("PersistenceStorage Init\n");

    filePath = storeFileName;
}
PersistenceStorage::~PersistenceStorage()
{
   
}
#endif 

PersistenceStorage& PersistenceStorage::getInstance()
{
}
uint32_t PersistenceStorage::load(const JsonObject& parameters, JsonObject& response) 
{
    psError_t ret = psERROR_NONE;
     string value;
     string  key ; 
    key = parameters.HasLabel("filePath") ? parameters["filePath"].String() : "";
    returnIfParamNotFound(value);    

    
    ret = loadPluginParamFromFile(filePath);

    if(ret != psERROR_NONE) {
	    loadPluginParamFromFile(filePath + "tmpDB");
    }
    else {
	    returnResponse(true, "success");
    }
}


uint32_t PersistenceStorage::setPluginParamSettings(const JsonObject& parameters, JsonObject& response)
{
    string key;
    string value;  
    key = parameters.HasLabel("key_name") ? parameters["key_name"].String() : "";
    returnIfParamNotFound(key);
    value = parameters.HasLabel("key_value") ? parameters["key_value"].String() : "";
    returnIfParamNotFound(value);
    
    if( key.empty() || value.empty())
    {
        returnResponse(false, "Given KEY or VALUE is empty...");
    }
    
    std::map <std::string, std::string> :: const_iterator eFound = _properties.find (key);
    if (eFound == _properties.end())
    {
        returnResponse(false, "Given KEY is not available");
    }
    string eRet = eFound->second;
    
    if (eRet.compare(value) == 0) {
        /* Same value. No need to do anything */
        returnResponse(true, "success");
    }

    /* Save a current copy before modifying */
    writePluginParamToFile(filePath + "tmpDB");

    _properties.erase (key);
    
    _properties.insert ({key, value });

     writePluginParamToFile(filePath);
    
    returnResponse(true, "success");
}

uint32_t PersistenceStorage::clearPluginParamSettings(const JsonObject& parameters, JsonObject& response)
{

}
psError_t PersistenceStorage::loadPluginParamFromFile (const string &fileName)
{
      char keyValue[1024]  = "";
    char key[1024] = "";
    FILE *filePtr = NULL;

    filePtr = fopen (fileName.c_str(), "r");
    if (filePtr != NULL) {
        while (!feof(filePtr))
        {
            if (fscanf (filePtr, "%s\t%s\n", key, keyValue) <= 0 )
            {
                cout << "fscanf failed !\n";
            }
            else
            {
                _properties.insert ({key, keyValue });
                return psERROR_NONE;        
            }
        }
        fclose (filePtr);
    }
    else {
         return psERROR_GENERAL;   
    }
    return psERROR_NONE;
}

void PersistenceStorage::writePluginParamToFile (const string &fileName)
{
        unlink(fileName.c_str());

        if(_properties.size() > 0)
    {
                /*
                        * Replacing the ofstream to fwrite
                        * Because the ofstream.close or ofstream.flush or ofstream.rdbuf->sync
                        * does not sync the data onto disk.
                        * TBD - This need to be changed to C++ APIs in future.
                */

                FILE *file = fopen (fileName.c_str(),"w");
                if (file != NULL)
                {
                        for ( auto it = _properties.begin(); it != _properties.end(); ++it )  {
                                string dataToWrite = it->first + "\t" + it->second + "\n";
                                unsigned int size = dataToWrite.length();
                                fwrite(dataToWrite.c_str(),1,size,file);
                                /*cout << "Size " << size <<  endl;*/
                                /*cout << "Item " << it->first << " Value" << it->second << endl;*/
                        }

                        fflush (file); //Flush buffers to FS
                        fsync(fileno(file)); // Flush file to HDD
                        fclose (file);
                }
        
}
}

}

}
