#include "MqttClient.h"
#include <QDebug>
#include <cmath>

// Use nlohmann/json for parsing (same lib as IntelliCare)
#include "../third_party/nlohmann/json.hpp"
using json = nlohmann::json;

// ── room name → index mapping (matches IntelliCare EnumTraits) ──────────────
static const QStringList ROOM_KEYS = {
    "kitchen", "living_room", "bedroom", "bathroom", "hallway"
};
static const QStringList ROOM_NAMES = {
    "Kitchen", "Living Room", "Bedroom", "Bathroom", "Hallway"
};

MqttClient::MqttClient(DashboardData* data, QObject* parent)
    : QObject(parent), m_data(data)
{
    mosquitto_lib_init();

    // Unique client ID for dashboard
    QString clientId = "intellicare_dashboard_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    m_mosq = mosquitto_new(clientId.toUtf8().constData(), true, this);

    mosquitto_connect_callback_set   (m_mosq, onConnectStatic);
    mosquitto_disconnect_callback_set(m_mosq, onDisconnectStatic);
    mosquitto_message_callback_set   (m_mosq, onMessageStatic);

    // Poll mosquitto loop via Qt timer (non-blocking)
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MqttClient::pollMosquitto);
}

MqttClient::~MqttClient() {
    if (m_timer) m_timer->stop();
    if (m_mosq) {
        mosquitto_loop_stop(m_mosq, true);
        mosquitto_disconnect(m_mosq);
        mosquitto_destroy(m_mosq);
    }
    mosquitto_lib_cleanup();
}

bool MqttClient::connectToBroker(const QString& host, int port) {
    int rc = mosquitto_connect(m_mosq, host.toUtf8().constData(), port, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        emit connectionError(QString("MQTT connect failed: %1").arg(mosquitto_strerror(rc)));
        return false;
    }
    mosquitto_loop_start(m_mosq);
    m_timer->start(100);
    return true;
}

void MqttClient::disconnect() {
    m_timer->stop();
    mosquitto_loop_stop(m_mosq, true);
    mosquitto_disconnect(m_mosq);
}

void MqttClient::publish(const QString& topic, const QString& payload) {
    if (!m_connected) return;
    QByteArray ba = payload.toUtf8();
    mosquitto_publish(m_mosq, nullptr, topic.toUtf8().constData(),
                      ba.size(), (const uint8_t*)ba.constData(), 0, false);
}

void MqttClient::pollMosquitto() {
    mosquitto_loop(m_mosq, 0, 1);
}

// ── Static callbacks ──────────────────────────────────────────────────────────
void MqttClient::onConnectStatic(struct mosquitto*, void* obj, int rc) {
    MqttClient* self = static_cast<MqttClient*>(obj);
    if (rc == 0) {
        self->m_connected = true;

        // Subscribe to all IntelliCare topics (QoS 2 — same as backend)
        auto sub = [&](const char* topic) {
            mosquitto_subscribe(self->m_mosq, nullptr, topic, 2);
        };

        sub("sensors/wristband/health");
        sub("sensors/wristband");        // some scenarios use this short form
        sub("sensors/wristband/motion");

        for (const auto& room : ROOM_KEYS) {
            sub(("sensors/" + room + "/fire").toUtf8().constData());
            sub(("sensors/" + room + "/pir").toUtf8().constData());
            sub(("actuators/" + room + "/sprinkler").toUtf8().constData());
            sub(("actuators/" + room + "/light").toUtf8().constData());
        }
        sub("actuators/lock");
        sub("actuators/dialer");

        for (const auto& room : ROOM_KEYS) {
            sub(("actuators/" + room + "/speaker").toUtf8().constData());
        }

        emit self->connected();
        self->m_data->addLog(AlertLevel::OK, "MQTT", "Connected to broker — all topics subscribed");
    } else {
        emit self->connectionError(QString("Broker refused: rc=%1").arg(rc));
    }
}

void MqttClient::onDisconnectStatic(struct mosquitto*, void* obj, int) {
    MqttClient* self = static_cast<MqttClient*>(obj);
    self->m_connected = false;
    emit self->disconnected();
    self->m_data->addLog(AlertLevel::WARNING, "MQTT", "Disconnected from broker");
}

void MqttClient::onMessageStatic(struct mosquitto*, void* obj,
                                  const struct mosquitto_message* msg) {
    MqttClient* self = static_cast<MqttClient*>(obj);
    QString topic   = QString::fromUtf8(msg->topic);
    QString payload = QString::fromUtf8((char*)msg->payload, msg->payloadlen);
    // Qt signals are not thread-safe from mosquitto thread; use invokeMethod
    QMetaObject::invokeMethod(self, [self, topic, payload]() {
        self->onMessage(topic, payload);
    }, Qt::QueuedConnection);
}

// ── Message dispatcher ────────────────────────────────────────────────────────
void MqttClient::onMessage(const QString& topic, const QString& payload) {
    QString sensorData = payload;
    try {
        auto j = json::parse(payload.toStdString());
        if (j.contains("payload") && j["payload"].contains("data"))
            sensorData = QString::fromStdString(j["payload"]["data"].dump());
    } catch (...) {}

    if (topic == "sensors/wristband/health") {
        parseHealthPayload(sensorData);
    } else if (topic == "sensors/wristband") {
        // Some scenarios send health data to sensors/wristband (without /health)
        parseHealthPayload(sensorData);
    } else if (topic == "sensors/wristband/motion") {
        parseMotionPayload(sensorData);
    } else if (topic == "actuators/lock") {
        parseLockPayload(payload);      // raw — parser extracts actionType itself
    } else if (topic == "actuators/dialer") {
        parseDialerPayload(payload);    // raw
    } else {
        int roomIdx = roomIndexFromTopic(topic);
        if (roomIdx < 0) return;

        if (topic.endsWith("/fire"))           parseFirePayload(roomIdx, sensorData);
        else if (topic.endsWith("/pir"))       parsePIRPayload(roomIdx, sensorData);
        else if (topic.endsWith("/sprinkler")) parseSprinklerPayload(roomIdx, payload); // raw
        else if (topic.endsWith("/light"))     parseLightPayload(roomIdx, payload);     // raw
        else if (topic.endsWith("/speaker"))   parseSpeakerPayload(roomIdx, payload);   // raw
    }
}

int MqttClient::roomIndexFromTopic(const QString& topic) const {
    for (int i = 0; i < ROOM_KEYS.size(); ++i) {
        if (topic.contains("/" + ROOM_KEYS[i] + "/")) return i;
    }
    return -1;
}

// ── Parsers ───────────────────────────────────────────────────────────────────
void MqttClient::parseHealthPayload(const QString& raw) {
    try {
        auto j = json::parse(raw.toStdString());
        HealthState h = m_data->health;
        if (j.contains("heartRate"))  h.heartRate  = j["heartRate"].get<int>();
        if (j.contains("spo2"))       h.spo2       = j["spo2"].get<int>();
        if (j.contains("systolic"))   h.systolic   = j["systolic"].get<int>();
        if (j.contains("diastolic"))  h.diastolic  = j["diastolic"].get<int>();
        h.lastUpdate = QDateTime::currentDateTime();
        m_data->setHealth(h);

        AlertLevel lvl = m_data->healthStatus();
        QString msg = QString("HR:%1bpm SPO2:%2% BP:%3/%4")
                          .arg(h.heartRate).arg(h.spo2).arg(h.systolic).arg(h.diastolic);
        m_data->addLog(lvl, "WRISTBAND_HEALTH", msg);
    } catch (const std::exception& e) {
        qWarning() << "parseHealthPayload error:" << e.what();
    }
}

void MqttClient::parseMotionPayload(const QString& raw) {
    try {
        auto j = json::parse(raw.toStdString());

        float accelX = j.value("accelX", 0.0f);
        float accelY = j.value("accelY", 0.0f);
        float accelZ = j.value("accelZ", 0.0f);

        float magnitude = 0.0f;
        if (j.contains("magnitude")) {
            magnitude = j["magnitude"].get<float>();
        } else if (accelX != 0.0f || accelY != 0.0f || accelZ != 0.0f) {
            magnitude = std::sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
        }

        HealthState h = m_data->health;
        h.fallMagnitude = magnitude;
        h.fallDetected  = (magnitude > FALL_MAGNITUDE_THRESHOLD);
        m_data->setHealth(h);

        if (h.fallDetected) {
            m_data->addLog(AlertLevel::ALARM, "WRISTBAND_MOTION",
                           QString("⚠ FALL DETECTED! Magnitude: %1").arg(magnitude, 0, 'f', 2));
            emit m_data->alarmTriggered("FALL DETECTED!");
        } else {
            m_data->addLog(AlertLevel::OK, "WRISTBAND_MOTION",
                           QString("Normal motion — magnitude: %1").arg(magnitude, 0, 'f', 2));
        }
    } catch (const std::exception& e) {
        qWarning() << "parseMotionPayload error:" << e.what();
    }
}

void MqttClient::parseFirePayload(int idx, const QString& raw) {
    try {
        auto j = json::parse(raw.toStdString());
        RoomFireData f;
        if (j.contains("temperature")) f.temperature = j["temperature"].get<float>();
        if (j.contains("smokeLevel"))  f.smokeLevel  = j["smokeLevel"].get<float>();
        if (j.contains("coLevel"))     f.coLevel     = j["coLevel"].get<int>();
        m_data->setRoomFire(idx, f);

        AlertLevel lvl = m_data->roomFireStatus(idx);
        QString msg = QString("%1 — Temp:%2°C Smoke:%3ppm CO:%4ppm")
                          .arg(m_data->rooms[idx].name)
                          .arg(f.temperature, 0, 'f', 1)
                          .arg(f.smokeLevel,  0, 'f', 0)
                          .arg(f.coLevel);
        m_data->addLog(lvl, "FIRE_" + m_data->rooms[idx].name.toUpper().replace(" ","_"), msg);
    } catch (const std::exception& e) {
        qWarning() << "parseFirePayload error:" << e.what();
    }
}

void MqttClient::parsePIRPayload(int idx, const QString& raw) {
    try {
        auto j = json::parse(raw.toStdString());
        RoomPIRData p;
        if (j.contains("motionDetected")) p.motionDetected = j["motionDetected"].get<bool>();
        if (p.motionDetected) p.lastDetected = QDateTime::currentDateTime();
        m_data->setRoomPIR(idx, p);

        QString status = p.motionDetected ? "Motion detected" : "No motion";
        m_data->addLog(AlertLevel::OK, "PIR_" + m_data->rooms[idx].name.toUpper().replace(" ","_"),
                       m_data->rooms[idx].name + ": " + status);
    } catch (const std::exception& e) {
        qWarning() << "parsePIRPayload error:" << e.what();
    }
}

void MqttClient::parseSprinklerPayload(int idx, const QString& raw) {
    try {
        auto outer = json::parse(raw.toStdString());

        // Hub sends full Message via JSONAdapter::encode:
        // { "deviceId":..., "payload": { "type":"COMMAND", "data": {"actionType":"START","value":""} } }
        json data = outer;
        if (outer.contains("payload") && outer["payload"].contains("data"))
            data = outer["payload"]["data"];

        bool on = false;
        if (data.contains("actionType")) {
            std::string action = data["actionType"].get<std::string>();
            on = (action == "START" || action == "TURN_ON");
        } else if (data.contains("active")) {
            // fallback: direct state field
            on = data["active"].get<bool>();
        }

        m_data->setRoomSprinkler(idx, on);
        m_data->addLog(on ? AlertLevel::WARNING : AlertLevel::OK,
                       "SPRINKLER_" + m_data->rooms[idx].name.toUpper().replace(" ","_"),
                       m_data->rooms[idx].name + " sprinkler: " + (on ? "ACTIVATED" : "off"));
    } catch (...) {}
}

void MqttClient::parseLightPayload(int idx, const QString& raw) {
    try {
        auto outer = json::parse(raw.toStdString());

        json data = outer;
        if (outer.contains("payload") && outer["payload"].contains("data"))
            data = outer["payload"]["data"];

        bool on = false;
        int  br = 0;
        if (data.contains("actionType")) {
            std::string action = data["actionType"].get<std::string>();
            if (action == "TURN_ON") {
                on = true; br = 100;
            } else if (action == "TURN_OFF") {
                on = false; br = 0;
            } else if (action == "SET_LEVEL" && data.contains("value")) {
                br = std::stoi(data["value"].get<std::string>());
                on = (br > 0);
            }
        } else {
            // fallback: direct state fields
            if (data.contains("on"))         on = data["on"].get<bool>();
            if (data.contains("brightness")) br = data["brightness"].get<int>();
        }

        m_data->setRoomLight(idx, on, br);
        m_data->addLog(AlertLevel::OK,
                       "LIGHT_" + m_data->rooms[idx].name.toUpper().replace(" ","_"),
                       m_data->rooms[idx].name + " light: " + (on ? QString("ON (%1%)").arg(br) : "off"));
    } catch (...) {}
}

void MqttClient::parseLockPayload(const QString& raw) {
    try {
        auto outer = json::parse(raw.toStdString());

        json data = outer;
        if (outer.contains("payload") && outer["payload"].contains("data"))
            data = outer["payload"]["data"];

        GlobalActuatorState g = m_data->globals;
        if (data.contains("actionType")) {
            std::string action = data["actionType"].get<std::string>();
            g.locked = (action == "LOCK");
        } else if (data.contains("locked")) {
            g.locked = data["locked"].get<bool>();
        }

        m_data->setGlobals(g);
        m_data->addLog(AlertLevel::OK, "LOCK",
                       g.locked ? "Door locked" : "Door UNLOCKED");
    } catch (...) {}
}

void MqttClient::parseDialerPayload(const QString& raw) {
    try {
        auto outer = json::parse(raw.toStdString());

        json data = outer;
        if (outer.contains("payload") && outer["payload"].contains("data"))
            data = outer["payload"]["data"];

        GlobalActuatorState g = m_data->globals;
        if (data.contains("actionType")) {
            g.dialerAction = QString::fromStdString(data["actionType"].get<std::string>());
            g.dialerBusy   = true;
        }
        m_data->setGlobals(g);
        m_data->addLog(AlertLevel::ALARM, "DIALER",
                       "Emergency call triggered: " + g.dialerAction);
        emit m_data->alarmTriggered("Emergency call: " + g.dialerAction);
    } catch (...) {}
}

void MqttClient::parseSpeakerPayload(int idx, const QString& raw) {
    try {
        auto outer = json::parse(raw.toStdString());

        json data = outer;
        if (outer.contains("payload") && outer["payload"].contains("data"))
            data = outer["payload"]["data"];

        if (data.contains("actionType")) {
            std::string action = data["actionType"].get<std::string>();
            int volume = 0;
            if (action == "TURN_ON")  volume = 50;
            else if (action == "TURN_OFF") volume = 0;
            else if (action == "SET_LEVEL" && data.contains("value"))
                volume = std::stoi(data["value"].get<std::string>());

            m_data->rooms[idx].actuators.speakerLevel = volume;
            m_data->addLog(volume > 0 ? AlertLevel::WARNING : AlertLevel::OK,
                           "SPEAKER_" + m_data->rooms[idx].name.toUpper().replace(" ","_"),
                           m_data->rooms[idx].name + " speaker: " +
                           (volume > 0 ? QString("ON (vol %1)").arg(volume) : "off"));
            emit m_data->dataChanged();
        }
    } catch (...) {}
}
