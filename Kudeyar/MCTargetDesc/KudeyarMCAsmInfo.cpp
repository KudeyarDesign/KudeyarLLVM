//===-------- KudeyarMCAsmInfo.cpp ---------===//
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
#include "KudeyarMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

void KudeyarMCAsmInfo::anchor() { }

KudeyarMCAsmInfo::KudeyarMCAsmInfo(const Target &T, StringRef TT) 
{
  PointerSize = 4;

  PrivateGlobalPrefix = ".L";
  WeakRefDirective = "\t.weak\t";
  PCSymbol= ".";
  CommentString = ";";

  AlignmentIsInBytes = false;
  AllowNameToStartWithDigit = false;
  UsesELFSectionDirectiveForBSS = true;
}
