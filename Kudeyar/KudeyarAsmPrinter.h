//===-------- KudeyarAsmPrinter.h ---------===//
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
#ifndef KUDEYARASMPRINTER_H
#define KUDEYARASMPRINTER_H

#include "KudeyarMachineFunctionInfo.h"
#include "KudeyarMCInstLower.h"
#include "KudeyarSubtarget.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm 
{
class MCStreamer;
class MachineInstr;
class MachineBasicBlock;
class Module;
class raw_ostream;

class LLVM_LIBRARY_VISIBILITY KudeyarAsmPrinter : public AsmPrinter 
{

  void EmitInstrWithMacroNoAT(const MachineInstr *MI);

public:

  const KudeyarSubtarget *Subtarget;
  const KudeyarMachineFunctionInfo *KudeyarFI;
  KudeyarMCInstLower MCInstLowering;

  explicit KudeyarAsmPrinter(TargetMachine &TM,  MCStreamer &Streamer)
    : AsmPrinter(TM, Streamer), MCInstLowering(*this) 
  {
    Subtarget = &TM.getSubtarget<KudeyarSubtarget>();
  }

  virtual const char *getPassName() const 
  {
    return "Kudeyar Assembly Printer";
  }

  virtual bool runOnMachineFunction(MachineFunction &MF);

//- EmitInstruction() must exists or will have run time error.
  void EmitInstruction(const MachineInstr *MI);
  void printSavedRegsBitmask(raw_ostream &O);
  void printHex32(unsigned int Value, raw_ostream &O);
  void emitFrameDirective();
  const char *getCurrentABIString() const;
  virtual void EmitFunctionEntryLabel();
  virtual void EmitFunctionBodyStart();
  virtual void EmitFunctionBodyEnd();
  void EmitStartOfAsmFile(Module &M);
  virtual MachineLocation getDebugValueLocation(const MachineInstr *MI) const;
  void PrintDebugValueComment(const MachineInstr *MI, raw_ostream &OS);
};
}

#endif /* KUDEYARASMPRINTER_H */

