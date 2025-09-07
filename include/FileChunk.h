#pragma once
#include <vector>
#include <string>

constexpr size_t CHUNK_SIZE = 512 * 1024; // 512KB chunks

struct Chunk {
    int id;
    std::vector<char> data;
    bool downloaded;
    
    Chunk(int chunkId = 0) : id(chunkId), downloaded(false) {}
};

struct FileInfo {
    std::string filename;
    size_t fileSize;
    int totalChunks;
    std::vector<std::string> peers; // IP:Port format
    
    FileInfo() : fileSize(0), totalChunks(0) {}
    FileInfo(const std::string& name, size_t size) 
        : filename(name), fileSize(size) {
        totalChunks = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    }
};

class FileChunkManager {
public:
    FileChunkManager() : fileSize(0), totalChunks(0) {}
    FileChunkManager(const std::string& filename, size_t fileSize);
    
    bool isComplete() const;
    std::vector<int> getMissingChunks() const;
    bool setChunk(int chunkId, const std::vector<char>& data);
    std::vector<char> getChunk(int chunkId) const;
    bool saveToFile(const std::string& outputPath) const;
    bool loadFromFile(const std::string& inputPath);
    
    int getTotalChunks() const { return totalChunks; }
    size_t getFileSize() const { return fileSize; }

private:
    std::string filename;
    size_t fileSize;
    int totalChunks;
    std::vector<Chunk> chunks;
};