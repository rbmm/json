#include "stdafx.h"

_NT_BEGIN

NTSTATUS ReadFromFile(_In_ HANDLE hFile, _Out_ PVOID* ppb, _Out_ ULONG* pcb, _In_ ULONG _cb = 0, _In_ ULONG cb_ = 0)
{
	NTSTATUS status;
	FILE_STANDARD_INFORMATION fsi;
	IO_STATUS_BLOCK iosb;

	if (0 <= (status = NtQueryInformationFile(hFile, &iosb, &fsi, sizeof(fsi), FileStandardInformation)))
	{
		if (fsi.EndOfFile.QuadPart > 0x1000000)
		{
			status = STATUS_FILE_TOO_LARGE;
		}
		else
		{
			if (PVOID pb = new UCHAR[_cb + fsi.EndOfFile.LowPart + cb_])
			{
				if (0 > (status = NtReadFile(hFile, 0, 0, 0, &iosb, (PBYTE)pb + _cb, fsi.EndOfFile.LowPart, 0, 0)))
				{
					delete [] pb;
				}
				else
				{
					*ppb = pb;
					*pcb = (ULONG)iosb.Information;
				}
			}
			else
			{
				status = STATUS_NO_MEMORY;
			}
		}
	}

	return status;
}

NTSTATUS ReadFromFile(_In_ PCWSTR lpFileName, _Out_ PVOID* ppb, _Out_ ULONG* pcb, _In_ ULONG _cb = 0, _In_ ULONG cb_ = 0)
{
	UNICODE_STRING ObjectName;
	OBJECT_ATTRIBUTES oa = { sizeof(oa), 0, &ObjectName, OBJ_CASE_INSENSITIVE };

	NTSTATUS status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &ObjectName, 0, 0);

	if (0 <= status)
	{
		HANDLE hFile;
		IO_STATUS_BLOCK iosb;

		status = NtOpenFile(&hFile, FILE_GENERIC_READ, &oa, &iosb,
			FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

		RtlFreeUnicodeString(&ObjectName);

		if (0 <= status)
		{
			status = ReadFromFile(hFile, ppb, pcb, _cb, cb_);
			NtClose(hFile);
		}
	}

	return status;
}

#include "print.h"

void TestJson(PCWSTR pcsz);
void simplydemo();

void WINAPI ep(PWSTR psz )
{
	{
		PrintInfo pi;
		InitPrintf();
		if (PWSTR pc = wcschr(GetCommandLineW(), '*'))
		{
			do 
			{
				pc = wcschr(psz = ++pc, '*');
				if (pc) *pc = 0;
				TestJson(psz);
			} while (pc);
		}
		else
		{
			DbgPrint("usage: *file.json[*file.json]\r\n\r\n");
			simplydemo();
		}
	}

	ExitProcess(0);
}

_NT_END