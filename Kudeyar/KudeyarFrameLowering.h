//===-------- KudeyarFrameLowering.h ---------===//
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
#ifndef KUDEYAR_FRAMEINFO_H
#define KUDEYAR_FRAMEINFO_H

#include "Kudeyar.h"
#include "KudeyarSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm 
{
  class KudeyarSubtarget;

class KudeyarFrameLowering : public TargetFrameLowering 
{
protected:
  const KudeyarSubtarget &STI;

public:
  explicit KudeyarFrameLowering(const KudeyarSubtarget &sti)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 4, -4), STI(sti) 
  {
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                        const std::vector<CalleeSavedInfo> &CSI,
                                        const TargetRegisterInfo *TRI) const; 
  bool restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MI,
                                   const std::vector<CalleeSavedInfo> &CSI,
                                   const TargetRegisterInfo *TRI) const;

  bool hasFP(const MachineFunction &MF) const;
  bool hasReservedCallFrame(const MachineFunction &MF) const;
  void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;
};

} // End llvm namespace

#endif /* KUDEYAR_FRAMEINFO_H */
