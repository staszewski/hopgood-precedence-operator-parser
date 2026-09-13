cmake_minimum_required(VERSION 3.25)

project(
  hopgood_parser
  VERSION 0.1.0
  DESCRIPTION "A modern C++ reconstruction of Hopgood's operator-precedence parser"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HOPGOOD_ENABLE_SANITIZERS "Enable the supported development sanitizers" OFF)

add_library(hopgood_project_options INTERFACE)
target_compile_features(hopgood_project_options INTERFACE cxx_std_23)

if(MSVC)
  target_compile_options(hopgood_project_options INTERFACE /W4 /permissive-)
else()
  target_compile_options(hopgood_project_options INTERFACE -Wall -Wextra -Wpedantic)
endif()

if(HOPGOOD_ENABLE_SANITIZERS)
  if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "Enabling UBSan; this Apple Clang ASan runtime does not initialize on the current macOS host")
    target_compile_options(hopgood_project_options INTERFACE -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_options(hopgood_project_options INTERFACE -fsanitize=undefined)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(hopgood_project_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(hopgood_project_options INTERFACE -fsanitize=address,undefined)
  else()
    message(WARNING "Sanitizers are not configured for ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

add_library(hopgood src/ast.cpp src/token.cpp src/precedence.cpp src/parser.cpp)
target_include_directories(hopgood PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(hopgood PRIVATE hopgood_project_options)
add_executable(hopgood_parser src/main.cpp)
target_link_libraries(hopgood_parser PRIVATE hopgood hopgood_project_options)

include(CTest)

if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(Catch2)

  add_executable(
    hopgood_tests
    tests/smoke_test.cpp
    tests/token_test.cpp
    tests/precedence_test.cpp
    tests/parser_test.cpp
  )
  target_link_libraries(hopgood_tests PRIVATE hopgood hopgood_project_options Catch2::Catch2WithMain)

  add_test(NAME hopgood_tests COMMAND hopgood_tests)
endif()
