#include "VideoPlayer.h"
#include "AbstractVideo.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsVideoItem>
#include <QPushButton>

namespace {

const char *kOverlayButtonStyle = R"(
    QPushButton {
        background-color: rgba(0, 0, 0, 140);
        border: 1px solid rgba(255, 255, 255, 180);
        border-radius: 20px;
        color: white;
        font-size: 14px;
        font-weight: bold;
        padding: 4px 10px;
    }
    QPushButton:hover {
        background-color: rgba(30, 30, 30, 200);
    }
)";

} // namespace

VideoPlayer::VideoPlayer(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
    , m_item(new QGraphicsVideoItem())
    , m_overlayButton(new QPushButton())
    , m_overlayProxy(new QGraphicsProxyWidget(this))
{
    m_item->setParentItem(this);
    m_item->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
    m_item->setZValue(0);

    m_overlayButton->setStyleSheet(kOverlayButtonStyle);
    m_overlayButton->setFocusPolicy(Qt::NoFocus);
    m_overlayButton->setCursor(Qt::PointingHandCursor);
    m_overlayProxy->setWidget(m_overlayButton);
    m_overlayProxy->setZValue(1);

    connect(m_overlayButton, &QPushButton::clicked, this, [this] {
        if (!m_source) {
            return;
        }
        if (m_source->isActive()) {
            m_source->pause();
        } else {
            m_source->play();
        }
    });
}

VideoPlayer::~VideoPlayer()
{
    clear();
}

QGraphicsVideoItem *VideoPlayer::item() const
{
    return m_item;
}

AbstractVideo *VideoPlayer::source() const
{
    return m_source;
}

void VideoPlayer::setSource(AbstractVideo *source)
{
    if (m_source == source) {
        return;
    }

    clear();

    m_source = source;
    if (!m_source) {
        updateOverlay();
        return;
    }

    connect(m_source, &AbstractVideo::activeChanged, this, [this](bool) {
        updateOverlay();
    });
    connect(m_source, &AbstractVideo::errorOccurred, this, [this](const QString &message) {
        m_overlayButton->setToolTip(message);
    });

    m_source->bind(m_item);
    m_source->play();
    updateOverlay();
    layout();
}

void VideoPlayer::clear()
{
    if (!m_source) {
        return;
    }

    disconnect(m_source, nullptr, this, nullptr);
    m_source->stop();
    m_source->unbind();
    m_source = nullptr;
    updateOverlay();
}

void VideoPlayer::setCompact(bool compact)
{
    if (m_compact == compact) {
        return;
    }
    m_compact = compact;
    m_overlayProxy->setVisible(!compact);
    layout();
}

bool VideoPlayer::isCompact() const
{
    return m_compact;
}

void VideoPlayer::setGeometry(const QRectF &rect)
{
    QGraphicsWidget::setGeometry(rect);
    layout();
}

void VideoPlayer::layout()
{
    const QRectF area = rect();
    if (area.isEmpty()) {
        return;
    }

    m_item->setPos(area.topLeft());
    m_item->setSize(area.size());

    m_overlayButton->adjustSize();
    const QSizeF overlaySize = m_overlayProxy->size();
    const qreal margin = m_compact ? 4.0 : 8.0;
    m_overlayProxy->setPos(area.right() - overlaySize.width() - margin,
                           area.top() + margin);
}

void VideoPlayer::updateOverlay()
{
    if (!m_source) {
        m_overlayButton->setText(QStringLiteral("—"));
        m_overlayButton->setToolTip(QString());
        return;
    }

    const QString label = m_source->isActive() ? QStringLiteral("⏸") : QStringLiteral("▶");
    m_overlayButton->setText(label);
    m_overlayButton->setToolTip(m_source->description());
}
