/*.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++
Released into the Public Domain.

  This source code is provided without warranty of any kind.
  Expect bugs. Feedback would be greatly appreciated.
  Feel free to use and distribute in compiled form.
  Source redistribution is restricted to its unmodified form.

Copyright © 2008, Bornish
.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++.c++*/
// Author:	http://www.brainbench.com/transcript.jsp?pid=2619477
/*!
 Purpose:
	Activates / deactivates the SiRF Star III GPS receiver (GSC3f) on COM2 of an ASUS R2H UMPC.
*/
#ifndef _WINDOWS_
	#include <windows.h>
#endif

// Create custom code 0x222404
#ifndef FILE_DEVICE_UNKNOWN
	#define FILE_DEVICE_UNKNOWN	0x22
#endif
#ifndef METHOD_BUFFERED
	#define METHOD_BUFFERED	0x0
#endif
#ifndef FILE_ANY_ACCESS
	#define FILE_ANY_ACCESS	0x0
#endif
#ifndef CTL_CODE
	#define CTL_CODE(DeviceType,Function,Method,Access) \
		( \
			((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
		)
#endif

#define CTL_CUSTOM_CODE(DeviceType,Function,Method,Access) \
		( \
			(0x1 << 13) | CTL_CODE(DeviceType,Function,Method,Access) \
		)
#define IOCTL_GPS_DEVICE \
		CTL_CUSTOM_CODE(FILE_DEVICE_UNKNOWN,0x101,METHOD_BUFFERED,FILE_ANY_ACCESS)

#pragma message("   Automatically linking with Advapi32.lib")
#pragma comment(lib,"Advapi32.lib")

/*!
	Argument structure specific to ATKACPI.*/
struct DrvArg {
	short v[2];
	long port;
};
/*!
	Data structure specific to ATKACPI.*/
struct DrvData {
	long unknown;
	char code[4];
	long argNo;
	long argSize;
	DrvArg * args;
};

/*!
	Writes entry in registry.*/
BOOL SetRegistryValue(const char * szKey,const char * szValue,const char * szData,HKEY hOpenKey = HKEY_LOCAL_MACHINE) {
	if (
		!hOpenKey || !szKey || !szKey[0] || !szData
		) {
		::SetLastError((DWORD)E_INVALIDARG);
		return FALSE;
	}
	BOOL bRetVal = FALSE;
	DWORD dwDisposition;
	DWORD dwReserved = NULL;
	HKEY hTempKey = (HKEY)NULL;
	DWORD dwBufferLength = strlen(szData);
	if (
		ERROR_SUCCESS == ::RegCreateKeyEx(
			hOpenKey,szKey,dwReserved,
			(LPTSTR)NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,0,
			&hTempKey,&dwDisposition)
		) {
		dwBufferLength += sizeof(char);
		if (
			ERROR_SUCCESS == ::RegSetValueEx(
				hTempKey,szValue,dwReserved,
				REG_SZ,(LPBYTE)szData,dwBufferLength)
			) {
			bRetVal = TRUE;
		}
	}
	if (hTempKey) {
		::RegCloseKey(hTempKey);
	}
	return bRetVal;
}
/*!
	Reads entry from registry.*/
BOOL GetRegistryValue(const char * szKey,const char * szValue,char * szData,DWORD dwBufferLength,HKEY hOpenKey = HKEY_LOCAL_MACHINE) {
	if (
		!hOpenKey || !szKey || !szKey[0] || !szData
		) {
		::SetLastError((DWORD)E_INVALIDARG);
		return FALSE;
	}
	BOOL bRetVal = FALSE;
	DWORD dwReserved = NULL;
	DWORD dwType = REG_SZ;
	HKEY hTempKey = (HKEY)NULL;
	if (
		ERROR_SUCCESS == ::RegOpenKeyEx(
			hOpenKey,szKey,dwReserved,
			KEY_QUERY_VALUE,
			&hTempKey)
		) {
		SetLastError(0);
		if (
			ERROR_SUCCESS == ::RegQueryValueEx(
				hTempKey,szValue,NULL,
				&dwType,(LPBYTE)szData,&dwBufferLength)
			) {
			bRetVal = TRUE;
		}
	}
	if (hTempKey) {
		::RegCloseKey(hTempKey);
	}
	return bRetVal;
}

/*!
	Conventional entry point of the module.*/
int main(int argc, char * argv[], char * []) {
	const char * szSection = "Software\\gpsCtrl\\Device";
	const char * szKeyName = "Active";
	char szKeyValue[MAX_PATH];
	memset(szKeyValue,65,MAX_PATH);
	if (!GetRegistryValue(szSection,szKeyName,szKeyValue,MAX_PATH))
		memcpy(szKeyValue,"on",sizeof("on")+1);
	if (argc > 1) {
		if(memcmp(argv[1],"off",sizeof("off")) == 0) {
			memcpy(szKeyValue,"off",sizeof("off")+1);
		}
		else {
			memcpy(szKeyValue,"on",sizeof("on")+1);
		}
	}
	HANDLE hATKACPI = CreateFile("\\\\.\\ATKACPI",NULL,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
	if (hATKACPI == INVALID_HANDLE_VALUE) {
		return -1;
	}
	unsigned char lpOutBuffer[0x300];
	memset(lpOutBuffer,0,0x300);
	DWORD BytesReturned;
	DrvArg inArg;
	DrvData inData;
	inData.unknown = 2;
	if(memcmp(szKeyValue,"off",sizeof("off")) == 0) { // OnApply - OFF
		memcpy(inData.code,"SDOF",4);
	}
	else { // OnApply - ON
		memcpy(szKeyValue,"on",sizeof("on")+1); // ensure valid registry value
		memcpy(inData.code,"SDON",4);
	}
	inData.argNo = 1;
	inData.argSize = 8 * inData.argNo;
	inData.args = &inArg;
	inArg.v[0] = 0;
	inArg.v[1] = 4;
	inArg.port = 2;
	if (
		!DeviceIoControl(
			hATKACPI,IOCTL_GPS_DEVICE,&inData,sizeof(DrvData),
			lpOutBuffer,0x300,&BytesReturned,NULL)
		) {
		return -2;
	}
	if (!SetRegistryValue(szSection,szKeyName,szKeyValue)) {
		return -3;
	}
	CloseHandle(hATKACPI);
	return 0;
}
