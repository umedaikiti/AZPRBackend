//===-- AZPRTargetInfo.cpp - AZPR Target Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AZPR.h"
#include "llvm/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheAZPRTarget;

extern "C" void LLVMInitializeAZPRTargetInfo() { 
  DEBUG(dbgs() << ">> InitAZPRTargetInfo <<\n");
  RegisterTarget<Triple::azpr, /*HasJIT=*/false>
    X(TheAZPRTarget, "azpr", "AZPR");
}
