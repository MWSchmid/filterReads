#include "coordinator.h"

#include "../p502_SOURCE/readsHandlerIllumina/readshandlerilluminahelper.h"
#include "../p502_SOURCE/readsHandlerIllumina/readshandlerillumina.h"
#include "../p502_SOURCE/fastaHandler/fastahandler.h"
#include "coordinator.h"

#include <QtGui>

#include <iostream>

coordinator::coordinator()
{ //! just set some values - they will be overwritten by the workstring later on
    this->_allFine = true;
    this->_isCancelled = false;
    //! input and output files
    this->FILTERSEQUENCES = "";
    this->READSINFILE = "";
    this->READSOUTFILE = "";
    //! parameters for the read file handlers (taken from the reader/writer header files)
    this->SIZING = false;
    this->COMPRESSED = false;
    this->FIXEDLENGTH = 0;
    this->CHOPLEFT = 0;
    this->CHOPRIGHT = 0;
    this->FILTER = false;
    this->MINQUAL = 0;
    this->SKIPCHAR = QChar();
    this->UNCOMPLEXCHAR = QChar();
    this->UNCOMPLEXSTRING = QChar();
    this->UNCOMPLEXWORDSIZE = 0;
    //! buffer and coordination related things
    // variables related to the buffer of the reader
    this->_bufferSizeINPUT = 100000;
    this->_bufferINPUT.clear();
    this->_bufferINPUT.reserve(this->_bufferSizeINPUT);
    this->_usedSpaceINPUT = 0;
    this->_producerIsFinished = false;
    // variables related to the buffer of the writer
    this->_bufferSizeOUTPUT = 100000;
    this->_bufferOUTPUT.clear();
    this->_bufferOUTPUT.reserve(this->_bufferSizeOUTPUT);
    this->_usedSpaceOUTPUT = 0;
    //! START THE THREAD
    this->start();

}

coordinator::~coordinator()
{
    std::cerr << "calling destructor of coordinator - this should happen at the very end!" << std::endl << std::flush;
    emit this->stopThreads();
    // empty the queue of projects and add STOPTHREAD - this tells the tread to exit the forever loop
    {
        QMutexLocker locker(&this->_mutex);
        while (!this->_work.isEmpty()) {
            this->_work.dequeue();
        }
        this->_work.enqueue("STOPTHREAD");
        this->_workAdded.wakeOne();
    }
    // wait before calling the base class destructor
    this->wait();
}

void coordinator::addWork(QString workString) {
    QMutexLocker locker(&this->_mutex);
    this->_work.enqueue(workString);
    this->_workAdded.wakeOne();
}

void coordinator::run() {
    QString workString;

    forever {
        {// check if there is something to process
            QMutexLocker locker(&this->_mutex);
            if (this->_work.isEmpty()) {
                this->_workAdded.wait(&this->_mutex);
            }
            // take a project from the queue
            workString = this->_work.dequeue();
            // here is a keyword that cancels the run and stops everything
            if (workString == "STOPTHREAD") {
                break;
            }
        }

        emit this->processStatus("starting");
        this->_allFine = this->doProcess(workString);
        if (!this->_allFine) {
            if (this->_isCancelled) {
                this->_isCancelled = false;
                this->_allFine = true;
            } else {
                this->_errorMessage = "somewhere an error";
                emit this->errorMessage(this->_errorMessage);
            }
        }
        emit this->workFinished(workString);
        emit this->processStatus("idle");
        // check if all are processed
        {
            QMutexLocker locker(&this->_mutex);
            if (this->_work.isEmpty()) {
                emit this->idleAgain();
            }
        }
    }
}

bool coordinator::doProcess(QString workString) {
    QStringList temp = workString.split("|");
    this->READSINFILE = temp.at(0);
    this->READSOUTFILE = temp.at(1);
    this->FILTERSEQUENCES = temp.at(2);
    if (temp.at(3) == "y") { this->COMPRESSED = true; }
    if (temp.length() > 4) {
        if (temp.at(4) == "y") {
            this->SIZING = true;
            this->FIXEDLENGTH = temp.at(5).toInt();
            this->CHOPLEFT = temp.at(6).toInt();
            this->CHOPRIGHT = temp.at(7).toInt();
        }
        if (temp.at(8) == "y") {
            this->FILTER = true;
            this->MINQUAL = temp.at(9).toFloat();
            this->SKIPCHAR = temp.at(10).at(0);
            this->UNCOMPLEXCHAR = temp.at(11).at(0);
            this->UNCOMPLEXSTRING = temp.at(12);
            this->UNCOMPLEXWORDSIZE = temp.at(13).toInt();
        }
    }

    //! START THE OTHER THREADS
    readsHandlerIllumina producer(this->READSINFILE,
                                  "",
                                  "r",
                                  &this->_bufferSizeINPUT,
                                  &this->_bufferINPUT,
                                  &this->_bufferIsNotFullINPUT,
                                  &this->_bufferIsNotEmptyINPUT,
                                  &this->_mutexINPUT,
                                  &this->_usedSpaceINPUT,
                                  this->FILTER,
                                  this->SIZING,
                                  this->FIXEDLENGTH,
                                  this->CHOPLEFT,
                                  this->CHOPRIGHT,
                                  this->MINQUAL,
                                  this->SKIPCHAR,
                                  this->UNCOMPLEXCHAR,
                                  this->UNCOMPLEXSTRING,
                                  this->UNCOMPLEXWORDSIZE,
                                  this->COMPRESSED);

    readsHandlerIllumina consumer("",
                                  this->READSOUTFILE,
                                  "w",
                                  &this->_bufferSizeOUTPUT,
                                  &this->_bufferOUTPUT,
                                  &this->_bufferIsNotFullOUTPUT,
                                  &this->_bufferIsNotEmptyOUTPUT,
                                  &this->_mutexOUTPUT,
                                  &this->_usedSpaceOUTPUT,
                                  false,
                                  false,
                                  this->FIXEDLENGTH,
                                  0,
                                  0,
                                  0,
                                  QChar(),
                                  QChar(),
                                  QString(),
                                  0,
                                  this->COMPRESSED);

    readsFilter filter(this->FILTERSEQUENCES,
                       &this->_bufferSizeINPUT,
                       &this->_bufferINPUT,
                       &this->_bufferIsNotFullINPUT,
                       &this->_bufferIsNotEmptyINPUT,
                       &this->_mutexINPUT,
                       &this->_usedSpaceINPUT,
                       &this->_bufferSizeOUTPUT,
                       &this->_bufferOUTPUT,
                       &this->_bufferIsNotFullOUTPUT,
                       &this->_bufferIsNotEmptyOUTPUT,
                       &this->_mutexOUTPUT,
                       &this->_usedSpaceOUTPUT
                       );
    // set the connections
    connect(this, SIGNAL(stopThreads()), &producer, SLOT(cancelProcessing()), Qt::DirectConnection); //! NOTE THAT THE OTHER ONES WILL KEEP ON GOING UNTIL BUFFERS ARE EMPTY
    connect(&producer, SIGNAL(finished()), &filter, SLOT(producerFinished()), Qt::DirectConnection);
    connect(&filter, SIGNAL(finished()), &consumer, SLOT(producerFinished()), Qt::DirectConnection);

    //! start the threads
    producer.start();
    filter.start();
    consumer.start();

    //! wait for them
    producer.wait();
    filter.wait();
    consumer.wait();

    emit this->idleAgain();

    return(true);
}

void coordinator::cancelProcessing() {
    emit this->stopThreads();
}

