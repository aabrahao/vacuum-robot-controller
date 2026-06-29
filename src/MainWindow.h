#pragma once

#include <QList>
#include <QMainWindow>

class AbstractVideo;
class QLabel;
class WorkspaceView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    WorkspaceView *workspace() const;
    QList<AbstractVideo *> sources() const;
    void initialize();

private slots:
    void addCamera();
    void openFile();
    void openStream();
    void removeSource();
    void stopAll();
    void onControlTriggered(const QString &name);
    void onStatusMessage(const QString &message);
    void onErrorMessage(const QString &message);
    void onSourcesChanged();

private:
    bool addSource(AbstractVideo *source);
    void createActions();
    void createMenus();
    void updateActions();
    void updateStatus(const QString &message = QString());

    WorkspaceView *m_workspace = nullptr;
    QLabel *m_status = nullptr;
    QList<AbstractVideo *> m_sources;
    bool m_initialized = false;

    QAction *m_addCameraAction = nullptr;
    QAction *m_openFileAction = nullptr;
    QAction *m_openStreamAction = nullptr;
    QAction *m_removeAction = nullptr;
    QAction *m_stopAllAction = nullptr;
    QAction *m_quitAction = nullptr;
};
