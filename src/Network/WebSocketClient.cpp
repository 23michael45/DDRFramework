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

	if (m_pendpoint)
	{
		m_pendpoint->onconnect();
	}
}

void DDRFramework::connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl)
{
	m_status = "Failed";

	client::connection_ptr con = c->get_con_from_hdl(hdl);
	m_server = con->get_response_header("Server");
	m_error_reason = con->get_ec().message();

	if (m_pendpoint)
	{
		m_pendpoint->onfailed();
	}
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

	if (m_pendpoint)
	{
		m_pendpoint->onclose();
	}
}

void DDRFramework::connection_metadata::on_interrupt(client * c, websocketpp::connection_hdl hdl)
{
	m_status = "Interrupt";
	client::connection_ptr con = c->get_con_from_hdl(hdl);
	std::stringstream s;
	s << "close code: " << con->get_remote_close_code() << " ("
		<< websocketpp::close::status::get_string(con->get_remote_close_code())
		<< "), close reason: " << con->get_remote_close_reason();
	m_error_reason = s.str();
	if (m_pendpoint)
	{
		m_pendpoint->oninterrupt();
	}
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




DDRFramework::websocket_endpoint::websocket_endpoint() : m_next_id(0)
{
	m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
	m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

	m_endpoint.init_asio();
	m_endpoint.start_perpetual();

	m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
}

DDRFramework::websocket_endpoint::~websocket_endpoint()
{
	m_endpoint.stop_perpetual();

	for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
		if (it->second->get_status() != "Open") {
			// Only close open connections
			continue;
		}

		std::cout << "> Closing connection " << it->second->get_id() << std::endl;

		websocketpp::lib::error_code ec;
		m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
		if (ec) {
			std::cout << "> Error closing connection " << it->second->get_id() << ": "
				<< ec.message() << std::endl;
		}
	}

	m_thread->join();
}

int DDRFramework::websocket_endpoint::connect(std::string const & uri)
{
	websocketpp::lib::error_code ec;

	client::connection_ptr con = m_endpoint.get_connection(uri, ec);

	if (ec) {
		std::cout << "> Connect initialization error: " << ec.message() << std::endl;
		return -1;
	}

	int new_id = m_next_id++;
	connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, con->get_handle(), uri, this);
	m_connection_list[new_id] = metadata_ptr;

	con->set_open_handler(websocketpp::lib::bind(
		&connection_metadata::on_open,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_fail_handler(websocketpp::lib::bind(
		&connection_metadata::on_fail,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_close_handler(websocketpp::lib::bind(
		&connection_metadata::on_close,
		metadata_ptr,
		&m_endpoint,
		websocketpp::lib::placeholders::_1
	));
	con->set_message_handler(websocketpp::lib::bind(
		&connection_metadata::on_message,
		metadata_ptr,
		websocketpp::lib::placeholders::_1,
		websocketpp::lib::placeholders::_2
	));

	m_endpoint.connect(con);

	return new_id;
}

void DDRFramework::websocket_endpoint::close(int id, websocketpp::close::status::value code, std::string reason)
{
	websocketpp::lib::error_code ec;

	con_list::iterator metadata_it = m_connection_list.find(id);
	if (metadata_it == m_connection_list.end()) {
		std::cout << "> No connection found with id " << id << std::endl;
		return;
	}

	m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
	if (ec) {
		std::cout << "> Error initiating close: " << ec.message() << std::endl;
	}
}

void DDRFramework::websocket_endpoint::send(int id, std::string message)
{
	websocketpp::lib::error_code ec;

	con_list::iterator metadata_it = m_connection_list.find(id);
	if (metadata_it == m_connection_list.end()) {
		std::cout << "> No connection found with id " << id << std::endl;
		return;
	}

	m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
	if (ec) {
		std::cout << "> Error sending message: " << ec.message() << std::endl;
		return;
	}
}

DDRFramework::connection_metadata::ptr DDRFramework::websocket_endpoint::get_metadata(int id) const
{
	con_list::const_iterator metadata_it = m_connection_list.find(id);
	if (metadata_it == m_connection_list.end()) {
		return connection_metadata::ptr();
	}
	else {
		return metadata_it->second;
	}
}

void DDRFramework::websocket_endpoint::onmessage(std::string msg)
{
	if (m_onmsg)
	{
		m_onmsg(msg);
	}
}
void DDRFramework::websocket_endpoint::onconnect()
{
	if (m_onconnect)
	{
		m_onconnect();
	}
}
void DDRFramework::websocket_endpoint::onclose()
{
	if (m_onclose)
	{
		m_onclose();
	}
}
void DDRFramework::websocket_endpoint::onfailed()
{
	if (m_onfailed)
	{
		m_onfailed();
	}
}
void DDRFramework::websocket_endpoint::oninterrupt()
{
	if (m_oninterrupt)
	{
		m_oninterrupt();
	}
}


void DDRFramework::websocket_endpoint::bindonmessage(std::function<void(std::string)> func)
{
	m_onmsg = func;
}
void DDRFramework::websocket_endpoint::bindonconnect(std::function<void()> func)
{
	m_onconnect = func;
}
void DDRFramework::websocket_endpoint::bindonclose(std::function<void()> func)
{
	m_onclose= func;
}
void DDRFramework::websocket_endpoint::bindonfailed(std::function<void()> func)
{
	m_onfailed = func;
}
void DDRFramework::websocket_endpoint::bindoninterrupt(std::function<void()> func)
{
	m_oninterrupt = func;
}