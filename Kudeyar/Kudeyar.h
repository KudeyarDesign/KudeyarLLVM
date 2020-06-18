//===-------- Kudeyar.h ---------===//
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
#ifndef LLVM_TARGET_KUDEYAR_H
#define LLVM_TARGET_KUDEYAR_H

#include "MCTargetDesc/KudeyarMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm 
{
  class KudeyarTargetMachine;
  class FunctionPass;
  class formatted_raw_ostream;

  FunctionPass *createKudeyarISelDag(KudeyarTargetMachine &TM,
                                    CodeGenOpt::Level OptLevel);

//  FunctionPass *createKudeyarBranchSelectionPass();


} // end namespace llvm;

#endif /* LLVM_TARGET_KUDEYAR_H */

