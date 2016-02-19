
#ifndef TOAST_NOTIFICATION_H
#define TOAST_NOTIFICATION_H

#include <windows.h>

//#ifdef BUILDING_DLL
//#define DllExport __declspec(dllexport)
//#else
//#define DllExport __declspec(dllimport)
//#endif


typedef void (STDAPICALLTYPE *EventCallback)();


//DllExport
BOOL
SetAppId();

//DllExport
BOOL
DisplayToastNotification(const wchar_t* aImage, const wchar_t* aTitle, const wchar_t* aMessage, const wchar_t* aName, EventCallback aCallbackActive, EventCallback aCallbackDismiss);


//DllExport
BOOL
CloseToastNotification(const wchar_t* aName);


#endif // TOAST_NOTIFICATION_H