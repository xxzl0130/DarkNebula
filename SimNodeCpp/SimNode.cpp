#include "SimNode.h"

SimNode::SimNode(const std::string& nodeName, const std::string& nodeIP, uint16_t chunkPort, bool slowNode,
	const std::string& adminIP, uint16_t adminRecvPort, uint16_t adminSendPort, QObject* parent):
	QObject(parent),
	dn::SimNode(nodeName,nodeIP,chunkPort,slowNode,adminIP,adminRecvPort,adminSendPort),
	data_{0}
{
	// 注册数据类型用于Qt信号
	qRegisterMetaType<SimData>("SimData");
	qRegisterMetaType<uint32_t>("uint32_t");
	// 设置回调函数
	setSimStepCallback([this](auto&& PH1, auto&& PH2)
	{
		simStepFunc(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
	});
	setSimStepBackCallback([this](auto&& PH1, auto&& PH2)
	{
		replayStepBackFunc(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
	});
	setReplayStepCallback([this](auto&& PH1, auto&& PH2)
	{
		replayStepFunc(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
	});
	setInitCallback([this] { simInitFunc(); });
	setPauseCallback([this] { simPauseFunc(); });
	setStartCallback([this] { simStartFunc(); });
	setStopCallback([this] { simStopFunc(); });
	
	// 添加本节点的数据块
	this->addChunk("counter", data_, true);

	// 发送注册
	this->regIn();
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
	data_.data++;
	emit simStep(data_, time, steps);
}

void SimNode::replayStepBackFunc(uint32_t steps, double time)
{
	simStepFunc(steps, time);
}

void SimNode::replayStepFunc(uint32_t steps, double time)
{
	simStepFunc(steps, time);
}
