#include <QtGui>
#include <QScopedPointer>
#include <QtCore/qstate.h>
#include <QPluginLoader>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QWidget>
#include "GrymQtAndroidViewProxy.h"

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


class View : public QGraphicsView
{
    Q_OBJECT
public:
	View(QGraphicsScene * scene, QWidget * parent)
		: QGraphicsView(scene, parent)
		, quitButton(this)
		//, aview(this)
    {
        quitButton.resize(100, 60);
        quitButton.move(15,15);
        quitButton.setText("Quit");
        connect(&quitButton, SIGNAL(clicked()), QCoreApplication::instance(), SLOT(quit()));

		// aview.resize(500, 500);
		// aview.move(15, 100);
    }
protected:
    QPushButton quitButton;
	// GrymQtAndroidViewProxy aview;

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

    qDebug()<<TAG<<"Construct QApplication..."<<argc<<"args";
    QVector<char *> qargs;
    for(int i = 0; i < argc; ++i)
    {
        qargs.push_back(argv[i]);
        qDebug()<<"...arg"<<i<<":"<<argv[i];
    }
    QApplication app(argc, qargs.data());

    qDebug()<<TAG<<"Load pixmaps...";
    QPixmap kineticPix(":/images/kinetic.png");
    QPixmap bgPix(":/images/Time-For-Lunch-2.png");

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
	QScopedPointer<QWidget> window(new QWidget());

	QScopedPointer<QGLWidget> gl_layer(new QGLWidget(window.data()));
	gl_layer->setAutoFillBackground(false);
	gl_layer->setAttribute(Qt::WA_OpaquePaintEvent);
	gl_layer->setAttribute(Qt::WA_NoSystemBackground);
	gl_layer->setAttribute(Qt::WA_NoBackground);
	gl_layer->setAttribute(Qt::WA_StyledBackground, false);
	gl_layer->resize(100, 100);

	View * view = new View(&scene, gl_layer.data());

	QScopedPointer<GrymQtAndroidViewProxy> aview(new GrymQtAndroidViewProxy(window.data(), gl_layer.data()));
	aview->resize(500, 500);
	aview->move(15, 100);

	window->setObjectName("MainWindow"); // Used by TLW filter in mw_grym plugins (a name containing word "Window")

	window->setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Animated Tiles"));

	view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->setBackgroundBrush(bgPix);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    qDebug()<<TAG<<"Display window...";
#ifdef Q_OS_ANDROID
    QString pn = qt_android_get_current_plugin();
    qDebug()<<"Current Android plugin:"<<pn;
    if( pn.contains("Lite") ) // This is a plugin known to need and support fullscreen
		window->showFullScreen();
    else
		window->show(); // Other plugins go blankscreen on showFullScreen()
#else
	window->show();
#endif

	qDebug()<<TAG<<"Fixing sizes (FIXME)";
	gl_layer->resize(window->size());
	view->resize(window->size());



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

#if defined(Q_OS_ANDROID)
	gl_layer->__qpaDetachContext();
	aview->__qpaDetachContext();
#endif

    return ret;
}

#include "main.moc"
