TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

USR_CXXFLAGS += -std=c++17

USR_INCLUDES += -I$(TOP)/arrowSupport/include

DBD += NDPluginArrow.dbd

INC += NDPluginArrow.hpp

LIBRARY_IOC += NDPluginArrow

NDPluginArrow_SRCS += NDPluginArrow.cpp

include $(ADCORE)/ADApp/commonLibraryMakefile

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
