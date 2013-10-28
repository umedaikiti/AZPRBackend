//===-- AZPRISelLowering.h - AZPR DAG Lowering Interface --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that AZPR uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef SAMPLE_ISELLOWERING_H
#define SAMPLE_ISELLOWERING_H

#include "AZPR.h"
#include "AZPRSubtarget.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
namespace AZPRISD {
  enum {
    FIRST_NUMBER = ISD::BUILTIN_OP_END,

    // Jump and link (call)
    Call,

    // Return
    Ret,

    Hi,
    Lo,
    Or
  };
}

class AZPRSubtarget;
//===--------------------------------------------------------------------===//
// TargetLowering Implementation
//===--------------------------------------------------------------------===//

class AZPRTargetLowering : public TargetLowering {
  const AZPRSubtarget &Subtarget;

 public:
  explicit AZPRTargetLowering(AZPRTargetMachine &TM);

  /// LowerOperation - Provide custom lowering hooks for some operations.
  virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;
  virtual SDValue
  LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                       bool isVarArg,
                       const SmallVectorImpl<ISD::InputArg> &Ins,
                       DebugLoc dl, SelectionDAG &DAG,
                       SmallVectorImpl<SDValue> &InVals) const;

  virtual SDValue
  LowerCall(CallLoweringInfo &CLI,
            SmallVectorImpl<SDValue> &InVals) const;

  virtual SDValue
  LowerCallResult(SDValue Chain, SDValue InFlag,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::InputArg> &Ins,
                  DebugLoc dl, SelectionDAG &DAG,
                  SmallVectorImpl<SDValue> &InVals) const;

  virtual SDValue
  LowerReturn(SDValue Chain,
              CallingConv::ID CallConv, bool isVarArg,
              const SmallVectorImpl<ISD::OutputArg> &Outs,
              const SmallVectorImpl<SDValue> &OutVals,
              DebugLoc dl, SelectionDAG &DAG) const;

    /// getSetCCResultType - get the ISD::SETCC result ValueType
    EVT getSetCCResultType(EVT VT) const;

    std::pair<unsigned, const TargetRegisterClass*>
              getRegForInlineAsmConstraint(const std::string &Constraint,
              EVT VT) const;

    /// getTargetNodeName - This method returns the name of a target specific
    //  DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

    /// ReplaceNodeResults - Replace the results of node with an illegal result
    /// type with new values built out of custom code.
    ///
    virtual void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue>&Results,
                                    SelectionDAG &DAG) const;

    virtual MachineBasicBlock *
      EmitInstrWithCustomInserter(MachineInstr *MI,
                                  MachineBasicBlock *MBB) const;

 private:
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerLOAD(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSTORE(SDValue Op, SelectionDAG &DAG) const;
};
} // end of namespace llvm

#endif // SAMPLE_ISELLOWERING_H
