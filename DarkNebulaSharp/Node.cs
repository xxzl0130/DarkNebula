using System;
using System.Threading;
using NetMQ;
using NetMQ.Sockets;

namespace DarkNebulaSharp
{
    public abstract class Node
    {
        protected Node()
        {
            PubSocket = null;
            SubSocket = null;
            WorkThread = null;
            WorkStop = false;
            simSteps = 0;
            curTime = 0;
            SimTime = 10;
            SimFree = false;
            StepTime = 10;
            simState = SimStates.SimNop;
            ReplayState = (int) ReplayStates.ReplayNop;
            RecordName = "";
            WorkMutex = new Mutex(false);
            Poller = new NetMQPoller();
            OutBuffer = new byte[1*1024*1024];
            InBuffer = null;
        }

        ~Node()
        {
            StopWorking();
        }

        // 发布指令的socket
        protected PublisherSocket PubSocket;
        // 接收回报的socket
        protected SubscriberSocket SubSocket;
        // 工作线程
        protected Thread WorkThread;
        // 工作线程停止标志
        protected bool WorkStop;
        // 工作锁
        protected Mutex WorkMutex;

        protected UInt32 simSteps;
        // 仿真步数
        public UInt32 SimSteps { get => simSteps; }

        protected double curTime;
        // 当前仿真时间
        public double CurTime { get => curTime; }

        // 最长仿真时间
        public double SimTime { get; set; }
        // 自由仿真类型
        public bool SimFree { get; set; }
        // 仿真步长,ms
        public UInt32 StepTime { get; set; }

        protected SimStates simState;
        // 仿真状态
        public SimStates SimState { get => simState; }
        // 回放状态
        protected int ReplayState;
        // 记录名称
        public string RecordName { get; set; }
        // poller
        protected NetMQPoller Poller;

        // 开始工作线程
        protected void StartWorking()
        {
            if (WorkThread != null)
            {
                StopWorking();
                WorkThread = null;
            }

            WorkThread = new Thread(Working) {IsBackground = true};
            WorkThread.Start();
        }
        // 停止工作线程
        protected void StopWorking()
        {
            WorkStop = true;
            Poller?.Dispose();
            WorkMutex.WaitOne();
            WorkStop = false;
            WorkThread = null;
        }
        // 工作线程
        protected abstract void Working();
        // socket事件响应
        protected abstract void SocketReady(object sender, NetMQSocketEventArgs e);

        // 输出缓冲
        protected byte[] OutBuffer;
        // 输入缓冲
        protected byte[] InBuffer;
    }
}