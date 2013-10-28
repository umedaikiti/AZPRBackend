//===-- AZPRFrameLowering.cpp - AZPR Frame Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the AZPR implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "AZPRFrameLowering.h"
#include "AZPRInstrInfo.h"
#include "AZPRMachineFunction.h"
#include "MCTargetDesc/AZPRMCTargetDesc.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

bool AZPRFrameLowering::
hasFP(const MachineFunction &MF) const {
  return false;
}

void AZPRFrameLowering::
emitPrologue(MachineFunction &MF) const {
  DEBUG(dbgs() << ">> AZPRFrameLowering::emitPrologue <<\n");

  MachineBasicBlock &MBB   = MF.front();
  MachineFrameInfo *MFI = MF.getFrameInfo();

  const AZPRInstrInfo &TII =
    *static_cast<const AZPRInstrInfo*>(MF.getTarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
/*
  BuildMI(MBB, MBBI, dl, TII.get(AZPR::XORR), AZPR::r0)
      .addReg(AZPR::r0)
      .addReg(AZPR::r0);
*/

  // allocate fixed size for simplicity
  uint64_t StackSize = 4 * 16;

   // Update stack size
  MFI->setStackSize(StackSize);

  BuildMI(MBB, MBBI, dl, TII.get(AZPR::ADDUI), AZPR::r30)
      .addReg(AZPR::r30)
      .addImm(-StackSize);

}

void AZPRFrameLowering::
emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
  DEBUG(dbgs() << ">> AZPRFrameLowering::emitEpilogue <<\n");

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo *MFI            = MF.getFrameInfo();
  const AZPRInstrInfo &TII =
    *static_cast<const AZPRInstrInfo*>(MF.getTarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();

  // Get the number of bytes from FrameInfo
  uint64_t StackSize = MFI->getStackSize();

  // Adjust stack.
  BuildMI(MBB, MBBI, dl, TII.get(AZPR::ADDUI), AZPR::r30)
      .addReg(AZPR::r30)
      .addImm(StackSize);

}
