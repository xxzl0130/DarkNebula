#include <iostream>
#include <zmq.h>
#include <DarkNebula/AdminNode.h>
#include <DarkNebula/SimNode.h>
#include <nlohmann/json.hpp>
#include <thread>
using namespace std;

int main()
{
	dn::AdminNode admin(16666,18888);
	admin.setRegisterCallback([&](int id)
		{
			const auto node = admin.getNodeList()[id];
			cout << "Register: " << node.name << " " << node.ip << endl;
			cout << "Chunks:\n";
			const auto& chunks = admin.getChunkList();
			for(auto& it : node.chunks)
			{
				cout << chunks[it.first].first << " " << it.second << endl;
			}
		});
	admin.setInitOverCallback([&](int id)
		{
			if (id == dn::ALL_NODE)
			{
				cout << "All init!" << endl;
				//Sleep(500);
				cout << "Start!" << endl;
				admin.startSim();
				return;
			}
			const auto node = admin.getNodeList()[id];
			cout << "Init :" << node.name << " " << node.init << endl;
		});
	admin.setStepTime(1000);
	admin.setSimTime(20);
	
	dn::SimNode node1("node1", "127.0.0.1", 20000, false, "127.0.0.1", 16666, 18888);
	dn::SimNode node2("node2", "127.0.0.1", 30000, false, "127.0.0.1", 16666, 18888);
	int counter = 0;
	node1.addChunk("counter", counter, true);
	node1.regIn();
	node1.setInitCallback([&]()
	{
		counter = 0;
	});
	node1.setSimStepCallback([&](unsigned step, double time)
		{
			cout << "Node 1: " << ++counter << endl;
		});
	int counterRecv = 0;
	node2.addChunk("counter", counterRecv, false);
	node2.setSlowNode(true);
	node2.regIn();
	node2.setInitCallback([&]() {counterRecv = 0; });
	node2.setSimStepCallback([&](unsigned step, double time)
		{
			cout << "Node 2: " << counterRecv << endl;
			////std::this_thread::sleep_for(std::chrono::milliseconds(1800));
		});
	
	system("pause");
	//Sleep(100);
	//admin.setRecord(true, "test");
	//admin.setReplay(true, "test");
	admin.initSim();
	Sleep(100);
	//system("pause");

	Sleep(100000);
	
	return 0;
}
