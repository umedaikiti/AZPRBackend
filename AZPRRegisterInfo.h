//===-- AZPRRegisterInfo.h - AZPR Register Information Impl -----*- C++ -*-===//
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

#ifndef SAMPLEREGISTERINFO_H
#define SAMPLEREGISTERINFO_H

#include "AZPR.h"
#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "AZPRGenRegisterInfo.inc"

namespace llvm {
class AZPRSubtarget;
class TargetInstrInfo;
class Type;

struct AZPRRegisterInfo : public AZPRGenRegisterInfo {
  const TargetInstrInfo &TII;

  AZPRRegisterInfo(const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction* MF = 0) const /*override*/;
  const uint32_t *getCallPreservedMask(CallingConv::ID) const /*override*/;

  BitVector getReservedRegs(const MachineFunction &MF) const /*override*/;

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const /*override*/;

  /// Stack Frame Processing Methods
  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, RegScavenger *RS = NULL) const;

  /// Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const /*override*/;
};

} // end namespace llvm


#endif

