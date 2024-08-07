#ifndef ROSNEURO_ACQUISITION_PLUGIN_EEGDEV_CPP
#define ROSNEURO_ACQUISITION_PLUGIN_EEGDEV_CPP

#include "EGDDevice.hpp"

namespace rosneuro {
    EGDDevice::EGDDevice(void) : Device() {
        this->name_ = "egddev";
        this->grp_	   = nullptr;
        this->strides_ = nullptr;
    }

    EGDDevice::EGDDevice(NeuroFrame* frame) : Device(frame) {
        this->name_ = "egddev";
        this->grp_	   = nullptr;
        this->strides_ = nullptr;
    }

    EGDDevice::~EGDDevice(void) {
        this->destroy_egd_structures();
    }

    bool EGDDevice::Configure(NeuroFrame* frame, unsigned int framerate) {
        this->frame_	 = frame;
        this->framerate_ = framerate;

        if(!ros::param::get("~devarg", this->devarg_)) {
            ROS_ERROR("Missing 'devarg' in the server. 'devarg' is a mandatory parameter");
            return false;
        }

        ros::param::param<std::string>("~devext", this->devext_, "");

        int samplerate;
        ros::param::param<int>("~samplerate", samplerate, 0);
        this->samplerate_ = (unsigned int)samplerate;

        return true;
    }

    bool EGDDevice::Setup(void) {
        if(!this->setup_dev_capabilities()) {
            ROS_ERROR("Cannot setup device capabilities");
            return false;
        }

        if(!this->setup_neuro_data()) {
            ROS_ERROR("Cannot setup data");
            return false;
        }

        if(!this->setup_egd_structures()) {
            ROS_ERROR("Cannot setup data structures");
            return false;
        }

        if (!this->egd_acquisition_setup()) {
            ROS_ERROR("Cannot setup the acquisition");
            return false;
        }

        ROS_INFO("'%s' device correctly configured with samplerate=%d Hz", this->GetName().c_str(), this->samplerate_);
        return true;
    }

    bool EGDDevice::Open() {

        std::string dev_name_arg;
        bool isfile = false;

        if(this->devarg_.find(".bdf") != std::string::npos) {
            dev_name_arg.assign("datafile|path|");
            isfile = true;
        } else if(this->devarg_.find(".gdf") != std::string::npos) {
            dev_name_arg.assign("datafile|path|");
            isfile = true;
        }
        dev_name_arg.append(this->devarg_);

        if(!isfile & this->samplerate_ > 0) {
            if(this->devarg_.compare("gtec") == 0) {
                dev_name_arg += "|samplerate|" + std::to_string(this->samplerate_);
            } else if(this->devarg_.compare("eego") == 0) {
                dev_name_arg += "|SR|" + std::to_string(this->samplerate_);
            }
        }
        std::cout << "dev_name_arg: " << dev_name_arg << std::endl;
        dev_name_arg += this->devext_;

        if(!this->egd_open_device(dev_name_arg)) {
            std::cout << "QUA " << std::endl;
            return false;
        }

        return true;
    }

    bool EGDDevice::Close(void) {
        if(egd_close(this->egddev_) == -1) {
            std::cerr<<"[Error] - Cannot close the device '" << this->GetName() <<"': "
                     << std::strerror(errno)<<std::endl;
            return false;
        }

        this->destroy_egd_structures();
        return true;
    }

    bool EGDDevice::Start(void) {
        if(egd_start(this->egddev_) == -1) {
            std::cerr<<"[Error] - Cannot start the device '" << this->GetName() <<"': "
                     << std::strerror(errno) << std::endl;
            return false;
        }
        return true;
    }

    bool EGDDevice::Stop(void) {
        if(egd_stop(this->egddev_) == -1) {
            std::cerr<<"[Error] - Cannot stop the device '" << this->GetName() << "': "
                     << std::strerror(errno) <<std::endl;
            return false;
        }
        return true;
    }

    size_t EGDDevice::Get(void) {
        size_t size;

        size = egd_get_data(this->egddev_, this->frame_->eeg.nsamples(),
                            (void*)this->frame_->eeg.data(),
                            (void*)this->frame_->exg.data(),
                            (void*)this->frame_->tri.data());
        if (size == -1) {
            std::cerr<<"Error reading data: " << std::strerror(errno) << std::endl;
        }

        return size;
    }

    size_t EGDDevice::GetAvailable(void) {
        return egd_get_available(this->egddev_);
    }

    void EGDDevice::init_dev_capabilities(void) {
        this->devinfo.model	= "";
        this->devinfo.id	= "";
    }

    void EGDDevice::init_egd_structures(void) {
        this->grp_ = (grpconf*)malloc(EGD_DATA_GROUPS * sizeof(grpconf));
        memset(this->grp_, 0, EGD_DATA_GROUPS*sizeof(struct grpconf));

        this->strides_ = (size_t*)malloc(EGD_DATA_GROUPS * sizeof(size_t));
        memset(this->strides_, 0, EGD_DATA_GROUPS*sizeof(size_t));
    }

    bool EGDDevice::setup_dev_capabilities(void) {
        char* model			= nullptr;
        char* id			= nullptr;

        this->init_dev_capabilities();
        if(egd_get_cap(this->egddev_, EGD_CAP_DEVTYPE, &model) == -1) {
            std::cerr<<"[Error] - Cannot get device type: "<<strerror(errno)<<std::endl;
            return false;
        }

        if(egd_get_cap(this->egddev_, EGD_CAP_DEVID, &id) == -1) {
            std::cerr<<"[Error] - Cannot get device id: "<<strerror(errno)<<std::endl;
            return false;
        }

        this->devinfo.model	= std::string(model);
        this->devinfo.id	= std::string(id);

        return true;
    }

    bool EGDDevice::setup_neuro_data(void) {
        size_t neeg, nexg, ntri;
        NeuroDataInfo *ieeg, *iexg, *itri;

        if(egd_get_cap(this->egddev_, EGD_CAP_FS, &this->samplerate_) == -1) {
            std::cerr<<"[Error] - Cannot get device sampling rate: "<<strerror(errno)<<std::endl;
            return false;
        }
        size_t ns = (size_t)(this->samplerate_/this->framerate_);
        this->frame_->sr = this->samplerate_;

        if( (neeg = egd_get_numch(this->egddev_, EGD_EEG)) == -1) {
            std::cerr<<"[Error] - Cannot get number EEG channels: "<<strerror(errno)<<std::endl;
            return false;
        }

        if( (nexg = egd_get_numch(this->egddev_, EGD_SENSOR)) == -1) {
            std::cerr<<"[Error] - Cannot get number EXG channels: "<<strerror(errno)<<std::endl;
            return false;
        }

        if( (ntri = egd_get_numch(this->egddev_, EGD_TRIGGER)) == -1) {
            std::cerr<<"[Error] - Cannot get number TRIGGER channels: "<<strerror(errno)<<std::endl;
            return false;
        }

        this->frame_->eeg.reserve(ns, neeg);
        this->frame_->exg.reserve(ns, nexg);
        this->frame_->tri.reserve(ns, ntri);

        this->setup_neuro_info(this->frame_->eeg.info(), this->frame_->eeg.nchannels(), EGD_EEG);
        this->setup_neuro_info(this->frame_->exg.info(), this->frame_->exg.nchannels(), EGD_SENSOR);
        this->setup_neuro_info(this->frame_->tri.info(), this->frame_->tri.nchannels(), EGD_TRIGGER);

        return true;
    }

    void EGDDevice::setup_neuro_info(NeuroDataInfo* info, size_t nch, unsigned int index) {
        char unit[16] = {0};
        char transducter[128] = {0};
        char filtering[128] = {0};
        char label[32] = {0};
        double mm[2] = {0.0f, 1.0f};
        int isint = 0;

        if(info == nullptr) {
            return;
        }

        egd_channel_info(this->egddev_, index, 0, EGD_UNIT, unit,
                        EGD_TRANSDUCTER, transducter, EGD_PREFILTERING, filtering,
                        EGD_MM_D, mm, EGD_ISINT, &isint, EGD_EOL);

        info->unit			= std::string(unit);
        info->transducter	= std::string(transducter);
        info->prefiltering	= std::string(filtering);
        info->minmax[0]		= mm[0];
        info->minmax[1]		= mm[1];
        info->isint			= isint;

        info->labels.clear();

        for(auto i = 0; i<nch; i++) {
            egd_channel_info(this->egddev_, index, i, EGD_LABEL, label, EGD_EOL);
            info->labels.push_back(std::string(label));
        }
    }

    bool EGDDevice::setup_egd_structures(void) {
        this->init_egd_structures();

        this->grp_[0].sensortype = EGD_EEG;
        this->grp_[0].index		 = 0;
        this->grp_[0].iarray	 = 0;
        this->grp_[0].datatype	 = EGD_FLOAT;
        this->grp_[0].arr_offset = 0;
        this->grp_[0].nch		 = this->frame_->eeg.nchannels();

        this->grp_[1].sensortype = EGD_SENSOR;
        this->grp_[1].index		 = 0;
        this->grp_[1].iarray	 = 1;
        this->grp_[1].datatype	 = EGD_FLOAT;
        this->grp_[1].arr_offset = 0;
        this->grp_[1].nch		 = this->frame_->exg.nchannels();

        this->grp_[2].sensortype = EGD_TRIGGER;
        this->grp_[2].index		 = 0;
        this->grp_[2].iarray	 = 2;
        this->grp_[2].datatype	 = EGD_INT32;
        this->grp_[2].arr_offset = 0;
        this->grp_[2].nch		 = this->frame_->tri.nchannels();

        this->strides_[0] = this->frame_->eeg.stride();
        this->strides_[1] = this->frame_->exg.stride();
        this->strides_[2] = this->frame_->tri.stride();

        return true;
    }

    void EGDDevice::destroy_egd_structures(void) {
        if(this->grp_ != nullptr) {
            free(this->grp_);
        }
        this->grp_ = nullptr;

        if(this->strides_ != nullptr) {
            free(this->strides_);
        }
        this->strides_ = nullptr;
    }

    bool EGDDevice::egd_acquisition_setup(void){
        if(egd_acq_setup(this->egddev_, EGD_DATA_GROUPS, this->strides_, EGD_DATA_GROUPS, this->grp_) == -1) {
            ROS_ERROR("Cannot setup the device");
            return false;
        }
        return true;
    }

    bool EGDDevice::egd_open_device(std::string dev_name_arg){
        if(!(this->egddev_ = egd_open(dev_name_arg.c_str()))) {
            ROS_ERROR("Cannot open the '%s' device with arg=%s and samplerate=%d Hz",
                      this->GetName().c_str(), dev_name_arg.c_str(), this->samplerate_);
            return false;
        }
        ROS_INFO("'%s' device correctly opened with arg=%s", this->GetName().c_str(), dev_name_arg.c_str());
        return true;
    }
}

#endif
