#ifndef LETTERCOMBINATIONSTABLEVIEW_H
#define LETTERCOMBINATIONSTABLEVIEW_H

#include <QTableView>

class CLetterCombinationsTableView : public QTableView
{
    Q_OBJECT

public:
    explicit CLetterCombinationsTableView(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model) Q_DECL_OVERRIDE;

protected:
    void resizeEvent(QResizeEvent*) Q_DECL_OVERRIDE;
	void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;

private:
    void determineColumnWidthHints();

    QVector<int> m_widthHintVec;
    int m_totalWidthHint { 0 };
    double m_scaleFactor { 1.0 };
};

#endif // LETTERCOMBINATIONSTABLEVIEW_H
