PROJECT(sayo_cli C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "current platform: Linux ")
    set(PLATFORM linux)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "current platform: Windows")
    set(PLATFORM windows)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "current platform: macOS")
    set(PLATFORM mac)
else ()
    message(ERROR "unknown platform: ${CMAKE_SYSTEM_NAME}, you need set platform in CMakeList.txt manually.")
endif ()

set(JsonCpp_INCLUDE_DIR "extern/JsonCpp/include")
set(HIDAPI_INCLUDE_DIR "extern/HIDAPI/hidapi")

INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${JsonCpp_INCLUDE_DIR}
        ${HIDAPI_INCLUDE_DIR}
)

set(JsonCpp_SRC_DIR "extern/JsonCpp/src/lib_json")
set(HIDAPI_SRC_DIR "extern/HIDAPI/${PLATFORM}/")

AUX_SOURCE_DIRECTORY(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        DIR_SRCS)
AUX_SOURCE_DIRECTORY(
        ${JsonCpp_SRC_DIR}
        JsonCpp_SRCS)
AUX_SOURCE_DIRECTORY(
        ${HIDAPI_SRC_DIR}
        HIDAPI_SRCS)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread ")
endif ()

ADD_EXECUTABLE(sayo_cli main.cpp o2_protocol.cpp ${DIR_SRCS} ${JsonCpp_SRCS} ${HIDAPI_SRCS})


if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_link_libraries(sayo_cli "-framework IOKit")
    target_link_libraries(sayo_cli "-framework CoreFoundation")
endif ()
