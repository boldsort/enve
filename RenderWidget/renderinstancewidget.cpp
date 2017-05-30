#include "renderinstancewidget.h"
#include "global.h"

RenderInstanceWidget::RenderInstanceWidget(QWidget *parent) :
    QWidget(parent) {
    setStyleSheet("QWidget { background: rgb(45, 45, 45); }");
    mMainLayout = new QVBoxLayout(this);
    mMainLayout->addSpacing(MIN_WIDGET_HEIGHT/2);
    mMainLayout->setMargin(0);
    setLayout(mMainLayout);

    mTitleLayout = new QHBoxLayout();
    mTitleLayout->setMargin(0);

    mNameLabel = new QLabel(this);
    mNameLabel->setFixedHeight(MIN_WIDGET_HEIGHT);
    mContentArrow = new QPushButton(QIcon(":/icons/list_show_children.png"),
                                    "",
                                    this);
    mContentArrow->setCheckable(true);
    mContentArrow->setStyleSheet("background: transparent;"
                                 "border: 0;");
    mContentArrow->setIconSize(QSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT));
    mContentArrow->setFixedSize(MIN_WIDGET_HEIGHT, MIN_WIDGET_HEIGHT);
    connect(mContentArrow, SIGNAL(toggled(bool)),
            this, SLOT(setContentVisible(bool)));

    mTitleLayout->addWidget(mContentArrow);
    mTitleLayout->addWidget(mNameLabel);

    mMainLayout->addLayout(mTitleLayout);

    mContentWidget = new QWidget(this);
    mContentWidget->setStyleSheet("QWidget { background: rgb(45, 45, 45); }");
    mMainLayout->addWidget(mContentWidget);
}

RenderInstanceWidget::RenderInstanceWidget(RenderInstanceSettings *settings,
                                           QWidget *parent) :
    RenderInstanceWidget(parent) {
    mSettings = settings;
    updateFromSettings();
}

void RenderInstanceWidget::updateFromSettings() {
    mNameLabel->setText(mSettings->getName());
}

void RenderInstanceWidget::setContentVisible(const bool &bT) {
    if(bT) {
        mContentArrow->setIcon(QIcon(":/icons/list_hide_children.png"));
    } else {
        mContentArrow->setIcon(QIcon(":/icons/list_show_children.png"));
    }
    mContentWidget->setVisible(bT);
}