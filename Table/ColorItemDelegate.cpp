#include "ColorItemDelegate.h"
#include "CommonData.h"

#include <QPainter>

CColorItemDelegate::CColorItemDelegate(QObject* parent, double in_scaleFactor)
    : QItemDelegate(parent),
      m_scaleFactor(in_scaleFactor)
{}

void CColorItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int row = index.row();
    if (row % 2) {
        QColor backgroundColor;
        backgroundColor.setNamedColor("#f0f0f0");
        painter->fillRect(option.rect, backgroundColor);
    }

    QRect r = option.rect;  // Get the rect of the cell.

    painter->save();
    if (option.state & QStyle::State_Selected) {
        QLinearGradient gradient(QPointF(r.left(), r.top()), QPointF(r.left(), r.bottom()));
        gradient.setColorAt(0, QColor(9, 155, 221));
        gradient.setColorAt(0.5, QColor(45, 127, 249));
        gradient.setColorAt(0.501, QColor(45, 127, 249));
        gradient.setColorAt(1, QColor(16, 80, 200));
        QBrush brush(gradient);
        painter->fillRect(option.rect, brush);
    }
    painter->restore();
    
    int x, y, w, h;
    w = qRound(r.height() * 0.8);
    h = qRound(r.height() * 0.8);
    x = r.left() + qRound(2 * m_scaleFactor);
    y = r.top() + static_cast<int>(std::floor((r.height() - h) * 0.5));

    QRect colorRect(x, y, w, h);

    const QString sColor = index.data().canConvert(QMetaType::QString) ? index.data().toString() : "";
    QColor color;
    color.setNamedColor(sColor);
    QPainter innerPainter;
    painter->fillRect(colorRect, QBrush(color));

    QColor borderColor;
    borderColor.setNamedColor("#707070");
    painter->setPen(borderColor);
    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::white);
    }
    painter->drawRect(colorRect);
}
