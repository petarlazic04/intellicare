#include "DeviceControlPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

static const QStringList ROOM_KEYS_CTL  = {"kitchen","living_room","bedroom","bathroom","hallway"};
static const QStringList ROOM_NAMES_CTL = {"Kitchen","Living Room","Bedroom","Bathroom","Hallway"};

DeviceControlPanel::DeviceControlPanel(DashboardData* data, MqttClient* mqtt, QWidget* parent)
    : QFrame(parent), m_data(data), m_mqtt(mqtt)
{
    auto* vl = new QVBoxLayout(this);
    vl->setSpacing(10);
    vl->setContentsMargins(0,0,0,0);

    // Title
    auto* title = new QLabel("🎛  Device Control");
    title->setStyleSheet("font-size:14px; font-weight:700; color:#1f2937;");
    vl->addWidget(title);

    // ── Lights ─────────────────────────────────────────────────────────────
    auto* lightsBox = new QFrame;
    lightsBox->setStyleSheet("background:white; border-radius:8px; border:1px solid #e5e7eb;");
    auto* lightsL = new QVBoxLayout(lightsBox);
    lightsL->setContentsMargins(10,8,10,8);
    lightsL->setSpacing(4);

    auto* lightsTitle = new QLabel("💡 Lights");
    lightsTitle->setStyleSheet("font-size:12px; font-weight:700; color:#374151;");
    lightsL->addWidget(lightsTitle);

    for (int i = 0; i < 5; ++i) {
        auto* row = new QHBoxLayout;
        auto* lbl = new QLabel(ROOM_NAMES_CTL[i]);
        lbl->setStyleSheet("font-size:12px; color:#6b7280;");
        lbl->setMinimumWidth(90);

        m_lightBtns[i] = new QPushButton("OFF");
        m_lightBtns[i]->setFixedSize(52, 24);
        m_lightBtns[i]->setCheckable(true);
        m_lightBtns[i]->setStyleSheet(R"(
            QPushButton { background:#e5e7eb; color:#6b7280; border-radius:12px;
                          font-size:10px; font-weight:600; border:none; }
            QPushButton:checked { background:#1d4ed8; color:white; }
        )");

        int idx = i;
        connect(m_lightBtns[i], &QPushButton::toggled, this, [this, idx](bool on) {
            QString action = on ? "TURN_ON" : "TURN_OFF";
            publishCommand("actuators/" + ROOM_KEYS_CTL[idx] + "/light", action, on ? "100" : "0");
            m_lightBtns[idx]->setText(on ? "ON" : "OFF");
        });

        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(m_lightBtns[i]);
        lightsL->addLayout(row);
    }
    vl->addWidget(lightsBox);

    // ── Sprinklers (status read-only) ───────────────────────────────────────
    auto* sprBox = new QFrame;
    sprBox->setStyleSheet("background:white; border-radius:8px; border:1px solid #e5e7eb;");
    auto* sprL = new QGridLayout(sprBox);
    sprL->setContentsMargins(10,8,10,8);
    sprL->setSpacing(4);

    auto* sprTitle = new QLabel("🚿 Sprinklers");
    sprTitle->setStyleSheet("font-size:12px; font-weight:700; color:#374151;");
    sprL->addWidget(sprTitle, 0, 0, 1, 2);

    for (int i = 0; i < 5; ++i) {
        m_sprinklerLeds[i] = new QLabel("● " + ROOM_NAMES_CTL[i].left(8));
        m_sprinklerLeds[i]->setStyleSheet("font-size:11px; color:#d1d5db;");
        sprL->addWidget(m_sprinklerLeds[i], 1 + i/2, i%2);
    }
    vl->addWidget(sprBox);

    // ── Lock ────────────────────────────────────────────────────────────────
    auto* lockBox = new QFrame;
    lockBox->setStyleSheet("background:white; border-radius:8px; border:1px solid #e5e7eb;");
    auto* lockL = new QHBoxLayout(lockBox);
    lockL->setContentsMargins(10,8,10,8);

    auto* lockTitle = new QLabel("🔒 Door Lock");
    lockTitle->setStyleSheet("font-size:12px; font-weight:700; color:#374151;");

    m_lockBtn = new QPushButton("LOCKED");
    m_lockBtn->setCheckable(true);
    m_lockBtn->setFixedHeight(28);
    m_lockBtn->setStyleSheet(R"(
        QPushButton { background:#10b981; color:white; border-radius:6px;
                      font-size:11px; font-weight:700; border:none; padding:0 10px; }
        QPushButton:checked { background:#ef4444; }
    )");

    connect(m_lockBtn, &QPushButton::toggled, this, [this](bool unlocked) {
        QString action = unlocked ? "UNLOCK" : "LOCK";
        publishCommand("actuators/lock", action);
        m_lockBtn->setText(unlocked ? "UNLOCKED" : "LOCKED");
    });

    lockL->addWidget(lockTitle);
    lockL->addStretch();
    lockL->addWidget(m_lockBtn);
    vl->addWidget(lockBox);

    // ── Emergency button ─────────────────────────────────────────────────────
    m_emergencyBtn = new QPushButton("📞  EMERGENCY CALL");
    m_emergencyBtn->setMinimumHeight(48);
    m_emergencyBtn->setStyleSheet(R"(
        QPushButton {
            background: #ef4444;
            color: white;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 700;
            border: none;
        }
        QPushButton:hover {
            background: #dc2626;
        }
        QPushButton:pressed {
            background: #b91c1c;
        }
    )");
    connect(m_emergencyBtn, &QPushButton::clicked, this, &DeviceControlPanel::onEmergencyCall);
    vl->addWidget(m_emergencyBtn);

    vl->addStretch();
    refresh();
}

void DeviceControlPanel::refresh() {
    // Update light button states
    for (int i = 0; i < 5; ++i) {
        bool on = m_data->rooms[i].actuators.lightOn;
        m_lightBtns[i]->blockSignals(true);
        m_lightBtns[i]->setChecked(on);
        m_lightBtns[i]->setText(on ? "ON" : "OFF");
        m_lightBtns[i]->blockSignals(false);
    }

    // Sprinkler LEDs
    for (int i = 0; i < 5; ++i) {
        bool on = m_data->rooms[i].actuators.sprinklerOn;
        m_sprinklerLeds[i]->setText((on ? "● " : "○ ") + ROOM_NAMES_CTL[i].left(8));
        m_sprinklerLeds[i]->setStyleSheet(on
            ? "font-size:11px; color:#ef4444; font-weight:700;"
            : "font-size:11px; color:#d1d5db;");
    }

    // Lock
    bool locked = m_data->globals.locked;
    m_lockBtn->blockSignals(true);
    m_lockBtn->setChecked(!locked);
    m_lockBtn->setText(locked ? "LOCKED" : "UNLOCKED");
    m_lockBtn->blockSignals(false);
}

void DeviceControlPanel::onEmergencyCall() {
    auto btn = QMessageBox::question(this, "Emergency Call",
        "Call ambulance and notify family?",
        QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        publishCommand("actuators/dialer", "DIAL_AMBULANCE");
        m_data->addLog(AlertLevel::ALARM, "DIALER", "Manual emergency call triggered from dashboard");
    }
}

void DeviceControlPanel::publishCommand(const QString& topic, const QString& actionType,
                                         const QString& value) {
    if (!m_mqtt || !m_mqtt->isConnected()) return;
    QString payload = QString(R"({"actionType":"%1","value":"%2"})").arg(actionType).arg(value);
    m_mqtt->publish(topic, payload);
}
