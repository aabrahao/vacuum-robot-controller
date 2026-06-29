#include "MediaVideo.h"

#include <QAudioOutput>

MediaVideo::MediaVideo(const QUrl &source, QObject *parent)
    : AbstractVideo(parent)
    , m_source(source)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(m_source);

    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        emit activeChanged(state == QMediaPlayer::PlayingState);
    });
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &errorString) {
        emit errorOccurred(errorString);
    });
}

MediaVideo::~MediaVideo()
{
    stop();
    unbind();
}

QUrl MediaVideo::source() const
{
    return m_source;
}

QString MediaVideo::key() const
{
    return m_source.toString(QUrl::FullyEncoded);
}

QString MediaVideo::description() const
{
    return m_source.toString(QUrl::PreferLocalFile | QUrl::PrettyDecoded);
}

bool MediaVideo::isActive() const
{
    return m_player && m_player->playbackState() == QMediaPlayer::PlayingState;
}

void MediaVideo::play()
{
    if (m_player) {
        m_player->play();
    }
}

void MediaVideo::pause()
{
    if (m_player) {
        m_player->pause();
    }
}

void MediaVideo::stop()
{
    if (m_player) {
        m_player->stop();
    }
}

void MediaVideo::bind(QObject *output)
{
    if (m_player) {
        m_player->setVideoOutput(output);
    }
}

void MediaVideo::unbind()
{
    if (m_player) {
        m_player->setVideoOutput(static_cast<QObject *>(nullptr));
    }
}
