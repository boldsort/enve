#include "surface.h"
#include <math.h>
#include <algorithm>
#include "GL/gl.h"
#include "GUI/ColorWidgets/helpers.h"

Surface::Surface(const ushort &widthT,
                 const ushort &heightT,
                 const qreal &scale,
                 const bool &paintInOtherThread) {
    mIsTemporary = !paintInOtherThread;
    mScale = scale;
    mCurrentTiles = SPtrCreate(TilesData)(widthT, heightT,
                                          paintInOtherThread);
    mCurrentTiles->setCurrentlyUsed(true);
    setSize(widthT, heightT);
}

Surface::~Surface() {}

void Surface::duplicateTilesContentFrom(TilesData *tilesSrc) {
    mCurrentTiles->duplicateTilesContentFrom(tilesSrc);
}

qreal Surface::countDabsTo(const qreal &dist_between_dabs,
                           const qreal &x, const qreal &y) {
    qreal dx = x - mLastPaintedStrokeX;
    qreal dy = y - mLastPaintedStrokeY;

    return sqrt(dx*dx + dy*dy)/dist_between_dabs;
}

void replaceIfSmaller(const int &potentially_smaller,
                      int *to_be_replaced) {
    if(potentially_smaller < *to_be_replaced) {
        *to_be_replaced = potentially_smaller;
    }
}

void replaceIfBigger(const int &potentially_bigger,
                     int *to_be_replaced) {
    if(potentially_bigger > *to_be_replaced) {
        *to_be_replaced = potentially_bigger;
    }
}

void Surface::getTileIdsOnRect(const qreal &x_min,
                               const qreal &x_max,
                               const qreal &y_min,
                               const qreal &y_max,
                               int *tile_x_min,
                               int *tile_x_max,
                               int *tile_y_min,
                               int *tile_y_max) {
    *tile_x_min = qFloor(x_min/TILE_DIM);
    if(*tile_x_min < 0) {
        *tile_x_min = 0;
    }
    *tile_x_max = qCeil(x_max/TILE_DIM);
    if(*tile_x_max >= mNTileCols) {
        *tile_x_max = mNTileCols - 1;
    }
    *tile_y_min = qFloor(y_min/TILE_DIM);
    if(*tile_y_min < 0) {
        *tile_y_min = 0;
    }
    *tile_y_max = qCeil(y_max/TILE_DIM);
    if(*tile_y_max >= mNTileRows) {
        *tile_y_max = mNTileRows - 1;
    }
}

void Surface::getColor(const qreal &cx,
                       const qreal &cy,
                       const qreal &hardness,
                       const qreal &opa,
                       const qreal &aspect_ratio,
                       const qreal &r,
                       const qreal &beta_deg,
                       qreal *red, qreal *green,
                       qreal *blue, qreal *alpha) {
    qreal cs = cos(beta_deg*2*PI/360);
    qreal sn = sin(beta_deg*2*PI/360);

    qreal red_sum = 0.;
    qreal green_sum = 0.;
    qreal blue_sum = 0.;
    qreal alpha_sum = 0.;
    qreal weight_sum = 0.;
    int min_tile_x;
    int max_tile_x;
    int min_tile_y;
    int max_tile_y;
    getTileIdsOnRect(cx - r, cx + r, cy - r, cy + r,
                     &min_tile_x, &max_tile_x, &min_tile_y, &max_tile_y);
    Tile ***tilesT = mCurrentTiles->getTiles();
    for(int i = min_tile_x; i <= max_tile_x; i++) {
        for(int j = min_tile_y; j <= max_tile_y; j++) {
            Tile *tile_t = tilesT[j][i];
            tile_t->getColor(cx, cy,
                             r, aspect_ratio, cs, sn,
                             hardness, opa,
                             &red_sum, &green_sum,
                             &blue_sum, &alpha_sum,
                             &weight_sum);
        }
    }
    if(alpha_sum < 0.01) {
        *red = 0.;
        *green = 0.;
        *blue = 0.;
        *alpha = 0.;
    } else {
        *red = red_sum/weight_sum;
        *green = green_sum/weight_sum;
        *blue = blue_sum/weight_sum;
        *alpha = alpha_sum/weight_sum;
    }
}

void Surface::clear() {
    mCurrentTiles->clearTiles();
}

void Surface::getTileDrawers(QList<TileSkDrawerCollection> *tileDrawers) {
    TileSkDrawerCollection coll;
    mCurrentTiles->getTileDrawers(&coll.drawers);
    tileDrawers->append(coll);
}

void Surface::clearTmp() {
    mCurrentTiles->clearTmp();
}

void Surface::saveToTmp() {
    mCurrentTiles->saveToTmp();
}

void Surface::getPickedUpRGBA(qreal *red_t,
                            qreal *green_t,
                            qreal *blue_t,
                            qreal *alpha_t) const {
    *red_t = picked_up_red;
    *green_t = picked_up_green;
    *blue_t = picked_up_blue;
    *alpha_t = picked_up_alpha;
}

void Surface::setPickedUpRGBA(qreal red_t,
                              qreal green_t,
                              qreal blue_t,
                              qreal alpha_t) {
    picked_up_red = red_t;
    picked_up_green = green_t;
    picked_up_blue = blue_t;
    picked_up_alpha = alpha_t;
}

void Surface::addPickedUpRGBAFromNewStroke(qreal red_t,
                                           qreal green_t,
                                           qreal blue_t,
                                           qreal alpha_t,
                                           const Brush *brush) {
    alpha_t *= brush->getColorPickUp();
    picked_up_alpha = picked_up_alpha*(1. - brush->getColorPickUpDecay());
    qreal alpha_sum = alpha_t + picked_up_alpha;
    if(alpha_sum < 0.0001) {
        picked_up_red = 0.;
        picked_up_green = 0.;
        picked_up_blue = 0.;
        picked_up_alpha = 0.;
    } else {
        picked_up_red = (red_t*alpha_t + picked_up_red*picked_up_alpha)/alpha_sum;
        picked_up_green = (green_t*alpha_t + picked_up_green*picked_up_alpha)/alpha_sum;
        picked_up_blue = (blue_t*alpha_t + picked_up_blue*picked_up_alpha)/alpha_sum;
        if(picked_up_alpha > 1.) {

        } else if(alpha_sum > 1.) {
            picked_up_alpha = 1.;
        } else {
            picked_up_alpha = alpha_sum;
        }
    }
}

void Surface::resetPickedUpRGBA(const Brush *brush) {
    picked_up_red = brush->getRed();
    picked_up_green = brush->getGreen();
    picked_up_blue = brush->getBlue();
    picked_up_alpha = brush->getInitialAlpha();
}

void Surface::strokeTo(const Brush *brush,
                       qreal x, qreal y,
                       const qreal &pressure,
                       const ushort &dt,
                       const bool &erase) {
    x *= mScale;
    y *= mScale;
    qreal dist_between_dabs =
            brush->getDistBetweenDabsPx()*mScale;
    qreal stroke_dx = x - mLastEventStrokeX;
    qreal stroke_dy = y - mLastEventStrokeY;
    mLastEventStrokeX = x;
    mLastEventStrokeY = y;
    qreal stroke_vx = stroke_dx*50/dt;
    qreal stroke_vy = stroke_dy*50/dt;
    qreal stroke_vel = mLastEventStrokeVel*0.8 +
            0.2*sqrt(stroke_vx*stroke_vx +
                     stroke_vy*stroke_vy);
    mLastEventStrokeVel = stroke_vel;

    applyXYNoise(brush->getStrokePositionNoisePx()*mScale,
                 mPreviousStrokeXNoise,
                 mNextStrokeXNoise,
                 mPreviousStrokeYNoise,
                 mNextStrokeYNoise,
                 brush->getStrokePositionNoiseFrequency(),
                 mStrokeNoiseCount, x, y);

    qreal dabs_to = countDabsTo(dist_between_dabs, x, y);

    qreal first_dab_dx = x - mLastPaintedStrokeX;
    qreal first_dab_dy = y - mLastPaintedStrokeY;;
    qreal dabs_dx = first_dab_dx;
    qreal dabs_dy = first_dab_dy;

    if(isZero4Dec(first_dab_dx) && isZero4Dec(first_dab_dy)) {
        first_dab_dx = 0.;
        first_dab_dy = 0.;
        dabs_dx = 0.;
        dabs_dy = 0.;
    } else {
        normalize(&first_dab_dx, &first_dab_dy,
                  dist_between_dabs);
        normalize(&dabs_dx, &dabs_dy,
                  dist_between_dabs);
    }
    qreal first_dab_x = mLastPaintedStrokeX + first_dab_dx;
    qreal first_dab_y = mLastPaintedStrokeY + first_dab_dy;

    int min_affected_tile_x = 10000;
    int max_affected_tile_x = 0;
    int min_affected_tile_y = 10000;
    int max_affected_tile_y = 0;

    int dabs_to_i = qFloor(dabs_to);
    if(dabs_to_i <= 0) return;
    qreal rotation_delay = brush->getRotationDelay();
    qreal angle_t =
            getAngleDeg(x - mLastPaintedStrokeX,
                        y - mLastPaintedStrokeY, 0., -1.);
    if(angle_t > 180.) {
        angle_t -= 180.;
    } else if(angle_t < 0.) {
        angle_t += 180.;
    }
    qreal angle_diff = fabs(angle_t - mLastStrokeBeta);
    qreal dest_angle;
    if(angle_diff > 120.) {
        dest_angle = angle_t;
    } else {
        dest_angle = angle_t*(1 - rotation_delay)*
                brush->getRotationInfluence() +
                mLastStrokeBeta*rotation_delay;
    }

    if(dest_angle > 180.) {
        dest_angle -= 180.;
    } else if(dest_angle < 0.) {
        dest_angle += 180.;
    }
    mLastStrokeBeta = dest_angle;
    qreal dab_r =
       qMin(brush->getRadius()*5,
            brush->getRadius() +
            brush->getPressureRadiusGainPx()*pressure +
            brush->getSpeedRadiusGain()*stroke_vel)*mScale;
    qreal dab_hardness = brush->getHardness() +
            brush->getPressureHardnessGain()*pressure +
            brush->getSpeedHardnessGain()*stroke_vel;
    qreal dab_opa = brush->getOpacity() +
            brush->getPressureOpacityGain()*pressure +
            brush->getSpeedOpacityGain()*stroke_vel;
    qreal dab_aspect_ratio = brush->getAspectRatio() +
            brush->getPressureAspectRatioGain()*pressure;

    if(!mIsTemporary) {
        qreal smudge_red;
        qreal smudge_green;
        qreal smudge_blue;
        qreal smudge_alpha;
        getColor(mLastPaintedStrokeX, mLastPaintedStrokeY,
                 dab_hardness, dab_opa,
                 dab_aspect_ratio, dab_r, dest_angle,
                 &smudge_red, &smudge_green,
                 &smudge_blue, &smudge_alpha);
        addPickedUpRGBAFromNewStroke(smudge_red,
                                     smudge_green,
                                     smudge_blue,
                                     smudge_alpha,
                                     brush);
    }

    qreal stroke_red;
    qreal stroke_green;
    qreal stroke_blue;
    qreal dab_alpha;
    bool fixed_color = isZero4Dec(brush->getHueNoise()) &&
            isZero4Dec(brush->getSaturationNoise()) &&
            isZero4Dec(brush->getValueNoise());

    Brush::getRGB(&stroke_red,
                  &stroke_green,
                  &stroke_blue);
    dab_alpha = brush->getContinuesAlpha();
    dab_alpha += brush->getPressureAlphaGain()*pressure;

    qreal alpha_sum_t = dab_alpha + picked_up_alpha;
    stroke_red = (stroke_red*dab_alpha +
                  picked_up_alpha*picked_up_red)/alpha_sum_t;
    stroke_green = (stroke_green*dab_alpha +
                    picked_up_alpha*picked_up_green)/alpha_sum_t;
    stroke_blue = (stroke_blue*dab_alpha +
                   picked_up_alpha*picked_up_blue)/alpha_sum_t;
    if(alpha_sum_t > 1.) {
        alpha_sum_t = 1.;
    }

    qreal dab_red = stroke_red;
    qreal dab_green = stroke_green;
    qreal dab_blue = stroke_blue;

    qreal stroke_h = 0.;
    qreal stroke_s = 0.;
    qreal stroke_v = 0.;
    if(!fixed_color) {
        stroke_h = stroke_red;
        stroke_s = stroke_green;
        stroke_v = stroke_blue;
        qrgb_to_hsv(stroke_h, stroke_s, stroke_v);
    }
    Tile ***tilesT = mCurrentTiles->getTiles();
    mCurrentTiles->dataChanged();
    for(int i = 0; i < dabs_to_i; i++) {
        if(!fixed_color) {
            dab_red = stroke_h;
            applyNoise(brush->getHueNoise(),
                       mPreviousHueNoise,
                       mNextHueNoise,
                       brush->getHueNoiseFrequency(),
                       mHueNoiseCount, dab_red);
            dab_green = stroke_s;
            applyNoise(brush->getSaturationNoise(),
                       mPreviousSaturationNoise,
                       mNextSaturationNoise,
                       brush->getSaturationNoiseFrequency(),
                       mSaturationNoiseCount, dab_green);
            dab_blue = stroke_v;
            applyNoise(brush->getValueNoise(),
                       mPreviousValueNoise,
                       mNextValueNoise,
                       brush->getValueNoiseFrequency(),
                       mValueNoiseCount, dab_blue);
            qhsv_to_rgb(dab_red, dab_green, dab_blue);
        }
        qreal dab_x = first_dab_x + i*dabs_dx +
                getNoise(brush->getDabPositionNoisePx()*mScale);
        qreal dab_y = first_dab_y + i*dabs_dy +
                getNoise(brush->getDabPositionNoisePx()*mScale);
        mLastPaintedStrokeX = dab_x;
        mLastPaintedStrokeY = dab_y;
        mLastDabRotationInc += brush->getRotationBetweenDabs();
        if(mLastDabRotationInc > 180.) {
            mLastDabRotationInc -= 180.;
        }

        qreal dab_x_min = dab_x - dab_r;
        qreal dab_x_max = dab_x + dab_r;
        qreal dab_y_min = dab_y - dab_r;
        qreal dab_y_max = dab_y + dab_r;

        int dab_min_tile_x;
        int dab_max_tile_x;
        int dab_min_tile_y;
        int dab_max_tile_y;
        getTileIdsOnRect(dab_x_min, dab_x_max,
                         dab_y_min, dab_y_max,
                         &dab_min_tile_x, &dab_max_tile_x,
                         &dab_min_tile_y, &dab_max_tile_y);

        replaceIfSmaller(dab_min_tile_x, &min_affected_tile_x);
        replaceIfSmaller(dab_min_tile_y, &min_affected_tile_y);
        replaceIfBigger(dab_max_tile_x, &max_affected_tile_x);
        replaceIfBigger(dab_max_tile_y, &max_affected_tile_y);
        qreal dab_rot = dest_angle + brush->getInitialRotation() +
                mLastDabRotationInc +
                getNoise(brush->getRotationNoise() );
        //#pragma omp parallel for
        for(int tx = dab_min_tile_x; tx <= dab_max_tile_x; tx++) {
            for(int ty = dab_min_tile_y; ty <= dab_max_tile_y; ty++) {
                tilesT[ty][tx]->addDabToDraw(dab_x, dab_y,
                                            dab_hardness,
                                            dab_opa*alpha_sum_t,
                                            dab_aspect_ratio,
                                            dab_r, dab_rot,
                                            dab_red, dab_green,
                                            dab_blue, erase);
            }
        }
    }

    for(int tx = min_affected_tile_x; tx <= max_affected_tile_x; tx++) {
        for(int ty = min_affected_tile_y; ty <= max_affected_tile_y; ty++) {
            tilesT[ty][tx]->addScheduler();
        }
    }

    mLastStrokePress = pressure;
    currentDataModified();
}

void Surface::startNewStroke(const Brush *brush,
                             qreal x, qreal y,
                             const qreal &pressure) {
    x *= mScale;
    y *= mScale;
    mStrokeNoiseCount = UCHAR_MAX;
    mPreviousStrokeXNoise = 0.;
    mPreviousStrokeYNoise = 0.;
    mNextStrokeXNoise = 0.;
    mNextStrokeYNoise = 0.;

    mRotationNoiseCount = UCHAR_MAX;
    mPreviousRotationNoise = 0.;
    mNextRotationNoise = 0.;

    mHueNoiseCount = UCHAR_MAX;
    mPreviousHueNoise = 0.;
    mNextHueNoise = 0.;

    mSaturationNoiseCount = UCHAR_MAX;
    mPreviousSaturationNoise = 0.;
    mNextSaturationNoise = 0.;

    mValueNoiseCount = UCHAR_MAX;
    mPreviousValueNoise = 0.;
    mNextValueNoise = 0.;

    mLastEventStrokeX = x;
    mLastEventStrokeY = y;
    mLastPaintedStrokeX = x;
    mLastPaintedStrokeY = y;
    mLastXSpeedOffset = 0.;
    mLastYSpeedOffset = 0.;
    mLastStrokePress = pressure;
    resetPickedUpRGBA(brush);

    mNextSecondColorAlpha = 0.;
    mPreviousSecondColorAlpha = 0.;
    mSecondColorAlphaCount = UCHAR_MAX;
}

void Surface::setSize(const ushort &width_t,
                      const ushort &height_t) {
    if(width_t == mWidth && height_t == mHeight) return;

    // initialize tiles
    int nTileCols = qCeil(width_t/static_cast<qreal>(TILE_DIM));
    int nTileRows = qCeil(height_t/static_cast<qreal>(TILE_DIM));

    mCurrentTiles->setSize(width_t, height_t);

    mWidth = width_t;
    mHeight = height_t;
    mNTileRows = nTileRows;
    mNTileCols = nTileCols;
}

void Surface::move(const int &xT, const int &yT) {
    mCurrentTiles->move(xT, yT);
}

#include <QImage>
void Surface::loadFromImage(const QImage &img) {
    mCurrentTiles->setImage(img);
}

void Surface::paintPress(const qreal &xT,
                         const qreal &yT,
                         const ulong &timestamp,
                         const qreal &pressure,
                         const Brush *brush) {
    mousePressEvent(xT, yT, timestamp, pressure, brush);
}

void Surface::tabletMoveEvent(const qreal &xT,
                              const qreal &yT,
                              const ulong &time_stamp,
                              const qreal &pressure,
                              const bool &erase,
                              const Brush *brush) {
    Q_UNUSED(time_stamp);
    strokeTo(brush,
             xT, yT,
             pressure, 100,
             erase);
}

void Surface::tabletReleaseEvent() {
    mCurrentTiles->finishTransform();
}

void Surface::tabletPressEvent(const qreal &xT,
                               const qreal &yT,
                               const ulong &time_stamp,
                               const qreal &pressure,
                               const bool &erase,
                               const Brush *brush) {
    startNewStroke(brush, xT, yT, pressure);
    tabletMoveEvent(xT, yT, time_stamp, pressure, erase, brush);
}

void Surface::mouseReleaseEvent() {
    tabletReleaseEvent();
}

void Surface::mousePressEvent(const qreal &xT,
                              const qreal &yT,
                              const ulong &timestamp,
                              const qreal &pressure,
                              const Brush *brush) {
    tabletPressEvent(xT, yT,
                     timestamp, pressure,
                     false, brush);
}

void Surface::mouseMoveEvent(const qreal &xT,
                             const qreal &yT,
                             const ulong &time_stamp,
                             const bool &erase,
                             const Brush *brush) {
    tabletMoveEvent(xT, yT,
                time_stamp, 1.0,
                erase, brush);
}
