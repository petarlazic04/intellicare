#pragma once

#include <string>
#include "../third_party/nlohmann/json.hpp"

using json = nlohmann::json;

//Actuator limits
#define MAX_SPEAKER_VOLUME 100
#define MAX_LIGHT_BRIGHTNESS 100

//Health critical values
#define CRITICAL_HEART_RATE_LOW 40
#define CRITICAL_HEART_RATE_HIGH 140
#define CRITICAL_SPO2_LOW 85
#define CRITICAL_SYSTOLIC_LOW 90 
#define CRITICAL_SYSTOLIC_HIGH 180
#define CRITICAL_DIASTOLIC_LOW 60  
#define CRITICAL_DIASTOLIC_HIGH 120

//Fire detection thresholds
#define FIRE_TEMP_THRESHOLD 70.0f
#define FIRE_SMOKE_THRESHOLD 500.0f
#define FIRE_CO_THRESHOLD 35

//Fall detection
#define FALL_MAGNITUDE_THRESHOLD 3.5f
#define FALL_DEBOUNCE_MS 250


//PIR motion timeout
#define PIR_MOTION_TIMEOUT_MS 30000

constexpr int MQTT_DEFAULT_QOS = 2;

enum class DeviceType {
  WRISTBAND,
  FIRE_SENSOR,
  PIR_SENSOR,
  DOOR_LOCK,
  SPRINKLER,
  LIGHT,
  SPEAKER,
  DIALER
};


enum class Room {
  KITCHEN,
  LIVING_ROOM,
  BEDROOM,
  BATHROOM,
  HALLWAY
};


enum class DeviceActionType {
  LOCK,
  UNLOCK,
  TURN_ON,
  TURN_OFF,
  START,
  STOP,
  SET_LEVEL,
  DIAL_AMBULANCE,
  DIAL_FIRE_BRIGADE,
  NOTIFY_FAMILY
};

enum class PayloadType {
  FIRE_DETECTOR_PAYLOAD,
  HEALTH_PAYLOAD,
  MOTION_PAYLOAD,
  PIR_PAYLOAD,
  COMMAND,
  UNKNOWN
};

enum class DialerActionType {
  DIAL_AMBULANCE,
  DIAL_FIRE_BRIGADE,
  NOTIFY_FAMILY
 };


struct FireDetectorData {
  float temperature;
  float smokeLevel;
  int coLevel;
};

struct HealthData {
  int heartRate;
  int spo2;

  int systolic; //gornji pritisak
  int diastolic; //donji pritisak
};

struct MotionData {
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;

  float magnitude;     // ukupna sila kretanja
};


struct PIRData {
  bool motionDetected;
};


struct DeviceCommand {
  DeviceActionType actionType;
  std::string value;
};


struct Payload {
  PayloadType payloadType;
  json data;
};

struct RoomState {
  FireDetectorData fire;
  PIRData pir;
  bool sprinklersOn;
  bool lightOn;
  int lightLevel;
  int speakerLevel;
};

struct Message {
  std::string deviceId;
  DeviceType deviceType;
  Room location;
  long timestamp;
  Payload payload;
};

