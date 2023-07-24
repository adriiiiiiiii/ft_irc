#include <Channel.hpp>

Channel::Channel() : topic("") {}
Channel::~Channel() {}

std::vector<std::string> Channel::getParticipants() const
{
	return this->participants;
}

std::string Channel::getTopic() const
{
	return this->topic;
}

void Channel::setTopic(std::string newTopic)
{
	this->topic = newTopic;
}


void Channel::addParticipant(std::string newParticipant)
{
	participants.push_back(newParticipant);
}

void Channel::removeParticipant(std::string participant)
{
	std::vector<std::string>::iterator userIt = std::find(participants.begin(), participants.end(), participant);
    if (userIt != participants.end())
        participants.erase(userIt);
}

void Channel::addOperator(std::string newOperator)
{
	operators.push_back(newOperator);
}

void Channel::removeOperator(std::string operatorName)
{
	std::vector<std::string>::iterator userIt = std::find(operators.begin(), operators.end(), operatorName);
    if (userIt != operators.end())
        operators.erase(userIt);
}

bool Channel::hasUser(std::string participant)
{
	std::vector<std::string>::iterator userIt = std::find(participants.begin(), participants.end(), participant);
    return (userIt != participants.end());
}

bool Channel::hasOperator(std::string operatorName)
{
	std::vector<std::string>::iterator userIt = std::find(operators.begin(), operators.end(), operatorName);
    return (userIt != operators.end());
}
