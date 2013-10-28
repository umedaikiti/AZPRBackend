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
#ifndef SAMPLE_TARGETMACHINE_H
#define SAMPLE_TARGETMACHINE_H

#include "AZPRFrameLowering.h"
#include "AZPRInstrInfo.h"
#include "AZPRISelLowering.h"
#include "AZPRSelectionDAGInfo.h"
#include "AZPRRegisterInfo.h"
#include "AZPRSubtarget.h"
#include "llvm/DataLayout.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Support/Debug.h"

namespace llvm {

class Module;

class AZPRTargetMachine : public LLVMTargetMachine {
  const DataLayout DL;
  AZPRSubtarget Subtarget;
  AZPRInstrInfo InstrInfo;
  AZPRFrameLowering FrameLowering;
  AZPRTargetLowering TLInfo;
  AZPRSelectionDAGInfo TSInfo;

 public:
  AZPRTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);

  virtual const AZPRInstrInfo *getInstrInfo() const {
    return &InstrInfo;
  }
  virtual const AZPRSubtarget *getSubtargetImpl() const {
    return &Subtarget;
  }
  virtual const AZPRRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }
  virtual const DataLayout *getDataLayout() const {
    return &DL;
  }
  virtual const AZPRTargetLowering *getTargetLowering() const {
    return &TLInfo;
  }
  virtual const AZPRFrameLowering *getFrameLowering() const{
    return &FrameLowering;
  }
  virtual const AZPRSelectionDAGInfo* getSelectionDAGInfo() const {
    return &TSInfo;
  }

  // Pass Pipeline Configuration
  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
};
} // end namespace llvm

#endif
