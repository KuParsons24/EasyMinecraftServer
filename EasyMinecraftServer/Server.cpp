#include "Server.h"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Server::Server() 
{
	Server::isServerRunning = 0;
	Server::serverFolderPath = "";
	Server::playersOnline = 0;
	Server::serverVersion = 0;
}

Server::Server(string path) 
{
	Server::isServerRunning = 0;
	Server::serverFolderPath = path;
	Server::playersOnline = 0;
	Server::serverVersion = 0;
}

void Server::startServer()
{


}

void Server::updateServer()
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
	string latestVersion = j["latest"]["release"];

	// Iterate to find the correct element containing the correct version manifest
	for (json::iterator i = j["versions"].begin(); i != j["versions"].end(); ++i)
	{
		//std::cout << *i;
		json element = *i;
		if (element["id"] == latestVersion)
		{
			//std::cout << element;
			std::cout << element["url"] << endl;
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


	std::cout << j["downloads"]["server"]["url"] << endl;

	// Set target url to the server.jar file
	myRequest.setOpt<curlpp::options::Url>(j["downloads"]["server"]["url"]);

	// Download the server.jar file and write to file
	std::ofstream serverFile("server.jar", std::ios::binary);
	if (serverFile.is_open())
	{
		curlpp::options::WriteStream ws(&serverFile);
		myRequest.setOpt(ws);
		myRequest.perform();
	}
	else
	{
		std::cout << "File \"server.jar\" could not be opened." << endl;
	}
}