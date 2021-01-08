using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;
using System.Threading.Tasks;
using NetMQ;
using NetMQ.Monitoring;
using NetMQ.Sockets;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace DarkNebulaSharp
{
    public class SimNode : Node
    {
        public SimNode()
        {
        }

        ~SimNode()
        {
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

        // 回放推进回调函数，可以不设置，则使用普通仿真函数
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

        // 回放模式后退一步函数，不设置时遇到回放将调用普通仿真函数
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
            chunkDict[name] = chunkList.Count;
            chunkList.Add(new Chunk(name, size, own));
        }

        /// <summary>
        /// 更新Chunk的数据，要求对Chunk有写入权限
        /// </summary>
        /// <typeparam name="T">类型通配</typeparam>
        /// <param name="name">名称</param>
        /// <param name="data">数据</param>
        public void SetChunkData<T>(string name, T data)
        {
            if (chunkDict.ContainsKey(name))
            {
                var chunk = chunkList[chunkDict[name]];
                if (chunk.Own)
                {
                    chunk.Buffer = Utils.StructureToByte(data);
                }
            }
        }

        /// <summary>
        /// 获取Chunk数据
        /// </summary>
        /// <typeparam name="T">类型通配</typeparam>
        /// <param name="name">名称</param>
        /// <param name="data">数据</param>
        public void GetChunkData<T>(string name, out T data)
        {
            if (chunkDict.ContainsKey(name))
            {
                var chunk = chunkList[chunkDict[name]];
                data = Utils.ByteToStructure<T>(chunk.Buffer.ToArray());
            }
            else
            {
                data = default(T);
            }
        }

        // 注册，完成后无法修改以上参数
        public bool RegIn()
        {
            if (running)
                return true;
            InitSocket();
            if (!SendReg())
                return false;
            StartWorking();
            running = true;

            return true;
        }

        private bool running = false;

        protected override void SocketReady(object sender, NetMQSocketEventArgs e)
        {
            var topic = System.Text.Encoding.UTF8.GetString(e.Socket.ReceiveFrameBytes());
            if (topic == DNVars.COMMAND_TOPIC) // 管理节点消息
            {
                ProcessAdminCommand();
                return;
            }
            // 数据节点消息
            if (chunkDict.ContainsKey(topic))
            {
                var chunk = chunkList[chunkDict[topic]];
                chunk.Buffer = e.Socket.ReceiveFrameBytes();
            }
        }

        // 发送注册消息
        private bool SendReg()
        {
            var info = new JObject {["name"] = nodeName, ["ip"] = nodeIP, ["slow"] = slowNode};
            var chunks = new JArray();
            var port = chunkPort;
            foreach (var it in chunkList)
            {
                var chunk = new JObject {["name"] = it.Name, ["own"] = it.Own};
                if (it.Own)
                    it.Port = port++;
                chunk["port"] = it.Port;
                chunks.Add(chunk);
            }

            info["chunks"] = chunks;
            Send2Admin(CommandCode.COMMAND_REG, info.ToString());

            return true;
        }

        // 初始化
        private void InitSocket()
        {
            PubSocket?.Dispose();
            SubSocket?.Dispose();

            PubSocket = new PublisherSocket();
            PubSocket.Connect("tcp://" + adminIP + ":" + adminRecvPort.ToString());

            SubSocket = new SubscriberSocket();
            SubSocket.Connect("tcp://" + adminIP + ":" + adminSendPort.ToString());
            SubSocket.Subscribe(DNVars.COMMAND_TOPIC);
            SubSocket.ReceiveReady += SocketReady;

            // 等待连接
            Thread.Sleep(50);
        }

        // 向管理节点发送指令
        private void Send2Admin(CommandCode code, string msg)
        {
            var bytes = System.Text.Encoding.UTF8.GetBytes(msg);
            Send2Admin(code, bytes);
        }

        // 向管理节点发送指令
        private void Send2Admin<T>(CommandCode code, T obj)
        {
            var bytes = Utils.StructureToByte(obj);
            Send2Admin(code, bytes);
        }

        // 向管理节点发送指令
        private void Send2Admin(CommandCode code, byte[] data)
        {
            var header = new CommandHeader {ID = ID, size = data.Length, code = code};
            var headerData = Utils.StructureToByte(header);
            var length = headerData.Length + data.Length;
            if (length > OutBuffer.Length) // Buffer翻倍
            {
                OutBuffer = new byte[length * 2];
            }
            headerData.CopyTo(OutBuffer, 0);
            data.CopyTo(OutBuffer, headerData.Length);
            PubSocket.SendMoreFrame(DNVars.REPLY_TOPIC).SendFrame(OutBuffer, length);
        }

        // 处理管理节点指令
        private void ProcessAdminCommand()
        {
            InBuffer = SubSocket.ReceiveFrameBytes();
            var header = Utils.ByteToStructure<CommandHeader>(InBuffer);
            if (header.ID != DNVars.ALL_NODE && header.ID != ID)
                return;
            switch (header.code)
            {
                case CommandCode.COMMAND_INIT:
                    var ok = Init();
                    if (ok != (UInt16)ErrorCode.ERR_NOP)
                    {
                        Send2Admin(CommandCode.COMMAND_INIT, ok);
                        break;
                    }

                    simState = SimStates.SimInit;
                    initCallback?.Invoke();
                    Send2Admin(CommandCode.COMMAND_INIT, ok);
                    break;
                case CommandCode.COMMAND_START:
                    simSteps = 0;
                    curTime = 0;
                    simState = SimStates.SimRun;
                    startCallback?.Invoke();
                    break;
                case CommandCode.COMMAND_STEP_FORWARD:
                    simState = SimStates.SimRun;
                    if (ReplayState == (int) ReplayStates.Replaying)
                    {
                        LoadNext();
                    }

                    if (!slowNode)
                    {
                        if (ReplayState == (int) ReplayStates.Replaying && replayStepCallback != null)
                        {
                            replayStepCallback(simSteps, curTime);
                        }
                        else
                        {
                            simStepCallback?.Invoke(simSteps, curTime);
                        }
                        ++simSteps;
                        SendChunks();
                    }
                    else if (!slowRunning)
                    {
                        slowThread = new Thread(() =>
                        {
                            slowMutex.WaitOne();
                            slowRunning = true;

                            if (ReplayState == (int)ReplayStates.Replaying && replayStepCallback != null)
                            {
                                replayStepCallback(simSteps, curTime);
                            }
                            else
                            {
                                simStepCallback?.Invoke(simSteps, curTime);
                            }
                            ++simSteps;

                            SendChunks();
                            slowRunning = false;
                            slowMutex.ReleaseMutex();
                        });
                        slowThread.Start();
                    }

                    Send2Admin(CommandCode.COMMAND_STEP_FORWARD,simSteps);
                    break;
                case CommandCode.COMMAND_STEP_BACKWARD:
                    simState = SimStates.SimRun;
                    if (ReplayState == (int)ReplayStates.Replaying)
                    {
                        LoadBack();
                    }

                    if (!slowNode)
                    {
                        if (ReplayState == (int)ReplayStates.Replaying && replayStepBackCallback != null)
                        {
                            replayStepBackCallback(simSteps, curTime);
                        }
                        else
                        {
                            simStepCallback?.Invoke(simSteps, curTime);
                        }
                        --simSteps;
                        SendChunks();
                    }
                    else if (!slowRunning)
                    {
                        slowThread = new Thread(() =>
                        {
                            slowMutex.WaitOne();
                            slowRunning = true;

                            if (ReplayState == (int)ReplayStates.Replaying && replayStepBackCallback != null)
                            {
                                replayStepBackCallback(simSteps, curTime);
                            }
                            else
                            {
                                simStepCallback?.Invoke(simSteps, curTime);
                            }
                            --simSteps;

                            SendChunks();
                            slowRunning = false;
                            slowMutex.ReleaseMutex();
                        });
                    }

                    Send2Admin(CommandCode.COMMAND_STEP_FORWARD, simSteps);
                    break;
                case CommandCode.COMMAND_PAUSE:
                    simState = SimStates.SimPause;
                    pauseCallback?.Invoke();
                    break;
                case CommandCode.COMMAND_STOP:
                    simState = SimStates.SimStop;
                    recordFileStream?.Close();
                    stopCallback?.Invoke();
                    break;
                case CommandCode.COMMAND_REG:
                    RegIn();
                    break;
            }
        }

        // 处理初始化信息
        private UInt16 Init()
        {
            var str = System.Text.Encoding.UTF8.GetString(InBuffer.Skip(Marshal.SizeOf<CommandHeader>()).ToArray());
            var info = (JObject)JsonConvert.DeserializeObject(str);
            if (info == null)
                return (UInt16)ErrorCode.ERR_INFO;

            SimTime = info["simTime"].Value<double>();
            SimFree = info["free"].Value<bool>();
            ReplayState = info["replay"].Value<int>();
            StepTime = info["step"].Value<uint>();
            RecordName = info["record"].Value<string>();

            var nodes = info["nodes"].Value<JObject>();
            if (nodes.ContainsKey(nodeName))
            {
                ID = nodes[nodeName]["id"].Value<int>();
            }
            else
            {
                return (UInt16)ErrorCode.ERR_INFO;
            }

            var chunks = info["chunks"].Value<JObject>();
            recordSize = 0;
            int connected = 0, subCount = 0;
            foreach (var it in chunkList)
            {
                if (chunks.ContainsKey(it.Name))
                {
                    var obj = chunks[it.Name];
                    it.ID = obj["id"].Value<int>();
                    it.Socket?.Dispose();
                    if (it.Own) // Pub
                    {
                        it.Socket = new PublisherSocket();
                        it.Socket.Bind("tcp://*:" + it.Port.ToString());
                        recordSize += it.Buffer.Length;
                    }
                    else
                    {
                        ++subCount;
                        it.Socket = new SubscriberSocket();
                        ((SubscriberSocket)it.Socket).Subscribe(System.Text.Encoding.UTF8.GetBytes(it.Name));
                        it.Socket.ReceiveReady += SocketReady;
                        Poller.Add(it.Socket);
                        var name = "inproc://monitor-" + it.Name;
                        var monitor = new NetMQMonitor(it.Socket, "inproc://monitor-" + it.Name, SocketEvents.Connected)
                        {
                            Timeout = TimeSpan.FromMilliseconds(3000)
                        };
                        monitor.Connected += (s, e) =>
                        {
                            ++connected;
                        };
                        Task.Factory.StartNew(monitor.Start);
                        it.Socket.Connect(obj["path"].Value<string>());
                    }
                }
                else
                {
                    return (UInt16)ErrorCode.ERR_INFO;
                }
            }
            
            if (recordSize != 0)
            {
                recordBuffer = new byte[recordSize];
            }

            // 记录文件
            try
            {
                recordFileStream?.Dispose();
                var filename = recordFolder + "/" + RecordName + "_" + nodeName + DNVars.RECORD_FILE_SUFFIX;
                if (ReplayState == (int) ReplayStates.Replaying)
                {
                    recordFileStream = new FileStream(filename,FileMode.Open);
                    var buffer = new byte[sizeof(uint)];
                    recordFileStream.Read(buffer, 0, 4);
                    if (Utils.ByteToStructure<uint>(buffer) != DNVars.RECORD_FILE_MAGIC)
                    {
                        recordFileStream.Close();
                        recordFileStream.Dispose();
                        return (UInt16) ErrorCode.ERR_FILE_READ;
                    }
                }
                else if (ReplayState == (int) ReplayStates.Recording)
                {
                    recordFileStream = new FileStream(filename, FileMode.Create);
                    recordFileStream.Write(Utils.StructureToByte(DNVars.RECORD_FILE_MAGIC), 0, sizeof(uint));
                    recordFileStream.FlushAsync();
                    return (UInt16)ErrorCode.ERR_FILE_WRITE;
                }
            }
            catch (IOException)
            {
                ReplayState = (int)ReplayStates.ReplayNop;
                recordFileStream?.Dispose();
                recordFileStream = null;
                return (UInt16)ErrorCode.ERR_FILE_WRITE;
            }

            var t0 = System.DateTime.Now;
            var ok = false;
            while ((System.DateTime.Now - t0).TotalMilliseconds < 3000) // 等待三秒
            {
                if (connected >= subCount)
                {
                    ok = true;
                    break;
                }
                Thread.Sleep(1);
            }

            return (UInt16) (ok?ErrorCode.ERR_NOP:ErrorCode.ERR_SOCKET);
        }

        // 发送一个数据块
        private void SendChunk(Chunk chunk)
        {
            chunk.Socket.SendMoreFrame(chunk.Name).SendFrame(chunk.Buffer);
            if (ReplayState == (int)ReplayStates.Recording)
            {
                recordFileStream?.Write(chunk.Buffer,0,chunk.Buffer.Length);
            }
        }

        // 发布自己所有要发布的数据块
        private void SendChunks()
        {
            foreach (var it in chunkList.Where(it => it.Own))
            {
                SendChunk(it);
            }
        }

        // 加载下一帧数据
        private void LoadNext()
        {
            if (ReplayState != (int) ReplayStates.Replaying || recordFileStream == null) 
                return;
            try
            {
                if (recordFileStream.Read(recordBuffer, 0, recordSize) < recordSize)
                {
                    ReplayState = (int)ReplayStates.ReplayNop;
                    recordFileStream?.Dispose();
                    recordFileStream = null;
                    return;
                }
                var offset = 0;
                foreach (var it in chunkList.Where(it => it.Own))
                {
                    it.Buffer = recordBuffer.Skip(offset).ToArray();
                    offset += it.Buffer.Length;
                }
            }
            catch (IOException)
            {
                ReplayState = (int) ReplayStates.ReplayNop;
                recordFileStream?.Dispose();
                recordFileStream = null;
                return;
            }
        }

        // 加载前一帧数据
        private void LoadBack()
        {
            if (ReplayState != (int)ReplayStates.Replaying || recordFileStream == null)
                return;
            recordFileStream.Seek(-2 * recordSize, SeekOrigin.Current);
            LoadNext();
        }

        // 数据块
        private Dictionary<string, int> chunkDict = new Dictionary<string, int>();
        // 数据块顺序
        private List<Chunk> chunkList = new List<Chunk>();
        // 本节点ID
        private int ID = -1;
        // 记录文件流
        private FileStream recordFileStream;
        // 记录文件一帧大小
        private int recordSize;
        // 缓冲区
        private byte[] recordBuffer;
        // slow线程
        private Thread slowThread;
        // slow锁
        private Mutex slowMutex = new Mutex(false);
        // slow标志
        private bool slowRunning = false;
    }
}