cmake_minimum_required(VERSION 3.16)
project(StarlelabCMD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(starlelab-cmd WIN32
    main.cpp
    terminal.cpp
    terminal.h
)

set_target_properties(starlelab-cmd PROPERTIES OUTPUT_NAME "starlelab cmd")

target_link_libraries(starlelab-cmd PRIVATE Qt6::Widgets)

if (WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${Qt6_DIR}/../../../bin")
    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET starlelab-cmd POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    "$<TARGET_FILE:starlelab-cmd>"
            COMMENT "Deploying Qt runtime")
    endif()
endif()