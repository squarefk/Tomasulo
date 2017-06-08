#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    bool is_auto_exec = false;
    bool is_run = false;

private slots:
    void init_button_clicked();
    void step_button_clicked();
    void auto_button_clicked();
    void timer_go_out();
};

#endif // MAINWINDOW_H
