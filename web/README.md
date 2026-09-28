# PRUTHVI Web UI

Responsive browser dashboard for PRUTHVI Home Automation.

## Run UI

From the project root:

```bash
cd ~/pruthvi
python3 -m http.server 8080 --directory web
```

Open:

```text
http://localhost:8080
```

## Current stage

This is a UI prototype. Device buttons update browser state locally.

The JavaScript is prepared for the future C++ API:

```text
POST /api/device
```

Example JSON:

```json
{
  "device": "led",
  "command": "LED_ON",
  "state": "ON"
}
```

## NodeMCU commands

LED: `LED_ON`, `LED_OFF`
Bulb 1: `RELAY1_ON`, `RELAY1_OFF`
Bulb 2: `RELAY2_ON`, `RELAY2_OFF`

The relay/load side should remain low-voltage during prototype testing.
