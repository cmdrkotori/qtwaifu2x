#include <QFileDialog>
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

void MainWindow::on_inputBrowse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QString(),
                                                    "Images (*.png *.jpg)");
    if (fileName.isNull())
        return;

    ui->inputFile->setText(fileName);
}

void MainWindow::on_outputBrowse_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    QString(),
                                                    "PNG Image (*.png)");

    if (fileName.isNull())
        return;

    ui->outputFile->setText(fileName);
}

void MainWindow::on_outputTemplate_toggled(bool checked)
{
    ui->outputFile->setEnabled(!checked);
    ui->outputBrowse->setEnabled(!checked);
}
