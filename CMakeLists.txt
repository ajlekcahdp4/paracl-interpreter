cmake_minimum_required(VERSION 3.11)
project(paracl)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT
                                                              ${INSOURCEBUILD})
  message(
    FATAL_ERROR
      "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it."
  )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CONSTEXPR_DEPTH_LIMIT 4096)
# Possibly support windows and MSVC compiler
if(MSVC)
  add_compile_options(/W2)
else()
  add_compile_options(-Wall -fconstexpr-depth=${CONSTEXPR_DEPTH_LIMIT})
endif()

find_package(GTest)
set(NOGTEST
    OFF
    CACHE BOOL "Disable GoogleTest")

enable_testing()
if(NOT NOGTEST
   AND GTEST_FOUND
   AND CMAKE_MINOR_VERSION GREATER_EQUAL 20)
  set(ENABLE_GTEST ON)
else()
  message(WARNING "Google Test disabled")
endif()

set(NOLINT
    ON
    CACHE BOOL "Disable clang-tidy")

if(NOT ${NOLINT})
  find_program(CLANG_TIDY_COMMAND clang-tidy)
  if(CLANG_TIDY_COMMAND)

    # Clang-tidy for linting
    set(CMAKE_CXX_CLANG_TIDY clang-tidy; -checks=google-*,cppcoreguidelines-*)
    set(CMAKE_C_CLANG_TIDY clang-tidy; -checks=google-*,cppcoreguidelines-*)

  else()
    message(WARNING "Unable to find clang-tidy, linter disabled")
  endif()

else()
  message(WARNING "-DNOLINT option provided, linter disabled")
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(BASH_PROGRAM bash)

add_subdirectory(3rd-party/ctti EXCLUDE_FROM_ALL)
add_library(ezvis INTERFACE)
target_include_directories(ezvis INTERFACE include/ezvis)
target_link_libraries(ezvis INTERFACE ctti)
# Add flags for GNU sampling profiler gprof in Debug build and RelWithDebInfo

option(PROFILE OFF)
if(PROFILE)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

option(SANITIZE OFF)
if(SANITIZE)
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined -fno-omit-frame-pointer)
endif()

set(BYTECODE_VM_SOURCES src/bytecode_vm/decl_vm.cc)

add_library(bytecode_vm STATIC ${BYTECODE_VM_SOURCES})
target_include_directories(bytecode_vm PUBLIC include)

set(PCLDIS_SOURCES src/pcldis.cc)

add_library(popl INTERFACE)
target_include_directories(popl INTERFACE 3rd-party/popl)

add_executable(pcldis ${PCLDIS_SOURCES})
target_link_libraries(pcldis popl bytecode_vm)

set(PCLVM_SOURCES src/pclvm.cc)

add_executable(pclvm ${PCLVM_SOURCES})
target_link_libraries(pclvm popl bytecode_vm)

# Find bison and flex and set CMP0098 policy to new to use binary dir
find_package(FLEX REQUIRED)
find_package(BISON 3.5 REQUIRED)

bison_target(
  parser src/frontend/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.y.cc
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bison_paracl_parser.hpp)
flex_target(scanner src/frontend/scanner.l
            ${CMAKE_CURRENT_BINARY_DIR}/scanner.l.cc)
add_flex_bison_dependency(scanner parser)

set(PARACL_COMPILER_SOURCES
    src/frontend/dumper.cc src/frontend/semantic_analyzer.cc
    src/frontend/ast_copier.cc src/codegen.cc)

add_library(
  paracl_compiler STATIC ${PARACL_COMPILER_SOURCES} ${BISON_parser_OUTPUTS}
                         ${FLEX_scanner_OUTPUTS})
target_include_directories(paracl_compiler PUBLIC include
                                                  ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(paracl_compiler ezvis)

add_executable(pclc src/pclc.cc)
target_link_libraries(pclc PUBLIC popl paracl_compiler bytecode_vm)

set(SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

if(BASH_PROGRAM)
  add_test(NAME test.paracl.external
           COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test.sh "$<TARGET_FILE:pclc>"
                   ${CMAKE_CURRENT_SOURCE_DIR}/test/external)

  add_test(NAME test.paracl.basic
           COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test.sh "$<TARGET_FILE:pclc>"
                   ${CMAKE_CURRENT_SOURCE_DIR}/test/basic)

  add_test(NAME test.paracl.functions
           COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test.sh "$<TARGET_FILE:pclc>"
                   ${CMAKE_CURRENT_SOURCE_DIR}/test/functions)
endif()

set(UNIT_TEST_SOURCES
  unit_tests/main.cc 
  unit_tests/ezvis.cc
  unit_tests/directed_graph.cc
  unit_tests/dag.cc
)

if(ENABLE_GTEST AND NOT PARACL_NO_TESTING__)
  add_executable(unit_tests ${UNIT_TEST_SOURCES})
  target_link_libraries(unit_tests ${GTEST_BOTH_LIBRARIES} ctti)
  target_include_directories(unit_tests PRIVATE include/ezvis include)
  gtest_discover_tests(unit_tests)
endif()
