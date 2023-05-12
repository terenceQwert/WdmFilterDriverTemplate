#include <windows.h>
#include <stdio.h>

#include "../common/common.h"

int main()
{

	HANDLE hDevice = 
		CreateFile(AppLookingforDeviceName,
					GENERIC_READ | GENERIC_WRITE,
					0,		// share mode none
					NULL,	// no security
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL );		// no template

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed to obtain file handle to device "
			"with Win32 error code: %d\n",
			 GetLastError() );
		return 1;
	}

	DWORD dRet=0;
	UCHAR dwBuffer[10] = { 0 };
	ReadFile(hDevice,&dwBuffer,sizeof(dwBuffer),&dRet,NULL);
	for (int i = 0; i < sizeof(dwBuffer) ; i++)
	{
		printf("0x%02x ", dwBuffer[i]);
	}
	printf("\n");
	printf("final return length = %d\n", dRet);
	CloseHandle(hDevice);

	return 0;
}