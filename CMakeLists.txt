cmake_minimum_required(VERSION 3.22)

project(js_juce_host VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

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

add_library(js_juce_quickjs STATIC
    ext/quickjs/quickjs.c
    ext/quickjs/cutils.c
    ext/quickjs/dtoa.c
    ext/quickjs/libregexp.c
    ext/quickjs/libunicode.c
)
target_include_directories(js_juce_quickjs PRIVATE ext/quickjs)
target_compile_definitions(js_juce_quickjs PRIVATE _GNU_SOURCE CONFIG_VERSION=\"2025-09-13\")

target_sources(js_juce_host
    PRIVATE
        src/Main.cpp
)

target_link_libraries(js_juce_host
    PRIVATE
        js_juce
        js_juce_quickjs
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(BUILD_TESTING)
    juce_add_console_app(js_juce_tests
        PRODUCT_NAME "js_juce_tests"
    )

    target_sources(js_juce_tests
        PRIVATE
            tests/RunUnitTests.cpp
    )

    target_compile_definitions(js_juce_tests
        PRIVATE
            JUCE_UNIT_TESTS=1
    )

    target_link_libraries(js_juce_tests
        PRIVATE
            js_juce
            js_juce_quickjs
            juce::juce_core
            juce::juce_events
            juce::juce_gui_basics
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    add_test(NAME js_juce_bridge_tests COMMAND js_juce_tests)
endif()
