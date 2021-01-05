using System;
using System.Collections.Generic;
using System.Threading;
using NetMQ;
using NetMQ.Sockets;

namespace DarkNebulaSharp
{
    public class SimNode : Node
    {
        public SimNode()
        {
        }

        ~SimNode()
        {
            StopWorking();
        }

        private string nodeName;
        // 节点名称
        public string NodeName
        {
            get => nodeName;
            set
            {
                if (!running)
                    nodeName = value;
            }
        }

        private string nodeIP = "127.0.0.1";
        // 节点IP
        public string NodeIP
        {
            get => nodeIP;
            set
            {
                if (!running)
                    nodeIP = value;
            }
        } 

        private UInt16 chunkPort = DNVars.CHUNK_PORT;
        // 数据块起始端口，每个本节点发布的数据块的端口在此基础上依次加一
        public UInt16 ChunkPort
        {
            get => chunkPort;
            set
            {
                if (!running)
                    chunkPort = value;
            }
        }

        private bool slowNode = false;
        // 慢速节点，即本节点不保证跟随每一步仿真
        public bool SlowNode
        {
            get => slowNode;
            set
            {
                if (!running)
                    slowNode = value;
            }
        }

        private string adminIP = "127.0.0.1";
        // 管理节点IP
        public string AdminIP
        {
            get => adminIP;
            set
            {
                if (!running)
                    adminIP = value;
            }
        }

        private UInt16 adminRecvPort = DNVars.ADMIN_RECEIVE_PORT;
        // 管理节点IP
        public UInt16 AdminRecvPort
        {
            get => adminRecvPort;
            set
            {
                if (!running)
                    adminRecvPort = value;
            }
        }

        private UInt16 adminSendPort = DNVars.ADMIN_SEND_PORT;
        // 管理节点IP
        public UInt16 AdminSendPort
        {
            get => adminSendPort;
            set
            {
                if (!running)
                    adminSendPort = value;
            }
        }

        private string recordFolder = ".";
        // 记录文件文件夹
        public string RecordFolder
        {
            get => recordFolder;
            set
            {
                if (!running)
                    recordFolder = value;
            }
        }

        // 仿真初始化回调函数
        private SimEventDelegate initCallback;
        public SimEventDelegate InitCallback
        {
            get => initCallback;
            set
            {
                if (!running) 
                    initCallback = value;
            }
        }

        // 仿真开始回调函数，可不设置
        private SimEventDelegate startCallback;
        public SimEventDelegate StartCallback_
        {
            get => startCallback;
            set
            {
                if (!running)
                    startCallback = value;
            }
        }

        // 仿真暂停回调函数，可不设置
        private SimEventDelegate pauseCallback;
        public SimEventDelegate PauseCallback
        {
            get => pauseCallback;
            set
            {
                if (!running)
                    pauseCallback = value;
            }
        }

        // 仿真停止回调函数，可不设置
        private SimEventDelegate stopCallback;
        public SimEventDelegate StopCallback
        {
            get => stopCallback;
            set
            {
                if (!running)
                    stopCallback = value;
            }
        }

        // 仿真推进函数
        private SimStepDelegate simStepCallback;
        public SimStepDelegate SimStepCallback
        {
            get => simStepCallback;
            set
            {
                if (!running)
                    simStepCallback = value;
            }
        }

        // 回放模式后退一步的回调函数，可以不设置，则使用推进函数
        private SimStepDelegate replayStepCallback;
        public SimStepDelegate ReplayStepCallback
        {
            get => replayStepCallback;
            set
            {
                if (!running)
                    replayStepCallback = value;
            }
        }

        // 回放推进函数，不设置时遇到回放将调用普通仿真函数
        private SimStepDelegate replayStepBackCallback;
        public SimStepDelegate ReplayStepBackCallback
        {
            get => replayStepBackCallback;
            set
            {
                if (!running)
                    replayStepBackCallback = value;
            }
        }

        /// <summary>
        /// 添加数据块
        /// </summary>
        /// <param name="name">数据块名称</param>
        /// <param name="size">数据块大小bytes</param>
        /// <param name="own">写入权限，即该数据块是否由自己发布</param>
        public void AddChunk(string name, int size, bool own)
        {
            if (running)
                return;
            ChunkDict[name] = new Chunk(name,size,own);
        }

        // 注册，完成后无法修改以上参数
        public bool RegIn()
        {
            throw new System.NotImplementedException();
            return true;
        }

        private bool running = false;
        protected override void Working()
        {
            throw new System.NotImplementedException();
        }

        protected override void SocketReady(object sender, NetMQSocketEventArgs e)
        {
            throw new System.NotImplementedException();
        }

        // 发送注册消息
        private bool SendReg()
        {
            throw new System.NotImplementedException();
        }

        // 初始化
        void InitSocket()
        {
            PubSocket.Dispose();
            SubSocket.Dispose();

            PubSocket = new PublisherSocket();
            PubSocket.Connect("tcp://" + adminIP + ":" + adminRecvPort.ToString());

            SubSocket = new SubscriberSocket();
            SubSocket.Connect("tcp://" + adminIP + ":" + adminSendPort.ToString());
            SubSocket.Subscribe(DNVars.COMMAND_TOPIC);

            // 等待连接
            Thread.Sleep(50);
        }

        // 向管理节点发送指令
        void Send2Admin(int code)
        {
            throw new System.NotImplementedException();
        }

        // 处理管理节点指令
        void ProcessAdminCommand()
        {
            throw new System.NotImplementedException();
        }

        // 处理初始化信息
        bool Init()
        {
            throw new System.NotImplementedException();
        }

        // 发送一个数据块
        void SendChunk(ref Chunk chunk)
        {
            throw new System.NotImplementedException();
        }

        // 发布自己所有要发布的数据块
        void SendChunks()
        {
            throw new System.NotImplementedException();
        }

        // 加载下一帧数据
        void LoadNext()
        {
            throw new System.NotImplementedException();
        }

        // 加载前一帧数据
        void LoadBack()
        {
            throw new System.NotImplementedException();
        }

        // 数据块
        private Dictionary<string, Chunk> ChunkDict;
        // 本节点ID
        private int ID = -1;
    }
}