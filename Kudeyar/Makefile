##===- lib/Target/Kudeyar/Makefile --------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source 
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
# Year: 2017
# Alexey Shistko     alexey@kudeyar.com
# Andrei Safronov    andrei@kudeyar.com
##===----------------------------------------------------------------------===##
LEVEL = ../../..
LIBRARYNAME = LLVMKudeyarCodeGen
TARGET = Kudeyar

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = KudeyarGenRegisterInfo.inc KudeyarGenInstrInfo.inc \
		KudeyarGenAsmWriter.inc \
		KudeyarGenDAGISel.inc KudeyarGenCallingConv.inc \
		KudeyarGenSubtargetInfo.inc

DIRS = InstPrinter TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

