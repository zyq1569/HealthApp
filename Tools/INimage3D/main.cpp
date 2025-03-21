#include "mainwindow.h"

#include <QApplication>
#include <QFileInfo>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	if (argc > 1)
	{
		QString filepath = argv[1];
		QFileInfo finfo(filepath);
		if (!finfo.exists())
		{
			w.show();
		}
		else
		{
			w.m_commdFilePath = filepath;
			w.starViewer();
		}
	}
	else
	{
		w.show();
	}

	return a.exec();
}
