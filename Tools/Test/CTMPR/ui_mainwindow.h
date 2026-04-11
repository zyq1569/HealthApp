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
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QVTKRenderWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QSplitter *m_SplitterLR;
    QSplitter *m_horizontalSplitter;
    QSplitter *m_vSplitterL;
    QWidget *layout4;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *_2;
    QVTKRenderWidget *m_axial2DView;
    QWidget *layout9;
    QVBoxLayout *_3;
    QVTKRenderWidget *m_sagital2DView;
    QSplitter *m_vSplitterR;
    QWidget *layout41;
    QVBoxLayout *_4;
    QHBoxLayout *_5;
    QVTKRenderWidget *m_coronal2DView;
    QWidget *layout91;
    QVBoxLayout *_6;
    QVTKRenderWidget *m_image3DView;
    QPushButton *m_selectPB;
    QPushButton *m_rawPB;
    QLabel *m_Message;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1032, 708);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        m_SplitterLR = new QSplitter(centralwidget);
        m_SplitterLR->setObjectName(QString::fromUtf8("m_SplitterLR"));
        m_SplitterLR->setGeometry(QRect(10, 60, 1011, 621));
        m_SplitterLR->setStyleSheet(QString::fromUtf8("QSplitter {\n"
"    background-color: #f0f0f0;\n"
"    border: 0px solid #d0d0d0;\n"
"}\n"
"\n"
"QSplitter::handle {\n"
"    width: 0px;\n"
"    background-color: #e0e0e0;\n"
"    border: 0px solid #c0c0c0;\n"
"}\n"
""));
        m_SplitterLR->setOrientation(Qt::Horizontal);
        m_horizontalSplitter = new QSplitter(m_SplitterLR);
        m_horizontalSplitter->setObjectName(QString::fromUtf8("m_horizontalSplitter"));
        m_horizontalSplitter->setLayoutDirection(Qt::LeftToRight);
        m_horizontalSplitter->setStyleSheet(QString::fromUtf8("QSplitter {\n"
"    background-color: #f0f0f0;\n"
"    border: 0px solid #d0d0d0;\n"
"}\n"
"\n"
"QSplitter::handle {\n"
"    width: 0px;\n"
"    background-color: #e0e0e0;\n"
"    border: 0px solid #c0c0c0;\n"
"}\n"
""));
        m_horizontalSplitter->setOrientation(Qt::Horizontal);
        m_horizontalSplitter->setHandleWidth(1);
        m_vSplitterL = new QSplitter(m_horizontalSplitter);
        m_vSplitterL->setObjectName(QString::fromUtf8("m_vSplitterL"));
        m_vSplitterL->setStyleSheet(QString::fromUtf8("QSplitter {\n"
"    background-color: #f0f0f0;\n"
"    border: 0px solid #d0d0d0;\n"
"}\n"
"\n"
"QSplitter::handle {\n"
"    width: 0px;\n"
"    background-color: #e0e0e0;\n"
"    border: 0px solid #c0c0c0;\n"
"}\n"
""));
        m_vSplitterL->setOrientation(Qt::Vertical);
        layout4 = new QWidget(m_vSplitterL);
        layout4->setObjectName(QString::fromUtf8("layout4"));
        vboxLayout = new QVBoxLayout(layout4);
        vboxLayout->setSpacing(0);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        _2 = new QHBoxLayout();
        _2->setSpacing(0);
        _2->setObjectName(QString::fromUtf8("_2"));
        _2->setContentsMargins(0, 0, 0, 0);

        vboxLayout->addLayout(_2);

        m_axial2DView = new QVTKRenderWidget(layout4);
        m_axial2DView->setObjectName(QString::fromUtf8("m_axial2DView"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_axial2DView->sizePolicy().hasHeightForWidth());
        m_axial2DView->setSizePolicy(sizePolicy);

        vboxLayout->addWidget(m_axial2DView);

        m_vSplitterL->addWidget(layout4);
        layout9 = new QWidget(m_vSplitterL);
        layout9->setObjectName(QString::fromUtf8("layout9"));
        _3 = new QVBoxLayout(layout9);
        _3->setSpacing(0);
        _3->setObjectName(QString::fromUtf8("_3"));
        _3->setContentsMargins(0, 0, 0, 0);
        m_sagital2DView = new QVTKRenderWidget(layout9);
        m_sagital2DView->setObjectName(QString::fromUtf8("m_sagital2DView"));
        sizePolicy.setHeightForWidth(m_sagital2DView->sizePolicy().hasHeightForWidth());
        m_sagital2DView->setSizePolicy(sizePolicy);

        _3->addWidget(m_sagital2DView);

        m_vSplitterL->addWidget(layout9);
        m_horizontalSplitter->addWidget(m_vSplitterL);
        m_vSplitterR = new QSplitter(m_horizontalSplitter);
        m_vSplitterR->setObjectName(QString::fromUtf8("m_vSplitterR"));
        m_vSplitterR->setStyleSheet(QString::fromUtf8("QSplitter {\n"
"    background-color: #f0f0f0;\n"
"    border: 0px solid #d0d0d0;\n"
"}\n"
"\n"
"QSplitter::handle {\n"
"    width: 0px;\n"
"    background-color: #e0e0e0;\n"
"    border: 0px solid #c0c0c0;\n"
"}\n"
""));
        m_vSplitterR->setOrientation(Qt::Vertical);
        m_vSplitterR->setChildrenCollapsible(false);
        layout41 = new QWidget(m_vSplitterR);
        layout41->setObjectName(QString::fromUtf8("layout41"));
        _4 = new QVBoxLayout(layout41);
        _4->setSpacing(0);
        _4->setObjectName(QString::fromUtf8("_4"));
        _4->setContentsMargins(0, 0, 0, 0);
        _5 = new QHBoxLayout();
        _5->setSpacing(6);
        _5->setObjectName(QString::fromUtf8("_5"));
        _5->setContentsMargins(0, 0, 0, 0);

        _4->addLayout(_5);

        m_coronal2DView = new QVTKRenderWidget(layout41);
        m_coronal2DView->setObjectName(QString::fromUtf8("m_coronal2DView"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_coronal2DView->sizePolicy().hasHeightForWidth());
        m_coronal2DView->setSizePolicy(sizePolicy1);

        _4->addWidget(m_coronal2DView);

        m_vSplitterR->addWidget(layout41);
        layout91 = new QWidget(m_vSplitterR);
        layout91->setObjectName(QString::fromUtf8("layout91"));
        _6 = new QVBoxLayout(layout91);
        _6->setSpacing(0);
        _6->setObjectName(QString::fromUtf8("_6"));
        _6->setContentsMargins(0, 0, 0, 0);
        m_image3DView = new QVTKRenderWidget(layout91);
        m_image3DView->setObjectName(QString::fromUtf8("m_image3DView"));
        sizePolicy.setHeightForWidth(m_image3DView->sizePolicy().hasHeightForWidth());
        m_image3DView->setSizePolicy(sizePolicy);
        m_image3DView->setMinimumSize(QSize(0, 0));

        _6->addWidget(m_image3DView);

        m_vSplitterR->addWidget(layout91);
        m_horizontalSplitter->addWidget(m_vSplitterR);
        m_SplitterLR->addWidget(m_horizontalSplitter);
        m_selectPB = new QPushButton(centralwidget);
        m_selectPB->setObjectName(QString::fromUtf8("m_selectPB"));
        m_selectPB->setGeometry(QRect(10, 10, 75, 23));
        m_rawPB = new QPushButton(centralwidget);
        m_rawPB->setObjectName(QString::fromUtf8("m_rawPB"));
        m_rawPB->setGeometry(QRect(100, 10, 75, 23));
        m_Message = new QLabel(centralwidget);
        m_Message->setObjectName(QString::fromUtf8("m_Message"));
        m_Message->setGeometry(QRect(210, 17, 101, 16));
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        m_selectPB->setText(QApplication::translate("MainWindow", "Select...", nullptr));
        m_rawPB->setText(QApplication::translate("MainWindow", "Raw->", nullptr));
        m_Message->setText(QApplication::translate("MainWindow", "\346\225\260\346\215\256\345\276\205\350\275\254", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
