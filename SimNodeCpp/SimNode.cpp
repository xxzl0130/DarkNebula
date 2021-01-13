#include "SimNode.h"

constexpr auto IniNodeName = "/Node/Name";
constexpr auto IniNodePort = "/Node/Port";
constexpr auto IniAdminPort = "/Admin/Port";
constexpr auto IniAdminIP = "/Admin/IP";
// 以下各项根据项目修改
constexpr auto IniFilename = "SimNode.ini";
constexpr auto IniNodeNameDefault = "SimNode";
constexpr auto IniNodePortDefault = 9999;
constexpr auto IniAdminPortDefault = 6666;
constexpr auto IniAdminIPDefault = "127.0.0.1";

SimNode::SimNode(const std::string& nodeName, const std::string& nodeIP, uint16_t chunkPort, bool slowNode,
	const std::string& adminIP, uint16_t adminRecvPort, uint16_t adminSendPort, QObject* parent):
	QObject(parent),
	dn::SimNode(nodeName,nodeIP,chunkPort,slowNode,adminIP,adminRecvPort,adminSendPort),
	data_{0}
{
	qRegisterMetaType<SimData>("SimData");
}

SimNode::~SimNode()
{
}

void SimNode::simInitFunc()
{
	// 在这里编写仿真初始化相关操作

	data_.data = 0;

	// 发送信号
	emit simInit();
}

void SimNode::simStartFunc()
{
	emit simStart();
}

void SimNode::simPauseFunc()
{
	emit simPause();
}

void SimNode::simStopFunc()
{
	emit simStop();
}

void SimNode::simStepFunc(uint32_t steps, double time)
{
}
