QT       += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = IntelliCareDashboard
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/MqttClient.cpp \
    src/DashboardData.cpp \
    src/widgets/RoomCard.cpp \
    src/widgets/MetricWidget.cpp \
    src/widgets/ActivityLog.cpp \
    src/widgets/DeviceControlPanel.cpp \
    src/widgets/HealthPanel.cpp \
    src/widgets/MotionPanel.cpp \
    src/widgets/ScenarioLauncher.cpp

HEADERS += \
    src/MainWindow.h \
    src/MqttClient.h \
    src/DashboardData.h \
    src/widgets/RoomCard.h \
    src/widgets/MetricWidget.h \
    src/widgets/ActivityLog.h \
    src/widgets/DeviceControlPanel.h \
    src/widgets/HealthPanel.h \
    src/widgets/MotionPanel.h \
    src/widgets/ScenarioLauncher.h

# mosquitto MQTT library
LIBS += -lmosquitto

# nlohmann/json (header-only) - copy from intellicare project
INCLUDEPATH += $$PWD/third_party

RESOURCES += resources/resources.qrc
