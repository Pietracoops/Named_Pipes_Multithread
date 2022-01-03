#pragma once

#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>
#include <iostream>
#include <atlstr.h>
#include <string>
#include <atomic>
#include <mutex>

#define BUFSIZE 1024//1024*16

class Pipes_Server
{
	//std::mutex mu;
	HANDLE hPipe;
	char buffer[1024];
	DWORD dwRead;

	BOOL fConnected;
	DWORD  dwThreadId = 0;
	LPCTSTR lpszPipename;

	HANDLE hThread;

	
	HANDLE hHeap;


public:

	std::string pipe_broadcast_string;
	int update_frequency;

	static DWORD WINAPI InstanceThread(LPVOID lpvParam);
	VOID GetAnswerToRequest(LPTSTR pchRequest, LPTSTR pchReply, LPDWORD pchBytes);
	int launch_server();
	Pipes_Server();
	~Pipes_Server();


};


class Pipes_Client
{

	HANDLE hPipe;
	char buffer[1024];
	DWORD dwRead;

public:


	Pipes_Client();
	~Pipes_Client();


};

