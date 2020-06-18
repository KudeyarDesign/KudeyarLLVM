//===-------- KudeyarInstrInfo.cpp ---------===//
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
#include "KudeyarInstrInfo.h"
#include "Kudeyar.h"
#include "KudeyarMachineFunctionInfo.h"
#include "KudeyarTargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR
#include "KudeyarGenInstrInfo.inc"

using namespace llvm;

KudeyarInstrInfo::KudeyarInstrInfo(KudeyarTargetMachine &tm)
  : KudeyarGenInstrInfo(Kudeyar::ADJCALLSTACKDOWN, Kudeyar::ADJCALLSTACKUP),
    TM(tm),
    RI(TM, *this) 
{}

const KudeyarRegisterInfo &KudeyarInstrInfo::getRegisterInfo() const 
{
  return RI;
}

void KudeyarInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I, DebugLoc DL,
                                  unsigned DestReg, unsigned SrcReg,
                                  bool KillSrc) const 
{
  unsigned Opc;
  Opc = Kudeyar::MOV_RR;

  BuildMI(MBB, I, DL, get(Opc), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}

void KudeyarInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                          unsigned SrcReg, bool isKill, int FrameIdx,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const 
{
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();

  MachineMemOperand *MMO =
    MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FrameIdx),
                            MachineMemOperand::MOStore,
                            MFI.getObjectSize(FrameIdx),
                            MFI.getObjectAlignment(FrameIdx));

  BuildMI(MBB, MI, DL, get(Kudeyar::MOV_MR32))
      .addFrameIndex(FrameIdx).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
}

void KudeyarInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                           unsigned DestReg, int FrameIdx,
                                           const TargetRegisterClass *RC,
                                           const TargetRegisterInfo *TRI) const
{
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();

  MachineMemOperand *MMO =
    MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FrameIdx),
                            MachineMemOperand::MOLoad,
                            MFI.getObjectSize(FrameIdx),
                            MFI.getObjectAlignment(FrameIdx));

  BuildMI(MBB, MI, DL, get(Kudeyar::MOV_RM32))
      .addReg(DestReg).addFrameIndex(FrameIdx).addImm(0).addMemOperand(MMO);
}

MachineInstr*
KudeyarInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF, int FrameIx,
                                        uint64_t Offset, const MDNode *MDPtr,
                                        DebugLoc DL) const 
{
  MachineInstrBuilder MIB = BuildMI(MF, DL, get(Kudeyar::DBG_VALUE))
    .addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
  return &*MIB;
}

bool KudeyarInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const 
{
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) 
  {
    --I;
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(I))
      break;

    // A terminator that isn't a branch can't easily be handled
    // by this analysis.
    if (!I->isBranch())
      return true;

    // Cannot handle indirect branches.
    if (I->getOpcode() == Kudeyar::JMP_R ||
        I->getOpcode() == Kudeyar::JMP_M)
      return true;

    // Handle unconditional branches.
    if (I->getOpcode() == Kudeyar::Bs) 
	{
      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a JMP, delete them.
      while (llvm::next(I) != MBB.end())
        llvm::next(I)->eraseFromParent();
      Cond.clear();
      FBB = 0;

      // Delete the JMP if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) 
	  {
        TBB = 0;
        I->eraseFromParent();
        I = MBB.end();
        continue;
      }

      // TBB is used to indicate the unconditinal destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }


    // Handle conditional branches.
    assert((I->getOpcode() == Kudeyar::BTs  ||  I->getOpcode() == Kudeyar::BFs) && "Invalid conditional branch");

	return true;
  }

  return false;
}

unsigned KudeyarInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const 
{
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) 
  {
    --I;
    if (I->isDebugValue())
      continue;
    if (I->getOpcode() != Kudeyar::BTs &&
        I->getOpcode() != Kudeyar::BFs &&
        I->getOpcode() != Kudeyar::Bs &&
        I->getOpcode() != Kudeyar::JMP_R &&
		I->getOpcode() != Kudeyar::JMP_M)
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

unsigned KudeyarInstrInfo::InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                              MachineBasicBlock *FBB,
                              const SmallVectorImpl<MachineOperand> &Cond,
                              DebugLoc DL) const 
{
  return 0;
/*
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 1 || Cond.size() == 0) &&
         "Kudeyar branch conditions have one component!");

  if (Cond.empty()) 
  {
    // Unconditional branch?
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(Kudeyar::JMP)).addMBB(TBB);
    return 1;
  }

  // Conditional branch.
  unsigned Count = 0;
  BuildMI(&MBB, DL, get( Kudeyar::JCC)).addMBB(TBB).addImm(Cond[0].getImm());
  ++Count;

  if (FBB) 
  {
    // Two-way Conditional branch. Insert the second branch.
    BuildMI(&MBB, DL, get( Kudeyar::JMP)).addMBB(FBB);
    ++Count;
  }
  return Count;
*/
}
