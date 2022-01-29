#include "pipes.h"


// links

// https://stackoverflow.com/questions/10600111/multiple-arguments-to-createthread-function
// https://docs.microsoft.com/en-us/windows/win32/ipc/multithreaded-pipe-server
// https://stackoverflow.com/questions/26561604/create-named-pipe-c-windows


//DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);
std::string broadcast_string;
std::string* broadcast_string_ptr;
std::atomic<bool> thread_terminated;
std::mutex m;

Pipes_Server::Pipes_Server(std::string pipename)
{
	DWORD WINAPI InstanceThread(LPVOID lpvParam);

	lpszPipename = TEXT("\\\\.\\pipe\\Foo");
	//lpszPipename = (LPCTSTR)pipename.c_str();
	hPipe = INVALID_HANDLE_VALUE;
	hThread = NULL;

	update_frequency = 1000;
	enable_log = false;

	broadcast_string_ptr = &pipe_broadcast_string;

	log_filename = "pipes_log.txt";

}

Pipes_Server::~Pipes_Server()
{
	delete broadcast_string_ptr;

	if (enable_log)
	{
		fout.close();
	}

}

void Pipes_Server::open_log()
{
	enable_log = true;
	fout.open(log_filename);
}

DWORD WINAPI Pipes_Server::InstanceThread(LPVOID lpvParam)
// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;

	// Do some extra error checking since the app will keep running even if this
	// thread fails.
	std::lock_guard<std::mutex> lock(m);
	if (lpvParam == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		thread_terminated = true;
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		thread_terminated = true;
		return (DWORD)-1;
	}

	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		thread_terminated = true;
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");

	// The thread's parameter is a handle to a pipe object instance. 

	//hPipe = tPkg->hPipe;
	hPipe = (HANDLE)(lpvParam);

	// Loop until done reading
	while (1)
	{
		//// Read client requests from the pipe. This simplistic code only allows messages
		//// up to BUFSIZE characters in length.
		//fSuccess = ReadFile(
		//	hPipe,        // handle to pipe 
		//	pchRequest,    // buffer to receive data 
		//	BUFSIZE * sizeof(TCHAR), // size of buffer 
		//	&cbBytesRead, // number of bytes read 
		//	NULL);        // not overlapped I/O 
		//
		//if (!fSuccess || cbBytesRead == 0)
		//{
		//	if (GetLastError() == ERROR_BROKEN_PIPE)
		//	{
		//		_tprintf(TEXT("InstanceThread: client disconnected.\n"));
		//	}
		//	else
		//	{
		//		_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
		//	}
		//	break;
		//}
		//
		//// Process the incoming message.
		//GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);

		std::string str = *broadcast_string_ptr; // Update the string with global broadcast_string
		_tcscpy_s(pchReply, str.size() + 1, CA2T(str.c_str()));	// Copy the string to Reply buffer
		cbReplyBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR); // Get size of reply

		// Write the reply to the pipe. 
		fSuccess = WriteFile(
			hPipe,        // handle to pipe 
			pchReply,     // buffer to write from 
			cbReplyBytes, // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbReplyBytes != cbWritten)
		{
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			break;
		}
		Sleep(150);
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	thread_terminated.store(true);

	printf("InstanceThread exiting.\n");
	return 1;
}

VOID Pipes_Server::GetAnswerToRequest(LPTSTR pchRequest,
	LPTSTR pchReply,
	LPDWORD pchBytes)
	// This routine is a simple function to print the client request to the console
	// and populate the reply buffer with a default data string. This is where you
	// would put the actual client request processing code that runs in the context
	// of an instance thread. Keep in mind the main thread will continue to wait for
	// and receive other client connections while the instance thread is working.
{
	_tprintf(TEXT("Client Request String:\"%s\"\n"), pchRequest);

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pchReply, BUFSIZE, TEXT("default answer from server"))))
	{
		*pchBytes = 0;
		pchReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}
	
	_tcscpy_s(pchReply, _tcslen(_T("Hello Pipe\n")), _T("Hello Pipe\n"));
	*pchBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR);
}

int Pipes_Server::launch_server()
{

	if (enable_log)
	{
		fout << "Launching Server..." << std::endl;
	}

	std::cout << "Launching Server..." << std::endl;
	fConnected = FALSE;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;

	for (;;)
	{

		_tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
		hPipe = CreateNamedPipe(
			lpszPipename,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 


		if (hPipe == INVALID_HANDLE_VALUE)
		{
			if (enable_log)
			{
				fout << "CreateNamedPipe failed, GLE=" << GetLastError() << std::endl;
			}

			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
			//std::cout << "CreateNamedPipe failed, GLE=" << GetLastError() << std::endl;
			return -1;
		}

		if (enable_log)
		{
			fout << "Waiting for client connection..." << std::endl;
		}

		std::cout << "Waiting for client connection..." << std::endl;
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		std::cout << "connection found!" << std::endl;
		

		if (fConnected)
		{
			if (enable_log)
			{
				fout << "connection found!" << std::endl;
			}

			printf("Client connected, creating a processing thread.\n");
			//std::cout << "Client connected, creating a processing thread." << std::endl;

			thread_terminated = false;
			// Create a thread for this client. 
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				InstanceThread,    // thread proc
				(LPVOID)hPipe,     // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL)
			{
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				//std::cout << "CreateThread failed, GLE=" << GetLastError() << std::endl;
				return -1;
			}
			else CloseHandle(hThread);
			
			
			//while (true) // update variable in a loop
			//{
			//
			//	broadcast_string = pipe_broadcast_string; // using pointer to update quicker this is now deprecated
			//
			//	if (thread_terminated.load()) // check if thread is still alive
			//	{
			//		break;
			//	}
			//	Sleep(update_frequency);
			//}


		}
		else
		{
			// The client could not connect, so close the pipe.
			std::cout << "Client could not connect, closing the pipe" << std::endl;
			CloseHandle(hPipe);

		}

	}

	return 0;

}








Pipes_Client::Pipes_Client()
{

}

Pipes_Client::~Pipes_Client()
{

}
