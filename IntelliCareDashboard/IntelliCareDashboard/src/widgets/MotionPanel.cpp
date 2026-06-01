#include "MotionPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

MotionPanel::MotionPanel(DashboardData* data, QWidget* parent)
    : QFrame(parent), m_data(data)
{
    auto* vl = new QVBoxLayout(this);
    vl->setSpacing(8);
    vl->setContentsMargins(0,0,0,0);

    // Title
    auto* title = new QLabel("🚶  Motion & Activity");
    title->setStyleSheet("font-size:14px; font-weight:700; color:#1f2937;");
    vl->addWidget(title);

    // Last motion row
    auto* infoBox = new QFrame;
    infoBox->setStyleSheet("background:white; border-radius:8px; border:1px solid #e5e7eb;");
    auto* infoL = new QVBoxLayout(infoBox);
    infoL->setContentsMargins(12,8,12,8);

    m_lastMotionLabel = new QLabel("Last motion: —");
    m_lastMotionLabel->setStyleSheet("font-size:13px; color:#374151;");
    m_fallRiskLabel   = new QLabel("Fall risk: Normal");
    m_fallRiskLabel->setStyleSheet("font-size:12px; color:#10b981; font-weight:600;");
    infoL->addWidget(m_lastMotionLabel);
    infoL->addWidget(m_fallRiskLabel);
    vl->addWidget(infoBox);

    // Room motion grid
    auto* grid = new QGridLayout;
    grid->setSpacing(6);
    const QStringList names = {"Kitchen","Living Room","Bedroom","Bathroom","Hallway"};
    for (int i = 0; i < 5; ++i) {
        auto* card = new QFrame;
        card->setStyleSheet("background:white; border-radius:6px; border:1px solid #e5e7eb;");
        card->setFixedHeight(46);
        auto* hl = new QHBoxLayout(card);
        hl->setContentsMargins(8,4,8,4);

        m_roomDots[i] = new QLabel("●");
        m_roomDots[i]->setStyleSheet("color:#10b981; font-size:14px;");
        m_roomDots[i]->setFixedWidth(20);

        m_roomLabels[i] = new QLabel(names[i]);
        m_roomLabels[i]->setStyleSheet("font-size:12px; color:#374151;");

        hl->addWidget(m_roomDots[i]);
        hl->addWidget(m_roomLabels[i]);
        hl->addStretch();

        grid->addWidget(card, i / 3, i % 3);
    }
    vl->addLayout(grid);

    refresh();
}

void MotionPanel::refresh() {
    // Last motion
    QString room = m_data->lastMotionRoom();
    QDateTime t  = m_data->lastMotionTime();

    if (t.isValid()) {
        int secsAgo = t.secsTo(QDateTime::currentDateTime());
        QString ago = secsAgo < 60
            ? QString("%1 sec ago").arg(secsAgo)
            : QString("%1 min ago").arg(secsAgo/60);
        m_lastMotionLabel->setText(QString("Last motion: %1 — %2").arg(room).arg(ago));
    } else {
        m_lastMotionLabel->setText("Last motion: No data");
    }

    // Fall risk
    bool fallDetected = m_data->health.fallDetected;
    if (fallDetected) {
        m_fallRiskLabel->setText("⚠  FALL DETECTED — HIGH RISK");
        m_fallRiskLabel->setStyleSheet("font-size:12px; color:#ef4444; font-weight:700;");
    } else {
        m_fallRiskLabel->setText("Fall risk: Normal");
        m_fallRiskLabel->setStyleSheet("font-size:12px; color:#10b981; font-weight:600;");
    }

    // Per-room motion dots
    for (int i = 0; i < 5; ++i) {
        const auto& pir = m_data->rooms[i].pir;
        if (pir.valid && pir.motionDetected) {
            m_roomDots[i]->setStyleSheet("color:#1d4ed8; font-size:14px;");
            m_roomLabels[i]->setStyleSheet("font-size:12px; color:#1d4ed8; font-weight:600;");
        } else {
            m_roomDots[i]->setStyleSheet("color:#d1d5db; font-size:14px;");
            m_roomLabels[i]->setStyleSheet("font-size:12px; color:#9ca3af;");
        }
    }
}
