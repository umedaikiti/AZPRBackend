//===-- AZPRMCCodeEmitter.cpp - Convert AZPR Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AZPRMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//
#define DEBUG_TYPE "mccodeemitter"
#include "MCTargetDesc/AZPRBaseInfo.h"
#include "MCTargetDesc/AZPRFixupKinds.h"
#include "MCTargetDesc/AZPRMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class AZPRMCCodeEmitter : public MCCodeEmitter {
  AZPRMCCodeEmitter(const AZPRMCCodeEmitter &); // DO NOT IMPLEMENT
  void operator=(const AZPRMCCodeEmitter &); // DO NOT IMPLEMENT
  const MCInstrInfo &MCII;
  const MCSubtargetInfo &STI;
  MCContext &Ctx;

 public:
  AZPRMCCodeEmitter(const MCInstrInfo &mcii, const MCSubtargetInfo &sti,
                    MCContext &ctx) :
            MCII(mcii), STI(sti) , Ctx(ctx) {}

  ~AZPRMCCodeEmitter() {}

  // EncodeInstruction - AsmStreamerから実行される
  // 命令をバイナリにして出力する
  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const;

 private:
  // getBinaryCodeForInstr - TableGenが自動生成
  // 命令のバイナリエンコーディングを取得
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

  // getMachineOpValue - TableGenの中から必ず参照される
  // オペランドのバイナリエンコーディングを取得
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups) const;

  // getMemEncoding - TableGenのDecoderMethodで指定
  // load/storeのオペランドのバイナリエンコーディングを取得
  unsigned getMemEncoding(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups) const;

  // getMoveTargetOpValue - TableGenのDecoderMethodで指定
  // moveのオペランドのバイナリエンコーディングを取得
  unsigned getMoveTargetOpValue(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getShiftImmValue(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;
  unsigned getLogicImmValue(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;
  unsigned getArithImmValue(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getBrCondTargetOpValue(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;

  // call命令のオペランドのバイナリエンコーディングを取得
  unsigned getCallTargetOpValue(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getAbsAddrHI16(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getAbsAddrLO16(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups) const;

}; // class AZPRMCCodeEmitter
}  // namespace

MCCodeEmitter *llvm::createAZPRMCCodeEmitter(const MCInstrInfo &MCII,
                                               const MCRegisterInfo &MRI,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx)
{
  return new AZPRMCCodeEmitter(MCII, STI, Ctx);
}

/// EncodeInstruction - Emit the instruction.
void AZPRMCCodeEmitter::
EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                  SmallVectorImpl<MCFixup> &Fixups) const
{
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups);

  // For now all instructions are 4 bytes
  int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

  for (int i = Size - 1; i >= 0; --i) {
    unsigned Shift = i * 8;
    OS << char((Binary >> Shift) & 0xff);
  }
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned AZPRMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = getAZPRRegisterNumbering(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isFPImm()) {
    return static_cast<unsigned>(APFloat(MO.getFPImm())
        .bitcastToAPInt().getHiBits(32).getLimitedValue());
  } 

  // MO must be an Expr.
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  assert (Kind == MCExpr::SymbolRef);

  AZPR::Fixups FixupKind = AZPR::Fixups(0);

//デバッグ用。使わない。
  MCSymbolRefExpr::VariantKind vk = cast<MCSymbolRefExpr>(Expr)->getKind();

  switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
  case MCSymbolRefExpr::VK_Mips_ABS_HI:
    FixupKind = AZPR::fixup_AZPR_HI16;
    break;
  case MCSymbolRefExpr::VK_Mips_ABS_LO:
    FixupKind = AZPR::fixup_AZPR_LO16;
    break;
  case MCSymbolRefExpr::VK_Mips_LO16 + 100:
    FixupKind = AZPR::fixup_AZPR_PC16;
    break;
  default:
    llvm_unreachable("Unknown VariantKind");
  } // switch

  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));

  // All of the information is in the fixup.
  return 0;
/*
  llvm_unreachable("not implemented");
  return 0;*/
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
/*unsigned AZPRMCCodeEmitter::
getMemEncoding(const MCInst &MI, unsigned OpNo,
               SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo).isImm());
  unsigned value = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups);
  return value & 0xFFFF;
}*/
unsigned AZPRMCCodeEmitter::
getMemEncoding(const MCInst &MI, unsigned OpNo,
               SmallVectorImpl<MCFixup> &Fixups) const {
  //llvm_unreachable("not implemented");
  // Base register is encoded in bits 19-16, offset is encoded in bits 15-0.
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups) << 21;
  unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups);

  return (OffBits & 0xFFFF) | RegBits;
}

/// getMoveTargetOpValue - Return binary encoding of the move
/// target operand.
unsigned AZPRMCCodeEmitter::
getMoveTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo).isImm());
  unsigned value = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups);
  return value & 0xFFFF;
}

unsigned AZPRMCCodeEmitter::
getShiftImmValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo).isImm());
  unsigned value = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups);
  return value & 0x1F;
}

unsigned AZPRMCCodeEmitter::
getLogicImmValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo).isImm() || MI.getOperand(OpNo).isExpr());//high, lowのため
  unsigned value = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups);
  return value & 0xFFFF;
}

unsigned AZPRMCCodeEmitter::
getArithImmValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {
  assert(MI.getOperand(OpNo).isImm());
  int value = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups);
  return (value << (sizeof(int) * 8 - 16)) >> (sizeof(int) * 8 - 16);
}

unsigned AZPRMCCodeEmitter::
getBrCondTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm()) {DEBUG(dbgs() << "getBrCondTargetOpValue: Immediate\n");return MO.getImm();}

  assert(MO.isExpr() && "getBrCondTargetOpValue expects only expressions or immediates");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(AZPR::fixup_AZPR_PC16)));
  return 0;
}
/*
unsigned AZPRMCCodeEmitter::
getCallTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isExpr() && "getCallTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(AZPR::fixup_AZPR_24)));
  return 0;
}
*/
/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned AZPRMCCodeEmitter::
getAbsAddrHI16(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isExpr() && "getCallTargetOpValueHI16 expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(/*Offset*/0, Expr,
                                   MCFixupKind(AZPR::fixup_AZPR_HI16)));
  return 0;
}

unsigned AZPRMCCodeEmitter::
getAbsAddrLO16(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isExpr() && "getCallTargetOpValueLO16 expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
                                   MCFixupKind(AZPR::fixup_AZPR_LO16)));
  return 0;
}

#include "AZPRGenMCCodeEmitter.inc"

