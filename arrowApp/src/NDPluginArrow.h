/**
 * Template header file fo NDPlugins
 *
 *
 * Author:Jakub Wlodek
 * Created on: 08/04/2022
 *
 */

#ifndef NDPluginArrow_H
#define NDPluginArrow_H

// Define necessary includes here

using namespace std;

// include base plugin driver
#include "NDArray.h"
#include "NDPluginFile.h"
// Include your external dependency library headers
#include <arrow/filesystem/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <arrow/pretty_print.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/table.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

// include epics/area detector libraries
#include <epicsMutex.h>
#include <epicsString.h>
#include <iocsh.h>

#include "NDArray.h"
// Include your plugin's header file here
#include <epicsExport.h>

// version numbers
#define NDARROW_VERSION 0
#define NDARROW_REVISION 0
#define NDARROW_MODIFICATION 0

// Error message formatters
#define ERR(msg)                                       \
    if (this->getLogLevel() >= NDArrowLogLevel::ERROR) \
        fprintf(stderr, "ERROR | %s::%s: %s\n", pluginName, __func__, msg);

#define ERR_ARGS(fmt, ...)                             \
    if (this->getLogLevel() >= NDArrowLogLevel::ERROR) \
        fprintf(stderr, "ERROR | %s::%s: " fmt "\n", pluginName, __func__, __VA_ARGS__);

// Warning message formatters
#define WARN(msg)                                        \
    if (this->getLogLevel() >= NDArrowLogLevel::WARNING) \
        fprintf(stderr, "WARNING | %s::%s: %s\n", pluginName, __func__, msg);

#define WARN_ARGS(fmt, ...)                              \
    if (this->getLogLevel() >= NDArrowLogLevel::WARNING) \
        fprintf(stderr, "WARNING | %s::%s: " fmt "\n", pluginName, __func__, __VA_ARGS__);

// Info message formatters
#define INFO(msg)                                     \
    if (this->getLogLevel() >= NDArrowLogLevel::INFO) \
        fprintf(stdout, "INFO | %s::%s: %s\n", pluginName, __func__, msg);

#define INFO_ARGS(fmt, ...)                           \
    if (this->getLogLevel() >= NDArrowLogLevel::INFO) \
        fprintf(stdout, "INFO | %s::%s: " fmt "\n", pluginName, __func__, __VA_ARGS__);

// Debug message formatters
#define DEBUG(msg)                                     \
    if (this->getLogLevel() >= NDArrowLogLevel::DEBUG) \
        fprintf(stdout, "DEBUG | %s::%s: %s\n", pluginName, __func__, msg);

#define DEBUG_ARGS(fmt, ...)                           \
    if (this->getLogLevel() >= NDArrowLogLevel::DEBUG) \
        fprintf(stdout, "DEBUG | %s::%s: " fmt "\n", pluginName, __func__, __VA_ARGS__);

enum class NDArrowLogLevel { NONE = 0, ERROR = 10, WARNING = 20, INFO = 30, DEBUG = 40 };

/* Plugin class, extends plugin driver */
class NDPLUGIN_API NDPluginArrow : public NDPluginFile {
    public:
        NDPluginArrow(const char* portName, int queueSize, int blockingCallbacks,
                      const char* NDArrayPort, int NDArrayAddr, int maxBuffers, size_t maxMemory,
                      int priority, int stackSize, int maxThreads);
        ~NDPluginArrow();

        virtual asynStatus openFile(const char* fileName, NDFileOpenMode_t openMode,
                                    NDArray* pArray);
        virtual asynStatus readFile(NDArray** pArray);
        virtual asynStatus writeFile(NDArray* pArray);
        virtual asynStatus closeFile();
        virtual void processCallbacks(NDArray* pArray);
        // void processCallbacks(NDArray *pArray);

        virtual asynStatus writeInt32(asynUser* pasynUser, epicsInt32 value);
        NDArrowLogLevel getLogLevel() { return this->logLevel; }

        arrow::Status writeCSV();

    protected:
#include "NDPluginArrowParamDefs.h"

    private:
        std::shared_ptr<arrow::Field> xCoord, yCoord, intensity;
        std::shared_ptr<arrow::Schema> schema;
        std::shared_ptr<arrow::Table> table;

        NDArrowLogLevel logLevel = NDArrowLogLevel::INFO;
};

// Def that computes the number of params specific to the plugin
#define NUM_ARROW_PARAMS ((int) (&ND_ARROW_LAST_PARAM - &ND_ARROW_FIRST_PARAM + 1))

#endif
