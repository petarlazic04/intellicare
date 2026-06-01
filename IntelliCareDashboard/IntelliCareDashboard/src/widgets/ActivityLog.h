#pragma once
#include <QFrame>
#include <QListWidget>
#include "../DashboardData.h"

class ActivityLog : public QFrame {
    Q_OBJECT
public:
    explicit ActivityLog(DashboardData* data, QWidget* parent = nullptr);

public slots:
    void appendEntry(const LogEntry& e);
    void clearLog();

private:
    DashboardData* m_data;
    QListWidget*   m_list;
};
