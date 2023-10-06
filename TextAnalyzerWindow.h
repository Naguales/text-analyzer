#ifndef TEXTANALYZERWINDOW_H
#define TEXTANALYZERWINDOW_H

#include "CommonData.h"
#include "Workers/FileReader.h"
#include "Workers/TextAnalyzer.h"
#include "Widgets/GlowedButton.h"
#include "Table/LetterCombinationsModel.h"
#include "Table/LetterCombinationsTableView.h"

#include <QMainWindow>
#include <QLabel>
#include <QMovie>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QChartView>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QVBoxLayout>

QT_CHARTS_USE_NAMESPACE

class CTextAnalyzerWindow : public QMainWindow
{
    Q_OBJECT

public:
    CTextAnalyzerWindow(QWidget* parent = nullptr);
    ~CTextAnalyzerWindow();

public slots:
    void updateTopLetterCombinations(const WordsVector&);
    void updateWordsProcessedCount(quint64);
    void updateTotalLetterCombinationsCount(quint64);

    //! Status from the CommonData::EFileReadingStatus enumeration.
    void changeProcessStatus(int);

signals:
    void fileProcessingStarted(const QString& fileName);
    void fileProcessingFinished();
    void histogramUpdatingFinished();

protected:
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private slots:
    void browsePath();
    void finishTextAnalyzing(const WordsVector&);

private:
    void init();

    void createWidgets();
    void createMainLayout();
    void createConnections();

    void createTextAnalyzingMovie();
    void breakTextAnalyzingMovie();

    void startTextAnalyzing();
    void stopTextAnalyzing();

    void dropUIState();

    QWidget* m_mainWidget { nullptr };
    CFileReaderWorker* m_fileReaderWorker { nullptr };
    QThread* m_fileReaderWorkerThread { nullptr };
    CTextAnalyzerWorker* m_textAnalyzerWorker { nullptr };
    QThread* m_textAnalyzerWorkerThread { nullptr };

    QLabel* m_filePathLabel { nullptr };
    QLineEdit* m_filePathLineEdit { nullptr };
    CGlowedButton* m_browseFilePushButton { nullptr };
    QScopedPointer<QMovie> m_textAnalyzingMovie { nullptr };
    QLabel* m_textAnalyzingMovieLabel { nullptr };
    QToolButton* m_statusToolButton { nullptr };
    QLabel* m_statusLabel { nullptr };
    QLabel* m_totalWordsProcessedLabel { nullptr };
    QLabel* m_totalWordsProcessedCountLabel { nullptr };
    QFrame* m_statusVerticalLine { nullptr };
    QWidget* m_statusInfoWidget { nullptr };

    QChart* m_histogram { nullptr };
    QChartView* m_histogramView { nullptr };
    QBarSeries* m_barSeries { nullptr };
    QBarCategoryAxis* m_axisX { nullptr };
    QValueAxis* m_axisY { nullptr };

    CLetterCombinationsModel* m_letterCombinationsModel { nullptr };
    CLetterCombinationsTableView* m_letterCombinationsTableView { nullptr };

    QVBoxLayout* m_mainLayout { nullptr };

    QString m_defaultPath { ".\\..\\..\\..\\TextAnalyzer\\Data" };
    QString m_textAnalyzingInProgressText { QObject::tr("Text analysis") };
    quint64 m_totalLetterCombinationsCount { 0 };
    double m_maxLetterCombinationsPercentage { 0.0 };
    double m_scaleFactor { 1.0 };

    CommonData::ETextAnalyzingStatus m_textAnalyzingStatus { CommonData::ETextAnalyzingStatus::etasIdle };
};

#endif // TEXTANALYZERWINDOW_H
