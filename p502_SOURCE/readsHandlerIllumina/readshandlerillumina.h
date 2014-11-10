#ifndef READSHANDLERILLUMINA_H
#define READSHANDLERILLUMINA_H

#include <QtCore>
#include <iostream>

#include "readshandlerilluminahelper.h"
#include "illuminareader.h"
#include "illuminawriter.h"

class readsHandlerIllumina : public QThread
{Q_OBJECT
public:
    readsHandlerIllumina(QString fileNameRead,
                         QString fileNameWrite,
                         QString mode,
                         const int *bufferSize,
                         QList<illuminaRead> *buffer,
                         QWaitCondition *bufferIsNotFull,
                         QWaitCondition *bufferIsNotEmpty,
                         QMutex *mutex,
                         int *usedSpace,
                         bool filter = false,
                         bool sizing = false,
                         int fixedLength = 0,
                         int chopLeft = 0,
                         int chopRight = 0,
                         float minAveQuality = 0,
                         QChar skipChar = QChar(),
                         QChar uncomplexChar = QChar(),
                         QString uncomplexString = QString(),
                         int uncomplexWordSize = 0,
                         bool compressed = false);

private:
    // things related to threadcontrol
    QString FILENAMEREAD;
    QString FILENAMEWRITE;
    QString MODE;
    const int *BUFFERSIZE;
    QList<illuminaRead> *BUFFER;
    QWaitCondition *BUFFERISNOTFULL;
    QWaitCondition *BUFFERISNOTEMPTY;
    QMutex *MUTEX;
    int *USEDSPACE;
    bool PRODUCERISFINISHED;
    // things related to the options of the reader/writer
    bool SIZING;
    int FIXEDLENGTH;
    int CHOPLEFT;
    int CHOPRIGHT;
    bool FILTER;
    float MINQUAL;
    QChar SKIPCHAR;
    QChar UNCOMPLEXCHAR;
    QString UNCOMPLEXSTRING;
    int UNCOMPLEXWORDSIZE;
    bool COMPRESSED;
    bool _isCancelled;

    //! run
    void run();

    //! functions to read into, and write from a buffer - are called by run()
    // a function that reads illuminaReads, and stores them in a buffer
    void readReads();

    // a function that takes illuminaReads from a buffer, and writes them to a file
    void writeReads();

signals:
    void finished();

public slots:
    void producerFinished(); // this function is required for the threaded writing. It should somehow only be linked to one thread (if there are several producers, one would maybe have to make an observer thread ?)

public slots:
    void cancelProcessing(); // this is only for the reader - the writer is not stoppable (only via producerFinished)
};

#endif // READSHANDLERILLUMINA_H
