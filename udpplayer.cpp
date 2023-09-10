#include "udpplayer.h"

UDPPlayer::UDPPlayer(QObject *parent) : QObject(parent)
{
    socket = new QUdpSocket();
    socket->bind(AUDIO_UDP_PORT);
    QAudioFormat format = getAudioFormat();
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format))
        format = info.nearestFormat(format);
    output = new QAudioOutput(format);
    device = output->start();
    connect(socket, &QUdpSocket::readyRead, this, &UDPPlayer::playData);
}

void UDPPlayer::playData()
{
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
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if (!info.isFormatSupported(format))
        format = info.nearestFormat(format);
    return format;
}
