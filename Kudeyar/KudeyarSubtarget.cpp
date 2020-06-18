//===-------- KudeyarSubtarget.cpp ---------===//
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
#include "KudeyarSubtarget.h"
#include "Kudeyar.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "KudeyarGenSubtargetInfo.inc"

using namespace llvm;

void KudeyarSubtarget::anchor() { }

KudeyarSubtarget::KudeyarSubtarget(const std::string &TT,
                                 const std::string &CPU,
                                 const std::string &FS) :
  KudeyarGenSubtargetInfo(TT, CPU, FS) 
{
  std::string CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);
}
