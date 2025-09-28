#include "Server.h"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdio>
#include <thread>
#include <list>
#include <algorithm>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <nlohmann/json.hpp>

#if defined(_WIN32)
	#define popen _popen
	#define pclose _pclose
#endif

using namespace std;

using json = nlohmann::json;

Server::Server() 
{
	Server::isServerRunning = 0;
	Server::serverFolderPath = "test";
	Server::playersOnline = 0;
	Server::serverVersion = 0;
	Server::tMC;
}

Server::Server(std::string path) 
{
	Server::isServerRunning = 0;
	Server::serverFolderPath = path;
	Server::playersOnline = 0;
	Server::serverVersion = 0;
	Server::tMC;
}

void Server::startServer()
{
	filesystem::path currentDir = filesystem::current_path();
	filesystem::current_path(this->serverFolderPath);

	//cout << filesystem::current_path() << endl;

	HANDLE g_hChildStd_IN_Rd = NULL; // Child's STDIN read handle
	HANDLE g_hChildStd_IN_Wr = NULL; // Child's STDIN write handle
	HANDLE g_hChildStd_OUT_Rd = NULL; // Child's STDOUT read handle
	HANDLE g_hChildStd_OUT_Wr = NULL; // Child's STDOUT write handle
	SECURITY_ATTRIBUTES saAttr; // Security attributes for pipe handles

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create pipe for child's STDOUT (parent reads, child writes)
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
		// Handle error
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited by the child
	// as the parent will be reading from it.
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
		// Handle error
	}

	// Create pipe for child's STDIN (parent writes, child reads)
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
		// Handle error
	}

	// Ensure the write handle to the pipe for STDIN is not inherited by the child
	// as the parent will be writing to it.
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
		// Handle error
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = g_hChildStd_IN_Rd;   // Child reads from this
	si.hStdOutput = g_hChildStd_OUT_Wr; // Child writes to this
	si.hStdError = g_hChildStd_OUT_Wr; // Redirect stderr to stdout pipe as well

	ZeroMemory(&pi, sizeof(pi));
	//system("start \"\"java -jar server.jar nogui");
	//system("java -jar server.jar nogui");

	wstring cmd = L"java -jar server.jar nogui";

	if (!CreateProcess(
		NULL,
		const_cast<LPWSTR>(cmd.c_str()), // Command line
		NULL,
		NULL,
		TRUE, // Inherit handles
		0,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		std::cerr << "CreateProcess failed." << std::endl;
	}

	// Close handles that the parent doesn't need
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(g_hChildStd_OUT_Wr);

	//cout << pi.hProcess << endl;
	//cout << pi.hThread << endl;
	
	// Example: Write to child's STDIN
	//std::string dataToSend = "Hello from parent!";
	//DWORD bytesWritten;
	//WriteFile(g_hChildStd_IN_Wr, dataToSend.c_str(), dataToSend.length(), &bytesWritten, NULL);

	this->tMC = thread(&Server::readServer, this, ref(g_hChildStd_OUT_Rd));

	// Example: Read from child's STDOUT
	//char buffer[256];
	//DWORD bytesRead;
	//ReadFile(g_hChildStd_OUT_Rd, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
	//if (bytesRead > 0) {
	//	buffer[bytesRead] = '\0';
	//	std::cout << "Child output: " << buffer << std::endl;
	//}

	//FILE* pipe = popen


	filesystem::current_path(currentDir);

	//std::cout << filesystem::current_path() << endl;

	//std::cout << "Im still running!!!" << std::endl;

	if (this->tMC.joinable())
	{
		this->tMC.join();
	}
}

void Server::updateServer(bool force = false)
{
	// Create HTTP request object
	curlpp::Easy myRequest;
	// Set target URL
	myRequest.setOpt<curlpp::options::Url>("https://launchermeta.mojang.com/mc/game/version_manifest.json");

	// Set output to write to a response object
	std::ostringstream response;
	curlpp::options::WriteStream ws(&response);
	myRequest.setOpt(ws);

	// Run request
	myRequest.perform();

	// Parse the response to a json object
	json j = json::parse(response.str());

	// Find latest version number
	std::string latestVersion = j["latest"]["release"];

	cout << "Latest Version of MC Server is " + latestVersion << endl;

	// If server is not latest -> update or If server.jar does not exist.
	if (force || !filesystem::exists(this->serverFolderPath + "/server.jar"))
	{
		// Iterate to find the correct element containing the correct version manifest
		for (json::iterator i = j["versions"].begin(); i != j["versions"].end(); ++i)
		{
			//std::cout << *i;
			json element = *i;
			if (element["id"] == latestVersion)
			{
				//std::cout << element;
				//std::cout << element["url"] << std::endl;
				// Set target url to the version manifest
				myRequest.setOpt<curlpp::options::Url>(element["url"]);
			}
		}

		// Clear the response object
		response.str("");
		response.clear();
		// Run request
		myRequest.perform();
		// Clear the json object and parse the new response
		j.clear();
		j = json::parse(response.str());


		//std::cout << j["downloads"]["server"]["url"] << std::endl;

		// Set target url to the server.jar file
		myRequest.setOpt<curlpp::options::Url>(j["downloads"]["server"]["url"]);


		// Check for folder path. Create if it does not exist
		if (!std::filesystem::exists(this->serverFolderPath))
		{
			std::filesystem::create_directory(this->serverFolderPath);
		}

		// Download the server.jar file and write to file
		std::ofstream serverFile(this->serverFolderPath + "/server.jar", std::ios::binary);
		if (serverFile.is_open())
		{
			cout << "Downloading..." << endl;
			curlpp::options::WriteStream ws(&serverFile);
			myRequest.setOpt(ws);
			myRequest.perform();
			cout << "Done" << endl;
		}
		else
		{
			std::cout << "File \"server.jar\" could not be opened." << std::endl;
		}
	}


}

void Server::readServer(HANDLE &strOutRd)
{
	while (true)
	{
		// Example: Read from child's STDOUT
		char buffer[256];
		DWORD bytesRead;
		ReadFile(strOutRd, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
		if (bytesRead > 0) {
			buffer[bytesRead] = '\0';
			std::cout << buffer;
		}
	}
}

int Server::readEula()
{
	ifstream eulaIn(this->serverFolderPath + "/eula.txt");

	bool writeToEula = false;

	string line;
	list<string> contents;

	string searchString = "eula=false";

	if (!eulaIn.is_open()) 
	{
		std::cerr << "Error opening eula!" << std::endl;
		return 1;
	}
	else
	{
		while (getline(eulaIn, line))
		{
			contents.push_back(line);
		}

		eulaIn.close();

		// Iterates through list and changes the correct value.
		for (list<string>::iterator i = contents.begin(); i != contents.end(); ++i)
		{
			if (*i == searchString)
			{
				*i = "eula=true";
				writeToEula = true;
			}
		}
	}

	if (writeToEula)
	{
		ofstream eulaOut(this->serverFolderPath + "/eula.txt");

		if (!eulaOut.is_open())
		{
			std::cerr << "Error opening eula!" << std::endl;
			return 1;
		}
		else 
		{
			// Iterates through list and changes the correct value.
			for (list<string>::iterator i = contents.begin(); i != contents.end(); ++i)
			{
				eulaOut << *i << endl;
			}

			eulaOut.close();
		}

	}

	return 0;

	// Finds string in list.
	//auto theOne = find(contents.begin(), contents.end(), searchString);

	//if (theOne != contents.end())
	//{
	//	cout << *theOne << endl;
	//}
	//else
	//{
	//	cout << "not found" << endl;
	//}
}