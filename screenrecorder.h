#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QThread>
#include <QMutex>

class ScreenRecorder : public QThread
{
    Q_OBJECT
private:
    char *server;

public:
    ScreenRecorder(char *server) : server(server) {}

protected:
    virtual void run();
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