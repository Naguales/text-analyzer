#include "LetterCombinationsTableView.h"
#include "ColorItemDelegate.h"
#include "CommonData.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

CLetterCombinationsTableView::CLetterCombinationsTableView(QWidget* parent)
    : QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // Enable standard logic of multiple rows selection in a table with the Shift, Ctrl modifiers and by the mouse.
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFocusPolicy(Qt::StrongFocus);
    setSortingEnabled(false);
    setAutoScroll(true);
    setShowGrid(false);
    setEditTriggers(QAbstractItemView::AllEditTriggers);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setMinimumSectionSize(1);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->hide();
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    verticalScrollBar()->setSingleStep(verticalHeader()->defaultSectionSize());
    QString styleSheet = QString("QTableView { outline: 0; border: %1px solid #e2e2e2; } QTableView::item:selected{ background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgb(9, 155, 221), stop : 0.5 rgb(45, 127, 249), stop : 0.501 rgb(45, 127, 249), stop : 1 rgb(16, 80, 200)); color: palette(highlightedText); };").arg(qRound(1 * m_scaleFactor));
    setStyleSheet(styleSheet);
}

void CLetterCombinationsTableView::setModel(QAbstractItemModel* model)
{
	QTableView::setModel(model);

    setItemDelegateForColumn(CommonData::elctcColor, new CColorItemDelegate(this, m_scaleFactor));

    horizontalHeader()->setSectionResizeMode(CommonData::elctcLetterCombination, QHeaderView::Interactive);
    horizontalHeader()->setSectionResizeMode(CommonData::elctcPercentage, QHeaderView::Fixed);
}

void CLetterCombinationsTableView::resizeEvent(QResizeEvent* event)
{
    const int columnCount = CommonData::elctcMaxColumn,
        tableViewWidth = event->size().width();
    QTableView::resizeEvent(event);

    determineColumnWidthHints();
    for (int i = 0; i < columnCount; i++) {
        horizontalHeader()->resizeSection(i, m_widthHintVec[i]);
    }
    if (m_totalWidthHint < tableViewWidth && m_widthHintVec.size() > CommonData::elctcPercentage) {
        const int addition = tableViewWidth - m_totalWidthHint;
        horizontalHeader()->resizeSection(CommonData::elctcLetterCombination, m_widthHintVec[CommonData::elctcLetterCombination] + addition);
    }
}

void CLetterCombinationsTableView::keyPressEvent(QKeyEvent* event)
{
	// Override processing of Ctrl+C.
	if (event->matches(QKeySequence::Copy)) {
		// Copy selected rows contents to the Clipboard with the Tab delimiter between cells and the New line delimiter between rows
		// to have the possibility to paste the selected contents in MS Excel.
		QString text;
		QStringList rowContents;
		int currentRowIndex = 0, cellCounter = 0;
		QModelIndexList cellIndexes = selectedIndexes();
		std::sort(cellIndexes.begin(), cellIndexes.end());	// Necessary, otherwise cells are in column order.

		foreach (const auto& cellIndex, cellIndexes) {
			cellCounter++;
			// Skip first column with icons because its cells have not text representations.
			if (!cellIndex.column()) {
				continue;
			}

			auto cellContents = cellIndex.data().toString();
			cellContents = cellContents.simplified();
			if (cellIndex.row() != currentRowIndex || cellIndexes.size() == cellCounter) {
				if (cellIndexes.size() == cellCounter) {
					rowContents << cellContents;
				}
				if (!rowContents.isEmpty()) {
					text += rowContents.join(QStringLiteral("\t")) + QStringLiteral("\n");
					rowContents.clear();
				}
			}
			currentRowIndex = cellIndex.row();
			rowContents << cellContents;
		}

		QApplication::clipboard()->setText(text);
		return;
	}
	return QTableView::keyPressEvent(event);
}

void CLetterCombinationsTableView::determineColumnWidthHints()
{
    int maxWidthHint = 0;
    const int columnCount = CommonData::elctcMaxColumn,
        frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth),
        headerMargin = style()->pixelMetric(QStyle::PM_HeaderMargin);

    m_widthHintVec.clear();
    m_widthHintVec.resize(columnCount);

    QString header;
    for (int i = 0; i < columnCount; ++i) {
        header = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        if (i == CommonData::elctcColor) {
            maxWidthHint = qRound(fontMetrics().horizontalAdvance(QStringLiteral("Color")) * 1.2);
        } else if (i == CommonData::elctcLetterCombination) {
            maxWidthHint = qRound(fontMetrics().horizontalAdvance(header) * 1.2);
        } else if (i == CommonData::elctcPercentage) {
            maxWidthHint = qRound(fontMetrics().horizontalAdvance(header) * 1.1);
        }
        m_widthHintVec[i] = maxWidthHint + headerMargin * 2 + frameWidth * 2;
    }

    m_totalWidthHint = 0;
    foreach (auto item, m_widthHintVec) {
        m_totalWidthHint += item;
    }
}
