#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "core.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->init_button, SIGNAL(clicked(bool)), this, SLOT(init_button_clicked()));
    connect(ui->step_button, SIGNAL(clicked(bool)), this, SLOT(step_button_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void process_ins(QString str) {
    ins_num = 0;
    QStringList strList = str.split('\n');
    for (int i = 0; i < strList.size(); ++i) {
        QString command = strList.at(i);
        QStringList parms = command.split(' ');
        if (parms.size() != 3 && parms.size() != 4) continue;

        if (parms.at(0) == QString("ADDD")) ins[++ins_num].op = ADDD;
        else if (parms.at(0) == QString("SUBD")) ins[++ins_num].op = SUBD;
            else if (parms.at(0) == QString("MULD")) ins[++ins_num].op = MULD;
                else if (parms.at(0) == QString("DIVD")) ins[++ins_num].op = DIVD;
                    else if (parms.at(0) == QString("LD")) ins[++ins_num].op = LD;
                        else if (parms.at(0) == QString("ST")) ins[++ins_num].op = ST;
                            else continue;
        if (ins[ins_num].op == LD || ins[ins_num].op == ST) {
            ins[ins_num].i = parms.at(1).mid(1).toInt();
            ins[ins_num].j = parms.at(2).toInt();
            ins[ins_num].k = 0;
        } else {
            ins[ins_num].i = parms.at(1).mid(1).toInt();
            ins[ins_num].j = parms.at(2).mid(1).toInt();
            ins[ins_num].k = parms.at(3).mid(1).toInt();
        }
    }
    fin_num = 0;
    next_start_num = 1;
    time_cnt = 0;
}

void process_mem(QString str) {
    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));
    QStringList strList = str.split('\n');
    for (int i = 0; i < strList.size(); ++i) {
        QString command = strList.at(i);
        QStringList parms = command.split(' ');
        if (parms.size() != 2) continue;
        mem[parms.at(0).toInt()] = parms.at(1).toFloat();
    }
}

void process_stack() {
    for (int i = 0; i < add_stack_size; ++i)
        add_stack[i].first = false;
    for (int i = 0; i < mult_stack_size; ++i)
        mult_stack[i].first = false;
    for (int i = 0; i < load_stack_size; ++i)
        load_stack[i].first = false;
    for (int i = 0; i < store_stack_size; ++i)
        store_stack[i].first = false;
}

QString show_symbol(int ins_num) {
    return ins_num ? ins[ins_num].symbol : QString("null") + " ";
}

QString show_time(int time) {
    if (time && time <= time_cnt)
        return QString::number(time);
    return QString("");
}

QString generate_result_textedit(){
    QString result = "";

    result += "Running state:\n";
    for (int i = 1; i <= ins_num; ++i)
        result += show_time(run_state[i][0]) + " " + show_time(run_state[i][1]) + " " + show_time(run_state[i][2]) + "\n";
    result += "\n";

    return result;
}

QString generate_load_textedit(){
    QString result = "";

    result += "Load Queue:\n";
    for (int i = 0; i < load_stack_size; ++i) {
        result += QString::number(load_stack[i].first) + " ";
        if (!load_stack[i].first) {result += "\n";continue;}
        result += QString::number(load_stack[i].second.i) + " ";
        result += QString::number(load_stack[i].second.j);
        result += "(" + QString::number(load_stack[i].second.j_) + ") ";
        result += show_symbol(load_stack[i].second.j_);
        result += "\n";
    }
    result += "\n";

    return result;
}

QString generate_store_textedit(){
    QString result = "";

    result += "Store Queue:\n";
    for (int i = 0; i < store_stack_size; ++i) {
        result += QString::number(store_stack[i].first) + " ";
        if (!store_stack[i].first) {result += "\n";continue;}
        result += QString::number(store_stack[i].second.i);
        result += "(" + QString::number(store_stack[i].second.i_) + ") ";
        result += QString::number(store_stack[i].second.j);
        result += show_symbol(store_stack[i].second.i_);
        result += "\n";
    }
    result += "\n";

    return result;
}

QString generate_memory_textedit(){
    QString result = "";

    for(int i = 0;i < 1000;i++){
        if(mem[i] > 1e-6 || mem[i] < -1e-6){

            result += QString::number(i) + " ";
            result += QString::number(mem[i]);
            result += "\n";
        }
    }

    return result;
}

QString generate_reser_textedit(){
    QString result = "";

    result += "Reservation:\n";
    for (int i = 0; i < add_stack_size; ++i) {
        result += QString::number(add_stack[i].first) + " ";
        if (!add_stack[i].first) {result += "\n";continue;}
        result += QString::number(add_stack[i].second.op) + " ";
        result += QString::number(add_stack[i].second.i) + " ";
        result += QString::number(add_stack[i].second.j);
        result += "(" + QString::number(add_stack[i].second.j_) + ") ";
        result += QString::number(add_stack[i].second.k);
        result += "(" + QString::number(add_stack[i].second.k_) + ") ";
        result += show_symbol(add_stack[i].second.j_);
        result += " ";
        result += show_symbol(add_stack[i].second.k_);
        result += "\n";
    }
    for (int i = 0; i < mult_stack_size; ++i) {
        result += QString::number(mult_stack[i].first) + " ";
        if (!mult_stack[i].first) {result += "\n";continue;}
        result += QString::number(mult_stack[i].second.op) + " ";
        result += QString::number(mult_stack[i].second.i) + " ";
        result += QString::number(mult_stack[i].second.j);
        result += "(" + QString::number(mult_stack[i].second.j_) + ") ";
        result += QString::number(mult_stack[i].second.k);
        result += "(" + QString::number(mult_stack[i].second.k_) + ") ";
        result += show_symbol(mult_stack[i].second.j_);
        result += " ";
        result += show_symbol(mult_stack[i].second.k_);
        result += "\n";
    }
    result += "\n";

    return result;
}

QString generate_regis_textedit(){
    QString result = "";

    result += "Registers:\n";
    for (int i = 0; i <= 10; ++i)
        result += QString::number(i) + " " + QString::number(reg[i]) + "\n";
    result += "\n";

    return result;
}
/*
QString generate_result() {
    QString result = "";

    result += "Running state:\n";
    for (int i = 1; i <= ins_num; ++i)
        result += show_time(run_state[i][0]) + " " + show_time(run_state[i][1]) + " " + show_time(run_state[i][2]) + "\n";
    result += "\n";

    result += "Load Queue:\n";
    for (int i = 0; i < load_stack_size; ++i) {
        result += QString::number(load_stack[i].first) + " ";
        if (!load_stack[i].first) {result += "\n";continue;}
        result += QString::number(load_stack[i].second.i) + " ";
        result += QString::number(load_stack[i].second.j);
        result += "(" + QString::number(load_stack[i].second.j_) + ") ";
        result += show_symbol(load_stack[i].second.j_);
        result += "\n";
    }
    result += "\n";

    result += "Store Queue:\n";
    for (int i = 0; i < store_stack_size; ++i) {
        result += QString::number(store_stack[i].first) + " ";
        if (!store_stack[i].first) {result += "\n";continue;}
        result += QString::number(store_stack[i].second.i);
        result += "(" + QString::number(store_stack[i].second.i_) + ") ";
        result += QString::number(store_stack[i].second.j);
        result += show_symbol(store_stack[i].second.i_);
        result += "\n";
    }
    result += "\n";

    result += "Reservation:\n";
    for (int i = 0; i < add_stack_size; ++i) {
        result += QString::number(add_stack[i].first) + " ";
        if (!add_stack[i].first) {result += "\n";continue;}
        result += QString::number(add_stack[i].second.op) + " ";
        result += QString::number(add_stack[i].second.i) + " ";
        result += QString::number(add_stack[i].second.j);
        result += "(" + QString::number(add_stack[i].second.j_) + ") ";
        result += QString::number(add_stack[i].second.k);
        result += "(" + QString::number(add_stack[i].second.k_) + ") ";
        result += show_symbol(add_stack[i].second.j_);
        result += show_symbol(add_stack[i].second.k_);
        result += "\n";
    }
    for (int i = 0; i < mult_stack_size; ++i) {
        result += QString::number(mult_stack[i].first) + " ";
        if (!mult_stack[i].first) {result += "\n";continue;}
        result += QString::number(mult_stack[i].second.op) + " ";
        result += QString::number(mult_stack[i].second.i) + " ";
        result += QString::number(mult_stack[i].second.j);
        result += "(" + QString::number(mult_stack[i].second.j_) + ") ";
        result += QString::number(mult_stack[i].second.k);
        result += "(" + QString::number(mult_stack[i].second.k_) + ") ";
        result += show_symbol(mult_stack[i].second.j_);
        result += show_symbol(mult_stack[i].second.k_);
        result += "\n";
    }
    result += "\n";

    result += "Registers:\n";
    for (int i = 0; i <= 10; ++i)
        result += QString::number(i) + " " + QString::number(reg[i]) + "\n";
    result += "\n";
    return result;
}
*/
void MainWindow::init_button_clicked() {
    ui->cycle_label->setText("0");
    ui->cycle_label->setStyleSheet("QLabel { color : black; }");
    /*ui->result_textedit->setText("");
    ui->load_textedit->setText("");
    ui->store_textedit->setText("");
    ui->reser_textedit->setText("");
    ui->regis_textedit->setText("");*/
    process_ins(ui->ins_textedit->toPlainText());
    process_mem(ui->mem_textedit->toPlainText());
    process_stack();
}

void MainWindow::step_button_clicked() {
    if (fin_num < ins_num) {
        ++time_cnt;
        push_next_ins();
        time_step();
        ui->cycle_label->setText(QString::number(time_cnt));
        ui->cycle_label->setStyleSheet("QLabel { color : black; }");
        ui->result_textedit->setText(generate_result_textedit());
        ui->load_textedit->setText(generate_load_textedit());
        ui->store_textedit->setText(generate_store_textedit());
        ui->reser_textedit->setText(generate_reser_textedit());
        ui->regis_textedit->setText(generate_regis_textedit());
        ui->memory_textedit->setText(generate_memory_textedit());
    } else {
        ui->cycle_label->setText(QString("已完成"));
        ui->cycle_label->setStyleSheet("QLabel { color : green; }");
        ui->result_textedit->setText(QString("总周期数：") + QString::number(time_cnt));
    }
}
