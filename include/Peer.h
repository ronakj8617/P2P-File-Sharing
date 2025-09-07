#pragma once
#include "FileChunk.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>

class Peer {
public:
    Peer(const std::string& trackerIP, int trackerPort, int peerPort);
    ~Peer();
    
    void start();
    void stop();
    
    bool registerFile(const std::string& filename);
    bool downloadFile(const std::string& filename, const std::string& outputPath);
    void listSharedFiles() const;
    
private:
    // Server functionality
    void serverLoop();
    void handlePeerRequest(int clientSock);
    
    // Client functionality
    bool downloadFromPeers(const std::string& filename, const std::vector<std::string>& peers, 
                          FileChunkManager& chunkManager);
    void downloadWorker(const std::string& peerAddress, const std::string& filename,
                       const std::vector<int>& chunks, FileChunkManager& chunkManager);
    
    // Tracker communication
    std::vector<std::string> lookupPeers(const std::string& filename);
    bool registerWithTracker(const std::string& filename, size_t fileSize);
    
    std::string trackerIP;
    int trackerPort;
    int peerPort;
    int serverSock;
    
    std::atomic<bool> running;
    std::thread serverThread;
    
    std::mutex sharedFilesMutex;
    std::unordered_map<std::string, FileChunkManager> sharedFiles;
    
    std::string getAddress() const;
};