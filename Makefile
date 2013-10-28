##===- lib/Target/AZPR/Makefile ----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMAZPRCodeGen
TARGET = AZPR

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = AZPRGenRegisterInfo.inc AZPRGenInstrInfo.inc \
                AZPRGenAsmWriter.inc AZPRGenCodeEmitter.inc \
                AZPRGenDAGISel.inc AZPRGenCallingConv.inc \
                AZPRGenSubtargetInfo.inc AZPRGenMCCodeEmitter.inc \
                AZPRGenEDInfo.inc AZPRGenDisassemblerTables.inc \
                AZPRGenAsmMatcher.inc

DIRS = AsmParser InstPrinter Disassembler TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

