//===-------- KudeyarRegisterInfo.td ---------===//
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
include "Target.td"

//===----------------------------------------------------------------------===//
//  Declarations that describe the Kudeyar register file
//===----------------------------------------------------------------------===//

class KudeyarReg<bits<4> num, string n> : Register<n> 
{
  field bits<4> Num = num;
  let Namespace = "Kudeyar";
}

class KudeyarRegWithSubregs<bits<4> num, string n, list<Register> subregs> 
  : RegisterWithSubRegs<n, subregs> 
{
  field bits<4> Num = num;
  let Namespace = "Kudeyar";
}
 
// Integer registers
def R0  : KudeyarReg< 0, "r0">,  DwarfRegNum<[0]>;
def R1  : KudeyarReg< 1, "r1">,  DwarfRegNum<[1]>;
def R2  : KudeyarReg< 2, "r2">,  DwarfRegNum<[2]>;
def R3  : KudeyarReg< 3, "r3">,  DwarfRegNum<[3]>;
def R4  : KudeyarReg< 4, "r4">,  DwarfRegNum<[4]>;
def R5  : KudeyarReg< 5, "r5">,  DwarfRegNum<[5]>;
def R6  : KudeyarReg< 6, "r6">,  DwarfRegNum<[6]>;
def R7  : KudeyarReg< 7, "r7">,  DwarfRegNum<[7]>;
def R8  : KudeyarReg< 8, "r8">,  DwarfRegNum<[8]>;
def R9  : KudeyarReg< 9, "r9">,  DwarfRegNum<[9]>;
def R10 : KudeyarReg<10, "r10">, DwarfRegNum<[10]>;
def R11 : KudeyarReg<11, "r11">, DwarfRegNum<[11]>;
def R12 : KudeyarReg<12, "r12">, DwarfRegNum<[12]>;
def R13 : KudeyarReg<13, "r13">, DwarfRegNum<[13]>;
def FP  : KudeyarReg<14, "fp">,  DwarfRegNum<[14]>;
def SP  : KudeyarReg<15, "sp">,  DwarfRegNum<[15]>;

// Current Program Status Register. 
def PSW  : KudeyarReg<0, "psw">;

// Register classes.
def GPR : RegisterClass<"Kudeyar", [i32], 32, (add 
  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, FP, SP)>;

// Condition code registers.
def CCR : RegisterClass<"Kudeyar", [i32], 32, (add PSW)> 
{
  let CopyCost = -1;  // Don't allow copying of status registers.
  let isAllocatable = 0;
}
