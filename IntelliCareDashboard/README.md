# IntelliCare Dashboard — Qt Creator Projekat

## Preduslovi / Prerequisites

```bash
# Ubuntu/Debian
sudo apt install qtcreator qt6-base-dev libmosquitto-dev mosquitto

# ili Qt5
sudo apt install qtcreator qt5-default libmosquitto-dev mosquitto
```

## Struktura projekta / Project structure

```
IntelliCareDashboard/
├── IntelliCareDashboard.pro       # Qt qmake projekat
├── third_party/
│   └── nlohmann/json.hpp          # Kopija iz intellicare/third_party/
├── resources/
│   └── resources.qrc
└── src/
    ├── main.cpp
    ├── MainWindow.h / .cpp        # Glavni prozor — ceo layout
    ├── DashboardData.h / .cpp     # Data model (mirrors DataModel.hpp)
    ├── MqttClient.h / .cpp        # MQTT subscriber (libmosquitto)
    └── widgets/
        ├── RoomCard.h/.cpp        # Kartica sobe (sidebar + fire grid)
        ├── MetricWidget.h/.cpp    # Jedan metric tile (HR, SpO2, BP...)
        ├── HealthPanel.h/.cpp     # Sekcija A — zdravlje
        ├── MotionPanel.h/.cpp     # Sekcija C — kretanje / PIR
        ├── DeviceControlPanel.h/.cpp  # Desni panel — aktuatori
        └── ActivityLog.h/.cpp     # Log tabela (dole)
```

## Pokretanje / Running

### 1. Pokreni MQTT broker
```bash
mosquitto -v
# ili
sudo systemctl start mosquitto
```

### 2. Pokreni IntelliCare backend (iz intellicare/ foldera)
```bash
cd intellicare/
make
./test_hub &      # Hub — procesira logiku i šalje komande
./test_house      # Simulator — generiše senzorske podatke
```

### 3. Otvori dashboard u Qt Creatoru
```
File → Open File or Project → IntelliCareDashboard.pro
Build → Build All  (Ctrl+B)
Run                (Ctrl+R)
```

Ili iz terminala:
```bash
cd IntelliCareDashboard
qmake && make
./IntelliCareDashboard
```

## Konekcija / How it connects

```
test_house → MQTT → test_hub
                  ↓
          IntelliCareDashboard (subscriber)
          - Subscribuje na SVE iste MQTT topike
          - Parsira isti JSON format (nlohmann/json)
          - Vizualizuje u realnom vremenu
```

**Izmene u intellicare projektu: NEMA.** Dashboard je pasivni MQTT listener.

## MQTT Topics koje Dashboard prati

| Topic                          | Opis                    |
|-------------------------------|-------------------------|
| sensors/wristband/health      | HR, SpO2, BP            |
| sensors/wristband/motion      | Akcelerometar, detekcija pada |
| sensors/{room}/fire           | Temperatura, dim, CO    |
| sensors/{room}/pir            | PIR senzor kretanja     |
| actuators/{room}/sprinkler    | Status sprinklera       |
| actuators/{room}/light        | Status svetla           |
| actuators/lock                | Stanje brave            |
| actuators/dialer              | Hitni pozivi            |

## Threshold vrednosti (identične sa IntelliCare)

| Parametar       | OK         | ALARM         |
|----------------|------------|---------------|
| Heart Rate      | 40–140 BPM | < 40 > 140    |
| SpO2            | > 85%      | < 85%         |
| Systolic BP     | 90–180     | < 90 > 180    |
| Temperature     | < 70°C     | > 70°C        |
| Smoke           | < 500 ppm  | > 500 ppm     |
| CO              | < 35 ppm   | > 35 ppm      |
| Fall magnitude  | < 3.5      | > 3.5         |
