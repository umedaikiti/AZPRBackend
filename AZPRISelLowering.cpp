//===-- AZPRISelLowering.cpp - AZPR DAG Lowering Implementation -----------===//
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

#define DEBUG_TYPE "azpr-lower"
#include "AZPRISelLowering.h"
#include "AZPRMachineFunction.h"
#include "AZPRTargetMachine.h"
#include "AZPRSubtarget.h"
#include "InstPrinter/AZPRInstPrinter.h"
#include "MCTargetDesc/AZPRMCTargetDesc.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Intrinsics.h"
#include "llvm/CallingConv.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
using namespace llvm;

static std::string getFlagsString(const ISD::ArgFlagsTy &Flags) {
  if (Flags.isZExt()) {
    return "ZExt";
  } else if (Flags.isSExt()) {
    return "SExt";
  } else if (Flags.isInReg()) {
    return "Reg";
  } else if (Flags.isSRet()) {
    return "SRet";
  } else if (Flags.isByVal()) {
    return "ByVal";
  } else if (Flags.isNest()) {
    return "Nest";
  } else {
    return "No Flags";
  }
}

//llcのdebugオプションでNodeの名前を表示する時とかに必要
const char *AZPRTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case AZPRISD::Call:         return "AZPRISD::Call";
  case AZPRISD::Ret:          return "AZPRISD::Ret";
  case AZPRISD::Hi:           return "AZPRISD::Hi";
  case AZPRISD::Lo:           return "AZPRISD::Lo";
  case AZPRISD::Or:           return "AZPRISD::Or";
  default:                    return NULL;
  }
}

AZPRTargetLowering::
AZPRTargetLowering(AZPRTargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileELF()),
    Subtarget(*TM.getSubtargetImpl()) {
  DEBUG(dbgs() << ">> AZPRTargetLowering::constructor <<\n");

  // booleanをどう表すかを定義
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  // ターゲットで利用できるレジスタを登録
  addRegisterClass(MVT::i32, &AZPR::CPUGRegsRegClass);
//  addRegisterClass(MVT::i32, &AZPR::CPUCRegsRegClass);

  // (符号)拡張ロード命令が対応していない型の操作方法を登録
  setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
  setLoadExtAction(ISD::EXTLOAD,  MVT::i8,  Custom);
//  setLoadExtAction(ISD::EXTLOAD,  MVT::i16, Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
//  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8,  Promote);
//  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i8,  Custom);
//  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, Promote);

  setTruncStoreAction(MVT::i32, MVT::i8 , Custom);

  // 関数のアラインメント
  setMinFunctionAlignment(2);

  // スタックポインタのレジスタを指定
  setStackPointerRegisterToSaveRestore(AZPR::r30);

  // レジスタの操作方法を計算
  computeRegisterProperties();

  setOperationAction(ISD::BR_CC, MVT::Other, Expand);
//  setOperationAction(ISD::SELECT, MVT::i32, Expand);
//  setOperationAction(ISD::SELECT_CC, MVT::Other, Expand);
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::LOAD, MVT::i8, Custom);
  setOperationAction(ISD::STORE, MVT::i8, Custom);
//  setOperationAction(ISD::LOAD, MVT::i16, Custom);
}

void
AZPRTargetLowering::ReplaceNodeResults(SDNode *N,
                                       SmallVectorImpl<SDValue> &Results,
                                       SelectionDAG &DAG) const {
  SDValue Res = LowerOperation(SDValue(N, 0), DAG);

  for (unsigned I = 0, E = Res->getNumValues(); I != E; ++I)
    Results.push_back(Res.getValue(I));
}

SDValue AZPRTargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode())
  {
    case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
    case ISD::LOAD:               return LowerLOAD(Op, DAG);
    case ISD::STORE:              return LowerSTORE(Op, DAG);
  }
  llvm_unreachable("not supported operation");
  return SDValue();
}

//これをしないとSelectionDAG::AssignTopologicalOrder()でUNREACHABLEになる
//たぶんNodeの作り直しをしている
static SDValue getTargetNode(SDValue Op, SelectionDAG &DAG) {
  EVT Ty = Op.getValueType();

  if (GlobalAddressSDNode *N = dyn_cast<GlobalAddressSDNode>(Op))
    return DAG.getTargetGlobalAddress(N->getGlobal(), Op.getDebugLoc(), Ty, 0);
/*
  if (ExternalSymbolSDNode *N = dyn_cast<ExternalSymbolSDNode>(Op))
    return DAG.getTargetExternalSymbol(N->getSymbol(), Ty, Flag);
  if (BlockAddressSDNode *N = dyn_cast<BlockAddressSDNode>(Op))
    return DAG.getTargetBlockAddress(N->getBlockAddress(), Ty, 0, Flag);
  if (JumpTableSDNode *N = dyn_cast<JumpTableSDNode>(Op))
    return DAG.getTargetJumpTable(N->getIndex(), Ty, Flag);
  if (ConstantPoolSDNode *N = dyn_cast<ConstantPoolSDNode>(Op))
    return DAG.getTargetConstantPool(N->getConstVal(), Ty, N->getAlignment(),
                                     N->getOffset(), Flag);
*/
  llvm_unreachable("Unexpected node type.");
  return SDValue();
}

static SDValue getAddrNonPIC(SDValue Op, SelectionDAG &DAG) {
  DebugLoc DL = Op.getDebugLoc();
  EVT Ty = Op.getValueType();
  SDValue Hi = DAG.getNode(AZPRISD::Hi, DL, Ty, getTargetNode(Op, DAG));
  SDValue Lo = DAG.getNode(AZPRISD::Lo, DL, Ty, getTargetNode(Op, DAG));

  return DAG.getNode(AZPRISD::Or, DL, Ty, Hi, Lo);
}

SDValue AZPRTargetLowering::LowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  // FIXME there isn't actually debug info here
  DebugLoc dl = Op.getDebugLoc();
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();

  if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
/*    const MipsTargetObjectFile &TLOF =
      (const MipsTargetObjectFile&)getObjFileLowering();

    // %gp_rel relocation
    if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine())) {
      SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                              MipsII::MO_GPREL);
      SDValue GPRelNode = DAG.getNode(MipsISD::GPRel, dl,
                                      DAG.getVTList(MVT::i32), &GA, 1);
      SDValue GPReg = DAG.getRegister(Mips::GP, MVT::i32);
      return DAG.getNode(ISD::ADD, dl, MVT::i32, GPReg, GPRelNode);
    }*/

    // %hi/%lo relocation
/*    const GlobalVariable *GVA = dyn_cast<GlobalVariable>(GV);
    if(GVA){
      DEBUG(dbgs() << "GlobalVariable\n");
      DEBUG(GVA->dump());
      const ConstantArray *CA = dyn_cast<ConstantArray>(GVA->getInitializer());
      if(GVA->hasInitializer() && CA){
        DEBUG(CA->dump());
        for(int i=0;i<CA->getNumOperands();i++){
          const ConstantInt *op = dyn_cast<ConstantInt>(CA->getOperand(i));

          if(op && op->getNumOperands()){
            const ConstantExpr *CE = dyn_cast<ConstantExpr>(op->getOperand(0));
            if(CE) DEBUG(CE->dump());
          }
        }
      }
    }*/
    DEBUG(dbgs() << "LowerGlobalAddress\n");
    return getAddrNonPIC(Op, DAG);
  }
  llvm_unreachable("AZPRTargetLowering::LowerGlobalAddress");
/*

  if (GV->hasInternalLinkage() || (GV->hasLocalLinkage() && !isa<Function>(GV)))
    return getAddrLocal(Op, DAG, HasMips64);

  if (LargeGOT)
    return getAddrGlobalLargeGOT(Op, DAG, MipsII::MO_GOT_HI16,
                                 MipsII::MO_GOT_LO16);

  return getAddrGlobal(Op, DAG,
                       HasMips64 ? MipsII::MO_GOT_DISP : MipsII::MO_GOT16);*/
}

//XCoreのを参照
SDValue AZPRTargetLowering::LowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  LoadSDNode *LD = cast<LoadSDNode>(Op.getNode());
  DebugLoc dl = Op.getDebugLoc();
  EVT SrcVT = LD->getMemoryVT();
  if(SrcVT == MVT::i8){//細かい条件は考えない
    SDValue tmp1 = DAG.getNode(ISD::AND, dl, MVT::i32, LD->getBasePtr(), DAG.getConstant(3, MVT::i32));
    tmp1 = DAG.getNode(ISD::SHL, dl, MVT::i32, tmp1, DAG.getConstant(3, MVT::i32));
    tmp1 = DAG.getNode(ISD::SUB, dl, MVT::i32, DAG.getConstant(24, MVT::i32), tmp1);
    SDValue tmp2 = DAG.getNode(ISD::AND, dl, MVT::i32, LD->getBasePtr(), DAG.getConstant(~3, MVT::i32));
//    return tmp2 = DAG.getLoad(LD->getAddressingMode(), ISD::NON_EXTLOAD /*LD->getExtensionType()*/, MVT::i32, dl,
//			LD->getChain(), tmp2, LD->getOffset(), MVT::i32, LD->getMemOperand());
    tmp2 = DAG.getLoad(getPointerTy(), dl, LD->getChain(),
                               tmp2, MachinePointerInfo(),
                               false, false, false, 0);
    tmp1 = DAG.getNode(ISD::SRL, dl, MVT::i32, tmp2, tmp1);
    tmp1 = DAG.getZExtOrTrunc(tmp1, dl, LD->getValueType(0)/*MVT::i8*/);//i8にtrunc, i8でいいのか?getValueTypeするべきか?
//    tmp1 = DAG.getNode(ISD::AND, dl, MVT::i32, tmp1, DAG.getConstant(0xFF, MVT::i32));
    SDValue Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, tmp2.getValue(1));
    SDValue Ops[] = { tmp1, Chain };
    return DAG.getMergeValues(Ops, 2, dl);
//    return DAG.getNode(ISD::ADD, dl, MVT::i32, LD->getOffset(), LD->getOffset());
  }
  llvm_unreachable("AZPRTargetLowering::LowerLOAD");
}

SDValue AZPRTargetLowering::LowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  StoreSDNode *ST = cast<StoreSDNode>(Op);
  DebugLoc dl = Op.getDebugLoc();
  SDValue Chain = ST->getChain();
  SDValue BasePtr = ST->getBasePtr();
  SDValue Value = ST->getValue();
  if(ST->getMemoryVT() != MVT::i8){
    llvm_unreachable("AZPRTargetLowering::LowerSTORE");
  }
//base & 3
  SDValue ShiftVal = DAG.getNode(ISD::AND, dl, MVT::i32, BasePtr, DAG.getConstant(3, MVT::i32));
//(base & 3) << 3
  ShiftVal = DAG.getNode(ISD::SHL, dl, MVT::i32, ShiftVal, DAG.getConstant(3, MVT::i32));
//24 - ((base & 3) << 3)
  ShiftVal = DAG.getNode(ISD::SUB, dl, MVT::i32, DAG.getConstant(24, MVT::i32), ShiftVal);
//base & (~3)
  SDValue addr_a = DAG.getNode(ISD::AND, dl, MVT::i32, BasePtr, DAG.getConstant(~3, MVT::i32));

  SDValue Load = DAG.getLoad(getPointerTy(), dl, Chain,
                       addr_a, MachinePointerInfo(),
                       false, false, false, 0);
  SDValue Mask = DAG.getNode(ISD::SHL, dl, MVT::i32, DAG.getConstant(0xff, MVT::i32), ShiftVal);
  Mask = DAG.getNOT(dl, Mask, MVT::i32);
  SDValue ValL = DAG.getNode(ISD::AND, dl, MVT::i32, Load, Mask);
  SDValue Value_ZExt32 = DAG.getZExtOrTrunc(Value, dl, MVT::i32);
  SDValue ValR = DAG.getNode(ISD::AND, dl, MVT::i32, Value_ZExt32, DAG.getConstant(0xff, MVT::i32));
  ValR = DAG.getNode(ISD::SHL, dl, MVT::i32, ValR, ShiftVal);
  SDValue Val = DAG.getNode(ISD::OR, dl, MVT::i32, ValL, ValR);

  return DAG.getStore(Load.getValue(1), dl, Val, addr_a, ST->getPointerInfo(),
                            ST->isVolatile(), ST->isNonTemporal(), 4);

}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "AZPRGenCallingConv.inc"

SDValue AZPRTargetLowering::
LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                     bool isVarArg,
                     const SmallVectorImpl<ISD::InputArg> &Ins,
                     DebugLoc dl, SelectionDAG &DAG,
                     SmallVectorImpl<SDValue> &InVals) const {
  DEBUG(dbgs() << ">> AZPRTargetLowering::LowerFormalArguments <<\n");
  DEBUG(dbgs() << "  Chain: ";  Chain->dumpr(););

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_AZPR);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // 引数がレジスタで渡された場合
      EVT RegVT = VA.getLocVT();
      const TargetRegisterClass *RC = &AZPR::CPUGRegsRegClass;//引数はGRegしか使えない

      DEBUG(dbgs() << "  Reg N" << i 
            << " LowerFormalArguments Unhandled argument type: "
            << RegVT.getSimpleVT().SimpleTy << "\n";);
      if (VA.getLocInfo() != CCValAssign::Full) {
        llvm_unreachable("not supported yet");
      }

      unsigned VReg = RegInfo.createVirtualRegister(RC);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);
      InVals.push_back(ArgValue);
    } else { // VA.isRegLoc()
      // 引数がメモリで渡された場合
 
      // Sanity check
      assert(VA.isMemLoc());
      // Load the argument to a virtual register
      unsigned ObjSize = VA.getLocVT().getSizeInBits()/8;
      DEBUG(dbgs() << "  Mem N" << i
            << " LowerFormalArguments Unhandled argument type: "
            << EVT(VA.getLocVT()).getEVTString()
            << "\n";);

      // フレームインデックスを作成する
      int FI = MFI->CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

      // スタックから引数を取得するためにloadノードを作成する
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      InVals.push_back(DAG.getLoad(VA.getLocVT(), dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(FI),
                                   false, false, false, 0));
    }
  }

  DEBUG(
      for (SmallVectorImpl<SDValue>::const_iterator i = InVals.begin();
           i != InVals.end(); ++i) {
        dbgs() << "  InVals: "; i->getNode()->dump();
      });
    
  DEBUG(dbgs() << ">> done LowerFormalArguments <<\n";);
  return Chain;
}

//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: isTailCall.
SDValue AZPRTargetLowering::
LowerCall(CallLoweringInfo &CLI,
          SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  DebugLoc &dl                          = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue InChain                       = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  DEBUG(dbgs() << ">> AZPRTargetLowering::LowerCall <<\n");
  DEBUG(dbgs() << "  InChain: "; InChain->dumpr(););
  DEBUG(dbgs() << "  Callee: "; Callee->dumpr(););

  // 末尾呼び出しは未対応
  isTailCall = false;
  
  // 関数のオペランドを解析してオペランドをレジスタに割り当てる
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_AZPR);

  // スタックを何Byte使っているか取得
  unsigned NumBytes = CCInfo.getNextStackOffset();
  DEBUG(dbgs() << "  stack offset: " << NumBytes << "\n");

  // 関数呼び出し開始のNode
  InChain = DAG.getCALLSEQ_START(InChain ,
                                 DAG.getConstant(NumBytes, getPointerTy(), true));

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SDValue StackPtr;

  // 引数をRegsToPassに追加していく
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    SDValue Arg = OutVals[i];
    CCValAssign &VA = ArgLocs[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;
    DEBUG(dbgs() << "  Arg: "; Arg->dumpr());

    // 引数が数値
    if (Flags.isByVal()) {
      assert(Flags.getByValSize() &&
             "ByVal args of size 0 should have been ignored by front-end.");
      llvm_unreachable("ByVal arguments are not supported");
      continue;
    }

    // 必要に応じてPromoteする
    switch (VA.getLocInfo()) {
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

    // レジスタ経由の引数はRegsToPassに追加
    if (VA.isRegLoc()) {
      DEBUG(dbgs() << "    Reg: " << VA.getLocReg() << "\n");
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());
      llvm_unreachable("MemLoc arguments are not supported");
    }
  }

  // レジスタをコピーするノードを作成
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    InChain = DAG.getCopyToReg(InChain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = InChain.getValue(1);
  }

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i32);
    DEBUG(dbgs() << "  Global: " << Callee.getNode() << "\n");
  } else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);
    DEBUG(dbgs() << "  External: " << Callee.getNode() << "\n");
  }

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(InChain);
  Ops.push_back(Callee);

  // 引数のレジスタをリストに追加
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));
  }

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  InChain = DAG.getNode(AZPRISD::Call, dl, NodeTys, &Ops[0], Ops.size());
  InFlag = InChain.getValue(1);

  // 関数呼び出し終了のNode
  InChain = DAG.getCALLSEQ_END(InChain,
                               DAG.getConstant(NumBytes, getPointerTy(), true),
                               DAG.getConstant(0, getPointerTy(), true),
                               InFlag);
  InFlag = InChain.getValue(1);

  // 戻り値の処理
  return LowerCallResult(InChain, InFlag, CallConv, isVarArg,
                         Ins, dl, DAG, InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue AZPRTargetLowering::
LowerCallResult(SDValue Chain, SDValue InFlag,
                CallingConv::ID CallConv, bool isVarArg,
                const SmallVectorImpl<ISD::InputArg> &Ins,
                DebugLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals) const {
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_AZPR);

  // 結果レジスタをコピー
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue AZPRTargetLowering::
LowerReturn(SDValue Chain,
            CallingConv::ID CallConv, bool isVarArg,
            const SmallVectorImpl<ISD::OutputArg> &Outs,
            const SmallVectorImpl<SDValue> &OutVals,
            DebugLoc dl, SelectionDAG &DAG) const {
  DEBUG(dbgs() << ">> AZPRTargetLowering::LowerReturn <<\n");
  DEBUG(dbgs() << " Chain: "; Chain->dumpr(););

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
		 getTargetMachine(), RVLocs, *DAG.getContext());

  // 戻り値を解析する
  CCInfo.AnalyzeReturn(Outs, RetCC_AZPR);

  // この関数で最初の戻り値の場合はレジスタをliveoutに追加
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
    for (unsigned i = 0; i != RVLocs.size(); ++i)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }

  SDValue Flag;

  // 戻り値をレジスタにコピーするノードを作成
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
                             OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
  }

  DEBUG(
      for (SmallVectorImpl<SDValue>::const_iterator i = OutVals.begin();
           i != OutVals.end(); ++i) {
        dbgs() << "  OutVals: "; i->getNode()->dump();
      });

  // 常に "ret $ra" を生成
  if (Flag.getNode())
    return DAG.getNode(AZPRISD::Ret, dl, MVT::Other,
                       Chain, DAG.getRegister(AZPR::r31, MVT::i32), Flag);
  else // Return Void
    return DAG.getNode(AZPRISD::Ret, dl, MVT::Other,
                       Chain, DAG.getRegister(AZPR::r31, MVT::i32));
}

EVT AZPRTargetLowering::
getSetCCResultType(EVT VT) const {
  return MVT::i32;
}

/// Given a register class constraint, like 'r', if this corresponds directly
/// to an LLVM register class, return a register of 0 and the register class
/// pointer.
std::pair<unsigned, const TargetRegisterClass*> AZPRTargetLowering::
getRegForInlineAsmConstraint(const std::string &Constraint, EVT VT) const
{
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
        return std::make_pair(0U, &AZPR::CPUGRegsRegClass);
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}


MachineBasicBlock *
AZPRTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                               MachineBasicBlock *BB) const {
  const TargetInstrInfo *TII = getTargetMachine().getInstrInfo();

  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator It = BB;
  ++It;
  DebugLoc dl = MI->getDebugLoc();

  MachineFunction *F = BB->getParent();

  MachineBasicBlock *thisMBB = BB;
  MachineBasicBlock *falseMBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *exitMBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(It, falseMBB);
  F->insert(It, exitMBB);

  exitMBB->splice(exitMBB->begin(), BB,
                  llvm::next(MachineBasicBlock::iterator(MI)),
                  BB->end());
  exitMBB->transferSuccessorsAndUpdatePHIs(BB);

  unsigned int Opc, LhsReg = MI->getOperand(1).getReg(), RhsReg = MI->getOperand(2).getReg(), TrueReg = MI->getOperand(3).getReg(), FalseReg = MI->getOperand(4).getReg();
  switch(MI->getOperand(5).getImm()){
  case ISD::SETEQ:
    Opc = AZPR::BE;
    break;
  case ISD::SETNE:
    Opc = AZPR::BNE;
    break;
  case ISD::SETUGT:
    Opc = AZPR::BUGT;
    break;
  case ISD::SETGT:
    Opc = AZPR::BSGT;
    break;
// a >= b ? c : d
//-> b > a ? d : c
  case ISD::SETUGE:
    Opc = AZPR::BUGT;
    LhsReg = MI->getOperand(2).getReg();
    RhsReg = MI->getOperand(1).getReg();
    TrueReg = MI->getOperand(4).getReg();
    FalseReg = MI->getOperand(3).getReg();
    break;
  case ISD::SETGE:
    Opc = AZPR::BSGT;
    LhsReg = MI->getOperand(2).getReg();
    RhsReg = MI->getOperand(1).getReg();
    TrueReg = MI->getOperand(4).getReg();
    FalseReg = MI->getOperand(3).getReg();
    break;
//a <= b ? c : d
//-> a > b ? d : c
  case ISD::SETLE:
    Opc = AZPR::BSGT;
    TrueReg = MI->getOperand(4).getReg();
    FalseReg = MI->getOperand(3).getReg();
    break;
  case ISD::SETULE:
    Opc = AZPR::BUGT;
    TrueReg = MI->getOperand(4).getReg();
    FalseReg = MI->getOperand(3).getReg();
    break;
// a < b ? c : d
//-> b > a ? c : d
  case ISD::SETULT:
    Opc = AZPR::BUGT;
    LhsReg = MI->getOperand(2).getReg();
    RhsReg = MI->getOperand(1).getReg();
    break;
  case ISD::SETLT:
    Opc = AZPR::BSGT;
    LhsReg = MI->getOperand(2).getReg();
    RhsReg = MI->getOperand(1).getReg();
    break;
  default:
    llvm_unreachable("Unsupported Condition");
    break;
  }

  BuildMI(BB, dl, TII->get(Opc))
    .addReg(RhsReg)
    .addReg(LhsReg)
    .addMBB(exitMBB);

  BB->addSuccessor(falseMBB);
  BB->addSuccessor(exitMBB);

  falseMBB->addSuccessor(exitMBB);

  BuildMI(*exitMBB, exitMBB->begin(), dl,
          TII->get(AZPR::PHI), MI->getOperand(0).getReg())
    .addReg(FalseReg).addMBB(falseMBB)
    .addReg(TrueReg).addMBB(thisMBB);

  MI->eraseFromParent();
  return exitMBB;
}
