#include "HealthPanel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>

HealthPanel::HealthPanel(DashboardData* data, QWidget* parent)
    : QFrame(parent), m_data(data)
{
    setObjectName("healthPanel");

    auto* vl = new QVBoxLayout(this);
    vl->setSpacing(8);
    vl->setContentsMargins(0, 0, 0, 0);

    // Section title
    auto* titleRow = new QHBoxLayout;
    auto* title = new QLabel("❤  Health Metrics");
    title->setStyleSheet("font-size:14px; font-weight:700; color:#1f2937;");
    m_statusLabel = new QLabel("● OK");
    m_statusLabel->setStyleSheet("color:#10b981; font-weight:700; font-size:12px;");
    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(m_statusLabel);
    vl->addLayout(titleRow);

    // Metric widgets grid
    auto* grid = new QGridLayout;
    grid->setSpacing(8);

    m_hrWidget   = new MetricWidget("💓", "Heart Rate",     "BPM");
    m_spo2Widget = new MetricWidget("🫁", "SpO₂",           "%");
    m_bpWidget   = new MetricWidget("🩺", "Blood Pressure", "mmHg");

    grid->addWidget(m_hrWidget,   0, 0);
    grid->addWidget(m_spo2Widget, 0, 1);
    grid->addWidget(m_bpWidget,   0, 2);

    vl->addLayout(grid);

    // Fall detection banner
    m_fallLabel = new QLabel("✓  No fall detected");
    m_fallLabel->setObjectName("fallLabel");
    m_fallLabel->setAlignment(Qt::AlignCenter);
    m_fallLabel->setStyleSheet(R"(
        QLabel#fallLabel {
            background: #d1fae5;
            color: #065f46;
            border-radius: 6px;
            padding: 6px 12px;
            font-weight: 600;
            font-size: 13px;
        }
    )");
    vl->addWidget(m_fallLabel);

    refresh();
}

void HealthPanel::refresh() {
    const auto& h  = m_data->health;
    AlertLevel lvl = m_data->healthStatus();

    // HR with trend
    QString hrTrend = "→";
    if (h.heartRate > m_prevHR + 2) hrTrend = "↑";
    else if (h.heartRate < m_prevHR - 2) hrTrend = "↓";
    m_prevHR = h.heartRate;

    auto hrLvl = AlertLevel::OK;
    if (h.heartRate < CRITICAL_HEART_RATE_LOW || h.heartRate > CRITICAL_HEART_RATE_HIGH)
        hrLvl = AlertLevel::ALARM;
    else if (h.heartRate < CRITICAL_HEART_RATE_LOW + 10 || h.heartRate > CRITICAL_HEART_RATE_HIGH - 10)
        hrLvl = AlertLevel::WARNING;

    m_hrWidget->setValue(h.valid ? QString::number(h.heartRate) : "—", hrLvl);
    m_hrWidget->setTrend(hrTrend);

    auto spo2Lvl = AlertLevel::OK;
    if (h.spo2 < CRITICAL_SPO2_LOW) spo2Lvl = AlertLevel::ALARM;
    else if (h.spo2 < CRITICAL_SPO2_LOW + 5) spo2Lvl = AlertLevel::WARNING;
    m_spo2Widget->setValue(h.valid ? QString::number(h.spo2) : "—", spo2Lvl);

    auto bpLvl = AlertLevel::OK;
    if (h.systolic < CRITICAL_SYSTOLIC_LOW || h.systolic > CRITICAL_SYSTOLIC_HIGH ||
        h.diastolic < CRITICAL_DIASTOLIC_LOW || h.diastolic > CRITICAL_DIASTOLIC_HIGH)
        bpLvl = AlertLevel::ALARM;
    QString bpStr = h.valid
        ? QString("%1/%2").arg(h.systolic).arg(h.diastolic)
        : "—/—";
    m_bpWidget->setValue(bpStr, bpLvl);

    // Fall banner
    if (h.fallDetected) {
        m_fallLabel->setText("⚠  FALL DETECTED!");
        m_fallLabel->setObjectName("fallLabelAlarm");
        m_fallLabel->setStyleSheet(R"(
            background: #ef4444;
            color: white;
            border-radius: 6px;
            padding: 8px 12px;
            font-weight: 700;
            font-size:14px;
        )");
    } else {
        m_fallLabel->setText("✓  No fall detected");
        m_fallLabel->setObjectName("fallLabel");
        m_fallLabel->setStyleSheet(R"(
            background: #d1fae5;
            color: #065f46;
            border-radius: 6px;
            padding: 6px 12px;
            font-weight: 600;
            font-size: 13px;
        )");
    }

    // Overall status badge
    switch (lvl) {
        case AlertLevel::OK:
            m_statusLabel->setText("● OK");
            m_statusLabel->setStyleSheet("color:#10b981; font-weight:700; font-size:12px;");
            break;
        case AlertLevel::WARNING:
            m_statusLabel->setText("● WARNING");
            m_statusLabel->setStyleSheet("color:#f59e0b; font-weight:700; font-size:12px;");
            break;
        case AlertLevel::ALARM:
            m_statusLabel->setText("● ALARM");
            m_statusLabel->setStyleSheet("color:#ef4444; font-weight:700; font-size:12px;");
            break;
    }
}
