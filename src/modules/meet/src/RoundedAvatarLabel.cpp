#include "RoundedAvatarLabel.h"

#include <QPainter>
#include <QStyle>

RoundedAvatarLabel::RoundedAvatarLabel(QWidget* parent)
    :QWidget(parent)
{
}

QSize RoundedAvatarLabel::sizeHint() const
{
    return _contentsSize;
}

QSize RoundedAvatarLabel::minimumSizeHint() const
{
    return _contentsSize;
}

void RoundedAvatarLabel::setContentsSize(const QSize &size)
{
    if(_contentsSize != size)
    {
        _contentsSize = size;
        updateGeometry();
    }
}

void RoundedAvatarLabel::setPixmap(const QPixmap &pixmap)
{
    _pixmap = pixmap;
    _cachePixmap = QPixmap();
    updateGeometry();
}

void RoundedAvatarLabel::setMaskOnPixmap(bool pixmap)
{
    if(maskPixmap != pixmap)
    {
        maskPixmap = pixmap;
        update();
    }
}

void RoundedAvatarLabel::setPixmapAlign(Qt::Alignment alignment)
{
    if(_align != alignment)
    {
        _align = alignment;
        update();
    }
}

void RoundedAvatarLabel::setScaleMode(PixmapScaleMode mode)
{
    if(_scaleMode != mode)
    {
        _scaleMode = mode;
        updateGeometry();
        _cachePixmap = QPixmap();
    }
}

void RoundedAvatarLabel::setRoundedType(RoundedType type)
{
    if(_roundType != type)
    {
        _roundType = type;
        update();
    }
}

void RoundedAvatarLabel::setRoundRadius(int xRadius, int yRadius)
{
    roundRadius.rx() = xRadius;
    roundRadius.ry() = yRadius;
    update();
}

void RoundedAvatarLabel::paintEvent(QPaintEvent *)
{
    if(_pixmap.isNull())
        return;

    _pixmap.setDevicePixelRatio(this->devicePixelRatioF());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect rect = paintRect();
    QPainterPath roundPath = roundMaskPath(maskPixmap ? rect : this->rect());
    if(!roundPath.isEmpty())
        painter.setClipPath(roundPath);

    if(_scaleMode == NoScale)
    {
        painter.drawPixmap(rect.topLeft(), _pixmap);
        return;
    }

    QSize new_size = rect.size() * this->devicePixelRatioF();
    if(new_size != _cachePixmap.size())
    {
        _cachePixmap = _pixmap.scaled(rect.size() * this->devicePixelRatioF(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    painter.drawPixmap(rect.topLeft(), _cachePixmap);
}

QRect RoundedAvatarLabel::paintRect()
{
    QSize wgt_size = _contentsSize;
    if(_contentsSize.isEmpty())
        wgt_size = this->size();
    QSize pix_size = _pixmap.size() / this->devicePixelRatioF();
    switch (_scaleMode) {
    case IgnoreAspectRatio:
        pix_size = wgt_size;
        break;
    case KeepAspectRatio:
        pix_size = pix_size.scaled(wgt_size, Qt::KeepAspectRatio);
        break;
    case KeepAspectRatioByExpanding:
        pix_size = pix_size.scaled(wgt_size, Qt::KeepAspectRatioByExpanding);
        break;
    default:
        break;
    }
    return QStyle::alignedRect(Qt::LeftToRight, _align, pix_size, this->rect());
}

QPainterPath RoundedAvatarLabel::roundMaskPath(const QRect &rect)
{
    QPainterPath path;

    if(_roundType == MinEdgeCircle || _roundType == MaxEdgeCircle)
    {
        int w = 0;
        if(_roundType == MinEdgeCircle)
            w = std::min(rect.width(), rect.height());
        else
            w = std::max(rect.width(), rect.height());
        QRectF circle(QPointF(0, 0), QSizeF(w, w));
        circle.moveCenter(QRectF(rect).center());
        path.addEllipse(circle);
    }
    else if(_roundType == PercentRadius)
    {
        path.addRoundedRect(rect,
                            qBound(0.0, 100.0, qreal(roundRadius.x())),
                            qBound(0.0, 100.0, qreal(roundRadius.y())), Qt::RelativeSize);

    }
    else if(_roundType == AbsoluteRadius)
    {
        path.addRoundedRect(rect, roundRadius.x(), roundRadius.y());
    }
    return path;
}
