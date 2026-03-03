#include <iostream>
#include <thread>
#include "SensorUtility.h"

//IMPORTANT: Check the USB port name! 
//usually "/dev/ttyUSB0"
const std::string PORT = "/dev/ttyACM0"; 

int main() {
    std::cout << "Starting Sensor Program..." << std::endl;

    SensorUtility imu(PORT);
    
    imu.start();

    while (true) {
        if (imu.isActive()) {
            std::cout << "STATUS: [ACTIVE] | "
                      << "Pitch: " << imu.getPitch() << " | "
                      << "Roll: " << imu.getRoll() << std::endl;
        } else {
            std::cout << "STATUS: [WAITING FOR SENSOR...]" << std::endl;
        }

        // Simulate doing other work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    imu.stop();
    return 0;
}