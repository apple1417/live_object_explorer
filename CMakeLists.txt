cmake_minimum_required(VERSION 3.28)
project(live_object_explorer VERSION 1)

add_library(live_object_explorer SHARED)

target_compile_features(live_object_explorer PUBLIC cxx_std_23)
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


add_library(imgui OBJECT
    "libs/imgui/imgui_demo.cpp"
    "libs/imgui/imgui_draw.cpp"
    "libs/imgui/imgui_tables.cpp"
    "libs/imgui/imgui_widgets.cpp"
    "libs/imgui/imgui.cpp"
    "libs/imgui/backends/imgui_impl_dx9.cpp"
    "libs/imgui/backends/imgui_impl_dx11.cpp"
    "libs/imgui/backends/imgui_impl_dx12.cpp"
    "libs/imgui/backends/imgui_impl_win32.cpp"
)
target_include_directories(imgui PUBLIC "libs/imgui" "libs/imgui/backends")

set(UNREALSDK_SHARED True)
add_subdirectory(libs/unrealsdk EXCLUDE_FROM_ALL)

set(CONFIGURE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/configure")

set(GIT_PRE_CONFIGURE_FILE "src/version.inl.in")
set(GIT_POST_CONFIGURE_FILE "${CONFIGURE_FILES_DIR}/version.inl")
include(common_cmake/git_watcher.cmake)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")
target_sources(live_object_explorer PUBLIC ${sources} "src/versioninfo.rc")
target_include_directories(live_object_explorer PUBLIC "src" "${CONFIGURE_FILES_DIR}")

target_precompile_headers(live_object_explorer PUBLIC "src/pch.h")
target_link_libraries(live_object_explorer PUBLIC
    imgui
    unrealsdk

    dxguid.lib
    dxgi.lib
)

if(MINGW)
    # MinGW needs these specified explicitly?
    target_link_libraries(live_object_explorer PUBLIC
        dwmapi.lib
        d3dcompiler.lib
    )
endif()

install(
    TARGETS live_object_explorer
    RUNTIME DESTINATION .
)
