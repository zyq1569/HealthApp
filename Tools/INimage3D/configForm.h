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

	void InitConfig(MainWindow* mainwindow);
private:
    Ui::ConfigForm *ui;
};

#endif // CONFIGFORM_H
