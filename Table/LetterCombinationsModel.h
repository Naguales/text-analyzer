#ifndef LETTERCOMBINATIONSMODEL_H
#define LETTERCOMBINATIONSMODEL_H

#include "CommonData.h"

#include <QAbstractTableModel>

//! The CLetterCombinationsModel class represents reports data for QTableView.
class CLetterCombinationsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    CLetterCombinationsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) Q_DECL_OVERRIDE;

    void setColors(const QVector<QColor>&);
    void refresh(const QVector<QPair<QString, double>>&);

private:
    QVector<QColor> m_colors;
    QVector<QPair<QString, double>> m_modelData;
};

#endif // LETTERCOMBINATIONSMODEL_H
