#ifndef TEXTANALYZER_H
#define TEXTANALYZER_H

#include "CommonData.h"

#include <QObject>
#include <QVector>
#include <QMap>

class CTextAnalyzerWorker : public QObject
{
    Q_OBJECT

public:
    CTextAnalyzerWorker();
    ~CTextAnalyzerWorker();

public slots:
    //! Analyze the given words, obtains their letter combinations and counts. Add these data into the storage and calculates the most common words' letter combinations.
    void process(const WordsMap& in_words);

    //! Finishes analyzing of remain words.
    void finishTextAnalyzing();

    //! Clears accumulated data.
    void finishProcessing();

signals:
    void topLetterCombinationsUpdated(const WordsVector&);
    void textAnalyzingFinished(const WordsVector&);
    void wordsProcessedCountUpdated(quint64);
    void totalLetterCombinationsCountUpdated(quint64);

private:
    //! Analyze the given words, obtains their letter combinations and counts. Add these data into the storage and calculates the most common words' letter combinations.
    //! @param in_force [in] - the parameter is used to finish text analyzing.
    void processImpl(const WordsMap& in_words, bool in_force = false);

    DictionaryMap findWordSubstrings(const QString& in_word);
    void addLetterCombinationToMap(const QString& in_word, DictionaryMap& inout_letterCombinations);

    DictionaryMap m_dictionary;                             //!< Store the dictionary in memory. For analyzing huge files should store the dictionary in file system.
    WordsLetterCombinationsMap m_wordsLetterCombinations;   //!< Contains letter combinations hashes and their count in words (in hash representation).
    WordsVector m_topLetterCombinations;                    //!< The current top letter combinations.
    quint64 m_topLetterCombinationsMinCount { 0 };          //!< The minimum letter combinations count in the most common words' letter combinations.
                                                            //!< It's used to check need to sort the dictionary and update letter combinations.
    quint64 m_wordsProcessed { 0 };
    quint64 m_totalLetterCombinationsCount { 0 };
};

#endif // TEXTANALYZER_H
