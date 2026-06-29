#pragma once

#include <QGraphicsProxyWidget>

class QBoxLayout;
class QPushButton;
class WorkspaceView;

class VideoToolbar : public QGraphicsProxyWidget
{
public:
    enum class Vertical { Top, Center, Bottom };
    enum class Horizontal { Left, Center, Right };

    explicit VideoToolbar(WorkspaceView *workspace);
    ~VideoToolbar() override;

    QBoxLayout *layout() const;

    void setVerticalAnchor(Vertical anchor);
    void setHorizontalAnchor(Horizontal anchor);
    Vertical verticalAnchor() const;
    Horizontal horizontalAnchor() const;

    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = {});
    void addStretch(int stretch = 1);

    void adjust();

    static QPushButton *makeButton(const QString &label, QWidget *parent);

private:
    void updateLayoutDirection();

    WorkspaceView *m_workspace = nullptr;
    QBoxLayout *m_layout = nullptr;
    Vertical m_vertical = Vertical::Center;
    Horizontal m_horizontal = Horizontal::Right;

    static constexpr qreal kMargin = 12.0;
};
