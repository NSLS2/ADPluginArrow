#include "TestNDFileArrow.h"


TEST_F(TestNDFileArrow, TestGetArrowDataType) {

    // Test for NDInt8
    shared_ptr<arrow::DataType> dtype = getArrowDataType(NDInt8);
    ASSERT_EQ(dtype->id(), arrow::Type::INT8);

    // Test for NDUInt8
    dtype = getArrowDataType(NDUInt8);
    ASSERT_EQ(dtype->id(), arrow::Type::UINT8);

    // Test for NDInt16
    dtype = getArrowDataType(NDInt16);
    ASSERT_EQ(dtype->id(), arrow::Type::INT16);

    // Test for NDUInt16
    dtype = getArrowDataType(NDUInt16);
    ASSERT_EQ(dtype->id(), arrow::Type::UINT16);

    // Test for NDInt32
    dtype = getArrowDataType(NDInt32);
    ASSERT_EQ(dtype->id(), arrow::Type::INT32);

    // Test for NDFloat32
    dtype = getArrowDataType(NDFloat32);
    ASSERT_EQ(dtype->id(), arrow::Type::FLOAT);

    // Test for NDFloat64
    dtype = getArrowDataType(NDFloat64);
    ASSERT_EQ(dtype->id(), arrow::Type::DOUBLE);
}

TEST_F(TestNDFileArrow, TestConvertImgToVectorsMonoNoZeros){
    const int rows = 2;
    const int cols = 3;
    const int nChannels = 1;
    uint16_t data[rows * cols * nChannels] = {
        10, 20, 30,
        40, 50, 60
    };

    auto [nRows, xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint16_t>(data, rows, cols, nChannels);

    // Check sizes
    ASSERT_EQ(nRows, 6);
    ASSERT_EQ(xCoords.size(), 6);
    ASSERT_EQ(yCoords.size(), 6);
    ASSERT_EQ(intensityValues.size(), 1);
    ASSERT_EQ(intensityValues[0].size(), 6);

    // Check values
    std::vector<uint8_t> expectedX = {0, 1, 2, 0, 1, 2};
    std::vector<uint8_t> expectedY = {0, 0, 0, 1, 1, 1};
    std::vector<uint16_t> expectedIntensity = {10, 20, 30, 40, 50, 60};
    ASSERT_EQ(xCoords, expectedX);
    ASSERT_EQ(yCoords, expectedY);
    ASSERT_EQ(intensityValues[0], expectedIntensity);
}

TEST_F(TestNDFileArrow, TestConvertImgToVectorsMonoWithZeros){
    const int rows = 2;
    const int cols = 3;
    const int nChannels = 1;
    uint16_t data[rows * cols * nChannels] = {
        10, 0, 30,
        0, 0, 60
    };

    auto [nRows, xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint16_t>(data, rows, cols, nChannels);

    // Check sizes
    ASSERT_EQ(nRows, 3);
    ASSERT_EQ(xCoords.size(), nRows);
    ASSERT_EQ(yCoords.size(), nRows);
    ASSERT_EQ(intensityValues.size(), 1);
    ASSERT_EQ(intensityValues[0].size(), nRows);

    // Check values
    std::vector<uint8_t> expectedX = {0, 2, 2};
    std::vector<uint8_t> expectedY = {0, 0, 1};
    std::vector<uint16_t> expectedIntensity = {10, 30, 60};
    ASSERT_EQ(xCoords, expectedX);
    ASSERT_EQ(yCoords, expectedY);
    ASSERT_EQ(intensityValues[0], expectedIntensity);
}

TEST_F(TestNDFileArrow, TestConvertImgToVectorsRGBNoZeros){
    const int rows = 2;
    const int cols = 2;
    const int nChannels = 3;
    uint8_t data[rows * cols * nChannels] = {
        // Pixel (0,0)
        255, 0, 0,
        // Pixel (0,1)
        0, 255, 0,
        // Pixel (1,0)
        0, 0, 255,
        // Pixel (1,1)
        255, 255, 0
    };

    auto [nRows, xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint8_t>(data, rows, cols, nChannels);

    // Check sizes
    ASSERT_EQ(nRows, 4);
    ASSERT_EQ(xCoords.size(), nRows);
    ASSERT_EQ(yCoords.size(), nRows);
    ASSERT_EQ(intensityValues.size(), 3);
    ASSERT_EQ(intensityValues[0].size(), nRows); // R channel
    ASSERT_EQ(intensityValues[1].size(), nRows); // G channel
    ASSERT_EQ(intensityValues[2].size(), nRows); // B channel

    // Check values
    std::vector<uint8_t> expectedX = {0, 1, 0, 1};
    std::vector<uint8_t> expectedY = {0, 0, 1, 1};
    std::vector<uint8_t> expectedR = {255, 0, 0, 255};
    std::vector<uint8_t> expectedG = {0, 255, 0, 255};
    std::vector<uint8_t> expectedB = {0, 0, 255, 0};
    ASSERT_EQ(xCoords, expectedX);
    ASSERT_EQ(yCoords, expectedY);
    ASSERT_EQ(intensityValues[0], expectedR);
    ASSERT_EQ(intensityValues[1], expectedG);
    ASSERT_EQ(intensityValues[2], expectedB);
}

TEST_F(TestNDFileArrow, TestConvertImgToVectorsRGBWithZeros){
    const int rows = 2;
    const int cols = 2;
    const int nChannels = 3;
    uint8_t data[rows * cols * nChannels] = {
        // Pixel (0,0)
        0, 0, 0,
        // Pixel (0,1)
        255, 0, 0,
        // Pixel (1,0)
        0, 0, 0,
        // Pixel (1,1)
        0, 0, 255
    };

    auto [nRows, xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint8_t>(data, rows, cols, nChannels);

    ASSERT_EQ(nRows, 2);
    // Check sizes
    ASSERT_EQ(xCoords.size(), nRows);
    ASSERT_EQ(yCoords.size(), nRows);
    ASSERT_EQ(intensityValues.size(), 3);
    ASSERT_EQ(intensityValues[0].size(), nRows); // R channel
    ASSERT_EQ(intensityValues[1].size(), nRows); // G channel
    ASSERT_EQ(intensityValues[2].size(), nRows); // B channel

    // Check values
    std::vector<uint8_t> expectedX = {1, 1};
    std::vector<uint8_t> expectedY = {0, 1};
    std::vector<uint8_t> expectedR = {255, 0};
    std::vector<uint8_t> expectedG = {0, 0};
    std::vector<uint8_t> expectedB = {0, 255};
    ASSERT_EQ(xCoords, expectedX);
    ASSERT_EQ(yCoords, expectedY);
    ASSERT_EQ(intensityValues[0], expectedR);
    ASSERT_EQ(intensityValues[1], expectedG);
    ASSERT_EQ(intensityValues[2], expectedB);
}

TEST_F(TestNDFileArrow, TestCreateSchemaMono){
    auto schema = createSchema(NDInt16, NDColorModeMono, 4, 4);
    // Check number of fields
    ASSERT_EQ(schema->num_fields(), 7);

    // // Check field names and types
    auto fields = schema->fields();

    ASSERT_EQ(fields[0]->name(), "X");
    ASSERT_EQ(fields[0]->type()->id(), arrow::Type::UINT8);
    ASSERT_EQ(fields[1]->name(), "Y");
    ASSERT_EQ(fields[1]->type()->id(), arrow::Type::UINT8);
    ASSERT_EQ(fields[2]->name(), "Intensity");
    ASSERT_EQ(fields[2]->type()->id(), arrow::Type::INT16);

    schema = createSchema(NDUInt16, NDColorModeMono, 1024, 1024);

    // Check number of fields
    ASSERT_EQ(schema->num_fields(), 7);

    // Check field names and types
    fields = schema->fields();
    ASSERT_EQ(fields[0]->name(), "X");
    ASSERT_EQ(fields[0]->type()->id(), arrow::Type::UINT16);
    ASSERT_EQ(fields[1]->name(), "Y");
    ASSERT_EQ(fields[1]->type()->id(), arrow::Type::UINT16);
    ASSERT_EQ(fields[2]->name(), "Intensity");
    ASSERT_EQ(fields[2]->type()->id(), arrow::Type::UINT16);

    // Only check the default attribute fields once
    ASSERT_EQ(fields[3]->name(), "NDArrayUniqueId");
    ASSERT_EQ(fields[3]->type()->id(), arrow::Type::INT32);
    ASSERT_EQ(fields[4]->name(), "NDArrayTimestamp");
    ASSERT_EQ(fields[4]->type()->id(), arrow::Type::DOUBLE);
    ASSERT_EQ(fields[5]->name(), "NDArrayEpicsTSSec");
    ASSERT_EQ(fields[5]->type()->id(), arrow::Type::UINT32);
    ASSERT_EQ(fields[6]->name(), "NDArrayEpicsTSnSec");
    ASSERT_EQ(fields[6]->type()->id(), arrow::Type::UINT32);


    // Probably an unlikely scenario, but test for super large image sizes
    schema = createSchema(NDFloat64, NDColorModeMono, 90000, 90000);

    // Check number of fields
    ASSERT_EQ(schema->num_fields(), 7);

    // Check field names and types
    fields = schema->fields();
    ASSERT_EQ(fields[0]->name(), "X");
    ASSERT_EQ(fields[0]->type()->id(), arrow::Type::UINT32);
    ASSERT_EQ(fields[1]->name(), "Y");
    ASSERT_EQ(fields[1]->type()->id(), arrow::Type::UINT32);
    ASSERT_EQ(fields[2]->name(), "Intensity");
    ASSERT_EQ(fields[2]->type()->id(), arrow::Type::DOUBLE);

}

TEST_F(TestNDFileArrow, TestCreateSchemaRGB1) {
    auto schema = createSchema(NDInt8, NDColorModeRGB1, 4, 4);
    // Check number of fields
    ASSERT_EQ(schema->num_fields(), 9);

    // // Check field names and types
    auto fields = schema->fields();

    ASSERT_EQ(fields[0]->name(), "X");
    ASSERT_EQ(fields[0]->type()->id(), arrow::Type::UINT8);
    ASSERT_EQ(fields[1]->name(), "Y");
    ASSERT_EQ(fields[1]->type()->id(), arrow::Type::UINT8);
    ASSERT_EQ(fields[2]->name(), "R");
    ASSERT_EQ(fields[2]->type()->id(), arrow::Type::INT8);
    ASSERT_EQ(fields[3]->name(), "G");
    ASSERT_EQ(fields[3]->type()->id(), arrow::Type::INT8);
    ASSERT_EQ(fields[4]->name(), "B");
    ASSERT_EQ(fields[4]->type()->id(), arrow::Type::INT8);

    // Only check the default attribute fields once
    ASSERT_EQ(fields[5]->name(), "NDArrayUniqueId");
    ASSERT_EQ(fields[5]->type()->id(), arrow::Type::INT32);
    ASSERT_EQ(fields[6]->name(), "NDArrayTimestamp");
    ASSERT_EQ(fields[6]->type()->id(), arrow::Type::DOUBLE);
    ASSERT_EQ(fields[7]->name(), "NDArrayEpicsTSSec");
    ASSERT_EQ(fields[7]->type()->id(), arrow::Type::UINT32);
    ASSERT_EQ(fields[8]->name(), "NDArrayEpicsTSnSec");
    ASSERT_EQ(fields[8]->type()->id(), arrow::Type::UINT32);

}
