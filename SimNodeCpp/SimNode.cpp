#include "SimNode.h"

SimNode::SimNode(const std::string& nodeName, const std::string& nodeIP, uint16_t chunkPort, bool slowNode,
	const std::string& adminIP, uint16_t adminRecvPort, uint16_t adminSendPort, QObject* parent):
	QObject(parent),
	dn::SimNode(nodeName,nodeIP,chunkPort,slowNode,adminIP,adminRecvPort,adminSendPort),
	data_{0}
{
	// ע��������������Qt�ź�
	qRegisterMetaType<SimData>("SimData");
	qRegisterMetaType<uint32_t>("uint32_t");
	// ���ûص�����
	setSimStepCallback(std::bind(&SimNode::simStepFunc, this,std::placeholders::_1,std::placeholders::_2));
	setSimStepBackCallback(std::bind(&SimNode::replayStepBackFunc, this,std::placeholders::_1,std::placeholders::_2));
	setReplayStepCallback(std::bind(&SimNode::replayStepFunc, this,std::placeholders::_1,std::placeholders::_2));
	setInitCallback(std::bind(&SimNode::simInitFunc, this));
	setPauseCallback(std::bind(&SimNode::simPauseFunc, this));
	setStartCallback(std::bind(&SimNode::simStartFunc, this));
	setStopCallback(std::bind(&SimNode::simStopFunc, this));
	
	// ���ӱ��ڵ�����ݿ�
	this->addChunk("counter", data_, true);

	// ����ע��
	this->regIn();
}

SimNode::~SimNode()
{
}

void SimNode::simInitFunc()
{
	// �������д�����ʼ����ز���
	data_.data = 0;

	// �����ź�
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