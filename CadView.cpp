#include "CadView.h"
#include <QMouseEvent>
#include <QApplication>
#include <QPaintEngine>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <AIS_Shape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GC_MakeCircle.hxx>
#include <Geom_Circle.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <Quantity_Color.hxx>

#ifdef __linux__
#include <Xw_Window.hxx>
#endif

CadView::CadView(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(400, 300);
}

QPaintEngine* CadView::paintEngine() const
{
    // Return null to prevent Qt from painting
    return nullptr;
}

void CadView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (m_viewer.IsNull())
    {
        // Use a single-shot timer to ensure window is fully mapped
        QTimer::singleShot(0, this, &CadView::initViewer);
    }
}

void CadView::initViewer()
{
    if (!m_viewer.IsNull())
        return;

    // Ensure the widget has a valid native window handle
    WId window_id = winId();
    if (window_id == 0)
    {
        qWarning("Invalid window ID");
        return;
    }

    // Process events to ensure window is fully created
    QApplication::processEvents();

    try
    {
        // --- OCCT OpenGL driver ---
        Handle(Aspect_DisplayConnection) disp = new Aspect_DisplayConnection();
        Handle(OpenGl_GraphicDriver) drv = new OpenGl_GraphicDriver(disp);

        m_viewer = new V3d_Viewer(drv);
        m_view = m_viewer->CreateView();
        m_context = new AIS_InteractiveContext(m_viewer);

// --- Create OCCT window from Qt widget ---
#ifdef __linux__
        Handle(Xw_Window) wind = new Xw_Window(disp, (Aspect_Drawable)window_id);
        m_view->SetWindow(wind);

        if (!wind->IsMapped())
            wind->Map();
#endif

        // Configure view
        m_view->SetBackgroundColor(Quantity_NOC_BLACK);
        m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_WHITE, 0.08);
        m_view->SetProj(V3d_XposYposZpos);

        // Must be called after SetWindow
        m_view->MustBeResized();
        m_view->ZFitAll();

        // --- Initialize OCAF document ---
        Handle(TDocStd_Application) app = new TDocStd_Application();
        app->NewDocument("BinOcaf", m_doc);

        qDebug("OCCT viewer initialized successfully");
    }
    catch (Standard_Failure const& e)
    {
        qWarning("OCCT initialization failed: %s", e.GetMessageString());
    }
}

void CadView::paintEvent(QPaintEvent *)
{
    if (!m_view.IsNull())
        m_view->Redraw();
}

void CadView::resizeEvent(QResizeEvent *)
{
    if (!m_view.IsNull())
        m_view->MustBeResized();
}

void CadView::setMode(Mode m)
{
    m_mode = m;
    m_points.clear();
}

gp_Pnt CadView::convertClickToPoint(const QPoint &p)
{
    if (m_view.IsNull())
        return gp_Pnt(0, 0, 0);

    Standard_Integer x = p.x();
    Standard_Integer y = p.y();
    Standard_Real X, Y, Z;

    m_view->Convert(x, y, X, Y, Z);

    return gp_Pnt(X, Y, Z);
}

void CadView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || m_view.IsNull())
        return;

    gp_Pnt p = convertClickToPoint(event->pos());
    m_points.append(p);

    if (m_mode == Mode::DrawLine && m_points.size() == 2)
    {
        drawLine(m_points[0], m_points[1]);
        m_points.clear();
    }
    else if (m_mode == Mode::DrawArc && m_points.size() == 3)
    {
        drawArc(m_points[0], m_points[1], m_points[2]);
        m_points.clear();
    }
    else if (m_mode == Mode::DrawCube)
    {
        drawCube(p);
        m_points.clear();
    }
}

void CadView::drawLine(const gp_Pnt &p1, const gp_Pnt &p2)
{
    if (m_context.IsNull())
        return;

    TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
    Handle(AIS_Shape) shape = new AIS_Shape(edge);
    m_context->Display(shape, Standard_True);
//    m_view->FitAll();
}

void CadView::drawArc(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &p3)
{
    if (m_context.IsNull())
        return;

    try {
        GC_MakeCircle circle_maker(p1, p2, p3);
        if (circle_maker.IsDone()) {
            Handle(Geom_Circle) circle = circle_maker.Value();
            TopoDS_Edge arc = BRepBuilderAPI_MakeEdge(circle, p1, p3);
            Handle(AIS_Shape) shape = new AIS_Shape(arc);
            m_context->Display(shape, Standard_True);
//            m_view->FitAll();
        }
    } catch (Standard_Failure const&) {
        qWarning("Failed to create arc - points may be collinear");
    }
}

void CadView::drawCube(const gp_Pnt &p)
{
    if (m_context.IsNull())
        return;

    BRepPrimAPI_MakeBox box_maker(p, 50.0, 50.0, 50.0);
    TopoDS_Shape box = box_maker.Shape();
    Handle(AIS_Shape) shape = new AIS_Shape(box);
    m_context->Display(shape, Standard_True);
//    m_view->FitAll();
}
