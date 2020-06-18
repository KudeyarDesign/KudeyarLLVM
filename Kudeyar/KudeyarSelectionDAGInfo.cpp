//===-------- KudeyarSelectionDAGInfo.cpp ---------===//
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
#define DEBUG_TYPE "kudeyar-selectiondag-info"
#include "KudeyarTargetMachine.h"
using namespace llvm;

KudeyarSelectionDAGInfo::KudeyarSelectionDAGInfo(const KudeyarTargetMachine &TM)
  : TargetSelectionDAGInfo(TM) 
{
}

KudeyarSelectionDAGInfo::~KudeyarSelectionDAGInfo() 
{
}
