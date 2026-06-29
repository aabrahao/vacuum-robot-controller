#include "WorkspaceView.h"
#include "AbstractVideo.h"
#include "VideoPlayer.h"
#include "VideoToolbar.h"

#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

namespace {

const char *kSliderStyle = R"(
    QSlider::groove:vertical {
        background: rgba(255, 255, 255, 40);
        width: 6px;
        border-radius: 3px;
    }
    QSlider::handle:vertical {
        background: white;
        height: 16px;
        width: 16px;
        margin: 0 -5px;
        border-radius: 8px;
    }
    QSlider::sub-page:vertical {
        background: rgba(255, 255, 255, 180);
        border-radius: 3px;
    }
    QSlider::add-page:vertical {
        background: rgba(255, 255, 255, 40);
        border-radius: 3px;
    }
)";

constexpr qreal kFloatingWidth = 160.0;
constexpr qreal kFloatingHeight = 90.0;
constexpr qreal kFloatingSpacing = 10.0;
constexpr qreal kFloatingMargin = 12.0;

} // namespace

WorkspaceView::WorkspaceView(QWidget *parent)
    : QGraphicsView(parent)
    , m_placeholder(new QGraphicsTextItem())
{
    auto *scene = new QGraphicsScene(this);
    setScene(scene);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QColor(24, 24, 24));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setMinimumSize(640, 480);

    setRenderHint(QPainter::Antialiasing, false);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);
    setAttribute(Qt::WA_NativeWindow, false);
    viewport()->setAttribute(Qt::WA_NativeWindow, false);

    m_placeholder->setPlainText(tr("No video playing.\nUse Video -> Add Camera, Open File, or Open Stream."));
    m_placeholder->setDefaultTextColor(QColor(220, 220, 220));
    m_placeholder->setZValue(5);
    scene->addItem(m_placeholder);
}

WorkspaceView::~WorkspaceView()
{
    m_shuttingDown = true;
    detachAll();
    qDeleteAll(m_toolbars);
    m_toolbars.clear();
}

void WorkspaceView::setup()
{
    if (m_ready) {
        return;
    }
    m_ready = true;

    auto *toolbar = new VideoToolbar(this);
    toolbar->setHorizontalAnchor(VideoToolbar::Horizontal::Right);
    toolbar->setVerticalAnchor(VideoToolbar::Vertical::Center);
    toolbar->addStretch();

    m_startButton = VideoToolbar::makeButton(QStringLiteral("▶"), toolbar->widget());
    m_stopButton = VideoToolbar::makeButton(QStringLiteral("■"), toolbar->widget());
    m_forwardButton = VideoToolbar::makeButton(QStringLiteral("▲"), toolbar->widget());
    m_backButton = VideoToolbar::makeButton(QStringLiteral("▼"), toolbar->widget());
    m_homeButton = VideoToolbar::makeButton(QStringLiteral("⌂"), toolbar->widget());

    toolbar->addWidget(m_startButton);
    toolbar->addWidget(m_stopButton);
    toolbar->addWidget(m_forwardButton);
    toolbar->addWidget(m_backButton);
    toolbar->addWidget(m_homeButton);

    m_slider = new QSlider(Qt::Vertical, toolbar->widget());
    m_slider->setRange(0, 100);
    m_slider->setValue(50);
    m_slider->setFixedHeight(120);
    m_slider->setFixedWidth(28);
    m_slider->setCursor(Qt::PointingHandCursor);
    m_slider->setStyleSheet(kSliderStyle);
    m_slider->setFocusPolicy(Qt::NoFocus);
    toolbar->addWidget(m_slider, 0, Qt::AlignHCenter);

    toolbar->addStretch();

    connect(m_startButton, &QPushButton::clicked, this, [this] {
        emit controlTriggered(QStringLiteral("start"));
    });
    connect(m_stopButton, &QPushButton::clicked, this, [this] {
        emit controlTriggered(QStringLiteral("stop"));
    });
    connect(m_forwardButton, &QPushButton::clicked, this, [this] {
        emit controlTriggered(QStringLiteral("forward"));
    });
    connect(m_backButton, &QPushButton::clicked, this, [this] {
        emit controlTriggered(QStringLiteral("back"));
    });
    connect(m_homeButton, &QPushButton::clicked, this, [this] {
        emit controlTriggered(QStringLiteral("home"));
    });
    connect(m_slider, &QSlider::valueChanged, this, [this](int value) {
        emit controlTriggered(QStringLiteral("speed:%1").arg(value));
    });

    fit();
    QTimer::singleShot(0, this, &WorkspaceView::fit);
}

int WorkspaceView::running() const
{
    return m_players.size();
}

bool WorkspaceView::hasPlayer(const AbstractVideo *source) const
{
    return playerFor(source) != nullptr;
}

VideoPlayer *WorkspaceView::playerFor(const AbstractVideo *source) const
{
    if (!source) {
        return nullptr;
    }

    const QString sourceKey = source->key();
    for (VideoPlayer *player : m_players) {
        if (player->source() && player->source()->key() == sourceKey) {
            return player;
        }
    }
    return nullptr;
}

VideoPlayer *WorkspaceView::addPlayer(AbstractVideo *source)
{
    auto *player = new VideoPlayer();
    m_players.append(player);
    scene()->addItem(player);
    player->setSource(source);

    connect(source, &AbstractVideo::errorOccurred, this, [this](const QString &message) {
        if (!m_shuttingDown) {
            emit errorMessage(tr("Video error: %1").arg(message));
        }
    });
    connect(source, &AbstractVideo::activeChanged, this, [this](bool) {
        if (!m_shuttingDown) {
            emit sourcesChanged();
        }
    });

    layoutPlayers();
    updateEmpty();
    return player;
}

void WorkspaceView::removePlayer(VideoPlayer *player)
{
    if (!m_players.removeOne(player)) {
        return;
    }

    player->clear();
    scene()->removeItem(player);
    delete player;

    layoutPlayers();
    updateEmpty();
}

void WorkspaceView::layoutPlayers()
{
    const QRectF viewRect = viewport()->rect();
    if (viewRect.isEmpty() || m_players.isEmpty()) {
        return;
    }

    VideoPlayer *primary = m_players.first();
    const int floatingCount = m_players.size() - 1;

    primary->setCompact(false);
    primary->setZValue(1);
    primary->setGeometry(viewRect);

    if (floatingCount == 0) {
        return;
    }

    qreal x = kFloatingMargin;
    const qreal y = viewRect.height() - kFloatingMargin - kFloatingHeight;

    for (VideoPlayer *player : m_players) {
        if (player == primary) {
            continue;
        }

        player->setCompact(true);
        player->setZValue(8);
        player->setGeometry(QRectF(x, y, kFloatingWidth, kFloatingHeight));
        x += kFloatingWidth + kFloatingSpacing;
    }
}

bool WorkspaceView::attach(AbstractVideo *source)
{
    if (!source || hasPlayer(source)) {
        return false;
    }

    addPlayer(source);

    emit sourcesChanged();
    emit statusMessage(tr("Playing: %1").arg(source->description()));
    return true;
}

void WorkspaceView::detach(const AbstractVideo *source)
{
    VideoPlayer *player = playerFor(source);
    if (!player) {
        return;
    }

    removePlayer(player);

    if (!m_shuttingDown) {
        emit sourcesChanged();
        emit statusMessage(tr("Stopped: %1").arg(source->description()));
    }
}

void WorkspaceView::detachAll()
{
    while (!m_players.isEmpty()) {
        removePlayer(m_players.first());
    }

    if (!m_shuttingDown) {
        emit sourcesChanged();
        emit statusMessage(tr("All videos stopped."));
    }
}

void WorkspaceView::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);
    fit();
}

void WorkspaceView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fit();
}

void WorkspaceView::fit()
{
    const QRectF viewRect = viewport()->rect();
    if (viewRect.isEmpty()) {
        return;
    }

    scene()->setSceneRect(viewRect);
    layoutPlayers();
    updateEmpty();

    for (VideoToolbar *toolbar : m_toolbars) {
        toolbar->adjust();
    }
}

void WorkspaceView::addToolbar(VideoToolbar *toolbar)
{
    if (!toolbar || m_toolbars.contains(toolbar)) {
        return;
    }
    m_toolbars.append(toolbar);
}

void WorkspaceView::removeToolbar(VideoToolbar *toolbar)
{
    m_toolbars.removeOne(toolbar);
}

void WorkspaceView::updateEmpty()
{
    const bool showPlaceholder = m_players.isEmpty();
    m_placeholder->setVisible(showPlaceholder);

    if (!showPlaceholder) {
        return;
    }

    const QRectF viewRect = viewport()->rect();
    const QRectF textRect = m_placeholder->boundingRect();
    m_placeholder->setPos((viewRect.width() - textRect.width()) / 2.0,
                          (viewRect.height() - textRect.height()) / 2.0);
}
