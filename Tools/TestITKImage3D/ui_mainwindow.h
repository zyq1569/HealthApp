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
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
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
    QPushButton *m_TestAll;
    QListView *m_renderingStyleListView;
    QLabel *m_labeltip;
    QGroupBox *groupBox;
    QPushButton *pBRemoveBed;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(873, 332);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        m_dcmDir = new QTextEdit(centralwidget);
        m_dcmDir->setObjectName(QString::fromUtf8("m_dcmDir"));
        m_dcmDir->setGeometry(QRect(10, 110, 491, 21));
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 140, 421, 25));
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

        m_TestAll = new QPushButton(layoutWidget);
        m_TestAll->setObjectName(QString::fromUtf8("m_TestAll"));

        horizontalLayout->addWidget(m_TestAll);

        m_renderingStyleListView = new QListView(centralwidget);
        m_renderingStyleListView->setObjectName(QString::fromUtf8("m_renderingStyleListView"));
        m_renderingStyleListView->setGeometry(QRect(5, 5, 853, 91));
        m_renderingStyleListView->setMaximumSize(QSize(16777215, 120));
        m_renderingStyleListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_renderingStyleListView->setIconSize(QSize(64, 64));
        m_renderingStyleListView->setMovement(QListView::Static);
        m_renderingStyleListView->setResizeMode(QListView::Adjust);
        m_renderingStyleListView->setViewMode(QListView::IconMode);
        m_renderingStyleListView->setUniformItemSizes(true);
        m_labeltip = new QLabel(centralwidget);
        m_labeltip->setObjectName(QString::fromUtf8("m_labeltip"));
        m_labeltip->setGeometry(QRect(0, 260, 841, 20));
        QFont font;
        font.setFamily(QString::fromUtf8("Fixedsys"));
        font.setPointSize(14);
        font.setBold(true);
        font.setWeight(75);
        m_labeltip->setFont(font);
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 180, 271, 80));
        pBRemoveBed = new QPushButton(groupBox);
        pBRemoveBed->setObjectName(QString::fromUtf8("pBRemoveBed"));
        pBRemoveBed->setGeometry(QRect(190, 40, 75, 23));
        MainWindow->setCentralWidget(centralwidget);
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
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">D:\\Test_DICOM\\jp2k</p></body></html>", nullptr));
        pBDir->setText(QApplication::translate("MainWindow", "SelectDir", nullptr));
        pBITK->setText(QApplication::translate("MainWindow", "2DImage", nullptr));
        pBITK3D->setText(QApplication::translate("MainWindow", "March-3D", nullptr));
        pBVolume3D->setText(QApplication::translate("MainWindow", "Volume-3D", nullptr));
        m_TestAll->setText(QApplication::translate("MainWindow", "Test All", nullptr));
        m_labeltip->setText(QApplication::translate("MainWindow", "Volume-3D:\351\274\240\346\240\207\357\274\232\344\270\255\351\224\256\347\247\273\345\212\250\345\233\276\345\203\217 \345\267\246\351\224\256\346\227\213\350\275\254\345\233\276\345\203\217  \345\217\263\351\224\256\357\274\232WW|WL\357\274\210ESC\351\224\256\350\277\230\345\216\237\357\274\211 \346\210\226\350\200\205(\344\270\255\351\224\256\345\215\225\345\207\273\345\210\207\346\215\242)\345\210\207\346\215\242 Zoom", nullptr));
        groupBox->setTitle(QApplication::translate("MainWindow", "Volume-3D", nullptr));
        pBRemoveBed->setText(QApplication::translate("MainWindow", "RemoveBed", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
