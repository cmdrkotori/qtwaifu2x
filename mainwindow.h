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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent* event);

private:
    void setupModelDirFallback();
    void consoleLog(QString text);
    void setRunningState(bool running);

private slots:
    void on_inputBrowse_clicked();

    void on_outputBrowse_clicked();

    void on_outputTemplate_toggled(bool checked);

    void on_scaleCustom_toggled(bool checked);

    void on_noise_toggled(bool checked);

    void on_modelBrowse_clicked();

    void on_renderStart_clicked();

    void waifu_readyRead();
    void waifu_finished(int exitCode, QProcess::ExitStatus status);

    void on_renderStop_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *waifu;
    QString modelDirFallback;
};

#endif // MAINWINDOW_H
