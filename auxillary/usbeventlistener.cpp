#include <auxillary/usbeventlistener.h>
//#ifdef _WIN32

#include <windows.h>
#include <dbt.h>
#include <QDebug>

bool DeviceEventFilter::nativeEventFilter(const QByteArray &/*eventType*/, void */*message*/, long *)
{
    qDebug()<<"in filter";
    return false;
}



//    if (eventType == "windows_dispatcher_MSG")
//    {
//        MSG *msg = static_cast<MSG *>(message);

//        if (msg->message == WM_DEVICECHANGE)
//        {
//            if (msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE)
//            {
//                // connect to this signal to reread available ports or devices etc
//                emit serialDeviceChanged(QString(eventType));
//            }
//        }
//    }
    // keep on checking

//#else
//#error Only windows build implemented(no usbeventlistener)
//#endif //_WIN32

