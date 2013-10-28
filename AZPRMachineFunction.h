//===-- AZPRMachineFunctionInfo.h - Private data used for AZPR ----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the AZPR specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef SAMPLE_MACHINE_FUNCTION_INFO_H
#define SAMPLE_MACHINE_FUNCTION_INFO_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include <utility>

namespace llvm {

/// AZPRFunctionInfo - This class is derived from MachineFunction private
/// AZPR target-specific information for each MachineFunction.
class AZPRMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

public:
  AZPRMachineFunctionInfo(MachineFunction& MF) {}
};
} // end of namespace llvm

#endif // SAMPLE_MACHINE_FUNCTION_INFO_H
