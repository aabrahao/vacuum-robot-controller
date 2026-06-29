#include "CameraVideo.h"

#include <QHash>
#include <QMediaCaptureSession>
#include <QMediaDevices>

namespace {

QString idFor(const QCameraDevice &device)
{
    return QString::fromUtf8(device.id());
}

QString normalize(const QString &description)
{
    QString normalized = description.trimmed();
    const int paren = normalized.lastIndexOf(QStringLiteral(" ("));
    if (paren > 0) {
        normalized = normalized.left(paren).trimmed();
    }
    return normalized;
}

bool isUsable(const QCameraDevice &device)
{
    return !device.isNull() && !device.videoFormats().isEmpty();
}

bool prefer(const QCameraDevice &candidate, const QCameraDevice &current)
{
    if (current.isNull()) {
        return true;
    }
    if (candidate.isDefault() && !current.isDefault()) {
        return true;
    }
    if (!candidate.isDefault() && current.isDefault()) {
        return false;
    }
    return candidate.videoFormats().size() > current.videoFormats().size();
}

QList<QCameraDevice> enumeratedDevices()
{
    QHash<QString, QCameraDevice> byDescription;

    for (const QCameraDevice &device : QMediaDevices::videoInputs()) {
        if (!isUsable(device)) {
            continue;
        }

        const QString key = normalize(device.description());
        if (!byDescription.contains(key) || prefer(device, byDescription.value(key))) {
            byDescription.insert(key, device);
        }
    }

    if (byDescription.isEmpty()) {
        for (const QCameraDevice &device : QMediaDevices::videoInputs()) {
            if (device.isNull()) {
                continue;
            }
            const QString key = normalize(device.description());
            if (!byDescription.contains(key) || prefer(device, byDescription.value(key))) {
                byDescription.insert(key, device);
            }
        }
    }

    return byDescription.values();
}

QCameraDevice preferredDevice(const QList<QCameraDevice> &devices)
{
    if (devices.isEmpty()) {
        return {};
    }

    const QCameraDevice defaultDevice = QMediaDevices::defaultVideoInput();
    for (const QCameraDevice &device : devices) {
        if (device.id() == defaultDevice.id()) {
            return device;
        }
    }

    for (const QCameraDevice &device : devices) {
        if (device.isDefault()) {
            return device;
        }
    }

    return devices.first();
}

} // namespace

QList<QCameraDevice> CameraVideo::enumerate()
{
    return enumeratedDevices();
}

QCameraDevice CameraVideo::preferred()
{
    return preferredDevice(enumeratedDevices());
}

CameraVideo::CameraVideo(const QCameraDevice &device, QObject *parent)
    : AbstractVideo(parent)
    , m_id(idFor(device))
    , m_device(device)
    , m_camera(new QCamera(device, this))
    , m_session(new QMediaCaptureSession(this))
{
    m_session->setCamera(m_camera);

    connect(m_camera, &QCamera::activeChanged, this, &AbstractVideo::activeChanged);
    connect(m_camera, &QCamera::errorOccurred, this, [this](QCamera::Error, const QString &errorString) {
        emit errorOccurred(errorString);
    });
}

CameraVideo::~CameraVideo()
{
    stop();
    unbind();
}

QString CameraVideo::id() const
{
    return m_id;
}

QString CameraVideo::key() const
{
    return m_id;
}

QCameraDevice CameraVideo::device() const
{
    return m_device;
}

QString CameraVideo::description() const
{
    return m_device.description();
}

bool CameraVideo::isActive() const
{
    return m_camera && m_camera->isActive();
}

void CameraVideo::play()
{
    if (m_camera && !m_camera->isActive()) {
        m_camera->start();
    }
}

void CameraVideo::pause()
{
    stop();
}

void CameraVideo::stop()
{
    if (m_camera && m_camera->isActive()) {
        m_camera->stop();
    }
}

void CameraVideo::bind(QObject *output)
{
    if (m_session) {
        m_session->setVideoOutput(output);
    }
}

void CameraVideo::unbind()
{
    if (m_session) {
        m_session->setVideoOutput(static_cast<QObject *>(nullptr));
    }
}
