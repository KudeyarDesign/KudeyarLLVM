//===-------- KudeyarCallingConv.td ---------===//
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
def CC_Kudeyar : CallingConv<[

  // Handles byval parameters.
  CCIfByVal<CCPassByVal<4, 4>>,
    
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,
  CCIfType<[f32], CCBitConvertToType<i32>>,

  // i64/f64 is passed in even pairs of GPRs
  // i64 is 8-aligned i32 here, so we may need to eat R1 as a pad register
  // (and the same is true for f64 if VFP is not enabled)
//  CCIfType<[i32], CCIfAlign<"8", CCAssignToRegWithShadow<[R0, R2], [R0, R1]>>>,
//  CCIfType<[i32], CCIf<"State.getNextStackOffset() == 0 &&"
//                       "ArgFlags.getOrigAlign() != 8",
//                       CCAssignToReg<[R0, R1, R2, R3]>>>,

  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,

//  CCIfType<[i32], CCIfAlign<"8", CCAssignToStackWithShadow<4, 8, R3>>>,
  CCIfType<[i32, f32], CCAssignToStack<4, 4>>,
  CCIfType<[f64], CCAssignToStack<8, 8>>
]>;

def RetCC_Kudeyar : CallingConv<[
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,
  CCIfType<[f32], CCBitConvertToType<i32>>,

  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[i64], CCAssignToRegWithShadow<[R0, R2], [R1, R3]>>
]>;

//===----------------------------------------------------------------------===//
// Callee-saved register lists.
//===----------------------------------------------------------------------===//
def CSR_NoFP : CalleeSavedRegs<(add R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, FP)>;

def CSR_FP : CalleeSavedRegs<(add R4, R5, R6, R7, R8, R9, R10, R11, R12, R13)>;

def CSR_NoFP_INTR : CalleeSavedRegs<(add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, FP )>;

def CSR_FP_INTR : CalleeSavedRegs<(add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13 )>;
