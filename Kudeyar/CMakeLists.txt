# CMakeLists.txt
# Our td all in Kudeyar.td, KudeyarRegisterInfo.td and KudeyarInstrInfo.td included in 
#  Kudeyar.td.
set(LLVM_TARGET_DEFINITIONS Kudeyar.td)

# Generate KudeyarGenRegisterInfo.inc and KudeyarGenInstrInfo.inc which included by 
#  your hand code C++ files. 
# KudeyarGenRegisterInfo.inc came from KudeyarRegisterInfo.td, KudeyarGenInstrInfo.inc 
#  came from KudeyarInstrInfo.td.
tablegen(LLVM KudeyarGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM KudeyarGenInstrInfo.inc -gen-instr-info)

tablegen(LLVM KudeyarGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM KudeyarGenMCCodeEmitter.inc -gen-emitter -mc-emitter)

tablegen(LLVM KudeyarGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM KudeyarGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM KudeyarGenCallingConv.inc -gen-callingconv)
tablegen(LLVM KudeyarGenSubtargetInfo.inc -gen-subtarget)

# KudeyarCommonTableGen must be defined
add_public_tablegen_target(KudeyarCommonTableGen)

# KudeyarCodeGen should match with LLVMBuild.txt KudeyarCodeGen
add_llvm_target(KudeyarCodeGen
  KudeyarAsmPrinter.cpp
  KudeyarInstrInfo.cpp
  KudeyarISelDAGToDAG.cpp
  KudeyarISelLowering.cpp
  KudeyarFrameLowering.cpp
  KudeyarMCInstLower.cpp
  KudeyarMachineFunctionInfo.cpp
  KudeyarRegisterInfo.cpp
  KudeyarSubtarget.cpp
  KudeyarTargetMachine.cpp
  KudeyarTargetObjectFile.cpp
  KudeyarSelectionDAGInfo.cpp
  )

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
