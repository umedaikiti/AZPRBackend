//===-- AZPRELFObjectWriter.cpp - AZPR ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/AZPRBaseInfo.h"
#include "MCTargetDesc/AZPRFixupKinds.h"
#include "MCTargetDesc/AZPRMCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>

using namespace llvm;

namespace {
struct RelEntry {
  RelEntry(const ELFRelocationEntry &R, const MCSymbol *S, int64_t O) :
      Reloc(R), Sym(S), Offset(O) {}
  ELFRelocationEntry Reloc;
  const MCSymbol *Sym;
  int64_t Offset;
};

typedef std::list<RelEntry> RelLs;
typedef RelLs::iterator RelLsIter;

class AZPRELFObjectWriter : public MCELFObjectTargetWriter {
 public:
  AZPRELFObjectWriter(uint8_t OSABI);
  virtual ~AZPRELFObjectWriter();

  virtual unsigned getEFlags() const;

  // オブジェクトを生成するときやリンク時にアドレス解決するために
  // ELFObjectWriterなどから参照される
  virtual unsigned GetRelocType(const MCValue &Target, const MCFixup &Fixup,
                                bool IsPCRel, bool IsRelocWithSymbol,
                                int64_t Addend) const;
};
}

AZPRELFObjectWriter::
AZPRELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*_is64Bit*/ false, OSABI, /*ELF::EM_NONE*/ELF::EM_MIPS,
                              /*HasRelocationAddend*/ false) {}

AZPRELFObjectWriter::~AZPRELFObjectWriter() {}

unsigned AZPRELFObjectWriter::getEFlags() const {
  return ELF::EF_MIPS_NOREORDER | ELF::EF_MIPS_ARCH_32R2;
}

unsigned AZPRELFObjectWriter::
GetRelocType(const MCValue &Target,
             const MCFixup &Fixup,
             bool IsPCRel,
             bool IsRelocWithSymbol,
             int64_t Addend) const {
  // determine the type of the relocation
  unsigned Type = (unsigned)ELF::R_MIPS_NONE;
  unsigned Kind = (unsigned)Fixup.getKind();

  switch (Kind) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case AZPR::fixup_AZPR_HI16:
    Type = ELF::R_MIPS_HI16;
    break;
  case AZPR::fixup_AZPR_LO16:
    Type = ELF::R_MIPS_LO16;
    break;
  case AZPR::fixup_AZPR_PC16:
    Type = ELF::R_MIPS_PC16;
    break;
  case FK_Data_4://関数のアドレスを配列に保存するため
    Type = ELF::R_MIPS_32;
    break;
  }

  return Type;
}

MCObjectWriter *llvm::createAZPRELFObjectWriter(raw_ostream &OS,
                                                  uint8_t OSABI) {
  MCELFObjectTargetWriter *MOTW = new AZPRELFObjectWriter(OSABI);
  return createELFObjectWriter(MOTW, OS, /*isLittleEndian*/ false);
}
