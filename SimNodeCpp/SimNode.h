#pragma once

#include <QObject>
#include <QSettings>
#include <string>
#include <QThread>
#include <DarkNebula/SimNode.h>

// 示例的仿真数据结构
struct SimData
{
	long long data;
};

class SimNode : public QObject, public dn::SimNode
{
	Q_OBJECT

public:
	/**
	 * \brief 构造函数
	 * \param nodeName 本节点名称，各节点名称要唯一
	 * \param nodeIP 本节点IP地址
	 * \param chunkPort 数据块起始端口，每个本节点发布的数据块的端口在此基础上依次加一
	 * \param slowNode 慢速节点，即本节点不保证跟随每一步仿真
	 * \param adminIP 管理节点IP
	 * \param adminRecvPort 管理节点接收指令的端口
	 * \param adminSendPort 管理节点发送指令的端口
	 * \param parent 父对象
	 */
	SimNode(const std::string& nodeName, const std::string& nodeIP = "127.0.0.1", uint16_t chunkPort = 10000, bool slowNode = false,
		const std::string& adminIP = "127.0.0.1", uint16_t adminRecvPort = dn::ADMIN_RECEIVE_PORT, uint16_t adminSendPort = dn::ADMIN_SEND_PORT,
		QObject* parent = nullptr);
	virtual ~SimNode();

signals:
	// 初始化完成信号
	void simInit();

	// 开始信号
	void simStart();

	// 暂停信号
	void simPause();
	
	/**
	 * \brief 仿真推进一步的信号
	 * \param data 数据
	 * \param simTime 时间
	 * \param simStep 步数
	 */
	void simStep(const SimData& data, double simTime, uint32_t simStep);

	// 仿真结束的信号
	void simStop();

protected:
	/// 仿真过程中的相关函数，均运行在子线程
	// 仿真初始化回调函数，必须实现
	void simInitFunc();
	// 仿真开始函数，可不实现
	void simStartFunc();
	// 仿真暂停函数，可不实现
	void simPauseFunc();
	// 仿真停止函数，可不实现
	void simStopFunc();
	// 仿真推进函数，必须实现
	void simStepFunc(uint32_t steps, double time);
	// 回放模式后退一步的函数，可以不实现，则使用推进函数
	void replayStepBackFunc(uint32_t steps, double time);
	// 回放推进函数，不实现时遇到回放将调用普通仿真函数
	void replayStepFunc(uint32_t steps, double time);

private:
	// 实际的数据对象
	SimData data_;
};
