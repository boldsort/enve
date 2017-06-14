#include "canvas.h"
#include <QMouseEvent>
#include <QMenu>
#include "pathpivot.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"
#include "Boxes/imagebox.h"
#include "Boxes/textbox.h"
#include "edge.h"
#include "Animators/singlepathanimator.h"
#include "pathpoint.h"
#include "Animators/pathanimator.h"

void Canvas::handleMovePathMousePressEvent() {
    mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mLastMouseEventPosRel);
    if(mLastPressedBox == NULL) {
        if(!isShiftPressed() ) {
            clearBoxesSelection();
        }
        mSelecting = true;
        startSelectionAtPoint(mLastMouseEventPosRel);
    } else {
        if(!isShiftPressed() && !mLastPressedBox->isSelected()) {
            clearBoxesSelection();
        }
    }
}

void Canvas::handleRightButtonMousePress(QMouseEvent *event) {
    if(mIsMouseGrabbing) {
        cancelCurrentTransform();
        clearAndDisableInput();
    } else {
        QPointF eventPos = mapCanvasAbsToRel(event->pos());
        BoundingBox *pressedBox = mCurrentBoxesGroup->getBoxAt(eventPos);
        if(pressedBox == NULL) {
            clearBoxesSelection();

            QMenu menu(mCanvasWindow->getCanvasWidget());

            menu.addAction("Paste");
            QMenu *linkCanvasMenu = menu.addMenu("Link Canvas");
            const QList<Canvas*> &listOfCanvas = mCanvasWindow->getCanvasList();
            Q_FOREACH(Canvas *canvas, listOfCanvas) {
                QAction *action = linkCanvasMenu->addAction(canvas->getName());
                if(canvas == this) {
                    action->setEnabled(false);
                    action->setVisible(false);
                }
            }

            QMenu *effectsMenu = menu.addMenu("Effects");
            effectsMenu->addAction("Blur");
//            effectsMenu->addAction("Brush");
            effectsMenu->addAction("Lines");
            effectsMenu->addAction("Circles");
            effectsMenu->addAction("Swirl");
            effectsMenu->addAction("Oil");
            effectsMenu->addAction("Implode");
            effectsMenu->addAction("Desaturate");

            QAction *selectedAction = menu.exec(event->globalPos());
            if(selectedAction != NULL) {
                if(selectedAction->text() == "Paste") {
                } else if(selectedAction->text() == "Blur") {
                    addEffect(new BlurEffect());
                } /*else if(selectedAction->text() == "Brush") {
                    addEffect(new BrushEffect());
                }*/ else if(selectedAction->text() == "Lines") {
                    addEffect(new LinesEffect());
                } else if(selectedAction->text() == "Circles") {
                    addEffect(new CirclesEffect());
                } else if(selectedAction->text() == "Swirl") {
                    addEffect(new SwirlEffect());
                } else if(selectedAction->text() == "Oil") {
                    addEffect(new OilEffect());
                } else if(selectedAction->text() == "Implode") {
                    addEffect(new ImplodeEffect());
                } else if(selectedAction->text() == "Desaturate") {
                    addEffect(new DesaturateEffect());
                } else { // link canvas
                    const QList<QAction*> &canvasActions =
                            linkCanvasMenu->actions();
                    int id = canvasActions.indexOf(selectedAction);
                    if(id >= 0) {
                        listOfCanvas.at(id)->createLink(
                                    mCurrentBoxesGroup)->centerPivotPosition();
                    }
                }
            } else {

            }
        } else {
            if(!pressedBox->isSelected()) {
                clearBoxesSelection();
                addBoxToSelection(pressedBox);
            }

            QMenu menu(mCanvasWindow->getCanvasWidget());

            menu.addAction("Apply Transformation");
            menu.addAction("Create Link");
            menu.addAction("Center Pivot");
            menu.addAction("Copy");
            menu.addAction("Cut");
            menu.addAction("Duplicate");
            menu.addAction("Group");
            menu.addAction("Ungroup");
            menu.addAction("Delete");

            QMenu *effectsMenu = menu.addMenu("Effects");
            effectsMenu->addAction("Blur");
            effectsMenu->addAction("Shadow");
//            effectsMenu->addAction("Brush");
            effectsMenu->addAction("Lines");
            effectsMenu->addAction("Circles");
            effectsMenu->addAction("Swirl");
            effectsMenu->addAction("Oil");
            effectsMenu->addAction("Implode");
            effectsMenu->addAction("Desaturate");
            effectsMenu->addAction("Alpha Matte");

            QMenu *pathEffectsMenu = menu.addMenu("Path Effects");
            QAction *discreteEffA =
                    pathEffectsMenu->addAction("Discrete Effect");
            QAction *duplicateEffA =
                    pathEffectsMenu->addAction("Duplicate Effect");

            QMenu *outlinePathEffectsMenu = menu.addMenu("Outline Effects");
            QAction *discretePathEffA =
                    outlinePathEffectsMenu->addAction("Discrete Effect");
            QAction *duplicatePathEffA =
                    outlinePathEffectsMenu->addAction("Duplicate Effect");

            QAction *selectedAction = menu.exec(event->globalPos());
            if(selectedAction != NULL) {
                if(selectedAction->text() == "Duplicate") {
                    duplicateSelectedBoxes();
                } else if(selectedAction->text() == "Delete") {
                    removeSelectedBoxesAndClearList();
                } else if(selectedAction->text() == "Apply Transformation") {
                    applyCurrentTransformationToSelected();
                } else if(selectedAction->text() == "Create Link") {
                    createLinkBoxForSelected();
                } else if(selectedAction->text() == "Group") {
                    groupSelectedBoxes();
                } else if(selectedAction->text() == "Ungroup") {
                    ungroupSelected();
                } else if(selectedAction->text() == "Center Pivot") {
                    centerPivotForSelected();
                } else if(selectedAction->text() == "Blur") {
                    applyBlurToSelected();
                } else if(selectedAction->text() == "Shadow") {
                    applyShadowToSelected();
                } else if(selectedAction->text() == "Brush") {
                    applyBrushEffectToSelected();
                } else if(selectedAction->text() == "Lines") {
                    applyLinesEffectToSelected();
                } else if(selectedAction->text() == "Circles") {
                    applyCirclesEffectToSelected();
                } else if(selectedAction->text() == "Swirl") {
                    applySwirlEffectToSelected();
                } else if(selectedAction->text() == "Oil") {
                    applyOilEffectToSelected();
                } else if(selectedAction->text() == "Implode") {
                    applyImplodeEffectToSelected();
                } else if(selectedAction->text() == "Desaturate") {
                    applyDesaturateEffectToSelected();
                } else if(selectedAction->text() == "Alpha Matte") {
                    applyAlphaMatteToSelected();
                } else if(selectedAction == discreteEffA) {
                    applyDiscretePathEffectToSelected();
                } else if(selectedAction == duplicateEffA) {
                    applyDuplicatePathEffectToSelected();
                } else if(selectedAction == discretePathEffA) {
                    applyDiscreteOutlinePathEffectToSelected();
                } else if(selectedAction == duplicatePathEffA) {
                    applyDuplicateOutlinePathEffectToSelected();
                }
            } else {

            }
        }
    }
}

void Canvas::clearHoveredEdge() {
    if(mHoveredEdge == NULL) return;
    mHoveredEdge = NULL;
}

void Canvas::handleMovePointMousePressEvent() {
    if (mLastPressedPoint == NULL) {
        if(isCtrlPressed() ) {
            clearPointsSelection();
            mLastPressedPoint = createNewPointOnLineNearSelected(
                                        mLastPressPosRel,
                                        isShiftPressed(),
                                        1./mCanvasTransformMatrix.m11());

        } else {
            mCurrentEdge = getEdgeAt(mLastPressPosRel);
            if(mCurrentEdge == NULL) {
                mSelecting = true;
                startSelectionAtPoint(mLastMouseEventPosRel);
            } else {
                clearPointsSelection();
                clearCurrentEndPoint();
                clearLastPressedPoint();
            }
        }
        clearHoveredEdge();
    } else {
        if(mLastPressedPoint->isSelected()) {
            return;
        }
        if(!isShiftPressed() &&
           !(mLastPressedPoint->isCtrlPoint())) {
            clearPointsSelection();
        }
        if(mLastPressedPoint->isCtrlPoint() ) {
            addPointToSelection(mLastPressedPoint);
        }
    }
}

#include "Boxes/particlebox.h"
void Canvas::handleLeftButtonMousePress() {
    if(mIsMouseGrabbing) {
        //handleMouseRelease(event->pos());
        //releaseMouseAndDontTrack();
        return;
    }

    grabMouseAndTrack();

    mDoubleClick = false;
    mMovesToSkip = 2;
    mFirstMouseMove = true;

    mLastPressPosRel = mLastMouseEventPosRel;
    mLastPressedPoint = getPointAtAbsPos(mLastMouseEventPosRel,
                                   mCurrentMode,
                                   1./mCanvasTransformMatrix.m11());

    if(mRotPivot->handleMousePress(mLastMouseEventPosRel,
                                   1./mCanvasTransformMatrix.m11())) {
    } else if(isMovingPath()) {
        if(mHoveredPoint == NULL) {
            handleMovePathMousePressEvent();
        } else {
            handleMovePointMousePressEvent();
        }
    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        mLastPressedBox = getPathAtFromAllAncestors(mLastPressPosRel);
    } else {
        if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMousePress();
        } // point adding mode
        else if (mCurrentMode == CanvasMode::MOVE_POINT) {
            handleMovePointMousePressEvent();
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {

            Circle *newPath = new Circle(mCurrentBoxesGroup);
            newPath->setAbsolutePos(mLastMouseEventPosRel, false);
            //newPath->startAllPointsTransform();
            clearBoxesSelection();
            addBoxToSelection(newPath);

            mCurrentCircle = newPath;

        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            Rectangle *newPath = new Rectangle(mCurrentBoxesGroup);
            newPath->setAbsolutePos(mLastMouseEventPosRel, false);
            //newPath->startAllPointsTransform();
            clearBoxesSelection();
            addBoxToSelection(newPath);

            mCurrentRectangle = newPath;
        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {

            TextBox *newPath = new TextBox(mCurrentBoxesGroup);
            newPath->setAbsolutePos(mLastMouseEventPosRel, false);

            mCurrentTextBox = newPath;

            clearBoxesSelection();
            addBoxToSelection(newPath);

        } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
            //setCanvasMode(CanvasMode::MOVE_POINT);
            ParticleBox *partBox = new ParticleBox(mCurrentBoxesGroup);
            partBox->setAbsolutePos(mLastMouseEventPosRel, false);
            clearBoxesSelection();
            addBoxToSelection(partBox);

            mLastPressedPoint = partBox->getBottomRightPoint();
        } else if(mCurrentMode == CanvasMode::ADD_PARTICLE_EMITTER) {
            Q_FOREACH(BoundingBox *box, mSelectedBoxes) {
                if(box->isParticleBox()) {
                    QPointF relPos = box->mapAbsPosToRel(mLastMouseEventPosRel);
                    if(box->getRelBoundingRectPath().contains(relPos.x(),
                                                              relPos.y())) {
                        ((ParticleBox*)box)->addEmitterAtAbsPos(
                                    mLastMouseEventPosRel);
                        break;
                    }
                }
            }
        }
    } // current mode allows interaction with points
}

QPointF Canvas::mapCanvasAbsToRel(const QPointF &pos) {
    return mCanvasTransformMatrix.inverted().map(pos);
}

void Canvas::setLastMouseEventPosAbs(const QPointF &abs) {
    mLastMouseEventPosAbs = abs;
    mLastMouseEventPosRel = mapCanvasAbsToRel(mLastMouseEventPosAbs);
}

void Canvas::setLastMousePressPosAbs(const QPointF &abs) {
    mLastPressPosAbs = abs;
    mLastPressPosRel = mapCanvasAbsToRel(mLastMouseEventPosAbs);
}

void Canvas::setCurrentMouseEventPosAbs(const QPointF &abs) {
    mCurrentMouseEventPosAbs = abs;
    mCurrentMouseEventPosRel = mapCanvasAbsToRel(mCurrentMouseEventPosAbs);
}

void Canvas::setCurrentMousePressPosAbs(const QPointF &abs) {
    mCurrentPressPosAbs = abs;
    mCurrentPressPosRel = mapCanvasAbsToRel(mCurrentMouseEventPosAbs);
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(isPreviewingOrRendering()) return;
    setLastMouseEventPosAbs(event->pos());
    if(event->button() != Qt::LeftButton) {
        if(event->button() == Qt::RightButton) {
            handleRightButtonMousePress(event);
        }
    } else {
        handleLeftButtonMousePress();
    }

    callUpdateSchedulers();
}

void Canvas::cancelCurrentTransform() {
    mTransformationFinishedBeforeMouseRelease = true;
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        cancelSelectedPointsTransform();
        if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
            mRotPivot->handleMouseRelease();
        }
    } else if(mCurrentMode == CanvasMode::MOVE_PATH) {
        if(mRotPivot->isSelected()) {
            mRotPivot->cancelTransform();
        } else {
            cancelSelectedBoxesTransform();
            if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
                mRotPivot->handleMouseRelease();
            }
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {

    } else if(mCurrentMode == PICK_PATH_SETTINGS) {
        mCanvasWindow->setCanvasMode(MOVE_PATH);
    }

    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
}

void Canvas::handleMovePointMouseRelease() {
    if(mRotPivot->isSelected()) {
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        finishSelectedPointsTransform();
    } else if(mSelecting) {
        mSelecting = false;
        if(mFirstMouseMove) {
            mLastPressedBox = mCurrentBoxesGroup->getBoxAt(mCurrentMouseEventPosRel);
            if((mLastPressedBox == NULL) ? true : mLastPressedBox->isGroup()) {
                BoundingBox *pressedBox = getPathAtFromAllAncestors(mCurrentMouseEventPosRel);
                if(pressedBox == NULL) {
                    if(!(isShiftPressed()) ) {
                        clearPointsSelectionOrDeselect();
                    }
                } else {
                    clearPointsSelection();
                    clearCurrentEndPoint();
                    clearLastPressedPoint();
                    setCurrentBoxesGroup((BoxesGroup*) pressedBox->getParent());
                    addBoxToSelection(pressedBox);
                }
            }
            if(mLastPressedBox != NULL) {
                if(isShiftPressed()) {
                    if(mLastPressedBox->isSelected()) {
                        removeBoxFromSelection(mLastPressedBox);
                    } else {
                        addBoxToSelection(mLastPressedBox);
                    }
                } else {
                    clearPointsSelection();
                    clearCurrentEndPoint();
                    clearLastPressedPoint();
                    selectOnlyLastPressedBox();
                }
            }
            return;
        }
        if(!isShiftPressed()) clearPointsSelection();
        moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        selectAndAddContainedPointsToSelection(mSelectionRect);
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isSelected()) {
                    removePointFromSelection(mLastPressedPoint);
                } else {
                    addPointToSelection(mLastPressedPoint);
                }
            }
        } else {
            selectOnlyLastPressedPoint();
        }
    } else {
        finishSelectedPointsTransform();
        if(mLastPressedPoint != NULL) {
            if(mLastPressedPoint->isCtrlPoint() ) {
                removePointFromSelection(mLastPressedPoint);
            }
        }
    }
}


void Canvas::handleMovePathMouseRelease() {
    if(mRotPivot->isSelected()) {
        if(!mFirstMouseMove) {
            mRotPivot->finishTransform();
        }
        mRotPivot->deselect();
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseRelease();
        finishSelectedBoxesTransform();
    } else if(mFirstMouseMove) {
        mSelecting = false;
        if(isShiftPressed() && mLastPressedBox != NULL) {
            if(mLastPressedBox->isSelected()) {
                removeBoxFromSelection(mLastPressedBox);
            } else {
                addBoxToSelection(mLastPressedBox);
            }
        } else {
            selectOnlyLastPressedBox();
        }
    } else if(mSelecting) {
        moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        mCurrentBoxesGroup->addContainedBoxesToSelection(mSelectionRect);
        mSelecting = false;
    } else {
        finishSelectedBoxesTransform();
    }
}

void Canvas::handleAddPointMousePress() {
    if(mCurrentEndPoint != NULL) {
        if(mCurrentEndPoint->isHidden()) {
            setCurrentEndPoint(NULL);
        }
    }
    PathPoint *pathPointUnderMouse = (PathPoint*) mLastPressedPoint;
    if( (pathPointUnderMouse == NULL) ? false :
            !pathPointUnderMouse->isEndPoint() ) {
        pathPointUnderMouse = NULL;
    }
    if(pathPointUnderMouse == mCurrentEndPoint &&
            pathPointUnderMouse != NULL) {
        return;
    }
    if(mCurrentEndPoint == NULL && pathPointUnderMouse == NULL) {

        VectorPath *newPath = new VectorPath(mCurrentBoxesGroup);
        clearBoxesSelection();
        addBoxToSelection(newPath);
        PathAnimator *newPathAnimator = newPath->getPathAnimator();
        SinglePathAnimator *newSinglePath = new SinglePathAnimator(
                                                    newPathAnimator);
        newPathAnimator->addSinglePathAnimator(newSinglePath);
        setCurrentEndPoint(newSinglePath->
                            addPointAbsPos(mLastMouseEventPosRel,
                                            mCurrentEndPoint) );
    } else {
        if(pathPointUnderMouse == NULL) {
            PathPoint *newPoint =
                    mCurrentEndPoint->addPointAbsPos(mLastMouseEventPosRel);
            //newPoint->startTransform();
            setCurrentEndPoint(newPoint);
        } else if(mCurrentEndPoint == NULL) {
            setCurrentEndPoint(pathPointUnderMouse);
        } else {
            //pathPointUnderMouse->startTransform();
            if(mCurrentEndPoint->getParentPath() ==
               pathPointUnderMouse->getParentPath()) {
                mCurrentEndPoint->getParentPath()->
                        connectPoints(mCurrentEndPoint,
                                      pathPointUnderMouse);
            }
            else {
                connectPointsFromDifferentPaths(mCurrentEndPoint,
                                                pathPointUnderMouse);
            }
            setCurrentEndPoint(pathPointUnderMouse);
        }
    } // pats is not null
}

void Canvas::handleAddPointMouseRelease() {
    if(mCurrentEndPoint != NULL) {
        mCurrentEndPoint->prp_updateInfluenceRangeAfterChanged();
        if(!mCurrentEndPoint->isEndPoint()) {
            setCurrentEndPoint(NULL);
        }
    }
}

void Canvas::handleMouseRelease() {
    if(mIsMouseGrabbing) {
        releaseMouseAndDontTrack();
    }
    if(!mDoubleClick) {
        if(mCurrentMode == CanvasMode::MOVE_POINT ||
           mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
            handleMovePointMouseRelease();
            if(mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
                mCanvasWindow->setCanvasMode(CanvasMode::ADD_PARTICLE_EMITTER);
            }
        } else if(isMovingPath()) {
            if(mLastPressedPoint == NULL) {
                handleMovePathMouseRelease();
            } else {
                handleMovePointMouseRelease();
                clearPointsSelection();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseRelease();
        } else if(mCurrentMode == PICK_PATH_SETTINGS) {
            if(mLastPressedBox != NULL) {
                mFillStrokeSettingsWidget->loadSettingsFromPath(
                            (VectorPath*) mLastPressedBox);
            }
            mCanvasWindow->setCanvasMode(MOVE_PATH);
        } else if(mCurrentMode == CanvasMode::ADD_TEXT) {
            if(mCurrentTextBox != NULL) {
                mCurrentTextBox->openTextEditor();
            }
        }
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if(isPreviewingOrRendering() || event->button() == Qt::MiddleButton) return;
    setCurrentMouseEventPosAbs(event->pos());
    mXOnlyTransform = false;
    mYOnlyTransform = false;
    if(mInputTransformationEnabled) {
        mFirstMouseMove = false;
    }
    clearAndDisableInput();
    if(mTransformationFinishedBeforeMouseRelease) {
        mTransformationFinishedBeforeMouseRelease = false;
    } else {
        handleMouseRelease();
    }

    mLastPressedBox = NULL;
    mHoveredPoint = mLastPressedPoint;
    mLastPressedPoint = NULL;

    if(mCurrentEdge != NULL) {
        if(!mFirstMouseMove) {
            mCurrentEdge->finishPassThroughTransform();
        }
        mHoveredEdge = mCurrentEdge;
        mHoveredEdge->generatePainterPath();
        mCurrentEdge = NULL;
    }

    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
}

QPointF Canvas::getMoveByValueForEventPos(const QPointF &eventPos) {
    QPointF moveByPoint = eventPos - mLastPressPosRel;
    if(mInputTransformationEnabled) {
        moveByPoint = QPointF(mInputTransformationValue,
                              mInputTransformationValue);
    }
    if(mYOnlyTransform) {
        moveByPoint.setX(0.);
    } else if(mXOnlyTransform) {
        moveByPoint.setY(0.);
    }
    return moveByPoint;
}

void Canvas::handleMovePointMouseMove() {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }
        mRotPivot->moveByAbs(getMoveByValueForEventPos(mCurrentMouseEventPosRel));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
           mRotPivot->handleMouseMove(mCurrentMouseEventPosRel,
                                      mLastPressPosRel,
                                      mXOnlyTransform, mYOnlyTransform,
                                      mInputTransformationEnabled,
                                      mInputTransformationValue,
                                      mFirstMouseMove,
                                      mCurrentMode);
    } else if(mCurrentEdge != NULL) {
        if(mFirstMouseMove) {
            mCurrentEdge->startPassThroughTransform();
        }
        mCurrentEdge->makePassThrough(mCurrentMouseEventPosRel);
    } else {
        if(mLastPressedPoint != NULL) {
            addPointToSelection(mLastPressedPoint);

            if(mLastPressedPoint->isCtrlPoint() ) {
                if(mFirstMouseMove) {
                    mLastPressedPoint->startTransform();
                }
                mLastPressedPoint->moveByAbs(
                        getMoveByValueForEventPos(mCurrentMouseEventPosRel));
                return;//
            }/* else {
                mCurrentBoxesGroup->moveSelectedPointsBy(getMoveByValueForEventPos(eventPos),
                                                         mFirstMouseMove);
            }*/
        }
        moveSelectedPointsByAbs(
                    getMoveByValueForEventPos(mCurrentMouseEventPosRel),
                    mFirstMouseMove);
    }
}

void Canvas::handleMovePathMouseMove() {
    if(mRotPivot->isSelected()) {
        if(mFirstMouseMove) {
            mRotPivot->startTransform();
        }

        mRotPivot->moveByAbs(getMoveByValueForEventPos(mCurrentMouseEventPosRel));
    } else if(mRotPivot->isRotating() || mRotPivot->isScaling() ) {
        mRotPivot->handleMouseMove(mCurrentMouseEventPosRel,
                                   mLastPressPosRel,
                                   mXOnlyTransform, mYOnlyTransform,
                                   mInputTransformationEnabled,
                                   mInputTransformationValue,
                                   mFirstMouseMove,
                                   mCurrentMode);
    } else {
        if(mLastPressedBox != NULL) {
            addBoxToSelection(mLastPressedBox);
            mLastPressedBox = NULL;
        }

        moveSelectedBoxesByAbs(
                    getMoveByValueForEventPos(
                        mCurrentMouseEventPosRel),
                    mFirstMouseMove);
    }
}

void Canvas::handleAddPointMouseMove() {
    if(mCurrentEndPoint == NULL) return;
    if(mCurrentEndPoint->getCurrentCtrlsMode() !=
       CtrlsMode::CTRLS_SYMMETRIC) {
        mCurrentEndPoint->setCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC, false);
    }
    mCurrentEndPoint->moveEndCtrlPtToAbsPos(mLastMouseEventPosRel);
}

void Canvas::updateTransformation() {
    if(mSelecting) {
        moveSecondSelectionPoint(mLastMouseEventPosRel);
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMouseMove();
    } else if(isMovingPath()) {
        if(mLastPressedPoint == NULL) {
            handleMovePathMouseMove();
        } else {
            handleMovePointMouseMove();
        }
    } else if(mCurrentMode == CanvasMode::ADD_POINT) {
        handleAddPointMouseMove();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if(isPreviewingOrRendering()) return;
    setCurrentMouseEventPosAbs(event->pos());
    if(!(event->buttons() & Qt::MiddleButton) &&
       !(event->buttons() & Qt::RightButton) &&
       !(event->buttons() & Qt::LeftButton) &&
       !mIsMouseGrabbing) {
        MovablePoint *lastHoveredPoint = mHoveredPoint;
        updateHoveredPoint();

        if(mRotPivot->isVisible() && mHoveredPoint == NULL) {
            if(mRotPivot->isPointAtAbsPos(
                        mCurrentMouseEventPosRel,
                        1./mCanvasTransformMatrix.m11()) ) {
                mHoveredPoint = mRotPivot;
            }
        }

        BoundingBox *lastHoveredBox = mHoveredBox;
        updateHoveredBox();

        VectorPathEdge *lastEdge = mHoveredEdge;
        clearHoveredEdge();
        if(mCurrentMode == MOVE_POINT) {
            updateHoveredEdge();
        }

        if(mHoveredPoint != lastHoveredPoint ||
           mHoveredBox != lastHoveredBox ||
           mHoveredEdge != lastEdge) {
            callUpdateSchedulers();
        }
        return;
    }

    if(mMovesToSkip > 0) {
        mMovesToSkip--;
        return;
    }

    if(event->buttons() & Qt::MiddleButton) {
        moveByRel(mCurrentMouseEventPosRel - mLastMouseEventPosRel);
    } else if(!mTransformationFinishedBeforeMouseRelease) {
        if(mSelecting) {
            moveSecondSelectionPoint(mCurrentMouseEventPosRel);
        } else if(mCurrentMode == CanvasMode::MOVE_POINT ||
                  mCurrentMode == CanvasMode::ADD_PARTICLE_BOX) {
            handleMovePointMouseMove();
        } else if(isMovingPath()) {
            if(mLastPressedPoint == NULL) {
                handleMovePathMouseMove();
            } else {
                handleMovePointMouseMove();
            }
        } else if(mCurrentMode == CanvasMode::ADD_POINT) {
            handleAddPointMouseMove();
        } else if(mCurrentMode == CanvasMode::ADD_CIRCLE) {
            if(isShiftPressed() ) {
                qreal lenR = pointToLen(mCurrentMouseEventPosRel -
                                        mLastPressPosRel);
                mCurrentCircle->moveRadiusesByAbs(QPointF(lenR, lenR));
            } else {
                mCurrentCircle->moveRadiusesByAbs(mCurrentMouseEventPosRel -
                                                  mLastPressPosRel);
            }
        } else if(mCurrentMode == CanvasMode::ADD_RECTANGLE) {
            if(isShiftPressed()) {
                QPointF trans = mCurrentMouseEventPosRel - mLastPressPosRel;
                qreal valF = qMax(trans.x(), trans.y() );
                trans = QPointF(valF, valF);
                mCurrentRectangle->moveSizePointByAbs(trans);
            } else {
                mCurrentRectangle->moveSizePointByAbs(mCurrentMouseEventPosRel -
                                                      mLastPressPosRel);
            }
        }
    }
    mLastMouseEventPosRel = mCurrentMouseEventPosRel;
    mFirstMouseMove = false;
    setLastMouseEventPosAbs(event->pos());
    callUpdateSchedulers();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    if(isPreviewingOrRendering()) return;
    if(event->delta() > 0) {
        zoomCanvas(1.1, event->posF());
    } else {
        zoomCanvas(0.9, event->posF());
    }
    mVisibleHeight = mCanvasTransformMatrix.m22()*mHeight;
    mVisibleWidth = mCanvasTransformMatrix.m11()*mWidth;
    
    if(mHoveredEdge != NULL) {
        mHoveredEdge->generatePainterPath();
    }

    callUpdateSchedulers();
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *) {
    mDoubleClick = true;

    mLastPressedPoint = createNewPointOnLineNearSelected(
                                    mLastPressPosRel,
                                    true,
                                    1./mCanvasTransformMatrix.m11());

    if(mLastPressedPoint == NULL) {
        BoundingBox *boxAt = mCurrentBoxesGroup->getBoxAt(mLastPressPosRel);
        if(boxAt == NULL) {
            if(mCurrentBoxesGroup != this) {
                setCurrentBoxesGroup((BoxesGroup*)
                                     mCurrentBoxesGroup->getParent());
            }
        } else {
            if(boxAt->isGroup()) {
                setCurrentBoxesGroup((BoxesGroup*) boxAt);
            } else if(boxAt->isText()) {
                ((TextBox*) boxAt)->openTextEditor();
            } else if(mCurrentMode == MOVE_PATH) {
                mCanvasWindow->setCanvasMode(MOVE_PATH);
            }
        }
    }
    
    callUpdateSchedulers();
}
