#ifndef SCULPTNODEBASE_H
#define SCULPTNODEBASE_H
#include <QPointF>

class SculptBrush;

class SculptNodeBase {
public:
    SculptNodeBase() {}
    SculptNodeBase(const qreal t, const QPointF& pos);

    void setPos(const QPointF& newPos) { mPos = newPos; }

    bool replacePosition(const SculptBrush& brush);
    bool dragPosition(const SculptBrush& brush);

    inline qreal t() const { return mT; }
    inline const QPointF& pos() const { return mPos; }
private:
    qreal mT;
    QPointF mPos;
};

#endif // SCULPTNODEBASE_H
