﻿using System;

namespace DarkNebulaSharp
{
    public enum SimStates
    {
		SimNop = 0,
        SimInit,        // 初始化
        SimStop,        // 停止中
        SimRun = 0x10,  // 运行中
        SimPause,       // 暂停中
        SimStep,        // 单步模式
	}

    public enum ReplayStates
    {
        ReplayNop = 0,
        Recording = 0x10,       // 记录数据
        Replaying = 0x20,		// 重放数据
    }

    // 指令
    public enum CommandCode
    {
        COMMAND_NOP = 0,
        COMMAND_INIT,               // 初始化
        COMMAND_REG,                // 注册
        COMMAND_START,              // 开始
        COMMAND_STEP_FORWARD,       // 向前推进一步
        COMMAND_STEP_BACKWARD,      // 向后推进一步
        COMMAND_STEP_IN,            // 单步模式
        COMMAND_PAUSE,              // 暂停
        COMMAND_STOP,               // 结束
    };

    public struct CommandHeader
    {
        // 节点ID，-1为全部
        public Int32 ID;
        public Int32 code;
        public UInt32 size;
    }

    public static class DNVars
    {
        public const int ALL_NODE = -1;
        public const string COMMAND_TOPIC = "command";
        public const int COMMAND_TOPIC_LEN = 7;
        public const string REPLY_TOPIC = "reply";
        public const int REPLY_TOPIC_LEN = 5;
        public const UInt16 ADMIN_RECEIVE_PORT = 6666;
        public const UInt16 ADMIN_SEND_PORT = 8888;
        public const UInt16 CHUNK_PORT = 10000;
        public const string RECORD_FILE_SUFFIX = ".dnr";
        public const UInt32 RECORD_FILE_MAGIC = 0x44417A9F;
    }

    public delegate void SimEventDelegate();

    public delegate void SimStepDelegate(uint step, double time);
}