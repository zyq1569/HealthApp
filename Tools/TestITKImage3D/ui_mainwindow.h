/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QTextEdit *m_dcmDir;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pBDir;
    QPushButton *pBITK;
    QPushButton *pBITK3D;
    QPushButton *pBVolume3D;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1099, 211);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        m_dcmDir = new QTextEdit(centralwidget);
        m_dcmDir->setObjectName(QString::fromUtf8("m_dcmDir"));
        m_dcmDir->setGeometry(QRect(10, 20, 901, 21));
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 50, 320, 25));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pBDir = new QPushButton(layoutWidget);
        pBDir->setObjectName(QString::fromUtf8("pBDir"));

        horizontalLayout->addWidget(pBDir);

        pBITK = new QPushButton(layoutWidget);
        pBITK->setObjectName(QString::fromUtf8("pBITK"));

        horizontalLayout->addWidget(pBITK);

        pBITK3D = new QPushButton(layoutWidget);
        pBITK3D->setObjectName(QString::fromUtf8("pBITK3D"));

        horizontalLayout->addWidget(pBITK3D);

        pBVolume3D = new QPushButton(layoutWidget);
        pBVolume3D->setObjectName(QString::fromUtf8("pBVolume3D"));

        horizontalLayout->addWidget(pBVolume3D);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1099, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "TestDcmITK", nullptr));
        m_dcmDir->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">D:\\log\\1.2.156.112605.189250942931889.230211080502.2.10668.11341</p></body></html>", nullptr));
        pBDir->setText(QApplication::translate("MainWindow", "SelectDir", nullptr));
        pBITK->setText(QApplication::translate("MainWindow", "2DImage", nullptr));
        pBITK3D->setText(QApplication::translate("MainWindow", "March-3D", nullptr));
        pBVolume3D->setText(QApplication::translate("MainWindow", "Volume-3D", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
