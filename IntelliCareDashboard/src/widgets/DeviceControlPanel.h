#pragma once
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <array>
#include "../DashboardData.h"
#include "../MqttClient.h"

class DeviceControlPanel : public QFrame {
    Q_OBJECT
public:
    explicit DeviceControlPanel(DashboardData* data, MqttClient* mqtt,
                                 QWidget* parent = nullptr);
    void refresh();

private slots:
    void onEmergencyCall();

private:
    DashboardData* m_data;
    MqttClient*    m_mqtt;

    std::array<QPushButton*, 5> m_lightBtns;
    std::array<QLabel*, 5>      m_sprinklerLeds;
    QPushButton* m_lockBtn;
    QLabel*      m_speakerLabel;
    QPushButton* m_emergencyBtn;

    void publishCommand(const QString& topic, const QString& actionType, const QString& value = "");
};
