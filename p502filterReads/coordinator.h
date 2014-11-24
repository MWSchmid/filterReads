#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "../p502_SOURCE/readsHandlerIllumina/readshandlerilluminahelper.h"
#include "../p502_SOURCE/readsHandlerIllumina/readshandlerillumina.h"
#include "readsfilter.h"

#include <QtGui>

class coordinator : public QThread
{ Q_OBJECT
private:
    //! some checks
    bool _allFine;
    bool _isCancelled;
    QString _errorMessage;

    //! input and output files
    QString FILTERSEQUENCES;
    QString READSINFILE;
    QString READSOUTFILE;

    //! parameters for the read file handlers (taken from the reader/writer header files)
    // some settings:
    bool SIZING;
    bool COMPRESSEDINPUT;
    bool COMPRESSEDOUTPUT;
    // choose a fixed length (starting from left) - is done after chopping
    int FIXEDLENGTH;
    // chop left or right
    int CHOPLEFT;
    int CHOPRIGHT;
    // the default values are all disabling the respective filter
    bool FILTER;
    // minimal average quality
    float MINQUAL;
    // skip reads with this character
    QChar SKIPCHAR;
    // search for uncomplex char, string, or words
    QChar UNCOMPLEXCHAR;
    QString UNCOMPLEXSTRING;
    int UNCOMPLEXWORDSIZE;

    //! buffer and coordination related things
    // variables related to the buffer of the reader
    int _bufferSizeINPUT;
    QList<illuminaRead> _bufferINPUT;
    QWaitCondition _bufferIsNotFullINPUT;
    QWaitCondition _bufferIsNotEmptyINPUT;
    QMutex _mutexINPUT;
    int _usedSpaceINPUT;
    bool _producerIsFinished;

    // variables related to the buffer of the writer
    int _bufferSizeOUTPUT;
    QList<illuminaRead> _bufferOUTPUT;
    QWaitCondition _bufferIsNotFullOUTPUT;
    QWaitCondition _bufferIsNotEmptyOUTPUT;
    QMutex _mutexOUTPUT;
    int _usedSpaceOUTPUT;


    //! variables used for controlling this thread
    QQueue<QString> _work;
    QWaitCondition _workAdded;
    QMutex _mutex;

public:
    coordinator();
    ~coordinator();
    void addWork(QString workString); // will add some work to the queue
    //! NOTE THAT ONE HAS TO SUPPLY AT LEAST FIVE ENTRIES - OR ALL. IN CASE OF FIVE: FILTERSEQUENCES, READSINFILE, READSOUTFILE, COMPRESSED, FIXEDLENGTH

protected:
    void run();

private:
    bool doProcess(QString workString);

public slots:
    void cancelProcessing();

signals:
    void processStatus(QString status); // signals what is going on
    void errorMessage(QString message);
    void workFinished(QString workString);
    void idleAgain();
    void stopThreads();
};

#endif // COORDINATOR_H
