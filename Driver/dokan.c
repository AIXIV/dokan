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


#include "dokan.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DokanUnload)
#pragma alloc_text (PAGE, DokanDispatchShutdown)
#pragma alloc_text (PAGE, DokanDispatchPnp)
#endif


ULONG g_Debug = DOKAN_DEBUG_DEFAULT;

#if _WIN32_WINNT < 0x0501
	PFN_FSRTLTEARDOWNPERSTREAMCONTEXTS DokanFsRtlTeardownPerStreamContexts;
#endif

NPAGED_LOOKASIDE_LIST	DokanIrpEntryLookasideList;
UNICODE_STRING			FcbFileNameNull;

FAST_IO_CHECK_IF_POSSIBLE DokanFastIoCheckIfPossible;

BOOLEAN
DokanFastIoCheckIfPossible (
    _In_ PFILE_OBJECT	FileObject,
    _In_ PLARGE_INTEGER	FileOffset,
    _In_ ULONG			Length,
    _In_ BOOLEAN		Wait,
    _In_ ULONG			LockKey,
    _In_ BOOLEAN		CheckForReadOperation,
	_Pre_notnull_
	_When_(return != FALSE, _Post_equal_to_(_Old_(IoStatus)))
	_When_(return == FALSE, _Post_valid_)
    PIO_STATUS_BLOCK	IoStatus,
    _In_ PDEVICE_OBJECT		DeviceObject
    )
{
	DDbgPrint("DokanFastIoCheckIfPossible\n");
	return FALSE;
}

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT  DriverObject,
	_In_ PUNICODE_STRING RegistryPath
	)

/*++

Routine Description:

	This routine gets called by the system to initialize the driver.

Arguments:

	DriverObject	- the system supplied driver object.
	RegistryPath	- the system supplied registry path for this driver.

Return Value:

	NTSTATUS

--*/

{
	PDEVICE_OBJECT		deviceObject;
	NTSTATUS			status;
	PFAST_IO_DISPATCH	fastIoDispatch;
	UNICODE_STRING		functionName;
	PDOKAN_GLOBAL		dokanGlobal = NULL;

	DDbgPrint("==> DriverEntry ver.%x, %s %s\n", DOKAN_DRIVER_VERSION, __DATE__, __TIME__);

	status = DokanCreateGlobalDiskDevice(DriverObject, &dokanGlobal);

	if (status != STATUS_SUCCESS) {
		DDbgPrint("  Could not create global disk device\n");
		return status;
	}

	__try
	{
		//
		// Set up dispatch entry points for the driver.
		//
		DriverObject->DriverUnload = DokanUnload;

		DriverObject->MajorFunction[IRP_MJ_CREATE] = DokanDispatchCreate;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = DokanDispatchClose;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DokanDispatchCleanup;

		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DokanDispatchDeviceControl;
		DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = DokanDispatchFileSystemControl;
		DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] = DokanDispatchDirectoryControl;

		DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = DokanDispatchQueryInformation;
		DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = DokanDispatchSetInformation;

		DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = DokanDispatchQueryVolumeInformation;
		DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION] = DokanDispatchSetVolumeInformation;

		DriverObject->MajorFunction[IRP_MJ_READ] = DokanDispatchRead;
		DriverObject->MajorFunction[IRP_MJ_WRITE] = DokanDispatchWrite;
		DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = DokanDispatchFlush;

		DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DokanDispatchShutdown;
		DriverObject->MajorFunction[IRP_MJ_PNP] = DokanDispatchPnp;

		DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL] = DokanDispatchLock;

		DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY] = DokanDispatchQuerySecurity;
		DriverObject->MajorFunction[IRP_MJ_SET_SECURITY] = DokanDispatchSetSecurity;

		fastIoDispatch = ExAllocatePool(sizeof(FAST_IO_DISPATCH));
		if (fastIoDispatch == NULL)
		{
			DDbgPrint("  Could not allocate fastIoDispatch structure\n");
			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}
		RtlZeroMemory(fastIoDispatch, sizeof(FAST_IO_DISPATCH));

		fastIoDispatch->SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH);
		fastIoDispatch->FastIoCheckIfPossible = DokanFastIoCheckIfPossible;

#pragma warning(suppress: 28175)
		DriverObject->FastIoDispatch = fastIoDispatch;


		ExInitializeNPagedLookasideList(
			&DokanIrpEntryLookasideList, NULL, NULL, 0, sizeof(IRP_ENTRY), TAG, 0);


#if _WIN32_WINNT < 0x0501
		RtlInitUnicodeString(&functionName, L"FsRtlTeardownPerStreamContexts");
		DokanFsRtlTeardownPerStreamContexts = MmGetSystemRoutineAddress(&functionName);
#endif
	}
	__finally
	{
		if (!NT_SUCCESS(status))
		{
			IoDeleteDevice(dokanGlobal->DeviceObject);
		}
	}


	DDbgPrint("<== DriverEntry\n");

	return( status );
}


VOID
DokanUnload(
	_In_ PDRIVER_OBJECT DriverObject
	)
/*++

Routine Description:

	This routine gets called to remove the driver from the system.

Arguments:

	DriverObject	- the system supplied driver object.

Return Value:

	NTSTATUS

--*/

{

	PDEVICE_OBJECT	deviceObject = DriverObject->DeviceObject;
	WCHAR			symbolicLinkBuf[] = DOKAN_GLOBAL_SYMBOLIC_LINK_NAME;
	UNICODE_STRING	symbolicLinkName;

	PAGED_CODE();
	DDbgPrint("==> DokanUnload\n");

	if (GetIdentifierType(deviceObject->DeviceExtension) == DGL) {
		DDbgPrint("  Delete Global DeviceObject\n");
		RtlInitUnicodeString(&symbolicLinkName, symbolicLinkBuf);
		IoDeleteSymbolicLink(&symbolicLinkName);
		IoDeleteDevice(deviceObject);
	}

	ExDeleteNPagedLookasideList(&DokanIrpEntryLookasideList);

	DDbgPrint("<== DokanUnload\n");
	return;
}



NTSTATUS
DokanDispatchShutdown(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
   )
{
	PAGED_CODE();
	DDbgPrint("==> DokanShutdown\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	DDbgPrint("<== DokanShutdown\n");
	return STATUS_SUCCESS;
}




NTSTATUS
DokanDispatchPnp(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
   )
{
	PIO_STACK_LOCATION	irpSp;
	NTSTATUS			status = STATUS_SUCCESS;

	PAGED_CODE();

	__try {
		DDbgPrint("==> DokanPnp\n");

		irpSp = IoGetCurrentIrpStackLocation(Irp);

		switch (irpSp->MinorFunction) {
		case IRP_MN_QUERY_REMOVE_DEVICE:
			DDbgPrint("  IRP_MN_QUERY_REMOVE_DEVICE\n");
			break;
		case IRP_MN_SURPRISE_REMOVAL:
			DDbgPrint("  IRP_MN_SURPRISE_REMOVAL\n");
			break;
		case IRP_MN_REMOVE_DEVICE:
			DDbgPrint("  IRP_MN_REMOVE_DEVICE\n");
			break;
		case IRP_MN_CANCEL_REMOVE_DEVICE:
			DDbgPrint("  IRP_MN_CANCEL_REMOVE_DEVICE\n");
			break;
		case IRP_MN_QUERY_DEVICE_RELATIONS:
			DDbgPrint("  IRP_MN_QUERY_DEVICE_RELATIONS\n");
			status = STATUS_INVALID_PARAMETER;
			break;
		default:
			DDbgPrint("   other minnor function %d\n", irpSp->MinorFunction);
			break;
			//IoSkipCurrentIrpStackLocation(Irp);
			//status = IoCallDriver(Vcb->TargetDeviceObject, Irp);
		}
	} __finally {
		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		DDbgPrint("<== DokanPnp\n");
	}

	return status;
}



BOOLEAN
DokanNoOpAcquire(
    _In_ PVOID Fcb,
    _In_ BOOLEAN Wait
    )
{
    UNREFERENCED_PARAMETER( Fcb );
    UNREFERENCED_PARAMETER( Wait );

	DDbgPrint("==> DokanNoOpAcquire\n");

    ASSERT(IoGetTopLevelIrp() == NULL);

    IoSetTopLevelIrp((PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

	DDbgPrint("<== DokanNoOpAcquire\n");
    
	return TRUE;
}


VOID
DokanNoOpRelease(
    _In_ PVOID Fcb
    )
{
	DDbgPrint("==> DokanNoOpRelease\n");
    ASSERT(IoGetTopLevelIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP);

    IoSetTopLevelIrp( NULL );

    UNREFERENCED_PARAMETER( Fcb );
	
	DDbgPrint("<== DokanNoOpRelease\n");
    return;
}


#define PrintStatus(val, flag) if(val == flag) DDbgPrint("  status = " #flag "\n")


VOID
DokanPrintNTStatus(
	_In_ NTSTATUS	Status)
{
	PrintStatus(Status, STATUS_SUCCESS);
	PrintStatus(Status, STATUS_NO_MORE_FILES);
	PrintStatus(Status, STATUS_END_OF_FILE);
	PrintStatus(Status, STATUS_NO_SUCH_FILE);
	PrintStatus(Status, STATUS_NOT_IMPLEMENTED);
	PrintStatus(Status, STATUS_BUFFER_OVERFLOW);
	PrintStatus(Status, STATUS_FILE_IS_A_DIRECTORY);
	PrintStatus(Status, STATUS_SHARING_VIOLATION);
	PrintStatus(Status, STATUS_OBJECT_NAME_INVALID);
	PrintStatus(Status, STATUS_OBJECT_NAME_NOT_FOUND);
	PrintStatus(Status, STATUS_OBJECT_NAME_COLLISION);
	PrintStatus(Status, STATUS_OBJECT_PATH_INVALID);
	PrintStatus(Status, STATUS_OBJECT_PATH_NOT_FOUND);
	PrintStatus(Status, STATUS_OBJECT_PATH_SYNTAX_BAD);
	PrintStatus(Status, STATUS_ACCESS_DENIED);
	PrintStatus(Status, STATUS_ACCESS_VIOLATION);
	PrintStatus(Status, STATUS_INVALID_PARAMETER);
	PrintStatus(Status, STATUS_INVALID_USER_BUFFER);
	PrintStatus(Status, STATUS_INVALID_HANDLE);
}



VOID
DokanNotifyReportChange0(
	_In_ PDokanFCB			Fcb,
	_In_ PUNICODE_STRING	FileName,
	_In_ ULONG				FilterMatch,
	_In_ ULONG				Action)
{
	USHORT	nameOffset;

	DDbgPrint("==> DokanNotifyReportChange %wZ\n", FileName);

	ASSERT(Fcb != NULL);
	ASSERT(FileName != NULL);

	// search the last "\"
	nameOffset = (USHORT)(FileName->Length/sizeof(WCHAR)-1);
	for(; FileName->Buffer[nameOffset] != L'\\'; --nameOffset)
		;
	nameOffset++; // the next is the begining of filename

	nameOffset *= sizeof(WCHAR); // Offset is in bytes

	FsRtlNotifyFullReportChange(
		Fcb->Vcb->NotifySync,
		&Fcb->Vcb->DirNotifyList,
		(PSTRING)FileName,
		nameOffset,
		NULL, // StreamName
		NULL, // NormalizedParentName
		FilterMatch,
		Action,
		NULL); // TargetContext

	DDbgPrint("<== DokanNotifyReportChange\n");
}


VOID
DokanNotifyReportChange(
	_In_ PDokanFCB	Fcb,
	_In_ ULONG		FilterMatch,
	_In_ ULONG		Action)
{
	ASSERT(Fcb != NULL);
	DokanNotifyReportChange0(Fcb, &Fcb->FileName, FilterMatch, Action);
}


VOID
PrintIdType(
	_In_ VOID* Id)
{
	if (Id == NULL) {
		DDbgPrint("    IdType = NULL\n");
		return;
	}
	switch (GetIdentifierType(Id)) {
	case DGL:
		DDbgPrint("    IdType = DGL\n");
		break;
	case DCB:
		DDbgPrint("   IdType = DCB\n");
		break;
	case VCB:
		DDbgPrint("   IdType = VCB\n");
		break;
	case FCB:
		DDbgPrint("   IdType = FCB\n");
		break;
	case CCB:
		DDbgPrint("   IdType = CCB\n");
		break;
	default:
		DDbgPrint("   IdType = Unknown\n");
		break;
	}
}


BOOLEAN
DokanCheckCCB(
	_In_ PDokanDCB	Dcb,
	_In_opt_ PDokanCCB	Ccb)
{
	if (GetIdentifierType(Dcb) != DCB) {
		PrintIdType(Dcb);
		return FALSE;
	}

	if (Ccb == NULL) {
		PrintIdType(Dcb);
		DDbgPrint("   ccb is NULL\n");
		return FALSE;
	}

	if (Ccb->MountId != Dcb->MountId) {
		DDbgPrint("   MountId is different\n");
		return FALSE;
	}

	if (!Dcb->Mounted) {
		DDbgPrint("  Not mounted\n");
		return FALSE;
	}

	return TRUE;
}

_Success_(return)
BOOLEAN DokanGetDispatchContext(
	_In_opt_ PFILE_OBJECT FileObject,
	_Outptr_result_nullonfailure_ PDokanCCB *Ccb,
	_Outptr_opt_result_nullonfailure_ PDokanFCB *Fcb
)
{
	PDokanCCB ccb = NULL;
	PDokanFCB fcb = NULL;
	*Ccb = ccb;
	if (Fcb != NULL)
	{

		*Fcb = fcb;
	}

	if (FileObject == NULL)
	{
		DDbgPrint("  FileObject == NULL\n");
		return FALSE;
	}

	ccb = FileObject->FsContext2;
	if (ccb == NULL || GetIdentifierType(ccb) != CCB)
	{
		DDbgPrint("  Invalid CCB\n");
		return FALSE;
	}

	if (Fcb != NULL)
	{
		fcb = ccb->Fcb;
		if (fcb == NULL || GetIdentifierType(fcb) != FCB)
		{
			DDbgPrint("  Invalid FCB\n");
			return FALSE;
		}
		*Fcb = fcb;
	}
	*Ccb = ccb;

	return TRUE;
}

_Success_(return)
BOOLEAN DokanGetDispatchParameters(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_opt_ PFILE_OBJECT FileObject,
	_Outptr_result_nullonfailure_ PDokanVCB *Vcb,
	_Outptr_result_nullonfailure_ PDokanCCB *Ccb,
	_Outptr_opt_result_nullonfailure_ PDokanFCB *Fcb
)
{
	PDokanVCB vcb = NULL;
	PDokanCCB ccb = NULL;
	PDokanFCB fcb = NULL;
	*Vcb = vcb;
	*Ccb = ccb;
	if (Fcb != NULL)
	{
		*Fcb = fcb;
		if (!DokanGetDispatchContext(FileObject, &ccb, &fcb))
		{
			return FALSE;
		}
	}
	else if (!DokanGetDispatchContext(FileObject, &ccb, NULL))
	{
		return FALSE;
	}

	vcb = DeviceObject->DeviceExtension;
	if (vcb == NULL || GetIdentifierType(vcb) != VCB)
	{
		DDbgPrint("  Invalid VCB\n");
		return FALSE;
	}

	if (!DokanCheckCCB(vcb->Dcb, ccb))
	{
		DDbgPrint("  Invalid CCB\n");
		return FALSE;
	}

	*Vcb = vcb;
	*Ccb = ccb;
	if (Fcb != NULL)
	{
		*Fcb = fcb;
	}

	return TRUE;
}


NTSTATUS
DokanAllocateMdl(
	_In_ PIRP	Irp,
	_In_ ULONG	Length
	)
{
	if (Irp->MdlAddress == NULL) {
		Irp->MdlAddress = IoAllocateMdl(Irp->UserBuffer, Length, FALSE, FALSE, Irp);

		if (Irp->MdlAddress == NULL) {
			DDbgPrint("    IoAllocateMdl returned NULL\n");
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		__try {
			MmProbeAndLockPages(Irp->MdlAddress, Irp->RequestorMode, IoWriteAccess);

		} __except (EXCEPTION_EXECUTE_HANDLER) {
			DDbgPrint("    MmProveAndLockPages error\n");
			IoFreeMdl(Irp->MdlAddress);
			Irp->MdlAddress = NULL;
			return STATUS_INSUFFICIENT_RESOURCES;
		}
	}
	return STATUS_SUCCESS;
}


VOID
DokanFreeMdl(
	_In_ PIRP	Irp
	)
{
	if (Irp->MdlAddress != NULL) {
		MmUnlockPages(Irp->MdlAddress);
		IoFreeMdl(Irp->MdlAddress);
		Irp->MdlAddress = NULL;
	}
}
