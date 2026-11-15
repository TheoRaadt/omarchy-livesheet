// livesheet — a keyboard-only math notebook. The actual notebook (typing,
// live math rendering, matrices) all lives in src/notebook.html, run
// inside Main.qml's WebEngineView; this file just boots the window and
// hands it the two small C++ helpers (Theme, Backend) it needs.

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>
#include <QtWebEngineQuick>

#include "backend.h"
#include "theme.h"

int main(int argc, char *argv[]) {
    // Required by Qt WebEngine: must run before QGuiApplication exists.
    QtWebEngineQuick::initialize();
    QGuiApplication app(argc, argv);
    app.setApplicationName("livesheet");
    app.setApplicationVersion("0.1.0");

    // Matches the window to livesheet.desktop, so the Wayland compositor
    // and the app launcher use the installed icon instead of a generic one.
    app.setDesktopFileName("livesheet");
    app.setWindowIcon(QIcon::fromTheme("livesheet"));

    QQuickStyle::setStyle("Material");

    Theme theme;
    Backend backend;

    // Exposes `theme` and `backend` as globals inside every QML file, so
    // Main.qml can read theme.accent and call backend.exportPdf(...)
    // without any extra wiring.
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("theme", &theme);
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.load(QUrl("qrc:/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
