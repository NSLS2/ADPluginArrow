TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

USR_CXXFLAGS += -std=c++17

DBD += NDFileArrow.dbd

INC += NDFileArrow.h
INC += NDFileArrowParamDefs.h

LIBRARY_IOC += NDFileArrow

NDFileArrow_SRCS += NDFileArrow.cpp NDFileArrowParamDefs.cpp

include $(ADCORE)/ADApp/commonLibraryMakefile

NDFileArrow_SYS_LIBS += arrow parquet

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
