cmake_minimum_required(VERSION 3.15)
project(object_detection LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
find_package(OpenCV 5 REQUIRED COMPONENTS
    core
    imgproc
    highgui
    videoio
    dnn
)
 
add_executable(object_detection main.cpp)
set_target_properties(object_detection PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
)
target_link_libraries(object_detection PRIVATE ${OpenCV_LIBS})