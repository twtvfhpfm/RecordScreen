#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtGui/QPixmap>
#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTime>
#include <memory>
#include <thread>
#include <stdlib.h>
#include <unistd.h>

class ImageSender: public QObject {
    Q_OBJECT
    public:
        ImageSender(QHostAddress ip, int port, int fps): fps_(fps) {
            client_.reset(new QTcpSocket());
            client_->connectToHost(ip, port);
            connect(client_.get(), &QTcpSocket::connected, [this]{
                qDebug() << "connect to server success";
                std::thread t(&ImageSender::sendThreadFunc, this);
                t.detach();
            });
            connect(client_.get(), &QTcpSocket::disconnected, [this]{
                qDebug() << "disconnected from server";
                exitThread = true;
            });
            connect(client_.get(), &QAbstractSocket::errorOccurred, [this]{
                qDebug() << "socket error: " << client_->errorString();
                exitThread = true;
            });
            connect(this, &ImageSender::frameReady, this, &ImageSender::writeData, Qt::BlockingQueuedConnection);
        }

    private:

        void sendThreadFunc() {
            QList<QScreen*> screens = QGuiApplication::screens();
            // 截取第一个屏幕
            QScreen* screen = screens.at(0);
            int count = 0;
            int64_t start_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
            while(!exitThread) {
                QPixmap screenshot = screen->grabWindow(0);
                // 保存截图到文件
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::WriteOnly);
                screenshot.save(&buffer, "jpg", 90);
                emit frameReady(bytes);
                buffer.close();
                if (count++ % 10 == 0) {
                    qDebug() << "save png len " << bytes.size() << "count " << count;
                }

                int64_t now_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
                int64_t shouldElapse = count * (1000 /fps_);
                int64_t actualElapse = now_time - start_time;
                if (shouldElapse > actualElapse) {
                    usleep((shouldElapse - actualElapse) * 1000);
                }
            }

        }

    private:
        std::unique_ptr<QTcpSocket> client_ = nullptr;
        bool exitThread = false;
        int fps_;

    signals:
        void frameReady(QByteArray&);

    public slots:
        void writeData(QByteArray& array) {
            char header[4];
            int size = array.size();
            header[0] = (size >> 24) & 0xFF;
            header[1] = (size >> 16) & 0xFF;
            header[2] = (size >> 8) & 0xFF;
            header[3] = size & 0xFF;
            if (sizeof(header) != client_->write(header, sizeof(header))) {
                qDebug() << "write fail";
            }
            if (size != client_->write(array)) {
                qDebug() << "write array fail";
            }
            client_->flush();
        }

};


