/************************************************************************
* �ɮצW��:Driver.cpp                                                 
* �@    ��:�i�|
* �������:2007-11-1
*************************************************************************/

#include "../common/Driver.h"
#include "../common/common.h"

NTSTATUS FilterAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
/************************************************************************
* �禡�W��:DriverEntry
* �\��y�z:��l���X�ʵ{���A�w��M�ӽеw��귽�A�s�ؤ��֪���
* �ѼƦC��:
      pDriverObject:�qI/O�޲z�����Ƕi�Ӫ��X�ʪ���
      pRegistryPath:�X�ʵ{���b�n�������������|
* ��^ ��:��^��l���X�ʪ��A
*************************************************************************/
#pragma INITCODE
extern "C" NTSTATUS DriverEntry (
			IN PDRIVER_OBJECT pDriverObject,
			IN PUNICODE_STRING ) 
{
	KdPrint(("MyFilterDriver:Enter DriverEntry\n"));

	//�n����L�X�ʩI�s�禡�J�f
	pDriverObject->DriverExtension->AddDevice = FilterAddDevice;
	pDriverObject->DriverUnload = HelloDDKUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloDDKCreate;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = HelloDDKClose;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloDDKDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = FilterRead;
	KdPrint(("MyFilterDriver:Leave DriverEntry\n"));

	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS FilterAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
// 	PAGED_CODE();
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING DeviceName;
	RtlInitUnicodeString( &DeviceName, ATTACH_TO_DRIVER);

	KdPrint(("MyFilterDriver:Enter FilterAddDevice\n"));
	PDEVICE_OBJECT DeviceObject = NULL;
	PFILE_OBJECT FileObject = NULL;
	//�M��DriverA�s�ت��˸m����
	ntStatus = IoGetDeviceObjectPointer(&DeviceName,FILE_ALL_ACCESS,&FileObject,&DeviceObject);

	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("MyFilterDriver:IoGetDeviceObjectPointer() 0x%x\n", ntStatus ));
		return ntStatus;
	}

	//�s�ئۤv���X�ʸ˸m����
	ntStatus = CreateDevice(DriverObject);

	if ( !NT_SUCCESS( ntStatus ) )
	{
		ObDereferenceObject( FileObject );
		DbgPrint( "IoCreateDevice() 0x%x!\n", ntStatus );
		return ntStatus;
	}

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;

	PDEVICE_OBJECT FilterDeviceObject = pdx->pDevice;

	//�N�ۤv���˸m���󱾸��bDriverA���˸m����W
	PDEVICE_OBJECT TargetDevice = IoAttachDeviceToDeviceStack( FilterDeviceObject,
										  DeviceObject );
	//�N���h�˸m����O���U��
	pdx->TargetDevice = TargetDevice;
	
	if ( !TargetDevice )
	{
		ObDereferenceObject( FileObject );
		IoDeleteDevice( FilterDeviceObject );
		DbgPrint( "IoAttachDeviceToDeviceStack() 0x%x!\n", ntStatus );
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	FilterDeviceObject->DeviceType = TargetDevice->DeviceType;
	FilterDeviceObject->Characteristics = TargetDevice->Characteristics;
	FilterDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	FilterDeviceObject->Flags |= ( TargetDevice->Flags & ( DO_DIRECT_IO |
														 DO_BUFFERED_IO ) );
	ObDereferenceObject( FileObject );

	KdPrint(("MyFilterDriver:attached on specific driver successfully!\n"));
	
	KdPrint(("MyFilterDriver:Leave FilterAddDevice\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* �禡�W��:CreateDevice
* �\��y�z:��l�Ƹ˸m����
* �ѼƦC��:
      pDriverObject:�qI/O�޲z�����Ƕi�Ӫ��X�ʪ���
* ��^ ��:��^��l�ƪ��A
*************************************************************************/
// #pragma INITCODE
#pragma PAGEDCODE
NTSTATUS CreateDevice (
		IN PDRIVER_OBJECT	pDriverObject) 
{
	NTSTATUS ntStatus;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
	
	//�s�ظ˸m�W��
	UNICODE_STRING devName;
	RtlInitUnicodeString( &devName, MY_FILTER_DRIVER_NAME);
	
	//�s�ظ˸m
	ntStatus = IoCreateDevice( pDriverObject,
						sizeof(DEVICE_EXTENSION),
						&(UNICODE_STRING)devName,
						FILE_DEVICE_UNKNOWN,
						0, TRUE,
						&pDevObj );
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;
	return STATUS_SUCCESS;
}

/************************************************************************
* �禡�W��:HelloDDKUnload
* �\��y�z:�t�d�X�ʵ{���������ާ@
* �ѼƦC��:
      pDriverObject:�X�ʪ���
* ��^ ��:��^���A
*************************************************************************/
#pragma PAGEDCODE
VOID HelloDDKUnload (IN PDRIVER_OBJECT pDriverObject) 
{
	PDEVICE_OBJECT	pNextObj;
	KdPrint(("MyFilterDriver:Enter DriverUnload\n"));
	pNextObj = pDriverObject->DeviceObject;

	while (pNextObj != NULL) 
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)
			pNextObj->DeviceExtension;
		pNextObj = pNextObj->NextDevice;
		//�q�˸m���|���u�X
		IoDetachDevice( pDevExt->TargetDevice);
		//�R���Ӹ˸m����
		IoDeleteDevice( pDevExt->pDevice );
	}
	KdPrint(("MyFilterDriver:Exit DriverUnload\n"));
}

/************************************************************************
* �禡�W��:HelloDDKDispatchRoutine
* �\��y�z:��ŪIRP�i��B�z
* �ѼƦC��:
      pDevObj:�\��˸m����
      pIrp:�qIO�ШD�]
* ��^ ��:��^���A
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("MyFilterDriver:Enter DispatchRoutine\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;
	// ����IRP
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest( pIrp, IO_NO_INCREMENT );
	KdPrint(("MyFilterDriver:Leave DispatchRoutine\n"));
	return ntStatus;
}

#pragma PAGEDCODE
NTSTATUS HelloDDKCreate(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("MyFilterDriver:Enter Create\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//
// 	// ����IRP
// 	pIrp->IoStatus.Status = ntStatus;
// 	pIrp->IoStatus.Information = 0;	// bytes xfered
// 	IoCompleteRequest( pIrp, IO_NO_INCREMENT );

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

    IoSkipCurrentIrpStackLocation (pIrp);

    ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);

	KdPrint(("MyFilterDriver:Leave Create\n"));

	return ntStatus;
}

NTSTATUS
FilterDriverIoCompletion(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)
{
	KdPrint(("MyFilterDRiver::FilterDriverIoComplete Routine\n"));
	if (Irp->PendingReturned == TRUE)
		IoMarkIrpPending(Irp);
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
NTSTATUS FilterRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("MyFilterDriver:Enter Read\n"));
	char* pchBuffer = "filter";
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//�N�ۤv����IRP�A�令�ѩ��h�X�ʭt�d

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
#if 0
	//�I�s���h�X��
  IoSkipCurrentIrpStackLocation (pIrp);
	// calll to next-lower driver
	ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);
#else
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	PIO_STACK_LOCATION stack = IoGetNextIrpStackLocation(pIrp);
	ULONG ulReadLength = stack->Parameters.Read.Length;
	IoSetCompletionRoutine(pIrp, FilterDriverIoCompletion, NULL, TRUE, TRUE, TRUE);
	// calll to next-lower driver
	ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);
#endif
	// modfiy data after IoCallDriver(...)
	memcpy(pIrp->AssociatedIrp.SystemBuffer, pchBuffer, strlen(pchBuffer));

	KdPrint(("MyFilterDriver:Leave Read\n"));
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0x05;
	return ntStatus;
}

#pragma PAGEDCODE
NTSTATUS HelloDDKClose(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) 
{
	KdPrint(("MyFilterDriver:Enter Close\n"));
	NTSTATUS ntStatus = STATUS_SUCCESS;

	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

    IoSkipCurrentIrpStackLocation (pIrp);

    ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);	
	
	KdPrint(("MyFilterDriver:Leave Close\n"));

	return ntStatus;
}


