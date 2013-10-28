//===-- AZPRAsmPrinter.cpp - AZPR LLVM assembly writer ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the AZPR assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "AZPR.h"
#include "AZPRInstrInfo.h"
#include "AZPRMCInstLower.h"
#include "AZPRTargetMachine.h"
#include "InstPrinter/AZPRInstPrinter.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace {
class AZPRAsmPrinter : public AsmPrinter {
 public:
  AZPRAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {}

  virtual const char *getPassName() const {
    return "AZPR Assembly Printer";
  }

  // should overwrite functions
  void EmitInstruction(const MachineInstr *MI) /*override*/;

  virtual bool isBlockOnlyReachableByFallthrough(const MachineBasicBlock*
                                                 MBB) const;

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       unsigned AsmVariant, const char *ExtraCode,
                       raw_ostream &O);
};
} // end of anonymous namespace

void AZPRAsmPrinter::
EmitInstruction(const MachineInstr *MI) {
  DEBUG(dbgs() << ">> AZPRAsmPinter::EmitInstruction <<\n");
  DEBUG(MI->dump());
  AZPRMCInstLower MCInstLowering(OutContext, *Mang, *this);
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  OutStreamer.EmitInstruction(TmpInst);
}

bool AZPRAsmPrinter::isBlockOnlyReachableByFallthrough(const MachineBasicBlock*
                                                       MBB) const {
  // The predecessor has to be immediately before this block.
  const MachineBasicBlock *Pred = *MBB->pred_begin();

  // If there isn't exactly one predecessor, it can't be a fall through.
  MachineBasicBlock::const_pred_iterator PI = MBB->pred_begin(), PI2 = PI;
  ++PI2;

  if (PI2 != MBB->pred_end())
    return false;

  // The predecessor has to be immediately before this block.
  if (!Pred->isLayoutSuccessor(MBB))
    return false;

  // If the block is completely empty, then it definitely does fall through.
  if (Pred->empty())
    return true;

  // Otherwise, check the last instruction.
  // Check if the last terminator is an unconditional branch.
  MachineBasicBlock::const_iterator I = Pred->end();
  while (I != Pred->begin() && !(--I)->isTerminator()) ;

  return !(I->isBarrier() || (I->getOpcode() == AZPR::BE && I->getOperand(0).getReg() == I->getOperand(1).getReg()));//?
}

// Print out an operand for an inline asm expression.
bool AZPRAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNum,
                                     unsigned AsmVariant,const char *ExtraCode,
                                     raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
    case MachineOperand::MO_Register:
      O << '$'
        << StringRef(AZPRInstPrinter::getRegisterName(MO.getReg())).lower();
      return false;
  }
  return true;
}

// Force static initialization.
extern "C" void LLVMInitializeAZPRAsmPrinter() {
  RegisterAsmPrinter<AZPRAsmPrinter> X(TheAZPRTarget);
}
