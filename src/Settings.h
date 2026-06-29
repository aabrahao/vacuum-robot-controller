#pragma once

#include <QHash>
#include <QString>
#include <QVariant>

class QMainWindow;

struct Property
{
    QString name;
    QVariant value;
    QVariant defaultValue;
    QString description;
};

class Settings
{
public:
    static Settings &instance();

    template<typename T>
    T get(const QString &name, const T &fallback = T{}) const
    {
        const QVariant variant = valueOf(name);
        if (!variant.isValid()) {
            return fallback;
        }
        if (variant.canConvert<T>()) {
            return variant.value<T>();
        }
        return fallback;
    }

    template<typename T>
    bool set(const QString &name, const T &value)
    {
        return set(name, QVariant::fromValue(value));
    }

    bool set(const QString &name, const QVariant &value);

    Property property(const QString &name) const;
    QStringList names() const;

    bool load();
    bool save() const;

    void apply(QMainWindow *window) const;
    void capture(const QMainWindow *window);
    void center(QMainWindow *window) const;

    Settings(const Settings &) = delete;
    Settings &operator=(const Settings &) = delete;

private:
    Settings() = default;

    QVariant valueOf(const QString &name) const;
    bool updateValue(const QString &name, const QVariant &value);

    QList<Property> m_properties;
    QHash<QString, int> m_index;
};
