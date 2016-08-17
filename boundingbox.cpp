#include "boundingbox.h"
#include "canvas.h"
#include "undoredo.h"
#include <QDebug>

BoundingBox::BoundingBox(BoundingBox *parent, BoundingBoxType type) :
    ConnectedToMainWindow(parent)
{
    mType = type;
    parent->addChild(this);
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
    updateCombinedTransform();
}

BoundingBox::BoundingBox(int boundingBoxId,
                         BoundingBox *parent, BoundingBoxType type) :
    ConnectedToMainWindow(parent) {
    QSqlQuery query;
    mType = type;

    QString queryStr = "SELECT * FROM boundingbox WHERE id = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr)) {
        query.next();
        int idM11 = query.record().indexOf("m11_trans");
        int idM12 = query.record().indexOf("m12_trans");
        int idM21 = query.record().indexOf("m21_trans");
        int idM22 = query.record().indexOf("m22_trans");
        int idDx = query.record().indexOf("dx_trans");
        int idDy = query.record().indexOf("dy_trans");
        int idPivotx = query.record().indexOf("pivotx");
        int idPivoty = query.record().indexOf("pivoty");
        qreal m11 = query.value(idM11).toReal();
        qreal m12 = query.value(idM12).toReal();
        qreal m21 = query.value(idM21).toReal();
        qreal m22 = query.value(idM22).toReal();
        qreal dx = query.value(idDx).toReal();
        qreal dy = query.value(idDy).toReal();
        qreal pivotX = query.value(idPivotx).toReal();
        qreal pivotY = query.value(idPivoty).toReal();
        mTransformMatrix.setMatrix(m11, m12, m21, m22, dx, dy);
        mRelRotPivotPos.setX(pivotX);
        mRelRotPivotPos.setY(pivotY);
    } else {
        qDebug() << "Could not load boundingbox with id " << boundingBoxId;
    }

    parent->addChild(this);
    updateCombinedTransform();
}

void BoundingBox::setParent(BoundingBox *parent, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new SetBoxParentUndoRedo(this, mParent, parent));
    }
    mParent = parent;
    if(mParent == NULL) {
        scheduleRemoval();
    } else {
        descheduleRemoval();
    }
    updateCombinedTransform();
}

BoundingBox *BoundingBox::getParent()
{
    return mParent;
}

bool BoundingBox::isGroup()
{
    return mType == TYPE_GROUP;
}

void BoundingBox::setPivotRelPos(QPointF relPos, bool saveUndoRedo, bool pivotChanged) {
    if(saveUndoRedo) {
        addUndoRedo(new SetPivotRelPosUndoRedo(this, mRelRotPivotPos, relPos,
                                               mPivotChanged, pivotChanged));
    }
    mPivotChanged = pivotChanged;
    mRelRotPivotPos = relPos;
    schedulePivotUpdate();
}

void BoundingBox::scheduleRemoval()
{
    mScheduledForRemove = true;
}

void BoundingBox::descheduleRemoval()
{
    mScheduledForRemove = false;
}

bool BoundingBox::isScheduldedForRemoval()
{
    return mScheduledForRemove;
}

void BoundingBox::setPivotAbsPos(QPointF absPos, bool saveUndoRedo, bool pivotChanged) {
    QPointF newPos = getCombinedTransform().inverted().map(absPos);
    if(saveUndoRedo) {
        addUndoRedo(new SetPivotRelPosUndoRedo(this, mRelRotPivotPos, newPos,
                                               mPivotChanged, pivotChanged));
    }
    mPivotChanged = pivotChanged;
    mRelRotPivotPos = newPos;
    schedulePivotUpdate();
}

void BoundingBox::applyTransformation(QMatrix transformation)
{
    startTransform();
    mTransformMatrix = mTransformMatrix*transformation;
    finishTransform();
}

QPointF BoundingBox::getPivotAbsPos()
{
    return getCombinedTransform().map(mRelRotPivotPos);
}

bool BoundingBox::isSelected()
{
    return mSelected;
}

void BoundingBox::select()
{
    mSelected = true;
    scheduleRepaint();
}

void BoundingBox::deselect()
{
    mSelected = false;
    scheduleRepaint();
}

BoundingBox::BoundingBox(MainWindow *window, BoundingBoxType type) : ConnectedToMainWindow(window)
{
    mType = type;
    mTransformMatrix.reset();
    mCombinedTransformMatrix.reset();
}

bool BoundingBox::isContainedIn(QRectF absRect)
{
    return absRect.contains(getBoundingRect());
}

BoundingBox *BoundingBox::getBoxAtFromAllAncestors(QPointF absPos) {
    if(pointInsidePath(absPos)) {
        return this;
    } else {
        return NULL;
    }
}

PaintSettings BoundingBox::getFillSettings()
{
    return PaintSettings();
}

StrokeSettings BoundingBox::getStrokeSettings()
{
    return StrokeSettings();
}

qreal BoundingBox::getCurrentCanvasScale()
{
     return mParent->getCurrentCanvasScale();
}

void BoundingBox::drawBoundingRect(QPainter *p) {
    QPen pen = p->pen();
    p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
    p->setBrush(Qt::NoBrush);
    p->drawRect(getBoundingRect());
    p->setPen(pen);
}

QMatrix BoundingBox::getCombinedTransform()
{
    return mCombinedTransformMatrix;
}

QPointF BoundingBox::getTranslation()
{
    return QPointF(mTransformMatrix.dx(), mTransformMatrix.dy());
}

void BoundingBox::scale(qreal scaleBy, QPointF absOrigin)
{
    scale(scaleBy, scaleBy, absOrigin);
}

void BoundingBox::scaleCenter(qreal scaleBy)
{
    scaleCenter(scaleBy, scaleBy);
}

void BoundingBox::scaleCenter(qreal scaleXBy, qreal scaleYBy)
{
    QPointF absOrigin = getBoundingRect().center();
    scaleFromSaved(scaleXBy, scaleYBy, absOrigin);
}

void BoundingBox::scaleRight(qreal scaleXBy)
{
    QPointF absOrigin = QPointF(getBoundingRect().right(), 1.f);
    scaleFromSaved(scaleXBy, 1.f, absOrigin);
}

void BoundingBox::scaleRightFixedRatio(qreal scaleXBy)
{
    QPointF absOrigin = QPointF(getBoundingRect().right(), getBoundingRect().center().y());
    scaleFromSaved(scaleXBy, scaleXBy, absOrigin);
}

void BoundingBox::scaleLeft(qreal scaleXBy)
{
    QPointF absOrigin = QPointF(getBoundingRect().left(), 1.f);
    scaleFromSaved(scaleXBy, 1.f, absOrigin);
}

void BoundingBox::scaleLeftFixedRatio(qreal scaleXBy)
{
    QPointF absOrigin = QPointF(getBoundingRect().left(), getBoundingRect().center().y());
    scaleFromSaved(scaleXBy, scaleXBy, absOrigin);
}

void BoundingBox::scaleTop(qreal scaleYBy)
{
    QPointF absOrigin = QPointF(1.f, getBoundingRect().top() );
    scaleFromSaved(1.f, scaleYBy, absOrigin);
}

void BoundingBox::scaleTopFixedRatio(qreal scaleYBy)
{
    QPointF absOrigin = QPointF(getBoundingRect().center().x(), getBoundingRect().top());
    scaleFromSaved(scaleYBy, scaleYBy, absOrigin);
}

void BoundingBox::scaleBottom(qreal scaleYBy)
{
    QPointF absOrigin = QPointF(1.f, getBoundingRect().bottom() );
    scaleFromSaved(1.f, scaleYBy, absOrigin);
}

void BoundingBox::scaleBottomFixedRatio(qreal scaleYBy)
{
    QPointF absOrigin = QPointF(getBoundingRect().center().x(), getBoundingRect().bottom());
    scaleFromSaved(scaleYBy, scaleYBy, absOrigin);
}


void BoundingBox::scaleBottomRight(qreal scaleXBy, qreal scaleYBy)
{
    QPointF absOrigin = getBoundingRect().bottomRight();
    scaleFromSaved(scaleXBy, scaleYBy, absOrigin);
}

void BoundingBox::scaleBottomLeft(qreal scaleXBy, qreal scaleYBy)
{
    QPointF absOrigin = getBoundingRect().bottomLeft();
    scaleFromSaved(scaleXBy, scaleYBy, absOrigin);
}

void BoundingBox::scaleTopRight(qreal scaleXBy, qreal scaleYBy)
{
    QPointF absOrigin = getBoundingRect().topRight();
    scaleFromSaved(scaleXBy, scaleYBy, absOrigin);
}

void BoundingBox::scaleTopLeft(qreal scaleXBy, qreal scaleYBy)
{
    QPointF absOrigin = getBoundingRect().topLeft();
    scaleFromSaved(scaleXBy, scaleYBy, absOrigin);
}

void BoundingBox::scaleBottomRight(qreal scaleBy)
{
    QPointF absOrigin = getBoundingRect().bottomRight();
    scaleFromSaved(scaleBy, scaleBy, absOrigin);
}

void BoundingBox::scaleBottomLeft(qreal scaleBy)
{
    QPointF absOrigin = getBoundingRect().bottomLeft();
    scaleFromSaved(scaleBy, scaleBy, absOrigin);
}

void BoundingBox::scaleTopRight(qreal scaleBy)
{
    QPointF absOrigin = getBoundingRect().topRight();
    scaleFromSaved(scaleBy, scaleBy, absOrigin);
}

void BoundingBox::scaleTopLeft(qreal scaleBy)
{
    QPointF absOrigin = getBoundingRect().topLeft();
    scaleFromSaved(scaleBy, scaleBy, absOrigin);
}

void BoundingBox::scaleFromSaved(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin)
{
    QPointF transPoint = -getCombinedTransform().inverted().map(absOrigin);

    mTransformMatrix = mSavedTransformMatrix;
    mTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mTransformMatrix.scale(scaleXBy, scaleYBy);
    mTransformMatrix.translate(transPoint.x(), transPoint.y());
    updateCombinedTransform();
}

int BoundingBox::saveToQuery(int parentId)
{
    QSqlQuery query;
    query.exec(QString("INSERT INTO boundingbox (boxtype, m11_trans, m12_trans, m21_trans, m22_trans, dx_trans, dy_trans, pivotx, pivoty, parentboundingboxid) "
               "VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10)").
                arg(mType).
                arg(mTransformMatrix.m11(), 0, 'f').
                arg(mTransformMatrix.m12(), 0, 'f').
                arg(mTransformMatrix.m21(), 0, 'f').
                arg(mTransformMatrix.m22(), 0, 'f').
                arg(mTransformMatrix.dx(), 0, 'f').
                arg(mTransformMatrix.dy(), 0, 'f').
                arg(mRelRotPivotPos.x(), 0, 'f').
                arg(mRelRotPivotPos.y(), 0, 'f').
                arg( (parentId == 0) ? "NULL" : QString::number(parentId) )
                );
    return query.lastInsertId().toInt();
}

void BoundingBox::scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin)
{
    QPointF transPoint = -getCombinedTransform().inverted().map(absOrigin);

    mTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mTransformMatrix.scale(scaleXBy, scaleYBy);
    mTransformMatrix.translate(transPoint.x(), transPoint.y());
    updateCombinedTransform();
}

void BoundingBox::rotateBy(qreal rot, QPointF absOrigin)
{
    QPointF transPoint = -getCombinedTransform().inverted().map(absOrigin);

    mTransformMatrix.translate(-transPoint.x(), -transPoint.y());
    mTransformMatrix.rotate(rot);
    mTransformMatrix.translate(transPoint.x(), transPoint.y());
    updateCombinedTransform();
}

void BoundingBox::moveBy(QPointF trans)
{
    trans = getCombinedTransform().inverted().map(trans) -
            getCombinedTransform().inverted().map(QPointF(0, 0));

    mTransformMatrix.translate(trans.x(), trans.y());
    updateCombinedTransform();
    schedulePivotUpdate();
}

void BoundingBox::startTransform()
{
    mSavedTransformMatrix = mTransformMatrix;
}

void BoundingBox::finishTransform()
{
    TransformChildParentUndoRedo *undoRedo = new TransformChildParentUndoRedo(this,
                                                           mSavedTransformMatrix,
                                                           mTransformMatrix);
    addUndoRedo(undoRedo);
}

void BoundingBox::cancelTransform() {
    setTransformation(mSavedTransformMatrix);
}

void BoundingBox::addChild(BoundingBox *child)
{
    startNewUndoRedoSet();
    child->setParent(this);
    addChildToListAt(mChildren.count(), child);
    updateChildrenId(mChildren.count() - 1);
    finishUndoRedoSet();
}

void BoundingBox::addChildToListAt(int index, BoundingBox *child, bool saveUndoRedo) {
    mChildren.insert(index, child);
    if(saveUndoRedo) {
        addUndoRedo(new AddChildToListUndoRedo(this, index, child));
    }
}

void BoundingBox::updateChildrenId(int firstId) {
    updateChildrenId(firstId, mChildren.length() - 1);
}

void BoundingBox::updateChildrenId(int firstId, int lastId) {
    startNewUndoRedoSet();
    for(int i = firstId; i <= lastId; i++) {
        mChildren.at(i)->setZListIndex(i);
    }
    finishUndoRedoSet();
}

void BoundingBox::removeChildFromList(int id, bool saveUndoRedo) {
    if(saveUndoRedo) {
        addUndoRedo(new RemoveChildFromListUndoRedo(this, id, mChildren.at(id)) );
    }
    mChildren.removeAt(id);
}

void BoundingBox::removeChild(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index < 0) {
        return;
    }
    startNewUndoRedoSet();
    removeChildFromList(index);
    updateChildrenId(index);
    child->setParent(NULL); // called to update
    finishUndoRedoSet();
}

void BoundingBox::moveUp()
{
    mParent->increaseChildZInList(this);
}

void BoundingBox::moveDown()
{
    mParent->decreaseChildZInList(this);
}

void BoundingBox::bringToFront()
{
    mParent->bringChildToEndList(this);
}

void BoundingBox::bringToEnd()
{
    mParent->bringChildToFrontList(this);
}

void BoundingBox::increaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, index + 1);
    updateChildrenId(index);
    finishUndoRedoSet();
}

void BoundingBox::decreaseChildZInList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, index - 1);
    updateChildrenId(index - 1);
    finishUndoRedoSet();
}

void BoundingBox::bringChildToEndList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == mChildren.count() - 1) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, mChildren.length() - 1);
    updateChildrenId(index);
    finishUndoRedoSet();
}

void BoundingBox::bringChildToFrontList(BoundingBox *child)
{
    int index = mChildren.indexOf(child);
    if(index == 0) {
        return;
    }
    startNewUndoRedoSet();
    moveChildInList(index, 0);
    updateChildrenId(0, index);
    finishUndoRedoSet();
}

void BoundingBox::moveChildInList(int from, int to, bool saveUndoRedo) {
    mChildren.move(from, to);
    if(saveUndoRedo) {
        addUndoRedo(new MoveChildInListUndoRedo(from, to, this) );
    }
}

void BoundingBox::setZListIndex(int z, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new SetBoundingBoxZListIndexUnoRedo(mZListIndex, z, this));
    }
    mZListIndex = z;
    scheduleRepaint();
}

int BoundingBox::getZIndex() {
    return mZListIndex;
}

void BoundingBox::setTransformation(QMatrix transMatrix)
{
    mTransformMatrix = transMatrix;
    updateCombinedTransform();
    schedulePivotUpdate();
}

QPointF BoundingBox::getAbsolutePos()
{
    return QPointF(mCombinedTransformMatrix.dx(), mCombinedTransformMatrix.dy());
}

void BoundingBox::updateAfterCombinedTransformationChanged()
{
    foreach(BoundingBox *child, mChildren) {
        child->updateCombinedTransform();
    }
}

void BoundingBox::updateCombinedTransform()
{
    if(mParent == NULL) {
        mCombinedTransformMatrix = mTransformMatrix;
    } else {
        mCombinedTransformMatrix = mTransformMatrix*
                mParent->getCombinedTransform();
    }
    updateAfterCombinedTransformationChanged();
}
