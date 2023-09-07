#ifndef AUDIOWORKER_H
#define AUDIOWORKER_H

#include <QThread>
#include <QMutex>

class AudioWorkerThread : public QThread
{
    Q_OBJECT
private:
    char **argv;

public:
    AudioWorkerThread(char **argv) : argv(argv) {}

protected:
    virtual void run();
};

#endif