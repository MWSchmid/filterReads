#include "illuminareader.h"

#include <QtCore>
#include <iostream>
#include <qtiocompressor.h>

#include "readshandlerilluminahelper.h"

illuminaReader::illuminaReader(int fixedLength, int chopLeft, int chopRight, float minAveQuality, QChar skipChar, QChar uncomplexChar, QString uncomplexString, int uncomplexWordSize, bool compressed)
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

illuminaReader::~illuminaReader()
{
    this->close();
}

bool illuminaReader::open(const QString &fastqFile, bool filter, bool sizing)
{
    this->FILTER = filter;
    this->SIZING = sizing;
    this->FASTQFILE.setFileName(fastqFile);
    if (!this->FASTQFILE.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: Cannot read file " << this->FASTQFILE.fileName() << ": " << this->FASTQFILE.errorString();
        return(false);
    } else {
        if (this->COMPRESSED) {
            this->COMPRESSOR = new QtIOCompressor(&this->FASTQFILE, 6, 1000000000);
            this->COMPRESSOR->setStreamFormat(QtIOCompressor::GzipFormat); //! TODO - CHECK FOR FORMAT AND HANDLE ERRORS
            if (!this->COMPRESSOR->open(QIODevice::ReadOnly)) {
                std::cerr << "Error: QtIOCompressor - Cannot open file " << qPrintable(fastqFile)
                          << ": " << qPrintable(this->COMPRESSOR->errorString())
                          << std::endl;
                return(false);
            }
            if (!this->COMPRESSOR->bytesAvailable()) {
                std::cerr << "weird - no bytes available" << std::endl << std::flush;
            }
            this->FASTQ.setDevice(this->COMPRESSOR);
        } else {
            this->FASTQ.setDevice(&this->FASTQFILE);
        }
    }
    return(true);
}

bool illuminaReader::readRead(illuminaRead &read)
{
    // check if one of them is at the end
    if (this->FASTQ.atEnd()) { return(false); }

    // search the header
    QString header = this->FASTQ.readLine();
    while ((header.at(0) != '@') && (!this->FASTQ.atEnd())) { header = this->FASTQ.readLine(); }
    header.remove(0,1); // @
    // read the nucleotide String
    if (this->FASTQ.atEnd()) { qDebug() << "premature end of fastq file"; return(false); }
    QString nucSeq = this->FASTQ.readLine();
    // read over the qual header
    if (this->FASTQ.atEnd()) { qDebug() << "premature end of fastq file"; return(false); }
    this->FASTQ.readLine(); //! should be fine, or?
    // read the quality
    if (this->FASTQ.atEnd()) { qDebug() << "premature end of fastq file"; return(false); }
    QString qualSeq = this->FASTQ.readLine();

    // chop if something to chop
    if (this->SIZING) {
        nucSeq.remove(0, this->CHOPLEFT);
        nucSeq.chop(this->CHOPRIGHT);
        nucSeq.resize(this->FIXEDLENGTH);
        qualSeq.remove(0, this->CHOPLEFT);
        qualSeq.chop(this->CHOPRIGHT);
        qualSeq.resize(this->FIXEDLENGTH);
    }

    // update the read
    read.update(header, nucSeq, qualSeq);

    // if no filter is requested, return...
    if (!this->FILTER) { return(true); }

    // otherwise check if the read is ok and read the next if it is not
    if ( (this->MINQUAL != 0) && (read.hasLowAverageQuality(this->MINQUAL)) )                       { return(this->readRead(read)); }
    if ( (this->SKIPCHAR != QChar()) && (read.hasChar(this->SKIPCHAR)))                             { return(this->readRead(read)); }
    if ( (this->UNCOMPLEXCHAR != QChar()) && (read.isUncomplexChar(this->UNCOMPLEXCHAR)))           { return(this->readRead(read)); }
    if ( (this->UNCOMPLEXSTRING != QString()) && (read.isUncomplexString(this->UNCOMPLEXSTRING)))   { return(this->readRead(read)); }
    if ( (this->UNCOMPLEXWORDSIZE != 0) && (read.isUncomplexWordsize(this->UNCOMPLEXWORDSIZE)))     { return(this->readRead(read)); }

    return(true);
}

bool illuminaReader::close()
{
    if (this->FASTQFILE.isOpen()) {
        if (this->COMPRESSED) { this->COMPRESSOR->close(); }
        this->FASTQFILE.close();
        if (this->FASTQFILE.error() != QFile::NoError) {
            qDebug() << "Error: Cannot read file " << this->FASTQFILE.fileName() << ": " << this->FASTQFILE.errorString();
            return(false);
        }
    }
    return(true);
}
