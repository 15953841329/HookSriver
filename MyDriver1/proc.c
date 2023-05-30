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
	//处理自己的业务...
	NTSTATUS nStatus = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;
	ULONG uInLength;
	ULONG uOutLength;
	ULONG uRead;
	ULONG uWrite;

	DbgPrint("IrpDeviceConrolProc...\n");

	// 设置临时变量的值
	uRead = 0;
	uWrite = 0x12345678;
	// 获取IRP数据
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	// 获取控制码
	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	// 获取缓冲区地址（输入输出是同一个）
	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
	// Ring3 发送数据的长度
	uInLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	// Ring0 发送数据的长度
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
	
		
		// 读取缓冲区
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uRead: %x\n", uRead);
		// 写入缓冲区
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uWrite: %x\n", uWrite);
		// 设置状态
		pIrp->IoStatus.Information = 2; // 返回两字节
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER3:
	{
		DbgPrint("IrpDeviceControlProc -> OPER3 输入字节数: %d\n", uInLength);
		DbgPrint("IrpDeviceControlProc -> OPER3 输出字节数: %d\n", uOutLength);
		// 读取缓冲区
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceControlProc -> OPER3 uRead: %x\n", uRead);
		// 写入缓冲区
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER3	uWrite: %x\n", uWrite);
		// 设置状态
		InterDLL(pDevObj, pIrp);

		pIrp->IoStatus.Information = 2; // 返回两字节
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER4:
	{
		memcpy(&uRead, pIoBuffer, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uRead: %x\n", uRead);
		// 写入缓冲区
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uWrite: %x\n", uWrite);
		// 设置状态
		pIrp->IoStatus.Information = 2; // 返回两字节
		nStatus = STATUS_SUCCESS;
		break;
	}
	case OPER5:
	{
		
		
		EndProcess(pDevObj, pIrp, pIoBuffer);
		memcpy(pIoBuffer, &uWrite, 4);
		DbgPrint("IrpDeviceControlProc -> OPER2 uWrite: %x\n", uWrite);
		// 设置状态
		pIrp->IoStatus.Information = 2; // 返回两字节
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

	//设置返回状态
	pIrp->IoStatus.Status = STATUS_SUCCESS;	//  getlasterror()得到的就是这个值
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

	// 创建设备
	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	nStatus = IoCreateDevice(
		pDriverObject,				//当前设备所属的驱动对象
		0,
		&DeviceName,			//设备对象的名称
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pDeviceObject			//设备对象指针
	);

	if (nStatus != STATUS_SUCCESS)
	{
		DbgPrint("IoCreateDevice Failed\n");
		return -1;
	}

	// 交互数据方式
	pDeviceObject->Flags |= DO_BUFFERED_IO;

	// 创建符号链接名称
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
