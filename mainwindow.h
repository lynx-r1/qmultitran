#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

QT_FORWARD_DECLARE_CLASS(QCompleter)
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

private slots:
    void on_lineEditTranslate_returnPressed ();

    void on_actionClearCache_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();

    void downloadingTranslation(int);
    void parseTranslationPage(bool ok);

private:
    void readConfig();
    void createCacheDir();

    QString cachePageName(const QString &filePath);
    QStringList loadDict();

    Ui::MainWindow *ui;

    QWebView *mWebView;
    QUrl mTranslationUrl;

    QCompleter *mCompleter;
    QStringList mWordDictList;
    QStringListModel *mWordDictModel;

    QString CacheDir;
    QString MultitranUrl;
};

#endif // MAINWINDOW_H
