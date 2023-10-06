#include "LetterCombinationsModel.h"

#include <QBrush>
#include <QLocale>

CLetterCombinationsModel::CLetterCombinationsModel(QObject* parent)
	: QAbstractTableModel(parent)
{}

int CLetterCombinationsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return m_modelData.size();
}

int CLetterCombinationsModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
    return CommonData::MaxLetterCombinationsTableColumns;
}

QVariant CLetterCombinationsModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}

    if (index.row() < 0 || index.row() >= m_modelData.size()) {
		return QVariant();
	}

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
        case CommonData::elctcColor:
            if (index.row() < m_colors.size()) {
                return m_colors.at(index.row()).name();
            }
        case CommonData::elctcLetterCombination:
            return m_modelData.at(index.row()).first;
        case CommonData::elctcPercentage:
            return QString("%1%").arg(QLocale::system().toString(m_modelData.at(index.row()).second, 'f', 3));
        }
	} else if (role == Qt::TextAlignmentRole) {
		switch (index.column()) {
        case CommonData::elctcColor:
        case CommonData::elctcLetterCombination:
        case CommonData::elctcPercentage:
			return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
		}
    } else if (role == Qt::BackgroundRole) {
        switch (index.column()) {
        case CommonData::elctcColor:
        case CommonData::elctcLetterCombination:
        case CommonData::elctcPercentage:
            const int row = index.row();
            if (row % 2) {
                QColor color("#f0f0f0");
                return QBrush(color);
            }
        }
    }
	return QVariant();
}

QVariant CLetterCombinationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const QString letterCombinationTitle = QObject::tr("Letter combination");
    static const QString percentageTitle = QObject::tr("Percentage");

	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	if (orientation == Qt::Horizontal) {
		switch (section) {
        case CommonData::elctcLetterCombination:
            return letterCombinationTitle;
        case CommonData::elctcPercentage:
            return percentageTitle;
		}
	}
	return QVariant();
}

Qt::ItemFlags CLetterCombinationsModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	switch (index.column()) {
    case CommonData::elctcColor:
    case CommonData::elctcLetterCombination:
    case CommonData::elctcPercentage:
		return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}
	return Qt::ItemIsEnabled;
}

bool CLetterCombinationsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == Qt::DisplayRole && index.row() < m_modelData.size() && index.column() < CommonData::MaxLetterCombinationsTableColumns) {
        auto& letterCombination = m_modelData[index.row()];
        bool ok = false;
        quint64 percentage = 0;
		switch (index.column()) {
        case CommonData::elctcColor:
            if (index.row() < m_colors.size() && QColor::isValidColor(value.toString())) {
                QColor color(value.toString());
                m_colors[index.row()] = color;
            }
			break;
        case CommonData::elctcLetterCombination:
            letterCombination.first = value.toString();
			break;
        case CommonData::elctcPercentage:
            percentage = value.toDouble(&ok);
            if (ok) {
                letterCombination.second = percentage;
            }
            break;
		default:
			Q_ASSERT(false);
		}

		emit dataChanged(index, index);
		return true;
	}
	return false;
}

void CLetterCombinationsModel::setColors(const QVector<QColor>& in_colors)
{
    beginResetModel();

    m_colors.clear();
    m_colors.shrink_to_fit();
    m_colors = in_colors;

    endResetModel();
}

void CLetterCombinationsModel::refresh(const QVector<QPair<QString, double>>& in_letterCombinations)
{
	beginResetModel();

	m_modelData.clear();
	m_modelData.shrink_to_fit();
    m_modelData = in_letterCombinations;

	endResetModel();
}
