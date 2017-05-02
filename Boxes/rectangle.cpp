#include "Boxes/rectangle.h"
#include "canvas.h"

Rectangle::Rectangle(BoxesGroup *parent) : PathBox(parent, TYPE_RECTANGLE)
{
    setName("Rectangle");

    mTopLeftPoint = new RectangleTopLeftPoint(this);
    mTopLeftPoint->setRelativePos(QPointF(0., 0.), false);
    mBottomRightPoint = new RectangleBottomRightPoint(this);
    mBottomRightPoint->setRelativePos(QPointF(0., 0.), false);

    //mTopLeftPoint->setBottomRightPoint(mBottomRightPoint);
    //mBottomRightPoint->setRadiusPoint(mRadiusPoint);

    mTopLeftPoint->prp_setName("top left");
    mBottomRightPoint->prp_setName("bottom right");

    ca_addChildAnimator(mTopLeftPoint);
    ca_addChildAnimator(mBottomRightPoint);

    mRadiusAnimator.prp_setName("radius");
    ca_addChildAnimator(&mRadiusAnimator);

    prp_setUpdater(new PathPointUpdater(this));
}

Rectangle::~Rectangle()
{

}

#include <QSqlError>
int Rectangle::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = PathBox::prp_saveToSql(query, parentId);

    int bottomRightPointId = mTopLeftPoint->prp_saveToSql(query);
    int topLeftPointId = mTopLeftPoint->prp_saveToSql(query);
    int radiusPointId = mRadiusAnimator.prp_saveToSql(query);

    if(!query->exec(QString("INSERT INTO rectangle (boundingboxid, "
                           "topleftpointid, bottomrightpointid, "
                           "radiuspointid) "
                "VALUES (%1, %2, %3, %4)").
                arg(boundingBoxId).
                arg(topLeftPointId).
                arg(bottomRightPointId).
                arg(radiusPointId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}

void Rectangle::duplicateRectanglePointsFrom(
        RectangleTopLeftPoint *topLeftPoint,
        RectangleBottomRightPoint *bottomRightPoint,
        QrealAnimator *radiusAnimator) {
    topLeftPoint->makeDuplicate(mTopLeftPoint);
    bottomRightPoint->makeDuplicate(mBottomRightPoint);
    radiusAnimator->makeDuplicate(&mRadiusAnimator);
}

BoundingBox *Rectangle::createNewDuplicate(BoxesGroup *parent) {
    return new Rectangle(parent);
}


void Rectangle::prp_loadFromSql(const int &boundingBoxId) {
    PathBox::prp_loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM rectangle WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idBottomRightPointId = query.record().indexOf("bottomrightpointid");
        int idTopLeftPointId = query.record().indexOf("topleftpointid");
        int idRadiusPointId = query.record().indexOf("radiuspointid");

        int bottomRightPointId = query.value(idBottomRightPointId).toInt();
        int topLeftPointId = query.value(idTopLeftPointId).toInt();
        int radiusPointId = query.value(idRadiusPointId).toInt();

        mBottomRightPoint->prp_loadFromSql(bottomRightPointId);
        mTopLeftPoint->prp_loadFromSql(topLeftPointId);
        mRadiusAnimator.prp_loadFromSql(radiusPointId);
    } else {
        qDebug() << "Could not load rectangle with id " << boundingBoxId;
    }

    if(!mPivotChanged) centerPivotPosition();
}

void Rectangle::startAllPointsTransform() {
    mTopLeftPoint->startTransform();
    mBottomRightPoint->startTransform();
    startTransform();
}

void Rectangle::finishAllPointsTransform() {
    mTopLeftPoint->finishTransform();
    mBottomRightPoint->finishTransform();
    finishTransform();
}

void Rectangle::moveSizePointByAbs(QPointF absTrans) {
    mBottomRightPoint->moveByAbs(absTrans);
}

MovablePoint *Rectangle::getBottomRightPoint() {
    return mBottomRightPoint;
}

void Rectangle::drawSelected(QPainter *p,
                             const CanvasMode &currentCanvasMode) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            mTopLeftPoint->draw(p);
            mBottomRightPoint->draw(p);

            mFillGradientPoints->drawGradientPoints(p);
            mStrokeGradientPoints->drawGradientPoints(p);
        }
        p->restore();
    }
}


MovablePoint *Rectangle::getPointAtAbsPos(const QPointF &absPtPos,
                                    const CanvasMode &currentCanvasMode,
                                    const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints->qra_getPointAt(absPtPos,
                                                              canvasScaleInv);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints->qra_getPointAt(absPtPos,
                                                                canvasScaleInv);
        }
    }
    if(pointToReturn == NULL) {
        if(mTopLeftPoint->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return mTopLeftPoint;
        }
        if(mBottomRightPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mBottomRightPoint;
        }
    }
    return pointToReturn;
}

void Rectangle::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                  QList<MovablePoint *> *list) {
    if(!mTopLeftPoint->isSelected()) {
        if(mTopLeftPoint->isContainedInRect(absRect)) {
            mTopLeftPoint->select();
            list->append(mTopLeftPoint);
        }
    }
    if(!mBottomRightPoint->isSelected()) {
        if(mBottomRightPoint->isContainedInRect(absRect)) {
            mBottomRightPoint->select();
            list->append(mBottomRightPoint);
        }
    }
}

void Rectangle::updatePath() {
    mPath = QPainterPath();
    QPointF topPos = mTopLeftPoint->getRelativePos();
    QPointF botPos = mBottomRightPoint->getRelativePos();
    qreal radius = mRadiusAnimator.qra_getCurrentValue();
    mPath.addRoundedRect(QRectF(topPos, botPos),
                         radius, radius);

    updateOutlinePath();
}

RectangleTopLeftPoint::RectangleTopLeftPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

}

RectangleBottomRightPoint::RectangleBottomRightPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

}
