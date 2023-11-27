#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QThread>
#include <QMutex>

class ScreenRecorder : public QThread
{
    Q_OBJECT
private:
    char *server;
    int pack_size;
    int frame_interval;
    int quality;

public:
    ScreenRecorder(char *server, int pack_size, int frame_interval, int quality)
        : server(server), pack_size(pack_size), frame_interval(frame_interval), quality(quality) {}

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