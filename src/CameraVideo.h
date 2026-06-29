#pragma once

#include "AbstractVideo.h"

#include <QCamera>
#include <QCameraDevice>
#include <QList>

class QMediaCaptureSession;

class CameraVideo : public AbstractVideo
{
    Q_OBJECT

public:
    explicit CameraVideo(const QCameraDevice &device, QObject *parent = nullptr);
    ~CameraVideo() override;

    static QList<QCameraDevice> enumerate();
    static QCameraDevice preferred();

    QString id() const;
    QString key() const override;
    QCameraDevice device() const;
    QString description() const override;
    bool isActive() const override;

    void play() override;
    void pause() override;
    void stop() override;

    void bind(QObject *output) override;
    void unbind() override;

private:
    QString m_id;
    QCameraDevice m_device;
    QCamera *m_camera = nullptr;
    QMediaCaptureSession *m_session = nullptr;
};
