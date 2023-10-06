#include "FileReader.h"

#include <QFile>
#include <QTextStream>

#include <regex>

CFileReaderWorker::CFileReaderWorker() {}
CFileReaderWorker::~CFileReaderWorker() {}

void CFileReaderWorker::process(const QString& in_fileName)
{
    m_isStop = false;
    QFile file(in_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit statusChanged(CommonData::efrsFileOpenError);
        return;
    }

    constexpr qint64 maxChunkSize = 4096;
    constexpr unsigned int chunksToProcess = 100;
    QString remainder;
    QTextStream textStream(&file);
    QString chunk = textStream.readLine(maxChunkSize);
    unsigned int chunkCounter = 0;
    while (!chunk.isNull()) {
        ++chunkCounter;
        if (m_isStop) {
            emit statusChanged(CommonData::efrsFileProcessingInterrupted);
            return;
        }
        processChunk(chunk, maxChunkSize, remainder, m_words);
        if (!(chunkCounter % chunksToProcess)) {
            emit chunkProcessed(m_words);
            m_words.clear();
            chunkCounter = 0;
        }
        chunk = textStream.readLine(maxChunkSize);
    }
    if (!m_words.isEmpty()) {
        emit chunkProcessed(m_words);
        m_words.clear();
    }
    if (!remainder.isEmpty()) {
        emit chunkProcessed({ { remainder, 1 } });
    }
    emit statusChanged(CommonData::efrsFileProcessingFinished);
}

void CFileReaderWorker::stopProcessing()
{
    m_isStop = true;
}

void CFileReaderWorker::processChunk(const QString& in_chunk, qint64 in_maxChunkSize, QString& inout_remainder, WordsMap& inout_words)
{
    auto addWordToMap = [&inout_words](const QString& in_word){
        if (!in_word.isEmpty()) {
            if (!inout_words.contains(in_word)) {
                inout_words[in_word] = 1;
            } else {
                ++inout_words[in_word];
            }
        }
    };

    auto reduceWordCountInMap = [&inout_words](const QString& in_word) {
        if (!in_word.isEmpty()) {
            if (inout_words.contains(in_word)) {
                --inout_words[in_word];
                if (!inout_words[in_word]) {
                    inout_words.remove(in_word);
                }
            }
        }
    };

    // Split the given string to individual words divided by whitespace and punctuation characters.
    std::wregex rx(L"[[:space:][:punct:]]+");
    std::wstring chunk((const wchar_t*)in_chunk.utf16());
    std::wsregex_token_iterator wordsIt(std::begin(chunk), std::end(chunk), rx, -1), wordsEndIt;
    QString firstWord, lastWord;
    int wordsCounter = 0;
    while (wordsIt != wordsEndIt) {
        QString word = QString::fromStdWString(*wordsIt);
        word = word.toLower();  // Consider words in lowercase.
        if (!wordsCounter) {
            firstWord = word;
        }
        lastWord = word;
        ++wordsIt;
        if (!word.isEmpty()) {
            addWordToMap(word);
            ++wordsCounter;
        }
    }

    if (wordsCounter == 1) {
        lastWord.clear();
    }

    // Analyze the chunk's head.
    int firstWordIndex = in_chunk.indexOf(firstWord, 0, Qt::CaseInsensitive);
    const bool chunkEndsWithFirstWord = in_chunk.endsWith(firstWord, Qt::CaseInsensitive);
    bool remainderFilled = false;

    // If chunk begins from the first word, then there aren't any whitespace or punctuation symbols before.
    // If not, then take and "stitch" it with the remainder word from the previous chunk.
    if (!firstWord.isEmpty() && !firstWordIndex && !inout_remainder.isEmpty()) {
        // Merge word head (the last word) from the previous chunk with word tail (the first word) from the current chunk.
        reduceWordCountInMap(firstWord);
        QString assembledRemainderWord = firstWord;
        assembledRemainderWord.prepend(inout_remainder);
        if ((wordsCounter == 1 && !chunkEndsWithFirstWord) || (wordsCounter == 1 && chunkEndsWithFirstWord && in_chunk.size() < in_maxChunkSize) || wordsCounter > 1) {
            addWordToMap(assembledRemainderWord);
            inout_remainder.clear();
        } else {
            inout_remainder = assembledRemainderWord;
            remainderFilled = true;
        }
    } else if (!inout_remainder.isEmpty()) {
        addWordToMap(inout_remainder);
        inout_remainder.clear();
    }

    // Analyze the chunk's tail.
    // Check and save remainder word, if necessary.
    if (!lastWord.isEmpty() && in_chunk.size() == in_maxChunkSize && in_chunk.endsWith(lastWord, Qt::CaseInsensitive)) {
        inout_remainder = lastWord;
        reduceWordCountInMap(lastWord);
    } else if (!remainderFilled && lastWord.isEmpty() && wordsCounter == 1 && !firstWord.isEmpty() && in_chunk.size() == in_maxChunkSize && chunkEndsWithFirstWord) {
        inout_remainder = firstWord;
        reduceWordCountInMap(firstWord);
    }
}
