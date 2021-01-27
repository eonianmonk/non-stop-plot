#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>

// used to identify newly connected devices via COM
class DeviceEventFilter : public QAbstractNativeEventFilter, public QObject
{
    Q_OBJECT

public:
    DeviceEventFilter() {};
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;

signals:
    void serialDeviceChanged(QString);
};
