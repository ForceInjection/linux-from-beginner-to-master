# Linux 网络优化之网卡及内核相关参数

[参考文档](broadcom-ethernet-network-adapter-user-guide.pdf)

| 命令 | 描述与分析 |
|------|------------|
| `ethtool -s ethX speed 25000 autoneg off` | **描述**：将链路速度强制设定为25 Gbps。如果某一端口链路已启用，则驱动程序不允许其他端口设置不同速度。<br>**分析**：该命令适用于需要固定高速率的情况，通常用于服务器端口或需要确定网络速率的接口。关闭自动协商可以避免链路速率变化的波动。 |
| `ethtool -i ethX` | **描述**：显示 `ethX` 接口的驱动、固件和软件包版本信息。<br>**分析**：用于排查驱动或固件版本相关问题，特别是当网络性能或稳定性出现异常时。 |
| `ethtool -k ethX` | **描述**：显示 `ethX` 接口的卸载功能。<br>**分析**：了解接口是否支持硬件卸载，有助于优化性能，降低CPU负载。 |
| `ethtool -K ethX tso off` | **描述**：关闭 TCP 分段卸载（TSO），该功能将 TCP 数据包分段任务卸载至网卡。<br>**分析**：关闭 TSO 可以在某些网络配置中提高精确控制，但可能会增加 CPU 使用率。 |
| `ethtool -K ethX gro off lro off` | **描述**：关闭通用接收卸载（GRO）和大型接收卸载（LRO）。<br>**分析**：关闭这些卸载功能可能适用于高性能应用场景中，通过减少延迟来提高网络响应速度。 |
| `ethtool -g ethX` | **描述**：显示 `ethX` 接口的当前环形缓冲区大小。<br>**分析**：查看缓冲区大小，了解数据包的缓冲空间，可用于网络性能优化。 |
| `ethtool -G ethX rx N` | **描述**：将 `ethX` 接口的接收（RX）环形缓冲区大小设置为 `N`。<br>**分析**：调整缓冲区大小以匹配网络流量需求，有助于减少丢包现象。 |
| `ethtool -S ethX` | **描述**：获取 `ethX` 接口的统计数据。<br>**分析**：统计信息有助于分析接口性能、排查问题和进行流量监控。 |
| `ethtool -l ethX` | **描述**：显示 `ethX` 接口的接收和发送队列数量。<br>**分析**：了解队列配置，有助于理解和优化网络流量的处理能力。 |
| `ethtool -L ethX rx 0 tx 0 combined M` | **描述**：将 `ethX` 的组合队列数设置为 `M`，用于优化性能。<br>**分析**：调整组合队列数量可以平衡 CPU 负载和网络吞吐量，以适应多线程网络应用。 |
| `ethtool -C ethX rx-frames N` | **描述**：设置中断聚合参数，将多个帧聚合成一个中断。可以配置不同参数以优化中断频率。<br>**分析**：调整中断参数可以减少高流量下的CPU中断频率，从而提高网络性能。 |
| `ethtool -x ethX` | **描述**：显示接收端流量分散（RSS）哈希分流表和RSS密钥。<br>**分析**：了解RSS配置有助于优化多核处理器上的网络性能。 |
| `ethtool -s ethX autoneg on speed 10000 duplex full` | **描述**：启用自动协商，并将 `ethX` 接口速度设置为10 Gbps，全双工模式。<br>**分析**：允许自动协商可在不需要固定速率的环境中提供适应性，适用于动态网络条件。 |
| `ethtool --show-eee ethX` | **描述**：显示 `ethX` 接口的节能以太网（EEE）状态。<br>**分析**：用于查看EEE节能状态，可以帮助在不影响性能的情况下降低功耗。 |
| `ethtool --set-eee ethX eee off` | **描述**：禁用 `ethX` 的 EEE 功能。<br>**分析**：在对低延迟需求较高的场景中，禁用 EEE 可以减少延迟。 |
| `ethtool --set-eee ethX eee on tx-lpi off` | **描述**：启用 EEE，但禁用低功耗空闲（LPI）。<br>**分析**：在部分节能的需求中，可以在启用EEE的同时禁用LPI以减少可能的性能损失。 |
| `ethtool -L ethX combined 1 rx 0 tx 0` | **描述**：禁用接收端流量分散（RSS），将组合队列数设置为1。<br>**分析**：关闭 RSS 可以在低流量环境中降低系统开销，但会降低多核处理能力。 |
| `ethtool -K ethX ntuple off` | **描述**：通过禁用 `ntuple` 过滤器来禁用加速接收流量分散（RFS）。<br>**分析**：适用于不需要接收流量分流的场景。 |
| `ethtool -K ethX ntuple on` | **描述**：启用加速RFS功能。<br>**分析**：启用加速RFS对高性能服务器场景有帮助，有助于分流流量并优化性能。 |
| `ethtool -t ethX` | **描述**：对 `ethX` 执行诊断自检。<br>**分析**：便于检测硬件故障，特别适用于故障排查和性能异常的网络接口。 |
| `ethtool -m ethX` | **描述**：查询和解码模块的EEPROM信息和光学诊断数据（如果支持）。<br>**分析**：帮助诊断光纤网络问题，适用于需要精确诊断的场景。 |
| `echo 32768 > /proc/sys/net/core/rps_sock_flow_entries` `echo 2048 > /sys/class/net/ethX/queues/rx-X/rps_flow_cnt` | **描述**：启用 `ethX` 的 RX 环 `X` 的接收流量分散（RFS）。<br>**分析**：RFS可用于优化多核 CPU 下的网络流量分发，提高应用程序性能。 |
| `sysctl -w net.core.busy_read=50` | **描述**：将设备接收环的读取时间设置为50微秒。降低等待数据到达的应用程序的延迟，但会提高 CPU 占用。<br>**分析**：在一些低延迟应用中可以提高响应速度，适用于要求高实时性的场景。 |
| `echo 4 > /sys/class/net/<NAME>/device/sriov_numvfs` | **描述**：启用单根 I/O 虚拟化（SR-IOV），在指定接口上创建4个虚拟功能（VF）。<br>**分析**：SR-IOV用于虚拟化环境中，将网络资源虚拟化为独立的VFs，提供隔离性和资源管理。 |
| `ip link set ethX vf 0 mac 00:12:34:56:78:9a` | **描述**：为 `ethX` 的 VF 0 设置 MAC 地址。<br>**分析**：用于控制VF的网络识别，可在虚拟化场景中为每个VF配置唯一的MAC地址。 |
| `ip link set ethX vf 0 state enable` | **描述**：启用 `ethX` 的 VF 0 链路状态。<br>**分析**：控制VF的启用和禁用，用于虚拟机网络的状态管理。 |
| `ip link set ethX vf 0 vlan 100` | **描述**：为 `ethX` 的 VF 0 指定 VLAN ID 100。<br>**分析**：在多租户环境中，通过为VF分配VLAN可以实现网络隔离。 |
| `modprobe 8021q;` `ip link add link <NAME> name <VLAN Interface Name> type vlan id <VLAN ID>` | **描述**：加载 `8021q` 模块以添加VLAN支持，然后创建指定 VLAN ID 的 VLAN 接口。例如，`modprobe 8021q; ip link add link ens3 name ens3.2 type vlan id 2` 在接口 `ens3` 上创建 VLAN ID 为 2 的接口。<br>**分析**：该命令组合适用于多租户或VLAN隔离环境，通过VLAN接口可以管理不同的子网。 |