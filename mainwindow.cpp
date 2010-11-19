/* Copyright (C) 2010 The QMultitran.
 *
 * This file is part of QMultitran.
 *
 * QMultitran is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QMultitran is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QMultitran.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <QCompleter>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <QMenu>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QProgressBar>
#include <QStringListModel>
#include <QSettings>
#include <QSystemTrayIcon>

#include <QWebElement>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebView>

#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"

int MESSAGE_TIMEOUT = 3000;

const QString CACHE_DIR = "cache";
const QString MULTITRAN_URL = "http://multitran.ru";
const QString MULTITRAN_EXE = "http://multitran.ru/c/m.exe";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mWebView = new QWebView;
    mTranslationUrl.setUrl (MULTITRAN_EXE);
    mSettings = new QSettings(APP_NAME, APP_NAME);

    configWebViewTranslation ();

    createWordDictModel ();
    createCompleter ();
    createTrayIcon ();

    createCacheDir ();

    createConnections ();
    readSettings ();
}

MainWindow::~MainWindow()
{
    writeSettigns ();

    delete mSettings;
    delete mWebView;
    delete ui;
}

// ----------------------------- public slots ------------------------------- //
void MainWindow::closeEvent (QCloseEvent *e)
{
    if (mSystemTrayIcon->isVisible ()) {
        hide ();
        e->ignore ();
    } else {
        QMainWindow::closeEvent (e);
        qApp->exit ();
    }
}

// --------------------------- private slots -------------------------------- //
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

        QNetworkProxy proxy = QNetworkProxy::applicationProxy ();
        if (!proxy.hostName ().isEmpty ()) {
            statusBar ()->showMessage (tr("Loading from web via proxy..."));
        } else {
            statusBar ()->showMessage (tr("Loading from web..."));
        }
        mWebView->load (mTranslationUrl);
    } else {
        QFileInfo info(cacheFileName);
        statusBar ()->showMessage (tr("Loading from cache..."));
        mWebView->load (info.absoluteFilePath ());
    }
}

void MainWindow::on_actionCheckUpdate_triggered ()
{
    if (checkUpdate ()) {
        mSystemTrayIcon->showMessage (tr("Update"), tr("Available new version."));
    }
}

void MainWindow::on_actionSettings_triggered ()
{
    SettingsDialog settings(this);
    settings.exec ();
}

void MainWindow::on_actionClearCache_triggered ()
{
    QMessageBox::StandardButton a = QMessageBox::question (this, tr("Warning"),
                                                           tr("Are you really want to delete "
                                                              "all localy saved words?"),
                                                           QMessageBox::Yes | QMessageBox::No,
                                                           QMessageBox::No);
    if (a == QMessageBox::Yes) {
        QDir d(CACHE_DIR);
        QStringList files = d.entryList (QStringList() << "*.html");
        foreach (QString fileName, files) {
            d.remove (fileName);
        }

        ui->actionBack->setEnabled (false);
        ui->actionForward->setEnabled (false);

        statusBar ()->showMessage (tr("Cache cleaned"), MESSAGE_TIMEOUT);
    } else {
        statusBar ()->showMessage (tr("Canceled"), MESSAGE_TIMEOUT);
    }
}

void MainWindow::on_actionAbout_triggered ()
{
    QString text = tr("QMultitran is a simple frontend for online version "
                      "of the dictionary Multitran (<a href='http://multitran.ru'>"
                      "http://multitran.ru</a>).");
    QMessageBox::about (this, tr("About"), text);
}

void MainWindow::on_actionExit_triggered ()
{
    qApp->quit ();
}

void MainWindow::on_webViewTranslation_urlChanged (const QUrl &url)
{
    QWebHistory *history = ui->webViewTranslation->history ();

    int iMaxItems = 100;
    bool bAllowBack = (history->backItems (iMaxItems).count() != 0);
    ui->actionBack->setEnabled (bAllowBack);

    bool bAllowForward = (history->forwardItems (iMaxItems).count() != 0);
    ui->actionForward->setEnabled (bAllowForward);

    QString filePath = url.path ();

    if (filePath != "blank") {
        QString word = getWordFromPath (filePath);
        ui->lineEditTranslate->setText (word);
    }
}

void MainWindow::on_webViewTranslation_linkClicked (const QUrl &url)
{
    QString clickedLink = url.toString ();
    bool bTranslation = clickedLink.contains (MULTITRAN_EXE + "?t=");

    if (bTranslation) {
        QWebFrame *frame = ui->webViewTranslation->page ()->mainFrame ();
        QWebElement document = frame->documentElement ();
        QWebElementCollection links = document.findAll ("a");

        foreach (QWebElement a, links) {
            QString href = a.attribute ("href");
            if (href == clickedLink) {
                QString word = a.toInnerXml ();
                ui->lineEditTranslate->setText (word);
                on_lineEditTranslate_returnPressed ();
            }
        }
    } else {
        QDesktopServices::openUrl (url);
    }
}

void MainWindow::loading (int progress)
{
    ui->progressBar->setValue (progress);
}

void MainWindow::iconActivated (QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        setVisible (!isVisible ());
        break;
    default:
        ;
    }
}

void MainWindow::iconMessageClicked ()
{
    QMessageBox::information (this, "hi", "hoy");
}

void MainWindow::parseTranslationPage (bool ok)
{
    if (!ok) {
        QMessageBox::information (this, tr("Error"), tr("Unable to load page"));
        statusBar ()->clearMessage ();
        return;
    }

    QString word = ui->lineEditTranslate->text ();
    QString filePath(cachePageName (word));
    if (!QFile::exists (filePath)) {
        const int TranslationTable = 6;

        QWebFrame *frame = mWebView->page ()->mainFrame ();
        QWebElement document = frame->documentElement ();
        QWebElementCollection tables = document.findAll ("table");

        QWebElement translateTable = tables.at (TranslationTable);
        QWebElementCollection imgs = translateTable.findAll ("img");
        foreach (QWebElement i, imgs) {
            QString imgVal = i.attribute ("src");
            if (!imgVal.isEmpty ()) {
                i.setAttribute ("src", QString("%1/%2").arg (MULTITRAN_URL)
                                .arg (imgVal));
            }
        }

        QString tableXml = translateTable.toOuterXml ();
        tableXml.replace ("m.exe", MULTITRAN_EXE);

        QFile cacheFile(filePath);
        if (cacheFile.open (QIODevice::WriteOnly)) {
            QString html = QString("<html><head></head><body>%1"
                                   "</body></html>").arg (tableXml);
            QTextStream out(&cacheFile);
            out.setCodec ("UTF-8");
            out << html;

            mWordDictList.append (word);
            mWordDictModel->setStringList (mWordDictList);
            statusBar ()->showMessage (tr("Word saved in cache"));
        } else {
            QMessageBox::information (this, tr("Error"), tr("Unable to open file %1 for writing")
                                      .arg (filePath));
            return;
        }
    }

    QFileInfo info(filePath);
    ui->webViewTranslation->setUrl (info.absoluteFilePath ());

    statusBar ()->showMessage ("Done", MESSAGE_TIMEOUT);
}

// ------------------------------ private ---------------------------------- //
void MainWindow::readSettings ()
{
    mSettings->beginGroup ("Proxy");
    bool bUseProxy = mSettings->value ("UseProxy", false).toBool ();

    if (bUseProxy) {
        QString hostName = mSettings->value ("ProxyHost").toString ();
        int port = mSettings->value ("ProxyPort").toInt ();
        QString userName = mSettings->value ("UserName").toString ();
        QString password = mSettings->value ("Password").toString ();

        QNetworkProxy proxy(QNetworkProxy::HttpProxy, hostName, port);

        bool bUseAuthentication = mSettings->value ("UseAuthentication", false).toBool ();
        if (bUseAuthentication) {
            proxy.setUser (userName);
            proxy.setPassword (password);
        }

        QNetworkProxy::setApplicationProxy(proxy);
    }
    mSettings->endGroup ();

    setGeometry (mSettings->value ("QMultitran/Geometry", QRect(0, 0, 570, 380)).toRect ());
    mSystemTrayIcon->setVisible (mSettings->value ("SystemTray/ShowSystemTray", true).toBool ());
}

void MainWindow::writeSettigns ()
{
    mSettings->setValue ("QMultitran/Geometry", geometry ());
}

void MainWindow::configWebViewTranslation ()
{
    ui->webViewTranslation->settings ()->setDefaultTextEncoding ("UTF-8");
    ui->webViewTranslation->settings ()->setAttribute (QWebSettings::JavascriptEnabled, false);
    ui->webViewTranslation->page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
}

void MainWindow::createWordDictModel ()
{
    mWordDictList = loadDict ();
    mWordDictModel = new QStringListModel(mWordDictList);
}

void MainWindow::createCompleter ()
{
    QCompleter *completer = new QCompleter(mWordDictModel, this);
    completer->setCaseSensitivity (Qt::CaseInsensitive);
    completer->setModelSorting (QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionMode (QCompleter::InlineCompletion);
    ui->lineEditTranslate->setCompleter (completer);
}

void MainWindow::createTrayIcon ()
{
    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(ui->actionCheckUpdate);
    trayIconMenu->addAction(ui->actionAbout);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionExit);

    mSystemTrayIcon = new QSystemTrayIcon(QPixmap(APP_ICON), this);
    mSystemTrayIcon->setContextMenu(trayIconMenu);
}

void MainWindow::createConnections ()
{
    connect (mWebView, SIGNAL(loadProgress(int)),
             this, SLOT(loading(int)));
    connect (mWebView, SIGNAL(loadFinished(bool)),
             this, SLOT(parseTranslationPage(bool)));

    connect (mSystemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect (mSystemTrayIcon, SIGNAL(messageClicked()),
             this, SLOT(iconMessageClicked()));

    connect (ui->actionForward, SIGNAL(triggered()),
             ui->webViewTranslation, SLOT(forward()));
    connect (ui->actionBack, SIGNAL(triggered()),
             ui->webViewTranslation, SLOT(back()));
}

void MainWindow::createCacheDir ()
{
    if (!QFile::exists (CACHE_DIR)) {
        QDir d = QDir::currentPath ();
        d.mkdir (CACHE_DIR);
    }
}

bool MainWindow::checkUpdate ()
{
    return true;
}

QString MainWindow::cachePageName (const QString &word)
{
    QString str = QString("%1%2%3.html").arg (CACHE_DIR)
                  .arg (QDir::separator ()).arg (word);
    return str;
}

QStringList MainWindow::loadDict ()
{
    QDir d(CACHE_DIR);
    QStringList files = d.entryList (QStringList() << "*.html");
    QStringList dict;
    foreach (QString file, files) {
        dict << fileNameToWord (file);
    }

    return dict;
}

QString MainWindow::getWordFromPath (const QString &path)
{
    QFileInfo info(path);
    QString fileName = info.fileName ();
    QString word = fileNameToWord (fileName);

    return word;
}

QString MainWindow::fileNameToWord (const QString &fileName)
{
    QString fn = fileName;
    fn.chop (5);
    return fn;
}
