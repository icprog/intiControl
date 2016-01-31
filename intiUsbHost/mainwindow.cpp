#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_vsWhite_valueChanged(int value)
{
    ui->lblWhite->setText(QString::number(value));
}
void MainWindow::on_vsRoyalBlue_valueChanged(int value)
{
    ui->lblRoyalBlue->setText(QString::number(value));
}
void MainWindow::on_vsBlue_valueChanged(int value)
{
    ui->lblBlue->setText(QString::number(value));
}
void MainWindow::on_vsGreen_valueChanged(int value)
{
    ui->lblGreen->setText(QString::number(value));
}
void MainWindow::on_vsRed_valueChanged(int value)
{
    ui->lblRed->setText(QString::number(value));
}
void MainWindow::on_vsYellow_valueChanged(int value)
{
    ui->lblYellow->setText(QString::number(value));
}
void MainWindow::on_vsViolet_valueChanged(int value)
{
    ui->lblViolet->setText(QString::number(value));
}
