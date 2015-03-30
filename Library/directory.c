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


#include "dokani.h"
#include "fileinfo.h"
#include "list.h"

#if _MSC_VER < 1300 // VC6
typedef ULONG ULONG_PTR;
#endif

typedef struct _DOKAN_FIND_DATA {
	WIN32_FIND_DATAW	FindData;
	LIST_ENTRY			ListEntry;
} DOKAN_FIND_DATA, *PDOKAN_FIND_DATA;


VOID
DokanFillDirInfo(
	_Inout_ PFILE_DIRECTORY_INFORMATION	Buffer,
	_In_ PWIN32_FIND_DATAW				FindData,
	_In_ ULONG							Index)
{
	ULONG nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	Buffer->FileIndex = Index;
	Buffer->FileAttributes = FindData->dwFileAttributes;
	Buffer->FileNameLength = nameBytes;

	Buffer->EndOfFile.HighPart = FindData->nFileSizeHigh;
	Buffer->EndOfFile.LowPart   = FindData->nFileSizeLow;
	Buffer->AllocationSize.HighPart = FindData->nFileSizeHigh;
	Buffer->AllocationSize.LowPart = FindData->nFileSizeLow;

	Buffer->CreationTime.HighPart = FindData->ftCreationTime.dwHighDateTime;
	Buffer->CreationTime.LowPart  = FindData->ftCreationTime.dwLowDateTime;

	Buffer->LastAccessTime.HighPart = FindData->ftLastAccessTime.dwHighDateTime;
	Buffer->LastAccessTime.LowPart  = FindData->ftLastAccessTime.dwLowDateTime;

	Buffer->LastWriteTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->LastWriteTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->ChangeTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->ChangeTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	RtlCopyMemory(Buffer->FileName, FindData->cFileName, nameBytes);
}


VOID
DokanFillFullDirInfo(
	_Inout_ PFILE_FULL_DIR_INFORMATION	Buffer,
	_In_ PWIN32_FIND_DATAW				FindData,
	_In_ ULONG							Index)
{
	ULONG nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	Buffer->FileIndex = Index;
	Buffer->FileAttributes = FindData->dwFileAttributes;
	Buffer->FileNameLength = nameBytes;

	Buffer->EndOfFile.HighPart = FindData->nFileSizeHigh;
	Buffer->EndOfFile.LowPart   = FindData->nFileSizeLow;
	Buffer->AllocationSize.HighPart = FindData->nFileSizeHigh;
	Buffer->AllocationSize.LowPart = FindData->nFileSizeLow;

	Buffer->CreationTime.HighPart = FindData->ftCreationTime.dwHighDateTime;
	Buffer->CreationTime.LowPart  = FindData->ftCreationTime.dwLowDateTime;

	Buffer->LastAccessTime.HighPart = FindData->ftLastAccessTime.dwHighDateTime;
	Buffer->LastAccessTime.LowPart  = FindData->ftLastAccessTime.dwLowDateTime;

	Buffer->LastWriteTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->LastWriteTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->ChangeTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->ChangeTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->EaSize = 0;

	RtlCopyMemory(Buffer->FileName, FindData->cFileName, nameBytes);
}



VOID
DokanFillIdFullDirInfo(
	_Inout_ PFILE_ID_FULL_DIR_INFORMATION	Buffer,
	_In_ PWIN32_FIND_DATAW					FindData,
	_In_ ULONG								Index)
{
	ULONG nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	Buffer->FileIndex = Index;
	Buffer->FileAttributes = FindData->dwFileAttributes;
	Buffer->FileNameLength = nameBytes;

	Buffer->EndOfFile.HighPart = FindData->nFileSizeHigh;
	Buffer->EndOfFile.LowPart   = FindData->nFileSizeLow;
	Buffer->AllocationSize.HighPart = FindData->nFileSizeHigh;
	Buffer->AllocationSize.LowPart = FindData->nFileSizeLow;

	Buffer->CreationTime.HighPart = FindData->ftCreationTime.dwHighDateTime;
	Buffer->CreationTime.LowPart  = FindData->ftCreationTime.dwLowDateTime;

	Buffer->LastAccessTime.HighPart = FindData->ftLastAccessTime.dwHighDateTime;
	Buffer->LastAccessTime.LowPart  = FindData->ftLastAccessTime.dwLowDateTime;

	Buffer->LastWriteTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->LastWriteTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->ChangeTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->ChangeTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->EaSize = 0;
	Buffer->FileId.QuadPart = 0;

	RtlCopyMemory(Buffer->FileName, FindData->cFileName, nameBytes);
}


VOID
DokanFillIdBothDirInfo(
	_Inout_ PFILE_ID_BOTH_DIR_INFORMATION	Buffer,
	_In_ PWIN32_FIND_DATAW					FindData,
	_In_ ULONG								Index)
{
	ULONG nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	Buffer->FileIndex = Index;
	Buffer->FileAttributes = FindData->dwFileAttributes;
	Buffer->FileNameLength = nameBytes;
	Buffer->ShortNameLength = 0;

	Buffer->EndOfFile.HighPart = FindData->nFileSizeHigh;
	Buffer->EndOfFile.LowPart   = FindData->nFileSizeLow;
	Buffer->AllocationSize.HighPart = FindData->nFileSizeHigh;
	Buffer->AllocationSize.LowPart = FindData->nFileSizeLow;

	Buffer->CreationTime.HighPart = FindData->ftCreationTime.dwHighDateTime;
	Buffer->CreationTime.LowPart  = FindData->ftCreationTime.dwLowDateTime;

	Buffer->LastAccessTime.HighPart = FindData->ftLastAccessTime.dwHighDateTime;
	Buffer->LastAccessTime.LowPart  = FindData->ftLastAccessTime.dwLowDateTime;

	Buffer->LastWriteTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->LastWriteTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->ChangeTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->ChangeTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->EaSize = 0;
	Buffer->FileId.QuadPart = 0;

	RtlCopyMemory(Buffer->FileName, FindData->cFileName, nameBytes);
}


VOID
DokanFillBothDirInfo(
	_Inout_ PFILE_BOTH_DIR_INFORMATION	Buffer,
	_In_ PWIN32_FIND_DATAW				FindData,
	_In_ ULONG							Index)
{
	ULONG nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	Buffer->FileIndex = Index;
	Buffer->FileAttributes = FindData->dwFileAttributes;
	Buffer->FileNameLength = nameBytes;
	Buffer->ShortNameLength = 0;

	Buffer->EndOfFile.HighPart = FindData->nFileSizeHigh;
	Buffer->EndOfFile.LowPart   = FindData->nFileSizeLow;
	Buffer->AllocationSize.HighPart = FindData->nFileSizeHigh;
	Buffer->AllocationSize.LowPart = FindData->nFileSizeLow;

	Buffer->CreationTime.HighPart = FindData->ftCreationTime.dwHighDateTime;
	Buffer->CreationTime.LowPart  = FindData->ftCreationTime.dwLowDateTime;

	Buffer->LastAccessTime.HighPart = FindData->ftLastAccessTime.dwHighDateTime;
	Buffer->LastAccessTime.LowPart  = FindData->ftLastAccessTime.dwLowDateTime;

	Buffer->LastWriteTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->LastWriteTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->ChangeTime.HighPart = FindData->ftLastWriteTime.dwHighDateTime;
	Buffer->ChangeTime.LowPart  = FindData->ftLastWriteTime.dwLowDateTime;

	Buffer->EaSize = 0;

	RtlCopyMemory(Buffer->FileName, FindData->cFileName, nameBytes);
}


VOID
DokanFillNamesInfo(
	_Inout_ PFILE_NAMES_INFORMATION	Buffer,
	_In_ PWIN32_FIND_DATAW			FindData,
	_In_ ULONG						Index)
{
	ULONG nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	Buffer->FileIndex = Index;
	Buffer->FileNameLength = nameBytes;

	RtlCopyMemory(Buffer->FileName, FindData->cFileName, nameBytes);
}

ULONG
DokanFillDirectoryInformation(
	_In_ FILE_INFORMATION_CLASS	DirectoryInfo,
	_Inout_ PVOID				Buffer,
	_Inout_ PULONG				LengthRemaining,
	_In_ PWIN32_FIND_DATAW		FindData,
	_In_ ULONG					Index)
{
	ULONG	nameBytes;
	ULONG	thisEntrySize;
	
	nameBytes = wcslen(FindData->cFileName) * sizeof(WCHAR);

	thisEntrySize = nameBytes;

	switch (DirectoryInfo) {
	case FileDirectoryInformation:
		thisEntrySize += sizeof(FILE_DIRECTORY_INFORMATION);
		break;
	case FileFullDirectoryInformation:
		thisEntrySize += sizeof(FILE_FULL_DIR_INFORMATION);
		break;
	case FileNamesInformation:
		thisEntrySize += sizeof(FILE_NAMES_INFORMATION);
		break;
	case FileBothDirectoryInformation:
		thisEntrySize += sizeof(FILE_BOTH_DIR_INFORMATION);
		break;
	case FileIdBothDirectoryInformation:
		thisEntrySize += sizeof(FILE_ID_BOTH_DIR_INFORMATION);
		break;
	default:
		break;
	}

	// Must be align on a 8-byte boundary.
	thisEntrySize = QuadAlign(thisEntrySize);

	// no more memory, don't fill any more
	if (*LengthRemaining < thisEntrySize) {
		DbgPrint("  no memory\n");
		return 0;
	}

	RtlZeroMemory(Buffer, thisEntrySize);

	switch (DirectoryInfo) {
	case FileDirectoryInformation:
		DokanFillDirInfo(Buffer, FindData, Index);
		break;
	case FileFullDirectoryInformation:
		DokanFillFullDirInfo(Buffer, FindData, Index);
		break;
	case FileNamesInformation:
		DokanFillNamesInfo(Buffer, FindData, Index);
		break;
	case FileBothDirectoryInformation:
		DokanFillBothDirInfo(Buffer, FindData, Index);
		break;
	case FileIdBothDirectoryInformation:
		DokanFillIdBothDirInfo(Buffer, FindData, Index);
		break;
	default:
		break;
	}

	*LengthRemaining -= thisEntrySize;

	return thisEntrySize;
}





int WINAPI
DokanFillFileData(
	_In_ PWIN32_FIND_DATAW	FindData,
	_In_ PDOKAN_FILE_INFO	FileInfo)
{
	PLIST_ENTRY listHead = ((PDOKAN_OPEN_INFO)FileInfo->DokanContext)->DirListHead;
	PDOKAN_FIND_DATA	findData;

	findData = malloc(sizeof(DOKAN_FIND_DATA));
	if (findData != NULL)
	{
		ZeroMemory(findData, sizeof(DOKAN_FIND_DATA));
		InitializeListHead(&findData->ListEntry);

		findData->FindData = *FindData;

		InsertTailList(listHead, &findData->ListEntry);
	}
	return 0;
}



VOID
ClearFindData(
  _In_opt_ PLIST_ENTRY	ListHead)
{
	// free all list entries
	while(!IsListEmpty(ListHead) && ListHead != NULL) {
		PLIST_ENTRY entry = RemoveHeadList(ListHead);
		PDOKAN_FIND_DATA find = CONTAINING_RECORD(entry, DOKAN_FIND_DATA, ListEntry);
		free(find);
	}
}



// add entry which matches the pattern specifed in EventContext
// to the buffer specifed in EventInfo
//
LONG
MatchFiles(
	_In_ PEVENT_CONTEXT			EventContext,
	_In_ PEVENT_INFORMATION		EventInfo,
	_In_ PLIST_ENTRY			FindDataList,
	_In_ BOOLEAN				PatternCheck)
{
	PLIST_ENTRY	thisEntry, listHead, nextEntry;

	ULONG	lengthRemaining = EventInfo->BufferLength;
	PVOID	currentBuffer	= EventInfo->Buffer;
	PVOID	lastBuffer		= currentBuffer;
	ULONG	index = 0;

	PWCHAR pattern = NULL;
	
	// search patten is specified
	if (PatternCheck && EventContext->Directory.SearchPatternLength != 0) {
		pattern = (PWCHAR)((SIZE_T)&EventContext->Directory.SearchPatternBase[0]
					+ (SIZE_T)EventContext->Directory.SearchPatternOffset);
	}

	listHead = FindDataList;

    for(thisEntry = listHead->Flink;
		thisEntry != listHead;
		thisEntry = nextEntry) {
        
		PDOKAN_FIND_DATA	find;
		nextEntry = thisEntry->Flink;

		find = CONTAINING_RECORD(thisEntry, DOKAN_FIND_DATA, ListEntry);

		DbgPrintW(L"FileMatch? : %s (%s,%d,%d)\n", find->FindData.cFileName,
			(pattern ? pattern : L"null"),
			EventContext->Directory.FileIndex, index);

		// pattern is not specified or pattern match is ignore cases
		if (!pattern || DokanIsNameInExpression(pattern, find->FindData.cFileName, TRUE)) {
			
			if(EventContext->Directory.FileIndex <= index) {
				// index+1 is very important, should use next entry index
				ULONG entrySize = DokanFillDirectoryInformation(
									EventContext->Directory.FileInformationClass,
									currentBuffer, &lengthRemaining, &find->FindData, index+1);
				// buffer is full
				if (entrySize == 0)
					break;
			
				// pointer of the current last entry
				lastBuffer = currentBuffer;

				// end if needs to return single entry
				if (EventContext->Flags & SL_RETURN_SINGLE_ENTRY) {
					DbgPrint("  =>return single entry\n");
					index++;
					break;
				}

				DbgPrint("  =>return\n");

				// the offset of next entry
				((PFILE_BOTH_DIR_INFORMATION)currentBuffer)->NextEntryOffset = entrySize;

				// next buffer position
				(PCHAR)currentBuffer += entrySize;
			}
			index++;
		}
	}

	// Since next of the last entry doesn't exist, clear next offset
	((PFILE_BOTH_DIR_INFORMATION)lastBuffer)->NextEntryOffset = 0;

	// acctualy used length of buffer
	EventInfo->BufferLength = EventContext->Directory.BufferLength - lengthRemaining;

	// NO_MORE_FILES
	if (index <= EventContext->Directory.FileIndex)
		return -1;

	return index;
}



VOID
DispatchDirectoryInformation(
	_In_ HANDLE				Handle,
	_In_ PEVENT_CONTEXT		EventContext,
	_In_ PDOKAN_INSTANCE		DokanInstance)
{
	PEVENT_INFORMATION	eventInfo;
	DOKAN_FILE_INFO		fileInfo;
	PDOKAN_OPEN_INFO	openInfo;
	int					status = 0;
	ULONG				fileInfoClass = EventContext->Directory.FileInformationClass;
	ULONG				sizeOfEventInfo = sizeof(EVENT_INFORMATION) + EventContext->Directory.BufferLength;

	BOOLEAN				patternCheck = TRUE;

	CheckFileName(EventContext->Directory.DirectoryName);

	eventInfo = DispatchCommon(
		EventContext, sizeOfEventInfo, DokanInstance, &fileInfo, &openInfo);

	if (eventInfo != NULL)
	{
		// check whether this is handled FileInfoClass
		if (fileInfoClass != FileDirectoryInformation &&
			fileInfoClass != FileFullDirectoryInformation &&
			fileInfoClass != FileNamesInformation &&
			fileInfoClass != FileIdBothDirectoryInformation &&
			fileInfoClass != FileBothDirectoryInformation) {
		
			DbgPrint("not suported type %d\n", fileInfoClass);

			// send directory info to driver
			eventInfo->BufferLength = 0;
			eventInfo->Status = STATUS_NOT_IMPLEMENTED;
			SendEventInformation(Handle, eventInfo, sizeOfEventInfo, DokanInstance);
			free(eventInfo);
			return;
		}

		// IMPORTANT!!
		// this buffer length is fixed in MatchFiles funciton
		eventInfo->BufferLength = EventContext->Directory.BufferLength;

		if (openInfo->DirListHead == NULL) {
			openInfo->DirListHead = malloc(sizeof(LIST_ENTRY));
			if (openInfo->DirListHead != NULL)
			{
				InitializeListHead(openInfo->DirListHead);
			}
			else
			{
				DbgPrint("###FindFiles: could not allocate dirListHead\n");
				eventInfo->BufferLength = 0;
				eventInfo->Status = STATUS_BUFFER_OVERFLOW;
				SendEventInformation(Handle, eventInfo, sizeOfEventInfo, DokanInstance);
				free(eventInfo);
				return;
			}
		}

		if (EventContext->Directory.FileIndex == 0) {
			ClearFindData(openInfo->DirListHead);
		}

		if (IsListEmpty(openInfo->DirListHead)) {

			DbgPrint("###FindFiles %04d\n", openInfo->EventId);

			// if user defined FindFilesWithPattern
			if (DokanInstance->DokanOperations->FindFilesWithPattern) {
				LPCWSTR	pattern = L"*";

				// if search pattern is specified
				if (EventContext->Directory.SearchPatternLength != 0) {
					pattern = (PWCHAR)((SIZE_T)&EventContext->Directory.SearchPatternBase[0]
						+ (SIZE_T)EventContext->Directory.SearchPatternOffset);
				}

				patternCheck = FALSE; // do not recheck pattern later in MatchFiles

				status = DokanInstance->DokanOperations->FindFilesWithPattern(
					EventContext->Directory.DirectoryName,
					pattern,
					DokanFillFileData,
					&fileInfo);

			}
			else if (DokanInstance->DokanOperations->FindFiles) {

				patternCheck = TRUE; // do pattern check later in MachFiles

				// call FileSystem specifeid callback routine
				status = DokanInstance->DokanOperations->FindFiles(
					EventContext->Directory.DirectoryName,
					DokanFillFileData,
					&fileInfo);
			}
			else {
				status = -1;
			}
		}



		if (status < 0) {

			if (EventContext->Directory.FileIndex == 0) {
				DbgPrint("  STATUS_NO_SUCH_FILE\n");
				eventInfo->Status = STATUS_NO_SUCH_FILE;
			}
			else {
				DbgPrint("  STATUS_NO_MORE_FILES\n");
				eventInfo->Status = STATUS_NO_MORE_FILES;
			}

			eventInfo->BufferLength = 0;
			eventInfo->Directory.Index = EventContext->Directory.FileIndex;
			// free all of list entries
			ClearFindData(openInfo->DirListHead);
		}
		else {
			LONG	index;
			eventInfo->Status = STATUS_SUCCESS;

			DbgPrint("index from %d\n", EventContext->Directory.FileIndex);
			// extract entries that match search pattern from FindFiles result
			index = MatchFiles(EventContext, eventInfo, openInfo->DirListHead, patternCheck);

			// there is no matched file
			if (index < 0) {
				if (EventContext->Directory.FileIndex == 0) {
					DbgPrint("  STATUS_NO_SUCH_FILE\n");
					eventInfo->Status = STATUS_NO_SUCH_FILE;
				}
				else {
					DbgPrint("  STATUS_NO_MORE_FILES\n");
					eventInfo->Status = STATUS_NO_MORE_FILES;
				}
				eventInfo->BufferLength = 0;
				eventInfo->Directory.Index = EventContext->Directory.FileIndex;

				ClearFindData(openInfo->DirListHead);

			}
			else {
				DbgPrint("index to %d\n", index);
				eventInfo->Directory.Index = index;
			}

		}

		// information for FileSystem
		openInfo->UserContext = fileInfo.Context;

		// send directory information to driver
		SendEventInformation(Handle, eventInfo, sizeOfEventInfo, DokanInstance);
		free(eventInfo);
	}
	else
	{
		EVENT_INFORMATION failureEventInfo;
		DbgPrint("###FindFiles: could not allocate eventInfo\n");
		SetupFailureEventInformation(EventContext, DokanInstance, &failureEventInfo);
		SendEventInformation(Handle, &failureEventInfo, sizeof(EVENT_INFORMATION), DokanInstance);
	}
	return;
}



#define DOS_STAR                        (L'<')
#define DOS_QM                          (L'>')
#define DOS_DOT                         (L'"')

// check whether Name matches Expression
// Expression can contain "?"(any one character) and "*" (any string)
// when IgnoreCase is TRUE, do case insenstive matching
//
// http://msdn.microsoft.com/en-us/library/ff546850(v=VS.85).aspx
// * (asterisk) Matches zero or more characters.
// ? (question mark) Matches a single character.
// DOS_DOT Matches either a period or zero characters beyond the name string.
// DOS_QM Matches any single character or, upon encountering a period or end
//        of name string, advances the expression to the end of the set of
//        contiguous DOS_QMs.
// DOS_STAR Matches zero or more characters until encountering and matching
//          the final . in the name.
BOOL DOKANAPI
DokanIsNameInExpression(
	_In_ LPCWSTR		Expression, // matching pattern
	_In_ LPCWSTR		Name, // file name
	_In_ BOOL			IgnoreCase)
{
	ULONG ei = 0;
	ULONG ni = 0;

	while (Expression[ei] != '\0') {

		if (Expression[ei] == L'*') {
			ei++;
			if (Expression[ei] == '\0')
				return TRUE;

			while (Name[ni] != '\0') {
				if (DokanIsNameInExpression(&Expression[ei], &Name[ni], IgnoreCase))
					return TRUE;
				ni++;
			}

		} else if (Expression[ei] == DOS_STAR) {

			ULONG p = ni;
			ULONG lastDot = 0;
			ei++;
			
			while (Name[p] != '\0') {
				if (Name[p] == L'.')
					lastDot = p;
				p++;
			}
			

			while (TRUE) {
				if (Name[ni] == '\0' || ni == lastDot)
					break;

				if (DokanIsNameInExpression(&Expression[ei], &Name[ni], IgnoreCase))
					return TRUE;
				ni++;
			}
			
		} else if (Expression[ei] == DOS_QM)  {
			
			ei++;
			if (Name[ni] != L'.') {
				ni++;
			} else {

				ULONG p = ni + 1;
				while (Name[p] != '\0') {
					if (Name[p] == L'.')
						break;
					p++;
				}

				if (Name[p] == L'.')
					ni++;
			}

		} else if (Expression[ei] == DOS_DOT) {
			ei++;

			if (Name[ni] == L'.')
				ni++;

		} else {
			if (Expression[ei] == L'?') {
				ei++; ni++;
			} else if(IgnoreCase && towupper(Expression[ei]) == towupper(Name[ni])) {
				ei++; ni++;
			} else if(!IgnoreCase && Expression[ei] == Name[ni]) {
				ei++; ni++;
			} else {
				return FALSE;
			}
		}
	}

	if (ei == wcslen(Expression) && ni == wcslen(Name))
		return TRUE;
	

	return FALSE;
}


