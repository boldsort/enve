#include "shadoweffect.h"
#include "rastereffects.h"
#include "Animators/coloranimator.h"
#include "Properties/boolproperty.h"
#include "Animators/qpointfanimator.h"
#include "pointhelpers.h"

ShadowEffect::ShadowEffect() :
    PixmapEffect("shadow", EFFECT_SHADOW) {
    mHighQuality = SPtrCreate(BoolProperty)("high quality");
    mBlurRadius = SPtrCreate(QrealAnimator)("blur radius");
    mOpacity = SPtrCreate(QrealAnimator)("opacity");
    mColor = SPtrCreate(ColorAnimator)();
    mTranslation = SPtrCreate(QPointFAnimator)("translation");

    mBlurRadius->setCurrentBaseValue(10.);

    mHighQuality->setValue(false);
    ca_addChildAnimator(mHighQuality);

    mBlurRadius->setValueRange(0., 1000.);
    ca_addChildAnimator(mBlurRadius);

    mTranslation->setBaseValue(QPointF(0., 0.));
    mTranslation->setValuesRange(-1000., 1000.);
    ca_addChildAnimator(mTranslation);

    mColor->qra_setCurrentValue(Qt::black);
    ca_addChildAnimator(mColor);

    mOpacity->setValueRange(0., 1000.);
    mOpacity->setCurrentBaseValue(100.);
    ca_addChildAnimator(mOpacity);
//    mScale.setCurrentValue(1.);
//    mScale.setName("scale");
//    mScale.blockPointer();
//    mScale.setValueRange(0., 10.);
//    addChildAnimator(&mScale);
}

stdsptr<PixmapEffectRenderData> ShadowEffect::getPixmapEffectRenderDataForRelFrameF(
        const qreal &relFrame, BoundingBoxRenderData*) {
    auto renderData = SPtrCreate(ShadowEffectRenderData)();
    renderData->blurRadius = mBlurRadius->getEffectiveValue(relFrame);
    renderData->hasKeys = mBlurRadius->anim_hasKeys();
    renderData->highQuality = mHighQuality->getValue();
    renderData->color = mColor->getColorAtRelFrame(relFrame);
    renderData->translation = mTranslation->
            getEffectiveValueAtRelFrame(relFrame);
    renderData->opacity = mOpacity->getEffectiveValue(relFrame)/100.;
    return GetAsSPtr(renderData, PixmapEffectRenderData);
}

void applyShadow(const SkBitmap &bitmap,
                 const qreal &scale,
                 const qreal &blurRadius,
                 const QColor &currentColor,
                 const QPointF &trans,
                 const bool &hasKeys,
                 const bool &highQuality,
                 const qreal &opacity = 1.) {
    SkBitmap shadowBitmap;
    shadowBitmap.allocPixels(bitmap.info());

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDstIn);
    SkCanvas shadowCanvas(shadowBitmap);
    shadowCanvas.clear(toSkColor(currentColor));
    shadowCanvas.drawBitmap(bitmap, 0, 0, &paint);
    shadowCanvas.flush();

    if(opacity > 1.) {
        RasterEffects::applyBlur(shadowBitmap, scale,
                  blurRadius, highQuality,
                  hasKeys, opacity);
    } else {
        RasterEffects::applyBlur(shadowBitmap, scale,
                  blurRadius, highQuality,
                  hasKeys);
        int alphaT = qMin(255, qMax(0, qRound(opacity*255) ));
        paint.setAlpha(static_cast<U8CPU>(alphaT));
    }

    SkCanvas dstCanvas(bitmap);
    paint.setBlendMode(SkBlendMode::kDstOver);
    dstCanvas.drawBitmap(shadowBitmap,
                         toSkScalar(trans.x()*scale),
                         toSkScalar(trans.y()*scale),
                         &paint);
    dstCanvas.flush();
}

void ShadowEffectRenderData::applyEffectsSk(const SkBitmap &bitmap,
                                            const qreal &scale) {
    applyShadow(bitmap, scale,
                blurRadius,
                color,
                translation,
                hasKeys,
                highQuality,
                opacity);
}

qreal ShadowEffect::getMargin() {
    return mBlurRadius->getCurrentBaseValue() +
            pointToLen(mTranslation->getEffectiveValue());
}

qreal ShadowEffect::getMarginAtRelFrame(const int &relFrame) {
    return mBlurRadius->getEffectiveValue(relFrame) +
            pointToLen(mTranslation->getEffectiveValueAtRelFrame(relFrame));
}
