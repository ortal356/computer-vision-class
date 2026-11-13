cmake_minimum_required(VERSION 3.20)
project(PaperclipCounter LANGUAGES CXX)

if(WIN32)
    set(CMAKE_SYSTEM_PROCESSOR "AMD64")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED OpenCV_DIR)
    set(OpenCV_DIR "E:/simulation/opencv/opencv/build")
endif()

# OpenCV 4.5.2's CMake package predates VS2022's MSVC version number.
if(MSVC)
    set(OpenCV_ARCH x64)
    set(OpenCV_RUNTIME vc15)
endif()

find_package(OpenCV REQUIRED)

add_executable(PaperclipCounter main.cpp)
target_link_libraries(PaperclipCounter PRIVATE ${OpenCV_LIBS})
target_include_directories(PaperclipCounter PRIVATE ${OpenCV_INCLUDE_DIRS})

if(MSVC)
    target_compile_options(PaperclipCounter PRIVATE /utf-8)
endif()

set_target_properties(PaperclipCounter PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
