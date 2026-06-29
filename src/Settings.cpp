#include "Settings.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QMainWindow>
#include <QScreen>
#include <QSettings>

namespace {

constexpr auto kDefaultSettingsResource = ":/configs/default.ini";

const QString kWindowWidth = QStringLiteral("window.width");
const QString kWindowHeight = QStringLiteral("window.height");
const QString kWindowMinWidth = QStringLiteral("window.min_width");
const QString kWindowMinHeight = QStringLiteral("window.min_height");

bool ensureUserSettings()
{
    QSettings settings;
    const QString userPath = settings.fileName();
    if (QFile::exists(userPath)) {
        return true;
    }

    QDir().mkpath(QFileInfo(userPath).absolutePath());
    if (!QFile::copy(kDefaultSettingsResource, userPath)) {
        return false;
    }

    return QFile::setPermissions(userPath,
                                 QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

Property readProperty(QSettings &settings, const QString &name)
{
    settings.beginGroup(name);

    Property property;
    property.name = name;
    property.value = settings.value(QStringLiteral("value"));
    property.defaultValue = settings.value(QStringLiteral("default"));
    property.description = settings.value(QStringLiteral("description")).toString();

    settings.endGroup();

    if (!property.defaultValue.isValid()) {
        property.defaultValue = property.value;
    }
    if (!property.value.isValid()) {
        property.value = property.defaultValue;
    }

    return property;
}

void writeProperty(QSettings &settings, const Property &property)
{
    settings.beginGroup(property.name);
    settings.setValue(QStringLiteral("value"), property.value);
    settings.setValue(QStringLiteral("default"), property.defaultValue);
    settings.setValue(QStringLiteral("description"), property.description);
    settings.endGroup();
}

} // namespace

Settings &Settings::instance()
{
    static Settings settings;
    return settings;
}

QVariant Settings::valueOf(const QString &name) const
{
    const auto it = m_index.constFind(name);
    if (it == m_index.constEnd()) {
        return {};
    }
    return m_properties.at(it.value()).value;
}

bool Settings::set(const QString &name, const QVariant &value)
{
    if (!updateValue(name, value)) {
        return false;
    }
    return save();
}

bool Settings::updateValue(const QString &name, const QVariant &value)
{
    const auto it = m_index.find(name);
    if (it == m_index.end()) {
        return false;
    }

    m_properties[it.value()].value = value;
    return true;
}

Property Settings::property(const QString &name) const
{
    const auto it = m_index.constFind(name);
    if (it == m_index.constEnd()) {
        return {};
    }
    return m_properties.at(it.value());
}

QStringList Settings::names() const
{
    QStringList result;
    result.reserve(m_properties.size());
    for (const Property &property : m_properties) {
        result.append(property.name);
    }
    return result;
}

bool Settings::load()
{
    if (!ensureUserSettings()) {
        return false;
    }

    QSettings settings;

    m_properties.clear();
    m_index.clear();

    const QStringList groups = settings.childGroups();
    m_properties.reserve(groups.size());

    for (const QString &name : groups) {
        if (name.isEmpty() || m_index.contains(name)) {
            continue;
        }

        Property property = readProperty(settings, name);
        m_index.insert(name, m_properties.size());
        m_properties.append(property);
    }

    return true;
}

bool Settings::save() const
{
    QSettings settings;
    settings.clear();

    for (const Property &property : m_properties) {
        writeProperty(settings, property);
    }

    settings.sync();
    return settings.status() == QSettings::NoError;
}

void Settings::apply(QMainWindow *window) const
{
    if (!window) {
        return;
    }

    window->resize(get<int>(kWindowWidth), get<int>(kWindowHeight));
    window->setMinimumSize(get<int>(kWindowMinWidth), get<int>(kWindowMinHeight));
}

void Settings::capture(const QMainWindow *window)
{
    if (!window) {
        return;
    }

    updateValue(kWindowWidth, window->width());
    updateValue(kWindowHeight, window->height());
    save();
}

void Settings::center(QMainWindow *window) const
{
    if (!window) {
        return;
    }

    if (QScreen *screen = QApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        window->move(available.center() - window->frameGeometry().center());
    }
}
