#include "illuminawriter.h"

#include <QtCore>
#include <iostream>
#include <qtiocompressor.h>

#include "readshandlerilluminahelper.h"

illuminaWriter::illuminaWriter(int fixedLength, int chopLeft, int chopRight, float minAveQuality, QChar skipChar, QChar uncomplexChar, QString uncomplexString, int uncomplexWordSize, bool compressed)
{
    this->FIXEDLENGTH = fixedLength;
    this->CHOPLEFT = chopLeft;
    this->CHOPRIGHT = chopRight;
    this->MINQUAL = minAveQuality;
    this->SKIPCHAR = skipChar;
    this->UNCOMPLEXCHAR = uncomplexChar;
    this->UNCOMPLEXSTRING = uncomplexString;
    this->UNCOMPLEXWORDSIZE = uncomplexWordSize;
    this->COMPRESSED = compressed;
}

illuminaWriter::~illuminaWriter()
{
    this->close();
}

bool illuminaWriter::open(const QString &fastqFile, bool filter, bool sizing)
{
    this->FILTER = filter;
    this->SIZING = sizing;
    this->FASTQFILE.setFileName(fastqFile);
    if (!this->FASTQFILE.open(QIODevice::WriteOnly)) {
        qDebug() << "Error: Cannot write file " << this->FASTQFILE.fileName() << ": " << this->FASTQFILE.errorString();
        return(false);
    } else {
        if (this->COMPRESSED) {
            this->COMPRESSOR = new QtIOCompressor(&this->FASTQFILE, 6, 1000000000);
            this->COMPRESSOR->setStreamFormat(QtIOCompressor::GzipFormat); //! TODO - CHECK FOR FORMAT AND HANDLE ERRORS
            if (!this->COMPRESSOR->open(QIODevice::WriteOnly)) {
                std::cerr << "Error: QtIOCompressor - Cannot open file " << qPrintable(fastqFile)
                          << ": " << qPrintable(this->COMPRESSOR->errorString())
                          << std::endl;
                return(false);
            }
            this->FASTQ.setDevice(this->COMPRESSOR);
        } else {
            this->FASTQ.setDevice(&this->FASTQFILE);
        }
    }
    return(true);
}

bool illuminaWriter::writeRead(illuminaRead read)
{
    // remove, chop and resize the read first (this is the reason why it is passed by value)
    if (this->SIZING) {
        read.BASES.remove(0, this->CHOPLEFT);
        read.BASES.chop(this->CHOPRIGHT);
        read.BASES.resize(this->FIXEDLENGTH);
        read.QUALS.remove(0, this->CHOPLEFT);
        read.QUALS.chop(this->CHOPRIGHT);
        read.QUALS.resize(this->FIXEDLENGTH);
    }

    if (this->FILTER) {
        if ( (this->MINQUAL != 0) && (read.hasLowAverageQuality(this->MINQUAL)) )                       { return(false); }
        if ( (this->SKIPCHAR != QChar()) && (read.hasChar(this->SKIPCHAR)))                             { return(false); }
        if ( (this->UNCOMPLEXCHAR != QChar()) && (read.isUncomplexChar(this->UNCOMPLEXCHAR)))           { return(false); }
        if ( (this->UNCOMPLEXSTRING != QString()) && (read.isUncomplexString(this->UNCOMPLEXSTRING)))   { return(false); }
        if ( (this->UNCOMPLEXWORDSIZE != 0) && (read.isUncomplexWordsize(this->UNCOMPLEXWORDSIZE)))     { return(false); }
    }

    // write into the files
    this->FASTQ << '@' << read.NAME << '\n' << read.BASES << "\n+\n" << read.QUALS << '\n';

    return(true);
}

bool illuminaWriter::close()
{
    if (this->FASTQFILE.isOpen()) {
        this->FASTQ.flush();
        if (this->COMPRESSED) { this->COMPRESSOR->close(); }
        this->FASTQFILE.close();
        if (this->FASTQFILE.error() != QFile::NoError) {
            qDebug() << "Error: Cannot write file " << this->FASTQFILE.fileName() << ": " << this->FASTQFILE.errorString();
            return(false);
        }
    }
    return(true);
}
