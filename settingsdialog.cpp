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

#include <QAbstractButton>
#include <QNetworkProxy>
#include <QSettings>
#include <QSystemTrayIcon>

#include "constants.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

using namespace Constants;

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBoxLocale->addItem (QPixmap(":/icons/flags/united_kingdom.png"),
                                 tr("English"), QString("en_GB"));
    ui->comboBoxLocale->addItem (QPixmap(":/icons/flags/russia.png"),
                                 tr("Russian"), QString("ru_RU"));

    mSettings = new QSettings(APP_NAME, APP_NAME);

    readSettigns ();
}

SettingsDialog::~SettingsDialog()
{
    delete mSettings;
    delete ui;
}

void SettingsDialog::on_buttonBox_clicked (QAbstractButton *button)
{
    switch (ui->buttonBox->buttonRole (button)) {
    case QDialogButtonBox::AcceptRole:
        apply ();
        accept ();
        break;
    case QDialogButtonBox::ApplyRole:
        apply ();
        break;
    case QDialogButtonBox::RejectRole:
        reject ();
        break;
    default:
        ;
    }

    writeSettings ();
}

void SettingsDialog::apply ()
{
    bool bUseProxy = ui->groupBoxUseProxy->isChecked ();
    if (bUseProxy) {
        QString hostName = ui->lineEditHostName->text ();
        int port = ui->spinBoxPort->value ();
        QString userName = ui->lineEditUserName->text ();
        QString password = ui->lineEditPassword->text ();

        QNetworkProxy proxy(QNetworkProxy::HttpProxy, hostName, port);

        bool bUseAuthentication = ui->groupBoxAuthentication->isChecked ();
        if (bUseAuthentication) {
            proxy.setUser (userName);
            proxy.setPassword (password);
        }

        QNetworkProxy::setApplicationProxy (proxy);
    } else {
        QNetworkProxy::setApplicationProxy (QNetworkProxy());
    }

    bool bSystemTray = ui->checkBoxSystemTray->isChecked ();
    QSystemTrayIcon *systray = parentWidget ()->findChild<QSystemTrayIcon *>();
    if (systray) {
        systray->setVisible (bSystemTray);
    }
}

void SettingsDialog::readSettigns ()
{
    mSettings->beginGroup ("Settings");
    ui->tabWidget->setCurrentIndex (mSettings->value ("CurrentTab").toInt ());
    ui->comboBoxLocale->setCurrentIndex (mSettings->value ("LocaleIndex", 0).toInt ());
    mSettings->endGroup ();

    mSettings->beginGroup ("Proxy");
    ui->groupBoxUseProxy->setChecked (mSettings->value ("UseProxy", false).toBool ());
    ui->lineEditHostName->setText (mSettings->value ("ProxyHost", QString("localhost"))
                                   .toString ());
    ui->spinBoxPort->setValue (mSettings->value ("ProxyPort", 3128).toInt ());
    ui->groupBoxAuthentication->setChecked (mSettings->value ("UseAuthentication", false)
                                            .toBool ());
    ui->lineEditUserName->setText (mSettings->value ("UserName").toString ());
    ui->lineEditPassword->setText (mSettings->value ("Password").toString ());
    mSettings->endGroup ();

    mSettings->beginGroup ("Locale");
    ui->groupBoxSystemLocale->setChecked (mSettings->value ("OverrideLocale", false).toBool ());
    mSettings->endGroup ();

    mSettings->beginGroup ("SystemTray");
    ui->checkBoxSystemTray->setChecked (mSettings->value ("ShowSystemTray", true).toBool ());
    mSettings->endGroup ();
}

void SettingsDialog::writeSettings ()
{
    mSettings->beginGroup ("Settings");
    mSettings->setValue ("CurrentTab", ui->tabWidget->currentIndex ());
    mSettings->setValue ("LocaleIndex", ui->comboBoxLocale->currentIndex ());
    mSettings->endGroup ();

    mSettings->beginGroup ("Proxy");
    mSettings->setValue ("UseProxy", ui->groupBoxUseProxy->isChecked ());
    mSettings->setValue ("ProxyHost", ui->lineEditHostName->text ());
    mSettings->setValue ("ProxyPort", ui->spinBoxPort->value ());
    mSettings->setValue ("UseAuthentication", ui->groupBoxAuthentication->isChecked ());
    mSettings->setValue ("UserName", ui->lineEditUserName->text ());
    mSettings->setValue ("Password", ui->lineEditPassword->text ());
    mSettings->endGroup ();

    mSettings->beginGroup ("Locale");
    mSettings->setValue ("OverrideLocale", ui->groupBoxSystemLocale->isChecked ());
    mSettings->setValue ("Locale", ui->comboBoxLocale->itemData (ui->comboBoxLocale->currentIndex ()));
    mSettings->endGroup ();

    mSettings->beginGroup ("SystemTray");
    mSettings->setValue ("ShowSystemTray", ui->checkBoxSystemTray->isChecked ());
    mSettings->endGroup ();
}
