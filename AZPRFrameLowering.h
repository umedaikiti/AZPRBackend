//===-- AZPRFrameLowering.h - Define frame lowering for AZPR ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef SAMPLE_FRAMELOWERING_H
#define SAMPLE_FRAMELOWERING_H

#include "AZPR.h"
#include "AZPRSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class AZPRSubtarget;

class AZPRFrameLowering : public TargetFrameLowering {
 protected:
  const AZPRSubtarget &STI;

 public:
  explicit AZPRFrameLowering(const AZPRSubtarget &sti)
      : TargetFrameLowering(StackGrowsDown, 8, 0), STI(sti) {
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const /*override*/;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const /*override*/;
  bool hasFP(const MachineFunction &MF) const /*override*/;
};
} // End llvm namespace

#endif
