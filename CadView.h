#pragma once
#include <QWidget>
#include <QList>
#include <QTimer>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <TDocStd_Document.hxx>
#include <TDocStd_Application.hxx>
#include <gp_Pnt.hxx>

class CadView : public QWidget
{
    Q_OBJECT
public:
    enum class Mode { None, DrawLine, DrawArc, DrawCube };
    explicit CadView(QWidget *parent = nullptr);
    void setMode(Mode m);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    QPaintEngine* paintEngine() const override;

private slots:
    void initViewer();

private:
    gp_Pnt convertClickToPoint(const QPoint &p);
    void drawLine(const gp_Pnt &p1, const gp_Pnt &p2);
    void drawArc(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &p3);
    void drawCube(const gp_Pnt &p);

private:
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    Mode m_mode = Mode::None;
    QList<gp_Pnt> m_points;
    Handle(TDocStd_Document) m_doc;
};
