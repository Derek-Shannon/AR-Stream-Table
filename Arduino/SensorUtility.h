#ifndef SENSOR_UTILITY_H
#define SENSOR_UTILITY_H

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class SensorUtility {
public:
    // Constructor takes the serial port 
    SensorUtility(const std::string& portName, int baudRate = 115200);
    ~SensorUtility();

    // Starts the background monitoring thread
    void start();
    
    // Stops the thread gracefully
    void stop();

    // Getters for the data
    float getRoll();
    float getPitch();
    
    // Returns true if data has been received recently (within 1 second)
    bool isActive();

private:
    void loop();               // The main loop for the background thread
    bool openSerial();         // Helper to open connection
    void closeSerial();        // Helper to close connection
    void parseData(std::string data);

    std::string portName;
    int baudRate;
    int serialFd;              // File descriptor for serial port

    std::thread workerThread;
    std::atomic<bool> running; // Flag to keep thread running

    // Shared data (protected by mutex)
    float currentRoll;
    float currentPitch;
    std::chrono::steady_clock::time_point lastDataTime;
    mutable std::mutex dataMutex; // Protects roll, pitch, and time
};

#endif