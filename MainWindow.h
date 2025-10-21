#pragma once
#include <QMainWindow>
#include "CadView.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onDrawLine();
    void onDrawArc();
    void onDrawCube();

private:
    CadView *view;
};
