#ifndef CONSOLERUNNER_H
#define CONSOLERUNNER_H

#include "coordinator.h"

// a mini class to run the data analysis in a console
class consoleRunner : public QThread
{ Q_OBJECT

private:
    QString command;
    coordinator processor;

public:
    consoleRunner(QString &command){
        QStringList temp = command.split(",");
        this->command = temp.join("|");
        // set connections
        connect(&processor, SIGNAL(processStatus(QString)), this, SLOT(printStatus(QString)), Qt::DirectConnection);
        //connect(&processor, SIGNAL(processProgress(int)), this, SLOT(printProgress(int)), Qt::DirectConnection);
        connect(&processor, SIGNAL(errorMessage(QString)), this, SLOT(printError(QString)), Qt::DirectConnection);
    }

    //~consoleRunner(){}

    void startAnalysis() {
        std::cerr << QTime::currentTime().toString().toStdString() << std::endl << std::flush;
        processor.addWork(this->command);
    }

protected:
    void run() {
        std::cerr << "running p502filterReads via the command line" << std::endl << std::flush;
        processor.addWork("STOPTHREAD");
        processor.wait();
        std::cerr << "stopping p502filterReads" << std::endl << std::flush;
        //emit this->finished(); // this cannot be done anymore in Qt5.8, should be sent automatically
    }

private slots:
    void printStatus(QString status){  std::cerr << qPrintable(QTime::currentTime().toString()) << " " << status.toStdString() << std::endl << std::flush; }
    void printProgress(int progress) { std::cerr << qPrintable(QTime::currentTime().toString()) << " " << progress << "%" << std::endl << std::flush; }
    void printError(QString errorMessage) { std::cerr << "ERROR: " << errorMessage.toStdString() << std::endl << std::flush; }
};

#endif // CONSOLERUNNER_H
