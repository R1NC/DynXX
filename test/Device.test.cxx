#include <gtest/gtest.h>
#include <DynXX/CXX/Device.hxx>

TEST(Device, DynxxDeviceType) {
    EXPECT_NE(dynxxDeviceType(), DynXXDeviceTypeX::Unknown);
}

TEST(Device, DynxxDeviceName) {
    EXPECT_FALSE(dynxxDeviceName().empty());
}

TEST(Device, DynxxDeviceManufacturer) {
    EXPECT_FALSE(dynxxDeviceManufacturer().empty());
}

TEST(Device, DynxxDeviceModel) {
    EXPECT_FALSE(dynxxDeviceModel().empty());
}

TEST(Device, DynxxDeviceOsVersion) {
    EXPECT_FALSE(dynxxDeviceOsVersion().empty());
}

TEST(Device, DynxxDeviceCpuArch) {
    EXPECT_NE(dynxxDeviceCpuArch(), DynXXDeviceCpuArchX::Unknown);
}
