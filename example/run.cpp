#include <ros/ros.h>
#include <rosneuro_data/NeuroData.hpp>
#include "rosneuro_acquisition_eegdev/EGDDevice.hpp"
#include <unistd.h>

int main(int argc, char** argv) {
	rosneuro::NeuroFrame frame;
	rosneuro::EGDDevice	egddev(&frame);

    ros::init(argc, argv, "test_egddevice");
	ros::param::set("devarg", argv[1]);
	ros::param::set("samplerate", 512);

	egddev.Configure(&frame, 16);

	if(!egddev.Open()) {
        return -1;
    }
	if(!egddev.Setup()) {
		std::cerr<<"SETUP ERROR"<<std::endl;
		return -1;
	}

	frame.eeg.dump();
	frame.exg.dump();
	frame.tri.dump();

	egddev.Close();

	return 0;
}
