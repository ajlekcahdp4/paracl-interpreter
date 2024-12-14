add_llvm_based_lib(paracl-llvm
  LLVM_COMPONENTS
  target
  core
  executionengine
  interpreter
  mcjit
  support
  SOURCES
  codegen.cpp
)
target_include_directories(paracl-llvm PUBLIC ${PARACL_INCLUDE_DIR})
target_link_libraries(paracl-llvm PUBLIC paracl_compiler)
