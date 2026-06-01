#pragma once
#include <QFrame>
#include <QLabel>
#include "../DashboardData.h"

class RoomCard : public QFrame {
    Q_OBJECT
public:
    explicit RoomCard(int roomIdx, DashboardData* data, QWidget* parent = nullptr);
    void refresh();

protected:
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;

signals:
    void clicked(int roomIdx);

private:
    int            m_idx;
    DashboardData* m_data;

    QLabel* m_nameLabel;
    QLabel* m_statusDot;
    QLabel* m_tempLabel;
    QLabel* m_smokeLabel;
    QLabel* m_coLabel;
    QLabel* m_pirLabel;
    QLabel* m_sprinklerLabel;

    void applyStatusStyle(AlertLevel lvl);
};
