//===-- AZPRSelectionDAGInfo.cpp - AZPR SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AZPRSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "azpr-selectiondag-info"
#include "AZPRTargetMachine.h"
using namespace llvm;

AZPRSelectionDAGInfo::AZPRSelectionDAGInfo(const AZPRTargetMachine &TM)
    : TargetSelectionDAGInfo(TM) {}

AZPRSelectionDAGInfo::~AZPRSelectionDAGInfo() {}
