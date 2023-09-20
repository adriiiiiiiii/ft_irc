#include "Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password) {}

Server::~Server() {}

bool Server::run()
{
	if ((_socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // 1-familia de direcciones(ipv4) 2-tipo de socket (tipo orientado a protocolo TCP) 3-protocolo (automatico, TCP)
		return (std::cout << "Error: could not create the sockets.." << std::endl, false);

	struct sockaddr_in serverAddress;
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;					// specing the family, interenet (address)
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);	// responding to anything
	serverAddress.sin_port = htons(_port);				// convert server port nb to network standart byte order (to avoid to conections use different byte order)

	if (bind(_socketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // set the address wher is gonna be listening
		return (std::cout << "Error: could not bind.." << std::endl, false);

	std::cout << "Waiting for a connection in '127.0.0.1' port: " << _port << std::endl; // localhost ip default = 127.0.0.1

	if (listen(_socketFd, serverAddress.sin_port) < 0)
		return (std::cout << "Error: trying to listeng" << std::endl, false);

	// fcntl(_socketFd, F_SETFL, O_NONBLOCK); // avoid system differences

	_pollFd[0].fd = _socketFd;
	_pollFd[0].events = POLLIN;
	_pollFd[0].revents = 0;

	while (true)
		if (handleClientConnections() == false)
			return (false);
}

std::vector<Client>::iterator Server::getClientByFd(int fd)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->getFd() == fd)
			return it;
	}
	return _clients.end();
}


bool Server::handleClientConnections()
{
	if (poll(_pollFd, _clients.size() + 1, -1) < 0)
		return (std::cout << "Error: syscall poll failed.." << std::endl, false);

	if (_pollFd[0].revents == POLLIN)
	{
		std::cout << "Incomming connecction..." << std::endl;
		_connectionFd = accept(_socketFd, (struct sockaddr *) NULL, NULL);
		if (_connectionFd == -1)
			return (std::cout << "Error accepting client's connection" << std::endl, false);

		//fcntl(_SocketFd, F_SETFL, O_NONBLOCK); // avoid system differences

		if (this->_clients.size() >= BACKLOG)
			return (std::cout << "Error: max connections limit reached" << std::endl, true);

		std::cout << "[SERVER]: A new connection has been made." << std::endl;

		_clients.push_back(Client(_connectionFd));

		// Saves the new connection
		size_t i = 1;
		while (i <= _clients.size() && _pollFd[i].fd != -1)
			i++;
		_pollFd[i].fd = _connectionFd;
		_pollFd[i].events = POLLIN;
	}

	handleClientCommunications();
	return (true);
}

void Server::handleClientCommunications()
{
	for (size_t i = 1; i <= _clients.size(); i++)
	{
		if (_pollFd[i].fd == -1)
			continue;

		if (_pollFd[i].revents == POLLIN)
		{
			char buffer[BUFFER_SIZE + 1];
			size_t readSize = read(_pollFd[i].fd, buffer, BUFFER_SIZE);
			if (readSize < 0)
				throw std::runtime_error("A Client Socket can't be read from");

			buffer[readSize] = '\0';

			if (readSize == 0)
				disconnectClient(_pollFd[i].fd);
			else
			{
				std::vector<Client>::iterator caller = getClientByFd(_pollFd[i].fd);
				if (caller == _clients.end())
				{
					std::cout << "[SERVER :: WARNING]: getClientByFd() failed before executing a command" << std::endl;
					continue;
				}
				// TODO: Handle not-ended inputs (see subject)
				this->handleClientInput(*caller, buffer);
			}
		}
	}
}

void Server::disconnectClient(int clientFd)
{
	std::cout << "[SERVER]: A Client was disconnected from the server" << std::endl;
	// Closes the FD (AKA: closes the connection)
	close(clientFd);

	// Removes the client from the Vector of Clients
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->getFd() == clientFd)
		{
			_clients.erase(it);
			break;
		}
	}

	// Remove the Client FD from the backlog
	for (size_t i = 0; i < BACKLOG; i++)
	{
		if (_pollFd[i + 1].fd == clientFd)
			_pollFd[i + 1].fd = -1;
		_pollFd[i].revents = 0;
	}
}

// server_utils.cpp
// void handleCommand(Client caller, std::string command, std::string body);

void Server::handleClientInput(Client &caller, std::string message)
{
	(void)caller;
	size_t spaceSeparator = message.find(' ');
	std::string command = (spaceSeparator == std::string::npos) ? message : message.substr(0, spaceSeparator);
	std::string body = (spaceSeparator == std::string::npos) ? IRC_ENDLINE : message.substr(spaceSeparator + 1);

	size_t endlinePosition = body.find(IRC_ENDLINE);
	if (endlinePosition != std::string::npos) // If the message does not end with '\r\n' should be ignored, but for now we accept it. TODO: change this
		body = body.substr(0, endlinePosition);

	handleCommand(caller, command, body);
}
