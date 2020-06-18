add_llvm_library(LLVMKudeyarDesc
  KudeyarMCTargetDesc.cpp
  KudeyarMCAsmInfo.cpp
  )

add_dependencies(LLVMKudeyarDesc KudeyarCommonTableGen)
