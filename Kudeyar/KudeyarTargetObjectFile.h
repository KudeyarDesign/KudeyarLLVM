//===-------- KudeyarTargetObjectFile.h ---------===//
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
#ifndef LLVM_TARGET_KUDEYAR_TARGETOBJECTFILE_H
#define LLVM_TARGET_KUDEYAR_TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm 
{

  class KudeyarTargetObjectFile : public TargetLoweringObjectFileELF 
  {
    const MCSection *SmallDataSection;
    const MCSection *SmallBSSSection;
  public:

    void Initialize(MCContext &Ctx, const TargetMachine &TM);


    /// IsGlobalInSmallSection - Return true if this global address should be
    /// placed into small data/bss section.
    bool IsGlobalInSmallSection(const GlobalValue *GV,
                                const TargetMachine &TM, SectionKind Kind)const;
    bool IsGlobalInSmallSection(const GlobalValue *GV,
                                const TargetMachine &TM) const;

    const MCSection *SelectSectionForGlobal(const GlobalValue *GV,
                                            SectionKind Kind,
                                            Mangler *Mang,
                                            const TargetMachine &TM) const;

    // TODO: Classify globals as cpu0 wishes.
  };
} // end namespace llvm

#endif /* #define LLVM_TARGET_KUDEYAR_TARGETOBJECTFILE_H */
