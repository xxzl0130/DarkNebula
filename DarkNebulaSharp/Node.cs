using System;
using System.Threading;
using NetMQ;
using NetMQ.Sockets;
using NetMQ.Core;

namespace DarkNebulaSharp
{
    abstract class Node
    {
        protected Node()
        {
            PubSocket = null;
            SubSocket = null;
            WorkThread = null;
            WorkStop = false;
            SimSteps = 0;
            CurTime = 0;
            SimTime = 10;
            SimFree = false;
            StepTime = 10;
            SimState = SimStates.SimNop;
            ReplayState = (int) ReplayStates.ReplayNop;
            RecordName = "";
            WorkMutex = new Mutex(false);
        }

        ~Node()
        {
        }

        // 发布指令的socket
        protected PublisherSocket PubSocket;
        // 接收回报的socket
        protected SubscriberSocket SubSocket;
        // 工作线程
        protected Thread WorkThread;
        // 监听线程停止标志
        protected bool WorkStop;
        // 监听锁
        protected Mutex WorkMutex;
        
        // 仿真步数
        public UInt32 SimSteps { get; }

        // 当前仿真时间
        public double CurTime { get; }
        // 最长仿真时间
        public double SimTime { get; set; }
        // 自由仿真类型
        public bool SimFree { get; set; }
        // 仿真步长,ms
        public UInt32 StepTime { get; set; }
        // 仿真状态
        public SimStates SimState { get; }
        // 回放状态
        protected int ReplayState;
        // 记录名称
        public string RecordName { get; set; }

        // 开始工作线程
        protected void StartWorking()
        {
            if (WorkThread != null)
            {
                StopWorking();
                WorkThread = null;
            }

            WorkThread = new Thread((() =>
            {
                WorkMutex.WaitOne();
                while (!WorkStop)
                {
                    Working();
                }
                WorkMutex.ReleaseMutex();
            })) {IsBackground = true};
            WorkThread.Start();
        }
        // 停止工作线程
        protected void StopWorking()
        {
            WorkStop = true;
            WorkMutex.WaitOne();
            WorkStop = false;
            WorkThread = null;
        }
        // 工作线程
        protected abstract void Working();
    }
}