#include "driver.h"
#define DEVICE_NAME L"\\Device\\MyDevice"
#define LINK_NAME L"\\??\\test" // \\\\.\\test

#define OPER1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER2 CTL_CODE(FILE_DEVICE_UNKNOWN,0x900,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER3 CTL_CODE(FILE_DEVICE_UNKNOWN,0xA00,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER4 CTL_CODE(FILE_DEVICE_UNKNOWN,0xB00,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER5 CTL_CODE(FILE_DEVICE_UNKNOWN,0xC00,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER6 CTL_CODE(FILE_DEVICE_UNKNOWN,0xD00,METHOD_BUFFERED,FILE_ANY_ACCESS)

VOID UnDriver(PDRIVER_OBJECT driver)
{
	DbgPrint(("Uninstall Driver Is OK \n"));
}

NTSTATUS IrpCreateProc(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("IrpCreateProc...\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS IrpCloseProc(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("IrpCloseProc...\n");
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS IrpDeviceControlProc(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	//�����Լ���ҵ��...
	NTSTATUS nStatus = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInLength;
	ULONG uOutLength;
	ULONG uRead;
	ULONG uWrite;

	DbgPrint("IrpDeviceConrolProc...\n");

	// ������ʱ������ֵ
	uRead = 0;
	uWrite = 0x12345678;
	// ��ȡIRP����
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	// ��ȡ������
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	// ��ȡ��������ַ�����������ͬһ����
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	// Ring3 �������ݵĳ���
	uInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	// Ring0 �������ݵĳ���
	uOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	switch (uIoControlCode)
	{
	case OPER1:
	{
		InterceptProcess(pDevObj, pIrp);
		pIrp->IoStatus.Information = 0;
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER2:
	{
	
		
		// ��ȡ������
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uRead: %x\n", uRead);
		// д�뻺����
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uWrite: %x\n", uWrite);
		// ����״̬
		pIrp->IoStatus.Information = 2; // �������ֽ�
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER3:
	{
		DbgPrint("IrpDeviceControlProc -> OPER3 �����ֽ���: %d\n", uInLength);
		DbgPrint("IrpDeviceControlProc -> OPER3 ����ֽ���: %d\n", uOutLength);
		// ��ȡ������
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceControlProc -> OPER3 uRead: %x\n", uRead);
		// д�뻺����
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER3	uWrite: %x\n", uWrite);
		// ����״̬
		InterDLL(pDevObj, pIrp);

		pIrp->IoStatus.Information = 2; // �������ֽ�
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER4:
	{
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uRead: %x\n", uRead);
		// д�뻺����
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uWrite: %x\n", uWrite);
		// ����״̬
		pIrp->IoStatus.Information = 2; // �������ֽ�
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER5:
	{
		
		
		EndProcess(pDevObj, pIrp, pIoBuffer);
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uWrite: %x\n", uWrite);
		// ����״̬
		pIrp->IoStatus.Information = 2; // �������ֽ�
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER6:
	{
		UnDriver(pDevObj, pIrp);
	}
	default:
		pIrp->IoStatus.Information = 0;
		break;
	}

	//���÷���״̬
	pIrp->IoStatus.Status = STATUS_SUCCESS;	//  getlasterror()�õ��ľ������ֵ
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

PDRIVER_OBJECT pDriverObject;
NTSTATUS test01()
{
	UNICODE_STRING DeviceName;
	UNICODE_STRING SymbolicLinkName;
	PDEVICE_OBJECT pDeviceObject = NULL;
	NTSTATUS nStatus;

	// �����豸
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	nStatus = IoCreateDevice(
		pDriverObject,				//��ǰ�豸��������������
		0,
		&DeviceName,			//�豸���������
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pDeviceObject			//�豸����ָ��
	);

	if (nStatus != STATUS_SUCCESS)
	{
		DbgPrint("IoCreateDevice Failed\n");
		return -1;
	}

	// �������ݷ�ʽ
	pDeviceObject->Flags |= DO_BUFFERED_IO;

	// ����������������
	RtlInitUnicodeString(&SymbolicLinkName, LINK_NAME);
	nStatus = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);
	if (nStatus != STATUS_SUCCESS)
	{
		DbgPrint("IoCreateSymbolicLink Failed\n");
		return -1;
	}

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = IrpCreateProc;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IrpCloseProc;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpDeviceControlProc;
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, PUNICODE_STRING RegistryPath)
{
	pDriverObject = Driver;
	DbgPrint(("hello zpchcbd \n"));
	Driver->DriverUnload = UnDriver;
	test01();
	return STATUS_SUCCESS;
}
