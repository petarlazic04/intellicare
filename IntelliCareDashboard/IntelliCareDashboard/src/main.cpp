#include <QApplication>
#include <QFont>
#include <QMetaType>
#include "MainWindow.h"
#include "DashboardData.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Register custom types for queued signal/slot connections
    qRegisterMetaType<LogEntry>("LogEntry");

    QFont font("Segoe UI");
    font.setPointSize(10);
    app.setFont(font);

    app.setApplicationName("IntelliCare Dashboard");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("IntelliCare");

    MainWindow window;
    window.show();

    return app.exec();
}
