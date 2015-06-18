#include "painter.h"
#include "ui_painter.h"
#include "qpainter.h"
#include <QPixMap>

Painter::Painter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Painter)
{
    ui->setupUi(this);
//    setHeight(750);
//    setWidth(1000);
    setFixedSize(this->width(),this->height());
//    this->setStyleSheet("background-color:white;");
    ui->label->setStyleSheet("background-color:white;");
    fudge = 7;//y-offset between drawing (user input) and painting (simulation).
}

Painter::~Painter()
{
    delete ui;
}

/**
 * @brief draws a line using specified coordinates, color, and style into the painter label.
 * @param startX
 * @param startY
 * @param endX
 * @param endY
 * @param color
 * @param lineStyle
 */
void Painter::paintCommand(int startX, int startY, int endX, int endY, QString color, QString lineStyle){

    //make an image to paint to.
    QImage* temp;
    if(ui->label->pixmap() == 0){
        temp = new QImage(ui->label->width(),ui->label->height(),QImage::Format_ARGB32);
    }else{
        temp = new QImage(ui->label->pixmap()->toImage());
    }
    if(temp->isNull()){
        std::cout<<"buttons clicked too fast. caused overload"<<std::endl;
        clearPainter();
        return;
    }
    QPainter painter(temp);
    QPen pen;

    //setup the pen so it looks nice
    pen.setColor(getPenColor(color));
    pen.setStyle(getPenStyle(lineStyle));
    pen.setWidth(5);
    painter.setPen(pen);

    //do the actual drawing.
    painter.drawLine(QPointF(startX,startY + fudge),QPointF(endX,endY + fudge));

    //have the label show what is in the image.
    ui->label->setPixmap(QPixmap::fromImage(*temp));


}
/**
 * @brief takes the color string and turns it into something useful.
 * @param color
 * @return appropriate color
 */
QColor Painter::getPenColor(QString color){
    QStringList colors;
    colors << "black" << "orange" << "yellow" << "green" << "red" << "blue" << "purple";
    switch(colors.indexOf(color)){
    case 0:
        return(Qt::black);
    case 1:
        std::cout<<"apparently orange is not an option" <<std::endl;
        return (Qt::black);
    case 2:
        return(Qt::yellow);
    case 3:
        return(Qt::green);
    case 4:
        return(Qt::red);
    case 5:
        return(Qt::blue);
    case 6:
        std::cout << "apparently purple is not an option"<<std::endl;
        return(Qt::black);
    default:
        return(Qt::black);
    }

}
/**
 * @brief turns style string into something useful.
 * @param style
 * @return appropriate style
 */
Qt::PenStyle Painter::getPenStyle(QString style){
    QStringList styles;
    styles << "solid" << "dashed" << "dashed dot";
    switch(styles.indexOf(style)){
    case 0:
        return Qt::SolidLine;
    case 1:
        return Qt::DashLine;
    case 2:
        return Qt::DashDotLine;
    default:
        return Qt::SolidLine;
    }
}

void Painter::clearPainter(){
    QImage *temp2 = new QImage(this->width(),this->height(),QImage::Format_ARGB32);
    ui->label->setPixmap(QPixmap::fromImage(*temp2));
}


