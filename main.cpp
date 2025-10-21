#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // Force Qt to use XCB before QApplication is created
    const char* session = std::getenv("XDG_SESSION_TYPE");

    if (session && std::strcmp(session, "wayland") == 0) {
        qDebug("Detected Wayland session → forcing xcb");
        qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    }

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
