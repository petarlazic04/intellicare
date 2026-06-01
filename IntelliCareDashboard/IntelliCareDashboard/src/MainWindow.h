#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <array>

#include "DashboardData.h"
#include "MqttClient.h"
#include "widgets/RoomCard.h"
#include "widgets/HealthPanel.h"
#include "widgets/MotionPanel.h"
#include "widgets/DeviceControlPanel.h"
#include "widgets/ActivityLog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshTimer();
    void onStartMonitoring();
    void onStopMonitoring();
    void onMqttConnected();
    void onMqttDisconnected();
    void onAlarmTriggered(const QString& msg);
    void onNewLogEntry(const LogEntry& e);

private:
    void buildUi();
    void buildHeader(QWidget* parent, QBoxLayout* mainLayout);
    void buildBody  (QWidget* parent, QBoxLayout* mainLayout);
    void updateHeader();
    void flashAlarm();

    // Data + MQTT
    DashboardData* m_data  = nullptr;
    MqttClient*    m_mqtt  = nullptr;
    QTimer*        m_refreshTimer = nullptr;
    QTimer*        m_alarmFlashTimer = nullptr;
    int            m_flashCount = 0;

    // Header widgets
    QLabel*      m_statusBadge   = nullptr;
    QLabel*      m_timestampLabel= nullptr;
    QLabel*      m_alertCountLabel = nullptr;
    QLabel*      m_modeLabel     = nullptr;
    QLabel*      m_mqttStatusLabel = nullptr;
    QPushButton* m_startBtn      = nullptr;
    QPushButton* m_stopBtn       = nullptr;

    // Center panels
    HealthPanel*        m_healthPanel = nullptr;
    MotionPanel*        m_motionPanel = nullptr;
    ActivityLog*        m_activityLog = nullptr;
    DeviceControlPanel* m_deviceCtl  = nullptr;

    // Room cards
    std::array<RoomCard*, 5> m_roomCards;
};
