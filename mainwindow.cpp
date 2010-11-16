#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QWebView>
#include <QWebFrame>
#include <QWebElement>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mWebView = new QWebView;
    ui->webViewTranslation->settings ()->setDefaultTextEncoding ("UTF-8");

    mTranslationUrl.setUrl ("http://multitran.ru/c/m.exe");

    connect (mWebView, SIGNAL(loadProgress(int)),
             this, SLOT(downloadingTranslation(int)));
    connect (mWebView, SIGNAL(loadFinished(bool)),
             this, SLOT(parseTranslatePage(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEditTranslate_returnPressed ()
{
    QString word = ui->lineEditTranslate->text ();
    QString cacheFileName = cachePageName (word);
    if (!QFile::exists (cacheFileName)) {
        qDebug () << "load from web";

        QList<QPair<QString, QString> > query;
        query.append (qMakePair(QString("CL"), QString("1")));
        query.append (qMakePair(QString("s"), ui->lineEditTranslate->text ()));
        query.append (qMakePair(QString("l1"), QString("1")));
        mTranslationUrl.setQueryItems (query);

        QNetworkRequest request;
        request.setUrl (mTranslationUrl);
        request.setRawHeader ("User-Agent", "QMultitran 0.0.1");

        mWebView->load (request);
    } else {
        qDebug () << "load from cache";
        QFileInfo info(cacheFileName);
        mWebView->load (info.absoluteFilePath ());
    }
}

void MainWindow::downloadingTranslation (int progress)
{
    qDebug () << progress;
}

void MainWindow::parseTranslatePage (bool ok)
{
    if (!ok) {
        qDebug () << "Unable to parse page";
        return;
    }

    QString word = ui->lineEditTranslate->text ();
    QString filePath(cachePageName (word));
    if (!QFile::exists (filePath)) {
        const int TranslationTable = 6;

        QWebFrame *frame = mWebView->page ()->mainFrame ();
        QWebElement document = frame->documentElement ();
        QWebElementCollection tables = document.findAll ("table");

        ui->webViewTranslation->setHtml (tables.at (TranslationTable).toOuterXml ());

        QFile cacheFile(filePath);
        if (cacheFile.open (QIODevice::WriteOnly)) {
            QString html = ui->webViewTranslation->page ()->mainFrame ()->toHtml ();
            QTextStream out(&cacheFile);
            out.setCodec ("UTF-8");
            out << html;
        } else {
            qDebug () << QString("Unable to open file %1 for writing").arg (filePath);
        }
    } else {
        QFileInfo info(filePath);
        ui->webViewTranslation->setUrl (info.absoluteFilePath ());
    }
}

QString MainWindow::cachePageName (const QString &word)
{
    return QString("cache/").append (word).append (".html");
}
