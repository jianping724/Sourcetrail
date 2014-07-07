#include "component/controller/SearchController.h"

#include "component/view/SearchView.h"
#include "data/access/GraphAccess.h"

SearchController::SearchController(std::shared_ptr<GraphAccess> graphAccess)
	: m_graphAccess(graphAccess)
{
}

SearchController::~SearchController()
{
}

void SearchController::search(const std::string& s)
{
	LOG_INFO("searching string: \"" + s + "\"");
	Id nodeId = m_graphAccess->getIdForNodeWithName(s);
	if (nodeId > 0)
	{
		LOG_INFO("Node with name \"" + s + "\" found.");
		MessageActivateToken message(nodeId);
		message.dispatch();
	}
	else
	{
		LOG_INFO("Node with name \"" + s + "\" not found.");
	}
}

void SearchController::handleMessage(MessageActivateToken* message)
{
	getView()->setText(m_graphAccess->getNameForNodeWithId(message->tokenId));
}

void SearchController::handleMessage(MessageFinishedParsing* message)
{
	getView()->setAutocompletionList(m_graphAccess->getNamesForNodesWithNamePrefix(""));
}

SearchView* SearchController::getView()
{
	return Controller::getView<SearchView>();
}
