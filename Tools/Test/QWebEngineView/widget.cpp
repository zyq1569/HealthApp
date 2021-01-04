#include "widget.h"
#include "ui_widget.h"
#include <QUrl>
#include <QWebEngineHistory>
#include <QWebEnginePage>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->view2->hide();
    connect(ui->view, &QWebEngineView::loadFinished, this, &Widget::slotLoadFinished);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &Widget::on_buttonBrowse_clicked);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_buttonBrowse_clicked()
{
    QString str = ui->lineEdit->text();
    QUrl url = QUrl(str);
    ui->view->load(url);
}

void Widget::on_buttonPre_clicked()
{
    QWebEngineHistory* history = ui->view->history();
    history->back();
}

void Widget::on_buttonNext_clicked()
{
    QWebEngineHistory* history = ui->view->history();
    history->forward();
}

void Widget::on_buttonInspect_clicked()
{
    QWebEnginePage* page = ui->view->page();
    ui->view2->page()->setInspectedPage(page);
    if(ui->view2->isHidden())
    {
        ui->view2->show();
    }
    else
    {
        ui->view2->hide();
    }

}

void Widget::slotLoadFinished()
{
    setWindowTitle(ui->view->title());
    setWindowIcon(ui->view->icon());
}


void Widget::on_buttonZoom_clicked()
{
    qreal factor = ui->view->zoomFactor();
    factor += 0.1;
    if(factor > 5)
    {
        factor = 5;
    }
    ui->view->setZoomFactor(factor);
}

void Widget::on_buttonDefault_clicked()
{
    ui->view->setZoomFactor(1.0);
}
