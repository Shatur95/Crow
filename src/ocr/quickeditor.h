/*
 *  Copyright (C) 2018 Ambareesh "Amby" Balaji <ambareeshbalaji@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef QUICKEDITOR_H
#define QUICKEDITOR_H

#include "settings/appsettings.h"

#include <QKeyEvent>
#include <QPainter>
#include <QStaticText>
#include <QWidget>

#include <vector>

class QMouseEvent;
class AppSettings;

class QuickEditor : public QWidget
{
    Q_OBJECT

public:
    explicit QuickEditor(QWidget *parent = nullptr);

    void loadSettings(const AppSettings &settings);
    void capture();

signals:
    void grabDone(const QPixmap &thePixmap);
    void grabCancelled();

private:
    enum MouseState : short {
        None = 0, // 0000
        Inside = 1 << 0, // 0001
        Outside = 1 << 1, // 0010
        TopLeft = 5, //101
        Top = 17, // 10001
        TopRight = 9, // 1001
        Right = 33, // 100001
        BottomRight = 6, // 110
        Bottom = 18, // 10010
        BottomLeft = 10, // 1010
        Left = 34, // 100010
        TopLeftOrBottomRight = TopLeft & BottomRight, // 100
        TopRightOrBottomLeft = TopRight & BottomLeft, // 1000
        TopOrBottom = Top & Bottom, // 10000
        RightOrLeft = Right & Left, // 100000
    };

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *) override;

    int boundsLeft(int newTopLeftX, const bool mouse = true);
    int boundsRight(int newTopLeftX, const bool mouse = true);
    int boundsUp(int newTopLeftY, const bool mouse = true);
    int boundsDown(int newTopLeftY, const bool mouse = true);

    void drawBottomHelpText(QPainter &painter);
    void drawDragHandles(QPainter &painter);
    void drawMagnifier(QPainter &painter);
    void drawMidHelpText(QPainter &painter);
    void drawSelectionSizeTooltip(QPainter &painter, bool dragHandlesVisible);

    void setMouseCursor(const QPointF &pos);
    MouseState mouseLocation(const QPointF &pos);

    void setBottomHelpText();
    void layoutBottomHelpText();

    void acceptSelection();

    static constexpr int handleRadiusMouse = 9;
    static constexpr int handleRadiusTouch = 12;
    static constexpr qreal increaseDragAreaFactor = 2.0;
    static constexpr int minSpacingBetweenHandles = 20;
    static constexpr int borderDragAreaSize = 10;

    static constexpr int selectionSizeThreshold = 100;

    static constexpr int selectionBoxPaddingX = 5;
    static constexpr int selectionBoxPaddingY = 4;
    static constexpr int selectionBoxMarginY = 5;

    static constexpr int bottomHelpMaxLength = 6;

    static constexpr int bottomHelpBoxPaddingX = 12;
    static constexpr int bottomHelpBoxPaddingY = 8;
    static constexpr int bottomHelpBoxPairSpacing = 6;
    static constexpr int bottomHelpBoxMarginBottom = 5;
    static constexpr int midHelpTextFontSize = 12;

    static constexpr int magnifierLargeStep = 15;

    static constexpr int magZoom = 5;
    static constexpr int magPixels = 16;
    static constexpr int magOffset = 32;

    static bool bottomHelpTextPrepared;

    QColor mMaskColor;
    QColor mStrokeColor;
    QColor mCrossColor;
    QColor mLabelBackgroundColor;
    QColor mLabelForegroundColor;
    QRectF mSelection;
    QPointF mStartPos;
    QPointF mInitialTopLeft;
    QString mMidHelpText;
    QFont mMidHelpTextFont;
    std::pair<QStaticText, std::vector<QStaticText>> mBottomHelpText[bottomHelpMaxLength];
    QFont mBottomHelpTextFont;
    QRect mBottomHelpBorderBox;
    QPoint mBottomHelpContentPos;
    int mBottomHelpGridLeftWidth;
    MouseState mMouseDragState;
    QPixmap mPixmap;
    qreal dprI;
    QPointF mMousePos;
    bool mMagnifierAllowed;
    bool mShowMagnifier;
    bool mToggleMagnifier;
    bool mReleaseToCapture;
    AppSettings::RegionRememberType mRememberRegion = AppSettings::defaultRegionRememberType();
    bool mDisableArrowKeys;
    QRect mPrimaryScreenGeo;
    int mbottomHelpLength;

    // Midpoints of handles
    QVector<QPointF> mHandlePositions = QVector<QPointF>{8};
    // Radius of handles is either handleRadiusMouse or handleRadiusTouch
    int mHandleRadius;
};

#endif // QUICKEDITOR_H
