#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum RunningState { RunningNothing, RunningOnce, RunningAll };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent* event);

private:
    bool setExecutable(const QString &folder);
    bool setModelFolder(const QString &folder);
    void checkFolders();
    QStringList processors();
    void consoleLog(QString text);

    void setRunningState(RunningState state);

private slots:
    void on_inputBrowse_clicked();

    void on_executableBrowse_clicked();
    void on_modelBrowse_clicked();

    void on_renderStartOnce_clicked();

    void waifu_readyRead();
    void waifu_finished(int exitCode, QProcess::ExitStatus status);

    void on_renderStop_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *waifu;
    QString executable;
    QString modelFolder;
};

#endif // MAINWINDOW_H
