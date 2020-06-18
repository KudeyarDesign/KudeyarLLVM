//===-------- KudeyarMCInstLower.h ---------===//
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
#ifndef KUDEYARINSTLOWER_H
#define KUDEYARINSTLOWER_H
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm 
{
  class MCContext;
  class MCInst;
  class MCOperand;
  class MachineInstr;
  class MachineFunction;
  class Mangler;
  class KudeyarAsmPrinter;

/// KudeyarMCInstLower - This class is used to lower an MachineInstr into an
//                    MCInst.
class LLVM_LIBRARY_VISIBILITY KudeyarMCInstLower 
{
  typedef MachineOperand::MachineOperandType MachineOperandType;
//  MCContext &Ctx;
  KudeyarAsmPrinter &Printer;
public:
  KudeyarMCInstLower(KudeyarAsmPrinter &asmprinter);
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
private:
  MCSymbol* GetGlobalAddressSymbol(const MachineOperand &MO) const;

  MCSymbol* GetExternalSymbolSymbol(const MachineOperand &MO) const;

  MCSymbol* GetJumpTableSymbol(const MachineOperand &MO) const; 

  MCSymbol* GetConstantPoolIndexSymbol(const MachineOperand &MO) const;

  MCSymbol* GetBlockAddressSymbol(const MachineOperand &MO) const;

  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

  MCOperand LowerOperand(const MachineOperand& MO, unsigned offset = 0) const;
};
}

#endif /* KUDEYARINSTLOWER_H */
