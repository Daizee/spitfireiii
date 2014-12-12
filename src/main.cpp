//
// main.cpp
// Project Spitfire
//
// Copyright (c) 2014 Daizee (rensiadz at gmail dot com)
//
// This file is part of Spitfire.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "includes.h"
#include "funcs.h"
#include "amf3.h"

#ifndef WIN32
#include <stdio.h>
void sigfunc(int sig_no);
#else
#define WIN32_LEAN_AND_MEAN   
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <signal.h>
#include <conio.h>
BOOL WINAPI ConsoleHandler(DWORD CEvent);
#endif

#include "Server.h"

Server * gserver = 0;

int main(int argc, char* argv[])
{
#ifndef WIN32
	struct sigaction sa = { 0 };
	sa.sa_handler = &sigfunc;
	sigaction(SIGINT, &sa, 0);
	sigaction(SIGHUP, &sa, 0);
	sigaction(SIGUSR1, &sa, 0);
	sigaction(SIGUSR2, &sa, 0);
#else
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
	{
		// unable to install handler... 
		// display message to the user
		printf("Unable to install handler!\n");
		return -1;
	}
#endif

	srand(unixtime());

	gserver = new Server();

	gserver->serverstatus = SERVERSTATUS_STARTING;

	try {
		//Initialize server and load config files
		if (!gserver->Init()) { return 0; }

		//Connect to SQL server
		if (!gserver->ConnectSQL()) { return 0; }

		//Open sockets
		if (!gserver->InitSockets()) { return 0; }
	}
	catch (std::exception& e)
	{
		gserver->consoleLogger->fatal(Poco::format("Init() Exception: %s", (string)e.what()));
		return 0;
	}
	catch (...)
	{
		gserver->consoleLogger->fatal("Unspecified Init() Exception.");
		return 0;
	}

	//Server is finally online here
	gserver->serverstatus = SERVERSTATUS_ONLINE;


	// Run the server until stopped.
	try
	{
		gserver->run();
	}
	catch (Poco::Data::MySQL::MySQLException * e)
	{
		gserver->consoleLogger->fatal(Poco::format("SQL Exception: %s", e->displayText()));
	}


	try
	{
		gserver->TimerThread();
	}
	catch (Poco::Data::MySQL::MySQLException * e)
	{
		gserver->consoleLogger->fatal(Poco::format("TimerThread() SQL Exception: %s", e->displayText()));
	}
	catch (...)
	{
		gserver->consoleLogger->fatal("Unspecified TimerThread() Exception.");
	}


	gserver->consoleLogger->information("Shutting down");

	//does this need a thread join here?

	gserver->stop_all();
	delete gserver;
	printf("Exiting...\n");
	return 0;
}


#ifndef WIN32
void sigfunc(int sig_no)
{
	printf("Signal received: %d\n", sig_no);
	//	if (sig_no == SIGSEGV)
	//	{
	//		Log("Seg fault onoes!");
	//	}
	/*else*/ if (sig_no == SIGINT)
	{
		printf("Interrupt request\n");
		gserver->Shutdown();
		//LoginServer->serverrunning = false;
	}
	else if (sig_no == SIGHUP)
	{
		printf("Rehash Signal\n");
	}
	else if (sig_no == SIGUSR1)
	{
		printf("user1 Signal\n");
	}
	else if (sig_no == SIGUSR2)
	{
		printf("user2 Signal\n");
	}
}
#else
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	char mesg[128];

	switch (CEvent)
	{
		case CTRL_C_EVENT:
			// 		MessageBox(NULL,
			// 			_T("CTRL+C received!"),_T("CEvent"),MB_OK);
			gserver->Shutdown();
			break;
		case CTRL_BREAK_EVENT:
			// 		MessageBox(NULL,
			// 			_T("CTRL+BREAK received!"),_T("CEvent"),MB_OK);
			gserver->Shutdown();
			break;
		case CTRL_CLOSE_EVENT:
			// 		MessageBox(NULL,
			// 			_T("Program being closed!"),_T("CEvent"),MB_OK);
			gserver->Shutdown();
			break;
		case CTRL_LOGOFF_EVENT:
			// 		MessageBox(NULL,
			// 			_T("User is logging off!"),_T("CEvent"),MB_OK);
			gserver->Shutdown();
			break;
		case CTRL_SHUTDOWN_EVENT:
			// 		MessageBox(NULL,
			// 			_T("User is logging off!"),_T("CEvent"),MB_OK);
			gserver->Shutdown();
			break;

	}
	return TRUE;
}
#endif