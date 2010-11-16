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

int MESSAGE_TIMEOUT = 4000;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mWebView = new QWebView;
    ui->webViewTranslation->settings ()->setDefaultTextEncoding ("UTF-8");

    mTranslationUrl.setUrl (MultitranUrl);

    mWordDictList = loadDict ();
    mWordDictModel = new QStringListModel(mWordDictList);

    mCompleter = new QCompleter(mWordDictModel, this);
    mCompleter->setCaseSensitivity (Qt::CaseInsensitive);
    mCompleter->setModelSorting (QCompleter::CaseSensitivelySortedModel);
    mCompleter->setCompletionMode (QCompleter::InlineCompletion);
    ui->lineEditTranslate->setCompleter (mCompleter);

    ui->labelMultitranIcon->setPixmap (QPixmap(":/icons/multitran.png"));

    readConfig ();
    createCacheDir ();

    connect (mWebView, SIGNAL(loadProgress(int)),
             this, SLOT(downloadingTranslation(int)));
    connect (mWebView, SIGNAL(loadFinished(bool)),
             this, SLOT(parseTranslationPage(bool)));
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
        QList<QPair<QString, QString> > query;
        query.append (qMakePair(QString("CL"), QString("1")));
        query.append (qMakePair(QString("s"), ui->lineEditTranslate->text ()));
        query.append (qMakePair(QString("l1"), QString("1")));
        mTranslationUrl.setQueryItems (query);

        statusBar ()->showMessage (tr("Loading from web..."));
        mWebView->load (mTranslationUrl);
    } else {
        QFileInfo info(cacheFileName);
        statusBar ()->showMessage (tr("Loading from cache..."));
        mWebView->load (info.absoluteFilePath ());
    }
}

void MainWindow::on_actionClearCache_triggered ()
{
    QDir d(CacheDir);
    QStringList files = d.entryList (QStringList() << "*.html");
    foreach (QString fileName, files) {
        d.remove (fileName);
    }

    statusBar ()->showMessage (tr("Cache cleaned"), MESSAGE_TIMEOUT);
}

void MainWindow::on_actionAbout_triggered ()
{
    QString text = tr("This is a simple frontend for online version of dictionry Multitran.");
    QMessageBox::about (this, tr("About"), text);
}

void MainWindow::on_actionExit_triggered ()
{
    qApp->quit ();
}

void MainWindow::downloadingTranslation (int progress)
{
    ui->progressBar->setValue (progress);
}

void MainWindow::parseTranslationPage (bool ok)
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

    statusBar ()->showMessage ("Done", MESSAGE_TIMEOUT);
}

void MainWindow::readConfig ()
{
    const QString configPath("qmultitran.conf");

    if (QFile::exists (configPath)) {
        QFile file(configPath);
        if (file.open (QIODevice::ReadOnly | QIODevice::Text)) {
            while (!file.atEnd ()) {
                QByteArray line = file.readLine ();

                QStringList list = QString(line).replace (QRegExp("[ \n\r]+"), "").split ("=");
                if (list[0] == "CACHE_DIR")
                    CacheDir = list[1];
                else if (list[0] == "MULTITRAN_URL")
                    MultitranUrl = list[1];
            }
        } else {
            QMessageBox::critical (this, tr("Error"), tr("Unable to read config file!"));
        }
    } else {
        QMessageBox::critical (this, tr("Error"), tr("There is no config file!"));
    }
}

void MainWindow::createCacheDir ()
{
    if (!QFile::exists (CacheDir)) {
        QDir d = QDir::currentPath ();
        d.mkdir (CacheDir);
    }
}

QString MainWindow::cachePageName (const QString &word)
{
    QString str = QString("%1%2%3.html").arg (CacheDir).arg (QDir::separator ()).arg (word);
    return str;
}

QStringList MainWindow::loadDict ()
{
    QDir d(CacheDir);
    QStringList files = d.entryList (QStringList() << "*.html");
    QStringList dict;
    foreach (QString word, files) {
        word.chop (5);
        dict << word;
    }

    return dict;
}
