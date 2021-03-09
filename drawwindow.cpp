#include "drawwindow.h"
#include "ui_drawwindow.h"

#include <QPainter>
#include <QPen>

DrawWindow::DrawWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DrawWindow)
{
    ui->setupUi(this);
    QPainter qp(this);
    drawLines(&qp);
}

DrawWindow::~DrawWindow()
{
    delete ui;
}

void DrawWindow::drawLines(QPainter *qp)
{
    QPen p(Qt::black, 2, Qt::SolidLine);
    qp -> setPen(p);
    qp -> drawLine(20, 40, 250, 40);
}
