#!/usr/bin/env bash
set -euo pipefail

PACKAGE_MANAGER=""

if command -v dnf >/dev/null 2>&1; then
  PACKAGE_MANAGER="dnf"
  CPP_PACKAGES=(gcc-c++ make)
  MQTT_PACKAGES=(mosquitto mosquitto-devel)
elif command -v apt-get >/dev/null 2>&1; then
  PACKAGE_MANAGER="apt"
  CPP_PACKAGES=(g++ make)
  MQTT_PACKAGES=(mosquitto libmosquitto-dev)
else
  echo "[ERROR] No supported package manager found. Install dependencies manually."
  echo "[ERROR] Supported managers: dnf, apt-get"
  exit 1
fi

PACKAGES=("${CPP_PACKAGES[@]}" "${MQTT_PACKAGES[@]}")

if command -v sudo >/dev/null 2>&1; then
  SUDO="sudo"
else
  SUDO=""
fi

echo "[INFO] Installing C++ build dependencies: ${CPP_PACKAGES[*]}"
echo "[INFO] Installing MQTT dependencies: ${MQTT_PACKAGES[*]}"
if [[ "$PACKAGE_MANAGER" == "dnf" ]]; then
  $SUDO dnf install -y "${PACKAGES[@]}"
else
  $SUDO apt-get update
  $SUDO apt-get install -y "${PACKAGES[@]}"
fi

if command -v systemctl >/dev/null 2>&1; then
  echo "[INFO] Enabling and starting mosquitto service"
  $SUDO systemctl enable --now mosquitto
else
  echo "[WARN] systemctl is not available. Start broker manually: mosquitto -v"
fi

echo "[INFO] Checking whether broker is listening on port 1883"
if command -v ss >/dev/null 2>&1 && ss -ltn | grep -q ':1883'; then
  echo "[OK] MQTT broker is active on port 1883"
else
  echo "[WARN] Port 1883 was not detected. Check status: systemctl status mosquitto"
fi

echo "[DONE] MQTT setup completed."
