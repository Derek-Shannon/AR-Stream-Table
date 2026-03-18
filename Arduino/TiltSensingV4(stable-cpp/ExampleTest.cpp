#include <iostream>
#include <iomanip>
#include "SensorUtility.h"

int main() {
    //IMPORTANT: Check the USB port name! 
    //usually "/dev/ttyUSB0"
    SensorUtility sensor("/dev/ttyUSB0", 115200);

    std::cout << "Starting Sensor Thread..." << std::endl;
    sensor.start();

    std::cout << "Waiting for data..." << std::endl;

    while (true) {
        if (sensor.isActive()) {
            std::cout << "\rPitch: " << std::fixed << std::setprecision(2) << std::setw(7) << sensor.getPitch() 
                      << " | Roll: " << std::fixed << std::setprecision(2) << std::setw(7) << sensor.getRoll() << std::flush;
        } else {
            std::cout << "\r[Waiting for sensor...]" << std::flush;
        }
        
        //Simulate doing other work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    sensor.stop();
    return 0;
}
