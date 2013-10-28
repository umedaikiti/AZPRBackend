//===-- AZPRTargetMachine.cpp - Define TargetMachine for AZPR -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about AZPR target spec.
//
//===----------------------------------------------------------------------===//

#include "AZPRTargetMachine.h"
#include "AZPR.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeAZPRTarget() {
  // Register the target.
  RegisterTargetMachine<AZPRTargetMachine> X(TheAZPRTarget);
}

AZPRTargetMachine::
AZPRTargetMachine(const Target &T, StringRef Triple,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, Triple, CPU, FS, Options, RM, CM, OL),
      DL("E-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32"),
      Subtarget(Triple, CPU, FS),
      InstrInfo(*this),
      FrameLowering(Subtarget),
      TLInfo(*this), TSInfo(*this) {}

namespace {
/// AZPR Code Generator Pass Configuration Options.
class AZPRPassConfig : public TargetPassConfig {
 public:
  AZPRPassConfig(AZPRTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  AZPRTargetMachine &getAZPRTargetMachine() const {
    return getTM<AZPRTargetMachine>();
  }

  virtual bool addInstSelector();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *AZPRTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new AZPRPassConfig(this, PM);
}

bool AZPRPassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createAZPRISelDag(getAZPRTargetMachine()));
  return false;
}

/// addPreEmitPass - This pass may be implemented by targets that want to run
/// passes immediately before machine code is emitted.  This should return
/// true if -print-machineinstrs should print out the code after the passes.
bool AZPRPassConfig::addPreEmitPass(){
  addPass(createAZPRDelaySlotFillerPass(getAZPRTargetMachine()));
  return true;
}
