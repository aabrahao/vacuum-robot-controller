#include "MainWindow.h"

#include <QApplication>
#include <QScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Vacuum Robot Controller"));
    app.setOrganizationName(QStringLiteral("FIU-ARC"));

    MainWindow window;
    window.resize(1280, 720);
    window.setMinimumSize(800, 600);

    if (QScreen *screen = QApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        const QSize size = window.size();
        window.move(available.center() - QPoint(size.width() / 2, size.height() / 2));
    }

    window.show();

    QTimer::singleShot(0, &window, [&window]() {
        window.initialize();
    });

    return app.exec();
}
