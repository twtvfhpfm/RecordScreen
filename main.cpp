#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtGui/QPixmap>
#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QTcpSocket>
#include <QHostAddress>
#include <memory>
#include <thread>
#include "ImageSender.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (argc < 4) {
        qDebug() << "usage: <ip> <port> <fps>";
        return -1;
    }
    int port = atoi(argv[2]);
    char* ip = argv[1];
    int fps = atoi(argv[3]);
    std::unique_ptr<ImageSender> sender = std::make_unique<ImageSender>(QHostAddress(ip), port, fps);

    return a.exec();
}

