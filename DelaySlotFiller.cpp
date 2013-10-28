//===-- DelaySlotFiller.cpp - SPARC delay slot filler ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a simple local pass that attempts to fill delay slots with useful
// instructions. If no instructions can be moved into the delay slot, then a
// NOP is placed.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "delay-slot-filler"
#include "AZPR.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"

using namespace llvm;

STATISTIC(FilledSlots, "Number of delay slots filled");

static cl::opt<bool> DisableDelaySlotFiller(
  "disable-azpr-delay-filler",
  cl::init(false),
  cl::desc("Disable the AZPR delay slot filler."),
  cl::Hidden);

namespace {
  struct Filler : public MachineFunctionPass {
    /// Target machine description which we query for reg. names, data
    /// layout, etc.
    ///
    TargetMachine &TM;
    const TargetInstrInfo *TII;

    static char ID;
    Filler(TargetMachine &tm) 
      : MachineFunctionPass(ID), TM(tm), TII(tm.getInstrInfo()) { }

    virtual const char *getPassName() const {
      return "AZPR Delay Slot Filler";
    }

    bool runOnMachineBasicBlock(MachineBasicBlock &MBB);
    bool runOnMachineFunction(MachineFunction &F) {
      bool Changed = false;
      for (MachineFunction::iterator FI = F.begin(), FE = F.end();
           FI != FE; ++FI)
        Changed |= runOnMachineBasicBlock(*FI);
      return Changed;
    }
  };
  char Filler::ID = 0;
} // end of anonymous namespace

/// createSparcDelaySlotFillerPass - Returns a pass that fills in delay
/// slots in Sparc MachineFunctions
///
FunctionPass *llvm::createAZPRDelaySlotFillerPass(TargetMachine &tm) {
  return new Filler(tm);
}


/// runOnMachineBasicBlock - Fill in delay slots for the given basic block.
/// We assume there is only one delay slot per delayed instruction.
///
bool Filler::runOnMachineBasicBlock(MachineBasicBlock &MBB) {
  bool Changed = false;

  for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ++I)
    if (I->hasDelaySlot()) {
      MachineBasicBlock::iterator J = I;

      ++FilledSlots;
      Changed = true;

      BuildMI(MBB, ++J, I->getDebugLoc(), TII->get(AZPR::NOP));
    }
  return Changed;
}

