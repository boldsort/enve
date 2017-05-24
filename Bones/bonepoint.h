#ifndef BONEPOINT_H
#define BONEPOINT_H
#include "movablepoint.h"

class WeightMap {
public:
    enum PaintMode : short {
        PAINT_ADD,
        PAINT_SUBSTRACT,
        PAINT_REPLACE
    };

    void paintAt(const qreal &x,
                 const qreal &y,
                 const qreal &radius,
                 const qreal &weight,
                 const qreal &hardness,
                 const qreal &opacity,
                 const PaintMode &paintMode);

    void setSize(const int &width, const int &height);
    static void setWeightImageColorTable(QImage *img);
private:
    QImage mWeightMapImg;
};

class PathWeightMap {
public:

    void setNumberOfControlPoints(const int &nPoints);
    void setPointWeight(const int &pointId, const qreal &weight);
    void applyToPath(QPainterPath *path);
    void startPath(const QPointF &p);
    void addPoint(const QPointF &c1,
                  const QPointF &c2,
                  const QPointF &p);
    QPainterPath finishPath();
private:
    QPointF mLastPoint;
    QPainterPath mPath;
    qreal *mControlPointsWeight = NULL;
    int mNumberOfControlPoints;
};

class BonePoint : public MovablePoint {
public:
    BonePoint(BoundingBox *bonesBox);
    BonePoint(BoundingBox *bonesBox, BonePoint *parentBonePt);

    void addChildBonePoint(BonePoint *childBonePt);
    void setParentBonePointVar(BonePoint *parentBonePt);
    void setParentBonePoint(BonePoint *parentBonePt);
protected:
    QSharedPointer<QrealAnimator> mRotAnimator;
    QSharedPointer<QPointFAnimator> mScaleAnimator;
    bool mBoneBeetweenThisAndPrev = false;
    QList<QSharedPointer<BonePoint> > mChildBonePoints;
    QSharedPointer<BonePoint> mParentBonePoint;
};


#include "Properties/boolproperty.h"
#include "Animators/transformanimator.h"
class Bone : public ComplexAnimator {
public:
    Bone(BonePoint *rootPoint,
         BonePoint *tipPoint);

    QPointF getCurrentRootAbsPos();

    QPointF getCurrentTipAbsPos();

    qreal getCurrentRotation();

    void moveTipPointToRelPos(const QPointF &relPos);

    BasicTransformAnimator *getTransformAnimator();

    void setParentBone(Bone *parentBone);

private:
    QSharedPointer<Bone> mParentBone;
    QMatrix mRelTransform;
    QMatrix mCombinedTransform;

    QSharedPointer<BoolProperty> mFixedLength =
            (new BoolProperty())->ref<BoolProperty>();
    QSharedPointer<BasicTransformAnimator> mTransformAnimator =
            (new BasicTransformAnimator())->ref<BasicTransformAnimator>();
    QSharedPointer<BonePoint> mRootPoint;
    QSharedPointer<BonePoint> mTipPoint;
};
#endif // BONEPOINT_H
