#pragma once

#include <QObject>
#include <QString>

class AbstractVideo : public QObject
{
    Q_OBJECT

public:
    explicit AbstractVideo(QObject *parent = nullptr);
    ~AbstractVideo() override;

    virtual QString description() const = 0;
    virtual QString key() const;
    virtual bool isActive() const = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    virtual void bind(QObject *output) = 0;
    virtual void unbind() = 0;

signals:
    void activeChanged(bool active);
    void errorOccurred(const QString &message);
};
