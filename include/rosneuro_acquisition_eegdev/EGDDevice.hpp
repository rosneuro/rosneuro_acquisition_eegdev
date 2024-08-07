#ifndef ROSNEURO_ACQUISITION_PLUGIN_EEGDEV_HPP
#define ROSNEURO_ACQUISITION_PLUGIN_EEGDEV_HPP

#include <errno.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include <eegdev.h>
#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
#include <rosneuro_data/NeuroData.hpp>
#include <rosneuro_acquisition/Device.hpp>
#include <gtest/gtest_prod.h>

#define EGD_DATA_GROUPS 3

namespace rosneuro {

    /*! \brief      EGD device class
     *
     * This class describes the puglin to a device compatible with the open-source libeegdev library
     * (http://neuro.debian.net/pkgs/libeegdev-dev.html), that can be created by the FactoryDevice.
     * The class allows the user to control the device, such as starting, stopping, opening and closing
     * the device. The framerate of data acquisition can be set up as well. The class derives from the Device
     * class, and so it shares its public methods.
     *
     * \sa FactoryDevice
     */
    class EGDDevice : public Device {

        public:
            /*! \brief      Constructor
             *
             * \param      frame  Data frame
             *
             */
            EGDDevice(NeuroFrame* frame);

            EGDDevice(void);

            /*! \brief      Destructor
             */
            virtual ~EGDDevice(void);

            bool Configure(NeuroFrame* frame, unsigned int framerate);

            /*! \brief      Set up the device
             *
             * \param      framerate  The framerate of data acquisition [Hz]
             *
             * \return     True if the set up is correctly performed, false otherwise
             */
            bool Setup(void);

            /*! \brief      Open the device
             *
             * \param      devname     Name of the device
             * \param      samplerate  Samplerate of the device [Hz]
             *
             * \return     True if the device is correctly opened, false otherwise
             */
            bool Open(void);

            /*! \brief      Close the device
             *
             * \return     True if the device is correctly closed, false otherwise
             */
            bool Close(void);

            /*! \brief      Start the device
             *
             * \return     True if the device is correctly started, false otherwise
             */
            bool Start(void);

            /*! \brief      Stop the device
             *
             * \return     True if the device is correctly stopped, false otherwise
             */
            bool Stop(void);

            /*! \brief      Get data from the device
             *
             * \return     Size of the data
             */
            size_t Get(void);

            /*! \brief      Get available data from the device
             *
             * \return     Size of the data
             */
            size_t GetAvailable(void);

        protected:
            virtual void init_dev_capabilities(void);
            virtual void init_egd_structures(void);
            virtual bool setup_dev_capabilities(void);
            virtual bool setup_neuro_data(void);
            virtual void setup_neuro_info(NeuroDataInfo* data, size_t nch, unsigned int index);
            virtual bool setup_egd_structures(void);
            virtual void destroy_egd_structures(void);
            virtual bool egd_acquisition_setup(void);
            virtual bool egd_open_device(std::string dev_name_arg);

            struct  eegdev*	egddev_;
            struct	grpconf* grp_;
            size_t* strides_;

            std::string		devarg_;
            std::string		devext_;
            unsigned int	samplerate_;
            unsigned int	framerate_;

            FRIEND_TEST(EGDDeviceTestSuite, DefaultConstructor);
            FRIEND_TEST(EGDDeviceTestSuite, OpenSuccess);
            FRIEND_TEST(EGDDeviceTestSuite, SetupReturnsTrue);
            FRIEND_TEST(EGDDeviceTestSuite, InitDevCapabilities);
            FRIEND_TEST(EGDDeviceTestSuite, InitEgdStructures);
    };

    PLUGINLIB_EXPORT_CLASS(rosneuro::EGDDevice, rosneuro::Device)
}

#endif
