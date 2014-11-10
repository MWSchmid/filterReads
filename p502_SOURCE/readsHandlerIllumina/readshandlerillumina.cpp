#include <QtCore>
#include <iostream>

#include "readshandlerillumina.h"
#include "readshandlerilluminahelper.h"
#include "illuminareader.h"
#include "illuminawriter.h"

readsHandlerIllumina::readsHandlerIllumina(QString fileNameRead,
                                           QString fileNameWrite,
                                           QString mode,
                                           const int *bufferSize,
                                           QList<illuminaRead> *buffer,
                                           QWaitCondition *bufferIsNotFull,
                                           QWaitCondition *bufferIsNotEmpty,
                                           QMutex *mutex,
                                           int *usedSpace,
                                           bool filter,
                                           bool sizing,
                                           int fixedLength,
                                           int chopLeft,
                                           int chopRight,
                                           float minAveQuality,
                                           QChar skipChar,
                                           QChar uncomplexChar,
                                           QString uncomplexString,
                                           int uncomplexWordSize,
                                           bool compressed)
{
    // thread control
    this->FILENAMEREAD = fileNameRead;
    this->FILENAMEWRITE = fileNameWrite;
    this->MODE = mode;
    this->BUFFERSIZE = bufferSize;
    this->BUFFER = buffer;
    this->BUFFERISNOTFULL = bufferIsNotFull;
    this->BUFFERISNOTEMPTY = bufferIsNotEmpty;
    this->MUTEX = mutex;
    this->USEDSPACE = usedSpace;
    this->PRODUCERISFINISHED = false;
    // filtering and sizing
    this->FILTER = filter;
    this->SIZING = sizing;
    this->FIXEDLENGTH = fixedLength;
    this->CHOPLEFT = chopLeft;
    this->CHOPRIGHT = chopRight;
    this->MINQUAL = minAveQuality;
    this->SKIPCHAR = skipChar;
    this->UNCOMPLEXCHAR = uncomplexChar;
    this->UNCOMPLEXSTRING = uncomplexString;
    this->UNCOMPLEXWORDSIZE = uncomplexWordSize;
    this->COMPRESSED = compressed;
    this->_isCancelled = false;
}

//! the signal that needs to be connected to the finished producer
void readsHandlerIllumina::producerFinished() {
    this->PRODUCERISFINISHED = true;
    this->MUTEX->lock();
    this->BUFFERISNOTEMPTY->wakeAll();
    this->MUTEX->unlock();
}

void readsHandlerIllumina::cancelProcessing() {
    this->_isCancelled = true;
}

void readsHandlerIllumina::run()
{
    if ( this->MODE == "r" ) {
        this->readReads();
    } else if ( this->MODE == "w" ) {
        this->writeReads();
    } else {
        qDebug() << "wrong mode";
    }
    emit this->finished();
}

void readsHandlerIllumina::readReads()
{
    illuminaReader reader(this->FIXEDLENGTH,
                          this->CHOPLEFT,
                          this->CHOPRIGHT,
                          this->MINQUAL,
                          this->SKIPCHAR,
                          this->UNCOMPLEXCHAR,
                          this->UNCOMPLEXSTRING,
                          this->UNCOMPLEXWORDSIZE,
                          this->COMPRESSED);
    illuminaRead read;

    reader.open(this->FILENAMEREAD, this->FILTER, this->SIZING);
    while (reader.readRead(read)) {
        if (this->_isCancelled) { break; }
        // push to the buffer
        this->MUTEX->lock();
        while (*(this->USEDSPACE) == *(this->BUFFERSIZE)) {
            this->BUFFERISNOTFULL->wait(this->MUTEX);
        }
        this->BUFFER->append(read);
        ++*(this->USEDSPACE);
        this->BUFFERISNOTEMPTY->wakeAll();
        this->MUTEX->unlock();
    }
    reader.close();
    qDebug() << "reader finished successfully";
}

void readsHandlerIllumina::writeReads()
{
    illuminaWriter writer(this->FIXEDLENGTH,
                          this->CHOPLEFT,
                          this->CHOPRIGHT,
                          this->MINQUAL,
                          this->SKIPCHAR,
                          this->UNCOMPLEXCHAR,
                          this->UNCOMPLEXSTRING,
                          this->UNCOMPLEXWORDSIZE,
                          this->COMPRESSED);
    illuminaRead read;

    writer.open(this->FILENAMEWRITE, this->FILTER, this->SIZING);

    //consume from a buffer, and write
    while ( true ) {
        this->MUTEX->lock();
        // wait if there is more to come
        while ( (*(this->USEDSPACE) == 0) && (!this->PRODUCERISFINISHED) ) {
            this->BUFFERISNOTEMPTY->wait(this->MUTEX); // this also unlocks the mutex
        }
        // exit the loop if the buffer is empty
        if ((*this->USEDSPACE) == 0) {
            this->MUTEX->unlock();
            break;
        }
        // otherwise take one read from the list
        read = this->BUFFER->takeFirst();
        writer.writeRead(read);
        --*(this->USEDSPACE);
        this->BUFFERISNOTFULL->wakeAll();
        this->MUTEX->unlock();
    }
    writer.close();
    qDebug() << "writer finished successfully";
}

