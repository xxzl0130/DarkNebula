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
        public byte[] Buffer;
        // socket
        public NetMQ.NetMQSocket Socket = null;
        // 链接
        public string Path;

        public Chunk()
        {
        }

        public Chunk(string name, int size, bool own, string path = "")
        {
            Name = name;
            Own = own;
            Buffer = new byte[size];
            Path = path;
        }
    }
}