#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <chrono>

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QTimer>
#include <QFile>

int cellSize = 10;
int pixHeight;
int pixWidth;
float Thicknesses = 0.5;
bool needRedraw = false;

FILE *EVO_output;

int cnt = 0;
int  REPAINT = 10;

long q;


int botsCount;

int makeCoord(int x, int y, int& ltX, int& ltY) {

    ltX = x*cellSize;
    ltY = y*cellSize;
    return ltX;
}

enum class GENOME {
    photosynthesis = 0,
    goUp = 1,
    goUr = 2,
    goR = 3,
    goDr = 4,
    goD = 5,
    goDl = 6,
    goL = 7,
    goUl = 8,
    eat = 9,
    wallU = 10,
    wallR = 11,
    wallD = 12,
    wallL = 13,
    destroyWall = 14

};

class bot {
public:
    bot();
    bot(const bot& other);
    int comand;
    int energy;
    int age;
    int type;
    int id;
    int disposition = 0;

    float factor = 2;
    float Mfactor = 3;

    bool first = true;
    bool dead = false;
    bool wall = false;

    int meatE = 0;          // r
    int phsyE = 0;          // g
    int minerE = 0;        // b
    std::vector<GENOME> genome;

    void paint(QPainter* qp, int x, int y);
    void paintDead(QPainter* qp, int x, int y);
    void live(int x, int y);
    GENOME mutation();
    static QRandomGenerator rnd;

    bool try_eat_neighbor(int me_x, int me_y, int victim_x, int victim_y, float & factor);
};

QRandomGenerator bot::rnd;

bot::bot()  : comand(0), energy(1), age(1), dead(false)
{
    int TOTAL_GENOME = 128;
    genome = std::vector<GENOME>(TOTAL_GENOME, GENOME::photosynthesis);
    genome[rand()%65] = mutation();

    if (EVO_output != nullptr)
        fclose(EVO_output);

//    EVO_output = fopen("C:\\Users\\2805n\\Desktop\\cur_file.txt", "at");
//    fprintf(EVO_output, );

    disposition = rand()%3 - 3;

}

bot::bot(const bot &other) : comand(0), energy(1), age(1), dead(false)
{
    genome = other.genome;
    genome[rand()%65] = mutation();
    disposition = rand()%3 - 3;
}

std::vector<std::vector<bot* > > world;





GENOME bot::mutation()   //                                                                              MUTATION
{
    int r = rand()%5;
    switch(r) {
    case 0: return GENOME::goD;
    case 1: return GENOME::goR;
    case 2: return GENOME::goUp;
    case 3: return GENOME::goL;
    case 4: return GENOME::eat;
//    case 5: return GENOME::wallD;
//    case 6: return GENOME::wallR;
//    case 7: return GENOME::wallU;
//    case 8: return GENOME::wallL;
//    case 10: return GENOME::destroyWall;

    default: return GENOME::photosynthesis;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QDesktopWidget *d = QApplication::desktop();

    pixHeight = d->height();
    pixWidth = d->width();
    if ((pixWidth > 3000) && (pixHeight < 2000)) {
        pixWidth = pixWidth / 2;
    }
    int worldWidth = pixWidth / cellSize;
    int worldHeight = pixHeight / cellSize;
    world.resize(worldWidth);
    for(int i = 0; i < worldWidth; ++i) {
        world[i].resize(worldHeight, nullptr);
    }
    spawnBot(rnd.bounded(worldWidth), rnd.bounded(worldHeight));

    ui->setupUi(this);

    timer = new QTimer(this);
    img = new QImage(worldWidth, worldHeight, QImage::Format_RGB888);

    connect(timer, &QTimer::timeout, this, &MainWindow::timer_fired);
    timer->start(2);
}

MainWindow::~MainWindow()
{
    delete timer;
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *e) {

    Q_UNUSED(e);

    img->fill(QColor(200, 200, 200));
    QPainter qp(this);
    drawLines(&qp);
    repaint_world(&qp);
    qp.end();
}

void MainWindow::drawLines(QPainter *qp)  /// прорисовка сетки
{
    QPen p(Qt::black, Thicknesses, Qt::SolidLine);
    qp -> setPen(p);

        int worldWidth = world.size();
        int worldHeight = world[0].size();
        for(int i = 0; i <= worldWidth; i++)
            qp->drawLine(i*cellSize, 0, i*cellSize, pixHeight);
        for(int i = 0; i < worldHeight; i++)
            qp->drawLine(0, i*cellSize, pixWidth, i*cellSize);
}

void MainWindow::repaint_world(QPainter *qp)
{    
    //    cnt = (++cnt) % REPAINT;
    //    if(cnt) return;
    for(int i = 0; i < world.size(); ++i) {
        for(int j = 0; j < world[i].size(); ++j) {
            bot* curbot = world[i][j];
            if(curbot != nullptr) {
                curbot->paint(qp, i, j);
            }
            //            else
            //paint_empty_cell(qp, i, j);
        }
    }
}

void MainWindow::paint_empty_cell(QPainter *qp, int x, int y)
{
    int ltX, ltY;
    makeCoord(x, y, ltX, ltY);
    qp->setBrush(QBrush(QColor(200, 100, 200)));
    qp->drawRect(ltX, ltY, cellSize, cellSize);
}

void spawnBot(int x, int y, const bot& other)
{
    world[x][y] = new bot(other);
}

void MainWindow::spawnBot(int x, int y)
{
    world[x][y] = new bot();
}

void MainWindow::timer_fired() /// перед каждой итерацией - обнуление рандомайзера и обход ботов
{
    q++;
    qDebug() << q;
    srand(time(NULL));
    for(int i = 0; i < world.size(); ++i) {
        for(int j = 0; j < world[i].size(); ++j) {
            bot* curbot = world[i][j];
            if(curbot != nullptr) {
                curbot->live(i, j);
            }
        }
    }
    this->repaint();
}

void bot::paint(QPainter *qp, int x, int y)  /// отрисовка бота
{
    int ltX, ltY;
    makeCoord(x, y, ltX, ltY);
    qp->setBrush(QBrush(QColor(meatE % 255, phsyE % 255, minerE % 255)));
    qp->drawRect(ltX, ltY, cellSize, cellSize);
}


bool checkEmptyCell (int x, int y) {
    bool res;
    if (world[(x + 1 + world.size()) % world.size()][y] == nullptr || world[(x + 1 + world.size()) % world.size()][y]->wall == true)
        res = true;
    else {
        if (world[x][(y - 1 + world[0].size()) % world[0].size()] == nullptr || world[x][(y - 1 + world[0].size()) % world[0].size()] -> wall == true)
            res = true;
        else {
            if (world[(x - 1 + world.size()) % world.size()][y] == nullptr || world[(x - 1 + world.size()) % world.size()][y] -> wall == true)
                res = true;
            else {
                if (world[x][(y + 1 + world[0].size()) % world[0].size()] == nullptr || world[x][(y + 1 + world[0].size()) % world[0].size()] -> wall == true)
                    res = true;
                else {
                    res = false;
                }
            }
        }
    }
}


QRandomGenerator rnd2;
void bot::live(int x, int y)
{
    if (dead == false  && wall == false) {

        if(age <= 2000) {
            factor += 0.05;
            age++;
        }
//        if(age > 2000) {
//            factor -=0.05;
//            age++;
//        }

        int worldWidth = world.size();
        int worldHeight = world[0].size();
        if(energy >= 250)   /// размножение
        {
            if (checkEmptyCell(x, y) == true || first == true) {
            int r = rand()%5 + 1;
MakeStep:
            switch (r) {
            case 1:
                if((x < (worldWidth -1)) && (world[(x + 1 + worldWidth) % worldWidth][y] == nullptr)) {
                    spawnBot(x + 1, y, *this);
                    world[x+1][y]->first = false;
                    first = false;
                    energy = energy / 2;
                }
                else {
                    r += rand()%3 + 1;
                    goto MakeStep;
                }
                break;
            case 2:
                if ((y < (worldHeight-1)) && (world[x][(y + 1 + worldHeight) % worldHeight] == nullptr)) {
                    spawnBot(x, y + 1, *this);
                    energy = energy / 2;
                    world[x][y+1]->first = false;
                    first = false;
                }
                else {
                    r++;
                    goto MakeStep;
                }
                break;
            case 3:
                if ((x > 0) && (world[(x - 1 + worldWidth) % worldWidth][y] == nullptr)) {
                    spawnBot(x -1, y, *this);
                    energy = energy / 2;
                    world[x-1][y]->first = false;
                    first = false;
                }
                else {
                    r++;
                    goto MakeStep;
                }
                break;
            case 4:
                if ((y > 0) && (world[x][(y + 1 + worldHeight) % worldHeight] == nullptr)) {
                    spawnBot(x, y - 1, *this);
                    energy = energy / 2;
                    world[x][y-1]->first = false;
                    first = false;
                }
                else {
                    r += rand()%3 - 4;
                    goto MakeStep;
                }
            }
            }

        }
        int r = rand()%1001;
        if ((energy < 0) || (age >= 2000 + r)) {   /// механизм смерти бота
            QPainter *qp;
            //world[x][y] = new bot;
            world[x][y]->dead = true;
            world[x][y]->meatE = 130;
            world[x][y]->phsyE = 130;
            world[x][y]->minerE = 130;
        }
        //if (energy <= 200 && energy>10){
        try_eat_neighbor(x, y, x+1, y, factor);
        try_eat_neighbor(x, y, x, y - 1, factor);
        try_eat_neighbor(x, y, x-1, y, factor);
        try_eat_neighbor(x, y, x, y+1, factor);
//        //}

        switch (genome[comand]) { /// команды генома
        case GENOME::photosynthesis:
            energy+=factor;
            if (phsyE < 255 - factor)
                phsyE+=factor;
            break;
        case GENOME::goUp:
            if(/*y < worldHeight - 1 && */world[x][(y + 1 + worldHeight) % worldHeight] == nullptr) {
                world[x][(y + 1 + worldHeight) % worldHeight] = this;
                world[x][y] = nullptr;
            }
            energy-=3;
            break;
        case GENOME::goR:
            if(/*x < worldWidth- 1 && */world[(x + 1 + worldWidth) % worldWidth][y] == nullptr) {
                world[(x + 1 + worldWidth) % worldWidth][y] = this;
                world[x][y] = nullptr;
            }
            energy-=3;
            break;
        case GENOME::goD:
            if(/*y > 0 && */world[x][(y - 1 + worldHeight) % worldHeight] == nullptr) {
                world[x][(y - 1 + worldHeight) % worldHeight] = this;
                world[x][y] = nullptr;
            }
            energy-=3;
            break;
        case GENOME::goL:
            if(/*x > 0 && */world[(x - 1 + worldWidth) % worldWidth][y] == nullptr) {
                world[(x + 1 + worldWidth) % worldWidth][y] = this;
                world[x][y] = nullptr;
            }
            energy-=3;
            break;
        case GENOME::eat:
            if ((disposition <=0) && (200 <= age) && (age <= 2000) && (energy <= 250)) {
                int r = rand()%8 + 1;               
                int c_x = 0, c_y = 0;
                for(int t = 0; t < 8; ++t) {
                    switch(r) {
                    case 1: //вправо
                        c_x =( x + 1) % worldWidth;
                        c_y = y;
                        break;
                    case 2: //ВПРАВО-вниз
                        c_x =( x + 1) % worldWidth;
                        c_y = (y - 1 + worldHeight) % worldHeight;
                        break;
                    case 3: //вниз
                        c_x = x;
                        c_y = (y - 1 + worldHeight) % worldHeight;
                        break;
                    case 4: //вниз влево
                        c_x = (x - 1 + worldWidth) % worldWidth;
                        c_y = (y - 1 + worldHeight) % worldHeight;
                        break;
                    case 5: //влево
                        c_x = (x - 1 + worldWidth) % worldWidth;
                        c_y = y;
                        break;
                    case 6: //влево-вверх
                        c_x = (x - 1 + worldWidth) % worldWidth;
                        c_y = (y + 1) % worldHeight;
                        break;
                    case  7: //вверх
                        c_x = x;
                        c_y = (y + 1) % worldHeight;
                        break;
                    case 8: //вверх-вправо
                        c_x =( x + 1) % worldWidth;
                        c_y = (y + 1) % worldHeight;
                        break;
                    }
                    if(world[c_x][c_y] != nullptr && world[c_x][c_y]->wall == false) {
                        delete world[c_x][c_y];
                        world[c_x][c_y] = world[x][y];
                        world[x][y] = nullptr;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                        break;
                    }   else {
                        r = ((r+1) % 8) + 1;
                    }
                }


                /*
                switch (r) {
                case 1:
                    if((x < ((x - 1 + worldWidth) % worldWidth)) && (world[(x + 1 + worldWidth) % worldWidth][y] != nullptr) && (world[x + 1][y]->wall == false)) {
                        world[x+1][y] = world[x][y];
                        world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;

                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 2:
                    if((x < ((x - 1 + worldWidth) % worldWidth)) && (world[(x + 1 + worldWidth) % worldWidth][(y - 1 + worldWidth) % worldWidth] != nullptr)) {
                        world[x+1][y-1] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 3:
                    if((x < ((x - 1 + worldWidth) % worldWidth)) && (world[x][(y - 1 + worldWidth) % worldWidth] != nullptr) && (world[x][y - 1]->wall == false)) {
                        world[x][y-1] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 4:
                    if((x > 0) && (y > 0) && (world[(x - 1 + worldWidth) % worldWidth][(y - 1 + worldWidth) % worldWidth] != nullptr) && (world[x - 1][y - 1]->wall == false)) {
                        world[x-1][y-1] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 5:
                    if((x > 0) && (world[(x - 1 + worldWidth) % worldWidth][y] != nullptr) && (world[(x - 1 + worldWidth) % worldWidth][y]->wall == false)) {
                        world[x-1][y] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 6:
                    if((x > 0) && (y < worldHeight-1) && (world[(x - 1 + worldWidth) % worldWidth][(y + 1 + worldWidth) % worldWidth] != nullptr) && (world[x - 1][y + 1]->wall == false)) {
                        world[x-1][y+1] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 7:
                //(world[x][(y + 1 + worldHeight) % worldHeight] != nullptr) && comment here
                    if((world[x][(y + 1 + worldWidth) % worldWidth] != nullptr)) {
                        world[x][(y + 1 + worldWidth) % worldWidth] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    else {
                        r++;
                        goto omt;
                    }
                    break;

                case 8:
                    if((x < ((x - 1 + worldWidth) % worldWidth)) && (y < ((y - 1 + worldWidth) % worldWidth)) && (world[(x + 1 + worldWidth) % worldWidth][(y + 1 + worldWidth) % worldWidth] != nullptr) && (world[x + 1][y + 1]->wall == false)) {
                        world[x+1][y+1] = world[x][y];
                        //world[x][y] = 0;
                        disposition --;
                        if (meatE == 0)
                            meatE += 215;
                        if (phsyE >= 10)
                            phsyE -= 10;
                        factor = 1.5;
                        energy += 30;
                    }
                    factor = -0.5;
                    break;
                }
                */
                for (int i = 0; i <= 7; ++i) {
                    r = rand()%65;
                    genome[r] = mutation();
                }

            }

        case GENOME::wallU:
            if (energy >= 200) {
            spawnBot(x, (y - 1 + worldHeight)% worldHeight, *this);
            world[x][(y - 1 + worldHeight) % worldHeight]->wall = true;
            world[x][(y - 1 + worldHeight) % worldHeight]->energy = 200;
            world[x][(y - 1 + worldHeight) % worldHeight]->meatE = 150;
            world[x][(y - 1 + worldHeight) % worldHeight]->phsyE = 75;
            world[x][(y - 1 + worldHeight) % worldHeight]->minerE = 0;
            }
            break;

        case GENOME::wallL:
            if (energy >= 200) {
            spawnBot((x + 1 + worldWidth) % worldWidth, y, *this);
            world[(x + 1 + worldWidth) % worldWidth][y]->wall = true;
            world[(x + 1 + worldWidth) % worldWidth][y]->energy = 200;
            world[(x + 1 + worldWidth) % worldWidth][y]->meatE = 150;
            world[(x + 1 + worldWidth) % worldWidth][y]->phsyE = 75;
            world[(x + 1 + worldWidth) % worldWidth][y]->minerE = 0;
            }
            break;

        case GENOME::wallD:
            if (energy >= 200) {
            spawnBot(x, (y + 1 + worldHeight)% worldHeight, *this);
            world[x][(y + 1 + worldHeight) % worldHeight]->wall = true;
            world[x][(y + 1 + worldHeight) % worldHeight]->energy = 200;
            world[x][(y + 1 + worldHeight) % worldHeight]->meatE = 150;
            world[x][(y + 1 + worldHeight) % worldHeight]->phsyE = 75;
            world[x][(y + 1 + worldHeight) % worldHeight]->minerE = 0;
            }
            break;

        case GENOME::wallR:
            if (energy >= 200) {
            spawnBot((x - 1 + worldWidth) % worldWidth, y, *this);
            world[(x - 1 + worldWidth) % worldWidth][y]->wall = true;
            world[(x - 1 + worldWidth) % worldWidth][y]->energy = 200;
            world[(x - 1 + worldWidth) % worldWidth][y]->meatE = 150;
            world[(x - 1 + worldWidth) % worldWidth][y]->phsyE = 75;
            world[(x - 1 + worldWidth) % worldWidth][y]->minerE = 0;
            }
            break;

//        case GENOME::destroyWall:
//            if () {

//            }
        }



        comand = (comand + 1) % genome.size();

    }
}

bool bot::try_eat_neighbor(int me_x, int me_y, int victim_x, int victim_y, float & factor)
{
    int worldWidth = world.size();
    int worldHeight = world[0].size();

    if(victim_x >= 0 && (victim_x < worldWidth) && victim_y >= 0 && victim_y < worldHeight && world[victim_x][victim_y] != nullptr && world[victim_x][victim_y]->dead) {
        energy += world[victim_x][victim_y]->energy / 3;
        delete world[victim_x][victim_y];
        world[victim_x][victim_y] = nullptr;
//        world[victim_x][victim_y] = world[me_x][me_y];
//        world[me_x][me_y] = nullptr;
        if (disposition < 0)
            disposition++;
        if (disposition > 0)
            disposition--;

        if (meatE >= 10)
            meatE -= 10;
        if (phsyE >= 10)
            phsyE -= 10;

        if (minerE <= 154) {
            minerE += 100;
        }

        factor -=5;
        return true;
    }
    return false;
}
