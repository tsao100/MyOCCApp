#include "CadView.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMouseEvent>
#include <QTimer>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GC_MakeCircle.hxx>
#include <Geom_Circle.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#ifdef __linux__
#include <Xw_Window.hxx>
#endif

#ifdef _WIN32
#include <WNT_Window.hxx>
#endif

CadView::CadView(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(400, 300);
}

QPaintEngine *CadView::paintEngine() const { return nullptr; }

void CadView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    if (m_view.IsNull()) {
        QTimer::singleShot(0, this, &CadView::initViewer);
    }
}

void CadView::initViewer() {
    if (!m_viewer.IsNull())
        return;

    WId window_id = winId();
    if (window_id == 0) {
        qWarning("Invalid window ID");
        return;
    }

    QApplication::processEvents();

    try {
        Handle(Aspect_DisplayConnection) disp = new Aspect_DisplayConnection();
        Handle(OpenGl_GraphicDriver) drv = new OpenGl_GraphicDriver(disp);

        m_viewer = new V3d_Viewer(drv);
        m_view = m_viewer->CreateView();
        m_context = new AIS_InteractiveContext(m_viewer);

#ifdef __linux__
        Handle(Xw_Window) wind = new Xw_Window(disp, (Aspect_Drawable)window_id);
        m_view->SetWindow(wind);

        if (!wind->IsMapped())
            wind->Map();
#elif defined(_WIN32)
        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)window_id);
        m_view->SetWindow(wind);

        if (!wind->IsMapped())
            wind->Map();
#endif

        m_view->SetBackgroundColor(Quantity_NOC_BLACK);
        m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.08);
        m_view->SetProj(V3d_XposYposZpos);

        m_view->MustBeResized();
        m_view->ZFitAll();

        Handle(TDocStd_Application) app = new TDocStd_Application();
        app->NewDocument("BinOcaf", m_doc);

        qDebug("OCCT viewer initialized successfully");
    } catch (Standard_Failure const &e) {
        qWarning("OCCT initialization failed: %s", e.GetMessageString());
    }
}

void CadView::paintEvent(QPaintEvent *) {
    if (!m_view.IsNull()) {
        try {
            m_view->Redraw();
        } catch (Standard_Failure const &e) {
            qWarning("Paint error: %s", e.GetMessageString());
        }
    }
}

void CadView::resizeEvent(QResizeEvent *) {
    if (!m_view.IsNull())
        m_view->MustBeResized();
}

void CadView::setMode(Mode m) {
    m_mode = m;
    m_points.clear();
}

gp_Pnt CadView::convertClickToPoint(const QPoint &p) {
    if (m_view.IsNull())
        return gp_Pnt(0, 0, 0);

    Standard_Integer x = p.x();
    Standard_Integer y = p.y();
    Standard_Real X, Y, Z;

    try {
        m_view->Convert(x, y, X, Y, Z);
    } catch (Standard_Failure const &e) {
        qWarning("Convert error: %s", e.GetMessageString());
        return gp_Pnt(0, 0, 0);
    }

    return gp_Pnt(X, Y, Z);
}

void CadView::mousePressEvent(QMouseEvent *event) {
    if (m_view.IsNull() || m_context.IsNull()) {
        qWarning("View or context not initialized");
        return;
    }

    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() == Qt::RightButton) {
        bool shiftPressed = (event->modifiers() & Qt::ShiftModifier);
        if (shiftPressed) {
            m_rotatingRoll = true;
            m_lastRotatePos = event->pos();
            setCursor(Qt::SizeAllCursor);
        } else {
            m_rotating = true;
            m_lastRotatePos = event->pos();
            m_view->StartRotation(event->pos().x(), event->pos().y());
            setCursor(Qt::SizeAllCursor);
        }
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    gp_Pnt p = convertClickToPoint(event->pos());
    m_points.append(p);

    if (m_mode == Mode::DrawLine && m_points.size() == 2) {
        drawLine(m_points[0], m_points[1]);
        m_points.clear();
    } else if (m_mode == Mode::DrawArc && m_points.size() == 3) {
        drawArc(m_points[0], m_points[1], m_points[2]);
        m_points.clear();
    } else if (m_mode == Mode::DrawCube) {
        drawCube(p);
        m_points.clear();
    }
}

void CadView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton && m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        return;
    }

    if (event->button() == Qt::RightButton && (m_rotating || m_rotatingRoll)) {
        m_rotating = false;
        m_rotatingRoll = false;
        setCursor(Qt::ArrowCursor);
        return;
    }
}

void CadView::mouseMoveEvent(QMouseEvent *event) {
    if (m_panning && !m_view.IsNull()) {
        QPoint cur = event->pos();
        int dx = cur.x() - m_lastPanPos.x();
        int dy = cur.y() - m_lastPanPos.y();
        m_view->Panning(dx, dy);
        m_lastPanPos = cur;
        update();
        return;
    }

    if (m_rotating && !m_view.IsNull()) {
        m_view->Rotation(event->pos().x(), event->pos().y());
        update();
        return;
    }

    if (m_rotatingRoll && !m_view.IsNull()) {
        QPoint cur = event->pos();
        int dx = cur.x() - m_lastRotatePos.x();
        Standard_Real angle = dx * 0.01;
        m_view->Turn(angle);
        m_lastRotatePos = cur;
        update();
        return;
    }
}

void CadView::wheelEvent(QWheelEvent *event) {
    if (m_view.IsNull())
        return;

    int steps = event->angleDelta().y() / 120;
    if (steps == 0)
        return;

    int mx = int(event->position().x());
    int my = int(event->position().y());

    m_view->StartZoomAtPoint(mx, my);
    m_view->ZoomAtPoint(mx, my, mx, my + steps * 10);

    update();
}

void CadView::drawLine(const gp_Pnt &p1, const gp_Pnt &p2) {
    if (m_context.IsNull())
        return;

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
    Handle(AIS_Shape) shape = new AIS_Shape(edge);
    m_context->Display(shape, Standard_True);

    m_shapes.push_back(edge);
}

void CadView::drawArc(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &p3) {
    if (m_context.IsNull())
        return;

    try {
        GC_MakeCircle circle_maker(p1, p2, p3);
        if (circle_maker.IsDone()) {
            Handle(Geom_Circle) circle = circle_maker.Value();
            TopoDS_Edge arc = BRepBuilderAPI_MakeEdge(circle, p1, p3);
            Handle(AIS_Shape) shape = new AIS_Shape(arc);
            m_context->Display(shape, Standard_True);

            m_shapes.push_back(arc);
        }
    } catch (Standard_Failure const &) {
        qWarning("Failed to create arc - points may be collinear");
    }
}

void CadView::drawCube(const gp_Pnt &p) {
    if (m_context.IsNull())
        return;

    BRepPrimAPI_MakeBox box_maker(p, 50.0, 50.0, 50.0);
    TopoDS_Shape box = box_maker.Shape();
    Handle(AIS_Shape) shape = new AIS_Shape(box);
    m_context->Display(shape, Standard_True);

    m_shapes.push_back(box);
}

bool CadView::saveToDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select directory to save BREP files"));
    if (dir.isEmpty())
        return false;

    QDir qdir(dir);
    if (!qdir.exists())
        return false;

    int idx = 1;
    for (const TopoDS_Shape &s : m_shapes) {
        QString fname = qdir.filePath(QString("shape_%1.brep").arg(idx++));
        try {
            if (!BRepTools::Write(s, fname.toLocal8Bit().constData())) {
                qWarning() << "Failed to write" << fname;
            }
        } catch (Standard_Failure const &e) {
            qWarning("Exception writing BREP: %s", e.GetMessageString());
        }
    }

    qDebug() << "Saved" << (idx - 1) << "shapes to" << dir;
    return true;
}

bool CadView::loadFromDirectory() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select directory to load BREP files"));
    if (dir.isEmpty())
        return false;

    QDir qdir(dir);
    QStringList breps =
        qdir.entryList(QStringList() << "*.brep" << "*.BREP" << "*.brep.gz",
                       QDir::Files, QDir::Name);
    if (breps.isEmpty()) {
        qDebug() << "No .brep files found in" << dir;
        return false;
    }

    if (!m_context.IsNull()) {
        m_context->EraseAll(Standard_False);
        m_shapes.clear();
    }

    BRep_Builder builder;
    for (const QString &f : breps) {
        QString full = qdir.filePath(f);
        TopoDS_Shape shape;
        try {
            if (BRepTools::Read(shape, full.toLocal8Bit().constData(), builder)) {
                Handle(AIS_Shape) ash = new AIS_Shape(shape);
                m_context->Display(ash, Standard_True);
                m_shapes.push_back(shape);
            } else {
                qWarning() << "Failed to read" << full;
            }
        } catch (Standard_Failure const &e) {
            qWarning("Exception reading BREP: %s", e.GetMessageString());
        }
    }

    m_view->ZFitAll();
    update();
    qDebug() << "Loaded" << m_shapes.size() << "shapes from" << dir;
    return true;
}
