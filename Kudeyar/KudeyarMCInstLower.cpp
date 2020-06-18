//===-------- KudeyarMCInstLower.cpp ---------===//
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
#include "KudeyarMCInstLower.h"
#include "KudeyarAsmPrinter.h"
#include "KudeyarInstrInfo.h"
//#include "MCTargetDesc/KudeyarBaseInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Target/Mangler.h"

using namespace llvm;

KudeyarMCInstLower::KudeyarMCInstLower(KudeyarAsmPrinter &asmprinter)
  : Printer(asmprinter) {}

KudeyarMCInstLower::KudeyarMCInstLower(KudeyarAsmPrinter &asmprinter)
  : Printer(asmprinter) {}

MCSymbol *KudeyarMCInstLower::
GetGlobalAddressSymbol(const MachineOperand &MO) const 
{
  switch (MO.getTargetFlags()) 
  {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  return Printer.Mang->getSymbol(MO.getGlobal());
}

MCSymbol *KudeyarMCInstLower::
GetExternalSymbolSymbol(const MachineOperand &MO) const 
{
  switch (MO.getTargetFlags()) 
  {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  return Printer.GetExternalSymbolSymbol(MO.getSymbolName());
}

MCSymbol *KudeyarMCInstLower::
GetJumpTableSymbol(const MachineOperand &MO) const 
{
  SmallString<256> Name;
  raw_svector_ostream(Name) << /* Printer.MAI->getPrivateGlobalPrefix()  << */ "JTI"
                            << Printer.getFunctionNumber() << '_'
                            << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  // Create a symbol for the name.
  return Printer.MF->getContext().GetOrCreateSymbol(Name.str());
}

MCSymbol *KudeyarMCInstLower::
GetConstantPoolIndexSymbol(const MachineOperand &MO) const 
{
  SmallString<256> Name;
  raw_svector_ostream(Name) << /* Printer.MAI->getPrivateGlobalPrefix() << */ "CPI"
                            << Printer.getFunctionNumber() << '_'
                            << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  // Create a symbol for the name.
  return Printer.MF->getContext().GetOrCreateSymbol(Name.str());
}

MCSymbol *KudeyarMCInstLower::
GetBlockAddressSymbol(const MachineOperand &MO) const 
{
  switch (MO.getTargetFlags()) 
  {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  return Printer.GetBlockAddressSymbol(MO.getBlockAddress());
}

MCOperand KudeyarMCInstLower::
LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const 
{
  // FIXME: We would like an efficient form for this, so we don't have to do a
  // lot of extra uniquing.
  const MCExpr *Expr = MCSymbolRefExpr::Create(Sym, Printer.MF->getContext());

  switch (MO.getTargetFlags()) 
  {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  if (!MO.isJTI() && MO.getOffset())
    Expr = MCBinaryExpr::CreateAdd(Expr,
                                   MCConstantExpr::Create(MO.getOffset(), Printer.MF->getContext()),
                                   Printer.MF->getContext());
  return MCOperand::CreateExpr(Expr);
}

void KudeyarMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const 
{
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) 
  {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;
    switch (MO.getType()) 
	{
    default:
      MI->dump();
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit()) continue;
      MCOp = MCOperand::CreateReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::CreateImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::CreateExpr(MCSymbolRefExpr::Create(
                         MO.getMBB()->getSymbol(), Printer.MF->getContext()));
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = LowerSymbolOperand(MO, GetGlobalAddressSymbol(MO));
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = LowerSymbolOperand(MO, GetExternalSymbolSymbol(MO));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = LowerSymbolOperand(MO, GetJumpTableSymbol(MO));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = LowerSymbolOperand(MO, GetConstantPoolIndexSymbol(MO));
      break;
    case MachineOperand::MO_BlockAddress:
      MCOp = LowerSymbolOperand(MO, GetBlockAddressSymbol(MO));
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    }

    OutMI.addOperand(MCOp);
  }
}

