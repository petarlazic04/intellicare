#pragma once
#include <QFrame>
#include <QLabel>
#include <array>
#include "../DashboardData.h"

class MotionPanel : public QFrame {
    Q_OBJECT
public:
    explicit MotionPanel(DashboardData* data, QWidget* parent = nullptr);
    void refresh();

private:
    DashboardData* m_data;
    QLabel* m_lastMotionLabel;
    QLabel* m_fallRiskLabel;
    std::array<QLabel*, 5> m_roomDots;
    std::array<QLabel*, 5> m_roomLabels;
};
