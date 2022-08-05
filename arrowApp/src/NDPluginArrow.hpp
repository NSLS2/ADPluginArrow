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

//Define necessary includes here

using namespace std;

//include base plugin driver
#include "NDPluginFile.h"

// Include your external dependency library headers
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/status.h>
#include <arrow/ipc/api.h>
#include <arrow/pretty_print.h>
#include <arrow/result.h>
#include <arrow/table.h>
#include <arrow/array.h>
#include <arrow/builder.h>
#include <arrow/api.h>


//version numbers
#define NDARROW_VERSION      	0
#define NDARROW_REVISION     	0
#define NDARROW_MODIFICATION 	0



// Define the PVStrings for all of your PV values here in the following format
//#define NDPluginArrowPVNameString 	"ARROW_PVNAME" 		//DTYP (ex. asynInt32, asynFloat64, asynOctet)


// Define all necessary tpyes, structs, and enums here


/* Plugin class, extends plugin driver */
class NDPLUGIN_API NDPluginArrow : public NDPluginFile {
    public:
        NDPluginArrow(const char *portName, int queueSize, int blockingCallbacks,
            const char* NDArrayPort, int NDArrayAddr, int maxBuffers,
            size_t maxMemory, int priority, int stackSize, int maxThreads);


        virtual asynStatus openFile(const char* fileName, NDFileOpenMode_t openMode, NDArray *pArray);
        virtual asynStatus readFile(NDArray** pArray);
        virtual asynStatus writeFile(NDArray* pArray);
        virtual asynStatus closeFile();
        //void processCallbacks(NDArray *pArray);

        virtual asynStatus writeInt32(asynUser* pasynUser, epicsInt32 value);

    protected:

        // Define the Param index variables here. Ex:
        // int NDPluginArrowPVName;


        // Define these two variables as the first and last param indexes.
        #define ND_ARROW_FIRST_PARAM 0
        #define ND_ARROW_LAST_PARAM 0 

    private:

        std::shared_ptr<arrow::Field> xCoord, yCoord, intensity;
        std::shared_ptr<arrow::Schema> schema;
        std::shared_ptr<arrow::Table> table;


        arrow::Status writeCSV();
};

// Def that computes the number of params specific to the plugin
#define NUM_ARROW_PARAMS ((int)(&ND_ARROW_LAST_PARAM - &ND_ARROW_FIRST_PARAM + 1))

#endif
