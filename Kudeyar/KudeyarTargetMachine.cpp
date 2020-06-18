//===-------- KudeyarTargetMachine.cpp ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Year: 2017
// Alexey Shistko     alexey@kudeyar.com
// Andrei Safronov    andrei@kudeyar.com
//===----------------------------------------------------------------------===//
#include "KudeyarTargetMachine.h"
#include "Kudeyar.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeKudeyarTarget() 
{
  // Register the target.
  RegisterTargetMachine<KudeyarTargetMachine> X(TheKudeyarTarget);
}

KudeyarTargetMachine::KudeyarTargetMachine(const Target &T,
                                         StringRef TT,
                                         StringRef CPU,
                                         StringRef FS,
                                         const TargetOptions &Options,
                                         Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
    DL(std::string("e-p:32:32-f64:32:64-i64:32:64")),
    InstrInfo(*this), 
	TLInfo(*this), 
	TSInfo(*this),
    FrameLowering(Subtarget), 
	STTI(&TLInfo), 
	VTTI(&TLInfo) { }

namespace 
{
/// Kudeyar Code Generator Pass Configuration Options.
class KudeyarPassConfig : public TargetPassConfig 
{
public:
  KudeyarPassConfig(KudeyarTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  KudeyarTargetMachine &getKudeyarTargetMachine() const {
    return getTM<KudeyarTargetMachine>();
  }

  virtual bool addInstSelector();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *KudeyarTargetMachine::createPassConfig(PassManagerBase &PM) 
{
  return new KudeyarPassConfig(this, PM);
}

bool KudeyarPassConfig::addInstSelector() 
{
  // Install an instruction selector.
  addPass(createKudeyarISelDag(getKudeyarTargetMachine(), getOptLevel()));
  return false;
}

bool KudeyarPassConfig::addPreEmitPass() 
{
  // Must run branch selection immediately preceding the asm printer.
//  addPass(createKudeyarBranchSelectionPass(getKudeyarTargetMachine(), getOptLevel()));
  return false;
}
