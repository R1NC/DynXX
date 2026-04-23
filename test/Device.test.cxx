#include <gtest/gtest.h>
#include <DynXX/CXX/Device.hxx>

class DynXXDeviceTestSuite : public ::testing::Test {};

TEST_F(DynXXDeviceTestSuite, Type) {
    EXPECT_NE(dynxxDeviceType(), DynXXDeviceTypeX::Unknown);
}

TEST_F(DynXXDeviceTestSuite, Name) {
    EXPECT_FALSE(dynxxDeviceName().empty());
}

TEST_F(DynXXDeviceTestSuite, Manufacturer) {
    EXPECT_FALSE(dynxxDeviceManufacturer().empty());
}

TEST_F(DynXXDeviceTestSuite, Model) {
    EXPECT_FALSE(dynxxDeviceModel().empty());
}

TEST_F(DynXXDeviceTestSuite, OsVersion) {
    EXPECT_FALSE(dynxxDeviceOsVersion().empty());
}

TEST_F(DynXXDeviceTestSuite, CpuArch) {
    EXPECT_NE(dynxxDeviceCpuArch(), DynXXDeviceCpuArchX::Unknown);
}



