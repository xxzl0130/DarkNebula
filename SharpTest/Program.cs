﻿using System;
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

        static void Main(string[] args)
        {
            var node = new DarkNebulaSharp.SimNode
            {
                ChunkPort = 20000, NodeName = "sharp", AdminRecvPort = 16666, AdminSendPort = 18888
            };
            node.AddChunk("counter", sizeof(int), false);
            Console.ReadKey();
            node.RegIn();

            Console.ReadKey();
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
