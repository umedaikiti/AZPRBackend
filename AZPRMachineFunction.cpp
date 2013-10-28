//===-- AZPRMachineFunctionInfo.cpp - Private data used for AZPR ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AZPRMachineFunction.h"
#include "AZPRInstrInfo.h"
#include "MCTargetDesc/AZPRMCTargetDesc.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

void AZPRMachineFunctionInfo::anchor() { }
