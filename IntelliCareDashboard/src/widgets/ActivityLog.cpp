#include "ActivityLog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>

ActivityLog::ActivityLog(DashboardData* data, QWidget* parent)
    : QFrame(parent), m_data(data)
{
    auto* vl = new QVBoxLayout(this);
    vl->setSpacing(6);
    vl->setContentsMargins(0,0,0,0);

    // Header
    auto* header = new QHBoxLayout;
    auto* title = new QLabel("📋  Activity Log");
    title->setStyleSheet("font-size:14px; font-weight:700; color:#1f2937;");

    auto* clearBtn = new QPushButton("Clear");
    clearBtn->setFixedSize(52, 24);
    clearBtn->setStyleSheet(R"(
        QPushButton { background:#f3f4f6; color:#6b7280; border-radius:4px;
                      font-size:11px; border:1px solid #d1d5db; }
        QPushButton:hover { background:#e5e7eb; }
    )");

    header->addWidget(title);
    header->addStretch();
    header->addWidget(clearBtn);
    vl->addLayout(header);

    m_list = new QListWidget;
    m_list->setStyleSheet(R"(
        QListWidget {
            background: white;
            border-radius: 8px;
            border: 1px solid #e5e7eb;
            font-family: 'Courier New', monospace;
            font-size: 12px;
        }
        QListWidget::item {
            padding: 4px 10px;
            border-bottom: 1px solid #f3f4f6;
        }
        QListWidget::item:selected {
            background: #eff6ff;
            color: #1d4ed8;
        }
    )");
    m_list->setMaximumHeight(180);
    vl->addWidget(m_list);

    connect(clearBtn, &QPushButton::clicked, this, &ActivityLog::clearLog);

    // Load existing entries
    for (const auto& e : m_data->logEntries())
        appendEntry(e);
}

void ActivityLog::appendEntry(const LogEntry& e) {
    QString timeStr = e.timestamp.toString("hh:mm:ss");
    QString text = QString("[%1] %2: %3").arg(timeStr).arg(e.source).arg(e.message);

    auto* item = new QListWidgetItem(text);

    switch (e.level) {
        case AlertLevel::OK:
            item->setForeground(QColor("#065f46"));
            item->setBackground(QColor("#f0fdf4"));
            break;
        case AlertLevel::WARNING:
            item->setForeground(QColor("#92400e"));
            item->setBackground(QColor("#fffbeb"));
            break;
        case AlertLevel::ALARM:
            item->setForeground(QColor("#991b1b"));
            item->setBackground(QColor("#fef2f2"));
            item->setFont([&]{ QFont f; f.setBold(true); return f; }());
            break;
    }

    // Insert at top (newest first)
    m_list->insertItem(0, item);

    // Keep max 100 visible entries
    while (m_list->count() > 100)
        delete m_list->takeItem(m_list->count() - 1);
}

void ActivityLog::clearLog() {
    m_list->clear();
}
