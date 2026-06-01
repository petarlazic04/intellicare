#include "RoomCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEnterEvent>

static QString statusColor(AlertLevel lvl) {
    switch (lvl) {
        case AlertLevel::OK:      return "#10b981";
        case AlertLevel::WARNING: return "#f59e0b";
        case AlertLevel::ALARM:   return "#ef4444";
    }
    return "#10b981";
}

RoomCard::RoomCard(int roomIdx, DashboardData* data, QWidget* parent)
    : QFrame(parent), m_idx(roomIdx), m_data(data)
{
    setFrameShape(QFrame::StyledPanel);
    setObjectName("roomCard");
    setCursor(Qt::PointingHandCursor);
    setMinimumWidth(160);

    auto* vl = new QVBoxLayout(this);
    vl->setSpacing(4);
    vl->setContentsMargins(10, 10, 10, 10);

    // Header row: status dot + room name
    auto* header = new QHBoxLayout;
    m_statusDot = new QLabel("●");
    m_statusDot->setFixedWidth(18);
    m_nameLabel = new QLabel;
    m_nameLabel->setStyleSheet("font-weight:700; font-size:13px; color:#1f2937;");

    header->addWidget(m_statusDot);
    header->addWidget(m_nameLabel);
    header->addStretch();
    vl->addLayout(header);

    // Separator
    auto* sep = new QFrame; sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#e5e7eb;");
    vl->addWidget(sep);

    // Metrics
    auto mkRow = [&](const QString& icon, QLabel*& lbl) {
        auto* row = new QHBoxLayout;
        auto* ic = new QLabel(icon);
        ic->setStyleSheet("font-size:11px; color:#6b7280;");
        ic->setFixedWidth(20);
        lbl = new QLabel("—");
        lbl->setStyleSheet("font-size:12px; color:#374151;");
        row->addWidget(ic); row->addWidget(lbl); row->addStretch();
        vl->addLayout(row);
    };

    mkRow("🌡", m_tempLabel);
    mkRow("💨", m_smokeLabel);
    mkRow("⚠", m_coLabel);
    mkRow("👁", m_pirLabel);
    mkRow("🚿", m_sprinklerLabel);

    setStyleSheet(R"(
        QFrame#roomCard {
            background: white;
            border-radius: 8px;
            border: 1px solid #e5e7eb;
        }
        QFrame#roomCard:hover {
            border: 1px solid #1d4ed8;
        }
    )");

    refresh();
}

void RoomCard::refresh() {
    const auto& room = m_data->rooms[m_idx];
    AlertLevel lvl   = m_data->roomFireStatus(m_idx);

    m_nameLabel->setText(room.name);
    m_statusDot->setStyleSheet(
        QString("color:%1; font-size:16px;").arg(statusColor(lvl)));

    if (room.fire.valid) {
        m_tempLabel->setText(QString("%1 °C").arg(room.fire.temperature, 0, 'f', 1));
        m_smokeLabel->setText(QString("%1 ppm").arg(room.fire.smokeLevel, 0, 'f', 0));
        m_coLabel->setText(QString("%1 ppm CO").arg(room.fire.coLevel));

        // Color warn/alarm values
        auto colorVal = [](QLabel* l, bool danger, bool warn) {
            if (danger)      l->setStyleSheet("font-size:12px; color:#ef4444; font-weight:700;");
            else if (warn)   l->setStyleSheet("font-size:12px; color:#f59e0b; font-weight:600;");
            else             l->setStyleSheet("font-size:12px; color:#374151;");
        };
        colorVal(m_tempLabel,  room.fire.temperature > FIRE_TEMP_THRESHOLD,
                               room.fire.temperature > FIRE_TEMP_THRESHOLD * 0.7f);
        colorVal(m_smokeLabel, room.fire.smokeLevel  > FIRE_SMOKE_THRESHOLD,
                               room.fire.smokeLevel  > FIRE_SMOKE_THRESHOLD * 0.5f);
        colorVal(m_coLabel,    room.fire.coLevel     > FIRE_CO_THRESHOLD,
                               room.fire.coLevel     > FIRE_CO_THRESHOLD * 0.7f);
    } else {
        m_tempLabel->setText("— °C");
        m_smokeLabel->setText("— ppm");
        m_coLabel->setText("— ppm CO");
    }

    if (room.pir.valid) {
        bool moving = room.pir.motionDetected;
        m_pirLabel->setText(moving ? "Motion active" : "No motion");
        m_pirLabel->setStyleSheet(moving
            ? "font-size:12px; color:#1d4ed8; font-weight:600;"
            : "font-size:12px; color:#6b7280;");
    } else {
        m_pirLabel->setText("No data");
        m_pirLabel->setStyleSheet("font-size:12px; color:#9ca3af;");
    }

    bool spr = room.actuators.sprinklerOn;
    m_sprinklerLabel->setText(spr ? "Sprinkler ON" : "Sprinkler off");
    m_sprinklerLabel->setStyleSheet(spr
        ? "font-size:12px; color:#ef4444; font-weight:700;"
        : "font-size:12px; color:#6b7280;");

    // Card background flashes red on ALARM
    if (lvl == AlertLevel::ALARM) {
        setStyleSheet(R"(
            QFrame#roomCard {
                background: #fef2f2;
                border-radius: 8px;
                border: 2px solid #ef4444;
            }
        )");
    } else if (lvl == AlertLevel::WARNING) {
        setStyleSheet(R"(
            QFrame#roomCard {
                background: #fffbeb;
                border-radius: 8px;
                border: 2px solid #f59e0b;
            }
        )");
    } else {
        setStyleSheet(R"(
            QFrame#roomCard {
                background: white;
                border-radius: 8px;
                border: 1px solid #e5e7eb;
            }
            QFrame#roomCard:hover {
                border: 1px solid #1d4ed8;
            }
        )");
    }
}

void RoomCard::mousePressEvent(QMouseEvent*)  { emit clicked(m_idx); }
void RoomCard::enterEvent(QEvent*)            { setGraphicsEffect(nullptr); }
void RoomCard::leaveEvent(QEvent*)            { }
