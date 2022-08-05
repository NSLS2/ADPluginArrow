/**
 * This file is a basic template for implementing areaDetector plugins.
 * You must implement all of the functions already listed here along with any 
 * additional plugin specific functions you require.
 * 
 * Author: Jakub Wlodek
 * Created on: 08/04/2022
 * 
 */



//include some standard libraries
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <stdio.h>


//include epics/area detector libraries
#include <epicsMutex.h>
#include <epicsString.h>
#include <iocsh.h>
#include "NDArray.h"
// Include your plugin's header file here
#include "NDPluginArrow.hpp"
#include <epicsExport.h>


// Error message formatters
#define ERR(msg)                                                                                 \
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "ERROR | %s::%s: %s\n", pluginName, functionName, \
              msg)

#define ERR_ARGS(fmt, ...)                                                              \
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "ERROR | %s::%s: " fmt "\n", pluginName, \
              functionName, __VA_ARGS__);

// Warning message formatters
#define WARN(msg) \
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "WARN | %s::%s: %s\n", pluginName, functionName, msg)

#define WARN_ARGS(fmt, ...)                                                            \
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "WARN | %s::%s: " fmt "\n", pluginName, \
              functionName, __VA_ARGS__);

// Log message formatters
#define LOG(msg) \
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s::%s: %s\n", pluginName, functionName, msg)

#define LOG_ARGS(fmt, ...)                                                                       \
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s::%s: " fmt "\n", pluginName, functionName, \
              __VA_ARGS__);




// Namespaces
using namespace std;

// Name of the plugin
static const char *pluginName="NDPluginArrow";


asynStatus NDPluginArrow::openFile(const char* fileName, NDFileOpenMode_t openMode, NDArray* pArray){
    const char* functionName = "openFile";


    return asynSuccess;
}


arrow::Status NDPluginArrow::writeCSV(){
    const char* functionName = "writeCSV";

    std::string filename;
    getStringParam(NDFileName, filename);

    ARROW_ASSIGN_OR_RAISE(auto output, arrow::io::FileOutputStream::Open(filename));
    auto writeOpts = arrow::csv::WriteOptions::Defaults();
    if(arrow::csv::WriteCSV(*(this->table), writeOpts, output.get()).ok()){
        ERR("Failed to write out csv file!");
    }
    return arrow::Status::OK();
}


asynStatus NDPluginArrow::writeFile(NDArray* pArray){
    const char* functionName = "writeFile";

    arrow::Status status;


    NDColorMode_t colorMode;
    NDDataType_t dataType = pArray->dataType;
    int xSize, ySize;
    NDAttribute* pAttribute;
    asynStatus adStatus = asynSuccess;

    pAttribute = pArray->pAttributeList->find("ColorMode");
    if (pAttribute) pAttribute->getValue(NDAttrInt32, &colorMode);

    if (colorMode != NDColorModeMono) {
        ERR("Only mono images are supported!");
        return asynError;
    }

    std::shared_ptr<arrow::Table> table;
    std::vector<std::shared_ptr<arrow::Array>> arrays;

    arrow::Int32Builder xCoordArrayBuilder;
    arrow::Int32Builder yCoordArrayBuilder;
    arrow::Int32Builder intensityArrayBuilder;
    std::vector<int32_t> xCoordVector;
    std::vector<int32_t> yCoordVector;
    std::vector<int32_t> intensityVector;


    int64_t numRows = 0;
    for(int i=0; i< ySize; i++) {
        for(int j = 0; j < xSize; j++){
            switch(dataType){
                case NDInt8:
                case NDUInt8:
                    if(((uint8_t*) pArray->pData)[i * xSize + j] != 0){
                        xCoordVector.push_back(j);
                        yCoordVector.push_back(i);
                        intensityVector.push_back((int32_t) ((uint8_t*) pArray->pData)[i * xSize + j]);
                        numRows++;
                    }
                case NDInt16:
                case NDUInt16:
                    if(((uint16_t*) pArray->pData)[i * xSize + j] != 0){
                        xCoordVector.push_back(j);
                        yCoordVector.push_back(i);
                        intensityVector.push_back((int32_t) ((uint16_t*) pArray->pData)[i * xSize + j]);
                        numRows++;
                    }
                default:
                    ERR("Data type not supported!");
                    return asynError;
            }

        }
    }
    
    xCoordArrayBuilder.Reserve(numRows);
    xCoordArrayBuilder.AppendValues(xCoordVector);
    
    yCoordArrayBuilder.Reserve(numRows);
    yCoordArrayBuilder.AppendValues(yCoordVector);
    
    intensityArrayBuilder.Reserve(numRows);
    intensityArrayBuilder.AppendValues(intensityVector);

    auto xCoordArrayPtr = xCoordArrayBuilder.Finish();
    auto yCoordArrayPtr = yCoordArrayBuilder.Finish();
    auto intensityArrayPtr = intensityArrayBuilder.Finish();
    if(!xCoordArrayPtr.ok() || !yCoordArrayPtr.ok() || !intensityArrayPtr.ok()) {
        ERR("Failed to build Arrow arrays!");
    }
    
    std::shared_ptr<arrow::Array> xCoordArray = *xCoordArrayPtr;
    std::shared_ptr<arrow::Array> yCoordArray = *yCoordArrayPtr;
    std::shared_ptr<arrow::Array> intensityArray = *intensityArrayPtr;

    arrays.push_back(xCoordArray);
    arrays.push_back(yCoordArray);
    arrays.push_back(intensityArray);
    
    
    this->table = arrow::Table::Make(this->schema, arrays);

    status = writeCSV();


    return asynSuccess;
}



asynStatus NDPluginArrow::closeFile(){

    const char* functionName = "closeFile";
    asynStatus status = asynSuccess;



    return status;
}

asynStatus NDPluginArrow::readFile(NDArray** pArray){

    const char* functionName = "readFile";
    asynStatus status = asynSuccess;



    return status;
}



/**
 * Override of NDPluginDriver function. Must be implemented by your plugin
 *
 * Performs callback when write operation is performed on an asynInt32 record
 * 
 * @params[in]: pasynUser	-> pointer to asyn User that initiated the transaction
 * @params[in]: value		-> value PV was set to
 * @return: success if PV was updated correctly, otherwise error
 */
asynStatus NDPluginArrow::writeInt32(asynUser* pasynUser, epicsInt32 value){
    const char* functionName = "writeInt32";
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;

    status = setIntegerParam(function, value);
    LOG_ARGS("function = %d value=%d", function, value);

    // TODO: Handle callbacks for any integer param write ops
    
    if(function < ND_ARROW_FIRST_PARAM){
        status = NDPluginDriver::writeInt32(pasynUser, value);
    }
    callParamCallbacks();
    if(status){
        ERR_ARGS("Failed to wrote Int32 val to PV: function = %d value=%d", function, value);
    }
    return status;
}



/* Process callbacks function inherited from NDPluginDriver.
 * You must implement this function for your plugin to accept NDArrays
 *
 * @params[in]: pArray -> NDArray recieved by the plugin from the camera
 * @return: void
*/

/*
void NDPluginArrow::processCallbacks(NDArray *pArray){
    static const char* functionName = "processCallbacks";
    NDArray *pScratch;
    asynStatus status = asynSuccess;
    NDArrayInfo arrayInfo;

    // If set to true, downstream plugins will perform callbacks on output pScratch
    // If false, no downstream callbacks will be performed
    bool performCallbacks = true;

    //call base class and get information about frame
    NDPluginDriver::beginProcessCallbacks(pArray);

    pArray->getInfo(&arrayInfo);

    //unlock the mutex for the processing portion
    this->unlock();

    // This sets the output of the plugin to the input array
    pScratch = pArray;

    // If we are manipulating the image/output, we allocate a new scratch frame
    // You will need to specify dimensions, and data type.

    //pScratch = pNDArrayPool->alloc(ndims, dims, dataType, 0, NULL
    //if(pScratch == NULL){
    //    ERR("Unable to allocate frame.")
    //    return;
    //}
    

    // Process the image here. pArray is read only, and if any image manipulation is required
    // a copy should be made into pScratch.
    // 
    // Note that this expects any external libraries to be thread safe. If they aren't, move
    // the processing to after this->lock();
    //
    // Access data with pArray->pData.
    // DO NOT CALL pArray.release()

    this->lock();

    // If pScratch was allocated, set the color mode and unique ID attributes here.

    //pScratch->pAttributeList->add("ColorMode", "Color Mode", NDAttrInt32, &colorMode);
    //pScratch->uniqueId = pArray->uniqueId;

    if(status == asynError){
        ERR("Image not processed correctly!");
        return;
    }

    NDPluginDriver::endProcessCallbacks(pScratch, false, performCallbacks);

    // If pScratch was allocated in this function, make sure to release it.
    // pScratch.release()

    callParamCallbacks();
}
*/



//constructror from base class, replace with your plugin name
NDPluginArrow::NDPluginArrow(
        const char *portName, int queueSize, int blockingCallbacks,
        const char *NDArrayPort, int NDArrayAddr,
        int maxBuffers, size_t maxMemory,
        int priority, int stackSize, int maxThreads)
        /* Invoke the base class constructor */
        : NDPluginFile(portName, queueSize, blockingCallbacks,
        NDArrayPort, NDArrayAddr, 1, maxBuffers, maxMemory,
        asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
        asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
        ASYN_CANBLOCK, 1, priority, stackSize, maxThreads)
{

    char versionString[25];

    // Initialize Parameters here, using the string vals and indexes from the header. Ex:
    // createParam(NDPluginArrowOctetString, 	asynParamOctet, 	&NDPluginArrowOctet);  -> asynParamOctet records (stringin, stringout, waveform) 
    // createParam(NDPluginArrowIntegerString, 	asynParamInt32, 	&NDPluginArrowInteger);  -> asynInt32 records (bo, bi, mbbo, mbbi, ao, ai)
    // createParam(NDPluginArrowFloatString, 	asynParamFloat64, 	&NDPluginArrowFloat);  -> asynParamFloat64 records (ao, ai, waveform) 






    this->xCoord = arrow::field("X", arrow::int32());
    this->yCoord = arrow::field("Y", arrow::int32());
    this->intensity = arrow::field("Intensity", arrow::int32());
    this->schema = arrow::schema({this->xCoord, this->yCoord, this->intensity});

    // Set some basic plugin info Params
    setStringParam(NDPluginDriverPluginType, "NDPluginArrow");
    epicsSnprintf(versionString, sizeof(versionString), "%d.%d.%d", NDARROW_VERSION, NDARROW_REVISION, NDARROW_MODIFICATION);
    setStringParam(NDDriverVersion, versionString);
    connectToArrayPort();
}



/**
 * External configure function. This will be called from the IOC shell of the
 * detector the plugin is attached to, and will create an instance of the plugin and start it
 * 
 * @params[in]	-> all passed to constructor
 */
extern "C" int NDArrowConfigure(
        const char *portName, int queueSize, int blockingCallbacks,
        const char *NDArrayPort, int NDArrayAddr,
        int maxBuffers, size_t maxMemory,
        int priority, int stackSize, int maxThreads){

    // Initialize instance of our plugin and start it.
    NDPluginArrow *pPlugin = new NDPluginArrow(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, maxBuffers, maxMemory, priority, stackSize, maxThreads);
    return pPlugin->start();
}


/* IOC shell arguments passed to the plugin configure function */
static const iocshArg initArg0 = { "portName", iocshArgString };
static const iocshArg initArg1 = { "frame queue size", iocshArgInt };
static const iocshArg initArg2 = { "blocking callbacks", iocshArgInt };
static const iocshArg initArg3 = { "NDArrayPort", iocshArgString };
static const iocshArg initArg4 = { "NDArrayAddr", iocshArgInt };
static const iocshArg initArg5 = { "maxBuffers", iocshArgInt };
static const iocshArg initArg6 = { "maxMemory", iocshArgInt };
static const iocshArg initArg7 = { "priority", iocshArgInt };
static const iocshArg initArg8 = { "stackSize", iocshArgInt };
static const iocshArg initArg9 = { "maxThreads", iocshArgInt };
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
                                            &initArg3,
                                            &initArg4,
                                            &initArg5,
                                            &initArg6,
                                            &initArg7,
                                            &initArg8,
                                            &initArg9};


// Define the path to your plugin's extern configure function above
static const iocshFuncDef initFuncDef = { "NDArrowConfigure", 10, initArgs };


/* link the configure function with the passed args, and call it from the IOC shell */
static void initCallFunc(const iocshArgBuf *args){
    NDArrowConfigure(
            args[0].sval, args[1].ival, args[2].ival,
            args[3].sval, args[4].ival, args[5].ival,
            args[6].ival, args[7].ival, args[8].ival, args[9].ival);
}


/* function to register the configure function in the IOC shell */
extern "C" void NDArrowRegister(void){
    iocshRegister(&initFuncDef,initCallFunc);
}


/* Exports plugin registration */
extern "C" {
    epicsExportRegistrar(NDArrowRegister);
}
