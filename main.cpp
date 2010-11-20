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

#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTranslator>
#include <QtCore/QTextCodec>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    // Set application defaults
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName(APP_AUTHOR);
    app.setOrganizationDomain(APP_URL);

    // Check for saved transelation and use it or use the system default
    QString locale;
    QTranslator translator;
    QSettings settings(APP_NAME, APP_NAME);
    bool bOverrideSystemLocale = settings.value ("Locale/OverrideLocale").toBool ();
    if (bOverrideSystemLocale) {
        locale = settings.value("Locale/Locale", "en_GB").toString();
    } else {
        locale = QLocale::system().name();
    }

    if (translator.load(":/locale/" + locale)) {
        qDebug("%s locale recognized, using translation.", qPrintable(locale));
        app.installTranslator(&translator);
    } else {
        qDebug("%s locale unrecognized, using default (en_GB).", qPrintable(locale));
    }

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QApplication::setQuitOnLastWindowClosed(false);
    } else {
        QMessageBox::critical(0, QObject::tr("QMultitran"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
    }

    MainWindow w;
    w.show();

    return app.exec();
}
