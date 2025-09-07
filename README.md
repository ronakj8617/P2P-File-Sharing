# P2P File Sharing System

A peer-to-peer file sharing system implemented in C++ with support for distributed file downloads, chunking, and multithreaded operations.

## Features

- **Distributed Architecture**: Tracker server coordinates peers, peers share files directly
- **File Chunking**: Files split into 512KB chunks for parallel downloads
- **Multithreaded**: Concurrent downloads from multiple peers
- **Resume Support**: Download missing chunks only
- **Simple Protocol**: Text-based communication protocol

## Architecture

```
+----------------------+       +----------------------+
|     Peer (Node A)    | <----> |     Peer (Node B)    |
|   - Acts as client   |        |   - Acts as client   |
|   - Acts as server   |        |   - Acts as server   |
+----------------------+       +----------------------+
            ^                              ^
            |                              |
            v                              v
                 +--------------------+
                 |   Tracker Server   |
                 | - Maintains list   |
                 |   of active peers  |
                 +--------------------+
```

## Building

```bash
./build.sh
```

Or manually:
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

### 1. Start Tracker Server
```bash
./build/tracker [port]
# Default port is 8080
./build/tracker 8080
```

### 2. Start Peer Nodes
```bash
./build/peer <tracker_ip> <tracker_port> <peer_port>
# Example:
./build/peer 127.0.0.1 8080 9001
```

### 3. Peer Commands
Once a peer is running, use these commands:

- `share <filename>` - Share a file with the network
- `download <filename> <output_path>` - Download a file from peers
- `list` - List currently shared files
- `help` - Show available commands
- `quit` - Exit the peer

## Example Session

**Terminal 1 - Tracker:**
```bash
./build/tracker 8080
```

**Terminal 2 - Peer A:**
```bash
./build/peer 127.0.0.1 8080 9001
> share example.txt
> list
```

**Terminal 3 - Peer B:**
```bash
./build/peer 127.0.0.1 8080 9002
> download example.txt downloaded_example.txt
```

## Protocol

### Tracker Protocol
- `REGISTER <filename> <filesize> <peer_address>` - Register a file
- `LOOKUP <filename>` - Find peers sharing a file
- `UNREGISTER <peer_address>` - Remove peer from tracker

### Peer Protocol
- `GET_CHUNK <filename> <chunk_id>` - Request a file chunk
- Response: chunk size followed by binary data

## Technical Details

- **Chunk Size**: 512KB per chunk
- **Transport**: TCP sockets
- **Threading**: One thread per peer connection
- **File I/O**: Binary file operations with chunk management
- **Error Handling**: Graceful failure handling and reconnection

## Extensions

This implementation can be extended with:

- **Resume Downloads**: Persistent chunk state
- **File Integrity**: SHA-256 checksums
- **Decentralized Tracker**: DHT implementation
- **Encryption**: TLS/SSL for secure transfers
- **GUI Interface**: Qt or web-based interface
- **Bandwidth Control**: Rate limiting and QoS

## Dependencies

- C++17 compiler (GCC/Clang)
- CMake 3.16+
- POSIX sockets (Linux/macOS)
- pthreads

## License

MIT License - see LICENSE file for details.