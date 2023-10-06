#include "TextAnalyzer.h"

#include <QHash>

#include <algorithm>

CTextAnalyzerWorker::CTextAnalyzerWorker() {};
CTextAnalyzerWorker::~CTextAnalyzerWorker() {};

void CTextAnalyzerWorker::process(const WordsMap& in_words)
{
    processImpl(in_words);
}

void CTextAnalyzerWorker::finishTextAnalyzing()
{
    processImpl({ { "", 0 } }, true);
}

void CTextAnalyzerWorker::finishProcessing()
{
    m_dictionary.clear();
    m_topLetterCombinations.clear();
    m_topLetterCombinations.shrink_to_fit();
    m_wordsLetterCombinations.clear();
    m_topLetterCombinationsMinCount = 0;
    m_totalLetterCombinationsCount = 0;
    m_wordsProcessed = 0;
}

void CTextAnalyzerWorker::processImpl(const WordsMap& in_words, bool in_force)
{
    bool isNeedToUpdateTopLetterCombinations = false;
    for (const auto& [word, count] : CommonData::asKeyValueRange(in_words)) {
        if (word.isEmpty()) {
            continue;
        }

        m_wordsProcessed += count;
        if (word.size() < CommonData::MinLetterCombinationLength) {
            continue;
        }

        // Check if the word with the calculated hash had already been analyzed and its letter combinations contained in storage.
        auto hash = qHash(word);
        auto wordsLetterCombinationIt = m_wordsLetterCombinations.find(hash);
        if (wordsLetterCombinationIt != m_wordsLetterCombinations.end()) {
            auto wordLetterCombinations = m_wordsLetterCombinations[hash];
            auto& wordCount = count;
            std::for_each(std::begin(wordLetterCombinations), std::end(wordLetterCombinations), [&](const auto& p){
                const quint64 updatedWordLetterCombinationsCount = p.second * wordCount;
                auto it = m_dictionary.find(p.first);
                if (it != std::end(m_dictionary)) {
                    it->second.second += updatedWordLetterCombinationsCount;
                    if (!isNeedToUpdateTopLetterCombinations && it->second.second > m_topLetterCombinationsMinCount) {
                        isNeedToUpdateTopLetterCombinations = true;
                    }
                }
                m_totalLetterCombinationsCount += updatedWordLetterCombinationsCount;
            });
        } else {
            auto wordLetterCombinations = findWordSubstrings(word);
            LetterCombinationsMap letterHashCombinations;
            for (const auto& [letterCombinationHash, letterCombinationData] : wordLetterCombinations) {
                letterHashCombinations[letterCombinationHash] = letterCombinationData.second;
            }
            m_wordsLetterCombinations[hash] = letterHashCombinations;
            auto& wordCount = count;
            std::for_each(std::begin(wordLetterCombinations), std::end(wordLetterCombinations), [&](const auto& p){
                const quint64 updatedWordLetterCombinationsCount = p.second.second * wordCount;
                auto it = m_dictionary.find(p.first);
                if (it != std::end(m_dictionary)) {
                    it->second.second += updatedWordLetterCombinationsCount;
                    if (!isNeedToUpdateTopLetterCombinations && it->second.second > m_topLetterCombinationsMinCount) {
                        isNeedToUpdateTopLetterCombinations = true;
                    }
                } else {
                    m_dictionary[p.first] = { p.second.first, updatedWordLetterCombinationsCount };
                    if (!isNeedToUpdateTopLetterCombinations && updatedWordLetterCombinationsCount > m_topLetterCombinationsMinCount) {
                        isNeedToUpdateTopLetterCombinations = true;
                    }
                }
                m_totalLetterCombinationsCount += updatedWordLetterCombinationsCount;
            });
        }
    }

    if (!in_force && !isNeedToUpdateTopLetterCombinations)
        return;

    // Copy the dictionary map to a vector and sort it in words' letter combinations count descending order.
    DictionaryVector vDictionary(m_dictionary.begin(), m_dictionary.end());
    std::stable_sort(std::begin(vDictionary), std::end(vDictionary), [](const auto& lhs, const auto& rhs){
        return lhs.second.second != rhs.second.second ? lhs.second.second > rhs.second.second : lhs.first < rhs.first;
    });

    // Obtain the most common words' letter combinations from the dictionary.
    WordsVector vTopLetterCombinations;
    DictionaryVector vTopLetterCombinationsDictionary;
    std::copy_n(std::begin(vDictionary), std::min(static_cast<size_t>(CommonData::TopLetterCombinationsCount), vDictionary.size()), std::back_inserter(vTopLetterCombinationsDictionary));
    std::transform(std::begin(vTopLetterCombinationsDictionary), std::end(vTopLetterCombinationsDictionary), std::back_inserter(vTopLetterCombinations), [](const auto& el) {
        return QPair(el.second.first, el.second.second);
    });

    // Check if the most common words' letter combinations have been changed.
    if (vTopLetterCombinations != m_topLetterCombinations) {
        m_topLetterCombinations = vTopLetterCombinations;

        // Calculate and store minimum letter combinations count in the most common words' letter combinations.
        QVector<quint64> vTopLetterCombinationsCount;
        std::transform(std::begin(vTopLetterCombinations), std::end(vTopLetterCombinations), std::back_inserter(vTopLetterCombinationsCount), [](const auto& el) {
            return el.second;
        });
        auto minWordsCountIt = std::min_element(std::begin(vTopLetterCombinationsCount), std::end(vTopLetterCombinationsCount));
        if (minWordsCountIt != std::end(vTopLetterCombinationsCount)) {
            m_topLetterCombinationsMinCount = *minWordsCountIt;
        }

        emit totalLetterCombinationsCountUpdated(m_totalLetterCombinationsCount);
        emit wordsProcessedCountUpdated(m_wordsProcessed);
        if (!in_force) {
            emit topLetterCombinationsUpdated(vTopLetterCombinations);
        } else {
            emit textAnalyzingFinished(vTopLetterCombinations);
        }
    } else {
        if (in_force) {
            WordsVector vTopLetterCombinationsEmpty;
            emit totalLetterCombinationsCountUpdated(m_totalLetterCombinationsCount);
            emit wordsProcessedCountUpdated(m_wordsProcessed);
            emit textAnalyzingFinished(vTopLetterCombinationsEmpty);
        }
    }
}

DictionaryMap CTextAnalyzerWorker::findWordSubstrings(const QString& in_word)
{
    DictionaryMap letterCombinations;
    for (int i = 0; i < in_word.size(); ++i) {
        for (int length = CommonData::MinLetterCombinationLength; length <= in_word.size() - i; ++length) {
            addLetterCombinationToMap(in_word.mid(i, length), letterCombinations);
        }
    }
    return letterCombinations;
}

void CTextAnalyzerWorker::addLetterCombinationToMap(const QString& in_word, DictionaryMap& inout_letterCombinations)
{
    if (in_word.length() >= CommonData::MinLetterCombinationLength) {
        auto hash = qHash(in_word);
        auto it = inout_letterCombinations.find(hash);
        if (it == std::end(inout_letterCombinations)) {
            inout_letterCombinations[hash] = { in_word, 1 };
        } else {
            ++it->second.second;
        }
    }
}
