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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    toggleProxyGui (false);

    QSettings s;
    ui->checkBoxUseProxy->setChecked (s.value ("/QMultitran/UseProxy", false).toBool ());
    ui->lineEditHostName->setText (s.value ("/QMultitran/ProxyHost", QString("localhost"))
                                   .toString ());
    ui->spinBoxPort->setValue (s.value ("/QMultitran/ProxyPort", 3128).toInt ());
    ui->groupBoxAuthentication->setChecked (s.value ("/QMultitran/UseAuthentication", false)
                                            .toBool ());
    ui->lineEditUserName->setText (s.value ("/QMultitran/UserName").toString ());
    ui->lineEditPassword->setText (s.value ("/QMultitran/Password").toString ());
}

SettingsDialog::~SettingsDialog()
{
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
        reject ();
    }
}

void SettingsDialog::on_checkBoxUseProxy_stateChanged (int state)
{
    switch (state) {
    case Qt::Unchecked:
        toggleProxyGui (false);
        break;
    case Qt::Checked:
        toggleProxyGui (true);
        break;
    }
}

void SettingsDialog::apply ()
{
    bool bUseProxy = ui->checkBoxUseProxy->isChecked ();
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

        QNetworkProxy::setApplicationProxy(proxy);

        QSettings s;
        s.setValue ("/QMultitran/ProxyHost", hostName);
        s.setValue ("/QMultitran/ProxyPort", port);
        s.setValue ("/QMultitran/UseAuthentication", bUseAuthentication);
        s.setValue ("/QMultitran/UserName", userName);
        s.setValue ("/QMultitran/Password", password);
    } else {
        QNetworkProxy::setApplicationProxy (QNetworkProxy());
    }

    QSettings s;
    s.setValue ("/QMultitran/UseProxy", bUseProxy);
}

void SettingsDialog::toggleProxyGui (bool toggle)
{
    ui->lineEditHostName->setEnabled (toggle);
    ui->spinBoxPort->setEnabled (toggle);
    ui->groupBoxAuthentication->setEnabled (toggle);
}
