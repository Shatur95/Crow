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

#ifndef POPUPWINDOW_H
#define POPUPWINDOW_H

#include <QMenu>
#include <QButtonGroup>
#include <QToolButton>

#include "qonlinetranslator.h"

namespace Ui {
class PopupWindow;
}

class PopupWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PopupWindow(QMenu *languagesMenu, QButtonGroup *sourceGroup, QButtonGroup *translationGroup, QWidget *parent = 0);
    ~PopupWindow();

    QButtonGroup *sourceButtons();
    QButtonGroup *translationButtons();
    QToolButton *sourceAutoButton();
    QToolButton *translationAutoButton();
    QToolButton *swapButton();
    QToolButton *sourceCopyButton();
    QToolButton *sourceSayButton();
    QToolButton *translationCopyAllButton();
    QToolButton *translationCopyButton();
    QToolButton *translationSayButton();


public slots:
    void setTranslation(const QString &text);
    void copySourceButton(QAbstractButton *button, const int &id);
    void copyTranslationButton(QAbstractButton *button, const int &id);
    void checkSourceButton(const int &id, const bool &checked);
    void checkTranslationButton(const int &id, const bool &checked);

private:
    void resizeEvent(QResizeEvent *event);
    void copyLanguageButtons(QButtonGroup *existingGroup, QButtonGroup *copyingGroup);

    Ui::PopupWindow *ui;

    QButtonGroup *sourceButtonGroup;
    QButtonGroup *translationButtonGroup;
};

#endif // POPUPWINDOW_H
