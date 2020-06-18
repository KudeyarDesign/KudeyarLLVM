//===-------- KudeyarMCTargetDesc.cpp ---------===//
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
#include "KudeyarMCTargetDesc.h"
#include "InstPrinter/KudeyarInstPrinter.h"
#include "KudeyarMCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "KudeyarGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "KudeyarGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "KudeyarGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createKudeyarMCInstrInfo() 
{
  MCInstrInfo *X = new MCInstrInfo();
  InitKudeyarMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createKudeyarMCRegisterInfo(StringRef TT) 
{
  MCRegisterInfo *X = new MCRegisterInfo();
  InitKudeyarMCRegisterInfo(X, Kudeyar::PSW);
  return X;
}

static MCSubtargetInfo *createKudeyarMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                    StringRef FS) 
{
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitKudeyarMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createKudeyarMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                                CodeModel::Model CM,
                                                CodeGenOpt::Level OL) 
{
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createKudeyarMCInstPrinter(const Target &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI,
                                                const MCSubtargetInfo &STI) 
{
  if (SyntaxVariant == 0)
    return new KudeyarInstPrinter(MAI, MII, MRI);
  return 0;
}

extern "C" void LLVMInitializeKudeyarTargetMC() 
{
  // Register the MC asm info.
  RegisterMCAsmInfo<KudeyarMCAsmInfo> X(TheKudeyarTarget);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheKudeyarTarget,
                                        createKudeyarMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheKudeyarTarget, createKudeyarMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheKudeyarTarget,
                                    createKudeyarMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheKudeyarTarget,
                                          createKudeyarMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheKudeyarTarget,
                                        createKudeyarMCInstPrinter);
}
