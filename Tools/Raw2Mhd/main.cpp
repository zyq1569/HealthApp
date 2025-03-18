#include "qtvtkrenderwindows.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtVTKRenderWindows w;
	if (argc == 11)
	{
		w.m_fileDir   = argv[1];
		w.m_width     = QString(argv[2]).toInt();
		w.m_hight     = QString(argv[3]).toInt();
		w.m_start     = QString(argv[4]).toInt();
		w.m_end       = QString(argv[5]).toInt();
		w.m_filesName = argv[6];
		QString temp  = argv[7];
		if (temp.toUpper() != "NULL")
		{
			w.m_extraName = argv[7];
		}
		else
		{
			w.m_extraName = "";
		}
		temp = argv[8];
		if (temp.toUpper() != "NULL")
		{
			w.m_numberStyle = argv[8];
		}
		else
		{
			w.m_numberStyle = "";
		}
		temp = argv[9];
		if (temp.toUpper() != "NULL")
		{
			w.m_saveFileName = argv[9];
		}
		else
		{
			w.m_saveFileName = "VTK_Data";
		}
		w.m_bzip = QString(argv[10]).toInt();
		
		w.m_argvstr += argv[0];
		for (int i = 1; i < 11; i++)
		{
			w.m_argvstr += " ";
			w.m_argvstr += argv[i];
		}
		w.raw2vtkmhd();
	}
	else
	{
		w.show();
		return a.exec();
	}

	return 1;
}
