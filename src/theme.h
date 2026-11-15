#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QProcess>
#include <QTimer>

class Theme : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString accent READ accent NOTIFY changed)
    Q_PROPERTY(QString accentForeground READ accentForeground NOTIFY changed)
    Q_PROPERTY(int radius READ radius NOTIFY changed)

public:
    // `currentDirectory` defaults to Omarchy's own theme state directory;
    // tests pass a temp directory instead. `hyprctl` lets tests substitute
    // a fake script for the real binary so rounding can be tested without
    // a running Hyprland session.
    explicit Theme(const QString &currentDirectory = {}, QObject *parent = nullptr, const QString &hyprctl = {});
    ~Theme() override;

    QString accent() const { return m_accent; }

    // Black or white, whichever reads better on top of the current accent.
    QString accentForeground() const;

    int radius() const { return m_radius; }

    // Pulls the accent color out of an Omarchy colors.toml file, or the
    // fallback yellow if the file is missing or has no accent line.
    static QString readAccent(const QString &path);

signals:
    // Fired whenever accent, accentForeground, or radius changes.
    void changed();

private:
    // Re-reads the accent file and re-arms the filesystem watcher on it
    // (themes replace the file rather than editing it in place, which
    // drops it from the watch unless this runs after every change).
    void reload();

    // Asks hyprctl for the current window-rounding value, if not already
    // waiting on a previous request.
    void refreshRounding();

    QString m_directory;
    QString m_accent = "#FFD60A";
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
    QString m_hyprctl;
    int m_radius = 0;
    QProcess m_roundingProcess;
    QTimer m_roundingPoll, m_roundingTimeout;
};
