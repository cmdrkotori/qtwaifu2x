#include <QMimeData>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
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

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
        ui->inputFile->setText(event->mimeData()->urls().first().toLocalFile());
}

void MainWindow::consoleLog(QString text)
{
    QTextCursor qtc = ui->console->textCursor();
    qtc.movePosition(QTextCursor::End);
    qtc.insertText(text);
    ui->console->setTextCursor(qtc);
}

void MainWindow::setRunningState(bool running)
{
    ui->renderStart->setEnabled(!running);
    ui->renderStop->setEnabled(running);
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

    ui->console->clear();

    waifu->setProgram("waifu2x-converter-cpp");

    QStringList args;
    double scaleRatio = 2.0;
    int noiseLevel = 0;
    if (ui->forceOpenCL->isChecked())
        args << "--force-OpenCL";
    if (ui->scaleCustom->isChecked())
        scaleRatio = ui->scaleValue->value();
    if (ui->noise->isChecked())
        noiseLevel = ui->noiseValue->value();
    args << "--scale_ratio" << QString::number(scaleRatio, 'f', 3);
    if (noiseLevel > 0)
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
            outputFile.append(QString("_noise%1").arg(noiseLevel));
        if (scaleRatio > 1.0)
            outputFile.append(QString("_scale%2").arg(QString::number(scaleRatio, 'f', 3)));
        outputFile.append(".png");
    }
    args << "-o" << outputFile;
    QString modelDir = ui->modelFolder->text();
    if (modelDir.isEmpty()) {
        modelDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.waifu2x/models";
    }
    while (modelDir.endsWith("/"))
        modelDir.chop(1);
    args << "--model_dir" << modelDir;
    waifu->setArguments(args);
    consoleLog("Program arguments:\n\t");
    consoleLog(args.join(" "));

    connect(waifu, &QProcess::readyRead,
            this, &MainWindow::waifu_readyRead);
    connect(waifu, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(waifu_finished(int,QProcess::ExitStatus)));
    consoleLog("\n\nLaunching waifu2x\n\n");
    waifu->start();
    setRunningState(true);
}

void MainWindow::waifu_readyRead()
{
    consoleLog(QString::fromUtf8(waifu->readAll()));
}

void MainWindow::waifu_finished(int exitCode, QProcess::ExitStatus status)
{
    if (!waifu)
        return;

    consoleLog(QString::fromUtf8(waifu->readAllStandardError()));
    consoleLog(QString("\nerror code: %1\n"
                "status: %2").arg(exitCode).arg(status));

    waifu->deleteLater();
    waifu = NULL;

    setRunningState(false);
}

void MainWindow::on_renderStop_clicked()
{
    if (!waifu)
        return;

    waifu->deleteLater();
    waifu = NULL;
}
