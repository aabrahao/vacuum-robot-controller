#include "VideoToolbar.h"
#include "WorkspaceView.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QtMath>

namespace {

const char *kButtonStyle = R"(
    QPushButton {
        background-color: rgba(0, 0, 0, 140);
        border: 1px solid rgba(255, 255, 255, 180);
        border-radius: 24px;
        color: white;
        font-size: 18px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: rgba(30, 30, 30, 200);
        border-color: rgba(255, 255, 255, 220);
    }
    QPushButton:pressed {
        background-color: rgba(255, 255, 255, 180);
        color: black;
    }
)";

} // namespace

VideoToolbar::VideoToolbar(WorkspaceView *workspace)
    : m_workspace(workspace)
{
    auto *panel = new QWidget();
    panel->setAttribute(Qt::WA_TranslucentBackground);
    panel->setStyleSheet(QStringLiteral("background: transparent;"));

    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, panel);
    m_layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    m_layout->setSpacing(10);

    setWidget(panel);
    setZValue(10);
    workspace->scene()->addItem(this);
    workspace->addToolbar(this);
}

VideoToolbar::~VideoToolbar()
{
    if (m_workspace) {
        m_workspace->removeToolbar(this);
    }
}

QBoxLayout *VideoToolbar::layout() const
{
    return m_layout;
}

void VideoToolbar::setVerticalAnchor(Vertical anchor)
{
    if (m_vertical == anchor) {
        return;
    }
    m_vertical = anchor;
    updateLayoutDirection();
    adjust();
}

void VideoToolbar::setHorizontalAnchor(Horizontal anchor)
{
    if (m_horizontal == anchor) {
        return;
    }
    m_horizontal = anchor;
    updateLayoutDirection();
    adjust();
}

VideoToolbar::Vertical VideoToolbar::verticalAnchor() const
{
    return m_vertical;
}

VideoToolbar::Horizontal VideoToolbar::horizontalAnchor() const
{
    return m_horizontal;
}

void VideoToolbar::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    m_layout->addWidget(widget, stretch, alignment);
}

void VideoToolbar::addStretch(int stretch)
{
    m_layout->addStretch(stretch);
}

void VideoToolbar::adjust()
{
    if (!m_workspace || !widget()) {
        return;
    }

    const QRectF viewRect = m_workspace->viewport()->rect();
    if (viewRect.isEmpty()) {
        return;
    }

    QWidget *panel = widget();
    panel->adjustSize();
    resize(panel->sizeHint());

    const QSizeF panelSize = size();

    qreal x = 0;
    switch (m_horizontal) {
    case Horizontal::Left:
        x = kMargin;
        break;
    case Horizontal::Right:
        x = viewRect.width() - panelSize.width() - kMargin;
        break;
    case Horizontal::Center:
        x = (viewRect.width() - panelSize.width()) / 2.0;
        break;
    }

    qreal y = 0;
    switch (m_vertical) {
    case Vertical::Top:
        y = kMargin;
        break;
    case Vertical::Bottom:
        y = viewRect.height() - panelSize.height() - kMargin;
        break;
    case Vertical::Center:
        y = qMax(kMargin, (viewRect.height() - panelSize.height()) / 2.0);
        break;
    }

    setPos(x, y);
}

QPushButton *VideoToolbar::makeButton(const QString &label, QWidget *parent)
{
    auto *button = new QPushButton(label, parent);
    button->setFixedSize(48, 48);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(kButtonStyle);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}

void VideoToolbar::updateLayoutDirection()
{
    const bool sideAnchored = m_horizontal == Horizontal::Left || m_horizontal == Horizontal::Right;
    m_layout->setDirection(sideAnchored ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
}
