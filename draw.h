#ifndef DRAW_H
#define DRAW_H
#include<QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class draw; }
QT_END_NAMESPACE

class draw : public QWidget
{
public:
    draw(QWidget *parent= 0);
protected:
    void paintEvent(QPaintEvent *event);
    void drawLines(QPainter *qp);
};

#endif // DRAW_H
