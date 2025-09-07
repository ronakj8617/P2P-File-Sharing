#pragma once
#include <string>
#include <vector>

namespace Utils {
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    bool fileExists(const std::string& filename);
    size_t getFileSize(const std::string& filename);
    std::string getLocalIP();
    
    // Socket utilities
    int createTCPSocket();
    int createServerSocket(int port);
    bool connectToServer(int sock, const std::string& ip, int port);
    std::string receiveMessage(int sock);
    bool sendMessage(int sock, const std::string& message);
    std::vector<char> receiveData(int sock, size_t expectedSize);
    bool sendData(int sock, const std::vector<char>& data);
}