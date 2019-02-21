#include "WebSocketClient.h"
bool DDRFramework::WebSocketClient::Connect(std::string uri)
{
	m_CurrentID = m_EndPoint.connect(uri);
	if (m_CurrentID != -1) {
		std::cout << "> Created connection with id " << m_CurrentID << std::endl;
		return true;
	}
	return false;
}

void DDRFramework::WebSocketClient::Close()
{
	if (m_CurrentID != -1) {
		int close_code = websocketpp::close::status::normal;
		m_EndPoint.close(m_CurrentID, close_code, "");
	}
}

void DDRFramework::WebSocketClient::Send(std::string msg)
{
	if (m_CurrentID != -1) {
		m_EndPoint.send(m_CurrentID, msg);
	}
}

void DDRFramework::WebSocketClient::Show()
{
	if (m_CurrentID != -1) {
		connection_metadata::ptr metadata = m_EndPoint.get_metadata(m_CurrentID);
		if (metadata) {
			std::cout << *metadata << std::endl;
		}
		else {
			std::cout << "> Unknown connection id " << m_CurrentID << std::endl;
		}
	}
}

std::ostream & DDRFramework::operator<<(std::ostream & out, connection_metadata const & data)
{
	out << "> URI: " << data.m_uri << "\n"
		<< "> Status: " << data.m_status << "\n"
		<< "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
		<< "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";
	out << "> Messages Processed: (" << data.m_messages.size() << ") \n";

	std::vector<std::string>::const_iterator it;
	for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
		out << *it << "\n";
	}

	return out;
}

