#pragma once
#include <string>

using namespace std;

class Server
{
private:

	string serverFolderPath;
	bool isServerRunning;
	int playersOnline;
	int serverVersion;


public:

	Server();

	Server(string);

	void startServer();

	void stopServer();

	void updateServer();



};

