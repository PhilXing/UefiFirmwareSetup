#include <windows.h>

#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI	0x0000000000000001
#define EFI_GLOBAL_VARIABLE_GUID			L"{8BE4DF61-93CA-11d2-AA0D-00E098032B8C}"
#define MESSAGE_BOX_TITLE					L"Reboot to UEFI Firmware UI"


//#include <stdio.h>

//DWORD WINAPI GetFirmwareEnvironmentVariable(
//  _In_  LPCTSTR lpName,
//  _In_  LPCTSTR lpGuid,
//  _Out_ PVOID   pBuffer,
//  _In_  DWORD   nSize
//);

bool RasiePrivileges(void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	DWORD len;

	if (!OpenProcessToken(GetCurrentProcess(), 
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
			&hToken)) {
				//printf("Failed OpenProcessToken\r\n");
				return false;
	}

	LookupPrivilegeValue(NULL, SE_SYSTEM_ENVIRONMENT_NAME,
		&tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, &len);

	if (GetLastError() != ERROR_SUCCESS) {
		return false;
	}

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
		&tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, &len);

	if (GetLastError() != ERROR_SUCCESS) {
		//printf("Failed RasiePrivileges()\r\n");
		return false;
	}
	return true;
}

//THE EASIEST WINAPI PROGRAM YOU WILL EVER DO!
int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	DWORD OsIndications;

	if (!RasiePrivileges()) {
		MessageBox(NULL, L"Fail to raise Privileges", MESSAGE_BOX_TITLE,  MB_OK | MB_ICONASTERISK);
		return 1;
	}
	GetFirmwareEnvironmentVariable(L"OsIndications",EFI_GLOBAL_VARIABLE_GUID, &OsIndications, sizeof(DWORD));
    if (GetLastError() ==ERROR_INVALID_FUNCTION) { // This.. is.. LEGACY BIOOOOOOOOS....
		MessageBox(NULL, L"This is an Legacy Windows", MESSAGE_BOX_TITLE,  MB_OK | MB_ICONASTERISK);
		return 1;
    } else{
		MessageBox(NULL, L"This is an UEFI Windows", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
		OsIndications = OsIndications | EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
		if (SetFirmwareEnvironmentVariable(L"OsIndications",EFI_GLOBAL_VARIABLE_GUID, &OsIndications, sizeof(DWORD))) {
			MessageBox(NULL, L"Set OS Indications success", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
		   // Shut down the system and force all applications to close.   
		   if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE,
			  SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
			  SHTDN_REASON_MINOR_UPGRADE |
			  SHTDN_REASON_FLAG_PLANNED))
			  MessageBox(NULL, L"Failed to Shut Down the system.", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
		} else {
		    if (GetLastError() == ERROR_NOACCESS) { // GUID Namespace does not exist....
				MessageBox(NULL, L"GUID Namespace does not exist", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
			} else {
				MessageBox(NULL, L"Set OS Indications fail", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
			}
	        return 2;
		}
        return 0;
    }
}
