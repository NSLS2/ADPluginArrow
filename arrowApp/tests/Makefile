TOP = ../..
include $(TOP)/configure/CONFIG

#----------------------------------------

USR_CPPFLAGS += -std=c++17

PROD_IOC = TestNDFileArrow

TestNDFileArrow_SRCS += TestNDFileArrowMain.cpp
TestNDFileArrow_SRCS += TestNDFileArrow.cpp

# Add additional test source files here
# TestNDFileArrow_SRCS +=

TestNDFileArrow_LIBS += NDFileArrow ADBase NDPlugin asyn gtest gmock $(EPICS_BASE_IOC_LIBS)

TestNDFileArrow_SYS_LIBS += arrow parquet

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
