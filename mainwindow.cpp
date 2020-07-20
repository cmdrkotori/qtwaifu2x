#include <QMimeData>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    waifu(NULL)
{
    ui->setupUi(this);
    connect(ui->modelFolder, &QLineEdit::textEdited,
            this, &MainWindow::checkFolders);
    connect(ui->executable, &QLineEdit::textEdited,
            this, &MainWindow::checkFolders);

    checkFolders();
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

bool MainWindow::setExecutable(const QString &folder)
{
    QStringList candidates;
    if (!folder.isEmpty())
        candidates << folder + "/waifu2x-converter-cpp";
    else
        candidates << "./waifu2x-converter-cpp"
                   << "/usr/local/bin/waifu2x-converter-cpp"
                   << QStandardPaths::findExecutable("waifu2x-converter-cpp");

    for (const QString &binary : candidates) {
        executable = binary;
        if (QFileInfo(executable).isExecutable())
            return true;
    }
    executable.clear();
    return false;
}

bool MainWindow::setModelFolder(const QString &folder)
{
    QStringList candidates;
    if (!folder.isEmpty()) {
        candidates << folder;
    } else {
        candidates << QFileInfo(executable).dir().path() + "/models_rgb"
                   << QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.waifu2x/models_rgb"
                   << QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.waifu2x"
                   << "/usr/local/share/waifu2x-converter-cpp"
                   << "/usr/share/waifu2x-converter-cpp";
    }

    for (const QString &baseFolder : candidates) {
        modelFolder = baseFolder;
        if (!modelFolder.isEmpty() && QDir(modelFolder).exists())
            return true;
    }
    modelFolder.clear();
    return false;
}

void MainWindow::checkFolders()
{
    bool exec = setExecutable(ui->executable->text());
    bool models = setModelFolder(ui->modelFolder->text());
    static const char badStyle[] = "background: #ba8a8a; color: black;";
    ui->executable->setStyleSheet(exec ? "" : badStyle);
    ui->modelFolder->setStyleSheet(models ? "" : badStyle);
    QStringList processors;
    if (exec && models)
        processors << "Autodetect" << this->processors();
    else
        processors << "Executable or OpenCL not found";
    int oldIndex = ui->processor->currentIndex();
    ui->processor->clear();
    ui->processor->addItems(processors);
    ui->processor->setCurrentIndex(oldIndex);
}

QStringList MainWindow::processors()
{
    QProcess p;
    p.setProgram(executable);
    p.setArguments({"--list-processor"});
    p.start();
    p.waitForFinished();
    QString processOutput = QString::fromLocal8Bit(p.readAll());
    QStringList lines = processOutput.split('\n', QString::SkipEmptyParts);
    QStringList out;
    for (QString &line : lines) {
        line = line.trimmed();
        if (line.count() && line.at(0).isNumber())
            out << line;
    }
    return out;
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

void MainWindow::on_executableBrowse_clicked()
{
    QString folderName = QFileDialog::getExistingDirectory(
                this, tr("Executable Folder"));

    if (folderName.isNull())
        return;

    ui->executable->setText(folderName);
    checkFolders();
}

void MainWindow::on_modelBrowse_clicked()
{
    QString folderName = QFileDialog::getExistingDirectory(
                this, tr("Model Folder"));

    if (folderName.isNull())
        return;

    ui->modelFolder->setText(folderName);
    checkFolders();
}

void MainWindow::on_renderStart_clicked()
{
    if (waifu)
        return;
    else
        waifu = new QProcess(this);

    ui->console->clear();

    waifu->setProgram(executable);

    QStringList args;
    double scaleRatio = ui->scaleValue->value();;
    int noiseLevel = ui->noiseValue->value();
    if (ui->forceOpenCL->isChecked())
        args << "--force-OpenCL";
    args << "--scale-ratio" << QString::number(scaleRatio, 'f', 3);
    if (noiseLevel > 0)
        args << "--noise-level" << QString::number(noiseLevel);
    args << "-m";
    if (noiseLevel > 0) {
        if (scaleRatio > 1.0)
            args << "noise-scale";
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
        QString suffix = "_waifu2x";
        if (noiseLevel > 0)
            suffix.append(QString("_noise%1").arg(noiseLevel));
        if (scaleRatio > 1.0)
            suffix.append(QString("_scale%2").arg(QString::number(scaleRatio, 'f', 3)));
        suffix.append(".png");
        QString trimmedFileName = qfi.completeBaseName().left(255-suffix.length());
        outputFile = qfi.dir().absolutePath() + "/" + trimmedFileName + suffix;
    }
    args << "-o" << outputFile;
    args << "--model-dir" << modelFolder;
    if (ui->processor->currentIndex() > 0)
        args << "--processor" << QString::number(ui->processor->currentIndex()-1);
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
