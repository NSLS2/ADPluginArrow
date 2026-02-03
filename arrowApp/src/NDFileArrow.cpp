/**
 * NDFileArrow.cpp
 */

#include "NDFileArrow.h"

shared_ptr<arrow::DataType> getArrowDataType(NDDataType_t dataType) {
    switch (dataType) {
        case NDInt8:
            return arrow::int8();
        case NDUInt8:
            return arrow::uint8();
        case NDInt16:
            return arrow::int16();
        case NDUInt16:
            return arrow::uint16();
        case NDInt32:
            return arrow::int32();
        case NDUInt32:
            return arrow::uint32();
        case NDInt64:
            return arrow::int64();
        case NDUInt64:
            return arrow::uint64();
        case NDFloat32:
            return arrow::float32();
        case NDFloat64:
            return arrow::float64();
        default:
            throw std::runtime_error("Unsupported NDDataType_t for Arrow field creation");
    }
}

shared_ptr<arrow::Schema> createSchema(NDDataType_t dataType,
                                       NDColorMode_t colorMode, int xSize, int ySize) {
    shared_ptr<arrow::DataType> pixelDataType, coordDataType;
    shared_ptr<arrow::FieldVector> fields = make_shared<arrow::FieldVector>();

    // Get required min field dtype from NDDataType_t
    pixelDataType = getArrowDataType(dataType);

    // Get min required dtype for dimensions. Usually will be uint16.
    if (xSize > std::numeric_limits<uint16_t>::max() || ySize > std::numeric_limits<uint16_t>::max()) {
        coordDataType = arrow::uint32();
    } else if (xSize > std::numeric_limits<uint8_t>::max() || ySize > std::numeric_limits<uint8_t>::max()) {
        coordDataType = arrow::uint16();
    } else {
        coordDataType = arrow::uint8();
    }

    fields->push_back(arrow::field("X", coordDataType));
    fields->push_back(arrow::field("Y", coordDataType));

    // TODO: Support other color modes
    if (colorMode == NDColorModeMono) {
        fields->push_back(arrow::field("Intensity", pixelDataType));
    } else if (colorMode == NDColorModeRGB1) {
        fields->push_back(arrow::field("R", pixelDataType));
        fields->push_back(arrow::field("G", pixelDataType));
        fields->push_back(arrow::field("B", pixelDataType));
    } else {
        throw std::runtime_error("Only Mono and RGB1 color modes are supported!");
    }

    // Default attribute fields
    fields->push_back(arrow::field("NDArrayUniqueId", arrow::int32()));
    fields->push_back(arrow::field("NDArrayTimestamp", arrow::float64()));
    fields->push_back(arrow::field("NDArrayEpicsTSSec", arrow::uint32()));
    fields->push_back(arrow::field("NDArrayEpicsTSnSec", arrow::uint32()));

    return arrow::schema(*fields);
}

asynStatus NDFileArrow::openFile(const char* fileName, NDFileOpenMode_t openMode,
                                   NDArray* pArray) {
    /* We don't support reading yet */
    if (openMode & NDFileModeRead) return (asynError);

    /* We don't support opening an existing file for appending yet */
    if (openMode & NDFileModeAppend) return (asynError);

    // cout << "Opening Arrow file: " << fileName << endl;

    // Create schema based on pArray info
    NDArrayInfo arrayInfo;
    pArray->getInfo(&arrayInfo);

    // cout << "Array Info - DataType: " << pArray->dataType
    //      << ", ColorMode: " << arrayInfo.colorMode
    //      << ", XSize: " << arrayInfo.xSize
    //      << ", YSize: " << arrayInfo.ySize << endl;
    this->schema =
        createSchema(pArray->dataType, arrayInfo.colorMode, arrayInfo.xSize, arrayInfo.ySize);

    // cout << "Created Arrow schema: " << this->schema->ToString() << endl;
    this->outfile = arrow::io::FileOutputStream::Open(fileName).ValueOrDie();
    // cout << "Opened output file stream." << endl;

    return asynSuccess;
}



asynStatus NDFileArrow::writeFile(NDArray* pArray) {
    arrow::Status status;

    NDArrayInfo arrayInfo;
    pArray->getInfo(&arrayInfo);

    // cout << "Writing NDArray uniqueId: " << pArray->uniqueId << endl;
    // cout << this->schema->ToString() << endl;

    // Get the types of each column from the schema created on file open
    arrow::Type::type coordDataType = this->schema->GetFieldByName("X")->type()->id();
    arrow::Type::type pixelType;
    if (arrayInfo.colorMode == NDColorModeMono) {
        pixelType = this->schema->GetFieldByName("Intensity")->type()->id();
    } else {
        pixelType = this->schema->GetFieldByName("R")->type()->id();
    }

    // cout << "Coordinate data type: " << coordDataType << ", Pixel data type: " << pixelType << endl;

    shared_ptr<arrow::Table> table;

    if (coordDataType == arrow::Type::UINT32) {
        switch(pixelType) {
            case arrow::Type::INT8:
                table = createArrowTableFromNDArray<uint32_t, int8_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::UINT8:
                table = createArrowTableFromNDArray<uint32_t, uint8_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::INT16:
                table = createArrowTableFromNDArray<uint32_t, int16_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::UINT16:
                table = createArrowTableFromNDArray<uint32_t, uint16_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::INT32:
                table = createArrowTableFromNDArray<uint32_t, int32_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::FLOAT:
                table = createArrowTableFromNDArray<uint32_t, float>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::DOUBLE:
                table = createArrowTableFromNDArray<uint32_t, double>(pArray, arrayInfo, this->schema);
                break;
            default:
                ERR("Unknown pixel data type in Arrow schema!");
                return asynError;
        }
    } else if (coordDataType == arrow::Type::UINT16) {
        switch(pixelType) {
            case arrow::Type::INT8:
                table = createArrowTableFromNDArray<uint16_t, int8_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::UINT8:
                table = createArrowTableFromNDArray<uint16_t, uint8_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::INT16:
                table = createArrowTableFromNDArray<uint16_t, int16_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::UINT16:
                table = createArrowTableFromNDArray<uint16_t, uint16_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::INT32:
                table = createArrowTableFromNDArray<uint16_t, int32_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::FLOAT:
                table = createArrowTableFromNDArray<uint16_t, float>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::DOUBLE:
                table = createArrowTableFromNDArray<uint16_t, double>(pArray, arrayInfo, this->schema);
                break;
            default:
                ERR("Unknown pixel data type in Arrow schema!");
                return asynError;
        }
    } else if (coordDataType == arrow::Type::UINT8) {
        switch(pixelType) {
            case arrow::Type::INT8:
                table = createArrowTableFromNDArray<uint8_t, int8_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::UINT8:
                table = createArrowTableFromNDArray<uint8_t, uint8_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::INT16:
                table = createArrowTableFromNDArray<uint8_t, int16_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::UINT16:
                table = createArrowTableFromNDArray<uint8_t, uint16_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::INT32:
                table = createArrowTableFromNDArray<uint8_t, int32_t>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::FLOAT:
                table = createArrowTableFromNDArray<uint8_t, float>(pArray, arrayInfo, this->schema);
                break;
            case arrow::Type::DOUBLE:
                table = createArrowTableFromNDArray<uint8_t, double>(pArray, arrayInfo, this->schema);
                break;
            default:
                ERR("Unknown pixel data type in Arrow schema!");
                return asynError;
        }
    } else {
        ERR("Unknown coordinate data type in Arrow schema!");
        return asynError;
    }

    // cout << "Created Arrow table: " << table->ToString() << endl;

    NDArrowFileFormat fileFormat;
    getIntegerParam(NDFileArrow_FileFormat, (int*) &fileFormat);

    arrow::Status writeStatus;
    if (fileFormat == NDArrowFileFormat::PARQUET) {
        writeStatus = parquet::arrow::WriteTable(*table, arrow::default_memory_pool(),
                                         this->outfile, 2048);
    } else if (fileFormat == NDArrowFileFormat::IPC) {
        auto ipcWriter = arrow::ipc::MakeFileWriter(this->outfile.get(), table->schema()).ValueOrDie();
        writeStatus = ipcWriter->WriteTable(*table);
        if (writeStatus == arrow::Status::OK())
            writeStatus = ipcWriter->Close();
    } else if (fileFormat == NDArrowFileFormat::CSV) {
        auto csvWriter = arrow::csv::MakeCSVWriter(this->outfile.get(), table->schema()).ValueOrDie();
        writeStatus = csvWriter->WriteTable(*table);
        if (writeStatus == arrow::Status::OK())
            writeStatus = csvWriter->Close();
    } else {
        ERR("Unsupported Arrow file format for writing!");
        return asynError;
    }

    if(writeStatus != arrow::Status::OK()) {
        ERR("Failed to write Arrow table to file!");
        return asynError;
    }

    return asynSuccess;
}

/**
asynStatus NDFileArrow::writeFile(){
    const char* functionName = "writeFile";

    asynStatus status = asynSuccess;



    return status;
}
*/

asynStatus NDFileArrow::readFile(NDArray** pArray) {

    return asynSuccess;
}


asynStatus NDFileArrow::closeFile() {

    if (this->outfile != nullptr) {
        arrow::Status status = this->outfile->Close();
        if (!status.ok()) {
            ERR_ARGS("Failed to close Arrow output file: %s", status.message().c_str());
            return asynError;
        }
        this->outfile = nullptr;
    }
    return asynSuccess;
}

/**
 * Constructor for NDFileArrow class.
 */
NDFileArrow::NDFileArrow(const char* portName, int queueSize, int blockingCallbacks,
                             const char* NDArrayPort, int NDArrayAddr, int maxBuffers,
                             size_t maxMemory, int priority, int stackSize, int maxThreads)
    /* Invoke the base class constructor */
    : NDPluginFile(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr, 1, maxBuffers,
                   maxMemory, asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
                   asynInt32ArrayMask | asynFloat64ArrayMask | asynGenericPointerMask,
                   ASYN_CANBLOCK, 1, priority, stackSize, maxThreads) {
    char versionString[25];

    this->createAllParams();

    // Set some basic plugin info Params
    setStringParam(NDPluginDriverPluginType, "NDFileArrow");
    epicsSnprintf(versionString, sizeof(versionString), "%d.%d.%d", NDARROW_VERSION,
                  NDARROW_REVISION, NDARROW_MODIFICATION);
    setStringParam(NDDriverVersion, versionString);

    this->supportsMultipleArrays = 0; // For now, only support single NDArray per file

    connectToArrayPort();
}

/**
 * Plugin configuration function.
 * Called directly from iocsh.
 */
extern "C" int NDFileArrowConfigure(const char* portName, int queueSize, int blockingCallbacks,
                                const char* NDArrayPort, int NDArrayAddr, int maxBuffers,
                                size_t maxMemory, int priority, int stackSize, int maxThreads) {
    // Initialize instance of our plugin and start it.
    NDFileArrow* pPlugin =
        new NDFileArrow(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
                          maxBuffers, maxMemory, priority, stackSize, maxThreads);
    return pPlugin->start();
}

/* IOC shell arguments passed to the plugin configure function */
static const iocshArg initArg0 = {"portName", iocshArgString};
static const iocshArg initArg1 = {"frame queue size", iocshArgInt};
static const iocshArg initArg2 = {"blocking callbacks", iocshArgInt};
static const iocshArg initArg3 = {"NDArrayPort", iocshArgString};
static const iocshArg initArg4 = {"NDArrayAddr", iocshArgInt};
static const iocshArg initArg5 = {"maxBuffers", iocshArgInt};
static const iocshArg initArg6 = {"maxMemory", iocshArgInt};
static const iocshArg initArg7 = {"priority", iocshArgInt};
static const iocshArg initArg8 = {"stackSize", iocshArgInt};
static const iocshArg initArg9 = {"maxThreads", iocshArgInt};
static const iocshArg* const initArgs[] = {&initArg0, &initArg1, &initArg2, &initArg3, &initArg4,
                                           &initArg5, &initArg6, &initArg7, &initArg8, &initArg9};

// Define the path to your plugin's extern configure function above
static const iocshFuncDef initFuncDef = {"NDFileArrowConfigure", 10, initArgs};

/* link the configure function with the passed args, and call it from the IOC
 * shell */
static void initCallFunc(const iocshArgBuf* args) {
    NDFileArrowConfigure(args[0].sval, args[1].ival, args[2].ival, args[3].sval, args[4].ival,
                     args[5].ival, args[6].ival, args[7].ival, args[8].ival, args[9].ival);
}

/* function to register the configure function in the IOC shell */
extern "C" void NDFileArrowRegister(void) { iocshRegister(&initFuncDef, initCallFunc); }

/* Exports plugin registration */
extern "C" {
epicsExportRegistrar(NDFileArrowRegister);
}
