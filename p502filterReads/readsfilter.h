#ifndef READSFILTER_H
#define READSFILTER_H

#include "../p502_SOURCE/readsHandlerIllumina/readshandlerilluminahelper.h"

#include <QtGui>


class readsFilter: public QThread
{ Q_OBJECT
public:
    readsFilter(QString &filterSequences,
                const int *bufferSizeINPUT,
                QList<illuminaRead> *bufferINPUT,
                QWaitCondition *bufferIsNotFullINPUT,
                QWaitCondition *bufferIsNotEmptyINPUT,
                QMutex *mutexINPUT,
                int *usedSpaceINPUT,
                const int *bufferSizeOUTPUT,
                QList<illuminaRead> *bufferOUTPUT,
                QWaitCondition *bufferIsNotFullOUTPUT,
                QWaitCondition *bufferIsNotEmptyOUTPUT,
                QMutex *mutexOUTPUT,
                int *usedSpaceOUTPUT);
    void run();
private:
    //! input and output files
    QString FILTERSEQUENCES;
    bool _producerIsFinished;

    //! buffer and coordination related things
    // variables related to the buffer of the reader
    const int *_bufferSizeINPUT;
    QList<illuminaRead> *_bufferINPUT;
    QWaitCondition *_bufferIsNotFullINPUT;
    QWaitCondition *_bufferIsNotEmptyINPUT;
    QMutex *_mutexINPUT;
    int *_usedSpaceINPUT;

    // variables related to the buffer of the writer
    const int *_bufferSizeOUTPUT;
    QList<illuminaRead> *_bufferOUTPUT;
    QWaitCondition *_bufferIsNotFullOUTPUT;
    QWaitCondition *_bufferIsNotEmptyOUTPUT;
    QMutex *_mutexOUTPUT;
    int *_usedSpaceOUTPUT;

public slots:
    void producerFinished();

};

#endif // READSFILTER_H
