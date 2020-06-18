//===-------- KudeyarAsmPrinter.cpp ---------===//
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
#define DEBUG_TYPE "kudeyar-asm-printer"
#include "KudeyarAsmPrinter.h"
#include "Kudeyar.h"
#include "KudeyarInstrInfo.h"
#include "InstPrinter/KudeyarInstPrinter.h"
//#include "MCTargetDesc/KudeyarBaseInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

bool KudeyarAsmPrinter::runOnMachineFunction(MachineFunction &MF) 
{
  KudeyarFI = MF.getInfo<KudeyarMachineFunctionInfo>();
  AsmPrinter::runOnMachineFunction(MF);
  return true;
}

//- EmitInstruction() must exists or will have run time error.
void KudeyarAsmPrinter::EmitInstruction(const MachineInstr *MI) 
{
  if (MI->isDebugValue()) 
  {
    SmallString<128> Str;
    raw_svector_ostream OS(Str);

    PrintDebugValueComment(MI, OS);
    return;
  }

  MCInst TmpInst0;
  MCInstLowering.Lower(MI, TmpInst0);
  OutStreamer.EmitInstruction(TmpInst0);
}

//===----------------------------------------------------------------------===//
//
//  Kudeyar Asm Directives
//
//  -- Frame directive "frame Stackpointer, Stacksize, RARegister"
//  Describe the stack frame.
//
//  -- Mask directives "(f)mask  bitmask, offset"
//  Tells the assembler which registers are saved and where.
//  bitmask - contain a little endian bitset indicating which registers are
//            saved on function prologue (e.g. with a 0x80000000 mask, the
//            assembler knows the register 31 (RA) is saved at prologue.
//  offset  - the position before stack pointer subtraction indicating where
//            the first saved register on prologue is located. (e.g. with a
//
//  Consider the following function prologue:
//
//    .frame  $fp,48,$ra
//    .mask   0xc0000000,-8
//       addiu $sp, $sp, -48
//       st $ra, 40($sp)
//       st $fp, 36($sp)
//
//    With a 0xc0000000 mask, the assembler knows the register 31 (RA) and
//    30 (FP) are saved at prologue. As the save order on prologue is from
//    left to right, RA is saved first. A -8 offset means that after the
//    stack pointer subtration, the first register in the mask (RA) will be
//    saved at address 48-8=40.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Mask directives
//===----------------------------------------------------------------------===//
//	.frame	$sp,8,$lr
//->	.mask 	0x00000000,0
//	.set	noreorder
//	.set	nomacro

// Create a bitmask with all callee saved registers for CPU or Floating Point
// registers. For CPU registers consider RA, GP and FP for saving if necessary.
void KudeyarAsmPrinter::printSavedRegsBitmask(raw_ostream &O) 
{
  // CPU and FPU Saved Registers Bitmasks
  unsigned CPUBitmask = 0;
  int CPUTopSavedRegOff;
 
  // Set the CPU and FPU Bitmasks
  const MachineFrameInfo *MFI = MF->getFrameInfo();
  const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
  // size of stack area to which FP callee-saved regs are saved.
  unsigned CPURegSize = Kudeyar::GPRRegClass.getSize();
  unsigned i = 0, e = CSI.size();

  // Set CPU Bitmask.
  for (; i != e; ++i) 
  {
    unsigned Reg = CSI[i].getReg();
    unsigned RegNum = Reg; // getKudeyarRegisterNumbering(Reg);
    CPUBitmask |= (1 << RegNum);
  }

  CPUTopSavedRegOff = CPUBitmask ? -CPURegSize : 0;

  // Print CPUBitmask
  O << "\t.mask \t"; printHex32(CPUBitmask, O);
  O << ',' << CPUTopSavedRegOff << '\n';
}

// Print a 32 bit hex number with all numbers.
void KudeyarAsmPrinter::printHex32(unsigned Value, raw_ostream &O) 
{
  O << "0x";
  for (int i = 7; i >= 0; i--)
    O.write_hex((Value & (0xF << (i*4))) >> (i*4));
}

//===----------------------------------------------------------------------===//
// Frame and Set directives
//===----------------------------------------------------------------------===//
//->	.frame	$sp,8,$lr
//	.mask 	0x00000000,0
//	.set	noreorder
//	.set	nomacro
/// Frame Directive
void KudeyarAsmPrinter::emitFrameDirective() 
{
  const TargetRegisterInfo &RI = *TM.getRegisterInfo();

  unsigned stackReg  = RI.getFrameRegister(*MF);
  unsigned returnReg = RI.getRARegister();
  unsigned stackSize = MF->getFrameInfo()->getStackSize();

  if (OutStreamer.hasRawTextSupport())
    OutStreamer.EmitRawText("\t.frame\t$" +
           StringRef(KudeyarInstPrinter::getRegisterName(stackReg)).lower() +
           "," + Twine(stackSize));
}

/// Emit Set directives.
const char *KudeyarAsmPrinter::getCurrentABIString() const 
{
//  switch (Subtarget->getTargetABI()) 
  {
//  case KudeyarSubtarget::O32:  return "abi32";
//  default: llvm_unreachable("Unknown Kudeyar ABI");;
  }
  return "abi32";
}

//		.type	main,@function
//->		.ent	main                    # @main
//	main:
void KudeyarAsmPrinter::EmitFunctionEntryLabel() 
{
  if (OutStreamer.hasRawTextSupport())
    OutStreamer.EmitRawText("\t.ent\t" + Twine(CurrentFnSym->getName()));
  OutStreamer.EmitLabel(CurrentFnSym);
}


//	.frame	$sp,8,$pc
//	.mask 	0x00000000,0
//->	.set	noreorder
//->	.set	nomacro
/// EmitFunctionBodyStart - Targets can override this to emit stuff before
/// the first basic block in the function.
void KudeyarAsmPrinter::EmitFunctionBodyStart() 
{
//  MCInstLowering.Initialize(Mang, &MF->getContext());

  emitFrameDirective();
  /*
  bool EmitCPLoad = (MF->getTarget().getRelocationModel() == Reloc::PIC_) &&
    KudeyarFI->globalBaseRegSet() &&
    KudeyarFI->globalBaseRegFixed();
  */

  if (OutStreamer.hasRawTextSupport()) 
  {
    SmallString<128> Str;
    raw_svector_ostream OS(Str);
    printSavedRegsBitmask(OS);
    OutStreamer.EmitRawText(OS.str());

    /*
    OutStreamer.EmitRawText(StringRef("\t.set\tnoreorder"));

    // Emit .cpload directive if needed.
    if (EmitCPLoad)
	//- .cpload	$t9
      OutStreamer.EmitRawText(StringRef("\t.cpload\t$t9"));
	//- .cpload	$10
    //  OutStreamer.EmitRawText(StringRef("\t.cpload\t$6"));

    OutStreamer.EmitRawText(StringRef("\t.set\tnomacro"));
     */ 
  }
}

//->	.set	macro
//->	.set	reorder
//->	.end	main
/// EmitFunctionBodyEnd - Targets can override this to emit stuff after
/// the last basic block in the function.
void KudeyarAsmPrinter::EmitFunctionBodyEnd() 
{
  // There are instruction for this macros, but they must
  // always be at the function end, and we can't emit and
  // break with BB logic.
  if (OutStreamer.hasRawTextSupport()) 
  {
//    OutStreamer.EmitRawText(StringRef("\t.set\tmacro"));
//    OutStreamer.EmitRawText(StringRef("\t.set\treorder"));
    OutStreamer.EmitRawText("\t.end\t" + Twine(CurrentFnSym->getName()));
  }
}

//	.section .mdebug.abi32
//	.previous
void KudeyarAsmPrinter::EmitStartOfAsmFile(Module &M) 
{
  /*
  // Tell the assembler which ABI we are using
  if (OutStreamer.hasRawTextSupport())
    OutStreamer.EmitRawText("\t.section .mdebug." +
                            Twine(getCurrentABIString()));

  // return to previous section
  if (OutStreamer.hasRawTextSupport())
    OutStreamer.EmitRawText(StringRef("\t.previous"));
  */
}

MachineLocation
KudeyarAsmPrinter::getDebugValueLocation(const MachineInstr *MI) const 
{
  // Handles frame addresses emitted in KudeyarInstrInfo::emitFrameIndexDebugValue.
  assert(MI->getNumOperands() == 4 && "Invalid no. of machine operands!");
  assert(MI->getOperand(0).isReg() && MI->getOperand(1).isImm() &&
         "Unexpected MachineOperand types");
  return MachineLocation(MI->getOperand(0).getReg(),
                         MI->getOperand(1).getImm());
}

void KudeyarAsmPrinter::PrintDebugValueComment(const MachineInstr *MI,
                                           raw_ostream &OS) 
{
  // TODO: implement
  OS << "PrintDebugValueComment()";
}

// Force static initialization.
extern "C" void LLVMInitializeKudeyarAsmPrinter() 
{
  RegisterAsmPrinter<KudeyarAsmPrinter> X(TheKudeyarTarget);
}
