#include <QtGui/QApplication>

#include <QTextCodec>

#include "mainwindow.h"

#include <QPushButton>
#include <QPropertyAnimation>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    MainWindow w;
    w.show();

    return a.exec();
}
