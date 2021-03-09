/*#include "draw.h"
#include"ui_draw.h"
#include<QWidget>
#include <QPainter>

draw::draw(QWidget *parent) : QWidget(parent), ui(new Ui::draw)
{
    ui->setupUi(this);
}

void draw::paintEvent(QPaintEvent *e) {

    Q_UNUSED(e);

    QPainter qp(this);
    drawLines(&qp);
}

void draw::drawLines(QPainter *qp)
{
    QPen p(Qt::black, 2, Qt::SolidLine);
    qp -> setPen(p);
    qp -> drawLine(20, 40, 250, 40);
}
*/
