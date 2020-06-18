//===-------- KudeyarInstrInfo.td ---------===//
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
include "KudeyarInstrFormats.td"

//def SDTBrCC : SDTypeProfile<0, 4, [    // brcc
//  SDTCisVT<0, OtherVT>, SDTCisSameAs<1, 2>, SDTCisVT<3, OtherVT>
//  ]>;

//def SDTBrCC : SDTypeProfile<0, 4, [    // brcc
//  SDTCisInt<0>, SDTCisSameAs<1, 2>, SDTCisVT<3, OtherVT>
//  ]>;



//def brcc : SDNode<"ISD::BR_CC"      , SDTBrCC,   [SDNPHasChain]>;

def SDT_KudeyarWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                                  SDTCisPtrTy<0>]>;
def SDT_KudeyarCmp          : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;
def SDT_KudeyarBrCC         : SDTypeProfile<0, 1, [SDTCisVT<0, OtherVT>]>;
def SDT_KudeyarSelectCC     : SDTypeProfile<1, 2, [SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>]>;
def SDT_KudeyarCall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_KudeyarCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_KudeyarCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

//===----------------------------------------------------------------------===//
// Kudeyar Specific Node Definitions.
//===----------------------------------------------------------------------===//
def Kudeyar_retflag  : SDNode<"KudeyarISD::RET_FLAG", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue]>;
def Kudeyar_retiflag : SDNode<"KudeyarISD::RETI_FLAG", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue]>;

def Kudeyar_Wrapper : SDNode<"KudeyarISD::Wrapper", SDT_KudeyarWrapper>;
def Kudeyar_cmpeq     : SDNode<"KudeyarISD::CMPEQ", SDT_KudeyarCmp, [SDNPOutGlue]>;
def Kudeyar_cmpgt     : SDNode<"KudeyarISD::CMPGT", SDT_KudeyarCmp, [SDNPOutGlue]>;
def Kudeyar_cmplt     : SDNode<"KudeyarISD::CMPLT", SDT_KudeyarCmp, [SDNPOutGlue]>;
def Kudeyar_cmpgtu    : SDNode<"KudeyarISD::CMPGTU", SDT_KudeyarCmp, [SDNPOutGlue]>;
def Kudeyar_cmpltu    : SDNode<"KudeyarISD::CMPLTU", SDT_KudeyarCmp, [SDNPOutGlue]>;
def Kudeyar_brcc_t    : SDNode<"KudeyarISD::BR_CC_T", SDT_KudeyarBrCC,
                            [SDNPHasChain, SDNPInGlue]>;
def Kudeyar_brcc_f    : SDNode<"KudeyarISD::BR_CC_F", SDT_KudeyarBrCC,
                            [SDNPHasChain, SDNPInGlue]>;
def Kudeyar_selectcc_t: SDNode<"KudeyarISD::SELECT_CC_T", SDT_KudeyarSelectCC,
                            [SDNPInGlue]>;
def Kudeyar_selectcc_f: SDNode<"KudeyarISD::SELECT_CC_F", SDT_KudeyarSelectCC,
                            [SDNPInGlue]>;
def Kudeyar_call    : SDNode<"KudeyarISD::CALL", SDT_KudeyarCall,
                     [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def Kudeyar_callseq_start :
                 SDNode<"ISD::CALLSEQ_START", SDT_KudeyarCallSeqStart,
                        [SDNPHasChain, SDNPOutGlue]>;
def Kudeyar_callseq_end :
                 SDNode<"ISD::CALLSEQ_END",   SDT_KudeyarCallSeqEnd,
                        [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Immediate operands with a shared generic render method.
class ImmAsmOperand : AsmOperandClass { let RenderMethod = "addImmOperands"; }

// imm4 predicate - Immediate in the range [0,15].
def imm4_AsmOperand: ImmAsmOperand { let Name = "imm4"; }
def imm4 : Operand<i32>, ImmLeaf<i32, 
  [{
    return Imm >= 0 && Imm < 16;
  }]> 
{
  let ParserMatchClass = imm4_AsmOperand;
}

def mem : Operand<iPTR> 
{
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPR, i32imm);
  let EncoderMethod = "getMemEncoding";
}

def addr : ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;

def brtarget    : Operand<OtherVT> 
{
//  let EncoderMethod = "getBranchTargetOpValue";
//  let OperandType = "OPERAND_PCREL";
//  let DecoderMethod = "DecodeBranchTarget";
  let PrintMethod = "printPCRelImmOperand";
}

def calltarget  : Operand<iPTR> 
{
  let EncoderMethod = "getJumpTargetOpValue";
}

// A list of registers separated by comma. Used by push/pop multiple.
def RegListAsmOperand : AsmOperandClass { let Name = "RegList"; }
def reglist : Operand<i32> 
{
  let EncoderMethod = "getRegisterListOpValue";
  let ParserMatchClass = RegListAsmOperand;
  let PrintMethod = "printRegisterList";
  let DecoderMethod = "DecodeRegListOperand";
}

// Helper fragments for loads.
// It's always safe to treat a anyext i16 load as a i32 load if the i16 is
// known to be 32-bit aligned or better. Ditto for i8 to i16.
def loadi16 : PatFrag<(ops node:$ptr), (i16 (unindexedload node:$ptr)), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  if (ExtType == ISD::NON_EXTLOAD)
    return true;
  if (ExtType == ISD::EXTLOAD)
    return LD->getAlignment() >= 2 && !LD->isVolatile();
  return false;
}]>;

def loadi16_anyext : PatFrag<(ops node:$ptr), (i32 (unindexedload node:$ptr)),[{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  if (ExtType == ISD::EXTLOAD)
    return LD->getAlignment() >= 2 && !LD->isVolatile();
  return false;
}]>;

def loadi32 : PatFrag<(ops node:$ptr), (i32 (unindexedload node:$ptr)), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  if (ExtType == ISD::NON_EXTLOAD)
    return true;
  if (ExtType == ISD::EXTLOAD)
    return LD->getAlignment() >= 4 && !LD->isVolatile();
  return false;
}]>;

def loadi8  : PatFrag<(ops node:$ptr), (i8 (load node:$ptr))>;

def loadi8_anyext : PatFrag<(ops node:$ptr), (i32 (unindexedload node:$ptr))>;

class ArithLogic_RR<bits<6> oper, string instr_asm, SDNode OpNode, bit isComm = 0>:
  Inst_RR<(outs GPR:$Rd), (ins GPR:$Rs2, GPR:$Rs),
     !strconcat(instr_asm, "\t$Rd, $Rs"),
     [(set GPR:$Rd, (OpNode GPR:$Rs2, GPR:$Rs))]> 
{
  let op = oper;
  let isCommutable = isComm;
  let isReMaterializable = 0;
  let Constraints = "$Rd = $Rs2";
}

class ArithLogic_RI4<bits<6> oper, string instr_asm, SDNode OpNode, bit isComm = 0>:
  Inst_RI4<(outs GPR:$Rd), (ins GPR:$Rs, imm4:$Imm),
     !strconcat(instr_asm, "\t$Rd, $Imm"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, imm4:$Imm))]> 
{
  let op = oper;
  let isCommutable = isComm;
  let isReMaterializable = 0;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RI32<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RI32<(outs GPR:$Rd), (ins GPR:$Rs, i32imm:$Imm),
     !strconcat(instr_asm, "\t$Rd, $Imm"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, imm:$Imm))]> 
{
  let op = oper;
  let isReMaterializable = 0;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RM32<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RM<(outs GPR:$Rd), (ins GPR:$Rs, mem:$src),
     !strconcat(instr_asm, "\t$Rd, $src"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, (load addr:$src)))]> 
{
  let op = oper;
  let size = 0x2;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RM16<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RM<(outs GPR:$Rd), (ins GPR:$Rs, mem:$src),
     !strconcat(instr_asm, "w\t$Rd, $src"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, (zextloadi16 addr:$src)))]> 
{
  let op = oper;
  let size = 0x1;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RMSX16<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RM<(outs GPR:$Rd), (ins GPR:$Rs, mem:$src),
     !strconcat(instr_asm, "sxw\t$Rd, $src"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, (sextloadi16 addr:$src)))]> 
{
  let op = oper;
  let size = 0x1;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RM8<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RM<(outs GPR:$Rd), (ins GPR:$Rs, mem:$src),
     !strconcat(instr_asm, "b\t$Rd, $src"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, (loadi8_anyext addr:$src)))]> 
{
  let op = oper;
  let size = 0x0;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RMSX8<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RM<(outs GPR:$Rd), (ins GPR:$Rs, mem:$src),
     !strconcat(instr_asm, "sxb\t$Rd, $src"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, (sextloadi8 addr:$src)))]> 
{
  let op = oper;
  let size = 0x0;
  let Constraints = "$Rd = $Rs";
}

class ArithLogic_RM_POST<bits<6> oper, string instr_asm, SDNode OpNode, int sz>:
  Inst_RM<(outs GPR:$Rd, GPR:$Rb_wb), (ins GPR:$Rs, GPR:$Rb),
          !strconcat(instr_asm, "\t$Rd, [$Rb]+"), []> 
{
  let op = oper;
  let size = sz;
  let amode = 0x1;
  let Constraints = "$Rb = $Rb_wb";
}

class ArithLogic_RM32_POST<bits<6> oper, string instr_asm, SDNode OpNode>:
  ArithLogic_RM_POST<oper, instr_asm, OpNode, 0x2>;

class ArithLogic_RM16_POST<bits<6> oper, string instr_asm, SDNode OpNode>:
  ArithLogic_RM_POST<oper, !strconcat(instr_asm, "w"), OpNode, 0x1>;

class ArithLogic_RM8_POST<bits<6> oper, string instr_asm, SDNode OpNode>:
  ArithLogic_RM_POST<oper, !strconcat(instr_asm, "b"), OpNode, 0x0>;

class ArithLogic_MR32<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_MR<(outs), (ins mem:$dst, GPR:$Rs),
     !strconcat(instr_asm, "\t$dst, $Rs"),
     [(store (OpNode (load addr:$dst), GPR:$Rs), addr:$dst)]> 
{
  let op = oper;
  let size = 0x2;
}

class ArithLogic_MR16<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_MR<(outs), (ins mem:$dst, GPR:$Rs),
     !strconcat(instr_asm, "w\t$dst, $Rs"),
     [(truncstorei16 (OpNode (loadi16_anyext addr:$dst), GPR:$Rs), addr:$dst)]> 
{
  let op = oper;
  let size = 0x1;
}

class ArithLogic_MR8<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_MR<(outs), (ins mem:$dst, GPR:$Rs),
     !strconcat(instr_asm, "b\t$dst, $Rs"),
     [(truncstorei8 (OpNode (loadi8_anyext addr:$dst), GPR:$Rs), addr:$dst)]> 
{
  let op = oper;
  let size = 0x0;
}

class ArithLogic_MR_POST<bits<6> oper, string instr_asm, SDNode OpNode, int sz>:
  Inst_MR<(outs GPR:$Rb_wb), (ins GPR:$Rb, GPR:$Rs),
          !strconcat(instr_asm, "\t[$Rb]+, $Rs"), []> 
{
  let op = oper;
  let size = sz;
  let amode = 0x1;
  let Constraints = "$Rb = $Rb_wb";
}

class ArithLogic_MR32_POST<bits<6> oper, string instr_asm, SDNode OpNode>:
  ArithLogic_MR_POST<oper, instr_asm, OpNode, 0x2>;

class ArithLogic_MR16_POST<bits<6> oper, string instr_asm, SDNode OpNode>:
  ArithLogic_MR_POST<oper, !strconcat(instr_asm, "w"), OpNode, 0x1>;

class ArithLogic_MR8_POST<bits<6> oper, string instr_asm, SDNode OpNode>:
  ArithLogic_MR_POST<oper, !strconcat(instr_asm, "b"), OpNode, 0x0>;

class ArithLogic_MI4_32<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_MI4<(outs), (ins mem:$dst, imm4:$Imm),
     !strconcat(instr_asm, "\t$dst, $Imm"),
     [(store (OpNode (load addr:$dst), (i32 imm4:$Imm)), addr:$dst)]> 
{
  let op = oper;
  let size = 0x2;
}

class Shift_RI<bits<6> oper, string instr_asm, SDNode OpNode>:
  Inst_RR<(outs GPR:$Rd), (ins GPR:$Rs, i32imm:$Imm),
     !strconcat(instr_asm, "\t$Rd, $Imm"),
     [(set GPR:$Rd, (OpNode GPR:$Rs, imm:$Imm))]> 
{
  let op = oper;
  let Constraints = "$Rd = $Rs";
}

let Defs = [PSW], isCompare = 1 in 
{
  class Compare_RR<bits<6> oper, string instr_asm, SDNode OpNode, bit isComm = 0>:
    Inst_RR<(outs), (ins GPR:$Rd, GPR:$Rs),
       !strconcat(instr_asm, "\t$Rd, $Rs"),
       [(OpNode GPR:$Rd, GPR:$Rs) , (implicit PSW)]> 
  {
    let op = oper;
    let isCommutable = isComm;
    let isReMaterializable = 0;
  }

  class Compare_RI4<bits<6> oper, string instr_asm, SDNode OpNode>:
    Inst_RI4<(outs), (ins GPR:$Rd, imm4:$Imm),
       !strconcat(instr_asm, "\t$Rd, $Imm"),
       [(OpNode GPR:$Rd, imm4:$Imm) , (implicit PSW)]> 
  {
    let op = oper;
    let isReMaterializable = 0;
  }

  class Compare_RI32<bits<6> oper, string instr_asm, SDNode OpNode>:
    Inst_RI32<(outs), (ins GPR:$Rd, i32imm:$Imm),
       !strconcat(instr_asm, "\t$Rd, $Imm"),
       [(OpNode GPR:$Rd, imm:$Imm) , (implicit PSW)]> 
  {
    let op = oper;
    let isReMaterializable = 0;
  }

  class Compare_RM32<bits<6> oper, string instr_asm, SDNode OpNode>:
    Inst_RM<(outs), (ins GPR:$Rd, mem:$src),
       !strconcat(instr_asm, "\t$Rd, $src"),
       [(OpNode GPR:$Rd, (load addr:$src)), (implicit PSW)]> 
  {
    let op = oper;
    let size = 0x2;
  }  



  class Compare_MR32<bits<6> oper, string instr_asm, SDNode OpNode>:
    Inst_MR<(outs), (ins mem:$src, GPR:$Rs),
       !strconcat(instr_asm, "\t$src, $Rs"),
       [(OpNode (load addr:$src), GPR:$Rs), (implicit PSW)]> 
  {
    let op = oper;
    let size = 0x2;
  } 
}

// ADJCALLSTACKDOWN/UP implicitly use/def SP because they may be expanded into
// a stack adjustment and the codegen must know that they may modify the stack
// pointer before prolog-epilog rewriting occurs.
let Defs = [SP], Uses = [SP] in 
{
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt),
                              "#ADJCALLSTACKDOWN",
                              [(Kudeyar_callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              "#ADJCALLSTACKUP",
                              [(Kudeyar_callseq_end timm:$amt1, timm:$amt2)]>;
}

let usesCustomInserter = 1 in 
{
  def Select32T : Pseudo<(outs GPR:$dst), (ins GPR:$src, GPR:$src2),
                        "# Select32T PSEUDO",
                        [(set GPR:$dst,
                          (Kudeyar_selectcc_t GPR:$src, GPR:$src2))]>;

  def Select32F : Pseudo<(outs GPR:$dst), (ins GPR:$src, GPR:$src2),
                        "# Select32F PSEUDO",
                        [(set GPR:$dst,
                          (Kudeyar_selectcc_f GPR:$src, GPR:$src2))]>;
}

//let neverHasSideEffects = 1 in
//def NOP : Pseudo<(outs), (ins), "nop", []>;

let isBranch = 1, isTerminator = 1 in
{
  def BTs: Inst_Bs<(outs), (ins brtarget:$offset),
                  "bt\t$offset",
				  [(Kudeyar_brcc_t bb:$offset)]>
  {
    let op = 0x37;
	let Uses = [PSW];
    let neverHasSideEffects = 1;
  }				   

  def BFs: Inst_Bs<(outs), (ins brtarget:$offset),
                  "bf\t$offset",
				  [(Kudeyar_brcc_f bb:$offset)]>
  {
    let op = 0x38;
	let Uses = [PSW];
    let neverHasSideEffects = 1;
  }

  def Bs: Inst_Bs<(outs), (ins brtarget:$targ),
                  "b\t$targ",
				  [(br bb:$targ)]>
  {
    let isBarrier = 1;
    let op = 0x39;
  }	

  let isIndirectBranch = 1, op = 0x003 in 
  {
    def JMP_R: Inst_R<(outs), (ins GPR:$Rd),
                    "jmp\t$Rd",
                    [(brind GPR:$Rd)]>;

    def JMP_Mr: Inst_M<(outs), (ins GPR:$Rb),
                    "jmp\t[$Rb]",
                    [(brind (load GPR:$Rb))]>;

    def JMP_M: Inst_M<(outs), (ins mem:$brdst),
                    "jmp\t$brdst",
                    [(brind (load addr:$brdst))]>;
  }
}

let mayStore = 1, Uses = [SP], Defs = [SP], hasExtraSrcRegAllocReq = 1 in
{
  def PUSH_R:
    Inst_R< (outs), (ins GPR:$Rd),
          "push\t$Rd",
          []>
  {
    let op = 0x005; 
  }

  def PUSHA:
    Inst_Rlist<(outs), (ins reglist:$reglist, variable_ops),
	     "push\t$reglist",
		 []>
  {
    let op = 0xFF0;
  }
}

let mayLoad = 1, Uses = [SP], Defs = [SP], hasExtraDefRegAllocReq = 1 in
{
  def POP_R:
    Inst_R< (outs GPR:$Rd), (ins),
          "pop\t$Rd",
          []>
  {
    let op = 0x006; 
  }

  def POPA:
    Inst_Rlist<(outs), (ins reglist:$reglist, variable_ops),
	     "pop\t$reglist",
		 []>
  {
    let op = 0xFF1;
  }
}

let op = 0x20 in
{
  let dir = 0 in
  {
    let sext = 0 in
	{
      def MOV_RMA32:
        Inst_RMA<(outs GPR:$Rd), (ins i32imm:$address),
            "mov\t$Rd, [$address]",
	        [(set GPR:$Rd, (load (Kudeyar_Wrapper tglobaladdr:$address)))]>
      {
	    let size = 0x2;
      }

      def MOVZX_RMA16:
        Inst_RMA<(outs GPR:$Rd), (ins i32imm:$address),
            "movzxw\t$Rd, [$address]",
	        [(set GPR:$Rd, (zextloadi16 (Kudeyar_Wrapper tglobaladdr:$address)))]>
      {
	    let size = 0x1;	
      }

      def MOVZX_RMA8:
        Inst_RMA<(outs GPR:$Rd), (ins i32imm:$address),
            "movzxb\t$Rd, [$address]",
	        [(set GPR:$Rd, (zextloadi8 (Kudeyar_Wrapper tglobaladdr:$address)))]>
      {
	    let size = 0x0;	
      }

      def MOVEX_RMA8:
        Inst_RMA<(outs GPR:$Rd), (ins i32imm:$address),
            "movzxb\t$Rd, [$address]",
	        [(set GPR:$Rd, (extloadi8 (Kudeyar_Wrapper tglobaladdr:$address)))]>
      {
	    let size = 0x0;	
      }
	}

    let sext = 1 in
	{
      def MOVSX_RMA16:
        Inst_RMA<(outs GPR:$Rd), (ins i32imm:$address),
            "movsxw\t$Rd, [$address]",
	        [(set GPR:$Rd, (sextloadi16 (Kudeyar_Wrapper tglobaladdr:$address)))]>
      {
	    let size = 0x1;	
      }

      def MOVSX_RMA8:
        Inst_RMA<(outs GPR:$Rd), (ins i32imm:$address),
            "movsxb\t$Rd, [$address]",
	        [(set GPR:$Rd, (sextloadi8 (Kudeyar_Wrapper tglobaladdr:$address)))]>
      {
	    let size = 0x0;	
      }
	}
  }

  let dir = 1, sext = 0 in
  {
    def MOV_MAR32:
      Inst_RMA<(outs), (ins i32imm:$address, GPR:$Rd),
          "mov\t[$address], $Rd",
	      [(store GPR:$Rd, (Kudeyar_Wrapper tglobaladdr:$address))]>
    {
	  let size = 0x2;	
    }

    def MOV_MAR8:
      Inst_RMA<(outs), (ins i32imm:$address, GPR:$Rd),
          "movb\t[$address], $Rd",
	      [(truncstorei8 GPR:$Rd, (Kudeyar_Wrapper tglobaladdr:$address))]>
    {
	  let size = 0x0;	
    }

    def MOV_MAR16:
      Inst_RMA<(outs), (ins i32imm:$address, GPR:$Rd),
          "movw\t[$address], $Rd",
	      [(truncstorei16 GPR:$Rd, (Kudeyar_Wrapper tglobaladdr:$address))]>
    {
	  let size = 0x1;	
    }
  }
}

def MOV_RR:
  Inst_RR<(outs GPR:$Rd), (ins GPR:$Rs),
     "mov\t$Rd, $Rs",
     []>
{
  let op = 0x02;
  let Rs = 0;
  let neverHasSideEffects = 1; 
}

let isReMaterializable = 1, isAsCheapAsAMove = 1 in 
{
  def MOV_RI4:
    Inst_RI4<(outs GPR:$Rd), (ins imm4:$Imm),
       "mov\t$Rd, $Imm",
       [(set GPR:$Rd, imm4:$Imm)]>
  {
    let op = 0x03;
  }

  def MOV_RI32:
    Inst_RR<(outs GPR:$Rd), (ins i32imm:$Imm),
       "mov\t$Rd, $Imm",
       [(set GPR:$Rd, imm:$Imm)]>
  {
    let op = 0x03;
  }
}

let op = 0x02 in
{
  def MOV_RM32:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "mov\t$Rd, $src",
       [(set GPR:$Rd, (load addr:$src))]>
  {
    let size = 0x2;
  }

  def MOV_RMr32:
    Inst_RM<(outs GPR:$Rd), (ins GPR:$Rb),
       "mov\t$Rd, [$Rb]",
       [(set GPR:$Rd, (load GPR:$Rb))]>
  {
    let size = 0x2;
  }

  def MOVZX_RM16:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "movzxw\t$Rd, $src",
       [(set GPR:$Rd, (zextloadi16 addr:$src))]>
  {
    let size = 0x1;
  }

  def MOVZX_RMr16:
    Inst_RM<(outs GPR:$Rd), (ins GPR:$Rb),
       "movzxw\t$Rd, [$Rb]",
       [(set GPR:$Rd, (zextloadi16 GPR:$Rb))]>
  {
    let size = 0x1;
  }

  def MOVEX_RM16:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "movzxw\t$Rd, $src",
       [(set GPR:$Rd, (extloadi16 addr:$src))]>
  {
    let size = 0x1;
  }

  def MOVZX_RM8:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "movzxb\t$Rd, $src",
       [(set GPR:$Rd, (zextloadi8 addr:$src))]>
  {
    let size = 0x0;
  }

  def MOVZX_RMr8:
    Inst_RM<(outs GPR:$Rd), (ins GPR:$Rb),
       "movzxb\t$Rd, [$Rb]",
       [(set GPR:$Rd, (zextloadi8 GPR:$Rb))]>
  {
    let size = 0x0;
  }

  def MOVEX_RM8:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "movzxb\t$Rd, $src",
       [(set GPR:$Rd, (extloadi8 addr:$src))]>
  {
    let size = 0x0;
  }

  def MOVEX_RMr8:
    Inst_RM<(outs GPR:$Rd), (ins GPR:$Rb),
       "movzxb\t$Rd, [$Rb]",
       [(set GPR:$Rd, (extloadi8 GPR:$Rb))]>
  {
    let size = 0x0;
  }

  let mayLoad = 1, hasExtraDefRegAllocReq = 1, amode = 0x1, Constraints = "$Rb = $Rb_wb" in 
  {
    def MOV_RM32_POST: 
	  Inst_RM<(outs GPR:$Rd, GPR:$Rb_wb), (ins GPR:$Rb),
          "mov\t$Rd, [$Rb]+", []>
    {
	  let size = 0x2;
	}

    def MOVZX_RM16_POST: 
	  Inst_RM<(outs GPR:$Rd, GPR:$Rb_wb), (ins GPR:$Rb),
          "movzxw\t$Rd, [$Rb]+", []>
    {
	  let size = 0x1;
	}

    def MOVSX_RM16_POST: 
	  Inst_RM<(outs GPR:$Rd, GPR:$Rb_wb), (ins GPR:$Rb),
          "movsxw\t$Rd, [$Rb]+", []>
    {
	  let size = 0x1;
	}

    def MOVZX_RM8_POST: 
	  Inst_RM<(outs GPR:$Rd, GPR:$Rb_wb), (ins GPR:$Rb),
          "movzxb\t$Rd, [$Rb]+", []>
    {
	  let size = 0x0;
	}

    def MOVSX_RM8_POST: 
	  Inst_RM<(outs GPR:$Rd, GPR:$Rb_wb), (ins GPR:$Rb),
          "movsxb\t$Rd, [$Rb]+", []>
    {
	  let size = 0x0;
	}
  }
}

let op = 0x3B in
{
  def MOVSX_RM16:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "movsxw\t$Rd, $src",
       [(set GPR:$Rd, (sextloadi16 addr:$src))]>
  {
    let size = 0x1;
  }

  def MOVSX_RMr16:
    Inst_RM<(outs GPR:$Rd), (ins GPR:$Rb),
       "movsxb\t$Rd, [$Rb]",
       [(set GPR:$Rd, (sextloadi16 GPR:$Rb))]>
  {
    let size = 0x1;
  }

  def MOVSX_RM8:
    Inst_RM<(outs GPR:$Rd), (ins mem:$src),
       "movsxb\t$Rd, $src",
       [(set GPR:$Rd, (sextloadi8 addr:$src))]>
  {
    let size = 0x0;
  }

  def MOVSX_RMr8:
    Inst_RM<(outs GPR:$Rd), (ins GPR:$Rb),
       "movsxb\t$Rd, [$Rb]",
       [(set GPR:$Rd, (sextloadi8 GPR:$Rb))]>
  {
    let size = 0x0;
  }
}

let op = 0x03 in
{
  def MOV_MI4_32:
      Inst_MI4<(outs), (ins mem:$dst, imm4:$Imm),
	  "mov\t$dst, $Imm",
	  [(store imm4:$Imm, addr:$dst)]>
  {
    let size = 0x2;
  }

  def MOV_MI4_16:
      Inst_MI4<(outs), (ins mem:$dst, imm4:$Imm),
	  "movw\t$dst, $Imm",
	  [(truncstorei16 imm4:$Imm, addr:$dst)]>
  {
    let size = 0x1;
  }

  def MOV_MI4_8:
      Inst_MI4<(outs), (ins mem:$dst, imm4:$Imm),
	  "movb\t$dst, $Imm",
	  [(truncstorei8 imm4:$Imm, addr:$dst)]>
  {
    let size = 0x0;
  }
}

let op = 0x02 in
{
  def MOV_MR32:
    Inst_MR<(outs), (ins mem:$dst, GPR:$Rs),
       "mov\t$dst, $Rs",
       [(store GPR:$Rs, addr:$dst)]>
  {
    let size = 0x2;
  }

  def MOV_MRr32:
    Inst_MR<(outs), (ins GPR:$Rb, GPR:$Rs),
       "mov\t[$Rb], $Rs",
       [(store GPR:$Rs, GPR:$Rb)]>
  {
    let size = 0x2;
  }

  def MOV_MR16:
    Inst_MR<(outs), (ins mem:$dst, GPR:$Rs),
       "movw\t$dst, $Rs",
       [(truncstorei16 GPR:$Rs, addr:$dst)]>
  {
    let size = 0x1;
  }

  def MOV_MRr16:
    Inst_MR<(outs), (ins GPR:$Rb, GPR:$Rs),
       "movw\t[$Rb], $Rs",
       [(truncstorei16 GPR:$Rs, GPR:$Rb)]>
  {
    let size = 0x1;
  }

  def MOV_MR8:
    Inst_MR<(outs), (ins mem:$dst, GPR:$Rs),
       "movb\t$dst, $Rs",
       [(truncstorei8 GPR:$Rs, addr:$dst)]>
  {
    let size = 0x0;
  }

  def MOV_MRr8:
    Inst_MR<(outs), (ins GPR:$Rb, GPR:$Rs),
       "movb\t[$Rb], $Rs",
       [(truncstorei8 GPR:$Rs, GPR:$Rb)]>
  {
    let size = 0x0;
  }

  let mayStore = 1, hasExtraDefRegAllocReq = 1, amode = 0x1, Constraints = "$Rb = $Rb_wb" in 
  {
    def MOV_MR32_POST: 
	  Inst_MR<(outs GPR:$Rb_wb), (ins GPR:$Rb, GPR:$Rs),
          "mov\t[$Rb]+, $Rs", []>
    {
	  let size = 0x2;
	}

    def MOV_MR16_POST: 
	  Inst_MR<(outs GPR:$Rb_wb), (ins GPR:$Rb, GPR:$Rs),
          "movw\t[$Rb]+, $Rs", []>
    {
	  let size = 0x1;
	}

    def MOV_MR8_POST: 
	  Inst_MR<(outs GPR:$Rb_wb), (ins GPR:$Rb, GPR:$Rs),
          "movb\t[$Rb]+, $Rs", []>
    {
	  let size = 0x0;
	}
  }
} 

let Constraints = "$Rd = $Rs1" in
{
  def NEG_R: Inst_R<(outs GPR:$Rd), (ins GPR:$Rs1),
          "neg\t$Rd",
		  [(set GPR:$Rd, (ineg GPR:$Rs1))]>
  {
    let op = 0x002;
  }

  def COM_R: Inst_R<(outs GPR:$Rd), (ins GPR:$Rs1),
          "com\t$Rd",
		  [(set GPR:$Rd, (not GPR:$Rs1))]>
  {
    let op = 0x001;
  }
}

let op = 0x001 in
{
  def COM_M32: Inst_M<(outs), (ins mem:$dst),
          "com\t$dst",  
		  [(store (not (loadi32 addr:$dst)), addr:$dst)]>
  {
    let size = 0x2;
  }

  def COM_M16: Inst_M<(outs), (ins mem:$dst),
         "comw\t$dst",  
		 [(truncstorei16 (not (loadi16_anyext addr:$dst)), addr:$dst)]>
  {
    let size = 0x1;
  }

  def COM_M8: Inst_M<(outs), (ins mem:$dst),
         "comb\t$dst",  
		 [(truncstorei8 (not (loadi8_anyext addr:$dst)), addr:$dst)]>
  {
    let size = 0x0;
  }
}

let op = 0x002 in
{
  def NEG_M32: Inst_M<(outs), (ins mem:$dst),
          "neg\t${dst:mem}",  
		  [(store (ineg (loadi32 addr:$dst)), addr:$dst)]>
  {
    let size = 0x2;
  }

  def NEG_M16: Inst_M<(outs), (ins mem:$dst),
         "negw\t$dst",  
		 [(truncstorei16 (ineg (loadi16_anyext addr:$dst)), addr:$dst)]>
  {
    let size = 0x1;
  }

  def NEG_M8: Inst_M<(outs), (ins mem:$dst),
         "negb\t$dst",  
		 [(truncstorei8 (ineg (loadi8_anyext addr:$dst)), addr:$dst)]>
  {
    let size = 0x0;
  }
}

def ADD_MR32 :  ArithLogic_MR32<0x04, "add", add>;
def SUB_MR32 :  ArithLogic_MR32<0x06, "sub", sub>; 
def AND_MR32 :  ArithLogic_MR32<0x08, "and", and>;
def OR_MR32  :  ArithLogic_MR32<0x0A, "or", or>;
def XOR_MR32 :  ArithLogic_MR32<0x0C, "xor", xor>;
def MUL_MR32 :  ArithLogic_MR32<0x0E, "mul", mul>;
def DIV_MR32 :  ArithLogic_MR32<0x12, "div", sdiv>;
def DIVU_MR32 : ArithLogic_MR32<0x14, "divu", udiv>;
def MOD_MR32 :  ArithLogic_MR32<0x16, "mod", srem>;
def MODU_MR32 : ArithLogic_MR32<0x18, "modu", urem>;
def SLL_MR32 :  ArithLogic_MR32<0x20, "sll", shl>;
def SRL_MR32 :  ArithLogic_MR32<0x24, "srl", srl>;
def SRA_MR32 :  ArithLogic_MR32<0x28, "sra", sra>;

def ADD_MR16 :  ArithLogic_MR16<0x04, "add", add>;
def SUB_MR16 :  ArithLogic_MR16<0x06, "sub", sub>; 
def AND_MR16 :  ArithLogic_MR16<0x08, "and", and>;
def OR_MR16  :  ArithLogic_MR16<0x0A, "or", or>;
def XOR_MR16 :  ArithLogic_MR16<0x0C, "xor", xor>;
def MUL_MR16 :  ArithLogic_MR16<0x0E, "mul", mul>;
def DIV_MR16 :  ArithLogic_MR16<0x12, "div", sdiv>;
def DIVU_MR16 : ArithLogic_MR16<0x14, "divu", udiv>;
def MOD_MR16 :  ArithLogic_MR16<0x16, "mod", srem>;
def MODU_MR16 : ArithLogic_MR16<0x18, "modu", urem>;
def SLL_MR16 :  ArithLogic_MR16<0x20, "sll", shl>;
def SRL_MR16 :  ArithLogic_MR16<0x24, "srl", srl>;
def SRA_MR16 :  ArithLogic_MR16<0x28, "sra", sra>;

def ADD_MR8 :  ArithLogic_MR8<0x04, "add", add>;
def SUB_MR8 :  ArithLogic_MR8<0x06, "sub", sub>; 
def AND_MR8 :  ArithLogic_MR8<0x08, "and", and>;
def OR_MR8  :  ArithLogic_MR8<0x0A, "or", or>;
def XOR_MR8 :  ArithLogic_MR8<0x0C, "xor", xor>;
def MUL_MR8 :  ArithLogic_MR8<0x0E, "mul", mul>;
def DIV_MR8 :  ArithLogic_MR8<0x12, "div", sdiv>;
def DIVU_MR8 : ArithLogic_MR8<0x14, "divu", udiv>;
def MOD_MR8 :  ArithLogic_MR8<0x16, "mod", srem>;
def MODU_MR8 : ArithLogic_MR8<0x18, "modu", urem>;
def SLL_MR8 :  ArithLogic_MR8<0x20, "sll", shl>;
def SRL_MR8 :  ArithLogic_MR8<0x24, "srl", srl>;
def SRA_MR8 :  ArithLogic_MR8<0x28, "sra", sra>;

def ADD_MI4_32: ArithLogic_MI4_32<0x05, "add", add>;
def SUB_MI4_32: ArithLogic_MI4_32<0x07, "sub", sub>;
def MUL_MI4_32: ArithLogic_MI4_32<0x0F, "mul", mul>;

def ADD_RM32 :  ArithLogic_RM32<0x04, "add", add>;
def SUB_RM32 :  ArithLogic_RM32<0x06, "sub", sub>; 
def AND_RM32 :  ArithLogic_RM32<0x08, "and", and>;
def OR_RM32  :  ArithLogic_RM32<0x0A, "or", or>;
def XOR_RM32 :  ArithLogic_RM32<0x0C, "xor", xor>;
def MUL_RM32 :  ArithLogic_RM32<0x0E, "mul", mul>;
def DIV_RM32 :  ArithLogic_RM32<0x12, "div", sdiv>;
def DIVU_RM32 : ArithLogic_RM32<0x14, "divu", udiv>;
def MOD_RM32 :  ArithLogic_RM32<0x16, "mod", srem>;
def MODU_RM32 : ArithLogic_RM32<0x18, "modu", urem>;
def SLL_RM32 :  ArithLogic_RM32<0x20, "sll", shl>;
def SRL_RM32 :  ArithLogic_RM32<0x24, "srl", srl>;
def SRA_RM32 :  ArithLogic_RM32<0x28, "sra", sra>;

def ADD_RM16 :  ArithLogic_RMSX16<0x04, "add", add>;
def SUB_RM16 :  ArithLogic_RMSX16<0x06, "sub", sub>; 
def AND_RM16 :  ArithLogic_RM16<0x08, "and", and>;
def OR_RM16  :  ArithLogic_RM16<0x0A, "or", or>;
def XOR_RM16 :  ArithLogic_RM16<0x0C, "xor", xor>;
def MUL_RM16 :  ArithLogic_RM16<0x0E, "mul", mul>;
def DIV_RM16 :  ArithLogic_RMSX16<0x12, "div", sdiv>;
def DIVU_RM16 : ArithLogic_RM16<0x14, "divu", udiv>;
def MOD_RM16 :  ArithLogic_RMSX16<0x16, "mod", srem>;
def MODU_RM16 : ArithLogic_RM16<0x18, "modu", urem>;
def SLL_RM16 :  ArithLogic_RM16<0x20, "sll", shl>;
def SRL_RM16 :  ArithLogic_RM16<0x24, "srl", srl>;
def SRA_RM16 :  ArithLogic_RM16<0x28, "sra", sra>;

def ADD_RM8 :  ArithLogic_RMSX8<0x04, "add", add>;
def SUB_RM8 :  ArithLogic_RMSX8<0x06, "sub", sub>; 
def AND_RM8 :  ArithLogic_RM8<0x08, "and", and>;
def OR_RM8  :  ArithLogic_RM8<0x0A, "or", or>;
def XOR_RM8 :  ArithLogic_RM8<0x0C, "xor", xor>;
def MUL_RM8 :  ArithLogic_RM8<0x0E, "mul", mul>;
def DIV_RM8 :  ArithLogic_RMSX8<0x12, "div", sdiv>;
def DIVU_RM8 : ArithLogic_RM8<0x14, "divu", udiv>;
def MOD_RM8 :  ArithLogic_RM8<0x16, "mod", srem>;
def MODU_RM8 : ArithLogic_RM8<0x18, "modu", urem>;
def SLL_RM8 :  ArithLogic_RM8<0x20, "sll", shl>;
def SRL_RM8 :  ArithLogic_RM8<0x24, "srl", srl>;
def SRA_RM8 :  ArithLogic_RM8<0x28, "sra", sra>;

def ADD_RR :  ArithLogic_RR<0x04, "add", add, 1>;
def SUB_RR :  ArithLogic_RR<0x06, "sub", sub>; 
def AND_RR :  ArithLogic_RR<0x08, "and", and, 1>;
def OR_RR  :  ArithLogic_RR<0x0A, "or", or, 1>;
def XOR_RR :  ArithLogic_RR<0x0C, "xor", xor, 1>;
def MUL_RR :  ArithLogic_RR<0x0E, "mul", mul, 1>;
def DIV_RR :  ArithLogic_RR<0x12, "div", sdiv>;
def DIVU_RR : ArithLogic_RR<0x14, "divu", udiv>;
def MOD_RR :  ArithLogic_RR<0x16, "mod", srem>;
def MODU_RR : ArithLogic_RR<0x18, "modu", urem>;
def SLL_RR :  ArithLogic_RR<0x20, "sll", shl>;
def SRL_RR :  ArithLogic_RR<0x24, "srl", srl>;
def SRA_RR :  ArithLogic_RR<0x28, "sra", sra>;

let Uses = [PSW] in
{
  def ADDE_RR :  ArithLogic_RR<0x21, "adde", adde, 1>;
  def SUBE_RR :  ArithLogic_RR<0x29, "sube", sube>; 
}

let Defs = [PSW] in
{
  def ADDC_RR :  ArithLogic_RR<0x25, "addc", addc, 1>;
  def SUBC_RR :  ArithLogic_RR<0x30, "subc", subc>; 
}

def ADD_RI4 :  ArithLogic_RI4<0x05, "add", add>;
def SUB_RI4 :  ArithLogic_RI4<0x07, "sub", sub>; 
def AND_RI4 :  ArithLogic_RI4<0x09, "and", and>;
def OR_RI4  :  ArithLogic_RI4<0x0B, "or", or>;
def XOR_RI4 :  ArithLogic_RI4<0x0D, "xor", xor>;
def MUL_RI4 :  ArithLogic_RI4<0x0F, "mul", mul>;
def DIV_RI4 :  ArithLogic_RI4<0x13, "div", sdiv>;
def DIVU_RI4 : ArithLogic_RI4<0x15, "divu", udiv>;
def MOD_RI4 :  ArithLogic_RI4<0x17, "mod", srem>;
def MODU_RI4 : ArithLogic_RI4<0x19, "modu", urem>;

def ADD_RI32 :  ArithLogic_RI32<0x05, "add", add>;
def SUB_RI32 :  ArithLogic_RI32<0x07, "sub", sub>; 
def AND_RI32 :  ArithLogic_RI32<0x09, "and", and>;
def OR_RI32  :  ArithLogic_RI32<0x0B, "or", or>;
def XOR_RI32 :  ArithLogic_RI32<0x0D, "xor", xor>;
def MUL_RI32 :  ArithLogic_RI32<0x0F, "mul", mul>;
def DIV_RI32 :  ArithLogic_RI32<0x13, "div", sdiv>;
def DIVU_RI32 : ArithLogic_RI32<0x15, "divu", udiv>;
def MOD_RI32 :  ArithLogic_RI32<0x17, "mod", srem>;
def MODU_RI32 : ArithLogic_RI32<0x19, "modu", urem>;

def SLL_RI :  Shift_RI<0x11, "sll", shl>;
def SRL_RI :  Shift_RI<0x13, "srl", srl>;
def SRA_RI :  Shift_RI<0x15, "sra", sra>;

def ADD_RM32_POST: ArithLogic_RM32_POST<0x04, "add", add>;
def SUB_RM32_POST: ArithLogic_RM32_POST<0x06, "sub", sub>;
def AND_RM32_POST: ArithLogic_RM32_POST<0x08, "and", and>;
def OR_RM32_POST:  ArithLogic_RM32_POST<0x0A, "or", or>;
def XOR_RM32_POST: ArithLogic_RM32_POST<0x06, "xor", xor>;

def ADDSX_RM16_POST: ArithLogic_RM16_POST<0x04, "addsx", add>;
def SUBSX_RM16_POST: ArithLogic_RM16_POST<0x06, "subsx", sub>;
def AND_RM16_POST: ArithLogic_RM16_POST<0x08, "and", and>;
def OR_RM16_POST:  ArithLogic_RM16_POST<0x0A, "or", or>;
def XOR_RM16_POST: ArithLogic_RM16_POST<0x06, "xor", xor>;

def ADDSX_RM8_POST: ArithLogic_RM8_POST<0x04, "addsx", add>;
def SUBSX_RM8_POST: ArithLogic_RM8_POST<0x06, "subsx", sub>;
def AND_RM8_POST: ArithLogic_RM8_POST<0x08, "and", and>;
def OR_RM8_POST:  ArithLogic_RM8_POST<0x0A, "or", or>;
def XOR_RM8_POST: ArithLogic_RM8_POST<0x06, "xor", xor>;

def ADD_MR32_POST: ArithLogic_MR32_POST<0x04, "add", add>;
def SUB_MR32_POST: ArithLogic_MR32_POST<0x06, "sub", sub>;
def AND_MR32_POST: ArithLogic_MR32_POST<0x08, "and", and>;
def OR_MR32_POST:  ArithLogic_MR32_POST<0x0A, "or", or>;
def XOR_MR32_POST: ArithLogic_MR32_POST<0x06, "xor", xor>;

def ADD_MR16_POST: ArithLogic_MR16_POST<0x04, "add", add>;
def SUB_MR16_POST: ArithLogic_MR16_POST<0x06, "sub", sub>;
def AND_MR16_POST: ArithLogic_MR16_POST<0x08, "and", and>;
def OR_MR16_POST:  ArithLogic_MR16_POST<0x0A, "or", or>;
def XOR_MR16_POST: ArithLogic_MR16_POST<0x06, "xor", xor>;

def ADD_MR8_POST: ArithLogic_MR8_POST<0x04, "add", add>;
def SUB_MR8_POST: ArithLogic_MR8_POST<0x06, "sub", sub>;
def AND_MR8_POST: ArithLogic_MR8_POST<0x08, "and", and>;
def OR_MR8_POST:  ArithLogic_MR8_POST<0x0A, "or", or>;
def XOR_MR8_POST: ArithLogic_MR8_POST<0x06, "xor", xor>;

let isReturn=1, isTerminator=1, isCodeGenOnly=1,
    isBarrier=1, hasCtrlDep=1 in
{
  def RET   : Inst_0 <(outs), (ins),
                "ret", [(Kudeyar_retflag)]> { let op = 0xFF0; }

  def RETI  : Inst_0 <(outs), (ins),
                "reti", [(Kudeyar_retiflag)]> { let op = 0xFF1; }
}

def CMPEQ_RI4:  Compare_RI4<0x2D, "cmpeq", Kudeyar_cmpeq>;
def CMPGT_RI4:  Compare_RI4<0x2F, "cmpgt", Kudeyar_cmpgt>;
def CMPLT_RI4:  Compare_RI4<0x33, "cmplt", Kudeyar_cmplt>;
def CMPGTU_RI4: Compare_RI4<0x35, "cmpgtu", Kudeyar_cmpgtu>;
def CMPLTU_RI4: Compare_RI4<0x37, "cmpltu", Kudeyar_cmpltu>;

def CMPEQ_RI32:  Compare_RI32<0x2D, "cmpeq", Kudeyar_cmpeq>;
def CMPGT_RI32:  Compare_RI32<0x2F, "cmpgt", Kudeyar_cmpgt>;
def CMPLT_RI32:  Compare_RI32<0x33, "cmplt", Kudeyar_cmplt>;
def CMPGTU_RI32: Compare_RI32<0x35, "cmpgtu", Kudeyar_cmpgtu>;
def CMPLTU_RI32: Compare_RI32<0x37, "cmpltu", Kudeyar_cmpltu>;

def CMPEQ_RM32:  Compare_RM32<0x2C, "cmpeq", Kudeyar_cmpeq>;
def CMPGT_RM32:  Compare_RM32<0x2E, "cmpgt", Kudeyar_cmpgt>;
def CMPLT_RM32:  Compare_RM32<0x32, "cmplt", Kudeyar_cmplt>;
def CMPGTU_RM32: Compare_RM32<0x34, "cmpgtu", Kudeyar_cmpgtu>;
def CMPLTU_RM32: Compare_RM32<0x36, "cmpltu", Kudeyar_cmpltu>;

def CMPEQ_MR32:  Compare_MR32<0x2C, "cmpeq", Kudeyar_cmpeq>;
def CMPGT_MR32:  Compare_MR32<0x2E, "cmpgt", Kudeyar_cmpgt>;
def CMPLT_MR32:  Compare_MR32<0x32, "cmplt", Kudeyar_cmplt>;
def CMPGTU_MR32: Compare_MR32<0x34, "cmpgtu", Kudeyar_cmpgtu>;
def CMPLTU_MR32: Compare_MR32<0x36, "cmpltu", Kudeyar_cmpltu>;
 
def CMPEQ_RR:  Compare_RR<0x2C, "cmpeq", Kudeyar_cmpeq, 1>;
def CMPGT_RR:  Compare_RR<0x2E, "cmpgt", Kudeyar_cmpgt>;
def CMPLT_RR:  Compare_RR<0x32, "cmplt", Kudeyar_cmplt>;
def CMPGTU_RR: Compare_RR<0x34, "cmpgtu", Kudeyar_cmpgtu>;
def CMPLTU_RR: Compare_RR<0x36, "cmpltu", Kudeyar_cmpltu>;


let isCall = 1, hasDelaySlot = 0 in
  // All calls clobber the non-callee saved registers. PSW is marked as
  // a use to prevent stack-pointer assignments that appear immediately
  // before calls from potentially appearing dead. Uses for argument
  // registers are added manually.
  let Defs = [SP],
      Uses = [SP] in 
  {
    def CALL: Inst_J<(outs), (ins calltarget:$address),
                  "call\t$address", [(Kudeyar_call imm:$address)]>
    {
	  let op = 0x3B;
	}						  

    def CALL_R: Inst_R<(outs), (ins GPR:$Rd),
                "call\t$Rd", [(Kudeyar_call GPR:$Rd)]>
    {
	  let op = 0x004;
	}

    def CALL_M: Inst_M<(outs), (ins mem:$dst),
                "call\t$dst", [(Kudeyar_call (load addr:$dst))]>
    {
	  let op = 0x004;
	}
  }

def : Pat<(i32 (Kudeyar_Wrapper tglobaladdr:$Imm)), (MOV_RI32 tglobaladdr:$Imm)>;
def : Pat<(i32 (Kudeyar_Wrapper texternalsym:$dst)), (MOV_RI32 texternalsym:$dst)>;
def : Pat<(i32 (Kudeyar_Wrapper tblockaddress:$dst)), (MOV_RI32 tblockaddress:$dst)>;
def : Pat<(i32 (Kudeyar_Wrapper tjumptable:$dst)), (MOV_RI32 tjumptable:$dst)>;

def : Pat<(add GPR:$src, (Kudeyar_Wrapper tglobaladdr :$src2)),
          (ADD_RI32 GPR:$src, tglobaladdr:$src2)>;
def : Pat<(add (Kudeyar_Wrapper tglobaladdr :$src2), GPR:$src),
          (ADD_RI32 GPR:$src, tglobaladdr:$src2)>;

def : Pat<(add GPR:$src, (Kudeyar_Wrapper texternalsym:$src2)),
          (ADD_RI32 GPR:$src, texternalsym:$src2)>;
def : Pat<(add GPR:$src, (Kudeyar_Wrapper tblockaddress:$src2)),
          (ADD_RI32 GPR:$src, tblockaddress:$src2)>;

// calls
def : Pat<(Kudeyar_call (i32 tglobaladdr:$dst)),
          (CALL tglobaladdr:$dst)>;
def : Pat<(Kudeyar_call (i32 texternalsym:$dst)),
          (CALL texternalsym:$dst)>;

