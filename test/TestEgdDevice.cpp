#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "EGDDevice.hpp"

namespace rosneuro {
    class EGDDeviceTest : public EGDDevice {
        public:
            EGDDeviceTest() : EGDDevice() {}
            EGDDeviceTest(NeuroFrame* frame) : EGDDevice(frame) {}
            ~EGDDeviceTest() {}
            virtual bool setup_dev_capabilities() { return true; }
            virtual bool setup_neuro_data() { return true; }
            virtual bool setup_egd_structures() { return true; }
            virtual bool egd_acquisition_setup() { return true; }
            virtual bool egd_open_device(std::string dev_name_arg) { return true; }
    };

    class EGDDeviceTestSuite : public ::testing::Test {
        public:
            EGDDeviceTestSuite() {}
            ~EGDDeviceTestSuite() {}
            void SetUp() {
                egd_device = new EGDDeviceTest();
                egd_device_frame = new EGDDeviceTest(new NeuroFrame());
            }
            EGDDeviceTest* egd_device;
            EGDDeviceTest* egd_device_frame;
    };

    TEST_F(EGDDeviceTestSuite, DefaultConstructor) {
        ASSERT_EQ(egd_device->name_, "egddev");
        ASSERT_EQ(egd_device->grp_, nullptr);
        ASSERT_EQ(egd_device->strides_, nullptr);

        ASSERT_EQ(egd_device_frame->name_, "egddev");
        ASSERT_EQ(egd_device_frame->grp_, nullptr);
        ASSERT_EQ(egd_device_frame->strides_, nullptr);
    }

    TEST_F(EGDDeviceTestSuite, ConfigureReturnsTrue) {
        NeuroFrame frame;
        unsigned int framerate = 42;
        ros::param::set("~devarg", "test");
        ASSERT_TRUE(egd_device->Configure(&frame, framerate));
        ros::param::del("~devarg");
    }

    TEST_F(EGDDeviceTestSuite, ConfigureReturnsFalse) {
        NeuroFrame frame;
        unsigned int framerate = 42;
        ASSERT_FALSE(egd_device->Configure(&frame, framerate));
    }

    TEST_F(EGDDeviceTestSuite, InitDevCapabilities) {
        egd_device->init_dev_capabilities();
        ASSERT_EQ(egd_device->devinfo.model, "");
        ASSERT_EQ(egd_device->devinfo.id, "");
    }

    TEST_F(EGDDeviceTestSuite, InitEgdStructures) {
        egd_device->init_egd_structures();
        EXPECT_NE(egd_device->grp_, nullptr);
        EXPECT_NE(egd_device->strides_, nullptr);
    }

    TEST_F(EGDDeviceTestSuite, Setup) {
        ASSERT_TRUE(egd_device_frame->Setup());
        ASSERT_TRUE(egd_device->Setup());
    }

    TEST_F(EGDDeviceTestSuite, Open) {
        ASSERT_TRUE(egd_device_frame->Open());
        ASSERT_TRUE(egd_device->Open());
    }
}

int main(int argc, char **argv) {
    ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Fatal);
    ros::init(argc, argv, "test_egd_device");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}