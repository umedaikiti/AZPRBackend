//===-- AZPRRegisterInfo.cpp - AZPR Register Information -== --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AZPR implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "azpr-reg-info"

#include "AZPRRegisterInfo.h"
#include "AZPR.h"
#include "llvm/Constants.h"
#include "llvm/Type.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/DebugInfo.h"

#include "MCTargetDesc/AZPRMCTargetDesc.h"

#define GET_REGINFO_TARGET_DESC
#include "AZPRGenRegisterInfo.inc"

using namespace llvm;

AZPRRegisterInfo::
AZPRRegisterInfo(const TargetInstrInfo &tii)
  : AZPRGenRegisterInfo(AZPR::r31), TII(tii) { }

//===----------------------------------------------------------------------===//
// Callee Saved Registers methods
//===----------------------------------------------------------------------===//

// 呼び出し先待避レジスタ
const uint16_t* AZPRRegisterInfo::
getCalleeSavedRegs(const MachineFunction *MF) const {
    return CSR_SingleFloatOnly_SaveList;
}

// 呼び出し元待避レジスタ
const uint32_t* AZPRRegisterInfo::
getCallPreservedMask(CallingConv::ID) const {  
    return CSR_SingleFloatOnly_RegMask;
}

BitVector AZPRRegisterInfo::
getReservedRegs(const MachineFunction &MF) const {
  static const uint16_t ReservedCPURegs[] = {
    AZPR::r0, AZPR::r30, AZPR::r31/*, AZPR::V0*/,AZPR::c0, AZPR::c1, AZPR::c2, AZPR::c3, AZPR::c4, AZPR::c5, AZPR::c6, AZPR::c7, AZPR::c8, AZPR::c9, AZPR::c10, AZPR::c11, AZPR::c12, AZPR::c13, AZPR::c14, AZPR::c15, AZPR::c16, AZPR::c17, AZPR::c18, AZPR::c19, AZPR::c20, AZPR::c21, AZPR::c22, AZPR::c23, AZPR::c24, AZPR::c25, AZPR::c26, AZPR::c27, AZPR::c28, AZPR::c29, AZPR::c30, AZPR::c31,
  };

  BitVector Reserved(getNumRegs());
  typedef TargetRegisterClass::iterator RegIter;

  for (unsigned I = 0; I < array_lengthof(ReservedCPURegs); ++I)
    Reserved.set(ReservedCPURegs[I]);

  return Reserved;
}

// ADJCALLSTACKDOWNとADJCALLSTACKUPを単純に削除する
void AZPRRegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  DEBUG(dbgs() << ">> AZPRRegisterInfo::eliminateCallFramePseudoInstr <<\n";);
  MBB.erase(I);
}

// FrameIndexをスタックポインタに置き換える
void AZPRRegisterInfo::
eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                    RegScavenger *RS) const {
  DEBUG(dbgs() << ">> AZPRRegisterInfo::eliminateFrameIndex <<\n";);

  MachineInstr &MI = *II;
  const MachineFunction &MF = *MI.getParent()->getParent();

  unsigned opIndex, imIndex;
  for (opIndex = 0; opIndex < MI.getNumOperands(); opIndex++) {
    if (MI.getOperand(opIndex).isFI()) break;
  }
  for (imIndex = opIndex; imIndex < MI.getNumOperands(); imIndex++) {
    if (MI.getOperand(imIndex).isImm()) break;
  }
  assert(opIndex < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  assert(imIndex < MI.getNumOperands() && "Instr doesn't have Immediate operand!");

  int FrameIndex = MI.getOperand(opIndex).getIndex();
  uint64_t stackSize = MF.getFrameInfo()->getStackSize();
  int64_t spOffset = MF.getFrameInfo()->getObjectOffset(FrameIndex);
  int64_t Offset = spOffset + stackSize + MI.getOperand(imIndex).getImm();
  unsigned FrameReg = AZPR::r30;

  DEBUG(errs() 
        << "\nFunction : " << MF.getFunction()->getName() << "\n"
        << "<--------->\n" << MI
        << "FrameIndex : " << FrameIndex << "\n"
        << "spOffset   : " << spOffset << "\n"
        << "stackSize  : " << stackSize << "\n"
        << "Offset     : " << Offset << "\n" << "<--------->\n");

  DEBUG(errs() << "Before:" << MI);
  MI.getOperand(opIndex).ChangeToRegister(FrameReg, false);
  MI.getOperand(imIndex).ChangeToImmediate(Offset);
  DEBUG(errs() << "After:" << MI);
}

unsigned AZPRRegisterInfo::
getFrameRegister(const MachineFunction &MF) const {
    return AZPR::r30;
}
