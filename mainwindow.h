#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRandomGenerator>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void spawnBot(int x, int y);
private:
    Ui::MainWindow *ui;

protected:
    void paintEvent(QPaintEvent *event);
    void drawLines(QPainter *qp);

private:
    void repaint_world(QPainter *qp);
    void paint_empty_cell(QPainter * qp, int x, int y);

    QRandomGenerator rnd;
    QTimer* timer;

    QImage* img;

private slots:
//    void on_bPlus10_clicked();
//    void on_bPlus100_clicked();
//    void on_bMinus10_clicked();
//    void on_bMinus100_clicked();
//    void on_bTPlus1_clicked();
//    void on_bTPlus01_clicked();
//    void on_bTMinus1_clicked();
//    void on_bTMinus01_clicked();
//    void on_bRefresh_clicked();

    void timer_fired();
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
