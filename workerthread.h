#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>

class MyThread : public QThread
{
    Q_OBJECT
protected:
    virtual void run();
signals:
    void signalGUI(QImage);
public slots:
    void terminateThread()
    {
        if (isRunning())
        {
            requestInterruption();
            wait();
        }
    }
};

#endif