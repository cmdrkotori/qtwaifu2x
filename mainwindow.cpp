#include <QFileDialog>
#include <QFileInfo>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    waifu(NULL)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    if (waifu) {
        delete waifu;
        waifu = NULL;
    }
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

void MainWindow::on_scaleCustom_toggled(bool checked)
{
    ui->scaleValue->setEnabled(checked);
}

void MainWindow::on_noise_toggled(bool checked)
{
    ui->noiseValue->setEnabled(checked);
}

void MainWindow::on_modelBrowse_clicked()
{
    QString folderName = QFileDialog::getExistingDirectory(
                this, tr("Model Folder"));

    if (folderName.isNull())
        return;

    ui->modelFolder->setText(folderName);
}

void MainWindow::on_renderStart_clicked()
{
    if (waifu)
        return;
    else
        waifu = new QProcess(this);

    waifu->setProgram("waifu2x-converter-cpp");

    QStringList args;
    double scaleRatio = 2.0;
    int noiseLevel = 0;
    args << "--force-OpenCL";
    if (ui->scaleCustom->isChecked())
        scaleRatio = ui->scaleValue->value();
    if (ui->noise->isChecked())
        noiseLevel = ui->noiseValue->value();
    args << "--scale_ratio" << QString::number(scaleRatio, 'f', 3);
    args << "--noise_level" << QString::number(noiseLevel);
    args << "-m";
    if (noiseLevel > 0) {
        if (scaleRatio > 1.0)
            args << "noise_scale";
        else
            args << "noise";
    } else {
        args << "scale";
    }
    args << "-i" << ui->inputFile->text();
    QString outputFile;
    if (ui->outputTemplate->isChecked()
            || (outputFile = ui->outputFile->text()).isEmpty()) {
        QFileInfo qfi(ui->inputFile->text());
        outputFile = qfi.dir().absolutePath() + "/" + qfi.completeBaseName() + "_waifu2x";
        if (noiseLevel > 0)
            outputFile.append(QString("noise%1").arg(noiseLevel));
        if (scaleRatio > 1.0)
            outputFile.append(QString("scale%2").arg(QString::number(scaleRatio, 'f', 3)));
        outputFile.append(".png");
    }
    args << "-o" << outputFile;
    waifu->setArguments(args);

    connect(waifu, &QProcess::readyRead,
            this, &MainWindow::waifu_readyRead);
    connect(waifu, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(waifu_finished(int,QProcess::ExitStatus)));
}

void MainWindow::waifu_readyRead()
{
    QString data = QString::fromUtf8(waifu->readAll());
    QTextCursor qtc = ui->console->textCursor();
    qtc.movePosition(QTextCursor::End);
    qtc.insertText(data);
    ui->console->setTextCursor(qtc);
}

void MainWindow::waifu_finished(int exitCode, QProcess::ExitStatus status)
{
    if (!waifu)
        return;

    waifu->deleteLater();
    waifu = NULL;
}

void MainWindow::on_renderStop_clicked()
{
    if (!waifu)
        return;

    waifu->deleteLater();
    waifu = NULL;
}
