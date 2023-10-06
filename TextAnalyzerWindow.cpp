#include "TextAnalyzerWindow.h"

#include <QBarSet>
#include <QLegend>
#include <QThread>
#include <QFile>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScreen>
#include <QLocale>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>
#include <QCoreApplication>

#include <algorithm>

CTextAnalyzerWindow::CTextAnalyzerWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_scaleFactor = logicalDpiX() / CommonData::LogicalDpiRefValue;
    init();
}

CTextAnalyzerWindow::~CTextAnalyzerWindow()
{
    m_textAnalyzingMovie.reset(nullptr);
}

void CTextAnalyzerWindow::updateTopLetterCombinations(const WordsVector& in_topLetterCombinations)
{
    auto topLetterCombinations = in_topLetterCombinations;
    std::sort(std::begin(topLetterCombinations), std::end(topLetterCombinations), [](const auto& lhs, const auto& rhs) {
        return lhs.second > rhs.second;
    });
    auto barSets = m_barSeries->barSets();
    if (barSets.empty() || in_topLetterCombinations.empty())
        return;

    int barCounter = 0;
    double maxLetterCombinationsPercentage = 0;
    QVector<QPair<QString, double>> vTableData;
    for (const auto& word : topLetterCombinations) {
        if (barCounter >= barSets.size()) {
            break;
        }
        double percentage = m_totalLetterCombinationsCount ? (word.second * 100.0) / m_totalLetterCombinationsCount : static_cast<double>(word.second);
        if (barSets[barCounter]) {
            barSets[barCounter]->setLabel(word.first);
            barSets[barCounter]->replace(0, percentage);
        }
        if (percentage > maxLetterCombinationsPercentage) {
            maxLetterCombinationsPercentage = percentage;
        }
        vTableData.push_back({ word.first, percentage });
        ++barCounter;
    }
    m_maxLetterCombinationsPercentage = maxLetterCombinationsPercentage;
    m_axisY->setRange(0, maxLetterCombinationsPercentage);
    m_letterCombinationsModel->refresh(vTableData);
}

void CTextAnalyzerWindow::updateTotalLetterCombinationsCount(quint64 in_totalLetterCombinationsCount)
{
    m_totalLetterCombinationsCount = in_totalLetterCombinationsCount;
}

void CTextAnalyzerWindow::updateWordsProcessedCount(quint64 in_wordsProcessedCount)
{
    m_totalWordsProcessedCountLabel->setText(QLocale::system().toString(in_wordsProcessedCount));
}

void CTextAnalyzerWindow::changeProcessStatus(int in_status)
{
    if (in_status == CommonData::efrsFileOpenError) {
        stopTextAnalyzing();
        m_statusToolButton->setVisible(true);
        m_statusToolButton->setIcon(QIcon(":/Resources/Warning"));
        m_statusLabel->setText(QObject::tr("File not found"));
    } else if (in_status == CommonData::efrsFileProcessingFinished) {
        // Notify the Text Analyzer Worker to finish processing.
        emit fileProcessingFinished();
    } else if (in_status == CommonData::efrsFileProcessingInterrupted) {
        if (m_fileReaderWorkerThread && m_fileReaderWorkerThread->isRunning()) {
            m_fileReaderWorkerThread->quit();
            m_fileReaderWorkerThread->wait();
        }
    }
}

void CTextAnalyzerWindow::closeEvent(QCloseEvent* event)
{
    if (m_fileReaderWorkerThread && m_fileReaderWorkerThread->isRunning()) {
        if (m_textAnalyzingStatus == CommonData::ETextAnalyzingStatus::etasTextAnalyzingInProgress && m_fileReaderWorker) {
            m_fileReaderWorker->stopProcessing();
        } else {
            m_fileReaderWorkerThread->quit();
            m_fileReaderWorkerThread->wait();
        }
    }
    if (m_textAnalyzerWorkerThread && m_textAnalyzerWorkerThread->isRunning()) {
        m_textAnalyzerWorkerThread->quit();
        m_textAnalyzerWorkerThread->wait();
    }
    QMainWindow::closeEvent(event);
}

void CTextAnalyzerWindow::browsePath()
{
    QDir defaultPathDir(m_defaultPath);
    QString path = defaultPathDir.isRelative() ? QCoreApplication::applicationDirPath() + "\\" + m_defaultPath : m_defaultPath,
            normalizedPath(CommonData::NormalizePath(path)),
            selectedFilePath;

    // Create a modal file dialog.
    // Returns an existing file selected by the user. If the user presses Cancel, it returns a null string.
    selectedFilePath = QFileDialog::getOpenFileName(this, QObject::tr("Select File to Analyze"), normalizedPath, QObject::tr("Text Files (*.txt)"));
    if (!selectedFilePath.isEmpty()) {
        selectedFilePath = QDir::toNativeSeparators(selectedFilePath);
        QFileInfo selectedFileInfo(selectedFilePath);
        if (selectedFileInfo.exists()) {
            // Start text analyzing.
            m_defaultPath = selectedFileInfo.dir().path();
            m_filePathLineEdit->setText(selectedFilePath);
            startTextAnalyzing();
        }
    } else {
        dropUIState();
    }
}

void CTextAnalyzerWindow::finishTextAnalyzing(const WordsVector& in_topWords)
{
    updateTopLetterCombinations(in_topWords);
    stopTextAnalyzing();
    m_statusToolButton->setVisible(true);
    m_statusToolButton->setIcon(QIcon(":/Resources/Success"));
    m_statusLabel->setText(QObject::tr("Text analysis finished"));
    emit histogramUpdatingFinished();
}

void CTextAnalyzerWindow::init()
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Window, Qt::white);
    QApplication::setPalette(palette);
    setWindowIcon(QIcon(":/Resources/Histogram"));
    setWindowTitle(QObject::tr("Text Analyzer"));

    m_fileReaderWorkerThread = new QThread(this);
    m_fileReaderWorker = new CFileReaderWorker();
    m_fileReaderWorker->moveToThread(m_fileReaderWorkerThread);

    m_textAnalyzerWorkerThread = new QThread(this);
    m_textAnalyzerWorker = new CTextAnalyzerWorker();
    m_textAnalyzerWorker->moveToThread(m_textAnalyzerWorkerThread);

    createWidgets();
    createMainLayout();
    createConnections();
    setCentralWidget(m_mainWidget);

    setMinimumSize(QSize(qRound(1200 * m_scaleFactor), qRound(742 * m_scaleFactor)));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // Set minimum size for the Scanner Main Window.
    const QSize preferredMinimumSize = QSize(qRound(1200 * m_scaleFactor), qRound(742 * m_scaleFactor));
    QDesktopWidget* pDesktop = QApplication::desktop();
    auto iWindowScreenNumber = pDesktop->screenNumber(this);
    auto screens = QGuiApplication::screens();
    const QRect availableGeometryRect = iWindowScreenNumber >= 0 && iWindowScreenNumber < screens.size() && screens[iWindowScreenNumber] ? screens[iWindowScreenNumber]->availableGeometry() : QRect();
    const QSize screenAvailableSize(availableGeometryRect.width(), availableGeometryRect.height());
    if (!availableGeometryRect.isNull() && (preferredMinimumSize.width() > screenAvailableSize.width() || qRound(preferredMinimumSize.height() * 1.07) > screenAvailableSize.height())) {
        this->setMinimumSize(screenAvailableSize);
        this->showMaximized();
    } else {
        this->setMinimumSize(preferredMinimumSize);
        this->resize(preferredMinimumSize);
    }

    m_fileReaderWorkerThread->start();
    m_textAnalyzerWorkerThread->start();
}

void CTextAnalyzerWindow::createWidgets()
{
    auto createActionPushButton = [&](const QString& buttonText, const QString& styleSheet){
        auto actionPushButton = new CGlowedButton(buttonText, CommonData::GlowColor(), this);
        actionPushButton->setStyleSheet(styleSheet);
        actionPushButton->setFocusPolicy(Qt::NoFocus);
        const int iActionPushButtonWidth = qRound(120 * m_scaleFactor);
        actionPushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        actionPushButton->setFixedWidth(iActionPushButtonWidth);
        return actionPushButton;
    };

    auto createStatusToolButton = [&](QWidget* parent, const QString& iconPath = QStringLiteral(":/Resources/Success")){
        auto statusToolButton = new QToolButton(parent);
        statusToolButton->setIcon(QIcon(iconPath));
        statusToolButton->setStyleSheet(CommonData::ToolButtonStyleSheet(m_scaleFactor));
        const int iStatusToolButtonFixedSize = qRound(35 * m_scaleFactor);
        statusToolButton->setIconSize(QSize(iStatusToolButtonFixedSize, iStatusToolButtonFixedSize));
        statusToolButton->setFocusPolicy(Qt::NoFocus);
        return statusToolButton;
    };

    auto createTextLabel = [&](const QString& labelText, QWidget* parent) {
        auto textLabel = new QLabel(labelText, parent);
        textLabel->setStyleSheet(QString("QLabel { color: rgb(59, 59, 59); margin: %1px; }").arg(QString::number(qRound(7 * m_scaleFactor))));
        return textLabel;
    };

    auto createCountLabel = [&](const QString& labelCount, QWidget* parent) {
        auto countLabel = new QLabel(labelCount, parent);
        countLabel->setStyleSheet(QString("QLabel { font-weight: bold; margin: %1px;}").arg(QString::number(qRound(7 * m_scaleFactor))));
        return countLabel;
    };

    auto createVerticalLine = [&](QWidget* parent){
        auto verticalLine = new QFrame(parent);
        verticalLine->setFrameStyle(QFrame::VLine | QFrame::Plain);
        verticalLine->setLineWidth(qRound(1 * m_scaleFactor));
        verticalLine->setStyleSheet("QFrame { color: rgb(220, 220, 220); } ");
        return verticalLine;
    };

    m_mainWidget = new QWidget(this);
    m_filePathLineEdit = new QLineEdit(this);
    m_filePathLineEdit->setEnabled(false);
    m_filePathLineEdit->setPlaceholderText(QObject::tr("Select File to Analyze"));
    m_filePathLineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_filePathLabel = createTextLabel(QObject::tr("&File"), this);
    m_filePathLabel->setBuddy(m_filePathLineEdit);

    m_browseFilePushButton = createActionPushButton(QObject::tr("Browse..."), CommonData::SignificantButtonWithHoverStyleSheet(m_scaleFactor));

    m_statusInfoWidget = new QWidget(this);

    m_statusLabel = createTextLabel("", m_statusInfoWidget);
    QFont statusLabelFont = m_statusLabel->font();
    statusLabelFont.setPixelSize(qRound(statusLabelFont.pixelSize() * 1.2));
    m_statusLabel->setFont(statusLabelFont);

    m_totalWordsProcessedLabel = createTextLabel(QObject::tr("Words processed"), m_statusInfoWidget);
    m_totalWordsProcessedCountLabel = createCountLabel(QString("%1").arg(0), m_statusInfoWidget);
    m_statusVerticalLine = createVerticalLine(m_statusInfoWidget);

    m_statusToolButton = createStatusToolButton(m_statusInfoWidget);
    m_textAnalyzingMovieLabel = new QLabel(m_statusInfoWidget);
    m_statusInfoWidget->setVisible(false);

    // Initialize the histogram.
    m_histogram = new QChart();
    m_histogram->setTitle(QObject::tr("Top letter combinations"));
    m_histogram->setAnimationOptions(QChart::NoAnimation);
    m_histogram->setLocalizeNumbers(true);

    m_barSeries = new QBarSeries();
    QMap<int, QColor> barSetColors {
        { 0, QColor(109, 95, 213) },
        { 1, QColor(7, 116, 166) },
        { 2, QColor(32, 159, 223) },
        { 3, QColor(94, 200, 248) },
        { 4, QColor(153, 202, 83) },
        { 5, QColor(181, 230, 29) },
        { 6, QColor(246, 166, 37) },
        { 7, QColor(255, 201, 14) },
        { 8, QColor(255, 174, 201) },
        { 9, QColor(242, 81, 82) }
    };
    QVector<QColor> letterCombinationsColors(CommonData::TopLetterCombinationsCount);
    for (int i = 0; i < CommonData::TopLetterCombinationsCount; ++i) {
        auto barSet = new QBarSet("");
        *barSet << 0;
        if (barSetColors.contains(i)) {
            barSet->setColor(barSetColors[i]);
            letterCombinationsColors[i] = barSetColors[i];
        }
        letterCombinationsColors[i] = barSet->color();
        m_barSeries->append(barSet);
    }
    m_histogram->addSeries(m_barSeries);

    QStringList categories { QObject::tr("Top letter combinations") };
    m_axisX = new QBarCategoryAxis();
    m_axisX->append(categories);
    m_histogram->addAxis(m_axisX, Qt::AlignBottom);
    m_barSeries->attachAxis(m_axisX);

    m_axisY = new QValueAxis();
    m_axisY->setTickCount(11);
    m_axisY->setRange(0, 1.0);
    m_axisY->setLabelFormat("%.3f %");
    m_axisY->setTitleVisible(true);
    m_axisY->setTitleText(QObject::tr("Percentage"));
    m_histogram->addAxis(m_axisY, Qt::AlignLeft);
    m_barSeries->attachAxis(m_axisY);

    m_histogram->legend()->setVisible(true);
    m_histogram->legend()->setAlignment(Qt::AlignBottom);

    m_histogramView = new QChartView(m_histogram, this);
    m_histogramView->setRenderHint(QPainter::Antialiasing);

    m_letterCombinationsModel = new CLetterCombinationsModel(this);
    m_letterCombinationsModel->setColors(letterCombinationsColors);
    m_letterCombinationsTableView = new CLetterCombinationsTableView(this);
    m_letterCombinationsTableView->setModel(m_letterCombinationsModel);
    m_letterCombinationsTableView->setFixedSize(qRound(350 * m_scaleFactor), qRound(350 * m_scaleFactor));
}

void CTextAnalyzerWindow::createMainLayout()
{
    m_mainLayout = new QVBoxLayout(this);

    auto filePathHBoxLayout = new QHBoxLayout;
    filePathHBoxLayout->addWidget(m_filePathLabel);
    filePathHBoxLayout->addWidget(m_filePathLineEdit);
    filePathHBoxLayout->addWidget(m_browseFilePushButton);
    filePathHBoxLayout->setSpacing(qRound(16 * m_scaleFactor));

    auto statusHBoxLayout = new QHBoxLayout;
    statusHBoxLayout->addWidget(m_textAnalyzingMovieLabel);
    statusHBoxLayout->addWidget(m_statusToolButton);
    statusHBoxLayout->addWidget(m_statusLabel);

    auto processingMetricsGridLyout = new QGridLayout;
    processingMetricsGridLyout->addWidget(m_totalWordsProcessedLabel, 0, 0, Qt::AlignLeft);
    processingMetricsGridLyout->addWidget(m_totalWordsProcessedCountLabel, 0, 1, Qt::AlignRight);

    auto statusInfoGridLayout = new QGridLayout;
    statusInfoGridLayout->addLayout(statusHBoxLayout, 0, 0, Qt::AlignRight);
    statusInfoGridLayout->addWidget(m_statusVerticalLine, 0, 1);
    statusInfoGridLayout->addLayout(processingMetricsGridLyout, 0, 2, Qt::AlignLeft);
    m_statusInfoWidget->setLayout(statusInfoGridLayout);

    auto dataRepresentationHBoxLayout = new QHBoxLayout;
    dataRepresentationHBoxLayout->addWidget(m_histogramView);
    dataRepresentationHBoxLayout->addWidget(m_letterCombinationsTableView);

    m_mainLayout->addLayout(filePathHBoxLayout);
    m_mainLayout->addWidget(m_statusInfoWidget);
    m_mainLayout->addLayout(dataRepresentationHBoxLayout);
    m_mainLayout->setSpacing(qRound(16 * m_scaleFactor));
    m_mainLayout->setMargin(qRound(16 * m_scaleFactor));
    m_mainWidget->setLayout(m_mainLayout);
}

void CTextAnalyzerWindow::createConnections()
{
    QObject::connect(m_fileReaderWorkerThread, &QThread::finished, m_fileReaderWorker, &CFileReaderWorker::deleteLater);
    QObject::connect(m_fileReaderWorkerThread, &QThread::finished, m_fileReaderWorkerThread, &QThread::deleteLater);
    QObject::connect(this, &CTextAnalyzerWindow::fileProcessingStarted, m_fileReaderWorker, &CFileReaderWorker::process);
    QObject::connect(m_fileReaderWorker, &CFileReaderWorker::statusChanged, this, &CTextAnalyzerWindow::changeProcessStatus);

    QObject::connect(m_textAnalyzerWorkerThread, &QThread::finished, m_textAnalyzerWorker, &CTextAnalyzerWorker::deleteLater);
    QObject::connect(m_textAnalyzerWorkerThread, &QThread::finished, m_textAnalyzerWorkerThread, &QThread::deleteLater);
    QObject::connect(m_fileReaderWorker, &CFileReaderWorker::chunkProcessed, m_textAnalyzerWorker, &CTextAnalyzerWorker::process);
    QObject::connect(m_textAnalyzerWorker, &CTextAnalyzerWorker::topLetterCombinationsUpdated, this, &CTextAnalyzerWindow::updateTopLetterCombinations);
    QObject::connect(m_textAnalyzerWorker, &CTextAnalyzerWorker::totalLetterCombinationsCountUpdated, this, &CTextAnalyzerWindow::updateTotalLetterCombinationsCount);
    QObject::connect(m_textAnalyzerWorker, &CTextAnalyzerWorker::wordsProcessedCountUpdated, this, &CTextAnalyzerWindow::updateWordsProcessedCount);
    // Inform text analyzer to finish text processing.
    QObject::connect(this, &CTextAnalyzerWindow::fileProcessingFinished, m_textAnalyzerWorker, &CTextAnalyzerWorker::finishTextAnalyzing);
    // When text analyzer has processed rest data, update UI.
    QObject::connect(m_textAnalyzerWorker, &CTextAnalyzerWorker::textAnalyzingFinished, this, &CTextAnalyzerWindow::finishTextAnalyzing);
    // Drop accumulated data during file processing in the Text Analyzer Worker after finish UI updating.
    QObject::connect(this, &CTextAnalyzerWindow::histogramUpdatingFinished, m_textAnalyzerWorker, &CTextAnalyzerWorker::finishProcessing);

    QObject::connect(m_browseFilePushButton, &CGlowedButton::clicked, this, &CTextAnalyzerWindow::browsePath);
}

void CTextAnalyzerWindow::createTextAnalyzingMovie()
{
    m_textAnalyzingMovie.reset(new QMovie(":/Resources/Throbber"));
    m_textAnalyzingMovie->setScaledSize(QSize(qRound(50 * m_scaleFactor), qRound(50 * m_scaleFactor)));
    if (m_textAnalyzingMovieLabel) {
        m_textAnalyzingMovieLabel->setMovie(m_textAnalyzingMovie.data());
    }
}

void CTextAnalyzerWindow::breakTextAnalyzingMovie()
{
    m_textAnalyzingMovie.reset(nullptr);
    if (m_textAnalyzingMovieLabel) {
        m_textAnalyzingMovieLabel->setMovie(nullptr);
    }
}

void CTextAnalyzerWindow::startTextAnalyzing()
{
    createTextAnalyzingMovie();
    m_browseFilePushButton->setDisabled(true);
    m_statusInfoWidget->setVisible(true);
    m_textAnalyzingMovieLabel->setVisible(true);
    m_statusToolButton->setVisible(false);
    m_totalWordsProcessedCountLabel->setText(QString("%1").arg(0));
    m_axisY->setRange(0, 1.0);
    m_maxLetterCombinationsPercentage = 0.0;
    auto barSets = m_barSeries->barSets();
    for (auto& barSet : barSets) {
        if (barSet) {
            barSet->setLabel("");
            barSet->insert(0, 0);
        }
    }
    m_statusLabel->setText(m_textAnalyzingInProgressText);
    m_textAnalyzingStatus = CommonData::ETextAnalyzingStatus::etasTextAnalyzingInProgress;
    m_textAnalyzingMovie->start();
    emit fileProcessingStarted(m_filePathLineEdit->text());
}

void CTextAnalyzerWindow::stopTextAnalyzing()
{
    if (m_textAnalyzingMovie) {
        m_textAnalyzingMovie->stop();
    }
    breakTextAnalyzingMovie();
    m_textAnalyzingMovieLabel->setVisible(false);
    m_browseFilePushButton->setEnabled(true);
    m_totalLetterCombinationsCount = 0;
    m_textAnalyzingStatus = CommonData::ETextAnalyzingStatus::etasIdle;
}

void CTextAnalyzerWindow::dropUIState()
{
    stopTextAnalyzing();
    m_statusInfoWidget->setVisible(false);
    m_filePathLineEdit->clear();
    m_totalWordsProcessedCountLabel->setText(QString("%1").arg(0));
    m_axisY->setRange(0, 1.0);
    m_maxLetterCombinationsPercentage = 0.0;
    auto barSets = m_barSeries->barSets();
    for (auto& barSet : barSets) {
        if (barSet) {
            barSet->setLabel("");
            barSet->insert(0, 0);
        }
    }
    QVector<QPair<QString, double>> vTableData(CommonData::TopLetterCombinationsCount);
    std::fill(std::begin(vTableData), std::end(vTableData), QPair{ "", 0.0 });
    m_letterCombinationsModel->refresh(vTableData);
}
