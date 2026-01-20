TOP = .
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Support))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *app))

include $(TOP)/configure/RULES_TOP


bobfiles:
	pixi run make-bobfiles

paramdefs:
	pixi run make-paramdefs

lint:
	pixi run lint
