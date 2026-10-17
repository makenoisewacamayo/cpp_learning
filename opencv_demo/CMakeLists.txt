cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)
set(CMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk)
set(CMAKE_OSX_DEPLOYMENT_TARGET=15.4)

project(learning_cpp_opencv_demo VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${CMAKE_JS_INC} "addons")
file(GLOB SOURCES "addons/*.cc" "addons/*.cpp" "addons/*.c")

# add these lines for opencv
set(HINT_OPENCV_DIR "${HOMEBREW_CELLAR}}/opencv/5.0.0_10/lib")
if(NOT DEFINED OpenCV_DIR)
  set(OpenCV_DIR "${HINT_OPENCV_DIR}/cmake/opencv5" CACHE PATH "OpenCV cmake dir")
endif()

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)
message(STATUS "OpenCV found: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")

include_directories(${OpenCV_INCLUDE_DIRS})




add_library(${PROJECT_NAME} SHARED ${SOURCES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${OpenCV_LIBS})

# node-addon-api include path
execute_process(
    COMMAND node -p "require('node-addon-api').include_dir"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    NAPI_VERSION=9
    NAPI_DISABLE_CPP_EXCEPTIONS
)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_SOURCE_DIR}/dist/$<TARGET_FILE_NAME:${PROJECT_NAME}>
)
