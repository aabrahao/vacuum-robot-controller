#pragma once

#include <QGraphicsView>
#include <QList>
#include <QResizeEvent>
#include <QShowEvent>
#include <QString>

class AbstractVideo;
class QGraphicsTextItem;
class QPushButton;
class QSlider;
class VideoPlayer;
class VideoToolbar;

class WorkspaceView : public QGraphicsView
{
    Q_OBJECT

    friend class VideoToolbar;

public:
    explicit WorkspaceView(QWidget *parent = nullptr);
    ~WorkspaceView() override;

    void setup();

    int running() const;
    bool hasPlayer(const AbstractVideo *source) const;

    bool attach(AbstractVideo *source);
    void detach(const AbstractVideo *source);
    void detachAll();

signals:
    void controlTriggered(const QString &name);
    void sourcesChanged();
    void statusMessage(const QString &message);
    void errorMessage(const QString &message);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    VideoPlayer *playerFor(const AbstractVideo *source) const;
    VideoPlayer *addPlayer(AbstractVideo *source);
    void removePlayer(VideoPlayer *player);
    void layoutPlayers();
    void fit();
    void updateEmpty();
    void addToolbar(VideoToolbar *toolbar);
    void removeToolbar(VideoToolbar *toolbar);

    QGraphicsTextItem *m_placeholder = nullptr;

    QList<VideoPlayer *> m_players;

    QList<VideoToolbar *> m_toolbars;

    QPushButton *m_startButton = nullptr;
    QPushButton *m_stopButton = nullptr;
    QPushButton *m_forwardButton = nullptr;
    QPushButton *m_backButton = nullptr;
    QPushButton *m_homeButton = nullptr;
    QSlider *m_slider = nullptr;

    bool m_ready = false;
    bool m_shuttingDown = false;
};
