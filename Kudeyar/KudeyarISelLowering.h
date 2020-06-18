//===-------- KudeyarISelLowering.h ---------===//
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
#ifndef KUDEYARISELLOWERING_H
#define KUDEYARISELLOWERING_H

#include "Kudeyar.h"
#include "KudeyarSubtarget.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm 
{
  namespace KudeyarISD 
  {
    enum NodeType 
    {
      // Start the numbering from where ISD NodeType finishes.
      FIRST_NUMBER = ISD::BUILTIN_OP_END,

      /// Return with a flag operand. Operand 0 is the chain operand.
      RET_FLAG,

      /// Same as RET_FLAG, but used for returning from ISRs.
      RETI_FLAG,

     /// CALL - These operations represent an abstract call
      /// instruction, which includes a bunch of information.
      CALL,

      /// Kudeyar conditional branches. Operand 0 is the chain operand, operand 1
      /// is the block to branch if condition is true, operand 2 is the
      /// condition code, and operand 3 is the flag operand produced by a CMP
      /// instruction.
      BR_CC_T,
	  BR_CC_F,

	  SELECT_CC_T,
	  SELECT_CC_F,

	  CMPEQ,
	  CMPGT,
	  CMPLT,
	  CMPGTU,
	  CMPLTU,

      Wrapper,
      DynAlloc,
      Sync
    };
  }

  //===--------------------------------------------------------------------===//
  // TargetLowering Implementation
  //===--------------------------------------------------------------------===//

  class KudeyarTargetLowering : public TargetLowering  
  {
  public:
    explicit KudeyarTargetLowering(KudeyarTargetMachine &TM);

    virtual MVT getShiftAmountTy(EVT LHSTy) const { return MVT::i32; }
    
    /// LowerOperation - Provide custom lowering hooks for some operations.
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    /// getTargetNodeName - This method returns the name of a target specific
    //  DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

  private:
    // Subtarget Info
    const KudeyarSubtarget *Subtarget;

    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const; 
    SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
	SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG) const;

	SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSETCC(SDValue Op, SelectionDAG &DAG) const;
	SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
	SDValue LowerSIGN_EXTEND(SDValue Op, SelectionDAG &DAG) const; 

    SDValue LowerCCCArguments(SDValue Chain,
                              CallingConv::ID CallConv,
                              bool isVarArg,
                              const SmallVectorImpl<ISD::InputArg> &Ins,
                              DebugLoc dl,
                              SelectionDAG &DAG,
                              SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerCCCCallTo(SDValue Chain, SDValue Callee,
                                     CallingConv::ID CallConv, bool isVarArg,
                                     bool isTailCall,
                                     const SmallVectorImpl<ISD::OutputArg>
                                       &Outs,
                                     const SmallVectorImpl<SDValue> &OutVals,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     DebugLoc dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals) const;

  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                                      CallingConv::ID CallConv, bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                      DebugLoc dl, SelectionDAG &DAG,
                                      SmallVectorImpl<SDValue> &InVals) const; 


	//- must be exist without function all
    virtual SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv, bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

    virtual SDValue
      LowerCall(TargetLowering::CallLoweringInfo &CLI,
                SmallVectorImpl<SDValue> &InVals) const;

 	//- must be exist without function all
    virtual SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  DebugLoc dl, SelectionDAG &DAG) const;

    virtual MachineBasicBlock* EmitInstrWithCustomInserter(MachineInstr *MI,
                                             MachineBasicBlock *BB) const; 
	 
    virtual bool getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                    SDValue &Base, SDValue &Offset,
                                    ISD::MemIndexedMode &AM, SelectionDAG &DAG) const;
  };
}

#endif // KudeyarISELLOWERING_H
