#include "../p502_SOURCE/readsHandlerIllumina/readshandlerilluminahelper.h"
#include "../p502_SOURCE/readsHandlerIllumina/readshandlerillumina.h"
#include "../p502_SOURCE/fastaHandler/fastahandler.h"
#include "readsfilter.h"

#include <QtGui>

#include <iostream>

readsFilter::readsFilter(QString &filterSequences,
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
                         int *usedSpaceOUTPUT
                     ) {
    this->FILTERSEQUENCES = filterSequences;
    this->_producerIsFinished = false;
    // variables related to the buffer of the reader
    this->_bufferSizeINPUT = bufferSizeINPUT;
    this->_bufferINPUT = bufferINPUT;
    this->_bufferIsNotFullINPUT = bufferIsNotFullINPUT;
    this->_bufferIsNotEmptyINPUT = bufferIsNotEmptyINPUT;
    this->_mutexINPUT = mutexINPUT;
    this->_usedSpaceINPUT = usedSpaceINPUT;

    // variables related to the buffer of the writer
    this->_bufferSizeOUTPUT = bufferSizeOUTPUT;
    this->_bufferOUTPUT = bufferOUTPUT;
    this->_bufferIsNotFullOUTPUT = bufferIsNotFullOUTPUT;
    this->_bufferIsNotEmptyOUTPUT = bufferIsNotEmptyOUTPUT;
    this->_mutexOUTPUT = mutexOUTPUT;
    this->_usedSpaceOUTPUT = usedSpaceOUTPUT;
}

void readsFilter::producerFinished() {
    this->_producerIsFinished = true;
    this->_mutexINPUT->lock();
    this->_bufferIsNotEmptyINPUT->wakeAll();
    this->_mutexINPUT->unlock();
}

void readsFilter::run() {
    fastaHandler references;
    if (!references.readSequences(this->FILTERSEQUENCES)) { std::cerr << "could not read fasta file" << std::endl << std::flush; }

    QMap<QString, QString >::const_iterator iter = references.sequences.constBegin();
    QString seqName = "";
    QString seq = "";
    QMap<QString, int> matchCounter;
    quint64 totalRemoved = 0;
    bool readMatches = false;

    // initialize the counter map
    iter = references.sequences.constBegin();
    while (iter != references.sequences.constEnd()) {
        seqName = iter.key();
        matchCounter[seqName] = 0;
        ++iter;
    }

    illuminaRead read;
    quint64 readcounter = 0; //! use to give a bit of progress info - at this moment, check if the thread was cancelled

    //! get the first read - we need the size
    int readSize = 0;
    while (true) {
        // start control the first buffer (INPUT)
        this->_mutexINPUT->lock();
        while ( (*(this->_usedSpaceINPUT) == 0) && (!this->_producerIsFinished) ) {
            //std::cerr << "entering wait" << std::endl << std::flush;
            this->_bufferIsNotEmptyINPUT->wait(this->_mutexINPUT);
        }
        if ((*this->_usedSpaceINPUT) == 0) {
            std::cerr << "very bad" << std::endl << std::flush;
            this->_mutexINPUT->unlock();
            break;
        }
        // take the alignment
        read = this->_bufferINPUT->first();
        //--*(this->_usedSpaceINPUT); THATS not the case now
        //this->_bufferIsNotFullINPUT->wakeAll(); for security, only wake up later
        this->_mutexINPUT->unlock();
        // end control the first buffer (INPUT)
        break;
    }
    readSize = read.BASES.length();
    std::cerr << "detected a read size of " << readSize << " bases." << std::endl << std::flush;


    //! now we can build the map for matching reads to the unwanted sequences
    QMultiHash<QString, QString> seqMap;
    QString subSeq = "";
    // initialize the match map
    iter = references.sequences.constBegin();
    while (iter != references.sequences.constEnd()) {
        seqName = iter.key();
        seq = iter.value();
        for (int i = 0; i < (seq.length()-readSize); ++i) {
            subSeq = seq.mid(i, readSize);
            seqMap.insert(subSeq, seqName);
        }
        ++iter;
    }

    // print for testing
    QMultiHash<QString, QString >::const_iterator mapIter = seqMap.constBegin();
    /*
    while (mapIter != seqMap.constEnd()) {
        std::cerr << mapIter.key().toStdString() << "\t" << mapIter.value().toStdString() << std::endl << std::flush;
        ++mapIter;
    }
    */

    //! NOTE that we did not remove the first read before - so it is not necessary to process it here

    while ( true ) {
        //! fetch a read
        // start control the first buffer (INPUT)
        this->_mutexINPUT->lock();
        while ( (*(this->_usedSpaceINPUT) == 0) && (!this->_producerIsFinished) ) {
            //std::cerr << "entering wait" << std::endl << std::flush;
            this->_bufferIsNotEmptyINPUT->wait(this->_mutexINPUT);
        }
        if ((*this->_usedSpaceINPUT) == 0) {
            //std::cerr << "exiting loop" << std::endl << std::flush;
            this->_mutexINPUT->unlock();
            break;
        }
        // take the alignment
        read = this->_bufferINPUT->takeFirst();
        --*(this->_usedSpaceINPUT);
        this->_bufferIsNotFullINPUT->wakeAll();
        this->_mutexINPUT->unlock();
        // end control the first buffer (INPUT)

        //! give some basic progress info
        ++readcounter;
        if ((readcounter % 1000000) == 0) { std::cerr << "processed " << readcounter << " reads." << std::endl << std::flush; }

        //! check the read
        readMatches = false;

        if (seqMap.contains(read.BASES)) {
            readMatches = true;
            ++totalRemoved;
            mapIter = seqMap.constFind(read.BASES);
            while (mapIter != seqMap.constEnd() && mapIter.key() == read.BASES) {
                seqName = mapIter.value();
                ++matchCounter[seqName];
                ++mapIter;
            }
        }

        //! write the read if it does not match
        if (!readMatches) {
            // start control the second buffer (OUTPUT)
            this->_mutexOUTPUT->lock();
            while (*(this->_usedSpaceOUTPUT) == *(this->_bufferSizeOUTPUT))
                this->_bufferIsNotFullOUTPUT->wait(this->_mutexOUTPUT);
            this->_bufferOUTPUT->append(read);
            ++*(this->_usedSpaceOUTPUT);
            this->_bufferIsNotEmptyOUTPUT->wakeAll();
            this->_mutexOUTPUT->unlock();
            // end control the second buffer (OUTPUT)
        }
    }

    std::cerr << "mapping stats - removed in total " << totalRemoved << " reads." << std::endl << std::flush;
    // write out the match stats
    QMap<QString, int >::const_iterator countIter = matchCounter.constBegin();
    int seqCount = 0;
    while (countIter != matchCounter.constEnd()) {
        seqName = countIter.key();
        seqCount = countIter.value();
        std::cerr << seqName.toStdString() << "\t" << seqCount << std::endl << std::flush;
        ++countIter;
    }

    emit this->finished();

}

