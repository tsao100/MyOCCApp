#pragma once

#include <QWidget>
#include <QPoint>
#include <vector>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <TDocStd_Document.hxx>
#include <TopoDS_Shape.hxx>

class CadView : public QWidget
{
    Q_OBJECT
public:
    enum class Mode { None, DrawLine, DrawArc, DrawCube };

    explicit CadView(QWidget *parent = nullptr);
    QPaintEngine* paintEngine() const override;

    void setMode(Mode m);

    bool saveToDirectory();
    bool loadFromDirectory();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void initViewer();

private:
    gp_Pnt convertClickToPoint(const QPoint &p);
    void drawLine(const gp_Pnt &p1, const gp_Pnt &p2);
    void drawArc(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &p3);
    void drawCube(const gp_Pnt &p);

private:
    Mode m_mode = Mode::None;
    QList<gp_Pnt> m_points;

    Handle(V3d_View)          m_view;
    Handle(V3d_Viewer)        m_viewer;
    Handle(AIS_InteractiveContext) m_context;
    Handle(TDocStd_Document)  m_doc;

    bool m_panning = false;
    QPoint m_lastPanPos;

    bool m_rotating = false;
    bool m_rotatingRoll = false;
    QPoint m_lastRotatePos;

    std::vector<TopoDS_Shape> m_shapes;
};
