//===-------- KudeyarMCTargetDesc.h ---------===//
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
#ifndef KUDEYARMCTARGETDESC_H
#define KUDEYARMCTARGETDESC_H

namespace llvm 
{
class Target;

extern Target TheKudeyarTarget;

} // End llvm namespace

// Defines symbolic names for Kudeyar registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "KudeyarGenRegisterInfo.inc"

// Defines symbolic names for the Kudeyar instructions.
#define GET_INSTRINFO_ENUM
#include "KudeyarGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "KudeyarGenSubtargetInfo.inc"

#endif /* KUDEYARMCTARGETDESC_H */
