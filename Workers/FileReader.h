#ifndef FILEREADER_H
#define FILEREADER_H

#include "CommonData.h"

#include <QObject>
#include <QAtomicInteger>

class CFileReaderWorker : public QObject
{
    Q_OBJECT

public:
    CFileReaderWorker();
    ~CFileReaderWorker();

public slots:
    //! Reads a file with the specified name by lines or chunks and processes obtained chunks - counts words.
    void process(const QString& fileName);

    //! Stops file processing. This method is thread safe.
    void stopProcessing();

signals:
    void chunkProcessed(const WordsMap&);

    //! Status from the CommonData::EFileReadingStatus enumeration.
    void statusChanged(int);

private:
    //! Splits the given string to individual words divided by whitespace characters and counts words in the given string.
    //! @param inout_remainder [in, out] - If the line length is greater than the maximum chunk size, then the first and last words in the chunk could be divided into parts.
    //!                                    It's necessary to merge word head (the last word) from the previous chunk with word tail (the first word) from the current chunk.
    void processChunk(const QString& in_chunk, qint64 in_maxChunkSize, QString& inout_remainder, WordsMap& inout_words);

    WordsMap m_words;
    QAtomicInteger<bool> m_isStop { false };
};

#endif // FILEREADER_H
