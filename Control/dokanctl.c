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

int InstallMode(_In_ int argc, _In_ PWCHAR argv[])
{
	if (argc < 4)
	{
		return ShowUsage();
	}
	switch (towlower(argv[2][0]))
	{
	case L'd':
		InstallDriver(argv[3]);
		return 0;
	case L'm':
	case L's':
		InstallMounter(argv[3]);
		return 0;
	case L'n':
		InstallNetworkProvider();
		return 0;
	default:
		return ShowUsage();
	}
}

int RemoveMode(_In_ int argc, _In_ PWCHAR argv[])
{
	switch (towlower(argv[2][0]))
	{
	case L'd':
		RemoveDriver();
		return 0;
	case L'm':
	case L's':
		RemoveMounter();
		return 0;
	case L'a':
		RemoveMounter();
		RemoveDriver();
		return 0;
	case L'n':
		RemoveNetworkProvider();
		return 0;
	default:
		return ShowUsage();
	}
}

int __cdecl
wmain(_In_ int argc, _In_ PWCHAR argv[])
{
	if (argc < 3 || wcslen(argv[1]) != 2 || argv[1][0] != L'/') {
		return ShowUsage();
	}

	switch (tolower(argv[1][1]))
	{
	case L'v':
		fprintf(stderr, "dokanctl : %s %s\n", __DATE__, __TIME__);
		fprintf(stderr, "Dokan version : %d\n", DokanVersion());
		fprintf(stderr, "Dokan driver version : 0x%X\n", DokanDriverVersion());
		return 0;
	case L'm':
		ShowMountList();
		return 0;
	case L'u':
		if (argc == 3)
		{
			return Unmount(argv[2], FALSE);
		}
		else if (argc == 4 && GetOption(argc, argv, 3) == L'f')
		{
			return Unmount(argv[2], TRUE);
		}
		else
		{
			return ShowUsage();
		}
	case L'i':
		return InstallMode(argc, argv);
	case L'r':
		return RemoveMode(argc, argv);
	case L'd':
		{
			WCHAR type = towlower(argv[2][0]);
			if (L'0' <= type && type <= L'9')
			{
				ULONG mode = type - L'0';
				if (DokanSetDebugMode(mode)) {
					fprintf(stderr, "set debug mode ok\n");
				}
				else {
					fprintf(stderr, "set debug mode failed\n");
				}
			}
			return 0;
		}
	default:
		return ShowUsage();
	}
}

