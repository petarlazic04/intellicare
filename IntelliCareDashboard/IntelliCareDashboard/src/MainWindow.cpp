#include "MainWindow.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QSplitter>
#include <QFrame>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("IntelliCare Dashboard");
    setMinimumSize(1280, 800);
    resize(1440, 900);

    // ── Data model + MQTT ─────────────────────────────────────────────────
    m_data = new DashboardData(this);
    m_mqtt = new MqttClient(m_data, this);

    connect(m_mqtt, &MqttClient::connected,       this, &MainWindow::onMqttConnected);
    connect(m_mqtt, &MqttClient::disconnected,    this, &MainWindow::onMqttDisconnected);
    connect(m_data, &DashboardData::newLogEntry,  this, &MainWindow::onNewLogEntry);
    connect(m_data, &DashboardData::alarmTriggered, this, &MainWindow::onAlarmTriggered);

    // Refresh UI immediately on any data change (don't wait for 3s timer)
    connect(m_data, &DashboardData::dataChanged, this, [this]() {
        if (m_healthPanel) m_healthPanel->refresh();
        if (m_motionPanel) m_motionPanel->refresh();
        if (m_deviceCtl)   m_deviceCtl->refresh();
        for (auto* c : m_roomCards) if (c) c->refresh();
        updateHeader();
    });

    // ── UI ────────────────────────────────────────────────────────────────
    buildUi();

    // ── Refresh timer (3 s) ───────────────────────────────────────────────
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshTimer);
    m_refreshTimer->start(1000);  // timestamp update only — panels refresh via dataChanged

    // ── Alarm flash timer ─────────────────────────────────────────────────
    m_alarmFlashTimer = new QTimer(this);
    m_alarmFlashTimer->setInterval(500);
    connect(m_alarmFlashTimer, &QTimer::timeout, this, &MainWindow::flashAlarm);

    // ── Auto-connect to broker ────────────────────────────────────────────
    m_mqtt->connectToBroker("localhost", 1883);

    statusBar()->showMessage("IntelliCare Dashboard v1.0 — Connecting to MQTT broker...");
}

MainWindow::~MainWindow() {}

// ─── Build UI ──────────────────────────────────────────────────────────────
void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    central->setStyleSheet("background: #f4f6f8; font-family: 'Segoe UI', Arial, sans-serif;");
    setCentralWidget(central);

    auto* mainVL = new QVBoxLayout(central);
    mainVL->setSpacing(0);
    mainVL->setContentsMargins(0, 0, 0, 0);

    buildHeader(central, mainVL);
    buildBody  (central, mainVL);
}

void MainWindow::buildHeader(QWidget* parent, QBoxLayout* mainVL) {
    auto* header = new QWidget;
    header->setFixedHeight(64);
    header->setStyleSheet("background: #1e3a5f; border-bottom: 3px solid #1d4ed8;");

    auto* hl = new QHBoxLayout(header);
    hl->setContentsMargins(20, 0, 20, 0);
    hl->setSpacing(16);

    // Logo + title
    auto* logo = new QLabel("🏠");
    logo->setStyleSheet("font-size:24px;");
    auto* appTitle = new QLabel("IntelliCare Dashboard");
    appTitle->setStyleSheet("font-size:20px; font-weight:700; color:white; letter-spacing:1px;");

    // MQTT status
    m_mqttStatusLabel = new QLabel("● Connecting...");
    m_mqttStatusLabel->setStyleSheet("color:#f59e0b; font-size:12px;");

    // Mode
    m_modeLabel = new QLabel("⏸  Idle");
    m_modeLabel->setStyleSheet("color:#94a3b8; font-size:12px; font-weight:600;");

    // Alert count
    m_alertCountLabel = new QLabel("0 alerts");
    m_alertCountLabel->setStyleSheet("background:#374151; color:#9ca3af; border-radius:10px;"
                                      "padding:2px 10px; font-size:11px;");

    // Status badge
    m_statusBadge = new QLabel("● OK");
    m_statusBadge->setStyleSheet("background:#065f46; color:#6ee7b7; border-radius:12px;"
                                  "padding:4px 14px; font-size:12px; font-weight:700;");

    // Timestamp
    m_timestampLabel = new QLabel;
    m_timestampLabel->setStyleSheet("color:#94a3b8; font-size:11px;");

    // Control buttons
    m_startBtn = new QPushButton("▶  START");
    m_startBtn->setFixedSize(90, 34);
    m_startBtn->setStyleSheet(R"(
        QPushButton { background:#10b981; color:white; border-radius:6px;
                      font-size:12px; font-weight:700; border:none; }
        QPushButton:hover { background:#059669; }
        QPushButton:disabled { background:#374151; color:#6b7280; }
    )");

    m_stopBtn = new QPushButton("■  STOP");
    m_stopBtn->setFixedSize(90, 34);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setStyleSheet(R"(
        QPushButton { background:#ef4444; color:white; border-radius:6px;
                      font-size:12px; font-weight:700; border:none; }
        QPushButton:hover { background:#dc2626; }
        QPushButton:disabled { background:#374151; color:#6b7280; }
    )");

    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartMonitoring);
    connect(m_stopBtn,  &QPushButton::clicked, this, &MainWindow::onStopMonitoring);

    // User label
    auto* userLabel = new QLabel("👤 Caregiver");
    userLabel->setStyleSheet("color:#94a3b8; font-size:12px;");

    hl->addWidget(logo);
    hl->addWidget(appTitle);
    hl->addSpacing(20);
    hl->addWidget(m_mqttStatusLabel);
    hl->addStretch();
    hl->addWidget(m_modeLabel);
    hl->addWidget(m_alertCountLabel);
    hl->addWidget(m_statusBadge);
    hl->addWidget(m_timestampLabel);
    hl->addSpacing(16);
    hl->addWidget(m_startBtn);
    hl->addWidget(m_stopBtn);
    hl->addSpacing(8);
    hl->addWidget(userLabel);

    mainVL->addWidget(header);
}

void MainWindow::buildBody(QWidget* parent, QBoxLayout* mainVL) {
    auto* body = new QWidget;
    auto* bodyHL = new QHBoxLayout(body);
    bodyHL->setContentsMargins(12, 12, 12, 12);
    bodyHL->setSpacing(12);

    // ── LEFT SIDEBAR: Room overview ──────────────────────────────────────
    auto* sidebar = new QWidget;
    sidebar->setFixedWidth(200);
    auto* sideVL = new QVBoxLayout(sidebar);
    sideVL->setSpacing(8);
    sideVL->setContentsMargins(0,0,0,0);

    auto* roomsTitle = new QLabel("🏠 Rooms");
    roomsTitle->setStyleSheet("font-size:13px; font-weight:700; color:#374151;"
                               "padding:4px 0; border-bottom:2px solid #1d4ed8; margin-bottom:4px;");
    sideVL->addWidget(roomsTitle);

    for (int i = 0; i < 5; ++i) {
        m_roomCards[i] = new RoomCard(i, m_data, sidebar);
        connect(m_roomCards[i], &RoomCard::clicked, this, [this](int idx){
            QMessageBox::information(this,
                m_data->rooms[idx].name + " Details",
                QString("Room: %1\n"
                        "Temperature: %2 °C\n"
                        "Smoke: %3 ppm\n"
                        "CO: %4 ppm\n"
                        "Motion: %5\n"
                        "Sprinkler: %6")
                    .arg(m_data->rooms[idx].name)
                    .arg(m_data->rooms[idx].fire.temperature, 0, 'f', 1)
                    .arg(m_data->rooms[idx].fire.smokeLevel,  0, 'f', 0)
                    .arg(m_data->rooms[idx].fire.coLevel)
                    .arg(m_data->rooms[idx].pir.motionDetected ? "Active" : "None")
                    .arg(m_data->rooms[idx].actuators.sprinklerOn ? "ON ⚠" : "Off")
            );
        });
        sideVL->addWidget(m_roomCards[i]);
    }
    sideVL->addStretch();
    bodyHL->addWidget(sidebar);

    // ── CENTER: Main metrics ─────────────────────────────────────────────
    auto* center = new QWidget;
    auto* centerVL = new QVBoxLayout(center);
    centerVL->setSpacing(12);
    centerVL->setContentsMargins(0,0,0,0);

    // Health + Motion row
    auto* topCenterHL = new QHBoxLayout;
    topCenterHL->setSpacing(12);

    // Health panel (in a styled card)
    auto* healthCard = new QFrame;
    healthCard->setStyleSheet("background:transparent;");
    auto* healthCardVL = new QVBoxLayout(healthCard);
    healthCardVL->setContentsMargins(0,0,0,0);
    m_healthPanel = new HealthPanel(m_data, healthCard);
    healthCardVL->addWidget(m_healthPanel);

    // Motion panel
    auto* motionCard = new QFrame;
    motionCard->setStyleSheet("background:transparent;");
    auto* motionCardVL = new QVBoxLayout(motionCard);
    motionCardVL->setContentsMargins(0,0,0,0);
    m_motionPanel = new MotionPanel(m_data, motionCard);
    motionCardVL->addWidget(m_motionPanel);

    topCenterHL->addWidget(healthCard, 3);
    topCenterHL->addWidget(motionCard, 2);
    centerVL->addLayout(topCenterHL);

    // Fire detection grid (5 rooms)
    auto* fireTitle = new QLabel("🔥 Fire Detection — All Rooms");
    fireTitle->setStyleSheet("font-size:13px; font-weight:700; color:#374151;"
                              "padding:4px 0; border-bottom:2px solid #1d4ed8; margin-top:4px;");
    centerVL->addWidget(fireTitle);

    auto* fireGrid = new QHBoxLayout;
    fireGrid->setSpacing(8);
    // Re-use room cards in expanded fire view — small summary cards
    for (int i = 0; i < 5; ++i) {
        auto* fc = new QFrame;
        fc->setObjectName("fireCard");
        fc->setMinimumHeight(110);
        auto* fcVL = new QVBoxLayout(fc);
        fcVL->setContentsMargins(10,8,10,8);
        fcVL->setSpacing(3);

        auto* fcName = new QLabel(m_data->rooms[i].name);
        fcName->setObjectName(QString("fcName%1").arg(i));
        fcName->setStyleSheet("font-size:12px; font-weight:700; color:#1f2937;");

        auto* fcTemp  = new QLabel("🌡 — °C");
        auto* fcSmoke = new QLabel("💨 — ppm");
        auto* fcCO    = new QLabel("⚠ — ppm CO");
        auto* fcStat  = new QLabel("● OK");

        fcTemp->setObjectName(QString("fcTemp%1").arg(i));
        fcSmoke->setObjectName(QString("fcSmoke%1").arg(i));
        fcCO->setObjectName(QString("fcCO%1").arg(i));
        fcStat->setObjectName(QString("fcStat%1").arg(i));

        for (auto* l : {fcTemp, fcSmoke, fcCO})
            l->setStyleSheet("font-size:11px; color:#6b7280;");
        fcStat->setStyleSheet("font-size:11px; color:#10b981; font-weight:700;");

        fcVL->addWidget(fcName);
        fcVL->addWidget(fcTemp);
        fcVL->addWidget(fcSmoke);
        fcVL->addWidget(fcCO);
        fcVL->addStretch();
        fcVL->addWidget(fcStat);

        fc->setStyleSheet("QFrame#fireCard { background:white; border-radius:8px; border:1px solid #e5e7eb; }");
        fireGrid->addWidget(fc);
    }
    centerVL->addLayout(fireGrid);

    // Activity log
    m_activityLog = new ActivityLog(m_data, center);
    connect(m_data, &DashboardData::newLogEntry, m_activityLog, &ActivityLog::appendEntry);
    centerVL->addWidget(m_activityLog);

    bodyHL->addWidget(center, 1);

    // ── RIGHT PANEL: Device control ───────────────────────────────────────
    auto* rightPanel = new QWidget;
    rightPanel->setFixedWidth(220);
    auto* rightVL = new QVBoxLayout(rightPanel);
    rightVL->setContentsMargins(0,0,0,0);

    m_deviceCtl = new DeviceControlPanel(m_data, m_mqtt, rightPanel);
    rightVL->addWidget(m_deviceCtl);

    bodyHL->addWidget(rightPanel);

    mainVL->addWidget(body, 1);
}

// ─── Refresh ──────────────────────────────────────────────────────────────
void MainWindow::onRefreshTimer() {
    updateHeader();

    for (auto* c : m_roomCards) c->refresh();
    m_healthPanel->refresh();
    m_motionPanel->refresh();
    m_deviceCtl->refresh();

    // Refresh fire grid cards
    for (int i = 0; i < 5; ++i) {
        AlertLevel lvl = m_data->roomFireStatus(i);
        const auto& f  = m_data->rooms[i].fire;

        auto find = [&](const QString& name) -> QLabel* {
            return findChild<QLabel*>(name);
        };

        if (auto* l = find(QString("fcTemp%1").arg(i)))
            l->setText(f.valid ? QString("🌡 %1 °C").arg(f.temperature, 0, 'f', 1) : "🌡 — °C");
        if (auto* l = find(QString("fcSmoke%1").arg(i)))
            l->setText(f.valid ? QString("💨 %1 ppm").arg(f.smokeLevel, 0, 'f', 0) : "💨 — ppm");
        if (auto* l = find(QString("fcCO%1").arg(i)))
            l->setText(f.valid ? QString("⚠ %1 ppm CO").arg(f.coLevel) : "⚠ — ppm CO");

        if (auto* l = find(QString("fcStat%1").arg(i))) {
            switch (lvl) {
                case AlertLevel::OK:
                    l->setText("● OK");
                    l->setStyleSheet("font-size:11px; color:#10b981; font-weight:700;");
                    break;
                case AlertLevel::WARNING:
                    l->setText("● WARNING");
                    l->setStyleSheet("font-size:11px; color:#f59e0b; font-weight:700;");
                    break;
                case AlertLevel::ALARM:
                    l->setText("⚠ ALARM");
                    l->setStyleSheet("font-size:11px; color:#ef4444; font-weight:700;");
                    break;
            }
        }

        auto* fc = findChild<QFrame*>(); // find fireCard by position not needed,
        // border color set on room cards above already
        Q_UNUSED(lvl);
    }
}

void MainWindow::updateHeader() {
    m_timestampLabel->setText(QDateTime::currentDateTime().toString("dd.MM.yyyy  hh:mm:ss"));

    AlertLevel overall = m_data->overallStatus();
    int alerts = m_data->activeAlertCount();

    switch (overall) {
        case AlertLevel::OK:
            m_statusBadge->setText("● SYSTEM OK");
            m_statusBadge->setStyleSheet("background:#065f46; color:#6ee7b7; border-radius:12px;"
                                          "padding:4px 14px; font-size:12px; font-weight:700;");
            break;
        case AlertLevel::WARNING:
            m_statusBadge->setText("⚠ WARNING");
            m_statusBadge->setStyleSheet("background:#78350f; color:#fcd34d; border-radius:12px;"
                                          "padding:4px 14px; font-size:12px; font-weight:700;");
            break;
        case AlertLevel::ALARM:
            m_statusBadge->setText("🚨 ALARM");
            m_statusBadge->setStyleSheet("background:#991b1b; color:#fca5a5; border-radius:12px;"
                                          "padding:4px 14px; font-size:12px; font-weight:700;");
            break;
    }

    if (alerts == 0) {
        m_alertCountLabel->setText("0 alerts");
        m_alertCountLabel->setStyleSheet("background:#374151; color:#9ca3af; border-radius:10px;"
                                          "padding:2px 10px; font-size:11px;");
    } else {
        m_alertCountLabel->setText(QString("%1 alert%2").arg(alerts).arg(alerts > 1 ? "s" : ""));
        m_alertCountLabel->setStyleSheet("background:#ef4444; color:white; border-radius:10px;"
                                          "padding:2px 10px; font-size:11px; font-weight:700;");
    }

    if (m_data->monitoringActive) {
        m_modeLabel->setText("● Monitoring Active");
        m_modeLabel->setStyleSheet("color:#34d399; font-size:12px; font-weight:600;");
    } else {
        m_modeLabel->setText("⏸  Idle");
        m_modeLabel->setStyleSheet("color:#94a3b8; font-size:12px; font-weight:600;");
    }
}

// ─── Slots ────────────────────────────────────────────────────────────────
void MainWindow::onStartMonitoring() {
    m_data->setMonitoring(true);
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_data->addLog(AlertLevel::OK, "DASHBOARD", "Monitoring started by user");
    statusBar()->showMessage("Monitoring active — receiving data from IntelliCare backend");
}

void MainWindow::onStopMonitoring() {
    m_data->setMonitoring(false);
    m_data->resetState();
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    statusBar()->showMessage("Monitoring stopped — state reset");
}

void MainWindow::onMqttConnected() {
    m_mqttStatusLabel->setText("● MQTT Connected");
    m_mqttStatusLabel->setStyleSheet("color:#34d399; font-size:12px;");
    statusBar()->showMessage("Connected to MQTT broker — listening for IntelliCare events");
}

void MainWindow::onMqttDisconnected() {
    m_mqttStatusLabel->setText("● MQTT Disconnected");
    m_mqttStatusLabel->setStyleSheet("color:#f87171; font-size:12px;");
    statusBar()->showMessage("MQTT connection lost — retrying...");
}

void MainWindow::onAlarmTriggered(const QString& msg) {
    m_flashCount = 6;
    m_alarmFlashTimer->start();
    statusBar()->showMessage("⚠ ALARM: " + msg);
}

void MainWindow::flashAlarm() {
    if (m_flashCount <= 0) {
        m_alarmFlashTimer->stop();
        centralWidget()->setStyleSheet("background:#f4f6f8; font-family:'Segoe UI',Arial,sans-serif;");
        return;
    }
    if (m_flashCount % 2 == 0) {
        centralWidget()->setStyleSheet("background:#fef2f2; font-family:'Segoe UI',Arial,sans-serif;");
    } else {
        centralWidget()->setStyleSheet("background:#f4f6f8; font-family:'Segoe UI',Arial,sans-serif;");
    }
    --m_flashCount;
}

void MainWindow::onNewLogEntry(const LogEntry&) {
    updateHeader();
}