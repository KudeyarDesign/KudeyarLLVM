//===-------- KudeyarISelLowering.cpp ---------===//
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
#define DEBUG_TYPE "kudeyar-lower"
#include "KudeyarISelLowering.h"
#include "KudeyarMachineFunctionInfo.h"
#include "KudeyarTargetMachine.h"
#include "KudeyarTargetObjectFile.h"
#include "KudeyarSubtarget.h"
//#include "MCTargetDesc/KudeyarBaseInfo.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Intrinsics.h"
#include "llvm/CallingConv.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

const char *KudeyarTargetLowering::getTargetNodeName(unsigned Opcode) const 
{
  switch (Opcode) 
  {
    case KudeyarISD::BR_CC_T:            return "KudeyarISD::BR_CC_T";
	case KudeyarISD::BR_CC_F:            return "KudeyarISD::BR_CC_F";
	case KudeyarISD::SELECT_CC_T:        return "KudeyarISD::SELECT_CC_T";
	case KudeyarISD::SELECT_CC_F:        return "KudeyarISD::SELECT_CC_f";
	case KudeyarISD::CMPEQ:              return "KudeyarISD::CMPEQ";
	case KudeyarISD::CMPGT:              return "KudeyarISD::CMPGT"; 
	case KudeyarISD::CMPLT:              return "KudeyarISD::CMPLT";
	case KudeyarISD::CMPGTU:             return "KudeyarISD::CMPGTU";
	case KudeyarISD::CMPLTU:             return "KudeyarISD::CMPLTU";
    case KudeyarISD::CALL:               return "KudeyarISD::CALL";
    case KudeyarISD::RET_FLAG:           return "KudeyarISD::RET_FLAG";
    case KudeyarISD::RETI_FLAG:          return "KudeyarISD::RETI_FLAG";
    case KudeyarISD::Wrapper:            return "KudeyarISD::Wrapper";
    default:                             return NULL;
  }
}

KudeyarTargetLowering::
KudeyarTargetLowering(KudeyarTargetMachine &TM)
  : TargetLowering(TM, new KudeyarTargetObjectFile()),
    Subtarget(&TM.getSubtarget<KudeyarSubtarget>()) 
{
  // Set up the register classes
  addRegisterClass(MVT::i32, &Kudeyar::GPRRegClass);

  // Used by legalize types to correctly generate the setcc result.
  // Without this, every float setcc comes with a AND/OR with the result,
  // we don't want this, since the fpcmp result goes to a flag register,
  // which is used implicitly by brcond and select operations.
  AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);

  // Division is expensive
  setIntDivIsCheap(false);

  setStackPointerRegisterToSaveRestore(Kudeyar::SP);
  setBooleanContents(ZeroOrOneBooleanContent);

//  setLoadExtAction(ISD::SEXTLOAD, MVT::i8,  Expand);
//  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Expand);

  // We have post-incremented loads / stores.
  setIndexedLoadAction(ISD::POST_INC, MVT::i8, Legal);
  setIndexedLoadAction(ISD::POST_INC, MVT::i16, Legal);
  setIndexedLoadAction(ISD::POST_INC, MVT::i32, Legal);

  setIndexedStoreAction(ISD::POST_INC, MVT::i8, Legal);
  setIndexedStoreAction(ISD::POST_INC, MVT::i16, Legal);
  setIndexedStoreAction(ISD::POST_INC, MVT::i32, Legal);

  setOperationAction(ISD::BlockAddress, MVT::i32, Custom);

  setOperationAction(ISD::GlobalAddress,  MVT::i32, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i32, Custom);
  setOperationAction(ISD::JumpTable, MVT::i32,   Custom);

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
//  setOperationAction(ISD::BR_CC, MVT::i8, Custom);
//  setOperationAction(ISD::BR_CC, MVT::i16, Custom);
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);

//  setOperationAction(ISD::SETCC, MVT::i8,  Custom);
//  setOperationAction(ISD::SETCC, MVT::i16, Custom);
  setOperationAction(ISD::SETCC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
//  setOperationAction(ISD::SELECT_CC, MVT::i8,  Custom);
//  setOperationAction(ISD::SELECT_CC, MVT::i16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);

//  setOperationAction(ISD::SIGN_EXTEND, MVT::i32, Custom);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);

  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);

  setOperationAction(ISD::MULHS, MVT::i32, Expand);
  setOperationAction(ISD::MULHU, MVT::i32, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);

//- Set .align 2
// It will emit .align 2 later
  setMinFunctionAlignment(2);

// must, computeRegisterProperties - Once all of the register classes are 
//  added, this allows us to compute derived properties we expose.
  computeRegisterProperties();
}

SDValue KudeyarTargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode())
  {
    case ISD::SIGN_EXTEND:      return LowerSIGN_EXTEND(Op, DAG);

    case ISD::GlobalAddress:    return LowerGlobalAddress(Op, DAG);
    case ISD::ExternalSymbol:   return LowerExternalSymbol(Op, DAG);
    case ISD::BlockAddress:     return LowerBlockAddress(Op, DAG);
    case ISD::JumpTable:        return LowerJumpTable(Op, DAG);

    case ISD::SETCC:            return LowerSETCC(Op, DAG);
//    case ISD::BRCOND:             return LowerBRCOND(Op, DAG);
    case ISD::BR_CC:            return LowerBR_CC(Op, DAG);
    case ISD::SELECT_CC:        return LowerSELECT_CC(Op, DAG);

//    case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);

	default: break; 
  }
  return SDValue();
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//

#include "KudeyarGenCallingConv.inc"

SDValue KudeyarTargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const 
{
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result = DAG.getTargetGlobalAddress(GV, Op.getDebugLoc(),
                                              getPointerTy(), Offset);
  return DAG.getNode(KudeyarISD::Wrapper, Op.getDebugLoc(),
                     getPointerTy(), Result);
}

SDValue KudeyarTargetLowering::LowerExternalSymbol(SDValue Op,
                                                  SelectionDAG &DAG) const 
{
  DebugLoc dl = Op.getDebugLoc();
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  SDValue Result = DAG.getTargetExternalSymbol(Sym, getPointerTy());

  return DAG.getNode(KudeyarISD::Wrapper, dl, getPointerTy(), Result);
}

SDValue KudeyarTargetLowering::LowerBlockAddress(SDValue Op,
                                                SelectionDAG &DAG) const 
{
  DebugLoc dl = Op.getDebugLoc();
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
  SDValue Result = DAG.getTargetBlockAddress(BA, getPointerTy());

  return DAG.getNode(KudeyarISD::Wrapper, dl, getPointerTy(), Result);
}


SDValue KudeyarTargetLowering::LowerJumpTable(SDValue Op, SelectionDAG &DAG) const
{
  DebugLoc dl = Op.getDebugLoc();
  JumpTableSDNode* JT = cast<JumpTableSDNode>(Op);
  SDValue Result = DAG.getTargetJumpTable(JT->getIndex(), getPointerTy());

  return DAG.getNode(KudeyarISD::Wrapper, dl, getPointerTy(), Result);
}

SDValue KudeyarTargetLowering::
LowerBRCOND(SDValue Op, SelectionDAG &DAG) const
{
  return Op;
}

static SDValue EmitCMP(SDValue &LHS, SDValue &RHS, ISD::CondCode CC,
                       DebugLoc dl, SelectionDAG &DAG, int& br_code) 
{
  // Minor optimization: if LHS is a constant, swap operands, then the
  // constant can be folded into comparison.
  if (LHS.getOpcode() == ISD::Constant)
    std::swap(LHS, RHS);
  int cmp_code = 0;
  switch (CC) 
  {
  default: llvm_unreachable("Invalid integer condition!");
  case ISD::SETEQ:
	br_code = KudeyarISD::BR_CC_T;
	cmp_code = KudeyarISD::CMPEQ;
    break;
  case ISD::SETNE:
 	br_code = KudeyarISD::BR_CC_F;
	cmp_code = KudeyarISD::CMPEQ;
    break;
  case ISD::SETULE:
 	br_code = KudeyarISD::BR_CC_F;
	cmp_code = KudeyarISD::CMPGTU;
	break;
  case ISD::SETUGE:
 	br_code = KudeyarISD::BR_CC_F;
	cmp_code = KudeyarISD::CMPLTU;
    break;
  case ISD::SETUGT:
  	br_code = KudeyarISD::BR_CC_T;
	cmp_code = KudeyarISD::CMPGTU;    
  case ISD::SETULT:
  	br_code = KudeyarISD::BR_CC_T;
	cmp_code = KudeyarISD::CMPLTU;
    break;
  case ISD::SETLE:
  	br_code = KudeyarISD::BR_CC_F;
	cmp_code = KudeyarISD::CMPGT;
    break;
  case ISD::SETGE:
   	br_code = KudeyarISD::BR_CC_F;
	cmp_code = KudeyarISD::CMPLT;
    break;
  case ISD::SETGT:
  	br_code = KudeyarISD::BR_CC_T;
	cmp_code = KudeyarISD::CMPGT;
    break;
  case ISD::SETLT:
  	br_code = KudeyarISD::BR_CC_T;
	cmp_code = KudeyarISD::CMPLT;
    break;
  }

  return DAG.getNode(cmp_code, dl, MVT::Glue, LHS, RHS);
}

SDValue KudeyarTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const 
{
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS   = Op.getOperand(2);
  SDValue RHS   = Op.getOperand(3);
  SDValue Dest  = Op.getOperand(4);
  DebugLoc dl   = Op.getDebugLoc();

  int br_code;
  SDValue Flag = EmitCMP(LHS, RHS, CC, dl, DAG, br_code);

  return DAG.getNode(br_code, dl, Op.getValueType(),
                     Chain, Dest, Flag);
}

SDValue KudeyarTargetLowering::LowerSETCC(SDValue Op, SelectionDAG &DAG) const 
{
  SDValue LHS   = Op.getOperand(0);
  SDValue RHS   = Op.getOperand(1);
  DebugLoc dl   = Op.getDebugLoc();

  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();
  int br_code;
  SDValue Flag = EmitCMP(LHS, RHS, CC, dl, DAG, br_code);

  EVT VT = Op.getValueType();
  SDValue One  = DAG.getConstant(1, VT);
  SDValue Zero = DAG.getConstant(0, VT);
  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SmallVector<SDValue, 4> Ops;
  Ops.push_back(One);
  Ops.push_back(Zero);
  Ops.push_back(Flag);
  if (br_code == KudeyarISD::BR_CC_T)
	br_code = KudeyarISD::SELECT_CC_T;
  else br_code = KudeyarISD::SELECT_CC_F;
  return DAG.getNode(br_code, dl, VTs, &Ops[0], Ops.size());
}

SDValue KudeyarTargetLowering::LowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const 
{
  SDValue LHS    = Op.getOperand(0);
  SDValue RHS    = Op.getOperand(1);
  SDValue TrueV  = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  DebugLoc dl    = Op.getDebugLoc();

  int br_code;
  SDValue Flag = EmitCMP(LHS, RHS, CC, dl, DAG, br_code);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SmallVector<SDValue, 3> Ops;
  Ops.push_back(TrueV);
  Ops.push_back(FalseV);
  Ops.push_back(Flag);

  if (br_code == KudeyarISD::BR_CC_T)
	br_code = KudeyarISD::SELECT_CC_T;
  else br_code = KudeyarISD::SELECT_CC_F;
  return DAG.getNode(br_code, dl, VTs, &Ops[0], Ops.size());
}

SDValue KudeyarTargetLowering::LowerSIGN_EXTEND(SDValue Op,
                                               SelectionDAG &DAG) const 
{
  SDValue Val = Op.getOperand(0);
  EVT VT      = Op.getValueType();
  DebugLoc dl = Op.getDebugLoc();

  return DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, VT,
                     DAG.getNode(ISD::ANY_EXTEND, dl, VT, Val),
                     DAG.getValueType(Val.getValueType()));
}

/// LowerCCCCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: sret.
SDValue
KudeyarTargetLowering::LowerCCCCallTo(SDValue Chain, SDValue Callee,
                                     CallingConv::ID CallConv, bool isVarArg,
                                     bool isTailCall,
                                     const SmallVectorImpl<ISD::OutputArg>
                                       &Outs,
                                     const SmallVectorImpl<SDValue> &OutVals,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     DebugLoc dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals) const 
{
  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_Kudeyar);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain ,DAG.getConstant(NumBytes,
                                                      getPointerTy(), true));

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) 
  {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) 
	{
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
        break;
    }

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) 
	{
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } 
	else 
	{
      assert(VA.isMemLoc());

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, dl, Kudeyar::SP, getPointerTy());

      SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(),
                                   StackPtr,
                                   DAG.getIntPtrConstant(VA.getLocMemOffset()));

      SDValue MemOp;
      ISD::ArgFlagsTy Flags = Outs[i].Flags;

      if (Flags.isByVal()) 
	  {
        SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), MVT::i32);
        MemOp = DAG.getMemcpy(Chain, dl, PtrOff, Arg, SizeNode,
                              Flags.getByValAlign(),
                              /*isVolatile*/false,
                              /*AlwaysInline=*/true,
                              MachinePointerInfo(),
                              MachinePointerInfo());
      } 
	  else 
	  {
        MemOp = DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo(),
                             false, false, 0);
      }

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        &MemOpChains[0], MemOpChains.size());

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) 
  {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i32);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(KudeyarISD::CALL, dl, NodeTys, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl,
                         DAG, InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue
KudeyarTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                      CallingConv::ID CallConv, bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                      DebugLoc dl, SelectionDAG &DAG,
                                      SmallVectorImpl<SDValue> &InVals) const 
{

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Kudeyar);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) 
  {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

SDValue
KudeyarTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const 
{
  SelectionDAG &DAG                     = CLI.DAG;
  DebugLoc &dl                          = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  // Kudeyar target does not yet support tail call optimization.
  isTailCall = false;

  switch (CallConv) 
  {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    return LowerCCCCallTo(Chain, Callee, CallConv, isVarArg, isTailCall,
                          Outs, OutVals, Ins, dl, DAG, InVals);
  case CallingConv::Kudeyar_INTR:
    report_fatal_error("ISRs cannot be called directly");
  }
}

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue
KudeyarTargetLowering::LowerFormalArguments(SDValue Chain,
                                         CallingConv::ID CallConv,
                                         bool isVarArg,
                                         const SmallVectorImpl<ISD::InputArg> &Ins,
                                         DebugLoc dl, SelectionDAG &DAG,
                                         SmallVectorImpl<SDValue> &InVals)
                                          const 
{
  switch (CallConv) 
  {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    return LowerCCCArguments(Chain, CallConv, isVarArg, Ins, dl, DAG, InVals);
  case CallingConv::Kudeyar_INTR:
    if (Ins.empty())
      return Chain;
    report_fatal_error("ISRs cannot have arguments");
  }
}

/// LowerCCCArguments - transform physical registers into virtual registers and
/// generate load operations for arguments places on the stack.
// FIXME: struct return stuff
SDValue
KudeyarTargetLowering::LowerCCCArguments(SDValue Chain,
                                        CallingConv::ID CallConv,
                                        bool isVarArg,
                                        const SmallVectorImpl<ISD::InputArg>
                                          &Ins,
                                        DebugLoc dl,
                                        SelectionDAG &DAG,
                                        SmallVectorImpl<SDValue> &InVals)
                                          const 
{
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  KudeyarMachineFunctionInfo *FuncInfo = MF.getInfo<KudeyarMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_Kudeyar);

  // Create frame index for the start of the first vararg value
  if (isVarArg) 
  {
    unsigned Offset = CCInfo.getNextStackOffset();
    FuncInfo->setVarArgsFrameIndex(MFI->CreateFixedObject(1, Offset, true));
  }

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) 
  {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) 
	{
       // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy) 
	  {
      default:
        {
#ifndef NDEBUG
          errs() << "LowerFormalArguments Unhandled argument type: "
               << RegVT.getSimpleVT().SimpleTy << "\n";
#endif
          llvm_unreachable(0);
        }
      case MVT::i32:
        unsigned VReg = RegInfo.createVirtualRegister(&Kudeyar::GPRRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

        // If this is an 8-bit value, it is really passed promoted to 16
        // bits. Insert an assert[sz]ext to capture this, then truncate to the
        // right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);

        InVals.push_back(ArgValue);
      }
    } 
	else 
	{
      // Sanity check
      assert(VA.isMemLoc());

      SDValue InVal;
      ISD::ArgFlagsTy Flags = Ins[i].Flags;

      if (Flags.isByVal()) 
	  {
        int FI = MFI->CreateFixedObject(Flags.getByValSize(),
                                        VA.getLocMemOffset(), true);
        InVal = DAG.getFrameIndex(FI, getPointerTy());
      } 
	  else 
	  {
        // Load the argument to a virtual register
        unsigned ObjSize = VA.getLocVT().getSizeInBits()/8;
        if (ObjSize > 4) 
		{
            errs() << "LowerFormalArguments Unhandled argument type: "
                << EVT(VA.getLocVT()).getEVTString()
                << "\n";
        }
        // Create the frame index object for this incoming parameter...
        int FI = MFI->CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

        // Create the SelectionDAG nodes corresponding to a load
        //from this parameter
        SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
        InVal = DAG.getLoad(VA.getLocVT(), dl, Chain, FIN,
                            MachinePointerInfo::getFixedStack(FI),
                            false, false, false, 0);
      }

      InVals.push_back(InVal);
    }
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
KudeyarTargetLowering::LowerReturn(SDValue Chain,
                                CallingConv::ID CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                DebugLoc dl, SelectionDAG &DAG) const 
{
 // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // ISRs cannot return any value.
  if (CallConv == CallingConv::Kudeyar_INTR && !Outs.empty())
    report_fatal_error("ISRs cannot return any value");

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Kudeyar);

  // If this is the first return lowered for this function, add the regs to the
  // liveout set for the function.
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) 
  {
    for (unsigned i = 0; i != RVLocs.size(); ++i)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }

  SDValue Flag;

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) 
  {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
                             OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
  }

  unsigned Opc = (CallConv == CallingConv::Kudeyar_INTR ?
                  KudeyarISD::RETI_FLAG : KudeyarISD::RET_FLAG);

  if (Flag.getNode())
    return DAG.getNode(Opc, dl, MVT::Other, Chain, Flag);

  // Return Void
  return DAG.getNode(Opc, dl, MVT::Other, Chain);
}

static void printBlock(MachineBasicBlock *BB)
{
  printf("\nBlock = %s\n", BB->getBasicBlock()->getName());
  for (MachineBasicBlock::instr_iterator it = BB->begin(); it != BB->end(); it++)
  {
    MachineInstr * insn = it;
	printf(">> insn = %d\n", insn->getOpcode());
  }
  printf("\n");
}

MachineBasicBlock*
KudeyarTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                  MachineBasicBlock *BB) const 
{
  unsigned Opc = MI->getOpcode();

  const TargetInstrInfo &TII = *getTargetMachine().getInstrInfo();
  DebugLoc dl = MI->getDebugLoc();

  assert((Opc == Kudeyar::Select32T || Opc == Kudeyar::Select32F) &&
         "Unexpected instr type to insert");

   // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = BB;
  ++I;

  //  thisMBB:
  //  ...
  //   TrueVal = ...
  //   cmpTY r1, r2
  //   jCC copy1MBB
  //   fallthrough --> copy0MBB
  MachineBasicBlock *thisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(I, copy0MBB);
  F->insert(I, copy1MBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  copy1MBB->splice(copy1MBB->begin(), BB,
                   llvm::next(MachineBasicBlock::iterator(MI)),
                   BB->end());
  copy1MBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(copy0MBB);
  BB->addSuccessor(copy1MBB);

  int br_code;
  switch (Opc)
  {
  case Kudeyar::Select32T:  br_code = Kudeyar::BTs;  break;
  case Kudeyar::Select32F:  br_code = Kudeyar::BFs;  break;
  default:  br_code = 0;
  }

  BuildMI(BB, dl, TII.get(br_code))
    .addMBB(copy1MBB);
 
   //  copy0MBB:
  //   %FalseValue = ...
  //   # fallthrough to copy1MBB
  BB = copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(copy1MBB);

  //  copy1MBB:
  //   %Result = phi [ %FalseValue, copy0MBB ], [ %TrueValue, thisMBB ]
  //  ...
  BB = copy1MBB;
  BuildMI(*BB, BB->begin(), dl, TII.get(Kudeyar::PHI),
          MI->getOperand(0).getReg())
    .addReg(MI->getOperand(2).getReg()).addMBB(copy0MBB)
    .addReg(MI->getOperand(1).getReg()).addMBB(thisMBB);

  MI->eraseFromParent();   // The pseudo instruction is gone now.
  return BB;
}

/// getPostIndexedAddressParts - returns true by value, base pointer and
/// offset pointer and addressing mode by reference if this node can be
/// combined with a load / store to form a post-indexed load / store.
bool KudeyarTargetLowering::getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                                      SDValue &Base,
                                                      SDValue &Offset,
                                                      ISD::MemIndexedMode &AM,
                                                      SelectionDAG &DAG) const 
{
  EVT VT;
  if (N->getOpcode() == ISD::LOAD)
  {
    LoadSDNode *LD = cast<LoadSDNode>(N); 
//  if (LD->getExtensionType() != ISD::NON_EXTLOAD)
//    return false;

    VT = LD->getMemoryVT();
  }
  else if (N->getOpcode() == ISD::STORE)
  {
    StoreSDNode *ST = cast<StoreSDNode>(N);
    VT = ST->getMemoryVT();
  }
  else return false;

  if (VT != MVT::i8 && VT != MVT::i16 && VT != MVT::i32)
    return false;

  if (Op->getOpcode() != ISD::ADD)
    return false;

  if (ConstantSDNode *RHS = dyn_cast<ConstantSDNode>(Op->getOperand(1))) 
  {
    uint64_t RHSC = RHS->getZExtValue();
    if ((VT == MVT::i32 && RHSC != 4) ||
		(VT == MVT::i16 && RHSC != 2) ||
        (VT == MVT::i8 && RHSC != 1))
      return false;

    Base = Op->getOperand(0);
    Offset = DAG.getConstant(RHSC, VT);
    AM = ISD::POST_INC;

    return true;
  }

  return false;
}
