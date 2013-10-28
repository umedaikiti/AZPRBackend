//===-- AZPR.td - Describe the AZPR Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the AZPR target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Registers
//===----------------------------------------------------------------------===//

class AZPRGReg<bits<5> num, string n> : Register<n> {
  field bits<5> num;
  let Namespace = "AZPR";
}

class AZPRCReg<bits<5> num, string n> : Register<n> {
  field bits<5> num;
  let Namespace = "AZPR";
}

// General Purpose Registers
foreach i = {0-31} in
	def r#i : AZPRGReg<i, "r"#!cast<string>(i)>, DwarfRegNum<[i]>;

// Control Registers
foreach i = {0-31} in
	def c#i : AZPRCReg<i, "c"#!cast<string>(i)>, DwarfRegNum<[i]>;

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

def CPUGRegs : RegisterClass<"AZPR", [i32], 32, (add
r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31
)>;

def CPUCRegs : RegisterClass<"AZPR", [i32], 32, (add
c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31
)>;

//===----------------------------------------------------------------------===//
// Functional units
//===----------------------------------------------------------------------===//

def ALU     : FuncUnit;

//===----------------------------------------------------------------------===//
// Instruction Itinerary classes
//===----------------------------------------------------------------------===//
def IICAlu    : InstrItinClass;
def IICLoad   : InstrItinClass;
def IICStore  : InstrItinClass;
def IICBranch : InstrItinClass;
def IICPseudo : InstrItinClass;
def IICPrivilege : InstrItinClass;
def IICSpecial : InstrItinClass;

//===----------------------------------------------------------------------===//
// AZPR Generic instruction itineraries.
//===----------------------------------------------------------------------===//

def AZPRGenericItineraries : ProcessorItineraries<[ALU], [], [
    InstrItinData<IICAlu    , [InstrStage<1,  [ALU]>]>,
    InstrItinData<IICLoad   , [InstrStage<1,  [ALU]>]>,
    InstrItinData<IICStore  , [InstrStage<1,  [ALU]>]>,
    InstrItinData<IICBranch , [InstrStage<1,  [ALU]>]>,
    InstrItinData<IICPseudo , [InstrStage<1,  [ALU]>]>
]>;

//===----------------------------------------------------------------------===//
// AZPR Operand Definitions.
//===----------------------------------------------------------------------===//

// load/storeで利用するオペランド.16bit
// printMethod: base(offset) 形式で出力
def mem : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops CPUGRegs, i16imm);
  let EncoderMethod = "getMemEncoding";
}

// 即値ロード用のオペランド. 20bit
// 19-0: 符号付き20bit整数
// EncoderMethod: bit列から符号付き20bit整数を取得
def movetarget : Operand<i32> {
  let EncoderMethod = "getMoveTargetOpValue";
}

def calltarget  : Operand<iPTR> {
  let EncoderMethod = "getCallTargetOpValue";
}

def calltargethi16  : Operand<iPTR> {
  let EncoderMethod = "getAbsAddrHI16";
  let PrintMethod = "printAbsAddrHI16";
}

def calltargetlo16  : Operand<iPTR> {
  let EncoderMethod = "getAbsAddrLO16";
  let PrintMethod = "printAbsAddrLO16";
}

def brtargethi16 : Operand<OtherVT> {
  let EncoderMethod = "getAbsAddrHI16";
  let PrintMethod = "printAbsAddrHI16";
}

def brtargetlo16  : Operand<OtherVT> {
  let EncoderMethod = "getAbsAddrLO16";
  let PrintMethod = "printAbsAddrLO16";
}

def brcondtarget : Operand<OtherVT> {
  let EncoderMethod = "getBrCondTargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeImmS16";
}

// 符号付き16bit整数
// 定数(ISD::Constant)で16bitで表現可能なもの
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

//===----------------------------------------------------------------------===//
// AZPR Complex Pattern Definitions.
//===----------------------------------------------------------------------===//

def addr : ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;

//===----------------------------------------------------------------------===//
// AZPR Format Definitions.
//===----------------------------------------------------------------------===//

class Format<bits<3> val> {
  bits<3> Value = val;
}

def Pseudo    : Format<0>;
def FormReg0  : Format<1>;
def FormReg1  : Format<2>;
def FormReg2  : Format<3>;
def FormReg3  : Format<4>;
def FormReg2I : Format<5>;
def FormRegMem : Format<6>;

// Generic AZPR Format
class AZPRInst<dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin, Format f>
      : Instruction
{
  field bits<32> Inst;
  Format Form = f;

  bits<6> Opcode = 0;

  let Namespace = "AZPR";
  let Size = 4;
  let Inst{31-26} = Opcode;
  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString   = asmstr;
  let Pattern     = pattern;
  let Itinerary   = itin;

  bits<3> FormBits = Form.Value;

  let DecoderNamespace = "AZPR";

  field bits<32> SoftFail = 0;
}

class AZPRPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>:
  AZPRInst<outs, ins, asmstr, pattern, IICPseudo, Pseudo> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

// FormReg0
class AZPRInstFormReg0<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInst<outs, ins, asmstr, pattern, itin, FormReg0>
{
  let Opcode = op;

  let Inst{25-0} = 0;
}

// FormReg1
class AZPRInstFormReg1<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInst<outs, ins, asmstr, pattern, itin, FormReg1>
{
  bits<5> ra;

  let Opcode = op;
  let Inst{25-21} = ra;
  let Inst{20-0} = 0;
}

class AZPRInstFormReg2<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInst<outs, ins, asmstr, pattern, itin, FormReg2>
{
  bits<5> ra;
  bits<5> rb;

  let Opcode = op;
  let Inst{25-21} = ra;
  let Inst{20-16} = rb;
  let Inst{15-0} = 0;
}

// FormReg3
class AZPRInstFormReg3<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInst<outs, ins, asmstr, pattern, itin, FormReg3>
{
  bits<5> ra;
  bits<5> rb;
  bits<5> rc;

  let Opcode = op;
  let Inst{25-21} = ra;
  let Inst{20-16} = rb;
  let Inst{15-11} = rc;
  let Inst{10-0} = 0;
}

class AZPRInstFormReg2I<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInst<outs, ins, asmstr, pattern, itin, FormReg2I>
{
  bits<5> ra;
  bits<5> rb;
  bits<16> immediate;

  let Opcode = op;
  let Inst{25-21} = ra;
  let Inst{20-16} = rb;
  let Inst{15-0} = immediate;
}

class AZPRInstFormRegMem<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInst<outs, ins, asmstr, pattern, itin, FormRegMem>
{
  bits<5> rb;
  bits<26> addr;

  let Opcode = op;
  let Inst{25-21} = addr{25-21};
  let Inst{20-16} = rb;
  let Inst{15-0} = addr{15-0};
}


//===----------------------------------------------------------------------===//
// AZPR profiles and nodes
//===----------------------------------------------------------------------===//

// SDTypeProfileはSDNodeの必要条件を定義する。
// 第一引数: 結果Node数
// 第二引数: オペランドNode数
// 第三引数: 制約条件(SDTypeConstraint)

// SDTCisInt<N>: N番目のオペランドはInt型
// SDTCisVT<N, VT>: N番目のオペランドはVT型

// SDNodeは新しいSDNodeを定義する。
// 第一引数: opcode
// 第二引数: 制約条件(SDTypeProfile)
// 第三引数: SDNodeProperty

// SDNodeProperty
// SDNPCommutative : // 可換
// SDNPAssociative : // 結合法則
// SDNPHasChain    : // R/W chain operand and result
// SDNPOutGlue     : // Write a flag result
// SDNPInGlue      : // Read a flag operand
// SDNPOptInGlue   : // Optionally read a flag operand
// SDNPMayStore    : // May write to memory, sets 'mayStore'.
// SDNPMayLoad     : // May read memory, sets 'mayLoad'.
// SDNPSideEffect  : // Sets 'HasUnmodelledSideEffects'.
// SDNPMemOperand  : // Touches memory, has assoc MemOperand
// SDNPVariadic    : // 可変引数を持つ
// SDNPWantRoot    : // ComplexPattern gets the root of match
// SDNPWantParent  : // ComplexPattern gets the parent

def SDT_AZPRRet          : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def AZPRRet : SDNode<"AZPRISD::Ret", SDT_AZPRRet, 
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def SDT_AZPRCall      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;

def AZPRCall : SDNode<"AZPRISD::Call",SDT_AZPRCall,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;

def SDT_AZPRCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_AZPRCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_AZPRCallSeqStart,
                           [SDNPHasChain, SDNPSideEffect, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_AZPRCallSeqEnd,
                           [SDNPHasChain, SDNPSideEffect,
                            SDNPOptInGlue, SDNPOutGlue]>;

def AZPRHi    : SDNode<"AZPRISD::Hi", SDTIntUnaryOp>;
def AZPRLo    : SDNode<"AZPRISD::Lo", SDTIntUnaryOp>;
def AZPROr    : SDNode<"AZPRISD::Or", SDTIntBinOp>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//

// Arithmetic and logical instructions with 3 register operands.
/*class ArithLogicInst<bits<6> op, string asmstr, SDNode OpNode, InstrItinClass itin, RegisterClass RC>
  : AZPRInstFormReg3<op, (outs RC:$rc), (ins RC:$ra, RC:$rb),
                      !strconcat(asmstr, "\t$ra, $rb, $rc"),
                      [(set RC:$rc, (OpNode RC:$ra, RC:$rb))], itin> {
}*/

// Load/Store common form
// DecoderMethod: objdumpなどでバイナリから値をデコードする
class FMem<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern, InstrItinClass itin>
      : AZPRInstFormRegMem<op, outs, ins, asmstr, pattern, itin> {
  let DecoderMethod = "DecodeMem";
}

// Load
let canFoldAsLoad = 1 in
class LoadM<bits<6> op, string asmstr, RegisterClass RC, PatFrag OpNode>:
  FMem<op, (outs RC:$rb), (ins mem:$addr),
     !strconcat(asmstr, "\t$rb, $addr"),
     [(set RC:$rb, (OpNode addr:$addr))], IICLoad>;

// Store
class StoreM<bits<6> op, string asmstr, RegisterClass RC, PatFrag OpNode>:
  FMem<op, (outs), (ins RC:$rb, mem:$addr),
     !strconcat(asmstr, "\t$rb, $addr"),
     [(OpNode RC:$rb, addr:$addr)], IICStore>;

class EffectiveAddress<bits<6> op, string asmstr, RegisterClass RC> :
  FMem<op, (outs RC:$rb), (ins mem:$addr),
     !strconcat(asmstr, "\t$rb, $addr"), [(set RC:$rb, addr:$addr)], IICAlu>;

// Return
class RetInst<bits<6> op, string asmstr>:
  AZPRInstFormReg1<op, (outs), (ins CPUGRegs:$ra),
     !strconcat(asmstr, "\t$ra"), [(AZPRRet CPUGRegs:$ra)], IICBranch> {
  let isBranch=1;
  let isTerminator=1;
  let isBarrier=1;
  let isReturn=1;
  let hasDelaySlot=1;
}

// Load Immediate
// DecoderMethod: 即値ロード用のデコード
/*
class LoadI<bits<6> op, string asmstr>:
  AZPRInstFormReg2I<op, (outs CPUGRegs:$rb), (ins movetarget:$immediate),
     !strconcat(asmstr, "\tr0, $rb, $immediate"), [(set CPUGRegs:$rb, immSExt16:$immediate)], IICLoad> {
  let DecoderMethod = "DecodeMoveTarget";
  let ra = 0;
}*/
/*
class Call<bits<6> op, string asmstr>:
  AZPRInstFormReg0<op, (outs), (ins calltarget:$other, variable_ops),
     !strconcat(asmstr, "\t$other"), [(AZPRCall imm:$other)],
     IICBranch> {
  let isCall=1;
  let DecoderMethod = "DecodeCallTarget";
}
*/

class Call<bits<6> op, string asmstr>:
  AZPRInstFormReg1<op, (outs), (ins CPUGRegs:$ra, variable_ops),
     !strconcat(asmstr, "\t$ra"), [(AZPRCall CPUGRegs:$ra)],
     IICBranch> {
  let isCall=1;
//  let DecoderMethod = "DecodeCallTarget";
  let hasDelaySlot=1;
  let Defs=[r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25];//関数呼び出しの時レジスタ保存されない問題のとりあえずの処置
}

//===----------------------------------------------------------------------===//
// AZPR Instruction definition
//===----------------------------------------------------------------------===//

class AlignedLoad<PatFrag Node> :
  PatFrag<(ops node:$ptr), (Node node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  return LD->getMemoryVT().getSizeInBits()/8 <= LD->getAlignment();
}]>;

class AlignedStore<PatFrag Node> :
  PatFrag<(ops node:$val, node:$ptr), (Node node:$val, node:$ptr), [{
  StoreSDNode *SD = cast<StoreSDNode>(N);
  return SD->getMemoryVT().getSizeInBits()/8 <= SD->getAlignment();
}]>;

// Load/Store PatFrags.
def load_a          : AlignedLoad<load>;
def store_a         : AlignedStore<store>;

def LDW  : LoadM<0b010110, "ldw", CPUGRegs, load_a>;
def STW : StoreM<0b010111, "stw", CPUGRegs, store_a>;
//def MOVE  : LoadI<0b000011, "ori">;
def CALL  : Call<0b010101, "call">;
def RET   : RetInst<0b010100, "jmp">{
  let isCodeGenOnly=1;
}

def LDWSB  : LoadM<0b010110, "ldwsb", CPUGRegs, sextloadi8>{
  let isCodeGenOnly = 1;
}

def LDWB  : LoadM<0b010110, "ldwb", CPUGRegs, extloadi8>{
  let isCodeGenOnly = 1;
}

def STWB : StoreM<0b010111, "stwb", CPUGRegs, truncstorei8>{
  let isCodeGenOnly = 1;
}


def LEA   : EffectiveAddress<0b001001, "lea(addui)", CPUGRegs>{
  let isCodeGenOnly=1;
}

let Uses=[c0] in {
 def RDCR : AZPRInstFormReg2<0b011001, (outs CPUGRegs:$rb), (ins CPUCRegs:$ra), "rdcr\t$ra, $rb", [], IICPrivilege>;
 def WRCR : AZPRInstFormReg2<0b011010, (outs CPUCRegs:$rb), (ins CPUGRegs:$ra), "wrcr\t$ra, $rb", [], IICPrivilege>;
}

def EXRT : AZPRInstFormReg0<0b011011, (outs), (ins), "exrt", [], IICPrivilege>{
  let Uses=[c0, c3]; //これだけ?
}

def TRAP : AZPRInstFormReg0<0b011000, (outs), (ins), "trap", [], IICSpecial>{
  let Defs=[c0, c1, c3, c5];
  let Uses=[c4];
}

//def : Pat<(store CPUGRegs:$rb, addr:$immediate),
//          (STW r0, CPUGRegs:$rb, addr:$immediate)>;
//def : Pat<(load addr:$immediate),
//          (LDW r0, addr:$immediate)>;

def NOP	: AZPRInstFormReg0<0b000000, (outs), (ins), "andr\tr0, r0, r0",[], IICAlu>;

def immZExt5  : PatLeaf<(imm), [{ return isUInt<5>(N->getZExtValue()); }]>;
def immZExt16  : PatLeaf<(imm), [{ return isUInt<16>(N->getZExtValue()); }]>;

def logicimm : Operand<i32>{
  let EncoderMethod = "getLogicImmValue";
}

def shiftimm : Operand<i32>{
  let EncoderMethod = "getShiftImmValue";
}

def arithimm : Operand<i32>{
  let EncoderMethod = "getArithImmValue";
  let DecoderMethod = "DecodeImmS16";
}

multiclass ShiftInst<bits<6> op, string asmstr, SDNode OpNode, InstrItinClass itin, RegisterClass RC>{
  def R : AZPRInstFormReg3<op, (outs RC:$rc), (ins RC:$ra, RC:$rb),
                      !strconcat(asmstr, "r\t$ra, $rb, $rc"),
                      [(set RC:$rc, (OpNode RC:$ra, RC:$rb))], itin>;
  def I : AZPRInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1}, (outs RC:$rb), (ins RC:$ra, shiftimm:$immediate),
                      !strconcat(asmstr, "i\t$ra, $rb, $immediate"),
                      [(set RC:$rb, (OpNode RC:$ra, immZExt5:$immediate))], itin>;
}

defm SHRL : ShiftInst<0b001100, "shrl", srl, IICAlu, CPUGRegs>;
defm SHLL : ShiftInst<0b001110, "shll", shl, IICAlu, CPUGRegs>;

//符号は?
multiclass LogicInst<bits<6> op, string asmstr, SDNode OpNode, InstrItinClass itin, RegisterClass RC>{
  def R : AZPRInstFormReg3<op, (outs RC:$rc), (ins RC:$ra, RC:$rb),
                      !strconcat(asmstr, "r\t$ra, $rb, $rc"),
                      [(set RC:$rc, (OpNode RC:$ra, RC:$rb))], itin>;
  def I : AZPRInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1}, (outs RC:$rb), (ins RC:$ra, logicimm:$immediate),
                      !strconcat(asmstr, "i\t$ra, $rb, $immediate"),
                      [(set RC:$rb, (OpNode RC:$ra, immZExt16:$immediate))], itin>;
}

defm AND : LogicInst<0b000000, "and", and, IICAlu, CPUGRegs>;
defm OR : LogicInst<0b000010, "or", or, IICAlu, CPUGRegs>;
defm XOR : LogicInst<0b000100, "xor", xor, IICAlu, CPUGRegs>;

class ArithRegInst<bits<6> op, string asmstr, SDNode OpNode, InstrItinClass itin, RegisterClass RC> : 
	AZPRInstFormReg3<op, (outs RC:$rc), (ins RC:$ra, RC:$rb),
	!strconcat(asmstr, "\t$ra, $rb, $rc"),
	[(set RC:$rc, (OpNode RC:$ra, RC:$rb))], itin>;

def ADDUR : ArithRegInst<0b001000, "addur", add, IICAlu, CPUGRegs>;
def SUBUR : ArithRegInst<0b001011, "subur", sub, IICAlu, CPUGRegs>;
def ADDUI : AZPRInstFormReg2I<0b001001,
	(outs CPUGRegs:$rb), (ins CPUGRegs:$ra, arithimm:$immediate),
	"addui\t$ra, $rb, $immediate",
	[(set CPUGRegs:$rb, (add CPUGRegs:$ra, immSExt16:$immediate))], IICAlu>;

def ADDSR : AZPRInstFormReg3<0b000110, (outs CPUGRegs:$rc), (ins CPUGRegs:$ra, CPUGRegs:$rb), "addsr\t$ra, $rb, $rc", [], IICAlu>;
def SUBSR : AZPRInstFormReg3<0b001010, (outs CPUGRegs:$rc), (ins CPUGRegs:$ra, CPUGRegs:$rb), "subsr\t$ra, $rb, $rc", [], IICAlu>;
def ADDSI : AZPRInstFormReg2I<0b000111,
	(outs CPUGRegs:$rb), (ins CPUGRegs:$ra, arithimm:$immediate),
	"addsi\t$ra, $rb, $immediate",
	[], IICAlu>;

class IndBr<bits<6> op, string asmstr>:
  AZPRInstFormReg1<op, (outs), (ins CPUGRegs:$ra),
     !strconcat(asmstr, "\t$ra"), [],
     IICBranch> {
  let isBranch=1;
  let isIndirectBranch=1;
  let isBarrier=1;//?
  let hasDelaySlot=1;
  let isTerminator=1;
}

def JMP : IndBr<0b010100, "jmp">;

class BrCond<bits<6> op, string asmstr, PatFrag cond_op, InstrItinClass itin, RegisterClass RC>:
  AZPRInstFormReg2I<op, (outs), (ins RC:$ra, RC:$rb, brcondtarget:$immediate),
   !strconcat(asmstr, "\t$ra, $rb, $immediate"),
   [(brcond (cond_op RC:$rb, RC:$ra), bb:$immediate)], itin> {
  let isBranch=1;
  let hasDelaySlot=1;
  let isTerminator=1;
}

def BE : BrCond<0b010000, "be", seteq, IICBranch, CPUGRegs>;
def BNE : BrCond<0b010001, "bne", setne, IICBranch, CPUGRegs>;
def BSGT : BrCond<0b010010, "bsgt", setgt, IICBranch, CPUGRegs>;
def BUGT : BrCond<0b010011, "bugt", setugt, IICBranch, CPUGRegs>;

//BUGT : rb > raでtrue
def : Pat<(brcond (setlt CPUGRegs:$ra, CPUGRegs:$rb), bb:$immediate), (BSGT CPUGRegs:$ra, CPUGRegs:$rb, bb:$immediate)>;
def : Pat<(brcond (setult CPUGRegs:$ra, CPUGRegs:$rb), bb:$immediate), (BUGT CPUGRegs:$ra, CPUGRegs:$rb, bb:$immediate)>;

//SPARC参照
def LO16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((unsigned)N->getZExtValue() & 0xFFFF,
                                   MVT::i32);
}]>;

def HI16 : SDNodeXForm<imm, [{
  // Transformation function: shift the immediate value down into the low bits.
  return CurDAG->getTargetConstant((unsigned)N->getZExtValue() >> 16, MVT::i32);
}]>;

let isCodeGenOnly=1 in {
  def CALLORRLO16 : AZPRInstFormReg2I<0b000011,
      (outs CPUGRegs:$rb), (ins CPUGRegs:$ra, calltargetlo16:$immediate),
      "ori\t$ra, $rb, $immediate",
      [(set CPUGRegs:$rb, (or CPUGRegs:$ra, tglobaladdr:$immediate))], IICLoad>;

  def CALLLoadHI16 : AZPRInstFormReg2I<0b000011,
      (outs CPUGRegs:$rb), (ins calltargethi16:$immediate),
      "ori\tr0, $rb, $immediate",
      [(set CPUGRegs:$rb, tglobaladdr:$immediate)], IICLoad> {
    let ra = 0;
  }
  def BRORRLO16 : AZPRInstFormReg2I<0b000011,
      (outs CPUGRegs:$rb), (ins CPUGRegs:$ra, brtargetlo16:$immediate),
      "ori\t$ra, $rb, $immediate",
      [], IICLoad>;

  def BRLoadHI16 : AZPRInstFormReg2I<0b000011,
      (outs CPUGRegs:$rb), (ins brtargethi16:$immediate),
      "ori\tr0, $rb, $immediate",
      [], IICLoad> {
    let ra = 0;
  }
}

let usesCustomInserter = 1 in {
  def SELECT_CC : AZPRPseudo<(outs CPUGRegs:$dst), (ins CPUGRegs:$lhs, CPUGRegs:$rhs, CPUGRegs:$T, CPUGRegs:$F, i32imm:$COND), "#SELECT_CC", []>;
}

//def : InstAlias<"nop", (set r0, (ANDR r0, r0))>;
//def : InstAlias<"move $imm, $ra", (set CPUGRegs:$ra, (ORI r0, immZExt16:$imm))>;

def : Pat<(i32 immSExt16:$val),
	  (ADDUI r0, imm:$val)>;
def : Pat<(i32 immZExt16:$val),
	  (XORI r0, imm:$val)>;

def : Pat<(i32 imm:$val),
          (ORI (SHLLI (ORI r0, (HI16 imm:$val)), 16), (LO16 imm:$val))>;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

let Defs = [r30], Uses = [r30] in {
def ADJCALLSTACKDOWN : AZPRPseudo<(outs), (ins i32imm:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : AZPRPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;
}

//===----------------------------------------------------------------------===//
//  Arbitrary patterns that map to one or more instructions
//===----------------------------------------------------------------------===//

def : Pat<(AZPRCall (i32 tglobaladdr:$dst)),
          (CALL (CALLORRLO16 (SHLLI (CALLLoadHI16 (i32 tglobaladdr:$dst)), 16), (i32 tglobaladdr:$dst)))>;
//def : Pat<(br bb:$dst),
//          (JMP  (BRORRLO16 (SHLLI (BRLoadHI16 bb:$dst), 16), bb:$dst))>;
def : Pat<(br bb:$dst),
          (BE  r0, r0, bb:$dst)>;
def : Pat<(AZPRCall (i32 texternalsym:$dst)),
          (CALL texternalsym:$dst)>;

def : Pat<(AZPRHi tglobaladdr:$in), (SHLLI (CALLLoadHI16 tglobaladdr:$in), 16)>;
def : Pat<(AZPRLo tglobaladdr:$in), (CALLORRLO16 r0, (i32 tglobaladdr:$in))>;
def : Pat<(AZPROr (AZPRHi tglobaladdr:$in), (AZPRLo tglobaladdr:$in_)),
      (CALLORRLO16 (SHLLI (CALLLoadHI16 (i32 tglobaladdr:$in)), 16), (i32 tglobaladdr:$in_))>;
def : Pat<(sext_inreg CPUGRegs:$rt, i8),
 (ORR (SHLLI (XORR (ADDUI (SHRLI (ANDI CPUGRegs:$rt, 255), 7), -1), (ADDUI r0, -1)), 8), (ANDI CPUGRegs:$rt, 255))>;

def SL32minus : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(32 - (unsigned)N->getZExtValue(),
                                   MVT::i32);
}]>;
def : Pat<(sra CPUGRegs:$ra, immZExt5:$imm),
(ORR (SHLLI (XORR (ADDUI (SHRLI CPUGRegs:$ra, 31), -1), (ADDUI r0, -1)), (SL32minus immZExt5:$imm)), (SHRLI CPUGRegs:$ra, immZExt5:$imm))>;
//$imm = 0のときはないのか?

//0 < $rb < 32 のとき以外は動作は未定義(たぶん)
def : Pat<(sra CPUGRegs:$ra, CPUGRegs:$rb),
(ORR (SHLLR (XORR (ADDUI (SHRLI CPUGRegs:$ra, 31), -1), (ADDUI r0, -1)), (SUBUR (ADDUI r0, 32), CPUGRegs:$rb)), (SHRLR CPUGRegs:$ra, CPUGRegs:$rb))>;

//===----------------------------------------------------------------------===//
// AZPR Calling Convention
//===----------------------------------------------------------------------===//

def CC_AZPR : CallingConv<[
  // i8/i16型の引数はi32型に昇格する
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // 整数型はレジスタに渡す
  CCIfType<[i32], CCAssignToReg<[r1, r2, r3, r4, r5, r6]>>
]>;

def RetCC_AZPR : CallingConv<[
  // i32型はV0レジスタに渡す
  CCIfType<[i32], CCAssignToReg<[r1]>>
]>;

//===----------------------------------------------------------------------===//
// Callee-saved register lists.
//===----------------------------------------------------------------------===//

// 呼び出し先待避レジスタ(Callee-saved register)
def CSR_SingleFloatOnly : CalleeSavedRegs<(add /*(sequence "r%u", 3, 0),*/ r26, r27, r28, r29, r31)>;

//===----------------------------------------------------------------------===//
// AZPR processors supported.
//===----------------------------------------------------------------------===//

def AZPRInstrInfo : InstrInfo;

def : Processor<"azpr32", AZPRGenericItineraries, []>;

def AZPRAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def AZPRAsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "$";
}

def AZPRAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def AZPR : Target {
  let InstructionSet = AZPRInstrInfo;
  let AssemblyWriters = [AZPRAsmWriter];
  let AssemblyParsers = [AZPRAsmParser];
  let AssemblyParserVariants = [AZPRAsmParserVariant];
}
