#pragma once
#include "DarkNebulaGlobal.h"

#include "Node.h"

namespace dn
{
	// 仿真指令回调函数
	typedef std::function<void(void)> SimEventCallback;
	// 仿真推进回调函数
	typedef std::function<void(uint32_t,double)> SimStepCallback;
	
	class DN_EXPORT SimNode : public Node
	{
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
		 */
		SimNode(const std::string& nodeName, const std::string& nodeIP = "127.0.0.1", uint16_t chunkPort = 10000, bool slowNode = false,
			const std::string& adminIP = "127.0.0.1", uint16_t adminRecvPort = ADMIN_RECEIVE_PORT, uint16_t adminSendPort = ADMIN_SEND_PORT);
		virtual ~SimNode();
		
		/// 节点参数设置，需要在注册以前设置
		// 节点名称
		void setNodeName(const std::string& name);
		// 节点IP
		void setNodeIP(const std::string& IP);
		// 数据块起始端口
		void setChunkPort(uint16_t port);
		// 设置是否为慢速节点
		void setSlowNode(bool slow);
		// 管理节点IP
		void setAdminIP(const std::string& IP);
		// 管理节点接收指令的端口
		void setAdminRecvPort(uint16_t port);
		// 管理节点发送指令的端口
		void setAdminSendPort(uint16_t port);
		/**
		 * \brief 添加数据块
		 * \tparam T 数据块类型
		 * \param name 数据块名称，各节点之间同一个数据块的名称要保持一致
		 * \param data 数据变量，每步仿真前数据会更新到这里
		 * \param write 是否拥有该数据块写入权限
		 */
		template <typename T>
		void addChunk(const std::string& name,T& data,bool write)
		{
			addChunk(name, &data, sizeof T, write);
		}
		/**
		 * \brief 添加数据块
		 * \param name 数据块名称，各节点之间同一个数据块的名称要保持一致
		 * \param pData 数据变量指针，每步仿真前数据会更新到这里
		 * \param size 数据大小
		 * \param write 是否拥有该数据块写入权限
		 */
		void addChunk(const std::string& name, void* pData, size_t size, bool write);

		// 注册，之后无法修改参数
		bool regIn();

		/// 回调函数设置，注意回调函数均运行在子线程
		// 仿真初始化回调函数
		void setInitCallback(SimEventCallback callback);
		// 仿真开始回调函数，可不设置
		void setStartCallback(SimEventCallback callback);
		// 仿真暂停回调函数，可不设置
		void setPauseCallback(SimEventCallback callback);
		// 仿真停止回调函数，可不设置
		void setStopCallback(SimEventCallback callback);
		// 仿真推进函数
		void setSimStepCallback(SimStepCallback callback);
		// 回放模式后退一步的回调函数，可以不设置，则使用推进函数
		void setSimStepBackCallback(SimStepCallback callback);
		// 回放推进函数，不设置时遇到回放将调用普通仿真函数
		void setReplayStepCallback(SimStepCallback callback);

	private:
		// 数据块定义
		struct Chunk
		{
			// 名称
			std::string name;
			// 容量
			size_t size = 0;
			// 所有权限
			bool own = false;
			// 初始化
			bool init = false;
			// 端口
			uint16_t port = 0;
			// 编号
			int id = 0;
			// socket指针
			void* socket = nullptr;
			// 数据指针
			void* pData = nullptr;
			// 缓存指针
			std::unique_ptr<char[]> buffer = nullptr;
			Chunk(){}
			Chunk(const std::string& n, size_t s, bool write, void* p) :
				name(n), size(s),own(write), init(false),port(0),id(-1),socket(nullptr),pData(p),buffer(new char[size]){}
		};
		
		// 运行线程
		void working() override;
		// 发送注册消息
		bool sendReg();
		// 初始化
		void initSocket();
		// 向管理节点发送指令
		void send2Admin(int code, const void* data = nullptr, size_t size = 0);
		// 处理管理节点指令
		void processAdminCommand();
		// 处理初始化信息
		void init();
		// 发送一个数据块
		void sendChunk(Chunk& chunk);
		// 发布自己所有要发布的数据块
		void sendChunks();
		// 将所有数据更新到用户数据里
		void copyChunks();

	private:
		SimEventCallback initCallback_, startCallback_, pauseCallback_, stopCallback_;
		SimStepCallback simStepCallback_, replayStepCallback_, replayStepBackCallback_;

		// 节点名称
		std::string nodeName_;
		// 节点IP
		std::string nodeIP_;
		// 数据块IP
		uint16_t chunkPort_;
		// 管理节点IP
		std::string adminIP_;
		// 管理节点接收端口
		uint16_t adminRecvPort_;
		// 管理节点发送端口
		uint16_t adminSendPort_;
		// 慢速节点
		bool slowNode_;
		// 运行中
		bool running_;
		
		// 监听列表
		std::vector<zmq_pollitem_t> pollitems_;
		// 数据块列表
		std::vector<Chunk> chunkList_;
		// 数据块集合
		std::set<std::string> chunkSet_;
		// 本节点id
		int id_;
		// 慢速节点锁
		std::mutex slowMutex_;
		// 慢速节点线程
		std::thread* slowThread_;
		// 慢速运行中
		std::atomic_bool slowRunning_;
	};
}

