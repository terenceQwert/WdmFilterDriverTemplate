/************************************************************************
* �ɮצW��:Driver.h                                                 
* �@    ��:�i�|
* �������:2007-11-1
*************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTDDK.h>
#ifdef __cplusplus
}
#endif 

#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

#if 0
typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;	//�W��
	PDEVICE_OBJECT TargetDevice;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
#else
typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pDevice;
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT TargetDevice;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING  ustrDeviceName;
	UNICODE_STRING  ustrSymLinkName;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;
#endif
// �禡�ŧi

NTSTATUS CreateDevice (IN PDRIVER_OBJECT pDriverObject);
VOID HelloDDKUnload (IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp);
NTSTATUS HelloDDKCreate(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp);
NTSTATUS HelloDDKClose(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) ;
NTSTATUS HelloDDKRead(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp) ;



