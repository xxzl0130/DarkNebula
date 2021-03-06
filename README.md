# DarkNebula
基于ZeroMQ的分布式仿真系统  

## 设计思路
主要利用0MQ的PUB/SUB模式，实现分布式的数据分发共享、管理节点对仿真节点的控制和监控。  

## 功能
- 管理节点向仿真节点发送仿真指令  
- 仿真节点向管理节点回报仿真状态  
- 仿真节点实现记录数据  
- 仿真节点读取数据文件重现历史  
- 管理节点可以控制仿真节点以仿真还是重放形式运行  

## 实现思路
### 管理节点
监听仿真节点回报topic，各仿真节点向这个topic发送自己的状态信息。  
发布指令topic，各仿真节点监听该topic，接收指令并做进一步处理。  
#### 初始化流程
1. 发布初始化指令  
2. 仿真节点回报节点名称、数据块信息  
3. 为节点和数据块分别分配id和端口号，通过广播发布，后续通信均以id为标识，提高效率  
4. 各仿真节点在收到分配信息后开始连接要接收的数据块的topic，全部连接成功后回报初始化完成  
5. 所有节点初始化成功后记为初始化成功  

#### 仿真流程
1. 设定仿真步长、仿真速度  
2. 设置高精度定时器，单位1ms
3. 检查是否到达该推进仿真的时间
4. 检查各节点是否完成仿真
5. 推进仿真/结束仿真

### 仿真节点
监听管理指令topic，监听要接收的数据块topic，发布数据块topic。  
要接收的topic加入poll列表，在接收线程中循环接收数据。  
收到仿真指令后调用相应回调函数，再发送回报数据。  

### 数据块
数据块类要保存数据块的大小、指针、名称、ID、读写权限等信息，同时保存socket相关信息，但不需要由用户手动创建，而是由仿真节点管理。  
用户程序中创建一个实际使用的数据对象，数据块中创建一个同样大小的buffer，收到数据后存在buffer中，只在仿真推进前将数据更新到用户侧数据块，以避免访问冲突，也无需加锁（因为所有数据接收在一个线程）。  

## 依赖
### C++
* [vcpkg](https://github.com/microsoft/vcpkg)  
* [libzmq](https://github.com/zeromq/libzmq)，使用`vcpkg install zeromq:x64-windows`命令安装。  
* [nlohmann-json](https://github.com/nlohmann/json)，使用`vcpkg install nlohmann-json:x64-windows`命令安装。  
直接使用dll不需要安装依赖库，导入dll即可。  

### C#
* [NetMQ](https://github.com/zeromq/netmq)，使用NuGet安装即可。  
* [Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json)，使用NuGet安装即可。
使用C#库需要在客户端也引用这两个库。  

## 使用
### C++
可参考`SimNodeCpp`项目，该项目以Qt框架编写，通过继承SimNode和QObject实现仿真节点，并将仿真运行在子线程，将数据通过Qt的信号/槽机制传输给UI线程。  
C++中的数据块在内部保存了指针，并在每次仿真调用前后自动更新和发布，不需要手动更新数据。  
实际项目中参考示例将各个仿真函数修改实现，再编写界面即可。  

### C#
可参考`SimNodeSharp`项目，该项目为Windows Form程序，程序中直接实例化了SimNode然后设置了回调函数，没有像C++项目那样以继承的方式实现。  
C#中的数据块无法实现自动更新，需要在每次仿真推进中手动获取和写入数据。  
另外需要注意，回调函数也同样处于子线程中，操作UI界面需要注意回到UI线程。  