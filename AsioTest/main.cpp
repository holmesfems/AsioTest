#include <cstdio>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
class Client
{
	boost::asio::io_service& io_service_;
	boost::asio::ip::tcp::socket socket_;

public:
	Client(boost::asio::io_service& io_service)
		: io_service_(io_service),
		socket_(io_service)
	{}

	void connect(std::string ip_address, int port)
	{
		socket_.async_connect(
			boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip_address), port),
			boost::bind(&Client::on_connect, this, boost::asio::placeholders::error));
	}

	void on_connect(const boost::system::error_code& error)
	{
		if (error) {
			std::cout << "connect failed : " << error.message() << std::endl;
		}
		else {
			std::cout << "connected" << std::endl;
			std::string msg;
			std::getline(std::cin, msg);
			//std::cin >> msg;
			//std::cout << "sending:" << msg << std::endl;
			send(msg + "\n");
			std::cout.flush();
				//std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void send(std::string msg)
	{
		
		boost::asio::async_write(
			socket_,
			boost::asio::buffer(msg.c_str(), msg.length()),
			boost::bind(&Client::on_send, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred, msg));
		//boost::system::error_code error;
		//boost::asio::write(socket_, boost::asio::buffer(msg), error);
		//if (error) {
		//	std::cout << "send failed: " << error.message() << std::endl;
		//}
		//else {
		//	std::cout << "send correct!" << std::endl;
		//}
		
	}

	void on_send(const boost::system::error_code& error, size_t bytes_transferred,std::string sent)
	{
		if (error)
		{
			std::cout << "Failed to send" << std::endl;
			std::cout.flush();
		}
		else
		{
			std::cout << "Succeed to send, lenth = " << bytes_transferred << std::endl;
			std::cout.flush();
			if (sent != "exit\n")
			{
				std::string msg;
				std::getline(std::cin, msg);
				send(msg + "\n");
			}
		}
	}

};

class Server {
	boost::asio::io_service& io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::streambuf receive_buff_;
	int port;
	int status = 0;
public:
	bool accepted = false;
	Server(boost::asio::io_service& io_service, int port)
		: io_service_(io_service),
		acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
		socket_(io_service),
		port(port) {}

	void start_accept()
	{
		acceptor_.async_accept(
			socket_,
			boost::bind(&Server::on_accept, this, boost::asio::placeholders::error));
	}

	void receive()
	{

		//status = 0;
		std::cout << "start recieving..." << std::endl;
		std::cout.flush();
		boost::asio::async_read_until(
			socket_,
			receive_buff_,
			"\n",
			boost::bind(&Server::on_receive, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		//while (true)
		//{
		//	switch (status)
		//	{
		//	case -1://failed
		//		return "";
		//	case 0:
		//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		//		continue;
		//	}
		//	if (status == 1) break;
		//}
		//std::cout << "recievied?" << std::endl;
		//const char* data = boost::asio::buffer_cast<const char*>(receive_buff_.data());
		//return std::string(data);
		//boost::asio::streambuf receive_buffer;
		//boost::system::error_code error;
		//boost::asio::read_until(socket_, receive_buffer, "\n", error);

		//if (error && error != boost::asio::error::eof) {
		//	std::cout << "receive failed: " << error.message() << std::endl;
		//	return "";
		//}
		//else {
		//	const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
		//	//std::cout << data << std::endl;
		//	return data;
		//}
	}

	// 受信完了
	// error : エラー情報
	// bytes_transferred : 受信したバイト数
	void on_receive(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (error && error != boost::asio::error::eof) {
			std::cout << "receive failed: " << error.message() << std::endl;
			status = -1;
		}
		else {
			std::cout << "recieve succeed " << "length = " << bytes_transferred << std::endl;
			std::string data = std::string(boost::asio::buffer_cast<const char*>(receive_buff_.data()), bytes_transferred);
			std::cout << data;
			if (bytes_transferred == 0 && error == boost::asio::error::eof)
			{
				std::cout << "exit server!" << std::endl;
			}
			else if (data != "exit\n")
			{
				receive_buff_.consume(receive_buff_.size());
				receive();
			}
		}
		std::cout.flush();
	}

private:
	void on_accept(const boost::system::error_code& error)
	{
		if (error) {
			std::cout << "accept failed: " << error.message() << std::endl;
		}
		else {
			std::cout << "accept succeed!" << std::endl;
			accepted = true;
			receive();
			//std::cout << "lalala" << std::endl;
		}
		std::cout.flush();
	}
};

int makeServer()
{
	std::string msg;
	boost::asio::io_service io_service;
	Server server(io_service,23333);
	server.start_accept();
	io_service.run();
	//server.receive();
	while (true)
	{
		std::cin >> msg;
		if (msg == "exit") break;
	}
	std::cout << "run to exit" << std::endl;
	
	//std::cout << "start recieving?" << std::endl;
	//server.receive();

	//while (true)
	//{
	//	if (!server.accepted)
	//	{
	//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//		continue;
	//	}
	//	msg = server.receive();
	//	if (msg == "")
	//		std::cout << "fail?" << std::endl;
	//	std::cout << msg;
	//	std::cout.flush();
	//	if (msg == "exit\n") break;
	//	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//}
	return 0;
}

int makeClient()
{
	std::string msg;
	boost::asio::io_service io_service;
	Client client(io_service);
	client.connect("127.0.0.1", 23333);
	io_service.run();
	std::cout << "client exit" << std::endl;
	//while (true)
	//{
	//	std::cin >> msg;
	//	std::cout << "sending:" << msg << std::endl;
	//	client.send(msg + "\n");
	//	std::cout.flush();
	//	if (msg == "exit") break;
	//	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//}
	return 0;
}

int main(int argc, char *argv[])
{
	printf("hello from AsioTest!\n");
	std::string cmd;
	if (argc == 1) std::cin >> cmd;
	else cmd = argv[1];
	if (cmd == "c")
		makeClient();
	else if (cmd == "s")
		makeServer();
	return 0;
}