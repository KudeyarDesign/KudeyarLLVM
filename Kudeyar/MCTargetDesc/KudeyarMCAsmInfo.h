//===-------- KudeyarMCAsmInfo.h ---------===//
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
#ifndef KUDEYARTARGETASMINFO_H
#define KUDEYARTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm 
{
  class StringRef;
  class Target;

  class KudeyarMCAsmInfo : public MCAsmInfo 
  {
    virtual void anchor();
  public:
    explicit KudeyarMCAsmInfo(const Target &T, StringRef TT);
  };

} // namespace llvm

#endif /* KUDEYARTARGETASMINFO_H */
