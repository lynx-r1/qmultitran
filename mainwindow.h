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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QUrl>

#include "constants.h"

using namespace Constants;

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QStringListModel)
QT_FORWARD_DECLARE_CLASS(QWebView)

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void closeEvent (QCloseEvent *);

private slots:
    void on_lineEditTranslate_returnPressed ();
    void on_comboBoxLanguage_currentIndexChanged(int index);
    void on_toolButtonCancelLoad_clicked();
    void on_pushButtonMultitranLogo_clicked();

    void on_actionCheckUpdate_triggered();
    void on_actionSettings_triggered();
    void on_actionClearCache_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();

    void on_webViewTranslation_urlChanged(const QUrl &url);
    void on_webViewTranslation_linkClicked(const QUrl &url);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void iconMessageClicked();

    void loading(int);
    void parseTranslationPage(bool ok);

private:
    void readSettings();
    void writeSettigns();

    void configWebViewTranslation();

    void fillComboBoxLanguage();

    void createWordDictModel();
    void createCompleter();
    void createTrayIcon();
    void createConnections();
    void createCacheDir();

    bool checkUpdate();
    bool isCacheEmpty();

    QString cachePageName(const QString &filePath);
    QStringList loadDict();

    QString getWordFromPath(const QString &path);
    QString fileNameToWord(const QString &fileName);

    enum StackWidget {
        ProgressBarLoad,
        ComboBoxLanguage
    };

    Ui::MainWindow *ui;

    QWebView *mWebView;

    QSettings *mSettings;
    QStringListModel *mWordDictModel;
    QSystemTrayIcon *mSystemTrayIcon;

    QUrl mTranslationUrl;
    QStringList mWordDictList;
};

#endif // MAINWINDOW_H
