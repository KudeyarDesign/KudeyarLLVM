//===-------- KudeyarTargetInfo.cpp ---------===//
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
#include "Kudeyar.h"
#include "llvm/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheKudeyarTarget;

extern "C" void LLVMInitializeKudeyarTargetInfo() 
{
  RegisterTarget<Triple::kudeyar> 
    X(TheKudeyarTarget, "kudeyar", "Kudeyar [experimental]");
}
