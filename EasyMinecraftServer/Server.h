#pragma once
#include <string>
#include <thread>
#include <windows.h>
//#include <filesystem>

// using namespace std;

class Server
{
private:

	std::string serverFolderPath;
	bool isServerRunning;
	int playersOnline;
	int serverVersion;
	std::thread tMC;


public:

	Server();

	Server(std::string);

	void startServer();

	void stopServer();

	void updateServer(bool);

	void readServer(HANDLE&);

	int readEula();

};

