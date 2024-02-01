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

    waifu = new WaifuProcess(this);
    connect(waifu, &WaifuProcess::log,
            this, &MainWindow::consoleLog);
    connect(waifu, &WaifuProcess::preparing,
            this, &MainWindow::waifu_preparing);
    connect(waifu, &WaifuProcess::finished,
            this, &MainWindow::waifu_finished);

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
    if (event->mimeData()->hasUrls()) {
        for (const auto &url : event->mimeData()->urls())
            ui->filesList->addItem(url.toLocalFile());
    }
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
    QStringList lines = processOutput.split('\n', Qt::SkipEmptyParts);
    QStringList out;
    for (QString &line : lines) {
        line = line.trimmed();
        if (line.length() && line.at(0).isNumber())
            out << line;
    }
    return out;
}


void MainWindow::setRunningState(RunningState state)
{
    running = state;
    ui->renderStartOnce->setEnabled(state == RunningNothing);
    ui->renderStartAll->setEnabled(state == RunningNothing);
    ui->renderStop->setEnabled(state != RunningNothing);
}

bool MainWindow::renderNext() {
    if (waifu->isRunning())
        return false;

    if (ui->filesList->count() < 1) {
        setRunningState(RunningNothing);
        return false;
    }

    if (executable.isEmpty())
        return false;

    if (modelFolder.isEmpty())
        return false;

    QString inputFile;
    inputFile = ui->filesList->item(0)->text();
    delete ui->filesList->item(0);

    return waifu->start(inputFile, ui->scaleValue->value(),
                        ui->noiseValue->value(), executable, modelFolder,
                        ui->forceOpenCL->isChecked(),
                        ui->processor->currentIndex());
}

void MainWindow::consoleLog(QString text)
{
    QTextCursor qtc = ui->console->textCursor();
    qtc.movePosition(QTextCursor::End);
    qtc.insertText(text);
    ui->console->setTextCursor(qtc);
}

void MainWindow::on_inputBrowse_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Open File"), QString(), "Images (*.png *.jpg)");
    if (fileNames.isEmpty())
        return;

    for (const auto &fileName : fileNames)
        ui->filesList->addItem(fileName);
}

void MainWindow::on_inputFolders_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Folder"),
                                                        QString());

    if (dirName.isNull())
        return;

    QDir d(dirName);
    d.setNameFilters({"*.jpg", "*.jpeg", "*.png"});
    for (const auto &fileInfo : d.entryInfoList())
        ui->filesList->addItem(fileInfo.absoluteFilePath());
}


void MainWindow::on_inputList_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (fileName.isNull())
        return;

    QFile f(fileName);
    if (!f.open(QFile::ReadOnly | QFile::Text))
        return;
    QTextStream s(&f);
    ui->filesList->addItems(s.readAll().split('\n', Qt::SkipEmptyParts));

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

void MainWindow::on_renderStartOnce_clicked()
{
    if (renderNext())
        setRunningState(RunningOnce);
}
void MainWindow::on_renderStartAll_clicked()
{
    if (renderNext())
        setRunningState(RunningAll);
}

void MainWindow::waifu_preparing()
{
    ui->console->clear();
}

void MainWindow::waifu_finished()
{
    if (running == RunningAll)
        on_renderStartAll_clicked();
    else
        setRunningState(RunningNothing);
}

void MainWindow::on_renderStop_clicked()
{
    waifu->stop();
    // rely on waifu_finished to set next running state
}


void MainWindow::on_filesExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File List"));
    if (fileName.isNull())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QStringList list;
    for (int i = 0; i < ui->filesList->count(); ++i) {
        list.append(ui->filesList->item(i)->text());
    }
    file.write(list.join('\n').toUtf8());
}

void MainWindow::on_filesRemove_clicked()
{
    if (ui->filesList->currentRow() >= 0)
        delete ui->filesList->currentItem();
}

void MainWindow::on_filesClear_clicked()
{
    ui->filesList->clear();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    waifu->setIgnoreExisting(checked);
}
