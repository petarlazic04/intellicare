#pragma once
#include <QFrame>
#include <QLabel>
#include "MetricWidget.h"
#include "../DashboardData.h"

class HealthPanel : public QFrame {
    Q_OBJECT
public:
    explicit HealthPanel(DashboardData* data, QWidget* parent = nullptr);
    void refresh();

private:
    DashboardData* m_data;

    MetricWidget* m_hrWidget;
    MetricWidget* m_spo2Widget;
    MetricWidget* m_bpWidget;
    QLabel*       m_fallLabel;
    QLabel*       m_statusLabel;

    int m_prevHR = 0;
};
