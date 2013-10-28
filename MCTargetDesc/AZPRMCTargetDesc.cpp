//===-- AZPRMCTargetDesc.cpp - AZPR Target Descriptions -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides AZPR specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "AZPRMCAsmInfo.h"
#include "AZPRMCTargetDesc.h"
#include "InstPrinter/AZPRInstPrinter.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "AZPRGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "AZPRGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "AZPRGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createAZPRMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitAZPRMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createAZPRMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitAZPRMCRegisterInfo(X, AZPR::r31);
  return X;
}

static MCSubtargetInfo *createAZPRMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                  StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitAZPRMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCAsmInfo *createAZPRMCAsmInfo(const Target &T, StringRef TT) {
  MCAsmInfo *MAI = new AZPRMCAsmInfo(T, TT);

  MachineLocation Dst(MachineLocation::VirtualFP);
  MachineLocation Src(AZPR::r30, 0);
  MAI->addInitialFrameState(0, Dst, Src);

  return MAI;
}

static MCCodeGenInfo *createAZPRMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                              CodeModel::Model CM,
                                              CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createAZPRMCInstPrinter(const Target &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI,
                                              const MCSubtargetInfo &STI) {
  return new AZPRInstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
                                    MCContext &Ctx, MCAsmBackend &MAB,
                                    raw_ostream &_OS,
                                    MCCodeEmitter *_Emitter,
                                    bool RelaxAll,
                                    bool NoExecStack) {
  Triple TheTriple(TT);

  return createELFStreamer(Ctx, MAB, _OS, _Emitter, RelaxAll, NoExecStack);
}

extern "C" void LLVMInitializeAZPRTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheAZPRTarget, createAZPRMCAsmInfo);
  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheAZPRTarget,
                                        createAZPRMCCodeGenInfo);
  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheAZPRTarget, createAZPRMCInstrInfo);
  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheAZPRTarget, createAZPRMCRegisterInfo);
  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(TheAZPRTarget,
                                        createAZPRMCCodeEmitter);
  // Register the object streamer.
  TargetRegistry::RegisterMCObjectStreamer(TheAZPRTarget, createMCStreamer);
  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(TheAZPRTarget,
                                       createAZPRAsmBackend);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheAZPRTarget,
                                          createAZPRMCSubtargetInfo);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheAZPRTarget,
                                        createAZPRMCInstPrinter);
}

