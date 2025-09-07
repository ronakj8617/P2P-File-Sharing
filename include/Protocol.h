#pragma once
#include <string>
#include <vector>

// Protocol messages
namespace Protocol {
    // Tracker messages
    constexpr const char* REGISTER = "REGISTER";
    constexpr const char* LOOKUP = "LOOKUP";
    constexpr const char* UNREGISTER = "UNREGISTER";
    
    // Peer messages
    constexpr const char* GET_CHUNK = "GET_CHUNK";
    constexpr const char* CHUNK_DATA = "CHUNK_DATA";
    constexpr const char* ERROR_MSG = "ERROR";
    
    // Message parsing
    struct RegisterMessage {
        std::string filename;
        size_t fileSize;
        std::string peerAddress;
    };
    
    struct LookupMessage {
        std::string filename;
    };
    
    struct GetChunkMessage {
        std::string filename;
        int chunkId;
    };
    
    // Message serialization/deserialization
    std::string serializeRegister(const RegisterMessage& msg);
    std::string serializeLookup(const LookupMessage& msg);
    std::string serializeGetChunk(const GetChunkMessage& msg);
    
    RegisterMessage parseRegister(const std::string& data);
    LookupMessage parseLookup(const std::string& data);
    GetChunkMessage parseGetChunk(const std::string& data);
}