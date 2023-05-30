#pragma once
#include <ntddk.h>
#include<string.h>
UCHAR* PsGetProcessImageFileName(IN PEPROCESS process);
void PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);
NTKERNELAPI PCHAR PsGetProcessImageFileNames(PEPROCESS Process);
NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
NTKERNELAPI UCHAR* PsGetProcessImageFileNamea(IN PEPROCESS Process);
// �ƹ�ǩ�����
BOOLEAN BypassCheckSign(PDRIVER_OBJECT pDriverObject)
{
#ifdef _WIN64
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG64 __Undefined1;
		ULONG64 __Undefined2;
		ULONG64 __Undefined3;
		ULONG64 NonPagedDebugInfo;
		ULONG64 DllBase;
		ULONG64 EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
		USHORT  LoadCount;
		USHORT  __Undefined5;
		ULONG64 __Undefined6;
		ULONG   CheckSum;
		ULONG   __padding1;
		ULONG   TimeDateStamp;
		ULONG   __padding2;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#else
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG unknown1;
		ULONG unknown2;
		ULONG unknown3;
		ULONG unknown4;
		ULONG unknown5;
		ULONG unknown6;
		ULONG unknown7;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
	} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
#endif

	PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	pLdrData->Flags = pLdrData->Flags | 0x20;

	return TRUE;
}
// ���ݽ���ID���ؽ���EPROCESS�ṹ��,ʧ�ܷ���NULL
PEPROCESS GetProcessNameByProcessId(HANDLE pid)
{
	PEPROCESS ProcessObj = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	Status = PsLookupProcessByProcessId(pid, &ProcessObj);
	if (NT_SUCCESS(Status))
		return ProcessObj;
	return NULL;
}

// ����ProcessName��ȡ�����̵�PID��
HANDLE GetPidByProcessName(char* ProcessName)
{
	PEPROCESS pCurrentEprocess = NULL;
	HANDLE pid = 0;
	for (int i = 0; i < 1000000000; i += 4)
	{
		pCurrentEprocess = GetProcessNameByProcessId((HANDLE)i);
		if (pCurrentEprocess != NULL)
		{
			pid = PsGetProcessId(pCurrentEprocess);
			if (strstr(PsGetProcessImageFileName(pCurrentEprocess), ProcessName) != NULL)
			{
				ObDereferenceObject(pCurrentEprocess);
				return pid;
			}
			ObDereferenceObject(pCurrentEprocess);
		}
	}
	return (HANDLE)-1;
}

// �����������,��ֹ���ý���
BOOLEAN KillProcess(PCHAR ProcessName)
{
	PEPROCESS pCurrentEprocess = NULL;
	HANDLE pid = 0;
	HANDLE Handle = NULL;
	OBJECT_ATTRIBUTES obj;
	CLIENT_ID cid = { 0 };
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	for (int i = 0; i < 10000000; i += 4)
	{
		pCurrentEprocess = GetProcessNameByProcessId((HANDLE)i);
		if (pCurrentEprocess != NULL)
		{
			pid = PsGetProcessId(pCurrentEprocess);

			// �жϵ�ǰ���������Ƿ�����Ҫ�����Ľ���
			if (strstr(PsGetProcessImageFileName(pCurrentEprocess), ProcessName) != NULL)
			{
				ObDereferenceObject(pCurrentEprocess);

				// �ҵ���ʼ����
				InitializeObjectAttributes(&obj, NULL, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
				cid.UniqueProcess = (HANDLE)pid;
				cid.UniqueThread = 0;

				// �򿪽���
				Status = ZwOpenProcess(&Handle, GENERIC_ALL, &obj, &cid);
				if (NT_SUCCESS(Status))
				{
					// ������ֹ�ź�
					ZwTerminateProcess(Handle, 0);
					ZwClose(Handle);
				}
				ZwClose(Handle);
				return TRUE;
			}
			ObDereferenceObject(pCurrentEprocess);
		}
	}
	return FALSE;
}



NTSTATUS EndProcess(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath,unsigned int pid)
{


	BOOLEAN Retn;
	Retn = KillProcess(pid);
	DbgPrint("����״̬: %d \n", Retn);

	
	return STATUS_SUCCESS;
}

PCHAR GetProcessNameByProcessId(HANDLE ProcessId)
{
	NTSTATUS st = STATUS_UNSUCCESSFUL;
	PEPROCESS ProcessObj = NULL;
	PCHAR string = NULL;
	st = PsLookupProcessByProcessId(ProcessId, &ProcessObj);
	if (NT_SUCCESS(st))
	{
		string = PsGetProcessImageFileName(ProcessObj);
		ObfDereferenceObject(ProcessObj);
	}
	return string;
}



// ���̻ص�����
VOID My_LyShark_Com_CreateProcessNotifyEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	char ProcName[16] = { 0 };
	if (CreateInfo != NULL)
	{
		strcpy_s(ProcName, 16, PsGetProcessImageFileName(Process));
		DbgPrint("[LyShark] ������ID: %ld | ��������: %s | ������: %s | ����·����%wZ \n", CreateInfo->ParentProcessId, GetProcessNameByProcessId(CreateInfo->ParentProcessId), PsGetProcessImageFileName(Process), CreateInfo->ImageFileName);

		// �ж��Ƿ�Ϊָ������
		if (0 == _stricmp(ProcName, "lyshark.exe"))
		{
			// ��ֹ��
			CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		strcpy_s(ProcName, 16, PsGetProcessImageFileName(Process));
		DbgPrint("[LyShark] ����[ %s ] �˳���, ���򱻹ر�", ProcName);
	}
}


NTSTATUS InterDLL(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;

	// �ƹ�ǩ�����
	// LINKER_FLAGS=/INTEGRITYCHECK
	BypassCheckSign(Driver);

	DbgPrint("hello lyshark.com \n");


	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)My_LyShark_Com_CreateProcessNotifyEx, FALSE);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[lyshark.com] �������̻ص�����");
	}
	return STATUS_SUCCESS;
}

NTSTATUS InterceptProcess(PDRIVER_OBJECT driver, PUNICODE_STRING reg_path)
{

	//�������½���ʱ,����PcreateProcessNotifyRoutineEx
	PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, FALSE);
	DbgPrint("hello: my salary!");


	return STATUS_SUCCESS;
}
void PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	if (!CreateInfo)
	{
		DbgPrint("���̽���id:%d..,��ǰ�߳�idΪ:\r\n", ProcessId, PsGetCurrentThreadId());
		return;
	}
	PCUNICODE_STRING ImageFileName = CreateInfo->ImageFileName;//��������
	HANDLE pUniqueProcess = CreateInfo->CreatingThreadId.UniqueProcess;//������id
	HANDLE pUniqueThread = CreateInfo->CreatingThreadId.UniqueThread;//������id

	DbgPrint("�߳�id:%d ��������id:%d ������id:%d �������߳�id:%d ��������:%wZ\n------:\r\n",
		PsGetCurrentThreadId(),
		ProcessId,
		pUniqueProcess,
		pUniqueThread,
		ImageFileName
	);

	//��ؽ���
	PCHAR imageName = PsGetProcessImageFileName(Process);

	
		CreateInfo->CreationStatus = STATUS_ACCESS_DENIED; //�ܾ�����
		DbgPrint("���̱����أ�%Wz", ImageFileName);


}