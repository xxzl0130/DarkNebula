using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using NetMQ;
using NetMQ.Sockets;
using DarkNebulaSharp;

namespace SharpTest
{
    class Program
    {
        private static int counter = 0;
        static void Main(string[] args)
        {
            var node = new DarkNebulaSharp.SimNode
            {
                ChunkPort = 20000, NodeName = "sharp", AdminRecvPort = 16666, AdminSendPort = 18888, 
                SlowNode = false
            };
            node.AddChunk("counter", sizeof(int), false);
            node.SimStepCallback = (step, time) =>
            {
                node.GetChunkData("counter",out counter);
                //counter++;
                //node.SetChunkData("counter", counter);
                Console.WriteLine(counter);
                //Thread.Sleep(1500);
            };
            node.ReplayStepCallback = (step, time) =>
            {
                node.GetChunkData("counter", out counter);
                Console.WriteLine(counter);
            };
            Console.ReadKey();
            node.RegIn();

            var sub = new SubscriberSocket("tcp://127.0.0.1:20000");
            sub.Subscribe("counter");
            var poller = new NetMQPoller {sub};
            sub.ReceiveReady += Sub_ReceiveReady;
            poller.RunAsync();

            Console.ReadKey();
            poller.Dispose();
        }

        private static void Sub_ReceiveReady(object sender, NetMQSocketEventArgs e)
        {
            var bytes = e.Socket.ReceiveFrameBytes();
            Console.WriteLine("sub:" + bytes[0]);
        }

        public static byte[] StructureToByte<T>(T structure)
        {
            int size = Marshal.SizeOf(typeof(T));
            byte[] buffer = new byte[size];
            IntPtr bufferIntPtr = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(structure, bufferIntPtr, true);
                Marshal.Copy(bufferIntPtr, buffer, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(bufferIntPtr);
            }
            return buffer;
        }

        public static T ByteToStructure<T>(byte[] dataBuffer)
        {
            object obj = null;
            int size = Marshal.SizeOf(typeof(T));
            IntPtr allocIntPtr = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.Copy(dataBuffer, 0, allocIntPtr, size);
                obj = Marshal.PtrToStructure(allocIntPtr, typeof(T));
            }
            finally
            {
                Marshal.FreeHGlobal(allocIntPtr);
            }
            return (T)obj;
        }
    }
}
