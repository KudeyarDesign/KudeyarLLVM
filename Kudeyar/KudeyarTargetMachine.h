//===-------- KudeyarTargetMachine.h ---------===//
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
#ifndef LLVM_TARGET_KUDEYAR_TARGETMACHINE_H
#define LLVM_TARGET_KUDEYAR_TARGETMACHINE_H

#include "KudeyarFrameLowering.h"
#include "KudeyarISelLowering.h"
#include "KudeyarInstrInfo.h"
#include "KudeyarRegisterInfo.h"
#include "KudeyarSelectionDAGInfo.h"
#include "KudeyarSubtarget.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetTransformImpl.h"

namespace llvm 
{
/// KudeyarTargetMachine
///
class KudeyarTargetMachine : public LLVMTargetMachine 
{
  KudeyarSubtarget        Subtarget;
  const DataLayout       DL;       // Calculates type size & alignment
  KudeyarInstrInfo        InstrInfo;
  KudeyarTargetLowering   TLInfo;
  KudeyarSelectionDAGInfo TSInfo;
  KudeyarFrameLowering    FrameLowering;
  ScalarTargetTransformImpl STTI;
  VectorTargetTransformImpl VTTI;

public:
  KudeyarTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);

  virtual const TargetFrameLowering *getFrameLowering() const 
  {
    return &FrameLowering;
  }
  
  virtual const KudeyarInstrInfo *getInstrInfo() const  { return &InstrInfo; }
  
  virtual const DataLayout *getDataLayout() const     { return &DL;}
  
  virtual const KudeyarSubtarget *getSubtargetImpl() const { return &Subtarget; }

  virtual const TargetRegisterInfo *getRegisterInfo() const 
  {
    return &InstrInfo.getRegisterInfo();
  }

  virtual const KudeyarTargetLowering *getTargetLowering() const 
  {
    return &TLInfo;
  }

  virtual const KudeyarSelectionDAGInfo* getSelectionDAGInfo() const 
  {
    return &TSInfo;
  }
  
  virtual const ScalarTargetTransformInfo *getScalarTargetTransformInfo()const 
  {
    return &STTI;
  }
  
  virtual const VectorTargetTransformInfo *getVectorTargetTransformInfo()const 
  {
    return &VTTI;
  }
  
  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
}; // KudeyarTargetMachine.

} // end namespace llvm

#endif /* LLVM_TARGET_KUDEYAR_TARGETMACHINE_H */
