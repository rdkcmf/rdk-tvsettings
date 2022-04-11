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

#ifndef PERSISTENCESTORAGE_H
#define PERSISTENCESTORAGE_H 

#include "string.h"
#include <pthread.h>
#include "Module.h"
#include "psError.h"


#define DECLARE_JSON_RPC_METHOD(method) \
uint32_t method(const JsonObject& parameters, JsonObject& response);

namespace WPEFramework {
namespace Plugin {

    class PersistenceStorage  : public PluginHost::JSONRPC, public PluginHost::IPlugin {
    private:
        PersistenceStorage(const PersistenceStorage&) = delete;
        PersistenceStorage& operator=(const PersistenceStorage&) = delete;

        std::map <std::string, std::string> _properties;
        std::string filePath;


public:
	PersistenceStorage();
        virtual ~PersistenceStorage();
	static PersistenceStorage& getInstance(void);
 	uint32_t load(const JsonObject& parameters, JsonObject& response);
        uint32_t setPluginParamSettings(const JsonObject& parameters, JsonObject& response);
	uint32_t clearPluginParamSettings(const JsonObject& parameters, JsonObject& response);
        psError_t loadPluginParamFromFile (const string &fileName);
	void      writePluginParamToFile (const string &fileName);

	BEGIN_INTERFACE_MAP(PersistenceStorage)
        INTERFACE_ENTRY(PluginHost::IPlugin)
        INTERFACE_ENTRY(PluginHost::IDispatcher)
        END_INTERFACE_MAP


    public:
        //   IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        virtual const string Initialize(PluginHost::IShell* service);
        virtual void Deinitialize(PluginHost::IShell* service);
        virtual string Information() const;

    private:
        uint8_t _skipURL;
        
        std::string numberToString (int number);
        int stringToNumber (std::string text);

    };

} // Namespace Plugin.
}
#endif // PERSISTENCESTORAGE_H
