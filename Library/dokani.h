/*
  Dokan : user-mode file system library for Windows

  Copyright (C) 2008 Hiroki Asakawa info@dokan-dev.net

  http://dokan-dev.net/en

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _DOKANI_H_
#define _DOKANI_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "public.h"
#include "dokan.h"
#include "dokanc.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct _DOKAN_INSTANCE {
	// to ensure that unmount dispatch is called at once
	CRITICAL_SECTION	CriticalSection;

	// store CurrentDeviceName
	// (when there are many mounts, each mount use 
	// other DeviceName)
	WCHAR	DeviceName[64];
	WCHAR	MountPoint[MAX_PATH];

	ULONG	DeviceNumber;
	ULONG	MountId;

	PDOKAN_OPTIONS		DokanOptions;
	PDOKAN_OPERATIONS	DokanOperations;

	LIST_ENTRY	ListEntry;
} DOKAN_INSTANCE, *PDOKAN_INSTANCE;


typedef struct _DOKAN_OPEN_INFO {
	BOOL			IsDirectory;
	ULONG			OpenCount;
	PEVENT_CONTEXT	EventContext;
	PDOKAN_INSTANCE	DokanInstance;
	ULONG64			UserContext;
	ULONG			EventId;
	PLIST_ENTRY		DirListHead;
} DOKAN_OPEN_INFO, *PDOKAN_OPEN_INFO;


BOOL
DokanStart(
	_In_ PDOKAN_INSTANCE	Instance);

BOOL
SendToDevice(
	_In_ LPCWSTR		DeviceName,
	_In_ DWORD			IoControlCode,
	_In_opt_ PVOID		InputBuffer,
	_In_ ULONG			InputLength,
	_Out_opt_ PVOID		OutputBuffer,
	_In_ ULONG			OutputLength,
	_Out_opt_ PULONG	ReturnedLength);

LPCWSTR
GetRawDeviceName(_In_ LPCWSTR	DeviceName);

DWORD __stdcall
DokanLoop(
	_In_ PVOID Param);


BOOL
DokanMount(
	_In_ LPCWSTR	MountPoint,
	_In_ LPCWSTR	DeviceName);

VOID
SendEventInformation(
	_In_ HANDLE					Handle,
	_In_ PEVENT_INFORMATION		EventInfo,
	_In_ ULONG					EventLength,
	_In_opt_ PDOKAN_INSTANCE	DokanInstance);

VOID
SetupFailureEventInformation(
	_In_ PEVENT_CONTEXT			EventContext,
	_In_ PDOKAN_INSTANCE		DokanInstance,
	_Out_ PEVENT_INFORMATION	EventInfo
);

_Success_(return != NULL)
_Ret_maybenull_
PEVENT_INFORMATION
DispatchCommon(
	_In_ PEVENT_CONTEXT								EventContext,
	_In_ ULONG										SizeOfEventInfo,
	_In_ PDOKAN_INSTANCE							DokanInstance,
	_Out_ PDOKAN_FILE_INFO							DokanFileInfo,
	_Outptr_result_maybenull_ PDOKAN_OPEN_INFO*		DokanOpenInfo);


VOID
DispatchDirectoryInformation(
	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchQueryInformation(
 	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchQueryVolumeInformation(
 	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchSetInformation(
 	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchRead(
 	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchWrite(
 	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchCreate(
	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchClose(
  	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchCleanup(
  	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchFlush(
  	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchUnmount(
  	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchLock(
	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchQuerySecurity(
	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


VOID
DispatchSetSecurity(
	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE	DokanInstance);


BOOLEAN
InstallDriver(
	_In_ SC_HANDLE  SchSCManager,
	_In_ LPCWSTR    DriverName,
	_In_ LPCWSTR    ServiceExe);


BOOLEAN
RemoveDriver(
	_In_ SC_HANDLE  SchSCManager,
	_In_ LPCWSTR    DriverName);


BOOLEAN
StartDriver(
	_In_ SC_HANDLE  SchSCManager,
	_In_ LPCWSTR    DriverName);


BOOLEAN
StopDriver(
	_In_ SC_HANDLE  SchSCManager,
	_In_ LPCWSTR    DriverName);


BOOLEAN
ManageDriver(
	_In_ LPCWSTR  DriverName,
	_In_ LPCWSTR  ServiceName,
	_In_ USHORT   Function);


BOOL
SendReleaseIRP(
	_In_ LPCWSTR DeviceName);

VOID
CheckFileName(
	_In_ LPWSTR	FileName);

VOID
ClearFindData(
	_In_opt_ PLIST_ENTRY ListHead);

DWORD WINAPI
DokanKeepAlive(
	_In_ PDOKAN_INSTANCE Param);


ULONG
GetNTStatus(_In_ DWORD ErrorCode);

PDOKAN_OPEN_INFO
GetDokanOpenInfo(
	_In_ PEVENT_CONTEXT		EventInfomation,
	_In_ PDOKAN_INSTANCE	DokanInstance);

VOID
ReleaseDokanOpenInfo(
	_In_ PEVENT_INFORMATION	EventInfomation,
	_In_ PDOKAN_INSTANCE	DokanInstance);


#ifdef __cplusplus
}
#endif


#endif
