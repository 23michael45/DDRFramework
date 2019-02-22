#include "MQTTAsyncClient.h"

namespace DDRFramework
{

	void MQTTAsyncClient::pub_action_listener::on_failure(const mqtt::token& tok)
	{
		cout << "\tListener failure for token: "
			<< tok.get_message_id() << endl;
	}

	void MQTTAsyncClient::pub_action_listener::on_success(const mqtt::token& tok)
	{
		cout << "\tListener success for token: "
			<< tok.get_message_id() << endl;
	}

	void MQTTAsyncClient::delivery_action_listener::on_failure(const mqtt::token& tok)
	{
		pub_action_listener::on_failure(tok);
		done_ = true;
	}

	void MQTTAsyncClient::delivery_action_listener::on_success(const mqtt::token& tok)
	{
		pub_action_listener::on_success(tok);
		done_ = true;
	}


	void MQTTAsyncClient::sub_action_listener::on_failure(const mqtt::token& tok)
	{
		std::cout << name_ << " failure";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
	}

	void MQTTAsyncClient::sub_action_listener::on_success(const mqtt::token& tok)
	{
		std::cout << name_ << " success";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		auto top = tok.get_topics();
		if (top && !top->empty())
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
		std::cout << std::endl;
	}





	MQTTAsyncClient::MQTTAsyncClient(std::string server, std::string clientid) : m_Client(server.c_str(),clientid.c_str()),m_SubListener("Subscription"),m_Callback(m_Client,*this)
	{
		m_IsConnected = false;
		m_Client.set_callback(m_Callback);
	}

	bool MQTTAsyncClient::Connect()
	{
		try {
			cout << "\nConnecting..." << endl;
			mqtt::token_ptr conntok = m_Client.connect();
			cout << "Waiting for the connection..." << endl;
			conntok->wait();
			cout << "  ...OK" << endl;

			return true;
		}
		catch (const mqtt::exception& exc) {
			cerr << exc.what() << endl;
		
			return false;
		}


	}

	void MQTTAsyncClient::Disconnect()
	{
		if (!m_IsConnected)
		{
			return;
		}
		try {
			// Disconnect
			cout << "\nDisconnecting..." << endl;
			mqtt::token_ptr conntok = m_Client.disconnect();
			conntok->wait();
			cout << "  ...OK" << endl;
		}
		catch (const mqtt::exception& exc) {
			cerr << exc.what() << endl;
		}
	}

	void MQTTAsyncClient::Publish(std::string topic, std::string content)
	{
		try {
			cout << "\nSending final message..." << endl;
			delivery_action_listener deliveryListener;
			mqtt::message_ptr pubmsg = mqtt::make_message(topic.c_str(), content.c_str());
			pubmsg->set_qos(QOS);

			m_Client.publish(pubmsg, nullptr, deliveryListener);

			while (!deliveryListener.is_done()) {
				this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			cout << "OK" << endl;
		}
		catch (const mqtt::exception& exc) {
			cerr << exc.what() << endl;
		}
	}

	void MQTTAsyncClient::Subscribe(std::string topic)
	{
		try {
			m_Client.subscribe(topic.c_str(), QOS, nullptr, m_SubListener);
		}
		catch (const mqtt::exception& exc) {
			cerr << exc.what() << endl;
		}
	}

	
}