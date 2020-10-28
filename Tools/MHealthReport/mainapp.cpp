#include "mainapp.h"
#include "ui_mainapp.h"

MainApp::MainApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainApp)
{
    ui->setupUi(this);
}

MainApp::~MainApp()
{
    delete ui;
}

