#include <QtGui>
#include <QScopedPointer>
#include <QtCore/qstate.h>
#include <QPluginLoader>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QWidget>
#include <QMainWindow>
#include <QGraphicsView>
#include "GrymQtAndroidViewGraphicsProxy.h"

#ifdef Q_OS_ANDROID
QString qt_android_get_current_plugin();
#endif

class MyAnimatedPixmap : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
	MyAnimatedPixmap(const QPixmap &pix)
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
    {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setFrameShape(QFrame::NoFrame); // OMG!

		quitButton.resize(150, 100);
		quitButton.move(20, 20);
        quitButton.setText("Quit");
        connect(&quitButton, SIGNAL(clicked()), QCoreApplication::instance(), SLOT(quit()));
    }
protected:
    QPushButton quitButton;
    void resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
		fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
};

class MyGlWidget
	: public QGLWidget
{
	Q_OBJECT
public:
	MyGlWidget(QWidget * parent)
		: QGLWidget(parent)
	{
	}

	virtual void paintGL()
	{
		qDebug()<<"Grym"<<__PRETTY_FUNCTION__;
		// glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glEnable(GL_SCISSOR_TEST);
		//glScissor(10, 10, width()-20, height()-20);
		QGLWidget::paintGL();
	}
};

class MyWindow
	: public QWidget
{
	Q_OBJECT
public:
	MyWindow()
		: QWidget(0)
		, gl_layer_(0)
		, view_(0)
		, scene_(-350, -350, 700, 700)
		, bgPix(":/images/Time-For-Lunch-2.png")
	{
		move(0, 0);

		setFocusPolicy(Qt::NoFocus);
		setObjectName("MainWindow");
		setWindowTitle("Protoview");
#if 1
		setAutoFillBackground(false);
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_NoBackground);
		setAttribute(Qt::WA_StyledBackground, false);
#endif

		// QGLFormat format;
		gl_layer_ = new QGLWidget(this);
		gl_layer_->setFocusPolicy(Qt::NoFocus);
#if 1
		gl_layer_->setAutoFillBackground(false);
		gl_layer_->setAttribute(Qt::WA_OpaquePaintEvent);
		gl_layer_->setAttribute(Qt::WA_NoSystemBackground);
		gl_layer_->setAttribute(Qt::WA_NoBackground);
		gl_layer_->setAttribute(Qt::WA_StyledBackground, false);		
#endif

		view_ = new View(&scene_, gl_layer_);
		view_->setFocusPolicy(Qt::NoFocus);

#if 1
		view_->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
#else
		view_->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
#endif

#if 1
		view_->setBackgroundBrush(bgPix);
		view_->setCacheMode(QGraphicsView::CacheBackground);
		view_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
#endif

		aview.reset(new GrymQtAndroidViewGraphicsProxy());
		scene_.addItem(aview.data());

		resize(1000, 1000);
		doLayout(size());
	}

	virtual ~MyWindow()
	{
		#if defined(Q_OS_ANDROID)
			gl_layer_->__qpaDetachContext();
		#endif
	}

	QGLWidget * glLayer() { return gl_layer_; }
	View * view() { return view_; }
	QGraphicsScene * scene() { return &scene_; }

protected:
	virtual void keyReleaseEvent(QKeyEvent * e)
	{
		if (e->key() == Qt::Key_Escape)
		{
			QCoreApplication::exit();
		}
		QWidget::keyReleaseEvent(e);
	}

	virtual void resizeEvent(QResizeEvent * e)
	{
		QWidget::resizeEvent(e);
		doLayout(e->size());
	}

	virtual void doLayout(const QSize & newsize)
	{
		gl_layer_->move(0, 0);
		gl_layer_->resize(newsize);

		scene_.setSceneRect(-newsize.width()/2, -newsize.height()/2, newsize.width(), newsize.height());

#if 1
		view_->move(0, 0);
		view_->resize(newsize);
#else
		view_->move(20, 20);
		view_->resize(newsize.width()-40, newsize.height()-40);
#endif
		aview->setGeometry(
			20 + scene_.sceneRect().left()
			, 150  + scene_.sceneRect().top()
			, scene_.sceneRect().width() - 50
			, scene_.sceneRect().height() - 200);
	}

private:
	QGLWidget * gl_layer_;
	View * view_;
	QGraphicsScene scene_;
	QScopedPointer<GrymQtAndroidViewGraphicsProxy> aview;
	QPixmap bgPix;
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
	QApplication::setGraphicsSystem(QLatin1String("opengl"));
    QApplication app(argc, qargs.data());

	QPixmap kineticPix(":/images/kinetic.png");

	// Ui
	qDebug()<<TAG<<"Creating view...";
	QScopedPointer<MyWindow> window(new MyWindow());

	QList<MyAnimatedPixmap *> items;
    for (int i = 0; i < 64; ++i) {
		MyAnimatedPixmap *item = new MyAnimatedPixmap(kineticPix);
        item->setOffset(-kineticPix.width()/2, -kineticPix.height()/2);
        item->setZValue(i);
        items << item;
		window->scene()->addItem(item);
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

	window->scene()->addItem(buttonParent);
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
		MyAnimatedPixmap *item = items.at(i);
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

    qDebug()<<TAG<<"Display window...";

	#ifdef Q_OS_ANDROID
		window->showFullScreen();
	#else
		window->show();
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
