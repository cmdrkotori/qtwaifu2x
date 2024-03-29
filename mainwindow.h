#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include "waifuprocess.h"

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

    void setRunningState(RunningState state);
    bool renderNext();

private slots:
    void consoleLog(QString text);

    void on_inputBrowse_clicked();
    void on_inputFolders_clicked();
    void on_inputList_clicked();

    void on_executableBrowse_clicked();
    void on_modelBrowse_clicked();

    void on_renderStartOnce_clicked();
    void on_renderStartAll_clicked();

    void waifu_preparing();
    void waifu_finished();

    void on_renderStop_clicked();

    void on_filesExport_clicked();
    void on_filesRemove_clicked();
    void on_filesClear_clicked();

    void on_checkBox_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    WaifuProcess *waifu;
    QString executable;
    QString modelFolder;
    RunningState running = RunningNothing;
};

#endif // MAINWINDOW_H
