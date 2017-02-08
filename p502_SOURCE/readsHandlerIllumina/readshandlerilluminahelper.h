#ifndef READSHANDLERILLUMINAHELPER_H
#define READSHANDLERILLUMINAHELPER_H

#include <QtCore>
#include <iostream>
#include <cassert>

//! NOTE THAT THE ILLUMINA AND SOLID READS ARE IDENTICAL
// important with solid is that the integer quals are translated to illumina like PHRED+33 quals
// if a -1 quality exists, it is set to 0 - which is a 33/! in dec/ascii PHRED+33 format

/*!
  for a read, there is a minimal information in an ultra small class (char arrays).
  however, in most cases, it is easier and safer to deal with the normal c++ types.
  */


//! the minimal version of a read.
// attention - they hold pointers to char arrays created with new.
// important to destroy them like that


class minimalIlluminaRead
{
public:
    //! attention, these are pointers to arrays created with new
    //! important to destroy them like that
    char* BASES;
    char* QUALS;

    //! constructor and destructor
    minimalIlluminaRead(char* baseArray, char* qualArray) {
        this->BASES = baseArray;
        this->QUALS = qualArray;
    }

    ~minimalIlluminaRead() {
        delete[] this->BASES;
        delete[] this->QUALS;
        this->BASES = 0;
        this->QUALS = 0;
    }
};

//! the full version of a read.
class illuminaRead
{
public:
    QString NAME;
    QString BASES;
    QString QUALS;

public:
    //! constructor related things
    illuminaRead() {
        this->NAME = "";
        this->BASES = "";
        this->QUALS = "";
    }

    illuminaRead(QString& name, QString& bases, QString& quals) {
        this->update(name, bases, quals);
    }

    //~solidRead(){}

    void update(QString& name, QString& bases, QString& quals) {
        this->NAME = name;
        this->BASES = bases;
        this->QUALS = quals;
        assert(this->BASES.length() == this->QUALS.length());
    }

    //! get average quality
    float getAverageQuality() {
        //std::cerr << "note: assuming 33-PHRED quality values" << std::endl << std::flush;
        int len = this->QUALS.length();
        int qualitySum = -33*len; // correct the offset of 33 in the 64-PHRED quality vals
        const QChar* unicodeQuals = this->QUALS.unicode();
        for (int i = 0; i < len; ++i) {
            qualitySum += static_cast<int>((*(unicodeQuals+i)).toAscii());
        }
        float qualityMean = qualitySum / static_cast<float>(len);
        return(qualityMean);
    }

    //! things related to filtering
    // check if a char is in the sequence
    bool hasChar(const QChar& filterChar) {
        return(this->BASES.contains(filterChar));
    }

    // check if it is a low complexity read (uncomplex == if a char occurs in less than 10% or in more than 40% of the cases)
    bool isUncomplexChar(const QChar& unChar) {
        float ratio = this->BASES.count(unChar) / static_cast<float>(this->BASES.length());
        if ((ratio < 0.1) || (ratio > 0.4)) { return(true); }
        return(false);
    }

    // check if it is a low complexity read (uncomplex == if a word occurs too often)
    bool isUncomplexString(const QString& unWord) {
        float ratio = this->BASES.count(unWord) / static_cast<float>((this->BASES.length()-unWord.length()));
        if (ratio > 0.3) { return(true); }
        return(false);
    }

    // check if it is a low complexity read (uncomplex == if a word occurs too often) -> works via the is_uncomplex_string function
    // is true if there is one word within the string that occurs too often
    bool isUncomplexWordsize(const int& wordSize) {
        QString word;
        for (int i = 0; i < (this->BASES.length()-wordSize); ++i) {
            word = this->BASES.mid(i, wordSize);
            if (this->isUncomplexString(word)) { return(true); }
        }
        return(false);
    }

    // check if it is a low quality read
    bool hasLowAverageQuality(const int& cutoff) {
        if (this->getAverageQuality() < cutoff) {return(true);}
        return(false);
    }

    minimalIlluminaRead getMinimalRead() {
        int numChars = this->BASES.length();
        char* baseArray = new char[numChars];
        char* qualArray = new char[numChars];
        const QChar* unicodeBases = this->BASES.unicode();
        const QChar* unicodeQuals = this->QUALS.unicode();
        for (int i = 0; i < numChars; ++i) {
            (*(baseArray+i)) = (*(unicodeBases+i)).toAscii();
            (*(qualArray+i)) = (*(unicodeQuals+i)).toAscii();
        }
        minimalIlluminaRead out(baseArray,qualArray);
        return(out);
    }
};

#endif // READSHANDLERILLUMINAHELPER_H
