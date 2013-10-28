//===-- AZPRSelectionDAGInfo.h - AZPR SelectionDAG Info ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the AZPR subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef SAMPLE_SELECTIONDAGINFO_H
#define SAMPLE_SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class AZPRTargetMachine;

class AZPRSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit AZPRSelectionDAGInfo(const AZPRTargetMachine &TM);
  ~AZPRSelectionDAGInfo();
};
} // end of namespace llvm

#endif
