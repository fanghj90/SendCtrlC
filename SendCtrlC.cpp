// SendCtrlC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"

// Return codes:
//   0: Success
//   1: No process id specified
//   2: Invalid process id (zero or non-numeric)
//   3: Specified process id does not exist
//   4: Specified process is not a console app
//   5: Unknown AttachConsole error
//   6: Failed to send CTRL+C signal to process
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc <= 1)
	{
		// Need process id.
		printf("cmd processid\n");
		return 1;
	}

	DWORD procId = _ttol(argv[1]);
	if (procId == 0)
	{
        // Invalid id
        printf("processid invalid\n");
		return 2;
	}

	// Ignore the CTRL+C event we send to ourselves.
	SetConsoleCtrlHandler(NULL, TRUE);

	// Detach from current console (can only be attached to one at a time).
	// Ignore error -- it just means we weren't already attached.
	FreeConsole();

	int bvresult = 0;
	// Attach to console of given proc id
	if (!AttachConsole(procId))
	{
		auto error = GetLastError();
		AttachConsole(-1);
		if (error == ERROR_GEN_FAILURE)
		{
			bvresult = 3;
            // Process does not exist
            printf("Process does not exist\n");
			//return 3;
		}
		else if (error == ERROR_INVALID_HANDLE)
        {
            bvresult = 4;
            // Process does not have a console.
            printf("Process does not have a console\n");
			//return 4;
        }
		else if (error == ERROR_ACCESS_DENIED)
		{
            bvresult = 5;
            printf("Does not have access. please run as adminstrator\n");
		}
		else
		{
			bvresult = 5;
			printf("Unknown AttachConsole error: %d\n", error);
		}
		//return 5;
	}
	if (bvresult == 0)
	{
		// Send CTRL+C to target process (and outselves).
		if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0))
        {
            auto error = GetLastError();
			AttachConsole(-1);
			bvresult = 6;
			printf("Failed to send CTRL+C signal to process: %d \n", error);
			//return 6;
		}
	}

	// 
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, procId);
    if (INVALID_HANDLE_VALUE == hProcess)
    {
        return 0;
    }
	if (bvresult == 0)
    {
		if (WaitForSingleObject(hProcess, 5 * 1000) != WAIT_OBJECT_0)
		{
			bvresult = 7;
		}
	}
	if (bvresult != 0)
	{
		TerminateProcess(hProcess, 0);
	}
    CloseHandle(hProcess);

	return 0;
}

