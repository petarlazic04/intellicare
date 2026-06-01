#include "MetricWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

MetricWidget::MetricWidget(const QString& icon, const QString& label,
                           const QString& unit, QWidget* parent)
    : QFrame(parent)
{
    setObjectName("metricWidget");
    setMinimumHeight(90);

    auto* hl = new QHBoxLayout(this);
    hl->setContentsMargins(12, 10, 12, 10);
    hl->setSpacing(10);

    m_iconLabel = new QLabel(icon);
    m_iconLabel->setStyleSheet("font-size:26px;");
    m_iconLabel->setFixedWidth(36);
    hl->addWidget(m_iconLabel);

    auto* vl = new QVBoxLayout;
    vl->setSpacing(2);

    auto* topRow = new QHBoxLayout;
    m_valueLabel = new QLabel("—");
    m_valueLabel->setStyleSheet("font-size:26px; font-weight:700; color:#1f2937;");
    m_unitLabel = new QLabel(unit);
    m_unitLabel->setStyleSheet("font-size:13px; color:#6b7280; margin-top:8px;");
    m_trendLabel = new QLabel("");
    m_trendLabel->setStyleSheet("font-size:16px; margin-top:6px;");
    topRow->addWidget(m_valueLabel);
    topRow->addWidget(m_unitLabel);
    topRow->addWidget(m_trendLabel);
    topRow->addStretch();
    vl->addLayout(topRow);

    m_nameLabel = new QLabel(label);
    m_nameLabel->setStyleSheet("font-size:11px; color:#9ca3af; text-transform:uppercase; letter-spacing:1px;");
    vl->addWidget(m_nameLabel);

    hl->addLayout(vl);

    setStyleSheet(R"(
        QFrame#metricWidget {
            background: white;
            border-radius: 8px;
            border: 1px solid #e5e7eb;
        }
    )");
}

void MetricWidget::setValue(const QString& val, AlertLevel lvl) {
    m_valueLabel->setText(val);

    QString color;
    QString border = "1px solid #e5e7eb";
    QString bg     = "white";

    switch (lvl) {
        case AlertLevel::OK:
            color  = "#1f2937";
            break;
        case AlertLevel::WARNING:
            color  = "#f59e0b";
            border = "2px solid #f59e0b";
            bg     = "#fffbeb";
            break;
        case AlertLevel::ALARM:
            color  = "#ef4444";
            border = "2px solid #ef4444";
            bg     = "#fef2f2";
            break;
    }

    m_valueLabel->setStyleSheet(
        QString("font-size:26px; font-weight:700; color:%1;").arg(color));

    setStyleSheet(QString(R"(
        QFrame#metricWidget {
            background: %1;
            border-radius: 8px;
            border: %2;
        }
    )").arg(bg).arg(border));
}

void MetricWidget::setTrend(const QString& arrow) {
    m_trendLabel->setText(arrow);
    if (arrow == "↑")      m_trendLabel->setStyleSheet("font-size:16px; color:#ef4444; margin-top:6px;");
    else if (arrow == "↓") m_trendLabel->setStyleSheet("font-size:16px; color:#1d4ed8; margin-top:6px;");
    else                   m_trendLabel->setStyleSheet("font-size:16px; color:#9ca3af; margin-top:6px;");
}
