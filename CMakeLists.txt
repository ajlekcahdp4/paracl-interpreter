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

find_program(BASH_PROGRAM bash)

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

# Tools
find_package(FLEX REQUIRED)
find_package(BISON 3.5 REQUIRED)

# External libraries
add_subdirectory(3rd-party/popl EXCLUDE_FROM_ALL)
add_subdirectory(3rd-party/ezvis EXCLUDE_FROM_ALL)
add_subdirectory(3rd-party/graphs EXCLUDE_FROM_ALL)

# Compiler source
set(BYTECODE_VM_SOURCES src/bytecode_vm/decl_vm.cc)

add_library(bytecode_vm STATIC ${BYTECODE_VM_SOURCES})
target_include_directories(bytecode_vm PUBLIC include)

set(PCLDIS_SOURCES src/pcldis.cc)

add_executable(pcldis ${PCLDIS_SOURCES})
target_link_libraries(pcldis popl bytecode_vm)

set(PCLVM_SOURCES src/pclvm.cc)

add_executable(pclvm ${PCLVM_SOURCES})
target_link_libraries(pclvm popl bytecode_vm)

bison_target(
  parser src/frontend/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.y.cc
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/bison_paracl_parser.hpp)
flex_target(scanner src/frontend/scanner.l
            ${CMAKE_CURRENT_BINARY_DIR}/scanner.l.cc)
add_flex_bison_dependency(scanner parser)

set(PARACL_COMPILER_SOURCES
    src/frontend/dumper.cc src/frontend/function_table_filler.cc
    src/frontend/call_collector.cc src/frontend/semantic_analyzer.cc
    src/frontend/ast_copier.cc src/codegen.cc)

add_library(
  paracl_compiler STATIC ${PARACL_COMPILER_SOURCES} ${BISON_parser_OUTPUTS}
                         ${FLEX_scanner_OUTPUTS})
target_include_directories(paracl_compiler PUBLIC include
                                                  ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(paracl_compiler PUBLIC ezvis graphs)

add_executable(pclc src/pclc.cc)
target_link_libraries(pclc PRIVATE ezvis popl paracl_compiler bytecode_vm)

set(SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
if(BASH_PROGRAM)
  add_subdirectory(test)
endif()
