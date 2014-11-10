#ifndef ILLUMINAREADER_H
#define ILLUMINAREADER_H

#include <QtCore>
#include <iostream>
//#include <qtiocompressor.h>

#include "readshandlerilluminahelper.h"

class QtIOCompressor;

class illuminaReader
{
public:
    illuminaReader(int fixedLength = 0,
                   int chopLeft = 0,
                   int chopRight = 0,
                   float minAveQuality = 0,
                   QChar skipChar = QChar(),
                   QChar uncomplexChar = QChar(),
                   QString uncomplexString = QString(),
                   int uncomplexWordSize = 0,
                   bool compressed = false);
    ~illuminaReader();

    bool open(const QString& fastqFile, bool filter = false, bool sizing = false);
    bool readRead(illuminaRead& read);
    bool close();

private:
    QFile FASTQFILE;
    QTextStream FASTQ;
    QtIOCompressor *COMPRESSOR; //! BE CAREFUL WITH THIS ONE
    // some settings:
    bool SIZING;
    bool COMPRESSED;
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
};

#endif // ILLUMINAREADER_H
