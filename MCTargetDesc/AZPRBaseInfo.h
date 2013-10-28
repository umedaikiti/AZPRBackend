//===-- AZPRBaseInfo.h - Top level definitions for MIPS MC ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the AZPR target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef SAMPLEBASEINFO_H
#define SAMPLEASEINFO_H

//#include "AZPRFixupKinds.h"
#include "AZPRMCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

/// getAZPRRegisterNumbering - Given the enum value for some register,
/// return the number that it corresponds to.
inline static unsigned getAZPRRegisterNumbering(unsigned RegEnum)
{
  switch (RegEnum) {
  case AZPR::r0:
    return 0;
  case AZPR::r1:
    return 1;
  case AZPR::r2:
    return 2;
  case AZPR::r3:
    return 3;
  case AZPR::r4:
    return 4;
  case AZPR::r5:
    return 5;
  case AZPR::r6:
    return 6;
  case AZPR::r7:
    return 7;
  case AZPR::r8:
    return 8;
  case AZPR::r9:
    return 9;
  case AZPR::r10:
    return 10;
  case AZPR::r11:
    return 11;
  case AZPR::r12:
    return 12;
  case AZPR::r13:
    return 13;
  case AZPR::r14:
    return 14;
  case AZPR::r15:
    return 15;
  case AZPR::r16:
    return 16;
  case AZPR::r17:
    return 17;
  case AZPR::r18:
    return 18;
  case AZPR::r19:
    return 19;
  case AZPR::r20:
    return 20;
  case AZPR::r21:
    return 21;
  case AZPR::r22:
    return 22;
  case AZPR::r23:
    return 23;
  case AZPR::r24:
    return 24;
  case AZPR::r25:
    return 25;
  case AZPR::r26:
    return 26;
  case AZPR::r27:
    return 27;
  case AZPR::r28:
    return 28;
  case AZPR::r29:
    return 29;
  case AZPR::r30:
    return 30;
  case AZPR::r31:
    return 31;
  case AZPR::c0:
    return 0;
  case AZPR::c1:
    return 1;
  case AZPR::c2:
    return 2;
  case AZPR::c3:
    return 3;
  case AZPR::c4:
    return 4;
  case AZPR::c5:
    return 5;
  case AZPR::c6:
    return 6;
  case AZPR::c7:
    return 7;
  case AZPR::c8:
    return 8;
  case AZPR::c9:
    return 9;
  case AZPR::c10:
    return 10;
  case AZPR::c11:
    return 11;
  case AZPR::c12:
    return 12;
  case AZPR::c13:
    return 13;
  case AZPR::c14:
    return 14;
  case AZPR::c15:
    return 15;
  case AZPR::c16:
    return 16;
  case AZPR::c17:
    return 17;
  case AZPR::c18:
    return 18;
  case AZPR::c19:
    return 19;
  case AZPR::c20:
    return 20;
  case AZPR::c21:
    return 21;
  case AZPR::c22:
    return 22;
  case AZPR::c23:
    return 23;
  case AZPR::c24:
    return 24;
  case AZPR::c25:
    return 25;
  case AZPR::c26:
    return 26;
  case AZPR::c27:
    return 27;
  case AZPR::c28:
    return 28;
  case AZPR::c29:
    return 29;
  case AZPR::c30:
    return 30;
  case AZPR::c31:
    return 31;
  default: llvm_unreachable("Unknown register number!");
  }
}
}

#endif
