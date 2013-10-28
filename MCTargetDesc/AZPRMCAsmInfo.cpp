//===-- AZPRMCAsmInfo.cpp - AZPR asm properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the AZPRMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "AZPRMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

AZPRMCAsmInfo::AZPRMCAsmInfo(const Target &T, StringRef TT) {
  PointerSize = 4;

  PrivateGlobalPrefix = ".L";
  //WeakRefDirective ="\t.weak\t";
  PCSymbol=".";
  CommentString = ";";

  AlignmentIsInBytes = false;
  AllowNameToStartWithDigit = true;
  UsesELFSectionDirectiveForBSS = true;
}
