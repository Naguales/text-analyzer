#ifndef COMMONDATA_H
#define COMMONDATA_H

#include <QString>
#include <QColor>
#include <QDir>
#include <QMap>
#include <unordered_map>

typedef QMap<QString, ulong> WordsMap;
typedef QVector<QPair<QString, quint64>> WordsVector;
typedef std::vector<std::pair<quint64, std::pair<QString, quint64>>> DictionaryVector;                  //!< Dictionary vector of letter combinations as keys in hash representation.
                                                                                                        //!< Value is combination of string representation of letter combination and its count.
typedef std::unordered_map<quint64, std::pair<QString, quint64>> DictionaryMap;                         //!< Dictionary map of letter combinations as keys in hash representation.
                                                                                                        //!< Value is combination of string representation of letter combination and its count.
typedef std::unordered_map<quint64, std::unordered_map<quint64, quint64>> WordsLetterCombinationsMap;   //!< Contains letter combinations and their count (value) for words (key) in hash representation.
typedef std::unordered_map<quint64, quint64> LetterCombinationsMap;                                     //!< Contains letter combinations (key) and their count (value) in hash representation.

namespace CommonData {

    inline constexpr int TopLetterCombinationsCount = 10;
    inline constexpr int MinLetterCombinationLength = 4;

    inline constexpr int MaxLetterCombinationsTableColumns = 3;

    inline constexpr int ButtonPaddingX = 5;
    inline constexpr int ButtonPaddingY = 5;
    inline constexpr int ButtonBorderWidth = 1;
    inline constexpr int ButtonBorderRadius = 3;

    // The horizontal resolution of the device in dots per inch corresponding to the 100% DPI setting.
    inline constexpr double LogicalDpiRefValue = 96.0;

    enum EFileReadingStatus
    {
        efrsFileOpenError = 0,
        efrsFileProcessingFinished,
        efrsFileProcessingInterrupted
    };

    enum ELetterCombinationsTableColumn {
        elctcColor = 0,
        elctcLetterCombination,
        elctcPercentage,
        elctcMaxColumn
    };

    enum class ETextAnalyzingStatus : unsigned char
    {
        etasIdle = 0,
        etasTextAnalyzingInProgress
    };

    static QColor DominantColor()
    {
        static QColor dominantColor(45, 127, 249);
        return dominantColor;
    }

    static QColor HoverColor()
    {
        static QColor hoverColor(16, 80, 200);
        return hoverColor;
    }

    static QColor PressedColor()
    {
        static QColor pressedColor(20, 100, 230);
        return pressedColor;
    }

    static QColor DisabledColor()
    {
        static QColor disabledColor(230, 230, 230);
        return disabledColor;
    }

    static QColor DisabledTextColor()
    {
        static QColor disabledTextColor(160, 160, 160);
        return disabledTextColor;
    }

    static QColor DisabledBorderColor()
    {
        static QColor disabledBorderColor(203, 203, 203);
        return disabledBorderColor;
    }

    static QColor GlowColor()
    {
        static QColor glowColor(16, 80, 200, 80);
        return glowColor;
    }

    static QString SignificantButtonStyleSheet(double scaleFactor = 1.0)
    {
        QColor dominantColor = DominantColor();
        const QString significantButtonStyleSheet = QString("QPushButton { font-weight: bold; margin: 0px; padding: %1 %2px; color: white; background-color: rgb(%3, %4, %5); border: %6px solid; border-radius: %7px; border-color: rgb(%8, %9, %10); }")
                .arg(QString::number(qRound(ButtonPaddingX * scaleFactor)),
                     QString::number(qRound(ButtonPaddingY * scaleFactor)),
                     QString::number(dominantColor.red()),
                     QString::number(dominantColor.green()),
                     QString::number(dominantColor.blue()),
                     QString::number(qRound(ButtonBorderWidth * scaleFactor)),
                     QString::number(qRound(ButtonBorderRadius * scaleFactor)),
                     QString::number(dominantColor.red()),
                     QString::number(dominantColor.green()),
                     QString::number(dominantColor.blue()));
        return significantButtonStyleSheet;
    }

    static QString SignificantButtonWithHoverStyleSheet(double scaleFactor = 1.0)
    {
        QString significantButtonWithHoverStyleSheet = SignificantButtonStyleSheet(scaleFactor);
        const QColor hoverColor = HoverColor(),
                pressedColor = PressedColor(),
                disabledColor = DisabledColor(),
                disabledTextColor = DisabledTextColor(),
                disabledBorderColor = DisabledBorderColor();
        significantButtonWithHoverStyleSheet.append(QString("QPushButton:hover { background-color: rgb(%1, %2, %3); }")
                                          .arg(QString::number(hoverColor.red()), QString::number(hoverColor.green()), QString::number(hoverColor.blue())));
        significantButtonWithHoverStyleSheet.append(QString("QPushButton:pressed { background-color: rgb(%1, %2, %3); }")
                                          .arg(QString::number(pressedColor.red()), QString::number(pressedColor.green()), QString::number(pressedColor.blue())));
        significantButtonWithHoverStyleSheet.append(QString("QPushButton:pressed:hover { background-color: rgb(%1, %2, %3); }")
                                          .arg(QString::number(pressedColor.red()), QString::number(pressedColor.green()), QString::number(pressedColor.blue())));
        significantButtonWithHoverStyleSheet.append(QString("QPushButton:disabled { background-color: rgb(%1, %2, %3); border-color: rgb(%4, %5, %6); color: rgb(%7, %8, %9); }")
                                          .arg(QString::number(disabledColor.red()), QString::number(disabledColor.green()), QString::number(disabledColor.blue()),
                                               QString::number(disabledBorderColor.red()), QString::number(disabledBorderColor.green()), QString::number(disabledBorderColor.blue()),
                                               QString::number(disabledTextColor.red()), QString::number(disabledTextColor.green()), QString::number(disabledTextColor.blue())));
        return significantButtonWithHoverStyleSheet;
    }

    static QString ToolButtonStyleSheet(double scaleFactor = 1.0)
    {
        QString toolButtonStyleSheet = QString("QToolButton { margin: 0px; padding: %1 %2 px; border: none; }")
                .arg(QString::number(qRound(ButtonPaddingX * scaleFactor)),
                     QString::number(qRound(ButtonPaddingY * scaleFactor)));
        return toolButtonStyleSheet;
    }

    static QString NormalizePath(const QString& path)
    {
        // QString::trimmed() returns a string that has whitespace removed from the start and the end.
        // QDir::cleanPath() returns path with directory separators normalized (converted to "/") and redundant ones removed,
        // and "."s and ".."s resolved (as far as possible).
        QString normalizedPath(QDir::cleanPath(path.trimmed()));

        QDir dir(normalizedPath);
        dir.makeAbsolute();

        return QDir::toNativeSeparators(dir.path());
    }

    template <typename T>
    class asKeyValueRange
    {
    public:
        asKeyValueRange(T& data) : m_data{data} {}

        auto begin() { return m_data.keyValueBegin(); }
        auto end() { return m_data.keyValueEnd(); }

    private:
        T& m_data;
    };

    static void RemovePunctuationSymbols(QString& inout_str)
    {
        auto it = std::remove_if(std::begin(inout_str), std::end(inout_str), [](const QChar& c) {
            return c.isPunct();
        });
        inout_str.chop(std::distance(it, std::end(inout_str)));
    }

} // namespace CommonData

#endif // COMMONDATA_H
