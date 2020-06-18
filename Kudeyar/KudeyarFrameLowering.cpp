//===-------- KudeyarFrameLowering.cpp ---------===//
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
#include "KudeyarFrameLowering.h"
#include "KudeyarInstrInfo.h"
#include "KudeyarMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/DataLayout.h"
#include "llvm/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

bool KudeyarFrameLowering::hasFP(const MachineFunction &MF) const 
{
  const MachineFrameInfo *MFI = MF.getFrameInfo();

  return (MF.getTarget().Options.DisableFramePointerElim(MF) ||
          MF.getFrameInfo()->hasVarSizedObjects() ||
          MFI->isFrameAddressTaken());
}

bool KudeyarFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const 
{
  return !MF.getFrameInfo()->hasVarSizedObjects();
}

void KudeyarFrameLowering::emitPrologue(MachineFunction &MF) const 
{
  MachineBasicBlock &MBB = MF.front();   // Prolog goes in entry BB
  MachineFrameInfo *MFI = MF.getFrameInfo();
  KudeyarMachineFunctionInfo *KudeyarFI = MF.getInfo<KudeyarMachineFunctionInfo>();
  const KudeyarInstrInfo &TII =
    *static_cast<const KudeyarInstrInfo*>(MF.getTarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Get the number of bytes to allocate from the FrameInfo.
  uint64_t StackSize = MFI->getStackSize();

  uint64_t NumBytes = 0;
  if (hasFP(MF)) 
  {
    // Calculate required stack adjustment
    uint64_t FrameSize = StackSize - 4;
    NumBytes = FrameSize - KudeyarFI->getCalleeSavedFrameSize();

    // Get the offset of the stack slot for the EBP register... which is
    // guaranteed to be the last slot by processFunctionBeforeFrameFinalized.
    // Update the frame offset adjustment.
    MFI->setOffsetAdjustment(-NumBytes);

    // Save FPW into the appropriate stack slot...
    BuildMI(MBB, MBBI, DL, TII.get(Kudeyar::PUSH_R))
      .addReg(Kudeyar::FP, RegState::Kill);

    // Update FPW with the new base value...
    BuildMI(MBB, MBBI, DL, TII.get(Kudeyar::MOV_RR), Kudeyar::FP)
      .addReg(Kudeyar::SP);

    // Mark the FramePtr as live-in in every block except the entry.
    for (MachineFunction::iterator I = llvm::next(MF.begin()), E = MF.end();
         I != E; ++I)
      I->addLiveIn(Kudeyar::FP);

  } else
    NumBytes = StackSize - KudeyarFI->getCalleeSavedFrameSize();

  // Skip the callee-saved push instructions.
  while (MBBI != MBB.end() && (MBBI->getOpcode() == Kudeyar::PUSH_R))
    ++MBBI;

  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  if (NumBytes) 
  { // adjust stack pointer: SP -= numbytes
    // If there is an SUB_RI of SP immediately before this instruction, merge
    // the two.
    //NumBytes -= mergeSPUpdates(MBB, MBBI, true);
    // If there is an ADD_RI or SUB_RI of SP immediately after this
    // instruction, merge the two instructions.
    // mergeSPUpdatesDown(MBB, MBBI, &NumBytes);

    if (NumBytes) 
    {
        MachineInstr *MI =
        BuildMI(MBB, MBBI, DL, TII.get(Kudeyar::SUB_RI32), Kudeyar::SP)
        .addReg(Kudeyar::SP).addImm(NumBytes);
     }
  }
}

void KudeyarFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const 
{
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  KudeyarMachineFunctionInfo *KudeyarFI = MF.getInfo<KudeyarMachineFunctionInfo>();
  const KudeyarInstrInfo &TII =
    *static_cast<const KudeyarInstrInfo*>(MF.getTarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  unsigned RetOpcode = MBBI->getOpcode();
  DebugLoc DL = MBBI->getDebugLoc();

  switch (RetOpcode) 
  {
  case Kudeyar::RET:
  case Kudeyar::RETI: break;  // These are ok
  default:
    llvm_unreachable("Can only insert epilog into returning blocks");
  }

  // Get the number of bytes to allocate from the FrameInfo
  uint64_t StackSize = MFI->getStackSize();
  unsigned CSSize = KudeyarFI->getCalleeSavedFrameSize();
  uint64_t NumBytes = 0;

  if (hasFP(MF)) 
  {
    // Calculate required stack adjustment
    uint64_t FrameSize = StackSize - 4;
    NumBytes = FrameSize - CSSize;

    // pop FPW.
    BuildMI(MBB, MBBI, DL, TII.get(Kudeyar::POP_R), Kudeyar::FP);
  } 
  else  NumBytes = StackSize - CSSize;

  // Skip the callee-saved pop instructions.
  while (MBBI != MBB.begin()) 
  {
    MachineBasicBlock::iterator PI = prior(MBBI);
    unsigned Opc = PI->getOpcode();
    if (Opc != Kudeyar::POP_R && !PI->isTerminator())
      break;
    --MBBI;
  }

  DL = MBBI->getDebugLoc();

  // If there is an ADD16ri or SUB16ri of SPW immediately before this
  // instruction, merge the two instructions.
  //if (NumBytes || MFI->hasVarSizedObjects())
  //  mergeSPUpdatesUp(MBB, MBBI, StackPtr, &NumBytes);

  if (MFI->hasVarSizedObjects()) 
  {
    BuildMI(MBB, MBBI, DL,
            TII.get(Kudeyar::MOV_RR), Kudeyar::SP).addReg(Kudeyar::FP);
    if (CSSize) 
    {
       MachineInstr *MI =
        BuildMI(MBB, MBBI, DL,
                TII.get(Kudeyar::SUB_RI32), Kudeyar::SP)
        .addReg(Kudeyar::SP).addImm(CSSize);
     }
  } 
  else 
  {
    // adjust stack pointer back: SP += numbytes
    if (NumBytes) 
    {
      MachineInstr *MI =
        BuildMI(MBB, MBBI, DL, TII.get(Kudeyar::ADD_RI32), Kudeyar::SP)
        .addReg(Kudeyar::SP).addImm(NumBytes);
    }
  }
}

#define PUSH_POP_LIST

// FIXME: Can we eleminate these in favour of generic code?
bool
KudeyarFrameLowering::spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                        const std::vector<CalleeSavedInfo> &CSI,
                                        const TargetRegisterInfo *TRI) const 
{
  if (CSI.empty())
    return false;

  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();
  KudeyarMachineFunctionInfo *MFI = MF.getInfo<KudeyarMachineFunctionInfo>();
  MFI->setCalleeSavedFrameSize(CSI.size() * 4);

#ifdef PUSH_POP_LIST
  if (CSI.size() > 1)
  {
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, TII.get(Kudeyar::PUSHA));
    for (unsigned i = CSI.size(); i != 0; --i) 
    {
      unsigned Reg = CSI[i-1].getReg();
      bool isKill = true;

      if (isKill)
        MBB.addLiveIn(Reg);

      MIB.addReg(Reg, getKillRegState(isKill));
    }
    MIB.setMIFlags(MachineInstr::FrameSetup);
  }
  else
  {
    unsigned Reg = CSI[0].getReg();
    // Add the callee-saved register as live-in. It's killed at the spill.
    MBB.addLiveIn(Reg);
    BuildMI(MBB, MI, DL, TII.get(Kudeyar::PUSH_R))
      .addReg(Reg, RegState::Kill);
  }

#else
  for (unsigned i = CSI.size(); i != 0; --i) 
  {
    unsigned Reg = CSI[i-1].getReg();
    // Add the callee-saved register as live-in. It's killed at the spill.
    MBB.addLiveIn(Reg);
    BuildMI(MBB, MI, DL, TII.get(Kudeyar::PUSH_R))
      .addReg(Reg, RegState::Kill);
  }
#endif // PUSH_POP_LIST
  return true;
}

bool
KudeyarFrameLowering::restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                                                 MachineBasicBlock::iterator MI,
                                        const std::vector<CalleeSavedInfo> &CSI,
                                        const TargetRegisterInfo *TRI) const 
{
  if (CSI.empty())
    return false;

  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();

#ifdef PUSH_POP_LIST
  if (CSI.size() > 1)
  {
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, TII.get(Kudeyar::POPA));
    for (unsigned i = CSI.size(); i != 0; --i) 
	{
      unsigned Reg = CSI[i-1].getReg();
      MIB.addReg(Reg, getDefRegState(true));
	}
  }
  else
  {
    unsigned Reg = CSI[0].getReg();
	BuildMI(MBB, MI, DL, TII.get(Kudeyar::POP_R), Reg);
  }
#else
  for (unsigned i = 0, e = CSI.size(); i != e; ++i)
    BuildMI(MBB, MI, DL, TII.get(Kudeyar::POP_R), CSI[i].getReg());
#endif // PUSH_POP_LIST

  return true;
}

void
KudeyarFrameLowering::processFunctionBeforeFrameFinalized(MachineFunction &MF)
                                                                         const 
{
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  // Create a frame entry for the FP register that must be saved.
  if (TFI->hasFP(MF)) 
  {
    int FrameIdx = MF.getFrameInfo()->CreateFixedObject(4, -4, true);
    (void)FrameIdx;
    assert(FrameIdx == MF.getFrameInfo()->getObjectIndexBegin() &&
           "Slot for FP register must be last in order to be found!");
  }
}
