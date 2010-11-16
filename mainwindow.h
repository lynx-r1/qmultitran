#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

QT_FORWARD_DECLARE_CLASS(QCompleter)
QT_FORWARD_DECLARE_CLASS(QProgressBar)
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

    void downloadingTranslation(int);
    void parseTranslatePage(bool ok);

private:
    QString cachePageName(const QString &filePath);
    QStringList loadDict();

    Ui::MainWindow *ui;

    QWebView *mWebView;
    QUrl mTranslationUrl;
    QProgressBar *mDownloadingProgress;

    QCompleter *mCompleter;
    QStringList mWordDictList;
    QStringListModel *mWordDictModel;
};

#endif // MAINWINDOW_H
