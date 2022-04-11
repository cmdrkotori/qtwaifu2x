#ifndef WAIFUPROCESS_H
#define WAIFUPROCESS_H

#include <QObject>
#include <QProcess>

class WaifuProcess : public QObject
{
    Q_OBJECT
public:
    explicit WaifuProcess(QObject *parent = nullptr);
    ~WaifuProcess();
    void start(QString inputFile, double scale, int noise, QString executable, QString modelFolder, bool forceOpenCL, int processor);
    void stop();
    bool isRunning();

signals:
    void preparing();
    void started();
    void finished();
    void log(QString s);

private slots:
    void waifu_readyRead();
    void waifu_finished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess *waifu = nullptr;
};

#endif // WAIFUPROCESS_H
