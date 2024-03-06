#include <Windows.h>
#include <stdio.h>
#include <string.h>

void OutputLastError() {
	DWORD errorCode = GetLastError();

#ifdef UNICODE
	LPWSTR errorMessage;
#else
	LPSTR errorMessage;
#endif // !UNICODE

	DWORD errorSize = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		&errorMessage,
		0,
		NULL
	);

#ifdef UNICODE
	wchar_t formattedMessageBuffer[1024];
	int bytesWritten = swprintf_s(formattedMessageBuffer, 1024, L"Error %d (0x%X): %s", errorCode, errorCode, errorMessage);
#else
	char formattedMessageBuffer[1024];
	int bytesWritten = sprintf_s(formattedMessageBuffer, 1024, "Error %d (0x%X): %s", errorCode, errorCode, errorMessage);
#endif // !UNICODE

	// print error message to dbg output
	OutputDebugString(formattedMessageBuffer);
	// free the error message buffer
	LocalFree(errorMessage);
}