#pragma once
#include "FileChunk.h"
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>

class Tracker {
public:
    Tracker(int port);
    ~Tracker();
    
    void start();
    void stop();
    
private:
    void serverLoop();
    void handleClient(int clientSock);
    void registerFile(const std::string& filename, size_t fileSize, const std::string& peerAddress);
    std::vector<std::string> lookupFile(const std::string& filename);
    void unregisterPeer(const std::string& peerAddress);
    
    int port;
    int serverSock;
    std::atomic<bool> running;
    std::thread serverThread;
    
    std::mutex registryMutex;
    std::unordered_map<std::string, FileInfo> fileRegistry;
    std::unordered_map<std::string, std::vector<std::string>> peerFiles; // peer -> files
};