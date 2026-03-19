#include "SensorUtility.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <cstring>
#include <fcntl.h>      // File control
#include <errno.h>      // Error number
#include <termios.h>    // POSIX terminal control
#include <unistd.h>     // UNIX standard function

SensorUtility::SensorUtility(const std::string& port, int baud) 
    : portName(port), baudRate(baud), serialFd(-1), running(false), currentRoll(0), currentPitch(0) {
}

SensorUtility::~SensorUtility() {
    stop();
}

void SensorUtility::start() {
    if (running) return;
    running = true;
    workerThread = std::thread(&SensorUtility::loop, this);
}

void SensorUtility::stop() {
    if (!running) return;
    running = false;
    if (workerThread.joinable()) {
        workerThread.join();
    }
    closeSerial();
}

bool SensorUtility::isActive() {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto now = std::chrono::steady_clock::now();
    //active if we heard from Arduino in the last 1000ms
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDataTime).count() < 1000;
}

float SensorUtility::getRoll() {
    std::lock_guard<std::mutex> lock(dataMutex);
    return currentRoll;
}

float SensorUtility::getPitch() {
    std::lock_guard<std::mutex> lock(dataMutex);
    return currentPitch;
}

bool SensorUtility::openSerial() {
    serialFd = open(portName.c_str(), O_RDWR | O_NOCTTY);
    if (serialFd == -1) {
        std::cerr << "[ERROR] Failed to open " << portName << ": " << strerror(errno) << std::endl;
        return false; 
    }

    // Configure Serial Port
    struct termios options;
    tcgetattr(serialFd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    
    options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    options.c_cflag |= (CLOCAL | CREAD | CS8); 
    
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
    options.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL); 
    options.c_oflag &= ~OPOST;

    // Set a 1-second read timeout
    options.c_cc[VMIN] = 0;   
    options.c_cc[VTIME] = 10; 

    tcsetattr(serialFd, TCSANOW, &options);
    
    //wait
    std::this_thread::sleep_for(std::chrono::seconds(2)); 
    tcflush(serialFd, TCIOFLUSH);
    
    return true;
}

void SensorUtility::closeSerial() {
    if (serialFd != -1) {
        close(serialFd);
        serialFd = -1;
    }
}

void SensorUtility::parseData(std::string data) {
    
    // Expected format: "<Pitch,Roll>"
    size_t start = data.find('<');
    size_t end = data.find('>');
    size_t comma = data.find(',');

    if (start != std::string::npos && end != std::string::npos && comma != std::string::npos) {
        if (end > start && comma > start && comma < end) {
            try {
                std::string pitchStr = data.substr(start + 1, comma - (start + 1));
                std::string rollStr = data.substr(comma + 1, end - (comma + 1));

                float p = std::stof(pitchStr);
                float r = std::stof(rollStr);

                // Update shared variables safely
                std::lock_guard<std::mutex> lock(dataMutex);
                currentPitch = p;
                currentRoll = r;
                lastDataTime = std::chrono::steady_clock::now();
            } catch (...) {
                // Parse error, ignore bad packet
            }
        }
    }
}

void SensorUtility::loop() {
    char buffer[256];
    std::string incompleteLine = "";

    while (running) {
        if (serialFd == -1) {
            // Attempt to reconnect every second
            std::cout << "[DEBUG] Attempting to open serial port..." << std::endl;
            if (openSerial()) {
                std::cout << "[DEBUG] Successfully opened port!" << std::endl;
                std::cout << "[SensorUtility] Connected to " << portName << std::endl;
            } else {
                std::cerr << "[DEBUG] ERROR: Could not open port." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
        }

        //Block for up to 1 second waiting for data
        int n = read(serialFd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = 0; // Null terminate
            std::string chunk(buffer);
            incompleteLine += chunk;

            size_t pos;
            while ((pos = incompleteLine.find('\n')) != std::string::npos) {
                std::string line = incompleteLine.substr(0, pos);
                parseData(line);
                incompleteLine.erase(0, pos + 1);
            }
        } else if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // This is normal, do nothing.
            } else {
                std::cerr << "[DEBUG] Read error detected! Errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
                closeSerial();
            }
        } else{
            std::cerr << "[DEBUG] DETACHED n=0"<< std::endl;
            closeSerial();
        }
    }
}
