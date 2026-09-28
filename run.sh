#!/bin/bash

cd "$(dirname "$0")"

echo "========================================"
echo "      PRUTHVI HOME AUTOMATION"
echo "========================================"
echo

ESP_IP="10.62.156.207"
WEB_PORT="8080"

echo "[1/3] Checking NodeMCU..."
if curl -s --max-time 3 "http://${ESP_IP}/api/status" >/dev/null; then
    echo "NodeMCU: ONLINE"
    echo "ESP8266: ${ESP_IP}"
else
    echo "NodeMCU: OFFLINE"
    echo "Check USB power / Wi-Fi connection."
    exit 1
fi

echo
echo "[2/3] Checking Web Server..."

if curl -s --max-time 2 "http://127.0.0.1:${WEB_PORT}" >/dev/null; then
    echo "Web Server: ALREADY RUNNING"
else
    echo "Starting Web Server..."
    python3 -m http.server ${WEB_PORT} --directory web >/tmp/pruthvi_web.log 2>&1 &
    sleep 1
    echo "Web Server: STARTED"
fi

echo
echo "[3/3] PRUTHVI READY"
echo "========================================"
echo "Dashboard : http://localhost:${WEB_PORT}"
echo "NodeMCU   : http://${ESP_IP}/"
echo "API       : http://${ESP_IP}/api/status"
echo "========================================"
