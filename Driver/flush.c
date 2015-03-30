/*
Dokan : user-mode file system library for Windows

Copyright (C) 2008 Hiroki Asakawa info@dokan-dev.net

http://dokan-dev.net/en

Copyright (c) 2015 AIXIV
https://github.com/AIXIV/dokan

Dokan is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "dokan.h"

#pragma alloc_text("PAGED_CODE", DokanDispatchFlush)
NTSTATUS
DokanDispatchFlush(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
	)
{
	PIO_STACK_LOCATION	irpSp;
	PFILE_OBJECT		fileObject;
	PVOID				buffer;
	NTSTATUS			status = STATUS_INVALID_PARAMETER;
	PDokanFCB			fcb;
	PDokanCCB			ccb;
	PDokanVCB			vcb;
	PEVENT_CONTEXT		eventContext;
	ULONG				eventLength;

	PAGED_CODE();
	
	__try {
		FsRtlEnterFileSystem();

		DDbgPrint("==> DokanFlush\n");

		irpSp		= IoGetCurrentIrpStackLocation(Irp);
		fileObject	= irpSp->FileObject;

		if (!DokanGetDispatchParameters(DeviceObject, fileObject, &vcb, &ccb, &fcb))
		{
			status = STATUS_INVALID_PARAMETER; // TODO: was STATUS_SUCCESS for whatever reason
			__leave;
		}

		DokanPrintFileName(fileObject);


		eventLength = sizeof(EVENT_CONTEXT) + fcb->FileName.Length;
		eventContext = AllocateEventContext(vcb->Dcb, Irp, eventLength, ccb);

		if (eventContext == NULL) {
			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}

		eventContext->Context = ccb->UserContext;
		DDbgPrint("   get Context %X\n", (ULONG)ccb->UserContext);

		// copy file name to be flushed
		eventContext->Flush.FileNameLength = fcb->FileName.Length;
		RtlCopyMemory(eventContext->Flush.FileName, fcb->FileName.Buffer, fcb->FileName.Length);

		CcUninitializeCacheMap(fileObject, NULL, NULL);
		//fileObject->Flags &= FO_CLEANUP_COMPLETE;

		// register this IRP to waiting IRP list and make it pending status
		status = DokanRegisterPendingIrp(DeviceObject, Irp, eventContext, 0);

	} __finally {

		// if status is not pending, must complete current IRPs
		if (status != STATUS_PENDING) {
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			DokanPrintNTStatus(status);
		} else {
			DDbgPrint("  STATUS_PENDING\n");
		}

		DDbgPrint("<== DokanFlush\n");

		FsRtlExitFileSystem();
	}

	return status;
}


VOID
DokanCompleteFlush(
	 _In_ PIRP_ENTRY			IrpEntry,
	 _In_ PEVENT_INFORMATION	EventInfo
	 )
{
	PIRP				irp;
	PIO_STACK_LOCATION	irpSp;
	NTSTATUS			status   = STATUS_SUCCESS;
	ULONG				info	 = 0;
	PDokanCCB			ccb;
	PFILE_OBJECT		fileObject;

	irp   = IrpEntry->Irp;
	irpSp = IrpEntry->IrpSp;

	//FsRtlEnterFileSystem();

	DDbgPrint("==> DokanCompleteFlush\n");

	fileObject = irpSp->FileObject;

	if (!DokanGetDispatchContext(fileObject, &ccb, NULL))
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{
		ccb->UserContext = EventInfo->Context;
		DDbgPrint("   set Context %X\n", (ULONG)ccb->UserContext);

		status = EventInfo->Status;
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	DokanPrintNTStatus(status);

	DDbgPrint("<== DokanCompleteFlush\n");

	//FsRtlExitFileSystem();

}

