#ifndef CONFIG_H
#define CONFIG_H

#include <QWidget>

namespace Ui
{
class Config;
}

class Config : public QWidget
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = nullptr);
    ~Config();

signals:
    void saveConfig(QString, QString, int, int);

private slots:
    void saveServerconfig();

public:
    void setConfig(QString serverip, QString serverport, int viewer, int report);

private:
    Ui::Config *ui;
};

#endif // CONFIG_H
