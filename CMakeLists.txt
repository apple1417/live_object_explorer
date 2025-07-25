cmake_minimum_required(VERSION 3.28)
project(live_object_explorer VERSION 1)

add_library(live_object_explorer SHARED)

target_compile_features(live_object_explorer PUBLIC cxx_std_20)
set_target_properties(live_object_explorer PROPERTIES
    COMPILE_WARNING_AS_ERROR True
    INTERPROCEDURAL_OPTIMIZATION True
    EXPORT_COMPILE_COMMANDS True
    PREFIX ""
)

if(MSVC)
    # Under MSVC, enable edit and continue in debug - which conflicts with LTO
    set_target_properties(live_object_explorer PROPERTIES
        MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>"
        INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>
    )
elseif(MINGW)
    # Under MinGW, only enable LTO in release mode - it causes linking errors in debug
    set_target_properties(live_object_explorer PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>
    )
endif()

if(MSVC)
    target_compile_options(live_object_explorer PRIVATE /W4)
else()
    target_compile_options(live_object_explorer PRIVATE -Wall -Wextra -Wpedantic)
endif()
# CMake doesn't understand warnings as errors for MinGW yet
if(MINGW)
    target_compile_options(live_object_explorer PRIVATE -Werror)
endif()

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp" "src/pyunrealsdk*.h")
target_sources(live_object_explorer PUBLIC ${sources})

target_precompile_headers(live_object_explorer PUBLIC "src/pch.h")

install(
    TARGETS live_object_explorer
    RUNTIME DESTINATION .
)
