#include "Peer.h"
#include "Protocol.h"
#include "Utils.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

Peer::Peer(const std::string& trackerIP, int trackerPort, int peerPort)
    : trackerIP(trackerIP), trackerPort(trackerPort), peerPort(peerPort), 
      serverSock(-1), running(false) {}

Peer::~Peer() {
    stop();
}

void Peer::start() {
    serverSock = Utils::createServerSocket(peerPort);
    if (serverSock < 0) {
        std::cerr << "Failed to create server socket on port " << peerPort << std::endl;
        return;
    }
    
    running = true;
    serverThread = std::thread(&Peer::serverLoop, this);
    
    std::cout << "Peer started on port " << peerPort << std::endl;
    std::cout << "Address: " << getAddress() << std::endl;
}

void Peer::stop() {
    running = false;
    
    if (serverSock >= 0) {
        close(serverSock);
        serverSock = -1;
    }
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

std::string Peer::getAddress() const {
    return Utils::getLocalIP() + ":" + std::to_string(peerPort);
}

bool Peer::registerFile(const std::string& filename) {
    if (!Utils::fileExists(filename)) {
        std::cerr << "File not found: " << filename << std::endl;
        return false;
    }
    
    size_t fileSize = Utils::getFileSize(filename);
    FileChunkManager chunkManager(filename, fileSize);
    
    if (!chunkManager.loadFromFile(filename)) {
        std::cerr << "Failed to load file: " << filename << std::endl;
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(sharedFilesMutex);
        sharedFiles[filename] = std::move(chunkManager);
    }
    
    if (!registerWithTracker(filename, fileSize)) {
        std::cerr << "Failed to register with tracker" << std::endl;
        return false;
    }
    
    std::cout << "Successfully registered file: " << filename << std::endl;
    return true;
}

bool Peer::registerWithTracker(const std::string& filename, size_t fileSize) {
    int sock = Utils::createTCPSocket();
    if (sock < 0) return false;
    
    if (!Utils::connectToServer(sock, trackerIP, trackerPort)) {
        close(sock);
        return false;
    }
    
    Protocol::RegisterMessage msg;
    msg.filename = filename;
    msg.fileSize = fileSize;
    msg.peerAddress = getAddress();
    
    std::string request = Protocol::serializeRegister(msg);
    Utils::sendMessage(sock, request);
    
    std::string response = Utils::receiveMessage(sock);
    close(sock);
    
    return response == "OK";
}

std::vector<std::string> Peer::lookupPeers(const std::string& filename) {
    int sock = Utils::createTCPSocket();
    if (sock < 0) return {};
    
    if (!Utils::connectToServer(sock, trackerIP, trackerPort)) {
        close(sock);
        return {};
    }
    
    Protocol::LookupMessage msg;
    msg.filename = filename;
    
    std::string request = Protocol::serializeLookup(msg);
    Utils::sendMessage(sock, request);
    
    std::string response = Utils::receiveMessage(sock);
    close(sock);
    
    auto parts = Utils::split(response, ' ');
    if (parts.size() < 2 || parts[0] != "PEERS") {
        return {};
    }
    
    std::vector<std::string> peers;
    for (size_t i = 2; i < parts.size(); ++i) {
        peers.push_back(parts[i]);
    }
    
    return peers;
}

bool Peer::downloadFile(const std::string& filename, const std::string& outputPath) {
    // Get file size and peers from tracker
    int sock = Utils::createTCPSocket();
    if (sock < 0) return false;
    
    if (!Utils::connectToServer(sock, trackerIP, trackerPort)) {
        close(sock);
        return false;
    }
    
    Protocol::LookupMessage msg;
    msg.filename = filename;
    
    std::string request = Protocol::serializeLookup(msg);
    Utils::sendMessage(sock, request);
    
    std::string response = Utils::receiveMessage(sock);
    close(sock);
    
    auto parts = Utils::split(response, ' ');
    if (parts.size() < 3 || parts[0] != "PEERS") {
        std::cerr << "No peers found for file: " << filename << std::endl;
        return false;
    }
    
    size_t fileSize = std::stoull(parts[1]);
    std::vector<std::string> peers;
    for (size_t i = 2; i < parts.size(); ++i) {
        peers.push_back(parts[i]);
    }
    
    std::cout << "Found " << peers.size() << " peers for " << filename 
              << " (size: " << fileSize << " bytes)" << std::endl;
    
    FileChunkManager chunkManager(filename, fileSize);
    
    if (downloadFromPeers(filename, peers, chunkManager)) {
        if (chunkManager.saveToFile(outputPath)) {
            std::cout << "Successfully downloaded: " << filename << std::endl;
            return true;
        }
    }
    
    std::cerr << "Failed to download file: " << filename << std::endl;
    return false;
}

bool Peer::downloadFromPeers(const std::string& filename, const std::vector<std::string>& peers, 
                            FileChunkManager& chunkManager) {
    auto missingChunks = chunkManager.getMissingChunks();
    if (missingChunks.empty()) {
        return true; // Already complete
    }
    
    // Distribute chunks among peers
    std::vector<std::thread> workers;
    std::mutex chunkMutex;
    
    for (const auto& peerAddress : peers) {
        std::vector<int> peerChunks;
        
        // Simple round-robin distribution
        size_t peerIndex = std::distance(peers.begin(), 
                                        std::find(peers.begin(), peers.end(), peerAddress));
        for (size_t i = 0; i < missingChunks.size(); ++i) {
            if (i % peers.size() == peerIndex) {
                peerChunks.push_back(missingChunks[i]);
            }
        }
        
        if (!peerChunks.empty()) {
            workers.emplace_back(&Peer::downloadWorker, this, peerAddress, 
                               filename, peerChunks, std::ref(chunkManager));
        }
    }
    
    // Wait for all workers to complete
    for (auto& worker : workers) {
        worker.join();
    }
    
    return chunkManager.isComplete();
}

void Peer::downloadWorker(const std::string& peerAddress, const std::string& filename,
                         const std::vector<int>& chunks, FileChunkManager& chunkManager) {
    auto addressParts = Utils::split(peerAddress, ':');
    if (addressParts.size() != 2) return;
    
    std::string ip = addressParts[0];
    int port = std::stoi(addressParts[1]);
    
    for (int chunkId : chunks) {
        int sock = Utils::createTCPSocket();
        if (sock < 0) continue;
        
        if (!Utils::connectToServer(sock, ip, port)) {
            close(sock);
            continue;
        }
        
        Protocol::GetChunkMessage msg;
        msg.filename = filename;
        msg.chunkId = chunkId;
        
        std::string request = Protocol::serializeGetChunk(msg);
        Utils::sendMessage(sock, request);
        
        // Receive chunk size first
        std::string sizeStr = Utils::receiveMessage(sock);
        if (sizeStr.empty()) {
            close(sock);
            continue;
        }
        
        size_t chunkSize = std::stoull(sizeStr);
        if (chunkSize > 0) {
            auto chunkData = Utils::receiveData(sock, chunkSize);
            if (chunkData.size() == chunkSize) {
                chunkManager.setChunk(chunkId, chunkData);
                std::cout << "Downloaded chunk " << chunkId << " from " << peerAddress << std::endl;
            }
        }
        
        close(sock);
    }
}

void Peer::serverLoop() {
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSock < 0) {
            if (running) {
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }
        
        std::thread clientThread(&Peer::handlePeerRequest, this, clientSock);
        clientThread.detach();
    }
}

void Peer::handlePeerRequest(int clientSock) {
    std::string message = Utils::receiveMessage(clientSock);
    if (message.empty()) {
        close(clientSock);
        return;
    }
    
    auto msg = Protocol::parseGetChunk(message);
    if (msg.filename.empty()) {
        Utils::sendMessage(clientSock, "0"); // Send 0 size for error
        close(clientSock);
        return;
    }
    
    std::lock_guard<std::mutex> lock(sharedFilesMutex);
    auto it = sharedFiles.find(msg.filename);
    if (it == sharedFiles.end()) {
        Utils::sendMessage(clientSock, "0");
        close(clientSock);
        return;
    }
    
    auto chunkData = it->second.getChunk(msg.chunkId);
    if (chunkData.empty()) {
        Utils::sendMessage(clientSock, "0");
        close(clientSock);
        return;
    }
    
    // Send chunk size first, then data
    Utils::sendMessage(clientSock, std::to_string(chunkData.size()));
    Utils::sendData(clientSock, chunkData);
    
    close(clientSock);
}

void Peer::listSharedFiles() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(sharedFilesMutex));
    
    if (sharedFiles.empty()) {
        std::cout << "No files currently shared." << std::endl;
        return;
    }
    
    std::cout << "Shared files:" << std::endl;
    for (const auto& [filename, chunkManager] : sharedFiles) {
        std::cout << "  " << filename << " (" << chunkManager.getFileSize() << " bytes)" << std::endl;
    }
}