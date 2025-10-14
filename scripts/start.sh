#!/bin/bash

# UmbraEternum Server Startup Script
# Version: 1.3.0

echo "========================================="
echo "  Starting UmbraEternum Server Stack"
echo "========================================="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if build directory exists
if [ ! -d "../build" ]; then
  echo -e "${RED}Error: Build directory not found!${NC}"
  echo "Please run 'cmake && make' first."
  exit 1
fi

# Create logs directory
mkdir -p ../logs

# Check if executables exist
if [ ! -f "../build/bin/umbra_server" ]; then
  echo -e "${RED}Error: umbra_server executable not found!${NC}"
  echo "Please build the project first."
  exit 1
fi

# Start servers
echo -e "${YELLOW}Starting servers...${NC}"
echo ""

# Start Auth Server
if [ -f "../build/bin/auth_server" ]; then
  echo -e "${GREEN}[1/5]${NC} Starting Auth Server..."
  ../build/bin/auth_server &
  AUTH_PID=$!
  sleep 1
fi

# Start World Server
if [ -f "../build/bin/world_server" ]; then
  echo -e "${GREEN}[2/5]${NC} Starting World Server..."
  ../build/bin/world_server &
  WORLD_PID=$!
  sleep 1
fi

# Start Zone Server
if [ -f "../build/bin/zone_server" ]; then
  echo -e "${GREEN}[3/5]${NC} Starting Zone Server (ID: 1)..."
  ../build/bin/zone_server 1 &
  ZONE_PID=$!
  sleep 1
fi

# Start Chat Server
if [ -f "../build/bin/chat_server" ]; then
  echo -e "${GREEN}[4/5]${NC} Starting Chat Server..."
  ../build/bin/chat_server &
  CHAT_PID=$!
  sleep 1
fi

# Start Gateway Server
if [ -f "../build/bin/gateway_server" ]; then
  echo -e "${GREEN}[5/5]${NC} Starting Gateway Server..."
  ../build/bin/gateway_server &
  GATEWAY_PID=$!
  sleep 1
fi

echo ""
echo "========================================="
echo -e "${GREEN}  All servers started successfully!${NC}"
echo "========================================="
echo ""
echo "Server PIDs:"
[ ! -z "$AUTH_PID" ] && echo "  Auth Server:    $AUTH_PID"
[ ! -z "$WORLD_PID" ] && echo "  World Server:   $WORLD_PID"
[ ! -z "$ZONE_PID" ] && echo "  Zone Server:    $ZONE_PID"
[ ! -z "$CHAT_PID" ] && echo "  Chat Server:    $CHAT_PID"
[ ! -z "$GATEWAY_PID" ] && echo "  Gateway Server: $GATEWAY_PID"
echo ""
echo "Press Ctrl+C to stop all servers"
echo ""

# Wait for all servers
wait

