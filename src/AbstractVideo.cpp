#include "AbstractVideo.h"

AbstractVideo::AbstractVideo(QObject *parent)
    : QObject(parent)
{
}

AbstractVideo::~AbstractVideo() = default;

QString AbstractVideo::key() const
{
    return description();
}
