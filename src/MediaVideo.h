#pragma once

#include "AbstractVideo.h"

#include <QMediaPlayer>
#include <QUrl>

class QAudioOutput;

class MediaVideo : public AbstractVideo
{
    Q_OBJECT

public:
    explicit MediaVideo(const QUrl &source, QObject *parent = nullptr);
    ~MediaVideo() override;

    QUrl source() const;
    QString key() const override;
    QString description() const override;
    bool isActive() const override;

    void play() override;
    void pause() override;
    void stop() override;

    void bind(QObject *output) override;
    void unbind() override;

private:
    QUrl m_source;
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
};
