/*
 *  Copyright © 2018-2020 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "addlanguagedialog.h"
#include "popupwindow.h"
#include "qhotkey.h"
#include "qtaskbarcontrol.h"
#include "selection.h"
#include "singleapplication.h"
#include "speakbuttons.h"
#include "settings/appsettings.h"
#include "settings/settingsdialog.h"
#include "transitions/languagedetectedtransition.h"
#include "transitions/retranslationtransition.h"
#include "transitions/textemptytransition.h"
#include "transitions/translatorabortedtransition.h"
#include "transitions/translatorerrortransition.h"
#ifdef Q_OS_WIN
#include "qgittag.h"
#include "updaterdialog.h"
#endif

#include <QClipboard>
#include <QFinalState>
#include <QMediaPlaylist>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QShortcut>
#include <QStateMachine>

MainWindow::MainWindow(const AppSettings &settings, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_translateSelectionHotkey(new QHotkey(this))
    , m_speakSelectionHotkey(new QHotkey(this))
    , m_speakTranslatedSelectionHotkey(new QHotkey(this))
    , m_stopSpeakingHotkey(new QHotkey(this))
    , m_showMainWindowHotkey(new QHotkey(this))
    , m_copyTranslatedSelectionHotkey(new QHotkey(this))
    , m_closeWindowsShortcut(new QShortcut(this))
    , m_stateMachine(new QStateMachine(this))
    , m_translator(new QOnlineTranslator(this))
    , m_trayMenu(new QMenu(this))
    , m_trayIcon(new TrayIcon(this))
    , m_taskbar(new QTaskbarControl(this))
{
    ui->setupUi(this);

    // Show a message that the application is already running
    connect(qobject_cast<SingleApplication *>(SingleApplication::instance()), &SingleApplication::instanceStarted, this, &MainWindow::showAppRunningMessage);

    // Selection requests
    connect(Selection::instance(), &Selection::requestedSelectionAvailable, this, &MainWindow::setSourceText, Qt::DirectConnection);

    // Text speaking
    ui->sourceSpeakButtons->setMediaPlayer(new QMediaPlayer);
    ui->translationSpeakButtons->setMediaPlayer(new QMediaPlayer);

    // Taskbar progress for text speaking
#if defined(Q_OS_WIN)
    m_taskbar->setWidget(this);
#endif
    connect(ui->sourceSpeakButtons, &SpeakButtons::stateChanged, this, &MainWindow::setTaskbarState);
    connect(ui->translationSpeakButtons, &SpeakButtons::stateChanged, this, &MainWindow::setTaskbarState);
    connect(ui->sourceSpeakButtons, &SpeakButtons::positionChanged, m_taskbar, &QTaskbarControl::setProgress);
    connect(ui->translationSpeakButtons, &SpeakButtons::positionChanged, m_taskbar, &QTaskbarControl::setProgress);

    // Shortcuts
    connect(m_closeWindowsShortcut, &QShortcut::activated, this, &MainWindow::close);
    connect(m_showMainWindowHotkey, &QHotkey::activated, this, &MainWindow::open);
    connect(m_translateSelectionHotkey, &QHotkey::activated, this, &MainWindow::translateSelection);
    connect(m_speakSelectionHotkey, &QHotkey::activated, this, &MainWindow::speakSelection);
    connect(m_speakTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::speakTranslatedSelection);
    connect(m_stopSpeakingHotkey, &QHotkey::activated, this, &MainWindow::stopSpeaking);
    connect(m_copyTranslatedSelectionHotkey, &QHotkey::activated, this, &MainWindow::copyTranslatedSelection);

    // Source and translation logic
    connect(ui->sourceLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->translationLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->sourceEdit, &SourceTextEdit::textChanged, this, &MainWindow::resetAutoSourceButtonText);

    // System tray icon
    m_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("window")), tr("Show window"), this, &MainWindow::open);
    m_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("dialog-object-properties")), tr("Settings"), this, &MainWindow::openSettings);
    m_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), tr("Exit"), QCoreApplication::instance(), &QCoreApplication::quit, Qt::QueuedConnection);
    m_trayIcon->setContextMenu(m_trayMenu);

    // State machine to handle translator signals async
    buildStateMachine();
    m_stateMachine->start();

    // App settings
    loadSettings(settings);

    // Load main window settings (these settings are loaded only at startup and cannot be configured in the settings dialog)
    ui->autoTranslateCheckBox->setChecked(settings.isAutoTranslateEnabled());
    ui->engineComboBox->setCurrentIndex(settings.currentEngine());
    ui->sourceLanguagesWidget->setLanguages(settings.languages(AppSettings::Source));
    ui->translationLanguagesWidget->setLanguages(settings.languages(AppSettings::Translation));
    ui->translationLanguagesWidget->checkButton(settings.checkedButton(AppSettings::Translation));
    ui->sourceLanguagesWidget->checkButton(settings.checkedButton(AppSettings::Source));

    restoreGeometry(settings.mainWindowGeometry());
    if (!settings.isStartMinimized())
        show();

#ifdef Q_OS_WIN
    // Check date for updates
    const AppSettings::Interval updateInterval = settings.checkForUpdatesInterval();
    QDate checkDate = settings.lastUpdateCheckDate();
    switch (updateInterval) {
    case AppSettings::Day:
        checkDate = checkDate.addDays(1);
        break;
    case AppSettings::Week:
        checkDate = checkDate.addDays(7);
        break;
    case AppSettings::Month:
        checkDate = checkDate.addMonths(1);
        break;
    case AppSettings::Never:
        return;
    }

    if (QDate::currentDate() >= checkDate) {
        auto *release = new QGitTag(this);
        connect(release, &QGitTag::finished, this, &MainWindow::checkForUpdates);
        release->get("crow-translate", "crow-translate");
    }
#endif
}

MainWindow::~MainWindow()
{
    AppSettings settings;
    settings.setMainWindowGeometry(saveGeometry());
    settings.setAutoTranslateEnabled(ui->autoTranslateCheckBox->isChecked());
    settings.setCurrentEngine(currentEngine());
    settings.setLanguages(AppSettings::Source, ui->sourceLanguagesWidget->languages());
    settings.setLanguages(AppSettings::Translation, ui->translationLanguagesWidget->languages());
    settings.setCheckedButton(AppSettings::Source, ui->sourceLanguagesWidget->checkedId());
    settings.setCheckedButton(AppSettings::Translation, ui->translationLanguagesWidget->checkedId());

    delete ui;
}

const QComboBox *MainWindow::engineCombobox() const
{
    return ui->engineComboBox;
}

const TranslationEdit *MainWindow::translationEdit() const
{
    return ui->translationEdit;
}

const QToolButton *MainWindow::swapButton() const
{
    return ui->swapButton;
}

const QToolButton *MainWindow::copySourceButton() const
{
    return ui->copySourceButton;
}

const QToolButton *MainWindow::copyTranslationButton() const
{
    return ui->copyTranslationButton;
}

const QToolButton *MainWindow::copyAllTranslationButton() const
{
    return ui->copyAllTranslationButton;
}

const LanguageButtonsWidget *MainWindow::sourceLanguageButtons() const
{
    return ui->sourceLanguagesWidget;
}

const LanguageButtonsWidget *MainWindow::translationLanguageButtons() const
{
    return ui->translationLanguagesWidget;
}

const SpeakButtons *MainWindow::sourceSpeakButtons() const
{
    return ui->sourceSpeakButtons;
}

const SpeakButtons *MainWindow::translationSpeakButtons() const
{
    return ui->translationSpeakButtons;
}

const QShortcut *MainWindow::closeWindowShortcut() const
{
    return m_closeWindowsShortcut;
}

AppSettings::LanguageFormat MainWindow::popupLanguageFormat() const
{
    return m_popupLanguageFormat;
}

QSize MainWindow::popupSize() const
{
    return m_popupSize;
}

double MainWindow::popupOpacity() const
{
    return m_popupOpacity;
}

void MainWindow::open()
{
    ui->sourceEdit->setFocus();

    setWindowState(windowState() & ~Qt::WindowMinimized);
    show();
    activateWindow();
}

void MainWindow::translateSelection()
{
    emit translateSelectionRequested();
}

void MainWindow::speakSelection()
{
    emit speakSelectionRequested();
}

void MainWindow::speakTranslatedSelection()
{
    emit speakTranslatedSelectionRequested();
}

void MainWindow::stopSpeaking()
{
    ui->sourceSpeakButtons->stopSpeaking();
    ui->translationSpeakButtons->stopSpeaking();
}

void MainWindow::copyTranslatedSelection()
{
    emit copyTranslatedSelectionRequested();
}

void MainWindow::clearText()
{
    // Clear source text without tracking for changes
    ui->sourceEdit->setRequestTranlationOnEdit(false);
    ui->sourceEdit->clear();
    if (ui->autoTranslateCheckBox->isChecked())
        ui->sourceEdit->setRequestTranlationOnEdit(true);

    clearTranslation();
}

void MainWindow::abortTranslation()
{
    m_translator->abort();
}

void MainWindow::swapLanguages()
{
    // Temporary disable toggle logic
    disconnect(ui->translationLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    disconnect(ui->sourceLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);

    LanguageButtonsWidget::swapCurrentLanguages(ui->sourceLanguagesWidget, ui->translationLanguagesWidget);

    // Re-enable toggle logic
    connect(ui->translationLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);
    connect(ui->sourceLanguagesWidget, &LanguageButtonsWidget::buttonChecked, this, &MainWindow::checkLanguageButton);

    // Copy translation to source text
    ui->sourceEdit->setPlainText(ui->translationEdit->translation());
    ui->sourceEdit->moveCursor(QTextCursor::End);
}

void MainWindow::openSettings()
{
    SettingsDialog config(this);
    if (config.exec() == QDialog::Accepted) {
        const AppSettings settings;
        loadSettings(settings);
    }
}

void MainWindow::setAutoTranslateEnabled(bool enabled)
{
    ui->autoTranslateCheckBox->setChecked(enabled);
}

void MainWindow::copySourceText()
{
    if (!ui->sourceEdit->toPlainText().isEmpty())
        QGuiApplication::clipboard()->setText(ui->sourceEdit->toPlainText());
}

void MainWindow::copyTranslation()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        QGuiApplication::clipboard()->setText(ui->translationEdit->translation());
}

void MainWindow::copyAllTranslationInfo()
{
    if (!ui->translationEdit->toPlainText().isEmpty())
        QGuiApplication::clipboard()->setText(ui->translationEdit->toPlainText());
}

void MainWindow::quit()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QMetaObject::invokeMethod(QCoreApplication::instance(), &QCoreApplication::quit, Qt::QueuedConnection);
#else
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
#endif
}

void MainWindow::requestTranslation()
{
    QOnlineTranslator::Language translationLang;
    if (ui->translationLanguagesWidget->isAutoButtonChecked())
        translationLang = AppSettings().preferredTranslationLanguage(ui->sourceLanguagesWidget->checkedLanguage());
    else
        translationLang = ui->translationLanguagesWidget->checkedLanguage();

    m_translator->translate(ui->sourceEdit->toSourceText(), currentEngine(), translationLang, ui->sourceLanguagesWidget->checkedLanguage());
}

// Re-translate to a secondary or a primary language if the autodetected source language and the translation language are the same
void MainWindow::requestRetranslation()
{
    const QOnlineTranslator::Language translationLang = AppSettings().preferredTranslationLanguage(m_translator->sourceLanguage());

    m_translator->translate(ui->sourceEdit->toSourceText(), currentEngine(), translationLang, m_translator->sourceLanguage());
}

void MainWindow::displayTranslation()
{
    if (!ui->translationEdit->parseTranslationData(m_translator)) {
        // Reset language on translation "Auto" button
        ui->translationLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);
        return;
    }

    // Display languages on "Auto" buttons
    if (ui->sourceLanguagesWidget->isAutoButtonChecked())
        ui->sourceLanguagesWidget->setAutoLanguage(m_translator->sourceLanguage());

    if (ui->translationLanguagesWidget->isAutoButtonChecked())
        ui->translationLanguagesWidget->setAutoLanguage(m_translator->translationLanguage());
    else
        ui->translationLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);

    // If window mode is notification, send a notification including the translation result
    const AppSettings settings;
    if (this->isHidden() && settings.windowMode() == AppSettings::Notification)
        m_trayIcon->showMessage(tr("Translation Result"), ui->translationEdit->toPlainText(), QSystemTrayIcon::NoIcon, m_translationNotificationDisplayTime*1000);
}

void MainWindow::clearTranslation()
{
    ui->translationEdit->clearTranslation();
    ui->translationLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);
}

void MainWindow::requestSourceLanguage()
{
    m_translator->detectLanguage(ui->sourceEdit->toSourceText(), currentEngine());
}

void MainWindow::parseSourceLanguage()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        QMessageBox::critical(this, tr("Unable to detect language"), m_translator->errorString());
        return;
    }

    ui->sourceLanguagesWidget->setAutoLanguage(m_translator->sourceLanguage());
}

void MainWindow::speakSource()
{
    ui->sourceSpeakButtons->speak(ui->sourceEdit->toSourceText(), ui->sourceLanguagesWidget->checkedLanguage(), currentEngine());
}

void MainWindow::speakTranslation()
{
    ui->translationSpeakButtons->speak(ui->translationEdit->translation(), ui->translationEdit->translationLanguage(), currentEngine());
}

void MainWindow::showTranslationWindow()
{
    // Always show main window if it already opened
    if (!isHidden()) {
        open();
        return;
    }

    const AppSettings settings;
    switch (settings.windowMode()) {
    case AppSettings::PopupWindow:
    {
        auto *popup = new PopupWindow(this);
        popup->show();
        popup->activateWindow();

        // Force listening for changes in source field
        if (!ui->autoTranslateCheckBox->isChecked())
            ui->sourceEdit->setRequestTranlationOnEdit(true);

        connect(popup, &PopupWindow::destroyed, [&] {
            // Undo force listening for changes
            if (!ui->autoTranslateCheckBox->isChecked())
                ui->sourceEdit->setRequestTranlationOnEdit(false);
        });
        break;
    }
    case AppSettings::MainWindow:
        open();
        break;
    case AppSettings::Notification:
        break;
    }
}

void MainWindow::copyTranslationToClipboard()
{
    if (m_translator->error() != QOnlineTranslator::NoError) {
        QMessageBox::critical(this, tr("Unable to translate text"), m_translator->errorString());
        return;
    }

    QGuiApplication::clipboard()->setText(ui->translationEdit->translation());
}

void MainWindow::forceSourceAutodetect()
{
    const AppSettings settings;
    if (settings.isForceSourceAutodetect()) {
        ui->sourceEdit->setRequestTranlationOnEdit(false);

        ui->sourceLanguagesWidget->checkAutoButton();

        if (ui->autoTranslateCheckBox->isChecked())
            ui->sourceEdit->setRequestTranlationOnEdit(true);
    }
}

void MainWindow::forceAutodetect()
{
    const AppSettings settings;
    ui->sourceEdit->setRequestTranlationOnEdit(false);

    if (settings.isForceTranslationAutodetect())
        ui->translationLanguagesWidget->checkAutoButton();

    if (settings.isForceSourceAutodetect())
        ui->sourceLanguagesWidget->checkAutoButton();

    if (ui->autoTranslateCheckBox->isChecked())
        ui->sourceEdit->setRequestTranlationOnEdit(true);
}

void MainWindow::setTranslationOnEditEnabled(bool enabled)
{
    ui->sourceEdit->setRequestTranlationOnEdit(enabled);
    if (enabled)
        ui->sourceEdit->markSourceAsChanged();
}

void MainWindow::resetAutoSourceButtonText()
{
    ui->sourceLanguagesWidget->setAutoLanguage(QOnlineTranslator::Auto);
}

void MainWindow::setTaskbarState(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_taskbar->setProgressVisible(true);
#if defined(Q_OS_WIN)
        m_taskbar->setWindowsProgressState(QTaskbarControl::Running);
#endif
        break;
    case QMediaPlayer::PausedState:
#if defined(Q_OS_WIN)
        m_taskbar->setWindowsProgressState(QTaskbarControl::Paused);
#endif
        break;
    case QMediaPlayer::StoppedState:
        m_taskbar->setProgressVisible(false);
#if defined(Q_OS_WIN)
        m_taskbar->setWindowsProgressState(QTaskbarControl::Stopped);
#endif
        break;
    }
}

void MainWindow::showAppRunningMessage()
{
    auto *message = new QMessageBox(this);
    message->setIcon(QMessageBox::Information);
    message->setText(tr("The application is already running"));
    message->setAttribute(Qt::WA_DeleteOnClose);

    open();
    message->open();
}

void MainWindow::setSourceText(const QString &text)
{
    ui->sourceEdit->setRequestTranlationOnEdit(false);
    ui->sourceEdit->setPlainText(text);
    if (ui->autoTranslateCheckBox->isChecked())
        ui->sourceEdit->setRequestTranlationOnEdit(true);
}

#ifdef Q_OS_WIN
void MainWindow::checkForUpdates()
{
    auto *release = qobject_cast<QGitTag *>(sender());
    release->deleteLater();
    if (release->error())
        return;

    const int installer = release->assetId(".exe");
    if (installer != -1 && QCoreApplication::applicationVersion() < release->tagName()) {
        auto *updater = new UpdaterDialog(release, installer, this);
        updater->setAttribute(Qt::WA_DeleteOnClose);
        updater->open();
    }

    AppSettings settings;
    settings.setLastUpdateCheckDate(QDate::currentDate());
}
#endif

void MainWindow::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LocaleChange: {
        // System language chaged
        AppSettings settings;
        const QLocale::Language lang = settings.language();
        if (lang == QLocale::AnyLanguage)
            AppSettings::applyLanguage(lang); // Reload language if application use system language
        break;
    }
    case QEvent::LanguageChange:
        // Reload UI if application language changed
        ui->retranslateUi(this);

        ui->sourceLanguagesWidget->retranslate();
        ui->translationLanguagesWidget->retranslate();

        m_trayMenu->actions().at(0)->setText(tr("Show window"));
        m_trayMenu->actions().at(1)->setText(tr("Settings"));
        m_trayMenu->actions().at(2)->setText(tr("Exit"));
        break;
    default:
        QMainWindow::changeEvent(event);
    }
}

void MainWindow::buildStateMachine()
{
    m_stateMachine->setGlobalRestorePolicy(QStateMachine::RestoreProperties);

    auto *idleState = new QState(m_stateMachine);
    auto *translationState = new QState(m_stateMachine);
    auto *translateSelectionState = new QState(m_stateMachine);
    auto *speakSourceState = new QState(m_stateMachine);
    auto *speakTranslationState = new QState(m_stateMachine);
    auto *speakSelectionState = new QState(m_stateMachine);
    auto *speakTranslatedSelectionState = new QState(m_stateMachine);
    auto *copyTranslatedSelectionState = new QState(m_stateMachine);
    m_stateMachine->setInitialState(idleState);

    buildTranslationState(translationState);
    buildTranslateSelectionState(translateSelectionState);
    buildSpeakSourceState(speakSourceState);
    buildSpeakTranslationState(speakTranslationState);
    buildSpeakSelectionState(speakSelectionState);
    buildSpeakTranslatedSelectionState(speakTranslatedSelectionState);
    buildCopyTranslatedSelectionState(copyTranslatedSelectionState);

    // Add transitions between all states
    for (QState *state : m_stateMachine->findChildren<QState *>()) {
        state->addTransition(ui->translateButton, &QToolButton::clicked, translationState);
        state->addTransition(ui->sourceEdit, &SourceTextEdit::translationRequested, translationState);
        state->addTransition(ui->sourceSpeakButtons, &SpeakButtons::playerMediaRequested, speakSourceState);
        state->addTransition(ui->translationSpeakButtons, &SpeakButtons::playerMediaRequested, speakTranslationState);

        state->addTransition(this, &MainWindow::translateSelectionRequested, translateSelectionState);
        state->addTransition(this, &MainWindow::speakSelectionRequested, speakSelectionState);
        state->addTransition(this, &MainWindow::speakTranslatedSelectionRequested, speakTranslatedSelectionState);
        state->addTransition(this, &MainWindow::copyTranslatedSelectionRequested, copyTranslatedSelectionState);
    }

    translationState->addTransition(translationState, &QState::finished, idleState);
    speakSourceState->addTransition(speakSourceState, &QState::finished, idleState);
    speakTranslationState->addTransition(speakTranslationState, &QState::finished, idleState);
    translateSelectionState->addTransition(translateSelectionState, &QState::finished, idleState);
    speakSelectionState->addTransition(speakSelectionState, &QState::finished, idleState);
    speakTranslatedSelectionState->addTransition(speakTranslatedSelectionState, &QState::finished, idleState);
}

void MainWindow::buildTranslationState(QState *state)
{
    auto *abortPreviousState = new QState(state);
    auto *requestState = new QState(state);
    auto *checkLanguagesState = new QState(state);
    auto *requestInOtherLangeState = new QState(state);
    auto *parseState = new QState(state);
    auto *clearTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(abortPreviousState);

    connect(abortPreviousState, &QState::entered, m_translator, &QOnlineTranslator::abort);
    connect(abortPreviousState, &QState::entered, ui->translationSpeakButtons, &SpeakButtons::stopSpeaking); // Stop translation speaking
    connect(requestState, &QState::entered, this, &MainWindow::requestTranslation);
    connect(requestInOtherLangeState, &QState::entered, this, &MainWindow::requestRetranslation);
    connect(parseState, &QState::entered, this, &MainWindow::displayTranslation);
    connect(clearTranslationState, &QState::entered, this, &MainWindow::clearTranslation);
    setupRequestStateButtons(requestState);
    setupRequestStateButtons(abortPreviousState);

    auto *noTextTransition = new TextEmptyTransition(ui->sourceEdit, abortPreviousState);
    noTextTransition->setTargetState(clearTranslationState);

    auto *translationRunningTransition = new TranslatorAbortedTransition(m_translator, abortPreviousState);
    translationRunningTransition->setTargetState(requestState);

    auto *otherLangTransition = new RetranslationTransition(m_translator, ui->translationLanguagesWidget, checkLanguagesState);
    otherLangTransition->setTargetState(requestInOtherLangeState);

    requestState->addTransition(m_translator, &QOnlineTranslator::finished, checkLanguagesState);
    checkLanguagesState->addTransition(parseState);
    requestInOtherLangeState->addTransition(m_translator, &QOnlineTranslator::finished, parseState);
    parseState->addTransition(finalState);
    clearTranslationState->addTransition(finalState);
}

void MainWindow::buildSpeakSourceState(QState *state)
{
    auto *initialState = new QState(state);
    auto *abortPreviousState = new QState(state);
    auto *requestLangState = new QState(state);
    auto *parseLangState = new QState(state);
    auto *speakTextState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(initialState);

    connect(abortPreviousState, &QState::entered, m_translator, &QOnlineTranslator::abort);
    connect(requestLangState, &QState::entered, this, &MainWindow::requestSourceLanguage);
    connect(parseLangState, &QState::entered, this, &MainWindow::parseSourceLanguage);
    connect(speakTextState, &QState::entered, this, &MainWindow::speakSource);
    setupRequestStateButtons(requestLangState);

    auto *langDetectedTransition = new LanguageDetectedTransition(ui->sourceLanguagesWidget, initialState);
    langDetectedTransition->setTargetState(speakTextState);

    auto *translationRunningTransition = new TranslatorAbortedTransition(m_translator, abortPreviousState);
    translationRunningTransition->setTargetState(requestLangState);

    auto *errorTransition = new TranslatorErrorTransition(m_translator, parseLangState);
    errorTransition->setTargetState(finalState);

    initialState->addTransition(abortPreviousState);
    requestLangState->addTransition(m_translator, &QOnlineTranslator::finished, parseLangState);
    parseLangState->addTransition(speakTextState);
    speakTextState->addTransition(finalState);
}

void MainWindow::buildTranslateSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *showWindowState = new QState(state);
    auto *translationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, Selection::instance(), &Selection::requestSelection);
    connect(showWindowState, &QState::entered, this, &MainWindow::showTranslationWindow);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(Selection::instance(), &Selection::requestedSelectionAvailable, showWindowState);
    showWindowState->addTransition(translationState);
    translationState->addTransition(translationState, &QState::finished, finalState);
}

void MainWindow::buildSpeakTranslationState(QState *state)
{
    auto *speakTextState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(speakTextState);

    connect(speakTextState, &QState::entered, this, &MainWindow::speakTranslation);

    speakTextState->addTransition(finalState);
}

void MainWindow::buildSpeakSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *speakSourceState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, Selection::instance(), &Selection::requestSelection);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceSourceAutodetect);
    buildSpeakSourceState(speakSourceState);

    setSelectionAsSourceState->addTransition(Selection::instance(), &Selection::requestedSelectionAvailable, speakSourceState);
    speakSourceState->addTransition(speakSourceState, &QState::finished, finalState);
}

void MainWindow::buildSpeakTranslatedSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *translationState = new QState(state);
    auto *speakTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, Selection::instance(), &Selection::requestSelection);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceAutodetect);
    buildSpeakTranslationState(speakTranslationState);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(Selection::instance(), &Selection::requestedSelectionAvailable, translationState);
    translationState->addTransition(translationState, &QState::finished, speakTranslationState);
    speakTranslationState->addTransition(speakTranslationState, &QState::finished, finalState);
}

void MainWindow::buildCopyTranslatedSelectionState(QState *state)
{
    auto *setSelectionAsSourceState = new QState(state);
    auto *translationState = new QState(state);
    auto *copyTranslationState = new QState(state);
    auto *finalState = new QFinalState(state);
    state->setInitialState(setSelectionAsSourceState);

    connect(setSelectionAsSourceState, &QState::entered, Selection::instance(), &Selection::requestSelection);
    connect(setSelectionAsSourceState, &QState::entered, this, &MainWindow::forceAutodetect);
    connect(copyTranslationState, &QState::entered, this, &MainWindow::copyTranslationToClipboard);
    buildTranslationState(translationState);

    setSelectionAsSourceState->addTransition(Selection::instance(), &Selection::requestedSelectionAvailable, translationState);
    translationState->addTransition(translationState, &QState::finished, copyTranslationState);
    copyTranslationState->addTransition(finalState);
}

void MainWindow::setupRequestStateButtons(QState *state)
{
    state->assignProperty(ui->translateButton, "enabled", false);
    state->assignProperty(ui->clearButton, "enabled", false);
    state->assignProperty(ui->abortButton, "enabled", true);
}

void MainWindow::loadSettings(const AppSettings &settings)
{
    // Interface
    ui->translationEdit->setFont(settings.font());
    ui->sourceEdit->setFont(settings.font());

    m_popupSize = {settings.popupWidth(), settings.popupHeight()};
    m_popupOpacity = settings.popupOpacity();

    m_translationNotificationDisplayTime = settings.translationNotificationDisplayTime();

    ui->sourceLanguagesWidget->setLanguageFormat(settings.mainWindowLanguageFormat());
    ui->translationLanguagesWidget->setLanguageFormat(settings.mainWindowLanguageFormat());
    m_popupLanguageFormat = settings.popupLanguageFormat();

    m_trayIcon->loadSettings(settings);

    // Translation
    m_translator->setSourceTranslitEnabled(settings.isSourceTranslitEnabled());
    m_translator->setTranslationTranslitEnabled(settings.isTranslationTranslitEnabled());
    m_translator->setSourceTranscriptionEnabled(settings.isSourceTranscriptionEnabled());
    m_translator->setTranslationOptionsEnabled(settings.isTranslationOptionsEnabled());
    m_translator->setExamplesEnabled(settings.isExamplesEnabled());
    ui->sourceEdit->setSimplifySource(settings.isSimplifySource());

    // Connection
    QNetworkProxy proxy;
    proxy.setType(settings.proxyType());
    if (proxy.type() == QNetworkProxy::HttpProxy || proxy.type() == QNetworkProxy::Socks5Proxy) {
        proxy.setHostName(settings.proxyHost());
        proxy.setPort(settings.proxyPort());
        if (settings.isProxyAuthEnabled()) {
            proxy.setUser(settings.proxyUsername());
            proxy.setPassword(settings.proxyPassword());
        }
    }
    QNetworkProxy::setApplicationProxy(proxy);

    // TTS
    ui->sourceSpeakButtons->setVoice(QOnlineTranslator::Yandex, settings.voice(QOnlineTranslator::Yandex));
    ui->sourceSpeakButtons->setEmotion(QOnlineTranslator::Yandex, settings.emotion(QOnlineTranslator::Yandex));

    // Global shortcuts
    if (QHotkey::isPlatformSupported() && settings.isGlobalShortuctsEnabled()) {
        m_translateSelectionHotkey->setShortcut(settings.translateSelectionShortcut(), true);
        m_speakSelectionHotkey->setShortcut(settings.speakSelectionShortcut(), true);
        m_stopSpeakingHotkey->setShortcut(settings.stopSpeakingShortcut(), true);
        m_speakTranslatedSelectionHotkey->setShortcut(settings.speakTranslatedSelectionShortcut(), true);
        m_showMainWindowHotkey->setShortcut(settings.showMainWindowShortcut(), true);
        m_copyTranslatedSelectionHotkey->setShortcut(settings.copyTranslatedSelectionShortcut(), true);
    } else {
        m_translateSelectionHotkey->setRegistered(false);
        m_speakSelectionHotkey->setRegistered(false);
        m_stopSpeakingHotkey->setRegistered(false);
        m_speakTranslatedSelectionHotkey->setRegistered(false);
        m_showMainWindowHotkey->setRegistered(false);
        m_copyTranslatedSelectionHotkey->setRegistered(false);
    }

    // Window shortcuts
    ui->sourceSpeakButtons->setSpeakShortcut(settings.speakSourceShortcut());
    ui->translationSpeakButtons->setSpeakShortcut(settings.speakTranslationShortcut());
    ui->translateButton->setShortcut(settings.translateShortcut());
    ui->swapButton->setShortcut(settings.swapShortcut());
    ui->copyTranslationButton->setShortcut(settings.copyTranslationShortcut());
    m_closeWindowsShortcut->setKey(settings.closeWindowShortcut());
}

// Toggle language logic
void MainWindow::checkLanguageButton(int checkedId)
{
    LanguageButtonsWidget *checkedGroup;
    LanguageButtonsWidget *anotherGroup;
    if (sender() == ui->sourceLanguagesWidget) {
        checkedGroup = ui->sourceLanguagesWidget;
        anotherGroup = ui->translationLanguagesWidget;
    } else {
        checkedGroup = ui->translationLanguagesWidget;
        anotherGroup = ui->sourceLanguagesWidget;
    }

    /* If the target and source languages are the same and they are not autodetect buttons,
     * then select previous checked language from just checked language group to another group */
    const QOnlineTranslator::Language checkedLang = checkedGroup->language(checkedId);
    if (checkedLang == anotherGroup->checkedLanguage() && !checkedGroup->isAutoButtonChecked() && !anotherGroup->isAutoButtonChecked()) {
        if (!anotherGroup->checkLanguage(checkedGroup->previousCheckedLanguage()))
            anotherGroup->checkAutoButton(); // Select "Auto" button if group do not have such language
        return;
    }

    // Check if selected language is supported by engine
    if (!QOnlineTranslator::isSupportTranslation(currentEngine(), checkedLang)) {
        for (int i = 0; i < ui->engineComboBox->count(); ++i) {
            if (QOnlineTranslator::isSupportTranslation(static_cast<QOnlineTranslator::Engine>(i), checkedLang)) {
                ui->engineComboBox->setCurrentIndex(i); // Check first supported language
                break;
            }
        }
        return;
    }

    ui->sourceEdit->markSourceAsChanged();
}

QOnlineTranslator::Engine MainWindow::currentEngine()
{
    return static_cast<QOnlineTranslator::Engine>(ui->engineComboBox->currentIndex());
}
