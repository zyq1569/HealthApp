#include "mainwindow.h"

#include <Windows.h>
#include <QApplication>
#include <QFileInfo>

RECT GetMonitorRectBounds(int i)
{
    DISPLAY_DEVICEW d;
    d.cb = sizeof(d);
    bool flag = EnumDisplayDevicesW(nullptr, i, &d, 0);
    DEVMODEW dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    flag = EnumDisplaySettingsExW(d.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0);
    RECT rect = { dm.dmPosition.x,dm.dmPosition.y, dm.dmPosition.x + dm.dmPelsWidth,dm.dmPosition.y + dm.dmPelsHeight };
    return rect;
}

int main(int argc, char *argv[])
{
    int monitorCount = ::GetSystemMetrics(SM_CMONITORS);
    QString scaleFactor;
    for (int i = 0; i < monitorCount; i++)
    {
        RECT dims = GetMonitorRectBounds(i);
        int h = dims.bottom - dims.top;
        double scale = (h > 1080) ? ((double)h / 1080.0) : 1.0;
        scaleFactor += (i == 0 ? "" : ";") + QString::number(scale, 'f', 1);
    }

    QByteArray env = scaleFactor.toUtf8();
    qputenv("QT_SCREEN_SCALE_FACTORS", env);

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

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
