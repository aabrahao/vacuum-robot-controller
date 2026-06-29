#include "MainWindow.h"

#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Vacuum Robot Controller"));
    app.setOrganizationName(QStringLiteral("FIU-ARC"));

    MainWindow window;
    window.show();

    QTimer::singleShot(0, &window, [&window]() {
        window.initialize();
    });

    return app.exec();
}
