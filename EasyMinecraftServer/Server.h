#pragma once
#include <string>

using namespace std;

class Server
{
private:

	string serverFolderPath;
	bool serverRunning;
	int playersOnline;
	int serverVersion;


public:

	Server();

	void startServer();

	void stopServer();

	void updateServer();



};

