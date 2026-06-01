#pragma once

#include <QString>
#include <QDateTime>
#include <QObject>
#include <array>

// ── Mirrors IntelliCare core/DataModel.hpp thresholds exactly ──────────────
constexpr int   CRITICAL_HEART_RATE_LOW   = 40;
constexpr int   CRITICAL_HEART_RATE_HIGH  = 140;
constexpr int   CRITICAL_SPO2_LOW         = 85;
constexpr int   CRITICAL_SYSTOLIC_LOW     = 90;
constexpr int   CRITICAL_SYSTOLIC_HIGH    = 180;
constexpr int   CRITICAL_DIASTOLIC_LOW    = 60;
constexpr int   CRITICAL_DIASTOLIC_HIGH   = 120;

constexpr float FIRE_TEMP_THRESHOLD       = 70.0f;
constexpr float FIRE_SMOKE_THRESHOLD      = 500.0f;
constexpr int   FIRE_CO_THRESHOLD         = 35;

constexpr float FALL_MAGNITUDE_THRESHOLD  = 3.5f;

// ── Enums ───────────────────────────────────────────────────────────────────
enum class AlertLevel { OK, WARNING, ALARM };
enum class RoomIndex  { KITCHEN=0, LIVING_ROOM=1, BEDROOM=2, BATHROOM=3, HALLWAY=4, COUNT=5 };

// ── Per-room state ──────────────────────────────────────────────────────────
struct RoomFireData {
    float temperature = 22.0f;
    float smokeLevel  = 0.0f;
    int   coLevel     = 0;
    bool  valid       = false;
};

struct RoomPIRData {
    bool motionDetected = false;
    QDateTime lastDetected;
    bool valid = false;
};

struct RoomActuatorData {
    bool sprinklerOn     = false;
    bool lightOn         = false;
    int  lightBrightness = 0;
    int  speakerLevel    = 0;
};

struct RoomState {
    QString         name;
    RoomFireData    fire;
    RoomPIRData     pir;
    RoomActuatorData actuators;
};

// ── Wristband / health ───────────────────────────────────────────────────────
struct HealthState {
    int  heartRate  = 72;
    int  spo2       = 98;
    int  systolic   = 120;
    int  diastolic  = 80;
    bool fallDetected = false;
    float fallMagnitude = 0.0f;
    QDateTime lastUpdate;
    bool valid = false;
};

// ── Lock / dialer ────────────────────────────────────────────────────────────
struct GlobalActuatorState {
    bool locked           = true;
    bool dialerBusy       = false;
    QString dialerAction;
};

// ── Activity log entry ───────────────────────────────────────────────────────
struct LogEntry {
    QDateTime  timestamp;
    AlertLevel level;
    QString    source;
    QString    message;
};
Q_DECLARE_METATYPE(LogEntry)

// ── Complete dashboard state ─────────────────────────────────────────────────
class DashboardData : public QObject {
    Q_OBJECT
public:
    explicit DashboardData(QObject* parent = nullptr);

    // 5 rooms: KITCHEN, LIVING_ROOM, BEDROOM, BATHROOM, HALLWAY
    std::array<RoomState, 5> rooms;

    HealthState         health;
    GlobalActuatorState globals;

    bool monitoringActive = false;
    QDateTime lastUpdate;

    // Derived helpers
    AlertLevel overallStatus()   const;
    AlertLevel roomFireStatus(int idx) const;
    AlertLevel healthStatus()    const;
    int        activeAlertCount()const;
    QString    lastMotionRoom()  const;
    QDateTime  lastMotionTime()  const;

    // Mutators (emit dataChanged)
    void setRoomFire(int idx, const RoomFireData& d);
    void setRoomPIR (int idx, const RoomPIRData&  d);
    void setHealth  (const HealthState& h);
    void setGlobals (const GlobalActuatorState& g);
    void setMonitoring(bool active);
    void setRoomSprinkler(int idx, bool on);
    void setRoomLight    (int idx, bool on, int brightness=100);

    void addLog(AlertLevel lvl, const QString& src, const QString& msg);
    void resetState();  // clears all sensor/actuator state after scenario ends
    const QList<LogEntry>& logEntries() const { return m_log; }

signals:
    void dataChanged();
    void newLogEntry(const LogEntry& e);
    void alarmTriggered(const QString& msg);

private:
    QList<LogEntry> m_log;
};
