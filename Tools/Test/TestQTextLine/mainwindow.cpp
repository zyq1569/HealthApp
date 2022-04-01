#include <QTextEdit>
#include <QTextBlock>
#include <QPushButton>
#include <QTextList>
#include <QPainter>
#include <math.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
//    : QMainWindow(parent)
    :ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ///
    ///
    pt=new QTextEdit(this);
    QTextCursor cur= pt->textCursor(); //获取光标(插入符)
    cur.movePosition(QTextCursor::Start); //设置光标位置
    QTextListFormat f;
    f.setStyle(QTextListFormat::ListDecimal);
    //设置列表编号样式
    f.setNumberPrefix("<");
    f.setNumberSuffix(">");
    //设置列表项目编号前/后缀
    px1=cur.insertList(f);
    //创建列表 px1
    QTextListFormat f1;
    f1.setStyle(QTextListFormat::ListDecimal);
    px2=cur.insertList(f1);
    //创建列表 px2
    //把文本块添加到列表 px1 中
    QTextBlockFormat bf; //设置块格式
    bf.setBackground(QColor(111,1,1));
    cur.insertBlock(bf); //创建块
    QTextBlock b1; //注意:这样直接创建的块不能被直接添加到列表中(因为该块未插入文档)。
    b1=cur.block(); //获取文档中上面创建的块,并赋值给 b1
    px1->add(b1); //把 b1 添加到列表中。
    //显示根边框
    QTextDocument *pd=pt->document();
    QTextFrame *pf=pd->rootFrame(); //获取根框架
    QTextFrameFormat ff;
    ff.setBorder(4);
    ff.setBorderBrush(QColor(128,128,128)); //设置框架边框宽度及颜色
    ff.setBorderStyle(QTextFrameFormat::BorderStyle_Solid); //框架边框线设置为实现
    pf->setFrameFormat(ff); //设置边框格式
    pt->resize(600,500);
    //设置按钮及快捷键,读者还可设置更多的快捷键,以更方便的处理文本。
    pb1=new QPushButton("Blod",this);
    pb1->move(22,244);
    pb2=new QPushButton("FontSize22",this);
    pb2->move(99,244);
    pb1->setShortcut(QKeySequence("Ctrl+F")); //设置 pb1 的快捷键
    pb2->setShortcut(QKeySequence("Ctrl+D"));
    //QObject::connect(pb1,&QPushButton::clicked,this,&B::f1);
    //QObject::connect(pb2,&QPushButton::clicked,this,&B::f2);

    ////--------------------------------------------------------------------------------------------------------------------------------------------------------
    QString text = "qweerewrwetwtwrtwgrgwgwrgsdfsegrywrreyeryeryeryreyeryer!@@##$$%%^&*()_&^%$#@!@#$%^&*(*&@@#$%^&*()(^%$#$%^&*()(*%$#^&*()(*&^%$%^&*()(*&^%$!";
    QFont font("宋体",20);
    QTextLayout textLayout(text, font);
    qreal margin = 10;
    qreal radius = qMin(width()/2.0, height()/2.0) - margin;
    QFontMetrics fm(font);
    qreal lineHeight = fm.height();
    qreal y = 0;
    textLayout.beginLayout();
    while (1)
    {
        // create a new line
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
        {
            break;
        }
        qreal x1 = qMax(0.0, pow(pow(radius,2)-pow(radius-y,2), 0.5));
        qreal x2 = qMax(0.0, pow(pow(radius,2)-pow(radius-(y+lineHeight),2), 0.5));
        qreal x = qMax(x1, x2) + margin;
        qreal lineWidth = (width() - margin) - x;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(x, margin+y));
        y += line.height();
    }
    textLayout.endLayout();
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.setBrush(QBrush(Qt::black));
    painter.setPen(QPen(Qt::black));
    textLayout.draw(&painter, QPoint(0,0));
    painter.setBrush(QBrush(QColor("#a6ce39")));
    painter.setPen(QPen(Qt::black));
    painter.drawEllipse(QRectF(-radius, margin, 2*radius, 2*radius));
    painter.end();
    //////////////////////////////////////////////////////////////////////////////------------------------------------------------------------------

}

MainWindow::~MainWindow()
{
    delete ui;
}

