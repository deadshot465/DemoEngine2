cmake_minimum_required(VERSION 3.15)
project(DemoEngine)

set(CMAKE_CXX_STANDARD 20)
#set(ENV{VK_ICD_FILENAMES} "/Users/Deadshot465/vulkansdk-macos-1.2.141.2/macOS/share/vulkan/icd.d/MoltenVK_icd.json")
#set(ENV{VK_LAYER_PATH} "/Users/Deadshot465/vulkansdk-macos-1.2.141.2/macOS/share/vulkan/explicit_layer.d")
#set(ENV{VULKAN_SDK} "/Users/Deadshot465/vulkansdk-macos-1.2.141.2/macOS")

find_library(CoreVideoLib CoreVideo)
find_library(IOKitLib IOKit)
find_library(OpenGLLib OpenGL)
find_library(CocoaLib Cocoa)
find_package(Vulkan REQUIRED)

include_directories(.)
include_directories(Includes)

add_executable(DemoEngine
        DemoEngine.cpp
        UtilsCommon.h
        Interfaces/IMappableVK.h
        Interfaces/IWindow.h Interfaces/IWindow.cpp
        GLVK/WindowGLVK.h GLVK/WindowGLVK.cpp
        GLVK/VK/BufferVK.h GLVK/VK/BufferVK.cpp
        GLVK/VK/GraphicsEngineVK.h GLVK/VK/GraphicsEngineVK.cpp
        GLVK/VK/ImageVK.h GLVK/VK/ImageVK.cpp
        GLVK/VK/PipelineVK.h GLVK/VK/PipelineVK.cpp
        GLVK/VK/ShaderVK.h GLVK/VK/ShaderVK.cpp
        GLVK/VK/UtilsVK.h)
target_include_directories(DemoEngine PUBLIC /Users/Deadshot465/vulkansdk-macos-1.2.141.2/macOS/include)
target_link_libraries(DemoEngine ${CoreVideoLib})
target_link_libraries(DemoEngine ${IOKitLib})
target_link_libraries(DemoEngine ${OpenGLLib})
target_link_libraries(DemoEngine ${CocoaLib})
#target_link_libraries(DemoEngine /Users/Deadshot465/vulkansdk-macos-1.2.141.2/macOS/Frameworks/vulkan.framework)
target_link_libraries(DemoEngine Vulkan::Vulkan)
target_link_libraries(DemoEngine ${PROJECT_SOURCE_DIR}/Libs/libglfw3.a)