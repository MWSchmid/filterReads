#include <QtGui/QApplication>
#include <iostream>
#include "widget.h"
#include "consoleRunner.h"
#include <getopt.h>

void usage() {
    std::cerr << std::endl << "Usage is ./programname [-c infile,outfile,fastaFile,compressedInput,compressedOutput,readlength]" << std::endl << std::endl <<
                 "infile must be a sorted bam file, outfile is bam." << std::endl <<
                 "fastaFile is the file containing the sequences you would like to filter" << std::endl <<
                 "compressedInput/Output: tells if the infile and outfile are gzip compressed (y or n)" << std::endl <<
                 "THIS ABOVE ARE THE MINIMUM REQUIREMENTS - HERE IS A LIST OF ALL OPTIONS AVAILABLE:" << std::endl << std::endl <<
                 "sizing: chop reads according to chopLeft and chopRight (y or n)" << std::endl <<
                 "readlength: chop all reads to this length (int)" << std::endl << std::endl <<
                 "chopLeft: cut <int> bp off the 5' end" << std::endl <<
                 "chopRight: cut <int> bp off the 3' end" << std::endl <<  std::endl <<
                 "filter: filter reads accoring to one or more of the next five arguments (y or n)" << std::endl <<
                 "minimal average quality: <float>" << std::endl <<
                 "skip reads with this char: <char>" << std::endl <<
                 "uncomplex: <char> remove reads where this character occurs in less than 10 % or more than 40 % of the bases" << std::endl <<
                 "uncomplex: <string> remove reads where this string makes more than 30 % of the read" << std::endl <<
                 "uncomplex: <int> like above, but checking for each k-mer of the read withing the read" << std::endl << std::endl <<
                 "if -c <arguments> above are given, the application runs on console, otherwise gui." << std::endl <<
                 "NOTE THAT THERE IS ABSOLUTELY NO CHECK FOR THE CORRECTNESS OF THE ARGUMENTS!" << std::endl << std::endl << std::flush;
    exit(8);
}

int c;
extern char *optarg;
extern int optind, optopt, opterr;

int main(int argc, char *argv[])
{
    // check command line
    bool commandLine = false;
    QString command = "";


    while ((c = getopt(argc, argv, "c:")) != -1) {
        switch(c) {
        case 'c':
            commandLine = true;
            command = optarg;
            break;
        case ':':
            std::cerr << "some stuff not specified" << std::endl << std::flush;
            break;
        case '?':
            std::cerr << "unknown argument" << std::endl << std::flush;
            usage();
        }
    }

    //command = "/media/DataDisk3/SHORT_READS_ATLAS/SRX111836.fastq.gz,/media/DataDisk3/SHORT_READS_ATLAS/SRX111836_filtered.fastq.gz,/media/DataDisk1/GFF3files/trashySequences.fasta,y,y";
    //command = "/media/DataDisk3/SHORT_READS_ATLAS/SRX336073.fastq.gz,/media/DataDisk3/SHORT_READS_ATLAS/SRX336073_filtered.fastq.gz,/media/DataDisk1/GFF3files/trashySequences.fasta,y,y";

    //! RUNNING ON COMMANDLINE
    if (commandLine)
    {
        QCoreApplication a(argc, argv);
        // initialize the runner (will do everything)
        consoleRunner runner(command);
        QObject::connect(&runner, SIGNAL(finished()), &a, SLOT(quit()), Qt::DirectConnection);
        runner.startAnalysis();
        runner.start();
        return a.exec();
    }
    //! RUNNING IN GUI
    else
    {
        QApplication a(argc, argv);
        Widget w;
        w.show();
        return a.exec();
    }
}
/* DOES NOT WORK:
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args = QApplication::arguments();
    QString command = "";
    if (args.length() > 1) {
        if (args.at(1) != "-c") { usage(); }
        command = args.at(2);
        command = "/media/DataDisk3/SHORT_READS_ATLAS/SRX111836.fastq.gz,/media/DataDisk3/SHORT_READS_ATLAS/SRX111836_filtered.fastq.gz,/media/DataDisk1/GFF3files/trashySequences.fasta,y,100";
        consoleRunner runner(command);
        QObject::connect(&runner, SIGNAL(finished()), &app, SLOT(quit()), Qt::DirectConnection);
        runner.startAnalysis();
        runner.start();
        //! NOTE THAT THERE ARE SOME ISSUES WITH THE THREADS - the reader finished whenever he likes to - and the filter could be faster...
        runner.wait(); //! WITHOUT THIS WAIT, THE COORDINATOR IS DESTROYED VERY QUICK
    } else {
        Widget w;
        w.show(); //! TODO
    }
    return app.exec();
}
*/

/* TESTING THE COMPRESSOR - WORKS - gives the same md5sum
#include "../p502_SOURCE/readsHandlerIllumina/readshandlerilluminahelper.h"
#include "../p502_SOURCE/readsHandlerIllumina/illuminareader.h"
#include "../p502_SOURCE/readsHandlerIllumina/illuminawriter.h"

int main(int argc, char *argv[])
{
    QString infile = "/media/DataDisk3/SHORT_READS_ATLAS/ERR229827.fastq.gz";
    QString outfile = "/media/DataDisk3/SHORT_READS_ATLAS/ERR229827_copy.fastq.gz";

    illuminaReader reader(52, 0, 0, 0, QChar(), QChar(), QString(), 0, true);
    illuminaWriter writer(52, 0, 0, 0, QChar(), QChar(), QString(), 0, true);
    illuminaRead read;
    reader.open(infile, false, true);
    writer.open(outfile, false, true);
    while (reader.readRead(read)) {
        writer.writeRead(read);
    }
    reader.close();
    writer.close();
    return 0;

}
*/
