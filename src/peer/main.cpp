#include "Peer.h"
#include <iostream>
#include <string>
#include <thread>
#include <sstream>

void printUsage() {
    std::cout << "P2P File Sharing Peer" << std::endl;
    std::cout << "Usage: ./peer <tracker_ip> <tracker_port> <peer_port>" << std::endl;
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  share <filename>           - Share a file" << std::endl;
    std::cout << "  download <filename> <path> - Download a file" << std::endl;
    std::cout << "  list                       - List shared files" << std::endl;
    std::cout << "  quit                       - Exit the program" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }
    
    std::string trackerIP = argv[1];
    int trackerPort = std::atoi(argv[2]);
    int peerPort = std::atoi(argv[3]);
    
    Peer peer(trackerIP, trackerPort, peerPort);
    peer.start();
    
    std::cout << "Peer started. Type 'help' for commands." << std::endl;
    
    std::string command;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        
        if (command.empty()) continue;
        
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "help") {
            printUsage();
            
        } else if (cmd == "share") {
            std::string filename;
            iss >> filename;
            
            if (filename.empty()) {
                std::cout << "Usage: share <filename>" << std::endl;
            } else {
                peer.registerFile(filename);
            }
            
        } else if (cmd == "download") {
            std::string filename, outputPath;
            iss >> filename >> outputPath;
            
            if (filename.empty() || outputPath.empty()) {
                std::cout << "Usage: download <filename> <output_path>" << std::endl;
            } else {
                peer.downloadFile(filename, outputPath);
            }
            
        } else if (cmd == "list") {
            peer.listSharedFiles();
            
        } else if (cmd == "quit" || cmd == "exit") {
            break;
            
        } else {
            std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
        }
    }
    
    peer.stop();
    return 0;
}