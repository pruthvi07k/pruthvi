# PRUTHVI HOME AUTOMATION

## Project Overview

PRUTHVI Home Automation is a C++ and ESP8266 based
low-voltage home automation prototype.

The Ubuntu C++ application communicates with the
NodeMCU ESP8266 through USB serial communication.

## Hardware

- NodeMCU ESP8266
- CP2102 USB-UART
- LED
- Relay-1
- Relay-2
- Low-voltage test loads
- Resistor
- Jumper wires

## Software

- Ubuntu Linux
- C++17
- Arduino CLI
- ESP8266 Arduino Core
- Arduino/C++ firmware

## GPIO Mapping

| Device | NodeMCU Pin | GPIO |
|--------|-------------|------|
| LED | D1 | GPIO5 |
| Relay-1 | D2 | GPIO4 |
| Relay-2 | D5 | GPIO14 |

## Serial Commands

| Command | Function |
|---------|----------|
| LED_ON | Turn LED ON |
| LED_OFF | Turn LED OFF |
| RELAY1_ON | Turn Relay-1 ON |
| RELAY1_OFF | Turn Relay-1 OFF |
| RELAY2_ON | Turn Relay-2 ON |
| RELAY2_OFF | Turn Relay-2 OFF |

## C++ Application

The Ubuntu application provides:

1. LED ON
2. LED OFF
3. BULB 1 ON
4. BULB 1 OFF
5. BULB 2 ON
6. BULB 2 OFF
0. EXIT

## Communication Flow

Ubuntu C++ Application
        |
        v
USB / CP2102
        |
        v
NodeMCU ESP8266
        |
        +---- D1 ---- LED
        |
        +---- D2 ---- Relay-1
        |
        +---- D5 ---- Relay-2

## How to Run

Connect the NodeMCU ESP8266 to the Ubuntu system through USB.

From the project directory:

```bash
cd ~/pruthvi
./run.sh


eof
