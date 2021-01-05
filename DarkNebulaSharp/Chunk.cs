using System;

namespace DarkNebulaSharp
{
    internal class Chunk
    {
        // 名称
        public string Name;
        // 所有权限
        public bool Own = false;
        // 端口
        public UInt16 Port = 0;
        // 编号
        public int ID = 0;
        // 缓冲区
        public NetMQ.Msg Buffer;
        // socket
        public NetMQ.Sockets.SubscriberSocket Socket = null;

        public Chunk()
        {
        }

        public Chunk(string name, int size, bool own)
        {
            Name = name;
            Own = own;
            Buffer = new NetMQ.Msg();
            Buffer.InitPool(size);
        }
    }
}