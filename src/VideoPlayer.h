#pragma once

#include <QGraphicsWidget>
#include <QString>

class AbstractVideo;
class QGraphicsProxyWidget;
class QGraphicsVideoItem;
class QPushButton;

class VideoPlayer : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QGraphicsItem *parent = nullptr);
    ~VideoPlayer() override;

    QGraphicsVideoItem *item() const;
    AbstractVideo *source() const;

    void setSource(AbstractVideo *source);
    void clear();

    void setCompact(bool compact);
    bool isCompact() const;

    void setGeometry(const QRectF &rect) override;

private:
    void layout();
    void updateOverlay();

    QGraphicsVideoItem *m_item = nullptr;
    QGraphicsProxyWidget *m_overlayProxy = nullptr;
    QPushButton *m_overlayButton = nullptr;
    AbstractVideo *m_source = nullptr;
    bool m_compact = false;
};
