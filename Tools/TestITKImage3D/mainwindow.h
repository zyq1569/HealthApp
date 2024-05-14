#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//https://blog.csdn.net/l1783111653/article/details/108457374
///
///
///不畏人知畏己知
#pragma once

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class vtkActor;
class QStandardItemModel;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
	void initImage3D_ITK_VTK(vtkActor *vtkactor);
	void showImage3D_ITK_VTK();
    void loadRenderingStyles();

private:
    /// Model that saves rendering styles.
    QStandardItemModel *m_renderingStyleModel;

private slots:
    void on_pBITK_clicked();

    void on_pBDir_clicked();

    void on_pBITK3D_clicked();

    void on_pBVolume3D_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H



