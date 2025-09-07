#include "FileChunk.h"
#include "Utils.h"
#include <fstream>
#include <iostream>

FileChunkManager::FileChunkManager(const std::string& filename, size_t fileSize)
    : filename(filename), fileSize(fileSize) {
    totalChunks = (fileSize + CHUNK_SIZE - 1) / CHUNK_SIZE;
    chunks.resize(totalChunks);
    
    for (int i = 0; i < totalChunks; ++i) {
        chunks[i].id = i;
        chunks[i].downloaded = false;
    }
}

bool FileChunkManager::isComplete() const {
    for (const auto& chunk : chunks) {
        if (!chunk.downloaded) {
            return false;
        }
    }
    return true;
}

std::vector<int> FileChunkManager::getMissingChunks() const {
    std::vector<int> missing;
    for (const auto& chunk : chunks) {
        if (!chunk.downloaded) {
            missing.push_back(chunk.id);
        }
    }
    return missing;
}

bool FileChunkManager::setChunk(int chunkId, const std::vector<char>& data) {
    if (chunkId < 0 || chunkId >= totalChunks) {
        return false;
    }
    
    chunks[chunkId].data = data;
    chunks[chunkId].downloaded = true;
    return true;
}

std::vector<char> FileChunkManager::getChunk(int chunkId) const {
    if (chunkId < 0 || chunkId >= totalChunks || !chunks[chunkId].downloaded) {
        return {};
    }
    return chunks[chunkId].data;
}

bool FileChunkManager::saveToFile(const std::string& outputPath) const {
    if (!isComplete()) {
        return false;
    }
    
    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& chunk : chunks) {
        file.write(chunk.data.data(), chunk.data.size());
    }
    
    return file.good();
}

bool FileChunkManager::loadFromFile(const std::string& inputPath) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    for (int i = 0; i < totalChunks; ++i) {
        size_t chunkSize = CHUNK_SIZE;
        if (i == totalChunks - 1) {
            // Last chunk might be smaller
            chunkSize = fileSize - (i * CHUNK_SIZE);
        }
        
        chunks[i].data.resize(chunkSize);
        file.read(chunks[i].data.data(), chunkSize);
        
        if (file.gcount() == static_cast<std::streamsize>(chunkSize)) {
            chunks[i].downloaded = true;
        } else {
            return false;
        }
    }
    
    return true;
}