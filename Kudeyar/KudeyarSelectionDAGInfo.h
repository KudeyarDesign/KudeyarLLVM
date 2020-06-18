//===-------- KudeyarSelectionDAGInfo.h ---------===//
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
#ifndef KUDEYARSELECTIONDAGINFO_H
#define KUDEYARSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm 
{

class KudeyarTargetMachine;

class KudeyarSelectionDAGInfo : public TargetSelectionDAGInfo 
{
public:
  explicit KudeyarSelectionDAGInfo(const KudeyarTargetMachine &TM);
  ~KudeyarSelectionDAGInfo();
};

}

#endif /* KUDEYARSELECTIONDAGINFO_H */
