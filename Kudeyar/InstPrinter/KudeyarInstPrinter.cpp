//===-------- KudeyarInstPrinter.cpp ---------===//
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
#define DEBUG_TYPE "asm-printer"
#include "KudeyarInstPrinter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#include "KudeyarGenAsmWriter.inc"

void KudeyarInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                  StringRef Annot) 
{
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void KudeyarInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const 
{
  OS << markup("<reg:")
     << getRegisterName(RegNo)
     << markup(">");
 }

void KudeyarInstPrinter::printPCRelImmOperand(const MCInst *MI, unsigned OpNo,
                                             raw_ostream &O) 
{
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm())
    O << Op.getImm();
  else {
    assert(Op.isExpr() && "unknown pcrel immediate operand");
    O << *Op.getExpr();
  }
}

void KudeyarInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O, const char *Modifier) 
{
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) 
  {
    O << getRegisterName(Op.getReg());
  } 
  else if (Op.isImm()) 
  {
    O << Op.getImm();
  } else 
  {
    assert(Op.isExpr() && "unknown operand kind in printOperand");
    O << /* '#' << */ *Op.getExpr();
  }
}

void KudeyarInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo,
                                           raw_ostream &O,
                                           const char *Modifier) 
{
  const MCOperand &Base = MI->getOperand(OpNo);
  const MCOperand &Disp = MI->getOperand(OpNo+1);

  // Print displacement first

  if (Disp.isExpr())
    O << *Disp.getExpr();
  else 
  {
    assert(Disp.isImm() && "Expected immediate in displacement field");
	if (Disp.getImm() != 0)
      O << Disp.getImm();
  }

  // Print register base field
  if (Base.getReg())
    O << '[' << getRegisterName(Base.getReg()) << ']';
}

void KudeyarInstPrinter::printRegisterList(const MCInst *MI, unsigned OpNum,
                                       raw_ostream &O) 
{
  O << "{";
  for (unsigned i = OpNum, e = MI->getNumOperands(); i != e; ++i) 
  {
    if (i != OpNum) O << ", ";
    printRegName(O, MI->getOperand(i).getReg());
  }
  O << "}";
}
