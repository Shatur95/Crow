/*
 *  Copyright © 2018 Gennady Chernyshchuk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTranslator>
#include <QShortcut>
#include <QTimer>
#include <QButtonGroup>

#include "qhotkey.h"
#include "qonlinetranslator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void translationTextChanged(const QString &text);
    void sourceButtonChanged(QAbstractButton *button, const int &id);
    void translationButtonChanged(QAbstractButton *button, const int &id);

private slots:
    void on_translateButton_clicked();
    void on_swapButton_clicked();
    void on_settingsButton_clicked();
    void on_sourceSayButton_clicked();
    void on_translationSayButton_clicked();
    void on_sourceCopyButton_clicked();
    void on_translationCopyButton_clicked();
    void on_translationCopyAllButton_clicked();

    void on_sourceAutoButton_triggered(QAction *language);
    void on_translationAutoButton_triggered(QAction *language);

    void on_sourceButtonGroup_buttonToggled(QAbstractButton *button, const bool &checked);
    void on_translationButtonGroup_buttonToggled(QAbstractButton *button, const bool &checked);

    void on_translateSelectedHotkey_activated();
    void on_saySelectedHotkey_activated();
    void on_showMainWindowHotkey_activated();

    void on_tray_activated(QSystemTrayIcon::ActivationReason reason);
    void on_autoTranslateCheckBox_toggled(const bool &state);

    void reloadTranslation();
    void loadProxy();
    void resetAutoSourceButtonText();

private:
    void loadSettings();

    // Language button groups
    void loadLanguageButtons(QButtonGroup *group, const QString &settingsName);
    void insertLanguage(QButtonGroup *group, const QString &settingsName, const QString &languageCode);
    void checkSourceButton(const int &id, const bool &checked);
    void checkTranslationButton(const int &id, const bool &checked);
    QList<QAction *> languagesList();
    QString selectedText();

    Ui::MainWindow *ui;
    QTranslator translator;
    QTimer autoTranslateTimer;
    QOnlineTranslator m_translationData;
    QMenu *languagesMenu;

    // System tray
    QMenu *trayMenu;
    QSystemTrayIcon *trayIcon;

    // Window shortcuts
    QShortcut *closeWindowsShortcut;

    // Global shortcuts
    QHotkey *translateSelectedHotkey;
    QHotkey *saySelectedHotkey;
    QHotkey *showMainWindowHotkey;

    // Language button groups
    QButtonGroup *sourceButtonGroup;
    QButtonGroup *translationButtonGroup;
};

#endif // MAINWINDOW_H
