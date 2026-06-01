#include "DashboardData.h"

DashboardData::DashboardData(QObject* parent) : QObject(parent) {
    rooms[0].name = "Kitchen";
    rooms[1].name = "Living Room";
    rooms[2].name = "Bedroom";
    rooms[3].name = "Bathroom";
    rooms[4].name = "Hallway";
}

AlertLevel DashboardData::roomFireStatus(int idx) const {
    const auto& f = rooms[idx].fire;
    if (!f.valid) return AlertLevel::OK;
    if (f.temperature > FIRE_TEMP_THRESHOLD ||
        f.smokeLevel  > FIRE_SMOKE_THRESHOLD ||
        f.coLevel     > FIRE_CO_THRESHOLD)
        return AlertLevel::ALARM;
    if (f.temperature > FIRE_TEMP_THRESHOLD * 0.7f ||
        f.smokeLevel  > FIRE_SMOKE_THRESHOLD * 0.5f ||
        f.coLevel     > FIRE_CO_THRESHOLD * 0.7f)
        return AlertLevel::WARNING;
    return AlertLevel::OK;
}

AlertLevel DashboardData::healthStatus() const {
    if (!health.valid) return AlertLevel::OK;
    if (health.fallDetected) return AlertLevel::ALARM;
    if (health.heartRate < CRITICAL_HEART_RATE_LOW  ||
        health.heartRate > CRITICAL_HEART_RATE_HIGH ||
        health.spo2      < CRITICAL_SPO2_LOW        ||
        health.systolic  < CRITICAL_SYSTOLIC_LOW    ||
        health.systolic  > CRITICAL_SYSTOLIC_HIGH   ||
        health.diastolic < CRITICAL_DIASTOLIC_LOW   ||
        health.diastolic > CRITICAL_DIASTOLIC_HIGH)
        return AlertLevel::ALARM;
    // Warnings: approaching critical
    if (health.heartRate < CRITICAL_HEART_RATE_LOW + 10 ||
        health.heartRate > CRITICAL_HEART_RATE_HIGH - 10 ||
        health.spo2      < CRITICAL_SPO2_LOW + 5)
        return AlertLevel::WARNING;
    return AlertLevel::OK;
}

AlertLevel DashboardData::overallStatus() const {
    AlertLevel worst = AlertLevel::OK;
    for (int i = 0; i < 5; ++i) {
        auto r = roomFireStatus(i);
        if (r > worst) worst = r;
    }
    auto h = healthStatus();
    if (h > worst) worst = h;
    return worst;
}

int DashboardData::activeAlertCount() const {
    int count = 0;
    for (int i = 0; i < 5; ++i)
        if (roomFireStatus(i) >= AlertLevel::WARNING) ++count;
    if (healthStatus() >= AlertLevel::WARNING) ++count;
    return count;
}

QString DashboardData::lastMotionRoom() const {
    int    best = -1;
    QDateTime bestTime;
    for (int i = 0; i < 5; ++i) {
        if (rooms[i].pir.valid && rooms[i].pir.motionDetected) {
            if (best == -1 || rooms[i].pir.lastDetected > bestTime) {
                best = i;
                bestTime = rooms[i].pir.lastDetected;
            }
        }
    }
    if (best >= 0) return rooms[best].name;
    // fallback: most recent detection
    bestTime = QDateTime();
    for (int i = 0; i < 5; ++i) {
        if (rooms[i].pir.valid && rooms[i].pir.lastDetected.isValid()) {
            if (!bestTime.isValid() || rooms[i].pir.lastDetected > bestTime) {
                best = i;
                bestTime = rooms[i].pir.lastDetected;
            }
        }
    }
    if (best >= 0) return rooms[best].name;
    return "None";
}

QDateTime DashboardData::lastMotionTime() const {
    QDateTime bestTime;
    for (int i = 0; i < 5; ++i) {
        if (rooms[i].pir.valid && rooms[i].pir.lastDetected.isValid())
            if (!bestTime.isValid() || rooms[i].pir.lastDetected > bestTime)
                bestTime = rooms[i].pir.lastDetected;
    }
    return bestTime;
}

void DashboardData::setRoomFire(int idx, const RoomFireData& d) {
    rooms[idx].fire = d;
    rooms[idx].fire.valid = true;
    lastUpdate = QDateTime::currentDateTime();
    emit dataChanged();
}

void DashboardData::setRoomPIR(int idx, const RoomPIRData& d) {
    rooms[idx].pir = d;
    rooms[idx].pir.valid = true;
    if (d.motionDetected)
        rooms[idx].pir.lastDetected = QDateTime::currentDateTime();
    lastUpdate = QDateTime::currentDateTime();
    emit dataChanged();
}

void DashboardData::setHealth(const HealthState& h) {
    health = h;
    health.valid = true;
    lastUpdate = QDateTime::currentDateTime();
    if (healthStatus() == AlertLevel::ALARM)
        emit alarmTriggered(health.fallDetected ? "FALL DETECTED!" : "Critical health values!");
    emit dataChanged();
}

void DashboardData::setGlobals(const GlobalActuatorState& g) {
    globals = g;
    emit dataChanged();
}

void DashboardData::setMonitoring(bool active) {
    monitoringActive = active;
    emit dataChanged();
}

void DashboardData::setRoomSprinkler(int idx, bool on) {
    rooms[idx].actuators.sprinklerOn = on;
    emit dataChanged();
}

void DashboardData::setRoomLight(int idx, bool on, int brightness) {
    rooms[idx].actuators.lightOn = on;
    rooms[idx].actuators.lightBrightness = brightness;
    emit dataChanged();
}

void DashboardData::addLog(AlertLevel lvl, const QString& src, const QString& msg) {
    LogEntry e;
    e.timestamp = QDateTime::currentDateTime();
    e.level = lvl;
    e.source = src;
    e.message = msg;
    m_log.prepend(e);
    if (m_log.size() > 200) m_log.removeLast();
    emit newLogEntry(e);
}

void DashboardData::resetState() {
    // Reset all room sensor + actuator data
    for (int i = 0; i < 5; ++i) {
        rooms[i].fire    = RoomFireData{};
        rooms[i].pir     = RoomPIRData{};
        rooms[i].actuators = RoomActuatorData{};
    }
    // Reset health
    health = HealthState{};
    // Reset global actuators
    globals = GlobalActuatorState{};

    addLog(AlertLevel::OK, "DASHBOARD", "State reset — ready for next scenario");
    emit dataChanged();
}
