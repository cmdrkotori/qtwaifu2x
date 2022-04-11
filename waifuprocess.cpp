#include <QDir>
#include <QFileInfo>
#include "waifuprocess.h"

WaifuProcess::WaifuProcess(QObject *parent)
    : QObject{parent}
{

}

void WaifuProcess::start(QString inputFile, double scale, int noise, QString executable, QString modelFolder, bool forceOpenCL, int processor)
{
    if (waifu)
        return;

    emit preparing();

    QStringList args;
    double scaleRatio = scale;
    int noiseLevel = noise;
    if (forceOpenCL)
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
    args << "-i" << inputFile;
    QString outputFile;
    if (true) {
        QFileInfo qfi(inputFile);
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
    if (processor > 0)
        args << "--processor" << QString::number(processor-1);
    emit log("Program arguments:\n\t");
    emit log(args.join(" "));

    waifu = new QProcess(this);
    waifu->setProgram(executable);
    waifu->setArguments(args);
    connect(waifu, &QProcess::readyRead,
            this, &WaifuProcess::waifu_readyRead);
    connect(waifu, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(waifu_finished(int,QProcess::ExitStatus)));

    emit log("\n\nLaunching waifu2x\n\n");
    waifu->start();
    emit started();
}

void WaifuProcess::stop()
{
    waifu->deleteLater();
    waifu = nullptr;
}

bool WaifuProcess::isRunning()
{
    return waifu != nullptr;
}

void WaifuProcess::waifu_readyRead()
{
    emit log(QString::fromUtf8(waifu->readAll()));
}

void WaifuProcess::waifu_finished(int exitCode, QProcess::ExitStatus status)
{
    if (!waifu)
        return;

    emit log(QString::fromUtf8(waifu->readAllStandardError()));
    emit log(QString("\nerror code: %1\n"
                "status: %2").arg(exitCode).arg(status));

    waifu->deleteLater();
    waifu = NULL;

    emit finished();
}

