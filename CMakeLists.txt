cmake_minimum_required(VERSION 3.16)
project(oop_lab6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Находим все файлы реализации (.cpp) в папке src и подпапках
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# Основная цель: app
# ВАЖНО: в этой лабе main.cpp лежит в корне, а не в src/
add_executable(app main.cpp ${SRC_FILES})

# Подключаем директорию include
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Подключение GoogleTest для тестирования
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Этот флаг часто нужен для Windows, но не мешает и в Linux
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Сборка тестов
# Находим все файлы тестов в папке tests
file(GLOB_RECURSE TEST_FILES "tests/*.cpp")

add_executable(run_tests ${TEST_FILES} ${SRC_FILES})
target_include_directories(run_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(run_tests PRIVATE gtest_main)

add_test(NAME unit_tests COMMAND run_tests)