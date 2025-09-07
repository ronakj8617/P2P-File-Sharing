#include "Utils.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <cstring>
#include <sys/types.h>

namespace Utils {

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

size_t getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;
    return file.tellg();
}

std::string getLocalIP() {
    struct ifaddrs *ifaddrs_ptr = nullptr;
    std::string result = "127.0.0.1";
    
    if (getifaddrs(&ifaddrs_ptr) == 0) {
        for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
                std::string ip = inet_ntoa(addr_in->sin_addr);
                
                // Skip loopback
                if (ip != "127.0.0.1" && std::strncmp(ifa->ifa_name, "lo", 2) != 0) {
                    result = ip;
                    break;
                }
            }
        }
        freeifaddrs(ifaddrs_ptr);
    }
    
    return result;
}

int createTCPSocket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

int createServerSocket(int port) {
    int sock = createTCPSocket();
    if (sock < 0) return -1;
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    
    if (listen(sock, 10) < 0) {
        close(sock);
        return -1;
    }
    
    return sock;
}

bool connectToServer(int sock, const std::string& ip, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    
    return connect(sock, (struct sockaddr*)&addr, sizeof(addr)) >= 0;
}

std::string receiveMessage(int sock) {
    char buffer[4096];
    ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) return "";
    
    buffer[bytes] = '\0';
    return std::string(buffer);
}

bool sendMessage(int sock, const std::string& message) {
    return send(sock, message.c_str(), message.length(), 0) > 0;
}

std::vector<char> receiveData(int sock, size_t expectedSize) {
    std::vector<char> data(expectedSize);
    size_t totalReceived = 0;
    
    while (totalReceived < expectedSize) {
        ssize_t bytes = recv(sock, data.data() + totalReceived, 
                           expectedSize - totalReceived, 0);
        if (bytes <= 0) break;
        totalReceived += bytes;
    }
    
    data.resize(totalReceived);
    return data;
}

bool sendData(int sock, const std::vector<char>& data) {
    size_t totalSent = 0;
    
    while (totalSent < data.size()) {
        ssize_t bytes = send(sock, data.data() + totalSent, 
                           data.size() - totalSent, 0);
        if (bytes <= 0) return false;
        totalSent += bytes;
    }
    
    return true;
}

} // namespace Utils