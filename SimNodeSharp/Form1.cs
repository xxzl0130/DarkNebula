using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using DarkNebulaSharp;

namespace SimNodeSharp
{
    public partial class Form1 : Form
    {
        private SimNode simNode;
        private int counter = 0;
        public Form1()
        {
            InitializeComponent();
            simNode = new SimNode
            {
                // 以下初始化参数根据项目自定义
                NodeName = "SimNodeSharp",
                NodeIP = "127.0.0.1",
                AdminIP = "127.0.0.1",
                AdminRecvPort = DNVars.ADMIN_RECEIVE_PORT,
                AdminSendPort = DNVars.ADMIN_SEND_PORT,
                ChunkPort = DNVars.CHUNK_PORT + 100,
                SlowNode = false
            };

            simNode.AddChunk("counter", 4, false);

            simNode.InitCallback = InitCallback;
            simNode.PauseCallback = PauseCallback;
            simNode.StopCallback = StopCallback;
            simNode.StartCallback = StartCallback;
            simNode.SimStepCallback = SimStepCallback;

            simNode.RegIn();
        }

        private void StartCallback()
        {
            // 由于回调函数在子线程中，要操作ui必须回到ui线程，下面各回调都一样
            this.Invoke(new Action(() =>
            {
                this.stateLabel.Text = "运行";
                this.regButton.Enabled = false;
            }));
        }

        private void SimStepCallback(uint step, double time)
        {
            // 将耗时的仿真操作放在子线程中
            // 需要手动获取数据
            simNode.GetChunkData("counter", out counter);
            // 下面是写入数据的示例，用于更新自己发布的数据
            // 这里没有写入权限，不会报错但也不会更新数据
            simNode.SetChunkData("counter", counter);
            // 回到ui线程操作ui
            this.Invoke(new Action(() =>
            {
                this.stepLabel.Text = step.ToString();
                this.timeLabel.Text = time.ToString("F2");
                this.valueLabel.Text = counter.ToString();
                this.stateLabel.Text = "运行";
            }));
        }

        private void StopCallback()
        {
            this.Invoke(new Action(() =>
            {
                this.stateLabel.Text = "停止";
                this.regButton.Enabled = true;
            }));
        }

        private void PauseCallback()
        {
            this.Invoke(new Action(() =>
            {
                this.stateLabel.Text = "暂停";
            }));
        }

        private void InitCallback()
        {
            this.Invoke(new Action(() =>
            {
                this.stateLabel.Text = "初始化";
            }));
        }

        private void regButton_Click(object sender, EventArgs e)
        {
            simNode.RegIn();
        }
    }
}
