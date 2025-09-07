#include "Protocol.h"
#include "Utils.h"
#include <sstream>

namespace Protocol {

std::string serializeRegister(const RegisterMessage& msg) {
    return std::string(REGISTER) + " " + msg.filename + " " + 
           std::to_string(msg.fileSize) + " " + msg.peerAddress;
}

std::string serializeLookup(const LookupMessage& msg) {
    return std::string(LOOKUP) + " " + msg.filename;
}

std::string serializeGetChunk(const GetChunkMessage& msg) {
    return std::string(GET_CHUNK) + " " + msg.filename + " " + std::to_string(msg.chunkId);
}

RegisterMessage parseRegister(const std::string& data) {
    auto parts = Utils::split(data, ' ');
    RegisterMessage msg;
    
    if (parts.size() >= 4 && parts[0] == REGISTER) {
        msg.filename = parts[1];
        msg.fileSize = std::stoull(parts[2]);
        msg.peerAddress = parts[3];
    }
    
    return msg;
}

LookupMessage parseLookup(const std::string& data) {
    auto parts = Utils::split(data, ' ');
    LookupMessage msg;
    
    if (parts.size() >= 2 && parts[0] == LOOKUP) {
        msg.filename = parts[1];
    }
    
    return msg;
}

GetChunkMessage parseGetChunk(const std::string& data) {
    auto parts = Utils::split(data, ' ');
    GetChunkMessage msg;
    
    if (parts.size() >= 3 && parts[0] == GET_CHUNK) {
        msg.filename = parts[1];
        msg.chunkId = std::stoi(parts[2]);
    }
    
    return msg;
}

}