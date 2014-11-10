#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "coordinator.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void runCommand();
    void searchReadsInfile();
    void searchReadsOutfile();
    void searchFastaFile();
    void removeFromQueue(QString workString);

private:
    Ui::Widget *ui;
    coordinator processor;

};

#endif // WIDGET_H
