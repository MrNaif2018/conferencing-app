#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>

class MyThread : public QThread
{
    Q_OBJECT
private:
    char **argv;

public:
    MyThread(char **argv) : argv(argv) {}

protected:
    virtual void run();
signals:
    void signalGUI(QImage);
};

#endif