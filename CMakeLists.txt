cmake_minimum_required(VERSION 3.22)

project(js_juce_host VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext/juce/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/juce" "${CMAKE_BINARY_DIR}/juce")
else()
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_modules("${CMAKE_CURRENT_SOURCE_DIR}/js_juce")

juce_add_gui_app(js_juce_host
    PRODUCT_NAME "js_juce_host"
)

target_sources(js_juce_host
    PRIVATE
        src/Main.cpp
)

target_link_libraries(js_juce_host
    PRIVATE
        js_juce
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
