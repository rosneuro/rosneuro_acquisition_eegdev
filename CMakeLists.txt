cmake_minimum_required(VERSION 2.8.3)
project(rosneuro_acquisition_eegdev)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(catkin REQUIRED COMPONENTS 
			 roscpp 
			 std_msgs
			 pluginlib
			 rosneuro_data
			 rosneuro_msgs
			 rosneuro_acquisition)

find_package(PkgConfig)
find_package(GTest REQUIRED)

SET(CMAKE_BUILD_TYPE RelWithDebInfo)

find_library(EEGDEV_LIBRARIES eegdev)
find_path(EEGDEV_INCLUDE_DIRS eegdev.h)

message(STATUS "eegdev lib: ${EEGDEV_LIBRARIES}")

if(NOT EEGDEV_LIBRARIES)
	message(WARNING "Library EEGDEV not found in the system. 
	The plugin will not be compiled and its functionalities will not be
	available. To fix it, install EEGDEV in your system and re-compile 
	the project")
else()
	message(STATUS "Library EEGDEV found. The plugin will be installed")

catkin_package(
  INCLUDE_DIRS 
	include
  LIBRARIES 
	${PROJECT_NAME}
  CATKIN_DEPENDS
  	roscpp
	std_msgs
	pluginlib
	rosneuro_data
	rosneuro_msgs
	rosneuro_acquisition

  DEPENDS
)


###########
## Build ##
###########

include_directories(${catkin_INCLUDE_DIRS} 
					${rosneuro_acquisition_INCLUDE_DIRS}
					${EEGDEV_INCLUDE_DIRS} 
					include
					include/${PROJECT_NAME}/)

add_library(${PROJECT_NAME} 
			src/EGDDevice.cpp
)

target_link_libraries(${PROJECT_NAME} 
					  ${catkin_LIBRARIES}
					  ${rosneuro_acquisition_LIBRARIES}
					  ${EEGDEV_LIBRARIES}
)

add_dependencies(${PROJECT_NAME} 
				 ${catkin_EXPORTED_TARGETS}
)

catkin_add_gtest(test_egd_device test/TestEgdDevice.cpp)
target_link_libraries(test_egd_device ${PROJECT_NAME} ${catkin_LIBRARIES} gtest gmock pthread)
include_directories(${GTEST_INCLUDE_DIRS} gmock pthread include/${PROJECT_NAME}/)

#################
## Executables ##
#################

add_executable(test_egddevice example/run.cpp)
target_link_libraries(test_egddevice rosneuro_acquisition_eegdev)

#################
##   Install   ##
#################
install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})

install(FILES plugin_eegdev.xml
	DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

endif()
