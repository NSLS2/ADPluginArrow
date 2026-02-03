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

    auto [xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint16_t>(data, rows, cols, nChannels);

    // Check sizes
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

    auto [xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint16_t>(data, rows, cols, nChannels);

    // Check sizes
    ASSERT_EQ(xCoords.size(), 3);
    ASSERT_EQ(yCoords.size(), 3);
    ASSERT_EQ(intensityValues.size(), 1);
    ASSERT_EQ(intensityValues[0].size(), 3);

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

    auto [xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint8_t>(data, rows, cols, nChannels);

    // Check sizes
    ASSERT_EQ(xCoords.size(), 4);
    ASSERT_EQ(yCoords.size(), 4);
    ASSERT_EQ(intensityValues.size(), 3);
    ASSERT_EQ(intensityValues[0].size(), 4); // R channel
    ASSERT_EQ(intensityValues[1].size(), 4); // G channel
    ASSERT_EQ(intensityValues[2].size(), 4); // B channel

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

    auto [xCoords, yCoords, intensityValues] = convertImgToVectors<uint8_t, uint8_t>(data, rows, cols, nChannels);

    // Check sizes
    ASSERT_EQ(xCoords.size(), 2);
    ASSERT_EQ(yCoords.size(), 2);
    ASSERT_EQ(intensityValues.size(), 3);
    ASSERT_EQ(intensityValues[0].size(), 2); // R channel
    ASSERT_EQ(intensityValues[1].size(), 2); // G channel
    ASSERT_EQ(intensityValues[2].size(), 2); // B channel

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

