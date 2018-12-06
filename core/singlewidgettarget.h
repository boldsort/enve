#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
#include "singlewidgetabstraction.h"

class QMenu;
class QAction;
class QMimeData;

enum SWT_Rule : short {
    SWT_NoRule,
    SWT_Selected,
    SWT_Visible,
    SWT_Invisible,
    SWT_Locked,
    SWT_Unlocked,
    SWT_Animated,
    SWT_NotAnimated
};

enum SWT_Target : short {
    SWT_CurrentCanvas,
    SWT_CurrentGroup,
    SWT_All
};


class SingleWidgetTarget : public SelfRef {
public:
    SingleWidgetTarget();
    virtual ~SingleWidgetTarget();

    void SWT_addChildAbstractionForTargetToAll(SingleWidgetTarget *target);
    void SWT_addChildAbstractionForTargetToAllAt(
            SingleWidgetTarget *target, const int &id);
    void SWT_removeChildAbstractionForTargetFromAll(
            SingleWidgetTarget *target);

    SingleWidgetAbstraction *SWT_createAbstraction(
            const UpdateFuncs &updateFuncs,
            const int &visiblePartWidgetId);
    void SWT_removeAbstractionFromList(
            const stdsptr<SingleWidgetAbstraction> &abs);

    virtual void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction*,
            const UpdateFuncs &,
            const int&) {}

    virtual SingleWidgetAbstraction* SWT_getAbstractionForWidget(
            const UpdateFuncs &updateFuncs,
            const int& visiblePartWidgetId) {
        return SWT_createAbstraction(updateFuncs, visiblePartWidgetId);
    }

    virtual bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool &parentSatisfies,
                                     const bool &parentMainTarget) {
        Q_UNUSED(rules);
        return parentSatisfies && !parentMainTarget;
    }

    void SWT_scheduleWidgetsContentUpdateWithRule(
            const SWT_Rule &rule);

    virtual bool SWT_visibleOnlyIfParentDescendant() {
        return true;
    }

    void SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty();
    void SWT_scheduleWidgetsContentUpdateWithTarget(
            SingleWidgetTarget *targetP, const SWT_Target &target);

    void SWT_moveChildAbstractionForTargetToInAll(
            SingleWidgetTarget *target, const int &id);

    virtual void SWT_addToContextMenu(QMenu *menu) {
        Q_UNUSED(menu);
    }
    virtual bool SWT_handleContextMenuActionSelected(
            QAction *selectedAction) {
        Q_UNUSED(selectedAction);
        return false;
    }

    virtual QMimeData *SWT_createMimeData() {
        return nullptr;
    }

    bool SWT_isVisible() {
        return SWT_mVisible;
    }

    void SWT_hide() {
        SWT_setVisible(false);
    }

    void SWT_show() {
        SWT_setVisible(true);
    }

    void SWT_setVisible(const bool &bT) {
        SWT_mVisible = bT;
        SWT_afterContentVisibilityChanged();
    }

    virtual void SWT_setChildrenAncestorDisabled(const bool &bT) {
        Q_UNUSED(bT);
    }

    void SWT_setDisabled(const bool &disable) {
        SWT_mDisabled = disable;
        SWT_setChildrenAncestorDisabled(SWT_isDisabled());
    }

    void SWT_disable() {
        SWT_setDisabled(true);
    }

    void SWT_enable() {
        SWT_setDisabled(false);
    }

    bool SWT_isDisabled() {
        return SWT_mDisabled || SWT_mAncestorDisabled;
    }

    void SWT_setAncestorDisabled(const bool &bT) {
        SWT_mAncestorDisabled = bT;
        SWT_setChildrenAncestorDisabled(SWT_isDisabled());
    }

    void SWT_afterContentVisibilityChanged();


    // Animators
    virtual bool SWT_isAnimator() { return false; }
    virtual bool SWT_isBoolAnimator() { return false; }
    virtual bool SWT_isColorAnimator() { return false; }
    virtual bool SWT_isComplexAnimator() { return false; }
    virtual bool SWT_isFakeComplexAnimator() { return false; }
    virtual bool SWT_isPixmapEffectAnimators() { return false; }
    virtual bool SWT_isPathEffectAnimators() { return false; }
    virtual bool SWT_isPixmapEffect() { return false; }
    virtual bool SWT_isPathEffect() { return false; }
    virtual bool SWT_isIntAnimator() { return false; }
    virtual bool SWT_isGradient() { return false; }
    virtual bool SWT_isPaintSettings() { return false; }
    virtual bool SWT_isStrokeSettings() { return false; }
    virtual bool SWT_isPathAnimator() { return false; }
    virtual bool SWT_isQPointFAnimator() { return false; }
    virtual bool SWT_isQrealAnimator() { return false; }
    virtual bool SWT_isQStringAnimator() { return false; }
    virtual bool SWT_isBasicTransformAnimator() { return false; }
    virtual bool SWT_isBoxTransformAnimator() { return false; }
    virtual bool SWT_isVectorPathAnimator() { return false; }
    virtual bool SWT_isAnimatedSurface() { return false; }
    // Boxes
    virtual bool SWT_isCanvas() { return false; }
    virtual bool SWT_isAnimationBox() { return false; }
    virtual bool SWT_isBoundingBox() { return false; }
    virtual bool SWT_isBonesBox() { return false; }
    virtual bool SWT_isBone() { return false; }
    virtual bool SWT_isBoxesGroup() { return false; }
    virtual bool SWT_isCircle() { return false; }
    virtual bool SWT_isImageBox() { return false; }
    virtual bool SWT_isImageSequenceBox() { return false; }
    virtual bool SWT_isLinkBox() { return false; }
    virtual bool SWT_isParticleBox() { return false; }
    virtual bool SWT_isPathBox() { return false; }
    virtual bool SWT_isRectangle() { return false; }
    virtual bool SWT_isTextBox() { return false; }
    virtual bool SWT_isVectorPath() { return false; }
    virtual bool SWT_isVideoBox() { return false; }
    virtual bool SWT_isPaintBox() { return false; }
    // Properties
    virtual bool SWT_isBoolProperty() { return false; }
    virtual bool SWT_isBoolPropertyContainer() { return false; }
    virtual bool SWT_isComboBoxProperty() { return false; }
    virtual bool SWT_isBoxTargetProperty() { return false; }
    virtual bool SWT_isProperty() { return false; }
    virtual bool SWT_isIntProperty() { return false; }
    // Sound
    virtual bool SWT_isSingleSound() { return false; }
protected:
    bool SWT_mAncestorDisabled = false;
    bool SWT_mVisible = true;
    bool SWT_mDisabled = false;
    QList<stdsptr<SingleWidgetAbstraction>> mSWT_allAbstractions;
};

typedef bool (SingleWidgetTarget::*SWT_Checker)();

struct SWT_TargetTypes {
    bool isTargeted(SingleWidgetTarget *target) const {
        Q_FOREACH(SWT_Checker func, targetsFunctionList) {
            if((target->*func)()) {
                return true;
            }
        }

        return false;
    }

    QList<SWT_Checker> targetsFunctionList;
};

#endif // SINGLEWIDGETTARGET_H