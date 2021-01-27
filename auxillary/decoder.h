#pragma once

#include <QObject>
#include <cstdint>

struct tUart_PACKET
{
    uint32_t time;
    uint16_t data;
    uint16_t type;
};

class tUart_Decoder : public QObject
{
    Q_OBJECT
public:
    explicit tUart_Decoder(QObject *parent = nullptr);
    void decode(const QByteArray& row_data, tUart_PACKET *packet);

private:
    void inverseQByteArray(QByteArray *array);

signals:

};

