#include "MainWindow.h"
#include <QMenuBar>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1000, 700);
    view = new CadView(this);
    setCentralWidget(view);

    QMenu *menu = menuBar()->addMenu("Draw");
    menu->addAction("Line", this, &MainWindow::onDrawLine);
    menu->addAction("Arc", this, &MainWindow::onDrawArc);
    menu->addAction("Cube", this, &MainWindow::onDrawCube);
}

void MainWindow::onDrawLine() { view->setMode(CadView::Mode::DrawLine); }
void MainWindow::onDrawArc()  { view->setMode(CadView::Mode::DrawArc); }
void MainWindow::onDrawCube() { view->setMode(CadView::Mode::DrawCube); }
