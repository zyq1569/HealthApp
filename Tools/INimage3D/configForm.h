#ifndef CONFIGFORM_H
#define CONFIGFORM_H

#include <QWidget>

class MainWindow;

namespace Ui {
class ConfigForm;
}

class ConfigForm : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigForm(QWidget *parent = nullptr);
    ~ConfigForm();

	void InitConfig();

private:
    Ui::ConfigForm *ui;
	MainWindow* m_mainwindow;
};

#endif // CONFIGFORM_H
