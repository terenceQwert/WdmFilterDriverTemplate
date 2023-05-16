#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
// #include "../common/Driver.h"
#include "../common/common.h"
#ifdef __cplusplus
}
#endif

typedef struct _DEVICE_EXTENSION
{
  PDEVICE_OBJECT fdo;
  PDEVICE_OBJECT NextStackDevice;
  UNICODE_STRING  ustrDeviceName;
  UNICODE_STRING  ustrSymLinkName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#define PAGECODE code_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA  data_seg("INIT")
extern "C" NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING
);
#define arraysize(p)  (sizeof(p)/sizeof((p)[0]))
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
// #pragma alloc_text (INIT, EchoPrintDriverVersion)
// #pragma alloc_text (PAGE, EchoEvtDeviceAdd)
#endif
NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
NTSTATUS HelloWDMDispatch(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
NTSTATUS StandardDriverRead(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
NTSTATUS StandardDriverWDMCLose( IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

VOID DumpDeviceStack(IN PDEVICE_OBJECT pdo);
VOID DisplayProcessName();
VOID LinkListTest();
VOID New_Test();

void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject);

#pragma INITCODE
extern "C"
NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING RegistryEntry
)
{
  KdPrint(("Enter MyStandardDriver:DriverEntry\n"));
  KdPrint(("Registry = %wZ\n", *RegistryEntry));
  DriverObject->DriverExtension->AddDevice = AddDevice;
  DriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp;
  DriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMDispatch;
  DriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDispatch;
  DriverObject->MajorFunction[IRP_MJ_WRITE] = HelloWDMDispatch;
  DriverObject->MajorFunction[IRP_MJ_READ] = StandardDriverRead;
  DriverObject->MajorFunction[IRP_MJ_CLOSE] = StandardDriverWDMCLose;
  DriverObject->DriverUnload = HelloWDMUnload;
  KdPrint(("Leave MyStandardDriver:DriverEntry\n"));
  return STATUS_SUCCESS;
}



#pragma PAGED_CODE
NTSTATUS AddDevice(
  IN PDRIVER_OBJECT DriverObject, 
  IN PDEVICE_OBJECT PhysicalDeviceObject  /* PDO*/
)
{
  PAGED_CODE();
  KdPrint(("Enter AddDevice\n"));

  NTSTATUS ntStatus = STATUS_SUCCESS;
  PDEVICE_OBJECT fdo;
  UNICODE_STRING  devName;
  KdPrint(("AddDevice initialize unicode\n"));

  //
  // L"\\Device\\MyWDMDeviceA"
  //
  
  RtlInitUnicodeString(&devName, MYWDM_NAME);
  KdPrint(("AddDevice IoCreateDevice\n"));
  ntStatus = IoCreateDevice(
    DriverObject,
    sizeof(DEVICE_EXTENSION),
    &devName,
    FILE_DEVICE_UNKNOWN,
    0,
    FALSE,
    &fdo
  );
  if (!NT_SUCCESS(ntStatus))
  {
    KdPrint(("AddDevice IoCreateDevice fail with status %r\n", ntStatus));
    return STATUS_SUCCESS;
//    return ntStatus; 
  }
  KdPrint(("AddDevice create name start\n"));
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
  pdx->fdo = fdo;
  pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
  KdPrint(("Print Device Address 0x%x\n", pdx->NextStackDevice));

  UNICODE_STRING  symLinkName;
  RtlInitUnicodeString(&symLinkName, SMBOL_LNK_NAME);
  pdx->ustrDeviceName = devName;
  pdx->ustrSymLinkName = symLinkName;

  ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);
  if (!NT_SUCCESS(ntStatus))
  {
    KdPrint(("AddDevice - create symbolic link fail with status = %d\n", ntStatus));
    IoDeleteSymbolicLink(&pdx->ustrDeviceName);
    ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);

    if (!NT_SUCCESS(ntStatus))
    {
      return ntStatus;
    }
  }
  fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
  fdo->Flags &= ~DO_DEVICE_INITIALIZING;
  KdPrint(("Leave AddDevice\n"));
#if 0
  DumpDeviceStack(PhysicalDeviceObject);
  DisplayProcessName();
#endif
  return STATUS_SUCCESS;
}

#pragma
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
  PAGED_CODE();
  KdPrint(("Enter Standard Driver DefaultHandler\n"));
  IoSkipCurrentIrpStackLocation(Irp);
  KdPrint(("Leave Standard Driver DefaultHandler\n"));
  return IoCallDriver(pdx->NextStackDevice, Irp);
}

#pragma
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP pIrp)
{
  PAGED_CODE();
  KdPrint(("Enter Standard Driver RemoveDevice\n"));
  pIrp->IoStatus.Status = DefaultPnpHandler(pdx, pIrp);
  IoDeleteSymbolicLink(&pdx->ustrSymLinkName);

  if (pdx->NextStackDevice)
    IoDetachDevice(pdx->NextStackDevice);
  IoDeleteDevice(pdx->fdo);
  KdPrint(("Leave Standard Driver RemoveDevice\n"));
  return STATUS_SUCCESS;
}

#pragma

NTSTATUS
StandardDriverRead(IN PDEVICE_OBJECT, IN PIRP irp)
{
  PAGED_CODE();
  KdPrint(("Enter Standard Driver: Enter Read\n"));
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
  KdPrint(("Standard Driver: buffer length = %d\n", stack->Parameters.Read.Length));

  //
  // only return 5 bytes
  //
  {
    ULONG length = stack->Parameters.Read.Length;
    char buf[5] = { 0x31,0x32,0x33,0x34,0x35 };
    unsigned char * pchBuf = (unsigned char*)irp->AssociatedIrp.SystemBuffer;
    memcpy((PVOID)&pchBuf[5], &buf, strlen(buf));
    pchBuf[length-1] = 0;
  }
  //
  //
  //

  //
  // make sure this irp working success
  //
  irp->IoStatus.Status = STATUS_SUCCESS;

  ///
  /// returned length is same as providing buffer
  ///
  irp->IoStatus.Information = stack->Parameters.Read.Length;  // no bytes transferred.
  
  //
  // this is the lowest level driver, we must call IoCompleteRequest routine to complete this irp traversal.
  //
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  KdPrint(("Enter Standard Driver: Exit  Read\n"));
  return STATUS_SUCCESS;
}

NTSTATUS HelloWDMDispatch(IN PDEVICE_OBJECT , IN PIRP irp)
{
  PAGED_CODE();
  KdPrint(("Enter Standard Driver DispatchRoutine\n"));
  irp->IoStatus.Status = STATUS_SUCCESS;
  irp->IoStatus.Information = 0;  // no bytes transferred.
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  KdPrint(("Leave Standard Driver DispatchRoutine\n"));
  return STATUS_SUCCESS;
}

#pragma
void HelloWDMUnload(IN PDRIVER_OBJECT )
{
  PAGED_CODE();
  KdPrint(("Enter Standard Driver Unload\n"));
//  LinkListTest(); BUGBUG
//  DisplayProcessName();
  KdPrint(("Leave Standard Driver Unload\n"));
}

#pragma PAGECODE
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{

  PAGED_CODE();
#if DBG
  static char * fcnName[] = {
    "IRP_MN_START_DEVICE",
    "IRP_MN_QUERY_REMOVE_DEVICE",
    "IRP_MN_REMOVE_DEVICE",
    "IRP_MN_CANCEL_REMOVE_DEVICE",
    "IRP_MN_STOP_DEVICE",
    "IRP_MN_QUERY_STOP_DEVICE",
    "IRP_MN_CANCEL_STOP_DEVICE",
    "IRP_MN_QUERY_DEVICE_RELATIONS",
    "IRP_MN_QUERY_INTERFACE",
    "IRP_MN_QUERY_CAPABILITIES",
    "IRP_MN_QUERY_RESOURCES",
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
    "IRP_MN_QUERY_DEVICE_TEXT",
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
    "",
    "IRP_MN_READ_CONFIG",
    "IRP_MN_WRITE_CONFIG",
    "IRP_MN_EJECT",
    "IRP_MN_SET_LOCK",
    "IRP_MN_QUERY_ID",
    "IRP_MN_QUERY_PNP_DEVICE_STATE",
    "IRP_MN_QUERY_BUS_INFORMATION",
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",
    "IRP_MN_SURPRISE_REMOVAL"
  };  
#endif // DBGS
  KdPrint(("Enter Standard Driver Pnp\n"));
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
  static NTSTATUS(*fcntab[]) (PDEVICE_EXTENSION pdx, PIRP Irp) = {
    DefaultPnpHandler,
    DefaultPnpHandler,
    HandleRemoveDevice,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler
  };
  ULONG fcn = stack->MinorFunction;
  if (fcn >= arraysize(fcntab))
  {
    status = DefaultPnpHandler(pdx, Irp);
    return status;
  }
  KdPrint(("PNP  Request (%s)\n", fcnName[fcn]));
  status = (*fcntab[fcn])(pdx, Irp);
  KdPrint(("Leave Standard Driver Pnp"));
  return status;
}


#pragma PAGEDCODE
NTSTATUS StandardDriverWDMCLose(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP pIrp)
{
  KdPrint(("StandardDRiver:Enter Close\n"));
  NTSTATUS ntStatus = STATUS_SUCCESS;
#if 0
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

  IoSkipCurrentIrpStackLocation(pIrp);

  ntStatus = IoCallDriver(pdx->TargetDevice, pIrp);
#else
  pIrp->IoStatus.Information = 0;
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
#endif
  KdPrint(("StandardDRiver:Leave Close\n"));

  return ntStatus;
}