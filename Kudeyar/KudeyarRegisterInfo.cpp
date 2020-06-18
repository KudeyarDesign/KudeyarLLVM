//===-------- KudeyarRegisterInfo.cpp ---------===//
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
#define DEBUG_TYPE "kudeyar-reg-info"

#include "KudeyarRegisterInfo.h"
#include "Kudeyar.h"
#include "KudeyarMachineFunctionInfo.h"
#include "KudeyarTargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#define GET_REGINFO_TARGET_DESC
#include "KudeyarGenRegisterInfo.inc"

using namespace llvm;

// FIXME: Provide proper call frame setup / destroy opcodes.
KudeyarRegisterInfo::KudeyarRegisterInfo(KudeyarTargetMachine &tm,
                                       const TargetInstrInfo &tii)
  : KudeyarGenRegisterInfo(Kudeyar::PSW), TM(tm), TII(tii) 
{
  StackAlign = TM.getFrameLowering()->getStackAlignment();
}

const uint16_t*
KudeyarRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const 
{
  const TargetFrameLowering *TFI = MF->getTarget().getFrameLowering();
  const Function* F = MF->getFunction();

  if (TFI->hasFP(*MF))
    return (F->getCallingConv() == CallingConv::Kudeyar_INTR ?
            CSR_FP_INTR_SaveList : CSR_FP_SaveList);
  else
    return (F->getCallingConv() == CallingConv::Kudeyar_INTR ?
            CSR_NoFP_INTR_SaveList : CSR_NoFP_SaveList);
}

BitVector KudeyarRegisterInfo::getReservedRegs(const MachineFunction &MF) const 
{
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  // Mark 4 special registers with subregisters as reserved.
  Reserved.set(Kudeyar::SP);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF))
    Reserved.set(Kudeyar::FP);

  return Reserved;
}

const TargetRegisterClass *
KudeyarRegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind)
                                                                         const 
{
  return &Kudeyar::GPRRegClass;
}

void KudeyarRegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const 
{
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  if (!TFI->hasReservedCallFrame(MF)) 
  {
    // If the stack pointer can be changed after prologue, turn the
    // adjcallstackup instruction into a 'sub SPW, <amt>' and the
    // adjcallstackdown instruction into 'add SPW, <amt>'
    // TODO: consider using push / pop instead of sub + store / add
    MachineInstr *Old = I;
    uint64_t Amount = Old->getOperand(0).getImm();
    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      Amount = (Amount+StackAlign-1)/StackAlign*StackAlign;

      MachineInstr *New = 0;
      if (Old->getOpcode() == TII.getCallFrameSetupOpcode()) 
      {
        New = BuildMI(MF, Old->getDebugLoc(),
                      TII.get(Kudeyar::SUB_RR), Kudeyar::SP)
          .addReg(Kudeyar::SP).addImm(Amount);
      } 
      else 
      {
        assert(Old->getOpcode() == TII.getCallFrameDestroyOpcode());
        // factor out the amount the callee already popped.
        uint64_t CalleeAmt = Old->getOperand(1).getImm();
        Amount -= CalleeAmt;
        if (Amount)
          New = BuildMI(MF, Old->getDebugLoc(),
                        TII.get(Kudeyar::ADD_RR), Kudeyar::SP)
            .addReg(Kudeyar::SP).addImm(Amount);
      }

      if (New) 
      {
        // The SR implicit def is dead.
        New->getOperand(3).setIsDead();

        // Replace the pseudo instruction with a new instruction...
        MBB.insert(I, New);
      }
    }
  } 
  else if (I->getOpcode() == TII.getCallFrameDestroyOpcode()) {
    // If we are performing frame pointer elimination and if the callee pops
    // something off the stack pointer, add it back.
    if (uint64_t CalleeAmt = I->getOperand(1).getImm()) 
    {
      MachineInstr *Old = I;
      MachineInstr *New =
        BuildMI(MF, Old->getDebugLoc(), TII.get(Kudeyar::SUB_RR),
                Kudeyar::SP).addReg(Kudeyar::SP).addImm(CalleeAmt);
      // The SRW implicit def is dead.
      New->getOperand(3).setIsDead();

      MBB.insert(I, New);
    }
  }

  MBB.erase(I);
}

void
KudeyarRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, RegScavenger *RS) const 
{
  assert(SPAdj == 0 && "Unexpected");

  unsigned i = 0;
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  DebugLoc dl = MI.getDebugLoc();
  while (!MI.getOperand(i).isFI()) 
  {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  int FrameIndex = MI.getOperand(i).getIndex();

  unsigned BasePtr = (TFI->hasFP(MF) ? Kudeyar::FP : Kudeyar::SP);
  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

  // Skip the saved PC
  Offset += 4;

  if (!TFI->hasFP(MF))
    Offset += MF.getFrameInfo()->getStackSize();
  else
    Offset += 4; // Skip the saved FP

  // Fold imm into offset
  Offset += MI.getOperand(i+1).getImm();

  if (MI.getOpcode() == Kudeyar::ADD_RR) 
  {
    // This is actually "load effective address" of the stack slot
    // instruction. We have only two-address instructions, thus we need to
    // expand it into mov + add

    MI.setDesc(TII.get(Kudeyar::MOV_RR));
    MI.getOperand(i).ChangeToRegister(BasePtr, false);

    if (Offset == 0)
      return;

    // We need to materialize the offset via add instruction.
    unsigned DstReg = MI.getOperand(0).getReg();
    if (Offset < 0)
      BuildMI(MBB, llvm::next(II), dl, TII.get(Kudeyar::SUB_RR), DstReg)
        .addReg(DstReg).addImm(-Offset);
    else
      BuildMI(MBB, llvm::next(II), dl, TII.get(Kudeyar::ADD_RR), DstReg)
        .addReg(DstReg).addImm(Offset);

    return;
  }

  MI.getOperand(i).ChangeToRegister(BasePtr, false);
  MI.getOperand(i+1).ChangeToImmediate(Offset);
}

unsigned KudeyarRegisterInfo::getFrameRegister(const MachineFunction &MF) const 
{
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return TFI->hasFP(MF) ? Kudeyar::FP : Kudeyar::SP;
}
