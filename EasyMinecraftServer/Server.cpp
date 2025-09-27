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
	curlpp::Easy myRequest;

	myRequest.setOpt<curlpp::options::Url>("https://launchermeta.mojang.com/mc/game/version_manifest.json");

	std::ostringstream response;

	curlpp::options::WriteStream ws(&response);

	myRequest.setOpt(ws);

	myRequest.perform();

	json j = json::parse(response.str());

	string latestVersion = j["latest"]["release"];

	for (json::iterator i = j["versions"].begin(); i != j["versions"].end(); ++i)
	{
		//std::cout << *i;
		json element = *i;
		if (element["id"] == latestVersion)
		{
			//std::cout << element;
			std::cout << element["url"] << endl;
			myRequest.setOpt<curlpp::options::Url>(element["url"]);
		}
	}

	response.str("");
	response.clear();
	myRequest.perform();
	j.clear();

	j = json::parse(response.str());


	std::cout << j["downloads"]["server"]["url"] << endl;

	myRequest.setOpt<curlpp::options::Url>(j["downloads"]["server"]["url"]);

	std::ofstream serverFile("server.jar", std::ios::binary);
	if (serverFile.is_open())
	{
		curlpp::options::WriteStream ws(&serverFile);
		myRequest.setOpt(ws);
		myRequest.perform();
	}
	else
	{

	}
}