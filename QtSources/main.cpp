/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtCore/qstate.h>
#include <QPluginLoader>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QMessageBox>

#ifdef Q_OS_ANDROID
QString qt_android_get_current_plugin();
#endif

class Pixmap : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    Pixmap(const QPixmap &pix)
        : QObject(), QGraphicsPixmapItem(pix)
    {
        setCacheMode(DeviceCoordinateCache);
    }
};

class Button : public QGraphicsWidget
{
    Q_OBJECT
public:
    Button(const QPixmap &pixmap, QGraphicsItem *parent = 0)
        : QGraphicsWidget(parent), _pix(pixmap)
    {
        setAcceptHoverEvents(true);
        setCacheMode(DeviceCoordinateCache);
    }

    QRectF boundingRect() const
    {
        return QRectF(-65, -65, 130, 130);
    }

    QPainterPath shape() const
    {
        QPainterPath path;
        path.addEllipse(boundingRect());
        return path;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
    {
        bool down = option->state & QStyle::State_Sunken;
        QRectF r = boundingRect();
        QLinearGradient grad(r.topLeft(), r.bottomRight());
        grad.setColorAt(down ? 1 : 0, option->state & QStyle::State_MouseOver ? Qt::white : Qt::lightGray);
        grad.setColorAt(down ? 0 : 1, Qt::darkGray);
        painter->setPen(Qt::darkGray);
        painter->setBrush(grad);
        painter->drawEllipse(r);
        QLinearGradient grad2(r.topLeft(), r.bottomRight());
        grad.setColorAt(down ? 1 : 0, Qt::darkGray);
        grad.setColorAt(down ? 0 : 1, Qt::lightGray);
        painter->setPen(Qt::NoPen);
        painter->setBrush(grad);
        if (down)
            painter->translate(2, 2);
        painter->drawEllipse(r.adjusted(5, 5, -5, -5));
        painter->drawPixmap(-_pix.width()/2, -_pix.height()/2, _pix);
    }

signals:
    void pressed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        emit pressed();
        update();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        update();
    }

private:
    QPixmap _pix;
};

/* Multiple windows are not supported on Android yet */
// #defined ABOUT_BUTTON

class View : public QGraphicsView
{
    Q_OBJECT
public:
    View(QGraphicsScene *scene) 
            : QGraphicsView(scene)
            , quitButton(this)
#ifdef ABOUT_BUTTON
            , aboutButton(this)
#endif
    {
        quitButton.resize(100, 60);
        quitButton.move(15,15);
        quitButton.setText("Quit");
        connect(&quitButton, SIGNAL(clicked()), QCoreApplication::instance(), SLOT(quit()));

#ifdef ABOUT_BUTTON
        aboutButton.resize(100, 60);
        aboutButton.move(120,15);
        aboutButton.setText("About");
        connect(&aboutButton, SIGNAL(clicked()), this, SLOT(about()));
#endif
    }

#ifdef ABOUT_BUTTON
public slots:
    void about()
    {
       QMessageBox::about(this, "About", "Qt Animated Tiles");
    }
#endif

protected:
    QPushButton quitButton;
#ifdef ABOUT_BUTTON
    QPushButton aboutButton;
#endif

    void resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
};

static const char* const TAG = "**** main() ****";

int main(int argc, char **argv)
{
    qDebug()<<"Static plugins "<<QPluginLoader::staticInstances().count();
    foreach(QObject * obj, QPluginLoader::staticInstances())
        qDebug()<<"Plugin: "<<obj;

    // qDebug()<<"main "<<argc<<","<<argv[0]<<argv[1];
    qDebug()<<TAG<<"Init resource...";
    Q_INIT_RESOURCE(animatedtiles);

    qDebug()<<TAG<<"Construct QApplication...";
    QApplication app(argc, argv);

    qDebug()<<TAG<<"Load pixmaps...";
    QPixmap kineticPix(":/images/kinetic.png");
    QPixmap bgPix(":/images/Time-For-Lunch-2.jpg");

    qDebug()<<TAG<<"Graphics scene...";
    QGraphicsScene scene(-350, -350, 700, 700);

    QList<Pixmap *> items;
    for (int i = 0; i < 64; ++i) {
        Pixmap *item = new Pixmap(kineticPix);
        item->setOffset(-kineticPix.width()/2, -kineticPix.height()/2);
        item->setZValue(i);
        items << item;
        scene.addItem(item);
    }

    // Buttons
    qDebug()<<TAG<<"Buttons...";
    QGraphicsItem *buttonParent = new QGraphicsRectItem;
    Button *ellipseButton = new Button(QPixmap(":/images/ellipse.png"), buttonParent);
    Button *figure8Button = new Button(QPixmap(":/images/figure8.png"), buttonParent);
    Button *randomButton = new Button(QPixmap(":/images/random.png"), buttonParent);
    Button *tiledButton = new Button(QPixmap(":/images/tile.png"), buttonParent);
    Button *centeredButton = new Button(QPixmap(":/images/centered.png"), buttonParent);

    ellipseButton->setPos(-100, -100);
    figure8Button->setPos(100, -100);
    randomButton->setPos(0, 0);
    tiledButton->setPos(-100, 100);
    centeredButton->setPos(100, 100);

    scene.addItem(buttonParent);
    // buttonParent->scale(0.75, 0.75);
    buttonParent->setPos(150, 150);
    buttonParent->setZValue(65);

    // States
    qDebug()<<TAG<<"States...";
    QState *rootState = new QState;
    QState *ellipseState = new QState(rootState);
    QState *figure8State = new QState(rootState);
    QState *randomState = new QState(rootState);
    QState *tiledState = new QState(rootState);
    QState *centeredState = new QState(rootState);

    // Values
    for (int i = 0; i < items.count(); ++i) {
        Pixmap *item = items.at(i);
        // Ellipse
        ellipseState->assignProperty(item, "pos",
                                         QPointF(cos((i / 63.0) * 6.28) * 250,
                                                 sin((i / 63.0) * 6.28) * 250));

        // Figure 8
        figure8State->assignProperty(item, "pos",
                                         QPointF(sin((i / 63.0) * 6.28) * 250,
                                                 sin(((i * 2)/63.0) * 6.28) * 250));

        // Random
        randomState->assignProperty(item, "pos",
                                        QPointF(-250 + qrand() % 500,
                                                -250 + qrand() % 500));

        // Tiled
        tiledState->assignProperty(item, "pos",
                                       QPointF(((i % 8) - 4) * kineticPix.width() + kineticPix.width() / 2,
                                               ((i / 8) - 4) * kineticPix.height() + kineticPix.height() / 2));

        // Centered
        centeredState->assignProperty(item, "pos", QPointF());
    }

    // Ui
    qDebug()<<TAG<<"Creating view...";
    View *view = new View(&scene);
    view->setObjectName("MainWindow"); // Used by TLW filter in mw_grym plugins (a name containing word "Window")

    // Window centering, incl. Android plugin debugging ;-)
    view->resize(220, 220);
    QDesktopWidget* desk = QApplication::desktop();
    QRect geom = desk->screenGeometry();
    int x = (geom.width()-view->width())/2, y = (geom.height()-view->height())/2;
    qDebug()<<"Desktop size is:"<<geom.width()<<"x"<<geom.height()<<"; desired window pos:"<<x<<","<<y;
    if( x>0 && y>0 )
        view->move(x, y);
    else
        qDebug()<<"(Won't move the window to negative position!)";

    view->setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Animated Tiles"));
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->setBackgroundBrush(bgPix);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    qDebug()<<TAG<<"Display window...";
#ifdef Q_OS_ANDROID
    QString pn = qt_android_get_current_plugin();
    qDebug()<<"Current Android plugin:"<<pn;
    if( pn.contains("Lite") ) // This is a plugin known to need and support fullscreen
        view->showFullScreen();
    else
        view->show(); // Other plugins go blankscreen on showFullScreen()
#else
    view->show();
#endif

    qDebug()<<TAG<<"State machine...";
    QStateMachine states;
    states.addState(rootState);
    states.setInitialState(rootState);
    rootState->setInitialState(centeredState);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    for (int i = 0; i < items.count(); ++i) {
        QPropertyAnimation *anim = new QPropertyAnimation(items[i], "pos");
        anim->setDuration(750 + i * 25);
        anim->setEasingCurve(QEasingCurve::InOutBack);
        group->addAnimation(anim);
    }
    QAbstractTransition *trans = rootState->addTransition(ellipseButton, SIGNAL(pressed()), ellipseState);
    trans->addAnimation(group);

    trans = rootState->addTransition(figure8Button, SIGNAL(pressed()), figure8State);
    trans->addAnimation(group);

    trans = rootState->addTransition(randomButton, SIGNAL(pressed()), randomState);
    trans->addAnimation(group);

    trans = rootState->addTransition(tiledButton, SIGNAL(pressed()), tiledState);
    trans->addAnimation(group);

    trans = rootState->addTransition(centeredButton, SIGNAL(pressed()), centeredState);
    trans->addAnimation(group);

    qDebug()<<TAG<<"Starting timer...";
    QTimer timer;
    timer.start(125);
    timer.setSingleShot(true);
    trans = rootState->addTransition(&timer, SIGNAL(timeout()), ellipseState);
    trans->addAnimation(group);

    states.start();

#ifdef QT_KEYPAD_NAVIGATION
    QApplication::setNavigationMode(Qt::NavigationModeCursorAuto);
#endif
    qDebug()<<"Executing app...";
    int ret = app.exec();
    qDebug()<<TAG<<"Done! C U";
    return ret;
}

#include "main.moc"
