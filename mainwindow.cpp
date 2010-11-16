#include <QDebug>

#include <QCompleter>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>

#include <QMessageBox>

#include <QNetworkReply>
#include <QNetworkRequest>

#include <QProgressBar>
#include <QStringListModel>

#include <QWebView>
#include <QWebFrame>
#include <QWebElement>

#include "mainwindow.h"
#include "ui_mainwindow.h"

QString CACHE_DIR = QString("cache");
QString MULTITRAN_URL = QString("http://multitran.ru/c/m.exe");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mWebView = new QWebView;
    ui->webViewTranslation->settings ()->setDefaultTextEncoding ("UTF-8");

    mTranslationUrl.setUrl (MULTITRAN_URL);

    mDownloadingProgress = new QProgressBar;
    mDownloadingProgress->setValue (0);
    statusBar ()->addPermanentWidget (mDownloadingProgress);

    mWordDictList = loadDict ();
    mWordDictModel = new QStringListModel(mWordDictList);

    mCompleter = new QCompleter(mWordDictModel, this);
    mCompleter->setCaseSensitivity (Qt::CaseInsensitive);
    mCompleter->setModelSorting (QCompleter::CaseSensitivelySortedModel);
    mCompleter->setCompletionMode (QCompleter::InlineCompletion);
    ui->lineEditTranslate->setCompleter (mCompleter);

    connect (mWebView, SIGNAL(loadProgress(int)),
             this, SLOT(downloadingTranslation(int)));
    connect (mWebView, SIGNAL(loadFinished(bool)),
             this, SLOT(parseTranslatePage(bool)));
}

MainWindow::~MainWindow()
{
    delete mWebView;
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

        mWebView->load (mTranslationUrl);
    } else {
        qDebug () << "load from cache";
        QFileInfo info(cacheFileName);
        mWebView->load (info.absoluteFilePath ());
    }
}

void MainWindow::downloadingTranslation (int progress)
{
    qDebug () << progress;
    mDownloadingProgress->setValue (progress);
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

            mWordDictList.append (word);
            mWordDictModel->setStringList (mWordDictList);
            qDebug () << "Word saved in cache";
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
    QString str = QString("%1/%2.html").arg (CACHE_DIR).arg (word);
    return str;
}

QStringList MainWindow::loadDict ()
{
    QDir d(CACHE_DIR);
    QStringList files = d.entryList (QStringList() << "*.html");
    QStringList dict;
    foreach (QString word, files) {
        word.chop (5);
        dict << word;
    }

    return dict;
}
