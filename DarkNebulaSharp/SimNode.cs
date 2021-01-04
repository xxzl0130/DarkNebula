namespace DarkNebulaSharp
{
    public class SimNode : Node
    {
        public SimNode()
        {
            running = false;
        }

        ~SimNode()
        {
            StopWorking();
        }

        protected override void Working()
        {
            throw new System.NotImplementedException();
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

        private bool running;
    }
}