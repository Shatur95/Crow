﻿/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "qonlinetranslator.h"
#include "qonlinetts.h"
#include "langbuttongroup.h"
#include "trayicon.h"

#include <QSettings>
#include <QLocale>
#include <QNetworkProxy>
#include <QTranslator>

class AppSettings : private QSettings
{
    Q_OBJECT

public:
    enum WindowMode {
        PopupWindow,
        MainWindow
    };
    Q_ENUM(WindowMode)
#ifdef Q_OS_WIN
    enum Interval {
        Day,
        Week,
        Month,
        Never
    };
    Q_ENUM(Interval)
#endif

    AppSettings(QObject *parent = nullptr);

    // General settings
    void setupLocale();
    QLocale::Language locale() const;
    void setLocale(QLocale::Language lang);
    void loadLocale(QLocale::Language lang);

    WindowMode windowMode() const;
    void setWindowMode(WindowMode mode);

    bool isTrayIconVisible() const;
    void setTrayIconVisible(bool visible);

    bool isStartMinimized() const;
    void setStartMinimized(bool minimized);

    bool isAutostartEnabled() const;
    void setAutostartEnabled(bool enabled);

#ifdef Q_OS_WIN
    Interval checkForUpdatesInterval() const;
    void setCheckForUpdatesInterval(Interval interval);

    QDate lastUpdateCheckDate() const;
    void setLastUpdateCheckDate(const QDate &date);
#endif

    // Interface settings
    double popupOpacity() const;
    void setPopupOpacity(double opacity);

    int popupHeight() const;
    void setPopupHeight(int height);

    int popupWidth() const;
    void setPopupWidth(int width);

    Qt::ToolButtonStyle popupLanguagesStyle() const;
    void setPopupLanguagesStyle(Qt::ToolButtonStyle style);

    Qt::ToolButtonStyle popupControlsStyle() const;
    void setPopupControlsStyle(Qt::ToolButtonStyle style);

    Qt::ToolButtonStyle windowLanguagesStyle() const;
    void setWindowLanguagesStyle(Qt::ToolButtonStyle style);

    Qt::ToolButtonStyle windowControlsStyle() const;
    void setWindowControlsStyle(Qt::ToolButtonStyle style);

    TrayIcon::IconType trayIconType() const;
    void setTrayIconType(TrayIcon::IconType icon);

    QString customIconPath() const;
    void setCustomIconPath(const QString &path);

    // Translation settings
    bool isSourceTranslitEnabled() const;
    void setSourceTranslitEnabled(bool enable);

    bool isTranslationTranslitEnabled() const;
    void setTranslationTranslitEnabled(bool enable);

    bool isSourceTranscriptionEnabled() const;
    void setSourceTranscriptionEnabled(bool enable);

    bool isTranslationOptionsEnabled() const;
    void setTranslationOptionsEnabled(bool enable);

    bool isExamplesEnabled() const;
    void setExamplesEnabled(bool enable);

    QOnlineTranslator::Language primaryLanguage() const;
    void setPrimaryLanguage(QOnlineTranslator::Language lang);

    QOnlineTranslator::Language secondaryLanguage() const;
    void setSecondaryLanguage(QOnlineTranslator::Language lang);

    // Speech synthesis settings
    QOnlineTts::Voice yandexVoice() const;
    void setYandexVoice(QOnlineTts::Voice voice);

    QOnlineTts::Voice bingVoice() const;
    void setBingVoice(QOnlineTts::Voice voice);

    QOnlineTts::Emotion yandexEmotion() const;
    void setYandexEmotion(QOnlineTts::Emotion emotion);

    // Connection settings
    QNetworkProxy::ProxyType proxyType() const;
    void setProxyType(QNetworkProxy::ProxyType type);

    QString proxyHost() const;
    void setProxyHost(const QString &hostName);

    quint16 proxyPort() const;
    void setProxyPort(quint16 port);

    bool isProxyAuthEnabled() const;
    void setProxyAuthEnabled(bool enabled);

    QString proxyUsername() const;
    void setProxyUsername(const QString &username);

    QString proxyPassword() const;
    void setProxyPassword(const QString &password);

    // Global shortcuts
    QString translateSelectionHotkey() const;
    void setTranslateSelectionHotkey(const QString &hotkey);
    static constexpr const char *defaultTranslateSelectionHotkey()
    { return "Ctrl+Alt+E"; }

    QString playSelectionHotkey() const;
    void setPlaySelectionHotkey(const QString &hotkey);
    static constexpr const char *defaultPlaySelectionHotkey()
    { return "Ctrl+Alt+S"; }

    QString playTranslatedSelectionHotkey() const;
    void setPlayTranslatedSelectionHotkey(const QString &hotkey);
    static constexpr const char *defaultPlayTranslatedSelectionHotkey()
    { return "Ctrl+Alt+F"; }

    QString stopSelectionHotkey() const;
    void setStopSelectionHotkey(const QString &hotkey);
    static constexpr const char *defaultStopSelectionHotkey()
    { return "Ctrl+Alt+G"; }

    QString showMainWindowHotkey() const;
    void setShowMainWindowHotkey(const QString &hotkey);
    static constexpr const char *defaultShowMainWindowHotkey()
    { return "Ctrl+Alt+C"; }

    QString copyTranslatedSelectionHotkey() const;
    void setCopyTranslatedSelectionHotkeyHotkey(const QString &hotkey);
    static constexpr const char *defaultCopyTranslatedSelectionHotkey()
    { return ""; }

    // Window shortcuts
    QString translateHotkey() const;
    void setTranslateHotkey(const QString &hotkey);
    static constexpr const char *defaultTranslateHotkey()
    { return "Ctrl+Return"; }

    QString closeWindowHotkey() const;
    void setCloseWindowHotkey(const QString &hotkey);
    static constexpr const char *defaultCloseWindowHotkey()
    { return "Ctrl+Q"; }

    QString playSourceHotkey() const;
    void setPlaySourceHotkey(const QString &hotkey);
    static constexpr const char *defaultPlaySourceHotkey()
    { return "Ctrl+S"; }

    QString stopSourceHotkey() const;
    void setStopSourceHotkey(const QString &hotkey);
    static constexpr const char *defaultStopSourceHotkey()
    { return "Ctrl+G"; }

    QString playTranslationHotkey() const;
    void setPlayTranslationHotkey(const QString &hotkey);
    static constexpr const char *defaultPlayTranslationHotkey()
    { return "Ctrl+Shift+S"; }

    QString stopTranslationHotkey() const;
    void setStopTranslationHotkey(const QString &hotkey);
    static constexpr const char *defaultStopTranslationHotkey()
    { return "Ctrl+Shift+G"; }

    QString copyTranslationHotkey() const;
    void setCopyTranslationHotkey(const QString &hotkey);
    static constexpr const char *defaultCopyTranslationHotkey()
    { return "Ctrl+Shift+C"; }

    // Buttons
    QOnlineTranslator::Language buttonLanguage(LangButtonGroup::GroupType group, int id) const;
    void setButtonLanguage(LangButtonGroup::GroupType group, int id, QOnlineTranslator::Language lang);

    int checkedButton(LangButtonGroup::GroupType group) const;
    void setCheckedButton(LangButtonGroup::GroupType group, int id);

    // Main window settings
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);

    bool isAutoTranslateEnabled() const;
    void setAutoTranslateEnabled(bool enable);

    QOnlineTranslator::Engine currentEngine() const;
    void setCurrentEngine(QOnlineTranslator::Engine currentEngine);

private:
    static QTranslator m_appTranslator;
#if defined(Q_OS_WIN)
    static QTranslator m_qtTranslator; // Qt library translations
#endif
};

#endif // APPSETTINGS_H
