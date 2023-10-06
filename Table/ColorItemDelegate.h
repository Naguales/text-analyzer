#ifndef COLORITEMDELEGATE_H
#define COLORITEMDELEGATE_H

#include <QItemDelegate>

class CColorItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
    CColorItemDelegate(QObject* parent = nullptr, double in_scaleFactor = 1.0);
	~CColorItemDelegate() = default;

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    double m_scaleFactor { 1.0 };
};

#endif // COLORITEMDELEGATE_H
