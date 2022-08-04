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
#include "NDPluginDriver.h"

//version numbers
#define ARROW_VERSION      	0
#define ARROW_REVISION     	0
#define ARROW_MODIFICATION 	0



// Define the PVStrings for all of your PV values here in the following format
//#define NDPluginArrowPVNameString 	"ARROW_PVNAME" 		//DTYP (ex. asynInt32, asynFloat64, asynOctet)


// Define all necessary tpyes, structs, and enums here


/* Plugin class, extends plugin driver */
class NDPluginArrow : public NDPluginDriver {
    public:
        NDPluginArrow(const char *portName, int queueSize, int blockingCallbacks,
            const char* NDArrayPort, int NDArrayAddr, int maxBuffers,
            size_t maxMemory, int priority, int stackSize, int maxThreads);


        void processCallbacks(NDArray *pArray);

        virtual asynStatus writeInt32(asynUser* pasynUser, epicsInt32 value);

    protected:

        // Define the Param index variables here. Ex:
        // int NDPluginArrowPVName;


        // Define these two variables as the first and last param indexes.
        #define ND_ARROW_FIRST_PARAM 0
        #define ND_ARROW_LAST_PARAM 0 

    private:

        // init all global variables here

        // init all plugin additional functions here

};

// Def that computes the number of params specific to the plugin
#define NUM_ARROW_PARAMS ((int)(&ND_ARROW_LAST_PARAM - &ND_ARROW_FIRST_PARAM + 1))

#endif
