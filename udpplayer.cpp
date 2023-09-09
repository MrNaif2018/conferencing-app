#include "udpplayer.h"

class TracingUDPSocket : public QUdpSocket
{
public:
    TracingUDPSocket(QObject *parent = nullptr) : QUdpSocket(parent) {}
    // read side
    qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *host = nullptr, quint16 *port = nullptr)
    {
        qDebug() << "Reading datagram";
        return QUdpSocket::readDatagram(data, maxlen, host, port);
    }

    qint64 readData(char *data, qint64 maxlen)
    {
        qDebug() << "Reading data";
        return QUdpSocket::readData(data, maxlen);
    }
};

UDPPlayer::UDPPlayer(QObject *parent) : QObject(parent)
{
    qDebug() << "Binding to port " << AUDIO_UDP_PORT;
    socket = new TracingUDPSocket();
    socket->bind(AUDIO_UDP_PORT);
    QAudioFormat format = getAudioFormat();
    qDebug() << "Binding to port v2 " << AUDIO_UDP_PORT;
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    qDebug() << info.deviceName();
    if (!info.isFormatSupported(format))
    {
        qDebug() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }
    qDebug() << "Binding to port v3 " << AUDIO_UDP_PORT;
    output = new QAudioOutput(format);
    device = output->start();
    qDebug() << "Binding to port v4 " << AUDIO_UDP_PORT;
    connect(socket, &QUdpSocket::readyRead, this, &UDPPlayer::playData);
    qDebug() << "Binding to port v5 " << AUDIO_UDP_PORT;
}

void UDPPlayer::playData()
{
    // qDebug() << "READ";
    //  You need to read datagrams from the udp socket
    while (socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(socket->pendingDatagramSize());
        socket->readDatagram(data.data(), data.size());
        device->write(data.data(), data.size());
    }
}

QAudioFormat getAudioFormat()
{
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");

    // If format isn't supported find the nearest supported
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    qDebug() << info.supportedCodecs();
    if (!info.isFormatSupported(format))
    {
        qDebug() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }
    return format;
}

void start_audio_input(char *server)
{
    QAudioFormat format = getAudioFormat();
    QAudioInput *input = new QAudioInput(format);
    TracingUDPSocket *socket = new TracingUDPSocket();
    socket->open(QIODevice::WriteOnly);
    qDebug() << "Binding to port " << AUDIO_UDP_PORT;
    socket->connectToHost(server, AUDIO_UDP_PORT);
    socket->waitForConnected();
    input->start(socket);
}