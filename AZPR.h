//===-- AZPR.h - Top-level interface for AZPR representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM AZPR back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_SAMPLE_H
#define TARGET_SAMPLE_H

#include "MCTargetDesc/AZPRMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"

namespace llvm {
  class AZPRTargetMachine;
  class FunctionPass;

  FunctionPass *createAZPRISelDag(AZPRTargetMachine &TM);
  FunctionPass *createAZPRDelaySlotFillerPass(TargetMachine &tm);
} // end namespace llvm;

#endif
