# ![Crow Translate logo](./dist/unix/generic/hicolor/72x72/apps/crow-translate.png) Crow Translate

[![GitHub (pre-)release](https://img.shields.io/github/release/crow-translate/crow-translate/all.svg)](https://github.com/crow-translate/crow-translate/releases)
[![Codacy grade](https://img.shields.io/codacy/grade/b28c6646bb324ffb98092f63a9b5896e.svg)](https://app.codacy.com/project/crow-translate/crow-translate/dashboard)

**Crow Translate** is a simple and lightweight translator programmed in **C++ / Qt** that allows to translate and speak text using Google, Yandex and Bing translate API.
You may also be interested in my library [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator "A library for Qt5 that provides free usage of Google, Yandex and Bing translate API.") used in this project. 

## Content

-   [Screenshots](#screenshots)
-   [Features](#features)
-   [Default keyboard shortcuts](#default-keyboard-shortcuts)
-   [CLI commands](#cli-commands)
-   [D-Bus API](#d-bus-api)
-   [Dependencies](#dependencies)
-   [Third-party](#third-party)
-   [Installation](#installation)
-   [Building](#building)
-   [Localization](#localization)

## Screenshots

### Linux KDE

<p align="center">
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/main.png" alt="Main screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/popup.png" width="250px" height="140px" alt="Popup screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/sentense.png" width="250px" height="140px" alt="Sentense screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/settings.png" width="250px" height="140px" alt="Settings screenshot"/>
</p>
<p align="center">
  <sub><b>Screenshots</b>: OS: <a href="https://www.archlinux.org">Arch Linux</a> | Desktop environment: <a href="https://www.kde.org/plasma-desktop">Plasma</a> | Theme: Breeze | Icons: <a href="https://github.com/PapirusDevelopmentTeam/papirus-icon-theme">Papirus</a> | Wallpaper: <a href="https://dynamicwallpaper.club/wallpaper/nrv0me8vd1">Catalina</a></sub>
</p>

### Windows 10

<p align="center">
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/main.png" alt="Main screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/popup.png" width="250px" height="140px" alt="Popup screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/sentense.png" width="250px" height="140px" alt="Sentense screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/settings.png" width="250px" height="140px" alt="Settings screenshot"/>
</p>
<p align="center">
  <sub><b>Screenshots</b>: OS: <a href="https://www.microsoft.com/en-us/software-download/windows10">Windows 10</a> | Theme: default | Wallpaper: <a href="https://wallpaperscraft.com/wallpaper/needles_spruce_branch_blur_114943">WallpapersCraft</a></sub>
</p>

## Features

-   Translate and speak text in any application that supports text selection
-   Support 117 different languages
-   Low memory consumption (~20MB)
-   Highly customizable shortcuts
-   Command-line interface with rich options
-   D-Bus API
-   Available for Linux and Windows

## Default keyboard shortcuts

You can change them in the settings. Some key sequences may not be available due to OS limitations.

Wayland does not support global shortcuts registration, but you can use [D-Bus](#d-bus-api) to bind actions in the system settings. For desktop environments that support [additional applications actions](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#extra-actions) (KDE, for example) you will see them predefined in the system shortcut settings. You can also use them for X11 sessions, but you need to disable global shortcuts registration in the application settings to avoid conflicts.

### Global

| Key                                             | Description                        |
| ----------------------------------------------- | ---------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>E</kbd> | Translate selected text            |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>S</kbd> | Speak selected text                |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>F</kbd> | Speak translation of selected text |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>G</kbd> | Stop speaking                      |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>C</kbd> | Show main window                   |

### In main window

| Key                                               | Description                             |
| ------------------------------------------------- | --------------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Return</kbd>               | Translate                               |
| <kbd>Ctrl</kbd> + <kbd>Q</kbd>                    | Close window                            |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                    | Speak source / pause text speaking      |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>S</kbd> | Speak translation / pause text speaking |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>C</kbd> | Copy translation to clipboard           |

## CLI commands

The program also has a console interface.

**Usage:** `crow [options] text`

| Option                     | Description                                                                                    |
| -------------------------- | ---------------------------------------------------------------------------------------------- |
| `-h, --help`               | Display help                                                                                   |
| `-v, --version`            | Display version information                                                                    |
| `-c, --codes`              | Display language codes                                                                         |
| `-a, --audio-only`         | Print text only for speaking when using `--speak-translation` or `--speak-source`              |
| `-s, --source <code>`      | Specify the source language (by default, engine will try to determine the language on its own) |
| `-t, --translation <code>` | Specify the translation language(s), splitted by '+' (by default, the system language is used) |
| `-l, --locale <code>`      | Specify the translator language (by default, the system language is used)                      |
| `-e, --engine <engine>`    | Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default        |
| `-p, --speak-translation`  | Speak the translation                                                                          |
| `-u, --speak-source`       | Speak the source                                                                               |
| `-f, --file`               | Read source text from files. Arguments will be interpreted as file paths                       |
| `-i, --stdin`              | Add stdin data to source text                                                                  |

**Note:** If you do not pass startup arguments to the program, the GUI starts.

## D-Bus API

Currently available only for [global shortcuts](#global).

    io.crow_translate.CrowTranslate
    └── /io/crow_translate/CrowTranslate/MainWindow
        ├── method void io.crow_translate.CrowTranslate.MainWindow.copyTranslatedSelection()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.open()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.speakSelection()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.speakTranslatedSelection()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.stopSpeaking()
        └── method void io.crow_translate.CrowTranslate.MainWindow.translateSelection()

For example, you can show main window using `dbus-send`:

```bash
dbus-send --type=method_call --dest=io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow io.crow_translate.CrowTranslate.MainWindow.open
```

Or via `qdbus`:

```bash
qdbus io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow io.crow_translate.CrowTranslate.MainWindow.open
# or shorter
qdbus io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow open
```

## Dependencies

**Arch Linux:** qt5-base qt5-multimedia qt5-x11extras gst-plugins-good openssl

**Debian:** gstreamer1.0-fluendo-mp3, qtgstreamer-plugins-qt5, gstreamer1.0-plugins-good, gstreamer1.0-alsa, gstreamer1.0-pulseaudio, libqt5multimedia5-plugins

## Third-party

### Libraries

This project uses the following third-party libraries:

-   [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator) - provides free usage of Google, Yandex and Bing translate API.
-   [QGitTag](https://github.com/crow-translate/QGitTag) - uses the GitHub API to provide information about releases.
-   [QHotkey](https://github.com/Skycoder42/QHotkey) - provides global shortcuts for desktop platforms.
-   [QTaskbarControl](https://github.com/Skycoder42/QTaskbarControl) - allows to create a taskbar/launcher progress for all desktop platforms.
-   [SingleApplication](https://github.com/itay-grudev/SingleApplication) - prevents launch of multiple application instances.

Therefore, if you want to clone this project, you need to use the `--recursive` option: 

```bash
git clone --recursive git@github.com:crow-translate/crow-translate.git
```

or you can initialize these modules later:

```bash
git clone git@github.com:crow-translate/crow-translate.git
git submodule init
git submodule update
```

### Icons

Only Linux supports icon theming. Windows use [Papirus](https://github.com/PapirusDevelopmentTeam/papirus-icon-theme "Free and open source SVG icon theme") icons.

[FlagKit](https://github.com/madebybowtie/FlagKit "Beautiful flag icons for usage in apps and on the web") icons are used for flags.

## Installation

### Windows

You can download the installer or archive from the [releases](https://github.com/crow-translate/crow-translate/releases/latest) page.

You can also install `crow-translate` from [scoop](https://scoop.sh).

### Linux

#### Pacman-based (Arch Linux, Manjaro, Chakra etc.)

You can install [crow-translate](https://aur.archlinux.org/packages/crow-translate) or [crow-translate-git](https://aur.archlinux.org/packages/crow-translate-git) from AUR.

#### APT-based (Debian, Ubuntu, Mint etc.)

The package is available on the [releases](https://github.com/crow-translate/crow-translate/releases/latest) page.

#### RPM-based (Fedora, RHEL, CentOS etc.)

You can install the package from the [Fedora Copr](https://copr.fedorainfracloud.org/coprs/faezebax/crow-translate).

#### openSUSE (Tumbleweed, Leap)

Users of openSUSE Tumbleweed (rolling distribution) can always install the latest version via `$ sudo zypper install crow-translate`
A package for users of the other openSUSE distributions like Leap 15.x is available on [OBS](https://software.opensuse.org/package/crow-translate?search_term=crow-translate).


## Building

### Automatic script

You can use the automatic script that builds **Crow Translate** and creates a package for your distribution:

```bash
cd dist/unix
./create-package.sh
```

Than you can install it as usual. The script will tell you where the package will be after the making. Currently, only **Arch Linux**, **Debian** and their derivatives are supported.

### Manual building

You can build **Crow Translate** by using the following commands:

```bash
qmake # Or qmake-qt5 in some distributions
make
make clean
```

You will then get a binary named `crow`.

### Build parameters

-   `PORTABLE_MODE` - Enable portable functionality. If you create file named `settings.ini` in the app folder and Crow will store the configuration in it. It also adds the “Portable Mode” option to the application settings, which does the same.

Build parameters are passed at the qmake stage: `qmake "DEFINES += PORTABLE_MODE"`.

## Localization

To help with localization you can use [Transifex](https://www.transifex.com/crow-translate/crow-translate) or translate files in `data/translations` with [Qt Linguist](https://doc.qt.io/Qt-5/linguist-translators.html) directly. To add a new language, make a request on the Transifex page or copy `data/translations/crow.ts` to `data/translations/crow_<ISO 639-1 language code>.ts`, translate it and send a pull request.
