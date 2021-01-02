#include <iostream>
#include <zmq.h>
#include <DarkNebula/AdminNode.h>
#include <nlohmann/json.hpp>
#include <thread>
using namespace std;

int main()
{
	dn::AdminNode admin(16666,18888);
	auto* ctx = zmq_ctx_new();
	auto* sub = zmq_socket(ctx, ZMQ_SUB);
	auto* pub = zmq_socket(ctx, ZMQ_PUB);
	zmq_connect(sub, "tcp://127.0.0.1:18888");
	zmq_connect(pub, "tcp://127.0.0.1:16666");
	zmq_setsockopt(sub, ZMQ_SUBSCRIBE, dn::COMMAND_TOPIC, strlen(dn::COMMAND_TOPIC));
	char buffer[128] = {};
	std::thread th([&]()
		{
			zmq_pollitem_t item{ sub,0,ZMQ_POLLIN };
				if(zmq_poll(&item,1,1000))
				{
					zmq_recv(sub, buffer, 128, 0);
					if(strcmp(buffer,dn::COMMAND_TOPIC) == 0)
					{
						cout << dn::COMMAND_TOPIC << endl;
						zmq_recv(sub, buffer, 128, 0);
						auto* header = reinterpret_cast<dn::CommandHeader*>(buffer);
						cout << "receive: id=" << header->ID << " code=" << header->code << " size=" << header->size << endl;
					}
				}
				std::this_thread::sleep_for(chrono::seconds(10));
		});
	th.detach();
	this_thread::sleep_for(chrono::milliseconds(100));
	admin.initSim();
	this_thread::sleep_for(chrono::milliseconds(100));
	const auto* str = R"({
		"name":"test",
		"ip":"127.0.0.1",
		"slow":false,
		"chunks":[
		{
			"name":"AA",
			"own":false
		},
		{
			"name":"BB",
			"own":true,
			"port":23333
		}
	]})";
	auto* header = reinterpret_cast<dn::CommandHeader*>(buffer);
	header->ID = 0;
	header->code = dn::COMMAND_REG;
	header->size = strlen(str);
	memcpy_s(buffer + sizeof dn::CommandHeader, header->size, str, header->size);
	zmq_send(pub, dn::REPLY_TOPIC, strlen(dn::REPLY_TOPIC), ZMQ_SNDMORE);
	zmq_send(pub, buffer, header->size + sizeof dn::CommandHeader, 0);
	std::cerr << zmq_strerror(zmq_errno()) << std::endl;
	this_thread::sleep_for(chrono::milliseconds(3000));
	zmq_close(sub);
	zmq_close(pub);
	zmq_ctx_destroy(ctx);
	return 0;
}
