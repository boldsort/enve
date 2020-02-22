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

#include "boxwithpatheffects.h"
#include "../PathEffects/patheffectcollection.h"
#include "../PathEffects/patheffectstask.h"

BoxWithPathEffects::BoxWithPathEffects(
        const QString &name, const eBoxType type) :
    BoundingBox(name, type) {
    mPathEffectsAnimators = enve::make_shared<PathEffectCollection>();
    mPathEffectsAnimators->prp_setName("path base effects");

    mFillPathEffectsAnimators = enve::make_shared<PathEffectCollection>();
    mFillPathEffectsAnimators->prp_setName("fill effects");

    mOutlineBasePathEffectsAnimators = enve::make_shared<PathEffectCollection>();
    mOutlineBasePathEffectsAnimators->prp_setName("outline base effects");

    mOutlinePathEffectsAnimators = enve::make_shared<PathEffectCollection>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");

    ca_addChild(mPathEffectsAnimators);
    ca_addChild(mFillPathEffectsAnimators);
    ca_addChild(mOutlineBasePathEffectsAnimators);
    ca_addChild(mOutlinePathEffectsAnimators);
}

#include "patheffectsmenu.h"
void BoxWithPathEffects::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<BoxWithPathEffects>()) return;
    menu->addedActionsForType<BoxWithPathEffects>();

    BoundingBox::setupCanvasMenu(menu);
    PathEffectsMenu::addPathEffectsToBoxActionMenu(menu);
}

void BoxWithPathEffects::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> pathOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setPathEffectsEnabled(checked);
    };
    menu->addCheckableAction("Path Effects",
                             getPathEffectsVisible(), pathOp)->
            setDisabled(mPathEffectsAnimators->ca_hasChildren());

    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> fillOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setFillEffectsEnabled(checked);
    };
    menu->addCheckableAction("Fill Effects",
                             getFillEffectsVisible(), fillOp)->
            setDisabled(mFillPathEffectsAnimators->ca_hasChildren());

    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> outBaseOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setOutlineBaseEffectsEnabled(checked);
    };
    menu->addCheckableAction("Outline Base Effects",
                             getOutlineBaseEffectsVisible(), outBaseOp)->
            setDisabled(mOutlineBasePathEffectsAnimators->ca_hasChildren());

    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> outOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setOutlineEffectsEnabled(checked);
    };
    menu->addCheckableAction("Outline Effects",
                             getOutlineEffectsVisible(), outOp)->
            setDisabled(mOutlinePathEffectsAnimators->ca_hasChildren());

    menu->addSeparator();

    BoundingBox::prp_setupTreeViewMenu(menu);
}

void BoxWithPathEffects::addPathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::addFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::addOutlineBasePathEffect(const qsptr<PathEffect>& effect) {
    mOutlineBasePathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::setPathEffectsEnabled(const bool enable) {
    mPathEffectsAnimators->SWT_setVisible(
                mPathEffectsAnimators->ca_hasChildren() || enable);
}

bool BoxWithPathEffects::getPathEffectsVisible() const {
    return mPathEffectsAnimators->SWT_isVisible();
}

void BoxWithPathEffects::setFillEffectsEnabled(const bool enable) {
    mFillPathEffectsAnimators->SWT_setVisible(
                mFillPathEffectsAnimators->ca_hasChildren() || enable);
}

bool BoxWithPathEffects::getFillEffectsVisible() const {
    return mFillPathEffectsAnimators->SWT_isVisible();
}

void BoxWithPathEffects::setOutlineBaseEffectsEnabled(const bool enable) {
    mOutlineBasePathEffectsAnimators->SWT_setVisible(
                mOutlineBasePathEffectsAnimators->ca_hasChildren() || enable);
}

bool BoxWithPathEffects::getOutlineBaseEffectsVisible() const {
    return mOutlineBasePathEffectsAnimators->SWT_isVisible();
}

void BoxWithPathEffects::setOutlineEffectsEnabled(const bool enable) {
    mOutlinePathEffectsAnimators->SWT_setVisible(
                mOutlinePathEffectsAnimators->ca_hasChildren() || enable);
}

bool BoxWithPathEffects::getOutlineEffectsVisible() const {
    return mOutlinePathEffectsAnimators->SWT_isVisible();
}

PathEffectCollection *BoxWithPathEffects::getPathEffectsAnimators() {
    return mPathEffectsAnimators.data();
}

PathEffectCollection *BoxWithPathEffects::getFillPathEffectsAnimators() {
    return mFillPathEffectsAnimators.data();
}

PathEffectCollection *BoxWithPathEffects::getOutlineBasrPathEffectsAnimators() {
    return mOutlineBasePathEffectsAnimators.data();
}

PathEffectCollection *BoxWithPathEffects::getOutlinePathEffectsAnimators() {
    return mOutlinePathEffectsAnimators.data();
}

bool BoxWithPathEffects::differenceInPathBetweenFrames(const int frame1, const int frame2) const {
    if(mPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    const auto parent = getParentGroup();
    if(!parent) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = parent->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = parent->prp_absFrameToRelFrame(absFrame2);
    return parent->differenceInPathBetweenFrames(pFrame1, pFrame2);
}

bool BoxWithPathEffects::differenceInOutlinePathBetweenFrames(const int frame1, const int frame2) const {
    if(mOutlineBasePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    const auto parent = getParentGroup();
    if(!parent) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = parent->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = parent->prp_absFrameToRelFrame(absFrame2);
    return parent->differenceInOutlinePathBetweenFrames(pFrame1, pFrame2);
}

bool BoxWithPathEffects::differenceInFillPathBetweenFrames(const int frame1, const int frame2) const {
    if(mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    const auto parent = getParentGroup();
    if(!parent) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = parent->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = parent->prp_absFrameToRelFrame(absFrame2);
    return parent->differenceInFillPathBetweenFrames(pFrame1, pFrame2);
}

void BoxWithPathEffects::addBasePathEffects(const qreal relFrame,
                                        QList<stdsptr<PathEffectCaller>>& list) {
    mPathEffectsAnimators->addEffects(relFrame, list);
    const auto parent = getParentGroup();
    if(!parent) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame = parent->prp_absFrameToRelFrameF(absFrame);
    parent->addBasePathEffects(parentRelFrame, list);
}

void BoxWithPathEffects::addFillEffects(const qreal relFrame,
                                        QList<stdsptr<PathEffectCaller>>& list) {
    mFillPathEffectsAnimators->addEffects(relFrame, list);
    const auto parent = getParentGroup();
    if(!parent) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame = parent->prp_absFrameToRelFrameF(absFrame);
    parent->addFillEffects(parentRelFrame, list);
}

void BoxWithPathEffects::addOutlineBaseEffects(const qreal relFrame,
                                               QList<stdsptr<PathEffectCaller>>& list) {
    mOutlineBasePathEffectsAnimators->addEffects(relFrame, list);
    const auto parent = getParentGroup();
    if(!parent) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame = parent->prp_absFrameToRelFrameF(absFrame);
    parent->addOutlineBaseEffects(parentRelFrame, list);
}

void BoxWithPathEffects::addOutlineEffects(const qreal relFrame,
                                           QList<stdsptr<PathEffectCaller>>& list) {
    mOutlinePathEffectsAnimators->addEffects(relFrame, list);
    const auto parent = getParentGroup();
    if(!parent) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame = parent->prp_absFrameToRelFrameF(absFrame);
    parent->addOutlineEffects(parentRelFrame, list);
}

void BoxWithPathEffects::getMotionBlurProperties(QList<Property*> &list) const {
    BoundingBox::getMotionBlurProperties(list);
    list.append(mPathEffectsAnimators.get());
    list.append(mFillPathEffectsAnimators.get());
    list.append(mOutlinePathEffectsAnimators.get());
}
