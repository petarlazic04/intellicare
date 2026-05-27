#pragma once
#include <QFrame>
#include <QLabel>
#include "../DashboardData.h"

// A single metric tile: icon + value + unit + label
class MetricWidget : public QFrame {
    Q_OBJECT
public:
    explicit MetricWidget(const QString& icon, const QString& label,
                          const QString& unit, QWidget* parent = nullptr);

    void setValue(const QString& val, AlertLevel lvl = AlertLevel::OK);
    void setTrend(const QString& arrow);   // "↑" "↓" "→"

private:
    QLabel* m_valueLabel;
    QLabel* m_unitLabel;
    QLabel* m_nameLabel;
    QLabel* m_trendLabel;
    QLabel* m_iconLabel;
};
