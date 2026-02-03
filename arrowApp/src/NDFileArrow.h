/*
 * Header file for the NDPluginArrow EPICS areaDetector plugin
 *
 * Author: Jakub Wlodek
 *
 * Copyright (c) : Brookhaven National Laboratory, 2025
 *
 */

#ifndef NDFILEARROW_H
#define NDFILEARROW_H

// Define necessary includes here

using namespace std;

// Include your external dependency library headers
#include <arrow/filesystem/api.h>
#include <arrow/io/api.h>
#include <arrow/ipc/api.h>
#include <arrow/pretty_print.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/table.h>
#include <arrow/util/key_value_metadata.h>

#include <arrow/api.h>

#include <arrow/csv/api.h>

#include <arrow/io/api.h>

#include <arrow/ipc/api.h>

#include <parquet/arrow/reader.h>

#include <parquet/arrow/writer.h>
#include <epicsExport.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <iocsh.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "NDArray.h"
#include "NDPluginFile.h"

// version numbers
#define NDARROW_VERSION 0
#define NDARROW_REVISION 0
#define NDARROW_MODIFICATION 1


// Error message formatters
#define ERR(msg)                                       \
    if (this->getLogLevel() >= NDArrowLogLevel::ERROR) \
        fprintf(stderr, "ERROR | %s::%s: %s\n", this->pluginName, __func__, msg);

#define ERR_ARGS(fmt, ...)                             \
    if (this->getLogLevel() >= NDArrowLogLevel::ERROR) \
        fprintf(stderr, "ERROR | %s::%s: " fmt "\n", this->pluginName, __func__, __VA_ARGS__);

#define ERR_TO_STATUS(fmt, ...)                                       \
    if (this->getLogLevel() >= NDArrowLogLevel::ERROR) {              \
        char errMsg[256];                                             \
        snprintf(errMsg, sizeof(errMsg), fmt, __VA_ARGS__);           \
        printf("ERROR | %s::%s: %s\n", this->pluginName, __func__, errMsg); \
        setStringParam(ADStatusMessage, errMsg);                      \
        setIntegerParam(ADStatus, ADStatusError);                     \
        callParamCallbacks();                                         \
    }

// Warning message formatters
#define WARN(msg)                                        \
    if (this->getLogLevel() >= NDArrowLogLevel::WARNING) \
        fprintf(stderr, "WARNING | %s::%s: %s\n", this->pluginName, __func__, msg);

#define WARN_ARGS(fmt, ...)                              \
    if (this->getLogLevel() >= NDArrowLogLevel::WARNING) \
        fprintf(stderr, "WARNING | %s::%s: " fmt "\n", this->pluginName, __func__, __VA_ARGS__);

#define WARN_TO_STATUS(fmt, ...)                                         \
    if (this->getLogLevel() >= NDArrowLogLevel::WARNING) {               \
        char warnMsg[256];                                               \
        snprintf(warnMsg, sizeof(warnMsg), fmt, __VA_ARGS__);            \
        printf("WARNING | %s::%s: %s\n", this->pluginName, __func__, warnMsg); \
        setStringParam(ADStatusMessage, warnMsg);                        \
        callParamCallbacks();                                            \
    }

// Info message formatters
#define INFO(msg)                                     \
    if (this->getLogLevel() >= NDArrowLogLevel::INFO) \
        fprintf(stdout, "INFO | %s::%s: %s\n", this->pluginName, __func__, msg);

#define INFO_ARGS(fmt, ...)                           \
    if (this->getLogLevel() >= NDArrowLogLevel::INFO) \
        fprintf(stdout, "INFO | %s::%s: " fmt "\n", this->pluginName, __func__, __VA_ARGS__);

#define INFO_TO_STATUS(fmt, ...)                                      \
    if (this->getLogLevel() >= NDArrowLogLevel::INFO) {               \
        char infoMsg[256];                                            \
        snprintf(infoMsg, sizeof(infoMsg), fmt, __VA_ARGS__);         \
        printf("INFO | %s::%s: %s\n", this->pluginName, __func__, infoMsg); \
        setStringParam(ADStatusMessage, infoMsg);                     \
        callParamCallbacks();                                         \
    }

enum class NDArrowLogLevel { NONE = 0, ERROR = 10, WARNING = 20, INFO = 30, DEBUG = 40 };

enum class NDArrowFileFormat {
    PARQUET = 0,
    CSV = 1,
    IPC = 2,
};

shared_ptr<arrow::DataType> getArrowDataType(NDDataType_t dataType);

/**
 * Representation of image data as vectors for Arrow storage.
 * CT: Coordinates type
 * PT: Pixel type
 */
template <typename CT, typename PT>
using vectorizedImg = std::tuple<std::vector<CT>, std::vector<CT>, std::vector<std::vector<PT>>>;
/**
 * Convert raw image data to vectorized format suitable for Arrow storage.
 * Skips pixels where all channel values are zero.
 * 
 * XT: X coordinate type
 * YT: Y coordinate type
 * PT: Pixel type
 */
template <typename CT, typename PT>
vectorizedImg<CT, PT> convertImgToVectors(PT* data, CT rows, CT cols, int nChannels) {
    cout << "Converting image to vectors: " << rows << "x" << cols << " with " << nChannels << " channels." << endl;

    vectorizedImg<CT, PT> reshapedData = {std::vector<CT>(), std::vector<CT>(), std::vector<std::vector<PT>>(nChannels)};

    for (CT i = 0; i < rows; ++i) {
        for (CT j = 0; j < cols; ++j) {
            std::vector<PT> channelValues(nChannels);
            bool allZeros = true;
            for (int c = 0; c < nChannels; ++c) {
                PT value = data[(i * cols + j) * nChannels + c];
                if (value != 0) allZeros = false;
                channelValues[c] = value;
            }

            if (allZeros) continue;  // Skip this pixel if all channel values are zero
            cout << "Pixel (" << j << ", " << i << ") is hot with value: " << static_cast<int>(channelValues[0]) << endl;

            std::get<0>(reshapedData).push_back(j);  // X coordinate
            std::get<1>(reshapedData).push_back(i);  // Y coordinate
            for (int c = 0; c < nChannels; ++c) {
                std::get<2>(reshapedData)[c].push_back(channelValues[c]);
            }
        }
    }
    cout << "Converted image to vectors with " << std::get<0>(reshapedData).size() << " hot pixels." << endl;
    return reshapedData;
}

template <typename CT, typename PT>
std::shared_ptr<arrow::Table> createArrowTableFromNDArray(NDArray* pArray, NDArrayInfo info, std::shared_ptr<arrow::Schema> schema) {
    auto [xCoords, yCoords, pixelChannels] = convertImgToVectors<CT, PT>(
        static_cast<PT*>(pArray->pData),
        info.ySize,
        info.xSize,
        info.colorMode == NDColorModeMono ? 1 : 3
    );

    cout << "Pixel channels size: " << pixelChannels.size() << endl;

    std::shared_ptr<arrow::Array> xCoordArr, yCoordArr;
    std::vector<std::shared_ptr<arrow::Array>> pixelArrays(pixelChannels.size());

    if constexpr (std::is_same<CT, uint8_t>::value) {
        xCoordArr = std::make_shared<arrow::UInt8Array>(xCoords.size(), arrow::Buffer::FromVector(xCoords));
        yCoordArr = std::make_shared<arrow::UInt8Array>(yCoords.size(), arrow::Buffer::FromVector(yCoords));
    } else if constexpr (std::is_same<CT, uint16_t>::value) {
        cout << "Creating UInt16 arrays for coordinates." << endl;
        xCoordArr = std::make_shared<arrow::UInt16Array>(xCoords.size(), arrow::Buffer::FromVector(xCoords));
        yCoordArr = std::make_shared<arrow::UInt16Array>(yCoords.size(), arrow::Buffer::FromVector(yCoords));
    } else if constexpr (std::is_same<CT, uint32_t>::value) {
        xCoordArr = std::make_shared<arrow::UInt32Array>(xCoords.size(), arrow::Buffer::FromVector(xCoords));
        yCoordArr = std::make_shared<arrow::UInt32Array>(yCoords.size(), arrow::Buffer::FromVector(yCoords));
    } else {
        throw std::runtime_error("Unsupported coordinate type for Arrow table creation");
    }

    if constexpr (std::is_same<PT, uint8_t>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::UInt8Array>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else if constexpr (std::is_same<PT, uint8_t>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::UInt8Array>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else if constexpr (std::is_same<PT, int16_t>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::Int16Array>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else if constexpr (std::is_same<PT, uint16_t>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::UInt16Array>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else if constexpr (std::is_same<PT, int32_t>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::Int32Array>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else if constexpr (std::is_same<PT, float>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::FloatArray>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else if constexpr (std::is_same<PT, double>::value) {
        for (size_t c = 0; c < pixelChannels.size(); ++c) {
            pixelArrays[c] = std::make_shared<arrow::DoubleArray>(pixelChannels[c].size(), arrow::Buffer::FromVector(pixelChannels[c]));
        }
    } else {
        throw std::runtime_error("Unsupported pixel type for Arrow table creation");
    }

    std::vector<std::shared_ptr<arrow::Array>> columns;
    columns.push_back(xCoordArr);
    columns.push_back(yCoordArr);
    for (const auto& pixelArr : pixelArrays) {
        columns.push_back(pixelArr);
    }

    shared_ptr<arrow::Table> table = arrow::Table::Make(schema, columns);
    cout << "Table in func " << table->ToString() << endl;
    return table;
}

shared_ptr<arrow::Schema> createSchema(NDDataType_t dataType,
                                       NDColorMode_t colorMode, int xSize, int ySize,
                                       NDArrowFileFormat fileFormat);

/* Plugin class, extends plugin driver */
class NDPLUGIN_API NDFileArrow : public NDPluginFile {
    public:
        NDFileArrow(const char* portName, int queueSize, int blockingCallbacks,
                      const char* NDArrayPort, int NDArrayAddr, int maxBuffers, size_t maxMemory,
                      int priority, int stackSize, int maxThreads);
        virtual ~NDFileArrow() {};

        virtual asynStatus openFile(const char* fileName, NDFileOpenMode_t openMode,
                                    NDArray* pArray);
        virtual asynStatus readFile(NDArray** pArray);
        virtual asynStatus writeFile(NDArray* pArray);
        virtual asynStatus closeFile();
        // virtual void processCallbacks(NDArray* pArray);

        // void processCallbacks(NDArray *pArray);

        // shared_ptr<arrow::Schema> createSchema(NDDataType_t dataType, NDColorMode_t colorMode,
        //                                        int xSize, int ySize, NDArrowFileFormat fileFormat);
        // shared_ptr<arrow::DataType> getArrowDataType(NDDataType_t dataType);


        NDArrowLogLevel getLogLevel() { return this->logLevel; }



    protected:
#include "NDFileArrowParamDefs.h"

    private:
        const char* pluginName = "NDFileArrow";

        void createAllParams();

        std::shared_ptr<arrow::Schema> schema = nullptr;
        std::shared_ptr<arrow::io::FileOutputStream> outfile = nullptr;

        NDArrowLogLevel logLevel = NDArrowLogLevel::DEBUG;
};

#endif // NDFILEARROW_H
