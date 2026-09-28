#!/bin/bash

cd "$(dirname "$0")/home_automation"

echo "================================"
echo "     PRUTHVI HOME AUTOMATION"
echo "================================"
echo

SERIAL_PORT=$(ls /dev/ttyUSB* 2>/dev/null | head -n 1)

if [ -z "$SERIAL_PORT" ]; then
    echo "NodeMCU not detected."
    echo "Connect the NodeMCU through USB."
    exit 1
fi

if [ ! -x ./home_automation ]; then
    echo "Building application..."
    g++ -std=c++17 -Wall -Wextra main.cpp -o home_automation
fi

echo "NodeMCU detected on $SERIAL_PORT"
echo

./home_automation
