#include <windows.h>

#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI	0x0000000000000001
#define EFI_GLOBAL_VARIABLE_GUID						L"{8BE4DF61-93CA-11d2-AA0D-00E098032B8C}"
#define MESSAGE_BOX_TITLE									L"PX GoFirmwareUI 0.3"

bool RasiePrivileges(void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	DWORD len;

	if (!OpenProcessToken(GetCurrentProcess(), 
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
			&hToken)) {
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
		return false;
	}

	return true;
}


int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	UINT64 OsIndicationsSupported;
	UINT64 OsIndications = 0;
	//
	// Raise process privileges for firmware variable access and system reboot
	//
	if (!RasiePrivileges()) {
		MessageBox(NULL, L"Fail to raise Privileges", MESSAGE_BOX_TITLE,  MB_OK | MB_ICONASTERISK);
		return 1;
	}
	//
	// Get firmware variable OsIndicationsSupported with error check
	//
	GetFirmwareEnvironmentVariable(L"OsIndicationsSupported",EFI_GLOBAL_VARIABLE_GUID, &OsIndicationsSupported, sizeof(OsIndicationsSupported));
    if (GetLastError() ==ERROR_INVALID_FUNCTION) { // This.. is.. LEGACY BIOOOOOOOOS....
		MessageBox(NULL, L"This is not an UEFI OS", MESSAGE_BOX_TITLE,  MB_OK | MB_ICONASTERISK);
		return 2;
    } 
	//
	// Check Boot To FW UI capability
	//
	if ((OsIndicationsSupported & EFI_OS_INDICATIONS_BOOT_TO_FW_UI) == 0) {
		MessageBox(NULL, L"Boot To FW UI is not supported by the system firmware.", MESSAGE_BOX_TITLE,  MB_OK | MB_ICONASTERISK);
		return 3;
	}
	//
	// Get firmware variable OsIndications with error check
	//
	GetFirmwareEnvironmentVariable(L"OsIndications",EFI_GLOBAL_VARIABLE_GUID, &OsIndications, sizeof(OsIndications));
	//
	// Set indication for Boot to Firmware UI with error check
	//
	OsIndications = OsIndications | EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
	if (!SetFirmwareEnvironmentVariable(L"OsIndications",EFI_GLOBAL_VARIABLE_GUID, &OsIndications, sizeof(OsIndications))) {
	    if (GetLastError() == ERROR_NOACCESS) { // GUID Namespace does not exist....
			MessageBox(NULL, L"GUID Namespace does not exist", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
		} else {
			MessageBox(NULL, L"Set OS Indications fail", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
		}
        return 5;
	}
	//
	// Reboot the system and force all applications to close
	//
	MessageBox(NULL, L"System will reboot to BIOS setup", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE,
 	  SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
 	  SHTDN_REASON_MINOR_UPGRADE |
	  SHTDN_REASON_FLAG_PLANNED)) {
		  MessageBox(NULL, L"Failed to Reboot the system.", MESSAGE_BOX_TITLE, MB_OK | MB_ICONASTERISK);
          return 6;
	}

	return 0;
}
