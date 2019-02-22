#include "WebSocketClient.h"


DDRFramework::connection_metadata::connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri, websocket_endpoint* pendpoint) : m_id(id)
, m_hdl(hdl)
, m_status("Connecting")
, m_uri(uri)
, m_server("N/A")
, m_pendpoint(pendpoint)
{

}





void DDRFramework::connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl)
{
	m_status = "Open";

	client::connection_ptr con = c->get_con_from_hdl(hdl);
	m_server = con->get_response_header("Server");
}

void DDRFramework::connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl)
{
	m_status = "Failed";

	client::connection_ptr con = c->get_con_from_hdl(hdl);
	m_server = con->get_response_header("Server");
	m_error_reason = con->get_ec().message();
}

void DDRFramework::connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl)
{
	m_status = "Closed";
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::stringstream s;
	s << "close code: " << con->get_remote_close_code() << " ("
		<< websocketpp::close::status::get_string(con->get_remote_close_code())
		<< "), close reason: " << con->get_remote_close_reason();
	m_error_reason = s.str();
}

void DDRFramework::connection_metadata::on_message(websocketpp::connection_hdl, client::message_ptr msg)
{
	/*if (msg->get_opcode() == websocketpp::frame::opcode::text) {
		m_messages.push_back("<< " + msg->get_payload());
	}
	else {
		m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
	}*/
	if (m_pendpoint)
	{
		m_pendpoint->onmessage(msg->get_payload());
	}
}

int DDRFramework::connection_metadata::get_id() const
{
	return m_id;
}

std::string DDRFramework::connection_metadata::get_status() const
{
	return m_status;
}




