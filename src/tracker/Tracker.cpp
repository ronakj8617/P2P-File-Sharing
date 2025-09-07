#include "Tracker.h"
#include "Protocol.h"
#include "Utils.h"
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

Tracker::Tracker(int port) : port(port), serverSock(-1), running(false) {}

Tracker::~Tracker() {
    stop();
}

void Tracker::start() {
    serverSock = Utils::createServerSocket(port);
    if (serverSock < 0) {
        std::cerr << "Failed to create server socket on port " << port << std::endl;
        return;
    }
    
    running = true;
    serverThread = std::thread(&Tracker::serverLoop, this);
    
    std::cout << "Tracker started on port " << port << std::endl;
}

void Tracker::stop() {
    running = false;
    
    if (serverSock >= 0) {
        close(serverSock);
        serverSock = -1;
    }
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void Tracker::serverLoop() {
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
        
        // Handle each client in a separate thread
        std::thread clientThread(&Tracker::handleClient, this, clientSock);
        clientThread.detach();
    }
}

void Tracker::handleClient(int clientSock) {
    std::string message = Utils::receiveMessage(clientSock);
    if (message.empty()) {
        close(clientSock);
        return;
    }
    
    auto parts = Utils::split(message, ' ');
    if (parts.empty()) {
        close(clientSock);
        return;
    }
    
    std::string response;
    
    if (parts[0] == Protocol::REGISTER && parts.size() >= 4) {
        auto msg = Protocol::parseRegister(message);
        registerFile(msg.filename, msg.fileSize, msg.peerAddress);
        response = "OK";
        std::cout << "Registered file: " << msg.filename << " from " << msg.peerAddress << std::endl;
        
    } else if (parts[0] == Protocol::LOOKUP && parts.size() >= 2) {
        auto msg = Protocol::parseLookup(message);
        auto peers = lookupFile(msg.filename);
        
        if (peers.empty()) {
            response = "NOT_FOUND";
        } else {
            auto it = fileRegistry.find(msg.filename);
            response = "PEERS " + std::to_string(it->second.fileSize);
            for (const auto& peer : peers) {
                response += " " + peer;
            }
        }
        
    } else if (parts[0] == Protocol::UNREGISTER && parts.size() >= 2) {
        unregisterPeer(parts[1]);
        response = "OK";
        
    } else {
        response = "ERROR Invalid command";
    }
    
    Utils::sendMessage(clientSock, response);
    close(clientSock);
}

void Tracker::registerFile(const std::string& filename, size_t fileSize, const std::string& peerAddress) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto& fileInfo = fileRegistry[filename];
    if (fileInfo.filename.empty()) {
        fileInfo = FileInfo(filename, fileSize);
    }
    
    // Add peer if not already present
    auto it = std::find(fileInfo.peers.begin(), fileInfo.peers.end(), peerAddress);
    if (it == fileInfo.peers.end()) {
        fileInfo.peers.push_back(peerAddress);
    }
    
    // Track files per peer
    peerFiles[peerAddress].push_back(filename);
}

std::vector<std::string> Tracker::lookupFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    auto it = fileRegistry.find(filename);
    if (it != fileRegistry.end()) {
        return it->second.peers;
    }
    
    return {};
}

void Tracker::unregisterPeer(const std::string& peerAddress) {
    std::lock_guard<std::mutex> lock(registryMutex);
    
    // Remove peer from all files
    for (auto& [filename, fileInfo] : fileRegistry) {
        auto it = std::find(fileInfo.peers.begin(), fileInfo.peers.end(), peerAddress);
        if (it != fileInfo.peers.end()) {
            fileInfo.peers.erase(it);
        }
    }
    
    // Remove peer's file list
    peerFiles.erase(peerAddress);
}