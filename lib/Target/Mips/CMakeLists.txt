llvm_add_library(
  ELDMipsLDBackend
  STATIC
  MIPSELFDynamic.cpp
  MIPSEmulation.cpp
  MIPSInfo.cpp
  MIPSLDBackend.cpp
  MIPSRelocator.cpp
  MIPSTargetMachine.cpp
  MIPSGOT.cpp
  MIPSPLT.cpp)

add_subdirectory(TargetInfo)

target_link_libraries(
  ELDMipsLDBackend PRIVATE ELDMipsTargetInfo ELDCore ELDFragment ELDObject
                           ELDTarget LLVMMipsCodeGen)