#include <QApplication>
#include <QFont>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Application-wide font
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
