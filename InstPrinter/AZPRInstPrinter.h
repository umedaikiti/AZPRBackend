//=== AZPRInstPrinter.h - Convert AZPR MCInst to assembly syntax -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints a AZPR MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef SAMPLE_INSTPRINTER_H
#define SAMPLE_INSTPRINTER_H
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/Support/Debug.h"

namespace llvm {
// These enumeration declarations were orignally in AZPRInstrInfo.h but
// had to be moved here to avoid circular dependencies between
// LLVMAZPRCodeGen and LLVMAZPRAsmPrinter.
class TargetMachine;

class AZPRInstPrinter : public MCInstPrinter {
 public:
  AZPRInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                  const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

  // must implement
  virtual void printRegName(raw_ostream &OS, unsigned RegNo) const /*override*/;
  virtual void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot) /*override*/;
  static const char *getRegisterName(unsigned RegNo);

private:
  // Autogenerated by tblgen.
  void printInstruction(const MCInst *MI, raw_ostream &O);

  // used in printInstruction
  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
  void printMemOperand(const MCInst *MI, int opNum, raw_ostream &O);
  void printAbsAddrHI16(const MCInst *MI, int opNum, raw_ostream &O);
  void printAbsAddrLO16(const MCInst *MI, int opNum, raw_ostream &O);
};
} // end namespace llvm

#endif
