cmake_minimum_required(VERSION 3.16)
project(kp CXX)

# ── Стандарт ─────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Платформа ─────────────────────────────────────────────────────────────────
if(NOT WIN32)
    message(FATAL_ERROR
        "Проект работает только на Windows: используются WinAPI и <conio.h>")
endif()

# ── Исходники ─────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES
    kp/main.cpp
    kp/admin/*.cpp
    kp/auth/*.cpp
    kp/clients/*.cpp
    kp/core/*.cpp
    kp/user/*.cpp
)

add_executable(kp ${SOURCES})

# ── Пути включения ────────────────────────────────────────────────────────────
# kp/         — чтобы работали пути вида "../core/Render.h"
# kp/core/    — для json.hpp (nlohmann, single-header)
target_include_directories(kp PRIVATE
    kp
    kp/core
)

# ── Определения ───────────────────────────────────────────────────────────────
target_compile_definitions(kp PRIVATE
    UNICODE
    _UNICODE
)

# ── Опции компилятора (MSVC) ──────────────────────────────────────────────────
if(MSVC)
    target_compile_options(kp PRIVATE
        /W3         # предупреждения
        /utf-8      # исходники в UTF-8
        /MP         # параллельная компиляция
    )
    # Консольная подсистема (не окно)
    set_target_properties(kp PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

# ── Данные: копируем data/ рядом с exe ────────────────────────────────────────
add_custom_command(TARGET kp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/kp/data
        $<TARGET_FILE_DIR:kp>/data
    COMMENT "Копирование data/ в папку сборки"
)
