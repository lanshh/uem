// uem.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string"
#include "usbfuns.h"
using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
    ULONG   devicesConnected;
    wprintf((TEXT("        console application for USB devices enumerate\r\n        by lanshh\r\n        20150408")));
    setlocale(LC_CTYPE, ".936");
    EnumerateHostControllers(&devicesConnected);
    //gets_s(str1,128);
    getchar();
	return 0;
}
