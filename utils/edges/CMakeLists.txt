cmake_minimum_required(VERSION 3.15)
project(edges_demo LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
find_package(OpenCV 5 REQUIRED COMPONENTS
    core
    imgproc
    highgui
    videoio
)
 
add_executable(edges main.cpp)
set_target_properties(edges PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
)
target_link_libraries(edges PRIVATE ${OpenCV_LIBS})