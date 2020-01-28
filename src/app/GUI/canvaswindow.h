// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CANVASWINDOW_H
#define CANVASWINDOW_H

#include <QWidget>
#include "glwindow.h"
#include "singlewidgettarget.h"
#include "keyfocustarget.h"
#include "smartPointers/ememory.h"
#include "canvas.h"
#include "Animators/SculptPath/sculptbrush.h"
class Brush;
class WindowSingleWidgetTarget;
enum class ColorMode : short;
enum class CanvasMode : short;
class SceneBoundGradient;
class BoundingBox;
class ContainerBox;
class SoundComposition;
class PaintSettingsApplier;
class RenderInstanceSettings;
class eTask;
class ImageBox;
class SingleSound;
class VideoBox;
class Canvas;
class PaintSettingsAnimator;
class OutlineSettingsAnimator;
class SimpleBrushWrapper;
class Actions;

class CanvasWindow : public GLWindow, public KeyFocusTarget {
    Q_OBJECT
public:
    explicit CanvasWindow(Document& document,
                          QWidget * const parent = nullptr);
    ~CanvasWindow();
    Canvas *getCurrentCanvas();
    const QMatrix& getViewTransform() const
    { return mViewTransform; }

    void setCurrentCanvas(Canvas * const canvas);
    bool hasNoCanvas();

    void finishAction();
    void queTasksAndUpdate();
    bool KFT_keyReleaseEvent(QKeyEvent *event);
    bool KFT_keyPressEvent(QKeyEvent *event);

    void setResolutionFraction(const qreal percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    ContainerBox *getCurrentGroup();
    void applyPaintSettingToSelected(const PaintSettingsApplier &setting);
    void setSelectedFillColorMode(const ColorMode mode);
    void setSelectedStrokeColorMode(const ColorMode mode);

    int getCurrentFrame();
    int getMaxFrame();

    void grabMouse();
    void releaseMouse();
    bool isMouseGrabber();

    void openSettingsWindowForCurrentCanvas();

    void writeState(eWriteStream& dst) const;
    void readState(eReadStream& src);

    bool event(QEvent *e);

    void resizeEvent(QResizeEvent* e);

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
protected:
    void KFT_setFocusToWidget();

    void KFT_clearFocus() {
        clearFocus();
        update();
    }
private:
    void setCanvasMode(const CanvasMode mode);
    void updatePaintModeCursor();
    void updateSculptModeCursor();

    Document& mDocument;
    Actions& mActions;

    QSize mOldSize{-1, -1};
    QMatrix mViewTransform;
    QPointF mPrevMousePos;
    QPointF mPrevPressPos;
    bool mValidPaintTarget = false;

    bool mValidSculptTarget = false;

    bool mBlockInput = false;
    bool mMouseGrabber = false;

    qsptr<WindowSingleWidgetTarget> mWindowSWTTarget;

    Canvas* mCurrentCanvas = nullptr;
    bool mFitToSizeBlocked = false;

    //void paintEvent(QPaintEvent *);

    void renderSk(SkCanvas * const canvas);
    void tabletEvent(QTabletEvent *e);

    bool handleCanvasModeChangeKeyPress(QKeyEvent *event);
    bool handleCutCopyPasteKeyPress(QKeyEvent *event);
    bool handleTransformationKeyPress(QKeyEvent *event);
    bool handleZValueKeyPress(QKeyEvent *event);
    bool handleParentChangeKeyPress(QKeyEvent *event);
    bool handleGroupChangeKeyPress(QKeyEvent *event);
    bool handleResetTransformKeyPress(QKeyEvent *event);
    bool handleRevertPathKeyPress(QKeyEvent *event);
    bool handleStartTransformKeyPress(const KeyEvent &e);
    bool handleSelectAllKeyPress(QKeyEvent *event);
    bool handleShiftKeysKeyPress(QKeyEvent *event);    
signals:
    void changeCanvasFrameRange(FrameRange);
public:
    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int id);

    QPointF mapToCanvasCoord(const QPointF& windowCoord);
    void translateView(const QPointF &trans);
    void zoomView(const qreal scaleBy, const QPointF &absOrigin);

    void fitCanvasToSize();
    void resetTransormation();
};

#endif // CANVASWINDOW_H
