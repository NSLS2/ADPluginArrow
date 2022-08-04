# ADPluginArrow

Repository containing Arrow areaDetector plugin source code.

### Installation

To install ADPluginArrow, clone it into your `areaDetector` directory, enter it and run:

```
make
```

Note that `ADSupport`, `ADCore`, and any external dependencies must be built/installed first. 

Next, open `$(AREA_DETECTOR)/configure/RELEASE_PRODS.local`, and add:

```
ADPLUGINARROW=$(AREA_DETECTOR)/ADPluginNameArrow
```

Then, add the following to `$(AREA_DETECTOR)/ADCore/ADApp/commonDriverMakefile`

```
ifdef ADPLUGINARROW
  $(DBD_NAME)_DBD += NDPluginArrow.dbd
  PROD_LIBS += NDPluginArrow
  # Add any external library dependancy links here
  # PROD_SYS_LIBS += ... For system libraries
  # PROD_LIBS += ... For libraries built as part of the plugin build process
endif
```

Next, enter the target `areaDetector` driver directory and rebuild it with `make`.

Finally, in either your IOC `st.cmd` startup file, or in `$(AREA_DETECTOR)/ADCore/iocBoot/commonPlugins.cmd` initialize the plugin for startup:

```
NDPluginArrowConfigure("ARR1", $(QSIZE), 0, "$(PORT)", 0, 0, 0, 0, 0, $(MAX_THREADS=5))
dbLoadRecords("$(ADPLUGINARROW/db/NDPluginArrow.template", "P=$(PREFIX), R=Arrow1:, PORT=ARR1, ADDR=0, TIMEOUT=1, NDARRAY_PORT=$(PORT), NAME=ARR1, NCHANS=$(XSIZE)")
set_requestfile_path("$(ADPLUGINARROW)/db")
```

### Credits

This plugin was built in part with the help of [ADPluginTemplate](https://github.com/jwlodek/ADPluginTemplate).
