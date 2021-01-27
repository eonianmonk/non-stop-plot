#include "decoder.h"
#include <QDebug>
#include <iomanip>
#include <cstring>
#include <cstdlib>

tUart_Decoder::tUart_Decoder(QObject *parent) : QObject(parent)
{ }

void tUart_Decoder::decode(const QByteArray& row_data, tUart_PACKET *packet)
{
    //this->inverseQByteArray(row_data); //size = 8
    //memcpy(packet, &row_data->data()[0],8); // don't know how to fit it :(
    memcpy(&packet->type,&row_data.data()[0],2);
    memcpy(&packet->data,&row_data.data()[2],2);
    memcpy(&packet->time,&row_data.data()[4],4);

    // corrupt packet
    if(packet->type != 1 && packet->type != 0)
    {
        packet->type = 0xdead;

#ifdef QT_DEBUG
        for(int i = 0;i<row_data.size();i++)
        {
            qDebug() << hex << (uint16_t)row_data.data()[i];
        }
#endif
    }
}

void tUart_Decoder::inverseQByteArray(QByteArray *array)
{
    size_t size = array->size() - 1;
    char temp;
    for(size_t i = 0; i < (size_t)((size+1)/2);i++)
    {
        temp = array->data()[size-i];
        array->data()[size-i]=array->data()[i];
        array->data()[i] = temp;
    }
}
