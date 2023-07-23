#include "../server/Server.hpp"

// :<server> 352 <user> <username> <hostname> <server_name> <nickname> <H|G>[*][@|+] :<hopcount> <real_name>
// :<server> 315 <user> :End of WHO list
void Server::usersOnNetwork(std::string param, int clientFd)
{
	param = "";
	std::string serverName = "server_name";

	std::vector<Client>::iterator it = findClientByFd(clientFd);
	if (it == clients.end())
	{
		std::cerr << "Client not found" << std::endl;
		return;
	}
	Client requestingClient = *it;
	// std::string username = requestingClient.getUsername();
	std::string username = "pepe";
	std::string userNickname = requestingClient.getNickname();
	// std::string hostname = requestingClient.getHostname();
	std::string hostname = "pepe";
	std::string nickname = requestingClient.getNickname();

	std::string channelStatus = "";
	// Logic to determine the channel status based on the user's membership in channels...

	std::string whoMessage = ":" + serverName + " 352 " + userNickname + " * " + username + " " + hostname + " " +
							 serverName + " " + nickname + " " + channelStatus + " :1 ";
	int retValue = send(clientFd, whoMessage.c_str(), whoMessage.size(), 0);
	if (retValue == -1)
	{
		std::cerr << "[SERVER-error]: send failed " << errno << strerror(errno) << std::endl;
		return;
	}
	std::string endOfWhoMessage = ":" + serverName + " 315 " + userNickname + " :End of WHO list";
	retValue = send(clientFd, endOfWhoMessage.c_str(), endOfWhoMessage.size(), 0);
	if (retValue == -1)
		std::cerr << "[SERVER-error]: send failed " << errno << strerror(errno) << std::endl;
}

void Server::getUserInfo(std::string targetNickname, int clientFd)
{
	Client* targetClient = findClientByNickname(targetNickname);
	if (targetClient != NULL)
	{
		std::string errorMessage = ":server_name 401 " + targetNickname + " :No such nick/channel\r\n";
		int retValue = send(clientFd, errorMessage.c_str(), errorMessage.size(), 0);
		if (retValue == -1)
			std::cerr << "[SERVER-error]: send failed " << errno << strerror(errno) << std::endl;
		return;
	}

	std::string username = targetClient->getNickname();
	std::string hostname = "pepe";
	// std::string hostname = targetClient.getHostname();
	std::string realname = "pepe";
	// std::string realname = targetClient.getRealname();

	// Send the WHOIS response to the client
	std::string whoisMessage = ":server_name 311 " + targetNickname + " " + targetNickname + " " + username + " " + hostname + " * :" + realname + "\r\n";
	int retValue = send(clientFd, whoisMessage.c_str(), whoisMessage.size(), 0);
	if (retValue == -1)
		std::cerr << "[SERVER-error]: send failed " << errno << strerror(errno) << std::endl;

	std::string endOfWhoisMessage = ":server_name 318 " + targetNickname + " " + targetNickname + " :End of WHOIS list\r\n";
	retValue = send(clientFd, endOfWhoisMessage.c_str(), endOfWhoisMessage.size(), 0);
	if (retValue == -1)
		std::cerr << "[SERVER-error]: send failed " << errno << strerror(errno) << std::endl;
}

