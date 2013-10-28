//===-- AZPRSubtarget.cpp - AZPR Subtarget Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AZPR specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "AZPRSubtarget.h"
#include "AZPR.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "AZPRGenSubtargetInfo.inc"

using namespace llvm;

AZPRSubtarget::AZPRSubtarget(const std::string &TT,
                                 const std::string &CPU,
                                 const std::string &FS)
    : AZPRGenSubtargetInfo(TT, CPU, FS) {
  std::string CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);
}
