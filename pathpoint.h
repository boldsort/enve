#ifndef PATHPOINT_H
#define PATHPOINT_H
#include "movablepoint.h"
#include <QSqlQuery>

class UndoRedoStack;

class VectorPath;

class CtrlPoint;

enum CanvasMode : short;

enum CtrlsMode {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};

class PathPoint : public MovablePoint
{
public:
    PathPoint(QPointF absPos, VectorPath *vectorPath);
    PathPoint(QPointF absPos, QPointF startCtrlAbsPos, QPointF endCtrlAbsPos, VectorPath *vectorPath);

    void startTransform();
    void finishTransform();

    void moveBy(QPointF absTranslation);
    void moveToAbs(QPointF absPos);

    QPointF getStartCtrlPtAbsPos();
    QPointF getStartCtrlPtValue();
    MovablePoint *getStartCtrlPt();

    QPointF getEndCtrlPtAbsPos();
    QPointF getEndCtrlPtValue();
    MovablePoint *getEndCtrlPt();

    void draw(QPainter *p, CanvasMode mode);

    PathPoint *getNextPoint();
    PathPoint *getPreviousPoint();

    bool isEndPoint();

    void setPointAsPrevious(PathPoint *pointToSet);
    void setPointAsNext(PathPoint *pointToSet);
    void setNextPoint(PathPoint *mNextPoint, bool saveUndoRedo = true);
    void setPreviousPoint(PathPoint *mPreviousPoint, bool saveUndoRedo = true);

    bool hasNextPoint();
    bool hasPreviousPoint();

    PathPoint *addPointAbsPos(QPointF absPos);
    PathPoint *addPoint(PathPoint *pointToAdd);

    void connectToPoint(PathPoint *point);
    void disconnectFromPoint(PathPoint *point);

    void remove();

    MovablePoint *getPointAtAbsPos(QPointF absPos, CanvasMode canvasMode);
    void rectPointsSelection(QRectF absRect, QList<MovablePoint *> *list);
    void updateStartCtrlPtVisibility();
    void updateEndCtrlPtVisibility();

    void setSeparatePathPoint(bool separatePathPoint);
    bool isSeparatePathPoint();

    void setCtrlsMode(CtrlsMode mode, bool saveUndoRedo = true);
    QPointF symmetricToAbsPos(QPointF absPosToMirror);
    QPointF symmetricToAbsPosNewLen(QPointF absPosToMirror, qreal newLen);
    void ctrlPointPosChanged(bool startPtChanged);
    void moveEndCtrlPtToAbsPos(QPointF endCtrlPt);
    void moveStartCtrlPtToAbsPos(QPointF startCtrlPt);
    void moveEndCtrlPtToRelPos(QPointF endCtrlPt);
    void moveStartCtrlPtToRelPos(QPointF startCtrlPt);
    void setCtrlPtEnabled(bool enabled, bool isStartPt, bool saveUndoRedo = true);
    VectorPath *getParentPath();
    void setRelativePos(QPointF relPos, bool saveUndoRedo = true);

    void saveToQuery(int vectorPathId);
    PathPoint(qreal relPosX, qreal relPosy,
              qreal startCtrlRelX, qreal startCtrlRelY,
              qreal endCtrlRelX, qreal endCtrlRelY, bool isFirst, VectorPath *vectorPath);
private:
    VectorPath *mVectorPath;
    CtrlsMode mCtrlsMode = CtrlsMode::CTRLS_SYMMETRIC;

    bool mSeparatePathPoint = false;
    PathPoint *mNextPoint = NULL;
    PathPoint *mPreviousPoint = NULL;
    bool mStartCtrlPtEnabled = true;
    CtrlPoint *mStartCtrlPt;
    bool mEndCtrlPtEnabled = true;
    CtrlPoint *mEndCtrlPt;
    void ctrlPointPosChanged(CtrlPoint *pointChanged, CtrlPoint *pointToUpdate);
    void setEndCtrlPtEnabled(bool enabled);
    void setStartCtrlPtEnabled(bool enabled);
};

#endif // PATHPOINT_H
