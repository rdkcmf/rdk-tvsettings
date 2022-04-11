# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2022 Sky UK
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_path (STAGING_INCDIR glib-2.0)

include_directories(include)
include_directories(${STAGING_INCDIR})
include_directories(${STAGING_INCDIR}/rdk/tv-hal)

include_directories(../tvmgr-plugins)

set (LIBRDKTV_SOURCES rdkTvSettings.cpp)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-attributes -Wall -Wextra -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes -Wall -Wextra -fpermissive -std=c++11")

add_library(rdktv SHARED ${LIBRDKTV_SOURCES})
target_link_libraries(rdktv "-lWPEFrameworkWebSocket -lWPEFrameworkCore -lWPEFrameworkSecurityUtil -lWPEFrameworkTracing")

install(TARGETS rdktv DESTINATION lib)
