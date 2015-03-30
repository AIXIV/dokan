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

#pragma alloc_text("PAGED_CODE", DokanDispatchQueryVolumeInformation)
NTSTATUS
DokanDispatchQueryVolumeInformation(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
   )
{
	NTSTATUS			status = STATUS_INVALID_PARAMETER;
	PIO_STACK_LOCATION  irpSp;
	PVOID				buffer;
	PFILE_OBJECT		fileObject;
	PDokanVCB			vcb;
	PDokanDCB			dcb;
	PDokanCCB			ccb;
	ULONG               info = 0;

	PAGED_CODE();

	__try {

		FsRtlEnterFileSystem();

		DDbgPrint("==> DokanQueryVolumeInformation\n");
		DDbgPrint("  ProcessId %lu\n", IoGetRequestorProcessId(Irp));

		irpSp			= IoGetCurrentIrpStackLocation(Irp);
		buffer			= Irp->AssociatedIrp.SystemBuffer;

		fileObject		= irpSp->FileObject;
		if (!DokanGetDispatchParameters(DeviceObject, fileObject, &vcb, &ccb, NULL))
		{
			status = STATUS_INVALID_PARAMETER;
			__leave;
		}
		dcb = vcb->Dcb;

		DDbgPrint("  FileName: %wZ\n", &fileObject->FileName);

		switch(irpSp->Parameters.QueryVolume.FsInformationClass) {
		case FileFsVolumeInformation:
			DDbgPrint("  FileFsVolumeInformation\n");
			break;

		case FileFsLabelInformation:
			DDbgPrint("  FileFsLabelInformation\n");
			break;
	        
		case FileFsSizeInformation:
			DDbgPrint("  FileFsSizeInformation\n");
			break;
	    
		case FileFsDeviceInformation:
			{
				PFILE_FS_DEVICE_INFORMATION device;
				DDbgPrint("  FileFsDeviceInformation\n");
				device = (PFILE_FS_DEVICE_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
				if (irpSp->Parameters.QueryVolume.Length < sizeof(FILE_FS_DEVICE_INFORMATION)) {
					status = STATUS_BUFFER_TOO_SMALL;
					info = sizeof(FILE_FS_DEVICE_INFORMATION);
					__leave;
				}
				device->DeviceType = dcb->DeviceType;
				device->Characteristics = dcb->DeviceCharacteristics;
				status = STATUS_SUCCESS;
				info = sizeof(FILE_FS_DEVICE_INFORMATION);
				__leave;
			}
			break;
	    
		case FileFsAttributeInformation:
			DDbgPrint("  FileFsAttributeInformation\n");
			break;
	    
		case FileFsControlInformation:
			DDbgPrint("  FileFsControlInformation\n");
			break;
	    
		case FileFsFullSizeInformation:
			DDbgPrint("  FileFsFullSizeInformation\n");
			break;
		case FileFsObjectIdInformation:
			DDbgPrint("  FileFsObjectIdInformation\n");
			break;
	    
		case FileFsMaximumInformation:
			DDbgPrint("  FileFsMaximumInformation\n");
			break;
	    
		default:
			break;
		}


		if (irpSp->Parameters.QueryVolume.FsInformationClass == FileFsVolumeInformation
			|| irpSp->Parameters.QueryVolume.FsInformationClass == FileFsSizeInformation
			|| irpSp->Parameters.QueryVolume.FsInformationClass == FileFsAttributeInformation
			|| irpSp->Parameters.QueryVolume.FsInformationClass == FileFsFullSizeInformation) {


			ULONG			eventLength = sizeof(EVENT_CONTEXT);
			PEVENT_CONTEXT	eventContext;

			if (ccb && !DokanCheckCCB(vcb->Dcb, fileObject->FsContext2)) {
				status = STATUS_INVALID_PARAMETER;
				__leave;
			}

			// this memory must be freed in this {}
			eventContext = AllocateEventContext(vcb->Dcb, Irp, eventLength, NULL);

			if (eventContext == NULL) {
				status = STATUS_INSUFFICIENT_RESOURCES;
				__leave;
			}
		
			if (ccb) {
				eventContext->Context = ccb->UserContext;
				eventContext->FileFlags = ccb->Flags;
				//DDbgPrint("   get Context %X\n", (ULONG)ccb->UserContext);
			}

			eventContext->Volume.FsInformationClass =
				irpSp->Parameters.QueryVolume.FsInformationClass;

			// the length which can be returned to user-mode
			eventContext->Volume.BufferLength = irpSp->Parameters.QueryVolume.Length;


			status = DokanRegisterPendingIrp(DeviceObject, Irp, eventContext, 0);
		}

	} __finally {

		if (status != STATUS_PENDING) {
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = info;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			DokanPrintNTStatus(status);
		}

		DDbgPrint("<== DokanQueryVolumeInformation\n");

		FsRtlExitFileSystem();
	}

	return status;
}


VOID
DokanCompleteQueryVolumeInformation(
	_In_ PIRP_ENTRY			IrpEntry,
	_In_ PEVENT_INFORMATION	EventInfo
)
{
	PIRP				irp;
	PIO_STACK_LOCATION	irpSp;
	NTSTATUS			status = STATUS_SUCCESS;
	ULONG				info = 0;
	ULONG				bufferLen = 0;
	PVOID				buffer = NULL;
	PDokanCCB			ccb;

	//FsRtlEnterFileSystem();

	DDbgPrint("==> DokanCompleteQueryVolumeInformation\n");

	irp = IrpEntry->Irp;
	irpSp = IrpEntry->IrpSp;

	if (!DokanGetDispatchContext(IrpEntry->FileObject, &ccb, NULL))
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else
	{

		// does not save Context!!
		// ccb->UserContext = EventInfo->Context;

		// buffer which is used to copy VolumeInfo
		buffer = irp->AssociatedIrp.SystemBuffer;

		// available buffer size to inform
		bufferLen = irpSp->Parameters.QueryVolume.Length;

		// if buffer is invalid or short of length
		if (bufferLen == 0 || buffer == NULL || bufferLen < EventInfo->BufferLength) {

			info = 0;
			status = STATUS_INSUFFICIENT_RESOURCES;

		}
		else {

			// copy the information from user-mode to specified buffer
			ASSERT(buffer != NULL);

			RtlZeroMemory(buffer, bufferLen);
			RtlCopyMemory(buffer, EventInfo->Buffer, EventInfo->BufferLength);

			// the written length
			info = EventInfo->BufferLength;

			status = EventInfo->Status;
		}
	}


	irp->IoStatus.Status = status;
	irp->IoStatus.Information = info;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	DokanPrintNTStatus(status);
	DDbgPrint("<== DokanCompleteQueryVolumeInformation\n");

	//FsRtlExitFileSystem();
}


#pragma alloc_text("PAGED_CODE", DokanDispatchSetVolumeInformation)
NTSTATUS
DokanDispatchSetVolumeInformation(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
   )
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;

	PAGED_CODE();

	//FsRtlEnterFileSystem();

	DDbgPrint("==> DokanSetVolumeInformation\n");

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	DDbgPrint("<== DokanSetVolumeInformation");

	//FsRtlExitFileSystem();

	return status;
}

