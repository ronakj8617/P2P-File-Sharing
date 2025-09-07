#!/bin/bash

echo "P2P File Sharing System Demo"
echo "============================"
echo ""
echo "This demo will:"
echo "1. Start a tracker server on port 8080"
echo "2. Start two peer nodes on ports 9001 and 9002"
echo "3. Share a test file from peer 1"
echo "4. Download the file to peer 2"
echo ""
echo "You'll need to run commands manually in each terminal."
echo ""

# Check if build exists
if [ ! -f "build/tracker" ] || [ ! -f "build/peer" ]; then
    echo "Building the project first..."
    ./build.sh
fi

echo "Demo Instructions:"
echo "=================="
echo ""
echo "Terminal 1 - Start Tracker:"
echo "  cd $(pwd)"
echo "  ./build/tracker 8080"
echo ""
echo "Terminal 2 - Start Peer A (Seeder):"
echo "  cd $(pwd)"
echo "  ./build/peer 127.0.0.1 8080 9001"
echo "  > share test_files/sample.txt"
echo "  > list"
echo ""
echo "Terminal 3 - Start Peer B (Downloader):"
echo "  cd $(pwd)"
echo "  ./build/peer 127.0.0.1 8080 9002"
echo "  > download sample.txt downloaded_sample.txt"
echo ""
echo "After download completes, verify the file:"
echo "  diff test_files/sample.txt downloaded_sample.txt"
echo ""
echo "Press Enter to continue..."
read

echo "Starting tracker in background for quick test..."
./build/tracker 8080 &
TRACKER_PID=$!

sleep 2

echo "Tracker started (PID: $TRACKER_PID)"
echo "You can now start peer nodes in separate terminals using the commands above."
echo ""
echo "To stop the tracker: kill $TRACKER_PID"