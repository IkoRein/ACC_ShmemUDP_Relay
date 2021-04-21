// ACCSM_UDP_Relay.cpp : Defines the entry point for the console application.
// Original ACC Example version, Ensi Ferrum
// Additions, Iko Rein


#include "stdafx.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <string>
#include <conio.h>

#include "SharedFileOut.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS false

#pragma optimize("",off)
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

/////////////////////////////////
// Version of the tool
float toolVersion = 1.09f;

const char* THE_END = "Here is the end.";
const char* PRESSED_ESC = "You pressed ESC key, so the application exits. ";
//const char* AV_NOTE = "Other keys do nothing now, as some AV programs might flag the use of those keys as false positives. ";
const char* ENJOYED = "I hope you enjoyed using this little tool. ";
const char* MORE_TEXT = "Restart the relay to get data from ";
const char* ACC = "Assetto Corsa Competizione";
const char* PC2 = "Project Cars 2";
const char* AMS2 = "Automobilista 2";
const char* F12019 = "F1 2019 ";
const char* F12018 = "F1 2018 ";
const char* F12017 = "F1 2017 ";
const char* F12016 = "F1 2016 ";
const char* NO_NEED = "You don't need this tool with ";
const char* OR = " or ";

struct SMElement
{
	HANDLE hMapFile;
	LPVOID  mapFileBuffer;
};

const char* ACC_SHM = "ACC Shm TEST AGAINST IDIOTIC AVs ";
const char* MAP_FAILED = "file mapping failed   ";
const char* MAP_VIEW_FAIL = "map wiew failed   ";
#define SEND_TO_FAILED "Send data via UDP failed because : %d" // //"sendto failed : %d"

//UDP default settings
int PORT = 9996;
char* IP = "127.0.0.1";

SOCKET s;

// 
SMElement m_physics;
SMElement m_graphics;
SMElement m_static;

void initPhysics()
{

	TCHAR szName[] = TEXT("Local\\acpmf_physics");

	m_physics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFilePhysics), szName);
	if (!m_physics.hMapFile)
	{
		wcout << MAP_FAILED << endl;
		//MessageBoxA(GetActiveWindow(), "A ", ACC_SHM, MB_OK); // "CreateFileMapping  failed during "
		return;
	}

	m_physics.mapFileBuffer = MapViewOfFile(m_physics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFilePhysics));
	if (!m_physics.mapFileBuffer)
	{
		wcout << MAP_VIEW_FAIL << endl;
		// Could use messageboxes, but better to write out instead
	}
}

void initGraphics()
{
	TCHAR szName[] = TEXT("Local\\acpmf_graphics");
	m_graphics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileGraphic), szName);
	if (!m_graphics.hMapFile)
	{
		wcout << MAP_FAILED << endl;
		// Could use messageboxes, but better to write out instead
		return;
	}
	m_graphics.mapFileBuffer = MapViewOfFile(m_graphics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileGraphic));
	if (!m_graphics.mapFileBuffer)
	{
		wcout << MAP_VIEW_FAIL << endl;
		// Could use messageboxes, but better to write out instead
	}
}

void initStatic()
{
	TCHAR szName[] = TEXT("Local\\acpmf_static");
	m_static.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileStatic), szName);
	if (!m_static.hMapFile)
	{
		wcout << MAP_FAILED << endl;
		// Could use messageboxes, but better to write out instead
		return;
	}

	m_static.mapFileBuffer = MapViewOfFile(m_static.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileStatic));
	if (!m_static.mapFileBuffer)
	{
		wcout << MAP_VIEW_FAIL << endl;
		// Could use messageboxes, but better to write out instead
	}
}


void dismiss(SMElement element)
{
	UnmapViewOfFile(element.mapFileBuffer);
	CloseHandle(element.hMapFile);
}


///////////////////////////////
// settings file
const char* SETTINGS_FILE = "relay_settings.ini";

// Mode 0: send packets as they come vailable
// Mode 1: send packets 1 to 1 with graphics packet
int sendMode = 1;

// Verbose
// 0: Only show start message and help
// 1: Show packet data
// 2: Show all messages
int verboseMode = 1;

// Sleep
int sleepInterval = 5;

boolean mapsInGlobalVariables = true;

////////////////////////
// 
const int ACC_OFF = 0;
const int ACC_REPLAY = 1;
const int ACC_LIVE = 2;
const int ACC_PAUSE = 3;
////////////////////////
// 
int ACCStatus = ACC_OFF;

std::string ip = IP;

////////////////////////
// 
std::string get_current_dir() {
	char buff[FILENAME_MAX]; //create string buffer to hold path
	char* retVal = _getcwd(buff, FILENAME_MAX);
	string current_working_dir(buff);
	return current_working_dir;
}

const char* INI = ".ini";

void readSettings() {

	int port = PORT;

	// TODO, check, if file exists, if not, create it
	///////////////////////////////////////
	// 
	// So we check the environment for the "APPDATA" directory.
	char* pValue;
	size_t len;
	errno_t err = _dupenv_s(&pValue, &len, "APPDATA");
	std::string str(pValue);

	//////////////////////////////////////
	// if it failed
	if (err) {
		wcout << "Failed to access APPDATA directory for the settings file: " << SETTINGS_FILE << endl;
		std::cout << "Using " << ip << ":" << port << " " << std::endl;
		return;
	}
	else {
		//////////////////////////////////////////////
		// create the directory in APPdata
		// and read the file or create it there
		// 
		std::string appdata = pValue;
		std::string folder(str + "\\ACC_SharedMemory_Relay");
		std::string filename = folder + "\\" + SETTINGS_FILE;

		std::wstring stemp = std::wstring(folder.begin(), folder.end());
		LPCWSTR foldername_w = stemp.c_str();

		if (CreateDirectory(foldername_w, NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			// CopyFile(...)
			//wcout << "success" << endl;
		}
		else {
			wcout << "failed to create the directory " << foldername_w << endl;

			std::cout << "Using " << ip << ":" << port << " " << std::endl;
			// Version 1.09,  this is not
			// critical, if cannot write the settings file
			// but return just in case 
			return;
			// Failed to create directory.
		}

		//strcat(appdata, "\\acc_shmem_relay\\test.html");
		//wcout << appdata.c_str() << " " << filename.c_str() << " " << endl;
		//wcout << filename.c_str() << endl;		

		////////////////////////////////////
		// If there was no settings file
		// create it with default settings
		if (!std::ifstream(filename)) {// Was SETTINGS_FILE
			std::ofstream outfile;
			outfile.open(filename); // Was SETTINGS_FILE
			outfile << "# ip address\n127.0.0.1\n# port\n9996\n# mode\n1\n# verbose\n0\n# sleep\n5";
			outfile.close();
		}

		//char ip[] = SERVER;

		std::ifstream myfile;
		wcout << "\nVersion " << (float)toolVersion << "\n For Assetto Corsa Competizione\n" << endl;

		wcout << "Reading '" << SETTINGS_FILE << "' from \n " << folder.c_str() << "\n" << endl; // Was SETTINGS_FILE

		///////////////////////////////////////////
		// Read the file, set the settings
		//
		myfile.open(filename); // Was SETTINGS_FILE
		if (myfile.is_open()) {
			std::string line;
			int currentLine = 0;
			while (myfile.good()) {
				getline(myfile, line);
				currentLine++;
				// TODO if line starts with #, do nothing

				if (currentLine == 2) {
					ip = line;
					std::cout << "IP       : " << ip << std::endl;
				}

				if (currentLine == 4) {
					port = std::stoi(line);
					PORT = port;
					std::cout << "PORT     : " << PORT << std::endl;
				}
				if (currentLine == 6) {
					sendMode = std::stoi(line);
					std::cout << "Send mode: " << sendMode << std::endl;
				}
				if (currentLine == 8) {
					//std::cout << line << endl;
					verboseMode = std::stoi(line);
					std::cout << "Verbose  : " << verboseMode << std::endl;
				}
				if (currentLine == 10) {
					sleepInterval = std::stoi(line);
					std::cout << "Sleep    : " << sleepInterval << std::endl;
				}
			}
			myfile.close();
		}
	}
	std::cout << "UDP Using: " << ip << ":" << port << " " << std::endl;
}

#pragma warning(disable:4996) 

/////////////////////////////////
// TODO V1.04, change the relay packet contents
#pragma pack(push, 1)
struct RelayVersionInfoV2
{
	byte type = 42;
	float version;
};
#pragma pack(pop)

struct sockaddr_in si_other;

int _tmain(int argc, _TCHAR* argv[])
{
	// TODO, make in V1.04 this to be the relay version packet
	RelayVersionInfoV2 versionInfov2 = { 42, toolVersion };
	//wcout << "Size of Phys " << sizeof RelayVersionInfoV2 << endl;
	//wcout << "Size of Phys " << sizeof SPageFilePhysics << endl;
	//wcout << "Size of Graph " << sizeof SPageFileGraphic << endl;
	//wcout << "Size of Stat " << sizeof SPageFileStatic << endl;
	///////////////////////////////
	//
	// Ints
	//
	readSettings();
	initPhysics();
	initGraphics();
	initStatic();
	//initUDP(PORT); // does not do anything yet.

	//struct RelayVersionInfo verbi = { 42, version };

	/////////////////////////////////
	//
	// Just show what can be done
	//
	// P   - for pause/resume data sending\n
	wcout << "\n---------------------------------"
		<< "\n  Press \n"
		//	<< "\n 1   - for physics packet" 
		//	<< "\n 2   - for graphics packet" 
		//	<< "\n 3   - for static packet" 
		<< "\n ESC - for exit\n---------------------------------"
		<< endl;

	/////////////////////////////////
	// 
	// Saved, but might not be needed
	int packetId = -1;
	int prevPacketId = -2;


	//int physicsPacketId = -2;
	int physicsPacketIdPrev = -1;
	//int graphicsPacketId = -2;
	int graphicsPacketIdPrev = -1;

	///////////////////////////////////
	//
	// Few settings
	//
	boolean silent = false;
	boolean sendPhysicsPacket = false;
	boolean sendGraphicsPacket = false;
	boolean sendStaticPacket = false;

	boolean sendPackets = true;

	////////////////////////////////////
	//
	// Initialize networking
	// We use Winsock in windows, this tool is not reayd
	// to be used on Linux
	//

	WSADATA wsa;

	/////////////////////////////////
	//Initialise winsock
	//if (!silent) {
	//	printf(" \n Initialising Winsock... "); //" \n Initialising Winsock... "
	//}
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("F : %d", WSAGetLastError()); // " Failed.Error, Code : % d "
		exit(EXIT_FAILURE);
	}
	//printf(" Initialised.\n\n");

	//////////////////////////////////////
	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		printf("G : %d", WSAGetLastError()); //" socket() failed : % d "
		exit(EXIT_FAILURE);
	}

	//////////////////////////////////////
	//setup address structure
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

	///////////////////////////////////////
	//
	// This we will always send
	//
	//wcout << " Sending data  to " << ip.c_str() << ":" << PORT << "\n-----------------------------" << endl;

	//////////////////////////////////
	//
	// Intial reading of the memory
	// 
	//if (!mapsInGlobalVariables) {
	SPageFilePhysics* pf = (SPageFilePhysics*)m_physics.mapFileBuffer;
	SPageFileGraphic* pg = (SPageFileGraphic*)m_graphics.mapFileBuffer;
	SPageFileStatic* ps = (SPageFileStatic*)m_static.mapFileBuffer;
	//}

	/////////////////////////////////
	// 
	// just in case, send one static packet
	// so we can set the track/car etc 
	//
	//if (!silent) {
	//wcout << "-#- SENT -STATIC- PACKET -#-" << endl;
	//}

	const char* message = static_cast<char*>(static_cast<void*>(ps));
	char buffer[1 + sizeof SPageFileStatic];
	//wcout << sizeof buffer << endl;
	buffer[0] = 3;
	for (int i = 0; i < sizeof SPageFileStatic; i++) {
		buffer[i + 1] = message[i];
	}

	///////////////////////////////
	// Send one static
	if (sendto(s, buffer, 1 + sizeof SPageFileStatic, 0, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR) {
		printf(SEND_TO_FAILED, WSAGetLastError()); // "sendto failed : %d"
		exit(EXIT_FAILURE);
	}

	//////////////////////////////
	// And one version info
	//////////////////////////////
	// And one version info V2
	if (sendto(s, (char*)&versionInfov2, sizeof RelayVersionInfoV2, 0, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR) {
		printf(SEND_TO_FAILED, WSAGetLastError()); // "sendto failed : %d"
		exit(EXIT_FAILURE);
	}

	//////////////////////////
	//
	// VAlues to test against, if we want
	// to send the data
	// 
	int prevPacketTest = -1;
	wchar_t carModelPrev[33];
	wchar_t trackPrev[33];
	long looped = 0;

	//bool isConsoleWindowFocussed = (GetConsoleWindow() == GetForegroundWindow());

	boolean run = true;
	boolean endText = false;
	DWORD        mode;          /* Preserved console mode */
	INPUT_RECORD event;         /* Input event */

	/* Get the console input handle */
	HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);

	/* Preserve the original console mode */
	GetConsoleMode(hstdin, &mode);

	/* Set to no line-buffering, no echo, no special-key-processing */
	SetConsoleMode(hstdin, 0);

	DWORD count;  /* ignored */
	boolean otherKey = false;
	boolean keyId = 0;

	while (run) {
		//isConsoleWindowFocussed = (GetConsoleWindow() == GetForegroundWindow());
		//wcout << looped << endl;

		sendPhysicsPacket = false;
		sendGraphicsPacket = false;
		sendStaticPacket = false;
		/*if (packetId > prevPacketId) {
			wcout << "here we run it";
			prevPacketId = packetId;
		}*/
		////////////////////////////////////////
		//
		// read the memory
		//
		if (mapsInGlobalVariables) {
			pf = (SPageFilePhysics*)m_physics.mapFileBuffer;
			pg = (SPageFileGraphic*)m_graphics.mapFileBuffer;
			ps = (SPageFileStatic*)m_static.mapFileBuffer;
		}
		else {
			SPageFilePhysics* pf = (SPageFilePhysics*)m_physics.mapFileBuffer;
			SPageFileGraphic* pg = (SPageFileGraphic*)m_graphics.mapFileBuffer;
			SPageFileStatic* ps = (SPageFileStatic*)m_static.mapFileBuffer;
		}

		if (!m_graphics.hMapFile) {
			wcout << MAP_FAILED << endl;
			//MessageBoxA(GetActiveWindow(), "a ", ACC_SHM, MB_OK); // " CreateFileMapping failed"
		}
		if (!m_graphics.mapFileBuffer) {
			wcout << MAP_VIEW_FAIL << endl;
			//MessageBoxA(GetActiveWindow(), "b ", ACC_SHM, MB_OK); //" MapViewOfFile failed"
		}


		/////////////////////////////////////////
		// Get the ACC status
		// 0, offline, 1 menu, 2 active
		ACCStatus = pg->status;

		///////////////////////////////////////
		//
		// Lap fractions changed in the Graphics packet
		// so we send new packet. The system can generate
		// several physics packages per single graphics package
		// so we limit this to one phys per one graphics package
		// 
		int packetTest = pg->packetId;

		////////////////////////////////
		//
		//
			//normalizedCarPosition;
		// MATCH Packets
		// i.e. send packets timed by the Graphics packet 
		//(has the lapFraction in it)
		if (sendMode == 1) {
			if (packetTest != prevPacketTest) {
				prevPacketTest = packetTest;
				//sendPacket = true;
				sendPhysicsPacket = true;
				sendGraphicsPacket = true;
				// TODO, some way to send this less frequently
				// Once every 20 seconds in avg, 100 packets per second sent
				// in my system
				if (pg->packetId % 2000 == 0) {
					sendStaticPacket = true;
				}
			}
		}

		if (sendMode == 0) { // send always packets, when new packet is available			
			// TODO, add here a check for the 
			if (pf->packetId != physicsPacketIdPrev) {
				sendPhysicsPacket = true;
				physicsPacketIdPrev = pf->packetId;
			}
			// at all graphPacket send also static
			if (pg->packetId != graphicsPacketIdPrev) {
				sendGraphicsPacket = true;
				if (pg->packetId % 2000 == 0) {
					sendStaticPacket = true;
				}
				graphicsPacketIdPrev = pg->packetId;
			}
		}

		////////////////////////////////////////
		//
		// ESC key pressed to exit the app
		//
		// Key codes https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
		//
		// GetAsyncKeyState(VK_ESC0x1B) 
		//endText = true;
		//if (GetConsoleWindow() == GetForegroundWindow())  // Only if the console is in focus
		//{
		//	if (!run && endText) {
		//		wcout << looped << " in focus " << endl;
		//	}
		//	
		//	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { // user pressed ESC TODO, change to key q or (E)xit
		//		run = false;
		//		break;
		//		//exit(0);
		//	}
		//}

		//////////////////////////////////////////
		//
		// Alternative to avoid virus flagging
		if (GetConsoleWindow() == GetForegroundWindow()) {
			if (_kbhit()) {

				/* Get the input event */
				ReadConsoleInput(hstdin, &event, 1, &count);

				/* Only respond to key release events */
				if ((event.EventType == KEY_EVENT)
					&& event.Event.KeyEvent.bKeyDown)
					switch (event.Event.KeyEvent.wVirtualKeyCode)
					{
					case VK_ESCAPE:
						run = false;
						break;
					case VK_F1:
						keyId = 1;
						break;
					case VK_F2:
						keyId = 2;
						break;
					case VK_F3:
						keyId = 3;
						break;
					default:
						// do nothin;
						otherKey = true;
						break;
					}
			}
		}


		///////////////////////////
		// 
		// Send physics packet
		// 
		if (sendPhysicsPacket && sendPackets) {
			if (!silent && verboseMode > 0) {
				wcout << "-#- SENT -PHYSICS- PACKET -#-" << endl;
			}

			//////////////////////////////////////////////
			// Adds number 1 to the head of the buffer
			// So we can easily detect the packet 
			char buffer[1 + sizeof SPageFilePhysics];
			buffer[0] = 1;
			const char* message = static_cast<char*>(static_cast<void*>(pf));
			for (int i = 0; i < sizeof SPageFilePhysics; i++) {
				buffer[i + 1] = message[i];
			}

			if (sendto(s, buffer, 1 + sizeof SPageFilePhysics, 0, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR)
			{
				printf(SEND_TO_FAILED, WSAGetLastError()); // "sendto() failed with error code : % d "
				exit(EXIT_FAILURE);
			}

			physicsPacketIdPrev = pf->packetId;
		}

		///////////////////////////////////
		//
		// Send graphics packet
		// 
		if (sendGraphicsPacket && sendPackets) {
			if (!silent && verboseMode > 0) {
				wcout << "-#- SENT -GRAPHICS- PACKET -#-" << endl;
			}

			/////////////////////////////////////
			//
			//const char* type = "2";
			const char* message = static_cast<char*>(static_cast<void*>(pg));
			char buffer[1 + sizeof SPageFileGraphic];
			//wcout << sizeof buffer << endl;
			//////////////////////////////////////////////
			// Adds number 2 to the head of the buffer
			// So we can easily detect the packet 
			buffer[0] = 2;
			for (int i = 0; i < sizeof SPageFileGraphic; i++) {
				buffer[i + 1] = message[i];
			}

			//if (sendto(s, static_cast<char*>(static_cast<void*>(pg)), sizeof SPageFileGraphic, 0, (struct sockaddr*)& si_other, sizeof(si_other)) == SOCKET_ERROR)
			if (sendto(s, buffer, 1 + sizeof SPageFileGraphic, 0, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR)
			{
				printf(SEND_TO_FAILED, WSAGetLastError()); //"sendto failed : %d"
				exit(EXIT_FAILURE);
			}
			graphicsPacketIdPrev = pg->packetId;
		}


		/////////////////////////////////////
		// 
		// Jus save the carMode and track
		// if we might want to do something with them
		//
		wchar_t carModelCurrent[33];
		wcscpy_s(carModelCurrent, ps->carModel);
		carModelCurrent[32] = L'\0';

		wchar_t trackCurrent[33];
		wcscpy_s(trackCurrent, ps->track);
		trackCurrent[32] = L'\0';

		//////////////////////////
		// TODO , add here more rules, when to send the
		// static packet
		// 
		if ((looped % 50000 == 0)
			|| ((graphicsPacketIdPrev % 113 == 2) || wcscmp(carModelPrev, carModelCurrent) != 0 || (wcscmp(trackPrev, trackCurrent) != 0)
				)) {
			sendStaticPacket = true;
		}
		if (ACCStatus == ACC_OFF) {
			sendStaticPacket = false;
		}


		/////////////////////////////////////
		//
		// Static file
		//
		//ps = (SPageFileStatic*)m_static.mapFileBuffer;
		if (sendStaticPacket && sendPackets) {
			if (!silent && verboseMode > 0) {
				wcout << "-#- SENT -STATIC- PACKET -#-" << endl;
			}

			/////////////////////////////////////
			//
			const char* message = static_cast<char*>(static_cast<void*>(ps));
			char buffer[1 + sizeof SPageFileStatic];
			//wcout << sizeof buffer << endl;
			//
			// And here we add number 3 to the buffer[0], so we can
			// use it to detect the packet in the tool
			//
			buffer[0] = 3;
			for (int i = 0; i < sizeof SPageFileStatic; i++) {
				buffer[i + 1] = message[i];
			}

			if (sendto(s, buffer, 1 + sizeof SPageFileStatic, 0, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR)
			{
				printf(SEND_TO_FAILED, WSAGetLastError()); // "sendto failed : %d"
				exit(EXIT_FAILURE);
			}
			wcscpy_s(carModelPrev, ps->carModel);
			wcscpy_s(trackPrev, ps->track);
		}


		/////////////////////////////////////
	//
	//
	// We send the relay version packet too
		// so the receiver can check the version
		// if needed. 
		// Relay version has id 42 as buffer[0]

		if (sendStaticPacket && sendPackets) {
			if (sendto(s, (char*)&versionInfov2, sizeof RelayVersionInfoV2, 0, (struct sockaddr*)&si_other, sizeof(si_other)) == SOCKET_ERROR) {
				printf(SEND_TO_FAILED, WSAGetLastError()); //"sendto failed : %d"
				exit(EXIT_FAILURE);
			}
		}
		// 1/200th of second, could be also 10 (1/100th of second)
		// We expect to run the loop some 50 to 100 times per second
		// So this keeps it clean and not eating all CPU
		Sleep(sleepInterval);  // wait 5 ms
		//ACCStatus = ACC_OFF;
		looped++;

	}

	/////////////////////////////////////
	//
	if (endText || looped == 1 || looped == 0) {
		wcout << THE_END << endl;
		wcout << PRESSED_ESC << endl;
		//wcout << AV_NOTE << endl;
		wcout << ENJOYED << endl;
		wcout << MORE_TEXT << ACC << endl;
		if (GetConsoleWindow() == GetForegroundWindow()) {
			wcout << NO_NEED << F12019 << ", " << F12018 << ", " << F12017 << OR << PC2 << " " << endl;
		}
	}
	/////////////////////////////////////
	//
	//
	//
	dismiss(m_graphics);
	dismiss(m_physics);
	dismiss(m_static);

	//wcout << "\n\n\nCLOSING THE APP" << endl;
	// Set the console back to what it was
	SetConsoleMode(hstdin, mode);

	/////////////////////////////////////
	//
	//
	//
	return 0;
	//exit(0);
}


