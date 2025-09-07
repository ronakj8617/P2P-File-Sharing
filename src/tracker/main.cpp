#include "Tracker.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <chrono>
#include <thread>

std::unique_ptr<Tracker> tracker;

void signalHandler(int) {
    std::cout << "\nShutting down tracker..." << std::endl;
    if (tracker) {
        tracker->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    tracker = std::make_unique<Tracker>(port);
    tracker->start();
    
    std::cout << "Tracker running on port " << port << ". Press Ctrl+C to stop." << std::endl;
    
    // Keep the main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}