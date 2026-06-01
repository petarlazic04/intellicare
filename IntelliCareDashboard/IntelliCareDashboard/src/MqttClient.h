#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <mosquitto.h>
#include "DashboardData.h"

/**
 * MqttClient
 * ----------
 * Wraps libmosquitto and subscribes to all IntelliCare MQTT topics.
 * Parses incoming JSON payloads (same format as IntelliCare's JSONAdapter)
 * and updates DashboardData accordingly.
 *
 * Topics subscribed:
 *   sensors/wristband/health      → HealthData
 *   sensors/wristband/motion      → MotionData (fall detection)
 *   sensors/<room>/fire           → FireDetectorData
 *   sensors/<room>/pir            → PIRData
 *   actuators/<room>/sprinkler    → sprinkler state
 *   actuators/<room>/light        → light state
 *   actuators/lock                → lock state
 *   actuators/dialer              → dialer action
 */
class MqttClient : public QObject {
    Q_OBJECT

public:
    explicit MqttClient(DashboardData* data, QObject* parent = nullptr);
    ~MqttClient();

    bool connectToBroker(const QString& host = "localhost", int port = 1883);
    void disconnect();
    bool isConnected() const { return m_connected; }

    // Publish a command (for actuator control from dashboard)
    void publish(const QString& topic, const QString& payload);

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& msg);

private slots:
    void pollMosquitto();

private:
    static void onConnectStatic   (struct mosquitto*, void* obj, int rc);
    static void onDisconnectStatic(struct mosquitto*, void* obj, int rc);
    static void onMessageStatic   (struct mosquitto*, void* obj,
                                   const struct mosquitto_message* msg);

    void onMessage(const QString& topic, const QString& payload);

    // Topic parsers
    void parseHealthPayload (const QString& payload);
    void parseMotionPayload (const QString& payload);
    void parseFirePayload   (int roomIdx, const QString& payload);
    void parsePIRPayload    (int roomIdx, const QString& payload);
    void parseSprinklerPayload(int roomIdx, const QString& payload);
    void parseLightPayload  (int roomIdx, const QString& payload);
    void parseSpeakerPayload(int roomIdx, const QString& payload);
    void parseLockPayload   (const QString& payload);
    void parseDialerPayload (const QString& payload);

    int roomIndexFromTopic(const QString& topic) const;

    struct mosquitto* m_mosq   = nullptr;
    DashboardData*    m_data   = nullptr;
    QTimer*           m_timer  = nullptr;
    bool              m_connected = false;
};
