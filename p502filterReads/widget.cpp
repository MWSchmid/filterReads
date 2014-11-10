#include <QtGui>
#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->findInputPushButton, SIGNAL(clicked()), this, SLOT(searchReadsInfile()));
    connect(ui->findOutputPushButton, SIGNAL(clicked()), this, SLOT(searchReadsOutfile()));
    connect(ui->findFilterPushButton, SIGNAL(clicked()), this, SLOT(searchFastaFile()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(runCommand()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(ui->cancelPushButton, SIGNAL(clicked()), &processor, SLOT(cancelProcessing())); //! TODO test if working
    connect(&processor, SIGNAL(workFinished(QString)), this, SLOT(removeFromQueue(QString)));
}

Widget::~Widget()
{
    // stop the processor and wait for it
    processor.addWork("STOPTHREAD");
    processor.wait();
    delete ui;
}

void Widget::runCommand() {
    QStringList temp;
    QString readsInfile = ui->readsInfileLineEdit->text();
    QString readsOutfile = ui->readsOutfileLineEdit->text();
    QString filterSequences = ui->fastaLineEdit->text();
    QString compressed = "n";
    if (readsInfile.right(3) == ".gz") { compressed = "y"; }
    QString sizing = "n";
    if (ui->sizingCheckBox->isChecked()) { sizing = "y"; }
    QString fixedLength = ui->fixedSizeSpinBox->text();
    QString chopLeft = ui->chopLeftSpinBox->text();
    QString chopRight = ui->chopRightSpinBox->text();
    QString filter = "n";
    if (ui->filterCheckBox->isChecked()) { filter = "y"; }
    QString minQual = ui->minQualSpinBox->text();
    QString skipChar = ui->skipCharLineEdit->text();
    QString uncomplexChar = ui->uncomplexCharLineEdit->text();
    QString uncomplexString = ui->uncomplexStringLineEdit->text();
    QString uncomplexWordsize = ui->uncomplexWordSizeSpinBox->text();
    temp << readsInfile << readsOutfile << filterSequences << compressed << sizing << fixedLength << chopLeft << chopRight << filter << minQual << skipChar << uncomplexChar << uncomplexString << uncomplexWordsize;
    std::cerr << "running with following parameters:" << std::endl << temp.join("\n").toStdString() << std::endl << std::flush;
    QString command = temp.join("|");
    this->ui->queueListWidget->addItem(command);
    ui->cancelPushButton->setEnabled(true);
    std::cerr << QTime::currentTime().toString().toStdString() << std::endl << std::flush;
    processor.addWork(command);
}

void Widget::searchReadsInfile() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("select a file with reads"),
                                                    ".",
                                                    tr("(compressed) illumina fastq (*.fastq *.gz)"));
    if (!fileName.isEmpty()) { ui->readsInfileLineEdit->setText(fileName); }
}

void Widget::searchReadsOutfile() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("select a file with reads"),
                                                    ".",
                                                    tr("(compressed) illumina fastq (*.fastq *.gz)"));
    if (!fileName.isEmpty()) { ui->readsOutfileLineEdit->setText(fileName); }

}

void Widget::searchFastaFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("select a file with fasta sequences you would like to remove"),
                                                    ".",
                                                    tr("fasta (*.fasta *.fas *.fa)"));
    if (!fileName.isEmpty()) { ui->fastaLineEdit->setText(fileName); }
}

void Widget::removeFromQueue(QString workString) {
    QList<QListWidgetItem *> items = this->ui->queueListWidget->findItems(workString, Qt::MatchExactly);
    foreach (QListWidgetItem * item, items) { delete item; }
    if (this->ui->queueListWidget->count() == 0) { ui->cancelPushButton->setDisabled(true); }
    std::cerr << QTime::currentTime().toString().toStdString() << std::endl << std::flush;
}

