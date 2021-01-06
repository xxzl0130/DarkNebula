using System;

namespace DarkNebulaSharp
{
    internal class Chunk
    {
        // utf-8保存的名称
        public byte[] Name;
        // 所有权限
        public bool Own = false;
        // 端口
        public UInt16 Port = 0;
        // 编号
        public int ID = 0;
        // 缓冲区
        public NetMQ.Msg Buffer;
        // socket
        public NetMQ.NetMQSocket Socket = null;

        public Chunk()
        {
        }

        public Chunk(string name, int size, bool own)
        {
            Name = System.Text.Encoding.UTF8.GetBytes(name);
            Own = own;
            Buffer = new NetMQ.Msg();
            Buffer.InitPool(size);
        }
    }
}