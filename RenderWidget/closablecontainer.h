#ifndef CLOSABLECONTAINER_H
#define CLOSABLECONTAINER_H
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>

class ClosableContainer : public QWidget {
    Q_OBJECT
public:
    explicit ClosableContainer(QWidget *parent = nullptr);

    void setLabelWidget(QWidget *widget);
    void setContentWidget(QWidget *widget);
protected:
    QHBoxLayout *mMainLayout = new QHBoxLayout();
    QVBoxLayout *mContLayout = new QVBoxLayout();
    QWidget *mLabelWidget = NULL;
    QWidget *mContWidget = NULL;
    QPushButton *mContentArrow = new QPushButton();
signals:

public slots:
    void setContentVisible(const bool &bT);
};

#endif // CLOSABLECONTAINER_H