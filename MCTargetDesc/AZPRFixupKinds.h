//===-- AZPRFixupKinds.h - AZPR Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SAMPLE_FIXUPKINDS_H
#define LLVM_SAMPLE_FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace AZPR {
  // Although most of the current fixup types reflect a unique relocation
  // one can have multiple fixup types for a given relocation and thus need
  // to be uniquely named.
  //
  // This table *must* be in the save order of
  // MCFixupKindInfo Infos[AZPR::NumTargetFixupKinds]
  // in AZPRAsmBackend.cpp.
  //
  enum Fixups {
    fixup_AZPR_HI16 = FirstTargetFixupKind,
    fixup_AZPR_LO16,
    fixup_AZPR_PC16,
    LastTargetFixupKind,
    NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
  };
} // namespace AZPR
} // namespace llvm


#endif // LLVM_SAMPLE_SAMPLEFIXUPKINDS_H
