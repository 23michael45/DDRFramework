#ifndef MQTTAsyncClient_h__
#define MQTTAsyncClient_h__

#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>	// For sleep
#include <atomic>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"

using namespace std;

namespace DDRFramework
{
	const int  QOS = 1;
	const auto TIMEOUT = std::chrono::seconds(10);
	const int	N_RETRY_ATTEMPTS = 5;
	class MQTTAsyncClient
	{
		/////////////////////////////////////////////////////////////////////////////

		/**
		 * A base action listener.
		 */
		class pub_action_listener : public virtual mqtt::iaction_listener
		{
		protected:
			void on_failure(const mqtt::token& tok) override;

			void on_success(const mqtt::token& tok) override;
		};

		/////////////////////////////////////////////////////////////////////////////

		/**
		 * A derived action listener for publish events.
		 */
		class delivery_action_listener : public pub_action_listener
		{
			atomic<bool> done_;

			void on_failure(const mqtt::token& tok) override;

			void on_success(const mqtt::token& tok) override;

		public:
			delivery_action_listener() : done_(false) {}
			bool is_done() const { return done_; }
		};



		// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

		class sub_action_listener : public virtual mqtt::iaction_listener
		{
			std::string name_;

			void on_failure(const mqtt::token& tok) override;

			void on_success(const mqtt::token& tok) override;

		public:
			sub_action_listener(const std::string& name) : name_(name) {}
		};



		/**
		 * Local callback & listener class for use with the client connection.
		 * This is primarily intended to receive messages, but it will also monitor
		 * the connection to the broker. If the connection is lost, it will attempt
		 * to restore the connection and re-subscribe to the topic.
		 */
		class mqtt_callback : public virtual mqtt::callback,
			public virtual mqtt::iaction_listener

		{
			// Counter for the number of connection retries
			int nretry_;
			// The MQTT client
			mqtt::async_client& cli_;
			MQTTAsyncClient& aclient;

			// Re-connection failure
			void on_failure(const mqtt::token& tok) override {
				std::cout << "Connection attempt failed" << std::endl;
				if (++nretry_ > N_RETRY_ATTEMPTS)
					exit(1);
			}

			// (Re)connection success
			// Either this or connected() can be used for callbacks.
			void on_success(const mqtt::token& tok) override {}

			// (Re)connection success
			void connected(const std::string& cause) override {
					aclient.m_IsConnected = true;

				if (aclient.m_OnConnect)
				{
					aclient.m_OnConnect(cause);
				}
			}

			// Callback for when the connection is lost.
			// This will initiate the attempt to manually reconnect.
			void connection_lost(const std::string& cause) override {


				nretry_ = 0;
				aclient.m_IsConnected = false;

				if (aclient.m_OnConnectionLost)
				{
					aclient.m_OnConnectionLost(cause);
				}
			}

			// Callback for when a message arrives.
			void message_arrived(mqtt::const_message_ptr msg) override {
				if (aclient.m_OnMsgArrived)
				{
					aclient.m_OnMsgArrived(msg->get_topic(), msg->to_string());
				}
			}

			void delivery_complete(mqtt::delivery_token_ptr token) override 
			{
			
				if (aclient.m_OnDeliveryComplete)
				{
					aclient.m_OnDeliveryComplete(token);
				}
			}

		public:
			mqtt_callback(mqtt::async_client& cli, MQTTAsyncClient& ac)
				: nretry_(0), cli_(cli), aclient(ac){}
		};
	public:
		MQTTAsyncClient(std::string server, std::string clientid);

		void Connect();
		void Disconnect();
		void Publish(std::string topic, std::string content);
		void Subscribe(std::string topic);

		bool m_IsConnected;


		std::function<void(const std::string& cause)> m_OnConnect;
		std::function<void(const std::string& cause)> m_OnConnectionLost;
		std::function<void(mqtt::delivery_token_ptr token)> m_OnDeliveryComplete;
		std::function<void(std::string topic, std::string content)> m_OnMsgArrived;
	private:
		mqtt::async_client m_Client;
		mqtt_callback m_Callback; 
		sub_action_listener m_SubListener;




	};
}
#endif // MQTTAsyncClient_h__
