### CREATIVE ENGINE CMAKE ####
# Exports:
# ${CREATIVE_ENGINE_INCLUDE_DIRS} - used in INCLUDE_DIRECTORIES(...)
# ${CREATIVE_ENGINE_SOURCE_FILES} - used in ADD_EXECUTABLE(...)
# ${CREATIVE_ENGINE_LINK_LIBRARIES} - used in TARGET_LINK_LIBRARIES(...)

set(CREATIVE_ENGINE_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")
if (IS_DIRECTORY "$ENV{CREATIVE_ENGINE_PATH}")
    set(CREATIVE_ENGINE_PATH "$ENV{CREATIVE_ENGINE_PATH}")
else ()
    set(CREATIVE_ENGINE_PATH "${CREATIVE_ENGINE_MODULE_DIR}")
endif ()

option(CONTROLLER_SUPPORT "Enable desktop gamepad input and rumble" ON)

list(INSERT CMAKE_MODULE_PATH 0 "${CREATIVE_ENGINE_PATH}/cmake.files")
if (WIN32 AND NOT EMSCRIPTEN)
    if (NOT MINGW)
        message(FATAL_ERROR "Creative Engine's Windows build requires a MinGW toolchain.")
    endif ()
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
    set(SDL2_LIBRARY SDL2::SDL2)
    set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
elseif (NOT EMSCRIPTEN)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
endif ()

include(ProcessorCount)
ProcessorCount(N)
if (NOT N EQUAL 0)
    set(N, N*2)         # use hyperthreads - processor count is 4 on an i7 that has 8 cores including hyperthreads
    set(${PROJECT_NAME}_FLAGS -j${N})
    set(${PROJECT_NAME}_args ${${PROJECT_NAME}_args} PARALLEL_LEVEL ${N})
endif ()

if (NOT EMSCRIPTEN)
    INCLUDE_DIRECTORIES(
            ${SDL2_INCLUDE_DIR}
            ${SDL2_IMAGE_INCLUDE_DIR}
            ${SDL2_MIXER_INCLUDE_DIR}
    )
endif ()

#Debug purposes
#ADD_COMPILE_DEFINITIONS(__XTENSA__=true)


# Exports:
# ${CREATIVE_ENGINE_INCLUDE_DIRS} - used in INCLUDE_DIRECTORIES(...)
# ${CREATIVE_ENGINE_SOURCE_FILES} - used in ADD_EXECUTABLE(...)
# ${CREATIVE_ENGINE_LINK_LIBRARIES} - used in TARGET_LINK_LIBRARIES(...)


########## RCOMP #########

function(compile_rcomp_and_resources)
    # resource compiler
    SET(RCOMP "${CREATIVE_ENGINE_PATH}/tools/rcomp")

    # build rcomp-src
    ADD_CUSTOM_COMMAND(
            OUTPUT rcomp
            COMMAND cd ${CREATIVE_ENGINE_PATH}/tools/rcomp-src && $(MAKE)  # $(MAKE) passes -j to make
            COMMENT "Building rcomp ${CREATIVE_ENGINE_PATH}"
    )

    # build Resources.bin
    ADD_CUSTOM_COMMAND(
            OUTPUT Resources.bin
            COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -name "Resources.bin" -exec rm -f {} +
            COMMAND find ${CMAKE_CURRENT_BINARY_DIR} -name "BResourceManager.cpp.o" -exec rm -f {} +
            COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/src && ${RCOMP} Resources.r
            DEPENDS rcomp
            COMMENT "Compiling Resources ${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()

# Used in INCLUDE_DIRECTORIES(...)
SET(CREATIVE_ENGINE_INCLUDE_DIRS
        ${CREATIVE_ENGINE_PATH}/src
        ${CREATIVE_ENGINE_PATH}/src/BStore
        ${CREATIVE_ENGINE_PATH}/src/BStore/DesktopStore
        ${CREATIVE_ENGINE_PATH}/src/Audio
        ${CREATIVE_ENGINE_PATH}/src/Audio/mixer
        ${CREATIVE_ENGINE_PATH}/src/Audio/SDL2Audio
        ${CREATIVE_ENGINE_PATH}/src/Audio/LDKAudio
        ${CREATIVE_ENGINE_PATH}/src/Display
        ${CREATIVE_ENGINE_PATH}/src/Display/DesktopDisplay
        ${CREATIVE_ENGINE_PATH}/src/Display/LDKDisplay
        ${CREATIVE_ENGINE_PATH}/src/Controls
        ${CREATIVE_ENGINE_PATH}/src/Controls/DesktopControls
        ${CREATIVE_ENGINE_PATH}/src/Controls/LDKControls
        ${CREATIVE_ENGINE_PATH}/src/Controls/GameShellControls
        ${CREATIVE_ENGINE_PATH}/src/Widgets
        ${CREATIVE_ENGINE_PATH}/src/libxmp
        ${CREATIVE_ENGINE_PATH}/src/libxmp/loaders
)


#This is for LibXMP minimal build
ADD_COMPILE_DEFINITIONS(LIBXMP_CORE_PLAYER=true)

# gather creative-engine sources
# Used in ADD_EXECUTABLE(...)
file(GLOB_RECURSE CREATIVE_ENGINE_SOURCE_FILES CONFIGURE_DEPENDS
        "${CREATIVE_ENGINE_PATH}/src/*.cpp"
        "${CREATIVE_ENGINE_PATH}/src/*.c"
        "${CREATIVE_ENGINE_PATH}/src/*.h"
)


set(_CE_SDL2_LIBRARIES)
if (NOT EMSCRIPTEN)
    set(_CE_SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})
endif ()

# could change to "CE_DIRECT_LINUX_CONTROS_ENABLED=true make"
# Direct linux controls (non-SDL2!)
if (DEFINED ENV{CE_DIRECT_LINUX_CONTROS_ENABLED})
    ADD_COMPILE_DEFINITIONS(__MODUS_TARGET_DIRECT_LINUX_CONTROLS__=true)
else ()
    ADD_COMPILE_DEFINITIONS(__MODUS_TARGET_DESKTOP_CONTROLS__=true)
endif ()


# Network matrix display
if (DEFINED ENV{CE_NETWORK_DISPLAY_ENABLED})
    ADD_COMPILE_DEFINITIONS(__MODUS_TARGET_NETWORK_DISPLAY__=true)

    INCLUDE_DIRECTORIES(
            ${CREATIVE_ENGINE_PATH}/src/Display/NetworkDisplay
            ${CREATIVE_ENGINE_PATH}/src/Controls/DirectLinuxControls
    )

    FIND_PACKAGE(Threads REQUIRED)
    FIND_PACKAGE(Boost REQUIRED)

    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

    SET(_BOOST_AND_THREADS
            ${CMAKE_THREAD_LIBS_INIT}
            ${Boost_System}
    )

elseif (DEFINED ENV{CE_DESKTOP_DISPLAY_ENABLED})
    ADD_COMPILE_DEFINITIONS(__MODUS_TARGET_DESKTOP_DISPLAY__=true)

    #Append image for video
    if (NOT EMSCRIPTEN)
        list(APPEND _CE_SDL2_LIBRARIES ${SDL2_IMAGE_LIBRARIES})
    endif ()

    SET(_BOOST_AND_THREADS "")
endif ()

# Used in TARGET_LINK_LIBRARIES(...)
SET(CREATIVE_ENGINE_LINK_LIBRARIES
        ${_CE_SDL2_LIBRARIES}
        ${_BOOST_AND_THREADS}
)

# Attach the runtime to a game executable. Windows and web use portable embedded resources.
function(creative_engine_attach target)
    cmake_parse_arguments(CE "" "RESOURCE_ARCHIVE" "" ${ARGN})
    target_sources(${target} PRIVATE ${CREATIVE_ENGINE_SOURCE_FILES})
    target_include_directories(${target} PRIVATE ${CREATIVE_ENGINE_INCLUDE_DIRS})
    target_compile_definitions(${target} PRIVATE
            __MODUS_TARGET_SDL2_AUDIO__=true
            CONTROLLER_SUPPORT=$<BOOL:${CONTROLLER_SUPPORT}>
    )
    if (NOT DEFINED ENV{CE_NETWORK_DISPLAY_ENABLED})
        target_compile_definitions(${target} PRIVATE __MODUS_TARGET_DESKTOP_DISPLAY__=true)
    endif ()
    target_link_libraries(${target} PRIVATE ${CREATIVE_ENGINE_LINK_LIBRARIES})

    if (EMSCRIPTEN OR WIN32)
        if (NOT CE_RESOURCE_ARCHIVE)
            message(FATAL_ERROR "creative_engine_attach requires RESOURCE_ARCHIVE for Windows and web.")
        endif ()
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        get_filename_component(archive "${CE_RESOURCE_ARCHIVE}" ABSOLUTE)
        set(generated "${CMAKE_CURRENT_BINARY_DIR}/creative-engine-${target}")
        set(resource_source "${generated}/Resources.cpp")
        set(resource_header "${generated}/Resources.h")
        set(embed_script "${CREATIVE_ENGINE_MODULE_DIR}/scripts/embed-resources.py")
        add_custom_command(OUTPUT "${resource_source}" "${resource_header}"
                COMMAND Python3::Interpreter "${embed_script}" "${archive}" "${generated}"
                DEPENDS "${embed_script}" "${archive}"
                VERBATIM
        )
        target_sources(${target} PRIVATE "${resource_source}" "${resource_header}")
        target_include_directories(${target} BEFORE PRIVATE "${generated}")
        target_compile_definitions(${target} PRIVATE CE_PORTABLE_RESOURCES)
    endif ()

    if (EMSCRIPTEN)
        set(web_saves "${CREATIVE_ENGINE_MODULE_DIR}/scripts/web-saves.js")
        set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${web_saves}")
        target_compile_options(${target} PRIVATE --use-port=sdl2)
        target_link_options(${target} PRIVATE --use-port=sdl2 -lidbfs.js --pre-js "${web_saves}")
    elseif (WIN32)
        target_compile_definitions(${target} PRIVATE SDL_MAIN_HANDLED BUILDING_STATIC)
        target_link_libraries(${target} PRIVATE winpthread)
        target_link_options(${target} PRIVATE -static)
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL2::SDL2>" "$<TARGET_FILE_DIR:${target}>"
                VERBATIM
        )
    endif ()
endfunction()
