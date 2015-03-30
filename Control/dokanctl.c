/*

The MIT License (MIT)

Copyright (c) 2007, 2008 Hiroki Asakawa asakaw@gmail.com
Copyright (c) 2015 AIXIV

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "dokan.h"
#include "dokanc.h"

#define DOKAN_DRIVER_SUBPATH L"\\dokan.sys"
#define DOKAN_MOUNTSERVICE_SUBPATH L"\\dokanMountService.exe"

int ShowMountList()
{
	DOKAN_CONTROL control;
	ULONG index = 0;
	ZeroMemory(&control, sizeof(DOKAN_CONTROL));

	control.Type = DOKAN_CONTROL_LIST;
	control.Option = 0;
	control.Status = DOKAN_CONTROL_SUCCESS;

	while(DokanMountControl(&control)) {
		if (control.Status == DOKAN_CONTROL_SUCCESS) {
			fwprintf(stderr, L"[% 2d] MountPoint: %s\n     DeviceName: %s\n",
				control.Option, control.MountPoint, control.DeviceName);
			control.Option++;
		} else {
			return 0;
		}
	}
	return 0;
}

int ShowUsage()
{
	fprintf(stderr,
		"dokanctrl /u MountPoint (/f)\n" \
		"dokanntl /m\n" \
		"dokanctl /i [d|s|a]\n" \
		"dokanctl /r [d|s|a]\n" \
		"dokanctl /v\n" \
		"\n" \
		"Example:\n" \
		"  /u M:               : Unmount M: drive\n" \
		"  /u C:\\mount\\dokan   : Unmount mount point C:\\mount\\dokan\n" \
		"  /u 1                : Unmount mount point 1\n" \
		"  /u M: /f            : Force unmount M: drive\n" \
		"  /m                  : Print mount points list\n" \
		"  /i s                : Install mounter service\n" \
		"  /r d                : Remove driver\n" \
		"  /r a                : Remove driver and mounter service\n" \
		"  /v                  : Print Dokan version\n");
	return -1;
}

int Unmount(_In_ LPCWSTR	MountPoint, _In_ BOOL ForceUnmount)
{
	int status = 0;
	DOKAN_CONTROL control;
	ZeroMemory(&control, sizeof(DOKAN_CONTROL));

	if (wcslen(MountPoint) == 1 && L'0' <= MountPoint[0] && MountPoint[0] <= L'9') {
		control.Type = DOKAN_CONTROL_LIST;
		control.Option = MountPoint[0] - L'0';
		DokanMountControl(&control);

		if (control.Status == DOKAN_CONTROL_SUCCESS) {
			status = DokanRemoveMountPoint(control.MountPoint);
		} else {
			fwprintf(stderr, L"Mount entry %d not found\n", control.Option);
			status = -1;
		}
	} else if (ForceUnmount) {
		control.Type = DOKAN_CONTROL_UNMOUNT;
		control.Option = DOKAN_CONTROL_OPTION_FORCE_UNMOUNT;
		wcscpy_s(control.MountPoint, sizeof(control.MountPoint) / sizeof(WCHAR), MountPoint);
		DokanMountControl(&control);

		if (control.Status == DOKAN_CONTROL_SUCCESS) {
			fwprintf(stderr, L"Unmount success: %s", MountPoint);
			status = 0;
		} else {
			fwprintf(stderr, L"Unmount failed: %s", MountPoint);
			status = -1;
		}

	} else {
		status = DokanRemoveMountPoint(MountPoint);
	}

	fwprintf(stderr, L"Unmount status = %d\n", status);
	return status;
}

#define GetOption(argc, argv, index) \
	(((argc) > (index) && \
		wcslen((argv)[(index)]) == 2 && \
		(argv)[(index)][0] == L'/')? \
		towlower((argv)[(index)][1]) : L'\0')

VOID InstallDriver(_In_ LPCWSTR DriverPath)
{
	if (DokanDriverInstall(DriverPath))
	{
		fprintf(stderr, "driver install ok\n");
	}
	else
	{
		fprintf(stderr, "driver install failed\n");
	}
}

VOID InstallMounter(_In_ LPCWSTR MounterPath)
{
	if (DokanMounterInstall(MounterPath))
	{
		fprintf(stderr, "mounter install ok\n");
	}
	else
	{
		fprintf(stderr, "mounter install failed\n");
	}
}

VOID RemoveDriver(VOID)
{
	if (DokanDriverDelete())
	{
		fprintf(stderr, "driver remove ok\n");
	}
	else
	{
		fprintf(stderr, "driver remove failed\n");
	}
}

VOID RemoveMounter(VOID)
{
	if (DokanMounterDelete())
	{
		fprintf(stderr, "mounter remove ok\n");
	}
	else
	{
		fprintf(stderr, "mounter remove failed\n");
	}
}

VOID InstallNetworkProvider(VOID)
{
	if (DokanNetworkProviderInstall())
	{
		fprintf(stderr, "network provider install ok\n");
	}
	else
	{
		fprintf(stderr, "network provider install failed\n");
	}
}

VOID RemoveNetworkProvider(VOID)
{
	if (DokanNetworkProviderUninstall())
	{
		fprintf(stderr, "network provider remove ok\n");
	}
	else
	{
		fprintf(stderr, "network provider remove failed\n");
	}
}

int __cdecl
wmain(_In_ int argc, _In_ PWCHAR argv[])
{
	WCHAR	driverFullPath[MAX_PATH];
	WCHAR	mounterFullPath[MAX_PATH];
	WCHAR	type;

	//setlocale(LC_ALL, "");

	if (!GetModuleFileName(NULL, mounterFullPath, MAX_PATH))
	{
		fprintf(stderr, "could not retrieve the applications parent directory\n");
		return -1;
	}


	// search the last "\" (remove own file name, leaves only path)
    WCHAR* lastSlash = wcsrchr(mounterFullPath, L'\\');
    *lastSlash = '\0';

	wcscpy_s(driverFullPath, MAX_PATH, mounterFullPath);

    wcscat_s(mounterFullPath, MAX_PATH, DOKAN_MOUNTSERVICE_SUBPATH);


	//GetSystemDirectory(driverFullPath, MAX_PATH);
    wcscat_s(driverFullPath, MAX_PATH, DOKAN_DRIVER_SUBPATH);

	fwprintf(stderr, L"driver path %s\n", driverFullPath);
	fwprintf(stderr, L"mounter path %s\n", mounterFullPath);


	if (GetOption(argc, argv, 1) == L'v') {
		fprintf(stderr, "dokanctl : %s %s\n", __DATE__, __TIME__);
		fprintf(stderr, "Dokan version : %d\n", DokanVersion());
		fprintf(stderr, "Dokan driver version : 0x%X\n", DokanDriverVersion());
		return 0;
	
	} else if (GetOption(argc, argv, 1) == L'm') {
		return ShowMountList();

	} else if (GetOption(argc, argv, 1) == L'u' && argc == 3) {
		return Unmount(argv[2], FALSE);

	} else if (GetOption(argc, argv, 1) == L'u' &&
				GetOption(argc, argv, 3) == L'f' && argc == 4) {
		return Unmount(argv[2], TRUE);

	} else if (argc < 3 || wcslen(argv[1]) != 2 || argv[1][0] != L'/' ) {
		return ShowUsage();
	}

	type = towlower(argv[2][0]);

	switch(towlower(argv[1][1])) {
	case L'i':
		switch (type)
		{
		case L'd':
			InstallDriver(driverFullPath);
			break;
		case L's':
		case L'm':
			InstallMounter(mounterFullPath);
			break;
		case L'a':
			InstallDriver(driverFullPath);
			InstallMounter(mounterFullPath);
			break;
		case L'n':
			InstallNetworkProvider();
			break;
		}
		break;

	case L'r':
		switch (type)
		{
		case L'd':
			RemoveDriver();
			break;
		case L's':
		case L'm':
			RemoveMounter();
			break;
		case L'a':
			RemoveDriver();
			RemoveMounter();
			break;
		case L'n':
			RemoveNetworkProvider();
			break;
		}
		break;
	case L'd':
		if (L'0' <= type && type <= L'9') {
			ULONG mode = type - L'0';
			if (DokanSetDebugMode(mode)) {
				fprintf(stderr, "set debug mode ok\n");
			} else {
				fprintf(stderr, "set debug mode failed\n");
			}
		}
		break;
	default:
		fprintf(stderr, "unknown option\n");
	}
	

	return 0;
}

