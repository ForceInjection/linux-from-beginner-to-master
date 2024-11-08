# TCP 统计信息详解

[原文](https://github.com/moooofly/MarkSomethingDown/blob/master/Linux/TCP%20%E7%9B%B8%E5%85%B3%E7%BB%9F%E8%AE%A1%E4%BF%A1%E6%81%AF%E8%AF%A6%E8%A7%A3.md)

Linux 上 TCP 相关统计信息包含在如下文件中

- `/proc/net/netstat`;
- `/proc/net/snmp`.

我们可以通过 `netstat -s` 的输出信息进行确认；

```bash
strace -e openat netstat -s
...
openat(AT_FDCWD, "/proc/net/snmp", O_RDONLY) = 3
openat(AT_FDCWD, "/proc/net/netstat", O_RDONLY) = 3
openat(AT_FDCWD, "/proc/net/sctp/snmp", O_RDONLY) = -1 ENOENT (No such file or directory)
...
Ip:
    1814407251 total packets received
    215059641 forwarded
    0 incoming packets discarded
    1401868523 incoming packets delivered
    1823238210 requests sent out
    6 outgoing packets dropped
Icmp:
    187 ICMP messages received
    0 input ICMP message failed.
    ICMP input histogram:
        destination unreachable: 42
        echo requests: 135
        echo replies: 10
    705 ICMP messages sent
    0 ICMP messages failed
    ICMP output histogram:
        destination unreachable: 200
        redirect: 360
        echo request: 10
        echo replies: 135
IcmpMsg:
        InType0: 10
        InType3: 42
        InType8: 135
        OutType0: 135
        OutType3: 200
        OutType5: 360
        OutType8: 10
Tcp:
    12712465 active connections openings
    5039448 passive connection openings
    476 failed connection attempts
    1238262 connection resets received
    948 connections established
    1348953352 segments received
    1503539050 segments send out
    134791 segments retransmited
    9 bad segments received.
    6204821 resets sent
Udp:
    52103933 packets received
    189 packets to unknown port received.
    0 packet receive errors
    4614230 packets sent
    0 receive buffer errors
    0 send buffer errors
UdpLite:
TcpExt:
    328157 invalid SYN cookies received
    12 resets received for embryonic SYN_RECV sockets
    6 ICMP packets dropped because socket was locked
    6423227 TCP sockets finished time wait in fast timer
    23400281 delayed acks sent
    4565 delayed acks further delayed because of locked socket
    Quick ack mode was activated 19047 times
    17434 packets directly queued to recvmsg prequeue.
    3072 bytes directly in process context from backlog
    4157625 bytes directly received in process context from prequeue
    536430572 packet headers predicted
    1506 packets header predicted and directly queued to user
    111772835 acknowledgments not containing data payload received
    492286694 predicted acknowledgments
    20837 times recovered from packet loss by selective acknowledgements
    Detected reordering 12 times using FACK
    Detected reordering 94 times using SACK
    10 congestion windows fully recovered without slow start
    580 congestion windows recovered without slow start by DSACK
    462 congestion windows recovered without slow start after partial ack
    TCPLostRetransmit: 7698
    2257 timeouts after SACK recovery
    118 timeouts in loss state
    92180 fast retransmits
    1993 forward retransmits
    9019 retransmits in slow start
    1214 other TCP timeouts
    TCPLossProbes: 45260
    TCPLossProbeRecovery: 22742
    1149 SACK retransmits failed
    19322 DSACKs sent for old packets
    2828 DSACKs sent for out of order packets
    20300 DSACKs received
    36 DSACKs for out of order packets received
    3118053 connections reset due to unexpected data
    397720 connections reset due to early user close
    1086 connections aborted due to timeout
    TCPDSACKIgnoredOld: 22
    TCPDSACKIgnoredNoUndo: 18684
    TCPSpuriousRTOs: 117
    TCPSackShifted: 16039
    TCPSackMerged: 60997
    TCPSackShiftFallback: 80693
    IPReversePathFilter: 14794
    TCPRetransFail: 8
    TCPRcvCoalesce: 14735247
    TCPOFOQueue: 1040525
    TCPOFOMerge: 2795
    TCPChallengeACK: 76059
    TCPSYNChallenge: 9
    TCPSpuriousRtxHostQueues: 1
    TCPAutoCorking: 20582938
    TCPFromZeroWindowAdv: 1
    TCPToZeroWindowAdv: 1
    TCPWantZeroWindowAdv: 41
    TCPSynRetrans: 1066
    TCPOrigDataSent: 897044248
    TCPHystartTrainDetect: 1279
    TCPHystartTrainCwnd: 34543
    TCPHystartDelayDetect: 7
    TCPHystartDelayCwnd: 277
    TCPACKSkippedSeq: 1150
IpExt:
    InBcastPkts: 810862
    InOctets: 721988251165
    OutOctets: 707498001976
    InBcastOctets: 249386542
    InNoECTPkts: 2011429226
    InECT0Pkts: 98
```

可以看出 `/proc/net/netstat` 文件中包含：

- `TcpExt`
- `IpExt`

```bash
cat /proc/net/netstat

TcpExt: SyncookiesSent SyncookiesRecv SyncookiesFailed EmbryonicRsts PruneCalled RcvPruned OfoPruned OutOfWindowIcmps LockDroppedIcmps ArpFilter TW TWRecycled TWKilled PAWSPassive PAWSActive PAWSEstab DelayedACKs DelayedACKLocked DelayedACKLost ListenOverflows ListenDrops TCPPrequeued TCPDirectCopyFromBacklog TCPDirectCopyFromPrequeue TCPPrequeueDropped TCPHPHits TCPHPHitsToUser TCPPureAcks TCPHPAcks TCPRenoRecovery TCPSackRecovery TCPSACKReneging TCPFACKReorder TCPSACKReorder TCPRenoReorder TCPTSReorder TCPFullUndo TCPPartialUndo TCPDSACKUndo TCPLossUndo TCPLostRetransmit TCPRenoFailures TCPSackFailures TCPLossFailures TCPFastRetrans TCPForwardRetrans TCPSlowStartRetrans TCPTimeouts TCPLossProbes TCPLossProbeRecovery TCPRenoRecoveryFail TCPSackRecoveryFail TCPSchedulerFailed TCPRcvCollapsed TCPDSACKOldSent TCPDSACKOfoSent TCPDSACKRecv TCPDSACKOfoRecv TCPAbortOnData TCPAbortOnClose TCPAbortOnMemory TCPAbortOnTimeout TCPAbortOnLinger TCPAbortFailed TCPMemoryPressures TCPSACKDiscard TCPDSACKIgnoredOld TCPDSACKIgnoredNoUndo TCPSpuriousRTOs TCPMD5NotFound TCPMD5Unexpected TCPSackShifted TCPSackMerged TCPSackShiftFallback TCPBacklogDrop PFMemallocDrop TCPMinTTLDrop TCPDeferAcceptDrop IPReversePathFilter TCPTimeWaitOverflow TCPReqQFullDoCookies TCPReqQFullDrop TCPRetransFail TCPRcvCoalesce TCPOFOQueue TCPOFODrop TCPOFOMerge TCPChallengeACK TCPSYNChallenge TCPFastOpenActive TCPFastOpenActiveFail TCPFastOpenPassive TCPFastOpenPassiveFail TCPFastOpenListenOverflow TCPFastOpenCookieReqd TCPSpuriousRtxHostQueues BusyPollRxPackets TCPAutoCorking TCPFromZeroWindowAdv TCPToZeroWindowAdv TCPWantZeroWindowAdv TCPSynRetrans TCPOrigDataSent TCPHystartTrainDetect TCPHystartTrainCwnd TCPHystartDelayDetect TCPHystartDelayCwnd TCPACKSkippedSynRecv TCPACKSkippedPAWS TCPACKSkippedSeq TCPACKSkippedFinWait2 TCPACKSkippedTimeWait TCPACKSkippedChallenge TCPWqueueTooBig
TcpExt: 0 0 328166 12 0 0 0 0 6 0 6423417 0 0 0 0 0 23401381 4565 19047 0 0 17434 3072 4157625 0 536449215 1506 111776451 492302869 0 20837 0 12 94 0 0 10 0 580 462 7698 0 2257 118 92180 1993 9019 1214 45260 22742 0 1149 0 0 19322 2828 20300 36 3118141 397735 0 1086 0 0 0 0 22 18684 117 0 0 16039 60997 80693 0 0 0 0 14794 0 0 0 8 14735740 1040528 0 2795 76061 9 0 0 0 0 0 0 1 0 20583658 1 1 41 1066 897074316 1279 34543 7 277 0 0 1150 0 0 0 0
IpExt: InNoRoutes InTruncatedPkts InMcastPkts OutMcastPkts InBcastPkts OutBcastPkts InOctets OutOctets InMcastOctets OutMcastOctets InBcastOctets OutBcastOctets InCsumErrors InNoECTPkts InECT1Pkts InECT0Pkts InCEPkts ReasmOverlaps
IpExt: 0 0 0 0 810886 0 722017288587 707527820077 0 0 249393982 0 0 2011501757 0 98 0 0
```

`/proc/net/snmp` 文件中包含：

- `Ip`
- `Icmp`
- `IcmpMsg`
- `Tcp`
- `Udp`
- `UdpLite`

```bash
cat /proc/net/snmp

Ip: Forwarding DefaultTTL InReceives InHdrErrors InAddrErrors ForwDatagrams InUnknownProtos InDiscards InDelivers OutRequests OutDiscards OutNoRoutes ReasmTimeout ReasmReqds ReasmOKs ReasmFails FragOKs FragFails FragCreates
Ip: 1 64 1814615909 0 0 215088193 0 0 1402022929 1823446932 6 0 0 0 0 0 0 0 0
Icmp: InMsgs InErrors InCsumErrors InDestUnreachs InTimeExcds InParmProbs InSrcQuenchs InRedirects InEchos InEchoReps InTimestamps InTimestampReps InAddrMasks InAddrMaskReps OutMsgs OutErrors OutDestUnreachs OutTimeExcds OutParmProbs OutSrcQuenchs OutRedirects OutEchos OutEchoReps OutTimestamps OutTimestampReps OutAddrMasks OutAddrMaskReps
Icmp: 187 0 0 42 0 0 0 0 135 10 0 0 0 0 705 0 200 0 0 0 360 10 135 0 0 0 0
IcmpMsg: InType0 InType3 InType8 OutType0 OutType3 OutType5 OutType8
IcmpMsg: 10 42 135 135 200 360 10
Tcp: RtoAlgorithm RtoMin RtoMax MaxConn ActiveOpens PassiveOpens AttemptFails EstabResets CurrEstab InSegs OutSegs RetransSegs InErrs OutRsts InCsumErrors
Tcp: 1 200 120000 -1 12713687 5039918 476 1238386 947 1349102897 1503701884 134798 9 6205402 0
Udp: InDatagrams NoPorts InErrors OutDatagrams RcvbufErrors SndbufErrors InCsumErrors
Udp: 52108719 189 0 4614687 0 0 0
UdpLite: InDatagrams NoPorts InErrors OutDatagrams RcvbufErrors SndbufErrors InCsumErrors
UdpLite: 0 0 0 0 0 0 0
```

本文主要讨论 TCP 和 IP 协议相关内容，并按照类别将上述文件内容进行了整合；

## 计数器分类
以下是 `netstat -s` 中各个 TCP 相关计数器的简要解释：

| **类别**| **计数器**| **说明**|
|-------------------|----------------------|----------|
| **常量**           | `RtoAlgorithm`       | 重传超时算法类型（例如Van Jacobson算法）。|
|                   | `RtoMin`             | 最小重传超时时间（毫秒）。|
|                   | `RtoMax`             | 最大重传超时时间（毫秒）。|
|                   | `MaxConn`            | 允许的最大并发连接数，-1 表示无限制。|
| **建链统计**        | `ActiveOpens`        | 主动连接建立次数（客户端发起）。|
|                   | `PassiveOpens`       | 被动连接建立次数（服务器端接受）。|
|                   | `AttemptFails`       | 建立连接失败次数。|
|                   | `CurrEstab`          | 当前处于已建立状态的连接数量。|
|                   | `EstabResets`        | 在建立状态下的连接重置次数。|
| **数据包统计**      | `InSegs`             | 收到的TCP段总数。|
|                   | `OutSegs`            | 发送的TCP段总数。|
|                   | `RetransSegs`        | 重传的TCP段数量。|
|                   | `InErrs`             | 收到的有错误的TCP段数量。|
|                   | `OutRsts`            | 发送的TCP重置段数量。|
|                   | `InCsumErrors`       | 校验和错误的入段数量。|
|                   | `EmbryonicRsts`      | 在连接建立过程中被重置的TCP连接数。|
| **Syncookies 相关**| `SyncookiesSent`     | 发送的syncookies数（在SYN洪水攻击下防止资源耗尽）。|
|                   | `SyncookiesRecv`     | 接收的syncookies数。|
|                   | `SyncookiesFailed`   | syncookies验证失败次数。|
| **TIME_WAIT 相关** | `TW`                 | 已进入TIME_WAIT状态的连接数。|
|                   | `TWRecycled`         | 被回收的TIME_WAIT状态的连接数。|
|                   | `TWKilled`           | 被杀掉的TIME_WAIT状态的连接数（因资源不足）。|
|                   | `TCPTimeWaitOverflow` | 超过TIME_WAIT连接的系统资源限制次数。|
| **RTO 相关**       | `TCPTimeouts`        | TCP超时发生次数。|
|                   | `TCPSpuriousRTOs`    | 错误RTO触发的次数。|
|                   | `TCPLossProbes`      | 用于丢失探测的发送次数。|
|                   | `TCPLossProbeRecovery` | 通过丢失探测恢复的连接次数。|
|                   | `TCPRenoRecoveryFail` | Reno恢复失败次数。|
|                   | `TCPSackRecoveryFail` | SACK恢复失败次数。|
|                   | `TCPRenoFailures`    | Reno恢复失败的总数。|
|                   | `TCPSackFailures`    | SACK恢复失败的总数。|
|                   | `TCPLossFailures`    | 发生丢失恢复失败的次数。|
| **Retrans 相关**   | `TCPFastRetrans`     | 快速重传的次数。|
|                   | `TCPForwardRetrans`  | 前向重传的次数。|
|                   | `TCPSlowStartRetrans` | 慢启动状态下的重传次数。|
|                   | `TCPLostRetransmit`  | 丢失重传次数。|
|                   | `TCPRetransFail`     | 重传失败次数。|
| **FastOpen 相关**  | `TCPFastOpenActive`  | 主动Fast Open的次数。|
|                   | `TCPFastOpenPassive` | 被动Fast Open的次数。|
|                   | `TCPFastOpenPassiveFail` | Fast Open 被动连接失败次数。|
|                   | `TCPFastOpenListenOverflow` | Fast Open侦听溢出次数。|
|                   | `TCPFastOpenCookieReqd` | 需要Fast Open Cookie的请求次数。|
| **MD5 相关**       | `TCPMD5NotFound`     | 未找到的MD5签名数（MD5签名验证失败）。|
|                   | `TCPMD5Unexpected`   | 未预期的MD5签名数。|
| **DelayedACK 相关**   | `DelayedACKs`        | 发送的延迟ACK数。|
|                   | `DelayedACKLocked`   | 锁定情况下延迟的ACK数。|
|                   | `DelayedACKLost`     | 丢失的延迟ACK数。|
|                   | `TCPSchedulerFailed` | 调度失败的次数。|
| **DSACK 相关**     | `TCPDSACKOldSent`    | 发送的老旧DSACK段数量。|
|                   | `TCPDSACKOfoSent`    | 发送的乱序DSACK段数量。|
|                   | `TCPDSACKRecv`       | 接收的DSACK段数量。|
|                   | `TCPDSACKOfoRecv`    | 接收的乱序DSACK段数量。|
|                   | `TCPDSACKIgnoredOld` | 忽略的老旧DSACK段数量。|
|                   | `TCPDSACKIgnoredNoUndo` | 忽略无撤销操作的DSACK段数量。|
| **Reorder 相关**   | `TCPFACKReorder`     | 基于FACK的重排序次数。|
|                   | `TCPSACKReorder`     | 基于SACK的重排序次数。|
|                   | `TCPRenoReorder`     | 基于Reno的重排序次数。|
|                   | `TCPTSReorder`       | 基于时间戳的重排序次数。|
| **Recovery 相关**  | `TCPRenoRecovery`    | 采用Reno的恢复次数。|
|                   | `TCPSackRecovery`    | 采用SACK的恢复次数。|
|                   | `TCPRenoRecoveryFail` | Reno恢复失败次数。|
|                   | `TCPSackRecoveryFail` | SACK恢复失败次数。|
| **Abort 相关**     | `TCPAbortOnData`     | 因收到数据而终止的连接数。|
|                   | `TCPAbortOnClose`    | 因关闭操作而终止的连接数。|
|                   | `TCPAbortOnMemory`   | 因内存不足而终止的连接数。|
|                   | `TCPAbortOnTimeout`  | 因超时而终止的连接数。|
|                   | `TCPAbortOnLinger`   | 因延迟而终止的连接数。|
|                   | `TCPAbortFailed`     | 终止失败的连接数。|
| **Reset 相关**     | `EstabResets`        | 在已建立连接状态下的重置次数。|
| **内存 Prune**     | `PruneCalled`        | 调用裁剪的次数（用于清除内存）。|
|                   | `RcvPruned`          | 接收的裁剪数据包数。|
|                   | `OfoPruned`          | 裁剪的乱序数据包数。|
|                   | `TCPMemoryPressures` | TCP内存压力触发次数。|
| **PAWS 相关**      | `PAWSPassive`        | 被动PAWS（保护免受老旧段影响）验证次数。|
|                   | `PAWSActive`         | 主动PAWS验证次数。|
|                   | `PAWSEstab`          | PAWS已建立连接的数量。|
| **Listen 相关**    | `ListenOverflows`    | 侦听队列溢出次数。|
|                   | `ListenDrops`        | 侦听队列丢弃次数。|
| **Undo 相关**      | `TCPFullUndo`        | 完全撤销的次数。|
|                   | `TCPPartialUndo`     | 部分撤销的次数。|
|                   | `TCPDSACKUndo`       | DSACK撤销的次数。|
|                   | `TCPLossUndo`        | 丢失恢复的撤销次数。|
|**快速路径与慢速路径**| `TCPHPHits`          | 高优先路径命中数。|
|                   | `TCPHPHitsToUser`    | 直接传递到用户的高优先路径命中数。|
|                   | `TCPPureAcks`        | 纯ACK数据包数（不包含数据的确认包）。|
|                   | `TCPHPAcks`          | 高优先路径ACK数据包数。｜

## 常量

这些常量是 Linux 3.10 中的默认值，仅在升级了内核版本时才需要关心一下这些值的变化。

| **名称**| **含义**|
|--------------|--------------------------------|
| `RtoAlgorithm` | 计算 TCP 重传超时 (RTO) 的算法类型，默认值为 1，采用 `Van Jacobson` `RTO` 算法，与 `RFC 2698` 标准一致。|
| `RtoMin`       | `RTO` 的最小值（毫秒），限制重传的最小等待时间，默认值为 **200ms**（即 1/5 HZ）。|
| `RtoMax`       | `RTO` 的最大值（毫秒），限制重传的最大等待时间，默认值为 **120s**（即 120 HZ）。|
| `MaxConn`      | TCP 连接的总数量上限，默认值为 -1（无上限）。实际连接数由系统资源（如内存、文件描述符）决定。|

## 连接统计相关

在这些统计值中，只有 **CurrEstab** 反映了系统的**当前状态**，而其他值则记录了**历史累计状态**。需要注意的是，**CurrEstab 包含了所有处于 `ESTABLISHED` 和 `CLOSE-WAIT` 状态的连接数**。

可以理解为：`ESTABLISHED` 和 `CLOSE-WAIT` 两种状态都表示在 `local => remote peer` 方向的连接尚未关闭。

| **名称**| **含义** |
|--------------|------|
| **ActiveOpens**   | **主动连接建立次数**：统计 TCP 连接从 `CLOSED` 状态直接转变为 `SYN-SENT` 状态的次数。在 `tcp_connect()` 函数中计数，相当于 SYN 包的发送次数（不包含重传）。 |
| **PassiveOpens**  | **被动连接建立次数**：统计 TCP 连接从 `LISTEN` 状态直接转变为 `SYN-RCVD` 状态的次数。但在 Linux 实现中，计数在三次握手成功、建立 `tcp_sock` 结构体后才加 1。 |
| **AttemptFails**  | **连接尝试失败次数**：统计 TCP 连接因不同原因而未能成功建立的次数，计算以下三种转换的总和：<br>a) `SYN-SENT` => `CLOSED` 次数<br>b) `SYN-RECV` => `CLOSED` 次数<br>c) `SYN-RECV` => `LISTEN` 次数<br><br>返回 `CLOSED` 的部分在 `tcp_done()` 中计数，返回 `LISTEN` 的部分在 `tcp_check_req()` 中计数。 |
| **EstabResets**   | **接收到连接重置次数**：统计 TCP 连接从 `ESTABLISHED` 或 `CLOSE-WAIT` 状态直接转变为 `CLOSED` 状态的次数，即包括以下两种情况的总和：<br>a) `ESTABLISHED` => `CLOSED` 次数<br>b) `CLOSE-WAIT` => `CLOSED` 次数<br><br>在 `tcp_set_state()` 函数中进行计数，若前一状态为 `ESTABLISHED` 或 `CLOSE-WAIT`，则计数加 1。 |
| **CurrEstab**     | **当前建立的连接数**：统计当前处于 `ESTABLISHED` 或 `CLOSE-WAIT` 状态的 TCP 连接数。在 `tcp_set_state()` 函数中处理，表示自连接进入 `ESTABLISHED` 状态后到进入 `CLOSED` 状态之前的连接数。 |

## 数据包统计相关

这些统计值反应的也是历史状态，独立的来看意义并不大。通常，我们会关注以下几个指标一段时间内的变化：

-  **（发送）TCP 分段重传占比**：`ΔRetransSegs / ΔOutSegs`。这个比例越低越好，如果超过20%，则应引起关注（具体阈值需根据实际情况确定）；
-  **（发送）RST 分段占比**：`ΔOutRsts / ΔOutSegs`。这个比例同样越低越好，一般应在1%以内；
-  **（接收）错误分段占比**：`ΔInErrs / ΔInSegs`。这个比例也应尽可能低，通常应在1%以内，由校验和错误导致的问题包比例应该更低。

| **名称**| **含义** |
|------------------|------|
| **InSegs**       | **段接收数量**：接收的所有 TCP 段总数，包括出错的段以及 `ESTABLISHED` 状态下接收的段。计数实现于 `tcp_v4_rcv()` 和 `tcp_v6_rcv()`。|
| **OutSegs**      | **段发送数量**：发送的所有 TCP 段总数，包含新数据包、重传包、SYN 包、SYN+ACK 包和 RST 包，但**不包括仅包含重传字节的段**。`RST` 包在 `tcp_v4_send_reset()` 中计数，`SYN-RECV` 和 `TIME-WAIT` 状态下的 ACK 包在 `tcp_v4_send_ack()` 中计数，IPv6 相关数据在 `tcp_v6_send_response()` 中计数，SYN+ACK 包在 `tcp_make_synack()` 中计数，其他包在 `tcp_transmit_skb()` 中计数。|
| **RetransSegs**  | **重传段数量**：重传的 TCP 段总数，即包含一个或多个先前已发送字节的段。重传的 `SYN` + `ACK` 包在 `tcp_v4_rtx_synack()` 和 `tcp_v6_rtx_synack()` 中计数，其他重传包在 `tcp_retransmit_skb()` 中计数。|
| **InErrs**       | **错误段接收数量**：接收到的所有有问题的 TCP 段总数，例如校验和错误的情况。序列号错误的包在 `tcp_validate_incoming()` 中计数，校验和出错的包在 `tcp_rcv_established()`、`tcp_v4_do_rcv()`、`tcp_v4_rcv()`、`tcp_v6_do_rcv()` 和 `tcp_v6_rcv()` 中计数。|
| **OutRsts**      | **发送 RST 段数量**：发送的带 `RST` 标记的 TCP 段总数。在 `tcp_v4_send_reset()`、`tcp_send_active_reset()` 和 `tcp_v6_send_response()` 中计数。|
| **InCsumErrors** | **校验和错误包接收数量**：统计收到的校验和错误的包，在内核 `3.10` 引入，进一步细化了 `InErrs` 指标，仅少部分 `InErrs` 属于此类型。 |
| **EmbryonicRsts**| **收到的 `SYN_RECV` 状态的 RST 数量**：当客户端在收到 `SYN` + `ACK` 后发送 `RST` 包时，该计数值增加，可能由恶意客户端行为引起。分析时，可计算每秒增加率或与其他事件比率进行比较，以判断该现象是长期积累还是频繁发生。|

## Syncookies 相关

**`syncookies`** 机制通常不会被触发，只有在 `tcp_max_syn_backlog` 队列满载时才会启动。因此，在一般情况下，`SyncookiesSent` 和 `SyncookiesRecv` 的值应为 0。但是，`SyncookiesFailed` 的值与 `syncookies` 机制是否触发无直接关系，因此可能会不为 0。其计数方式是：当一个处于 `LISTEN` 状态的 socket 收到不带 `SYN` 标志的数据包时，会调用 `cookie_v4_check()` 尝试验证 cookie 信息。如果验证失败，`SyncookiesFailed` 的计数便会加 1。

| **名称**| **含义** |
|----------------|------|
| **SyncookiesSent**   | **发送的 SYN cookies 数量**：当服务器的半连接队列（`SYN Queue`）已满，应用程序无法及时处理新连接时，内核使用 syncookie 技术向客户端发送带有特殊标记的 `SYN` + `ACK` 包，避免丢弃连接请求。|
| **SyncookiesRecv**   | **接收到的有效 SYN cookies 数量**：服务器在发送 syncookie 后，客户端正确返回此 cookie，且通过服务器验证的次数。|
| **SyncookiesFailed** |**接收到的无效 SYN cookies 数量**：服务器在发送 syncookie 后，客户端返回的 cookie 未通过验证，被视为无效的次数。|

> 注：`syncookies` 机制是为应对 **`SYN FLOOD`** 攻击而被提出来的。

## TIME-WAIT 相关

`TIME-WAIT` 状态是 TCP 协议状态机中的重要一环，通常出现在主动关闭连接的一方（一般是客户端）上，用于确保传输完成并防止旧连接数据干扰新连接。在客户端设备上，特别是用于频繁发起 HTTP 请求的客户端，可能会看到大量处于 `TIME-WAIT` 状态的 socket，`TIME-WAIT` 状态的数量（TW 值）通常接近 `TcpPassiveOpens` 的值。

| **名称**| **含义** |
|--------------------|------|
| **TW**                |  通过正常 `TIME-WAIT` 持续时间 (`TCP_TIMEWAIT_LEN`) 结束 `TIME-WAIT` 状态的 socket 数量。|
| **TWRecycled**        | 经超时回收的 `TIME-WAIT` 状态 socket 数量。当连接被重用（`net.ipv4.tcp_tw_reuse = 1` 开启）时，`TWRecycled` 计数会增加。|
| **TWKilled**          | 在**快速**计时器下结束 `TIME-WAIT` 状态的 TCP socket 数量，仅在 `net.ipv4.tcp_tw_recycle = 1` 时起作用。|
| **TCPTimeWaitOverflow** | 系统无法分配新的 `TIME-WAIT` socket 或 `TIME-WAIT` socket 计数 (`tw_count`) 超过 `tcp_max_tw_buckets` 限制时计数增加。在 `tcp_time_wait()` 函数中统计。|

## RTO 相关

`RTO`（重传超时）事件对 TCP 性能影响显著，因此监控 `RTO` 超时次数非常关键。频繁的 `RTO` 超时可能会导致延迟增大，网络传输效率降低。

在 Linux 内核 `3.10` 版本引入的 `TLP`（传输层探测）机制，能够将部分 `RTO` 事件转换为快速重传，从而减少 `TCPTimeouts` 的次数，有效降低因超时导致的性能损耗。


| **名称**| **含义** |
|-------------------------|----------|
| **TCPTimeouts**         | TCP 在 `RTO` 计时器下的超时次数：<br> a) 从 `CWR` 或 `Open` 状态下发生的首次 `RTO` 超时次数；<br> b) `SYN,ACK` 包的超时次数。 |
| **TCPSpuriousRTOs**     | `F-RTO` 机制检测到的虚假超时次数。|
| **TCPLossProbes**       | 发生 `PTO`（探测超时）事件导致发送 `TLP`（传输层探测）包的次数。 |
| **TCPLossProbeRecovery**| 由于 `TLP` 探测包成功修复丢失数据的次数。 |
| **TCPRenoRecoveryFail** | 进入 `Recovery` 阶段后仍然触发 `RTO` 超时的次数（对端不支持 `SACK` 选项）。 |
| **TCPSackRecoveryFail** | 进入 `Recovery` 阶段后仍然触发 `RTO` 超时的次数（对端支持 `SACK` 选项）。 |
| **TCPRenoFailures**     | 进入 `TCP_CA_Disorder` 阶段后发生 `RTO` 超时的次数（对端不支持 `SACK` 选项）。 |
| **TCPSackFailures**     | 进入 `TCP_CA_Disorder` 阶段后发生 `RTO` 超时的次数（对端支持 `SACK` 选项）。 |
| **TCPLossFailures**     | 进入 `TCP_CA_Loss` 阶段后发生 `RTO` 超时的次数。 |

## Retrans（重传）相关

这些计数器统计的重传包并不都是由于`RTO`（重传超时）导致的。如果将这些非`RTO`导致的重传包与`RetransSegs`的统计数据结合起来分析，我们可以得出一些结论：

- **非RTO重传比例**：若非`RTO`导致的重传占比很大，虽然这暗示了网络中存在一定的问题，但相比于`RTO`超时，这种情况通常被认为是较轻微的网络问题。
- **丢失重传包（LostRetransmit）**：`LostRetransmit`的数量应保持在较低水平。如果发现大量的重传包被丢弃，这可能表明网络中存在严重的问题，需要特别关注和进一步排查。


| **名称**| **含义** |
|----------------------|--------------------------|
| **TCPLostRetransmit**  | 丢失的重传 `skb` 数量，没有 `TSO` 时，等于丢失的重传包数量。|
| **TCPFastRetrans**     | 成功执行快速重传的 `skb` 数量。|
| **TCPForwardRetrans**  | 成功执行 `Forward Retransmissions` 的 `skb` 数量，这种重传涉及序号高于 `retransmit_hig` h的数据。|
| **TCPSlowStartRetrans**| 在慢启动状态下的重传次数，仅记录非 `RTO` 超时进入 `Loss` 状态下的重传数量。<br><br>目前找到的一种非 `RTO` 进入 `Loss` 状态的情况就是：`tcp_check_sack_reneging() `函数发现接收端违反(`renege`)了之前的 `SACK` 信息时，会进入 `Loss `状态。|
| **TCPRetransFail**     | 尝试执行快速重传（`FastRetrans`）、`Forward Retransmissions` 或慢启动重传（`SlowStartRetrans`）失败的次数。|

## TFO（TCP FastOpen）相关

**TCP FastOpen (TFO)** 是由 **Google** 提出的一项技术，旨在减少TCP三次握手的开销。其核心原理是在首次建立连接时，服务器计算出一个 **cookie** 并发送给客户端。随后，当客户端再次向服务器发起连接请求时，可以携带这个 **cookie** 以验证身份。如果 **cookie** 验证成功，服务器可以在未收到三次握手的最后一个 `ACK` 包之前，就将客户端在 `SYN` 包中的数据直接传递给应用层。

在 Linux `3.10` 内核及之后的版本中，TFO 功能由 `net.ipv4.tcp_fastopen` 这个 **sysctl** 开关控制，默认设置为 **0**（关闭状态）。目前，推荐保持 `TFO` 关闭，因为一些网络中的中间设备（`middle box`）可能会丢弃那些包含未知选项的 `SYN` 包。因此，在正常情况下，与 `TFO` 相关的计数器值应该为 **0**。然而，如果系统接收到了携带 `TFO` **cookie** 信息的 `SYN` 包，尤其是那些可能带有恶意的包，`TCPFastOpenPassive` 计数器的值可能会不为 **0**。

| **名称**| **含义**|
|------------------------|--------------------------------|
| **TCPFastOpenActive**        | 成功的主动出站 `TFO` 连接数量，即主动发送的、带 `TFO cookie` 的 `SYN` 包个数。|
| **TCPFastOpenActiveFail**     | 由于收到的 `SYN + ACK` 包未确认 `SYN` 包中携带的数据，导致的 `SYN` 数据重传次数。注意，此项不是指不支持 `TFO` 的服务器连接数，而是指带 `cookie` 和数据的初始 `SYN` 包未被确认时的重传次数。|
| **TCPFastOpenPassive**       | 成功建立的被动 `TFO` 入站连接数量，表示接收到带 `TFO cookie` 的 `SYN` 包的次数。|
| **TCPFastOpenPassiveFail**    | 由于接收到的带 `TFO cookie` 的入站 `SYN` 包包含无效 `cookie`，导致被动 `TFO` 连接建立失败的次数。 |
| **TCPFastOpenListenOverflow** | 入站 `SYN` 包数量超过监听队列最大长度，导致禁用 `TFO` 的次数。。|
| **TCPFastOpenCookieReqd**    | 收到请求 `TFO` 但未包含 `cookie` 的 `SYN` 包数量。每当收到这样的 `SYN` 包请求 `TFO cookie` 时，计数器加 1。|

## MD5 相关

`TCP MD5 Signature` 选项是为提高 BGP Session 的安全性而提出的，详见 [RFC 2385](https://tools.ietf.org/html/rfc2385)。因此，该功能在内核中是通过编译选项启用，而非通过 `sysctl` 接口配置的。如果内核编译时未启用 `CONFIG_TCP_MD5SIG` 选项，则不会支持 `TCP MD5 Signature`，下面两个计数器也将始终为 0。

| **名称**| **含义**|
|--------------------|--------------|
| **TCPMD5NotFound**    | 期望收到带有 MD5 选项的包，但实际收到的包中没有 MD5 选项时的计数。|
| **TCPMD5Unexpected**  | 不期望收到带有 MD5 选项的包，但实际收到的包中包含 MD5 选项时的计数。|

## DelayedACK 相关

`DelayedACK` 是内核默认支持的功能。即便启用 `DelayedACK`，每接收两个数据包仍需发送一个 `ACK`，因此 `DelayedACKs` 计数大致可估算为 `ACK` 的发送次数的一半。

`DelayedACKLocked` 计数则显示了应用程序与内核争用 `socket` 的情况。如果该计数在 `DelayedACKs` 中占比过高，可能需要分析应用程序的行为是否影响了正常通信。

| **名称**| **含义**|
|--------------------|--------------------------------|
| **DelayedACKs**       | 延迟发送的 ACK 数量：在等待第二个数据包以合并 `ACK` 发送时，如果未能等到，定时器会触发发送延迟的 ACK。每次调用 `tcp_send_ack()` 时计数增加，无论发送是否成功。触发点：`tcp_delack_timer()` |
| **DelayedACKLocked**  | 因 socket 被锁定而延迟的 ACK 次数：当试图发送延迟 ACK 时发现 socket 被锁定，则无法发送，并会重置定时器。触发点：`tcp_delack_timer()` |
| **DelayedACKLost**    | 快速 ACK 模式被激活的次数：遇到远程端重传的数据包，认为未接收到该数据包，从而发送了重复的延迟 ACK。<br> a) 当输入包不在接收窗口或 PAWS 校验失败时，计数加 1。触发点：`tcp_validate_incoming()->tcp_send_dupack()` <br> b) 若输入包的结束序列号小于 `RCV_NXT`，计数加 1。触发点：`tcp_data_queue()` |
| **TCPSchedulerFailed** | 延迟 ACK 处理期间预队列中仍有数据的次数：当数据加入 `prequeue` 后，期望 `userspace` 尽快处理（例如调用 `tcp_recvmsg()`）。若仍有数据未处理，可能暗示 `userspace` 处理速度较慢。触发点：`tcp_delack_timer()` |

## DSACK 相关

这些计数器用于统计 `DSACK`（`Duplicate SACK`）信息的发送和接收次数。

- `TCPDSACKOldSent` 与 `TCPDSACKOfoSent` 之和代表了发送出的 `DSACK` 信息总次数，通常 `OldSent` 的数量应该占主要部分。
- 类似地，`TCPDSACKRecv` 的计数往往远高于 `TCPDSACKOfoRecv`。
- DSACK 信息的发送依赖于 `net.ipv4.tcp_dsack` 配置项。如果 sent 计数器始终为零，建议检查该配置项是否已开启，**通常开启 DSACK** 可以提升网络性能。

| **名称**| **含义**|
|------------------------|-------------------------|
| **TCPDSACKOldSent**    | 为旧数据包发送的 `DSACK` 次数：如果接收到的重复数据包序号小于 `rcv_nxt`（接收端期望的下一个序号），则计数增加 1。|
| **TCPDSACKOfoSent**    | 为乱序数据包发送的 `DSACK` 次数：如果接收到的重复数据包序号大于 `rcv_nxt`，表明该包是乱序重复包，计数增加 1。|
| **TCPDSACKRecv**       | 接收到的 `DSACK` 次数：如果 `DSACK` 序号小于 `ACK` 序号，则视为旧的 `DSACK` 信息，计数增加 1。|
| **TCPDSACKOfoRecv**    | 接收到的乱序 `DSACK` 次数：如果 `DSACK` 序号大于 `ACK` 序号，表示该 `DSACK` 为乱序，计数增加 1。|
| **TCPDSACKIgnoredOld** | 重传时接收到重复的 `DSACK`，但丢弃了此 `DSACK` 信息：如果 `DSACK` 信息被判定为无效且已设置 `undo_marker`，则计数增加 1。|
| **TCPDSACKIgnoredNoUndo** | 接收到无效的 `DSACK` 信息并丢弃：如果 `DSACK` 信息无效且未设置 `undo_marker`，则计数增加 1。|

## Reorder 相关

当需要更新某条 TCP 连接的乱序值 (`reordering` 值) 时，以下计数器可能会被使用到。

这四个计数器是互斥关系，其中最少见的通常是 **TCPRenoReorder**，因为 `SACK` 已被广泛部署。

| **名称**| **含义**|
|-------------------|-----------------------|
| **TCPFACKReorder** | 使用 `FACK` 检测到乱序：`FACK`（`Forward ACK`）表示在使用 `SACK` 时，由对端确认接收的最高序列号，主要用于拥塞控制。|
| **TCPSACKReorder** | 使用 `SACK` 检测到乱序。|
| **TCPRenoReorder** | 使用快速重传 (`fast retransmit`) 检测到乱序。|
| **TCPTSReorder**   | 使用时间戳选项 (`timestamp option`) 检测到乱序。|

> 关于 partial ack 的完整内容可参考 [RFC6582](https://tools.ietf.org/html/rfc6582) ；

## Recovery 相关

该类型计数器统计的是进入快速重传阶段的总次数及失败次数；失败次数是指先进入 `Recovery` 阶段后又发生 `RTO` 超时，说明快速恢复（`Fast Recovery`）未成功。
由于 `SACK` 选项已被广泛应用，**TCPRenoRecovery** 的计数通常远小于 **TCPSackRecovery**。此外，**fail** 的次数越少越理想。

| **名称**| **含义**|
|------------------------|------------------------------------------------|
| **TCPRenoRecovery**    | 数据包丢失后通过快速重传进行恢复的次数（对端不支持 `SACK` 选项）。|
| **TCPSackRecovery**    | 使用选择性确认 (`SACK`) 功能从数据包丢失中恢复的次数。|
| **TCPRenoRecoveryFail** | 在不支持 `SACK` 的情况下，进入 `Recovery` 阶段后又发生 `RTO` 超时的次数。|
| **TCPSackRecoveryFail** | 在支持 `SACK` 的情况下，进入 `Recovery` 阶段后又发生 `RTO` 超时的次数。|

## Abort 相关

由于 **abort** 事件严重且通常伴随系统异常，因此这些计数器的状态应密切关注。后三个计数器若不为 0，往往预示着系统存在较为严重的问题，需特别留意。

| **名称**| **含义**|
|---------------------|------------------------------|
| **TCPAbortOnSyn**   | 收到非预期的 `SYN` 包时发送 `RST` 包给对端。例如，多个` NAT` 后的客户端使用相同的外部地址访问服务器时出现时间戳问题。|
| **TCPAbortOnData**  | 当处于 `FIN_WAIT_1` 或 `FIN_WAIT_2` 状态时收到超出连接范围的数据包，发送 `RST` 包。若 `TCP_LINGER2` 设置为负数，也会导致计数增加。         |
| **TCPAbortOnClose** | 当用户关闭 socket 但接收缓冲区仍有数据时，发送 `RST` 包。例如在 `tcp_close()` 中关闭 `socket` 且 `recv` 缓冲区有数据时，计数加 1。|
| **TCPAbortOnMemory**| **极为严重的问题**。发生在**孤立 `socket`(不包含文件句柄)** 数量或 `tcp_memory_allocated` 超过上限，内核需丢弃连接，有时发送 `RST` 包，有时不发送。|
| **TCPAbortOnTimeout** | 因各种计时器（`RTO`/`PTO`/`keepalive`）重传次数超过上限而导致的连接超时终止，计数加 1。|
| **TCPAbortOnLinger**  | 在 `tcp_close()` 中，`linger2` 被设置为负值时，`FIN_WAIT_2` 状态立即切换到 `CLOSED` 状态，计数增加。通常为 0。|
| **TCPAbortFailed**    | 在尝试发送 `RST` 时失败，例如因内存不足无法分配 `skb` 导致失败。一般计数为 0。|

## Reset 相关

TCP 连接中的 `Reset`（`RST`）操作用于立即终止不正常的连接状态。当检测到连接有问题或不需要继续传输时，系统会通过发送 `RST` 包来强制终止连接。`Reset` 通常出现在连接关闭异常或接收到无效数据时，比如不符合期望状态的包传入。

`Reset` 计数器对于监控网络异常尤为重要，尤其在 `ESTABLISHED` 或 `CLOSE-WAIT` 状态下发生 `RST`，可以帮助识别非正常连接关闭的情况。

| **名称**| **含义**|
|----------------|-----------------------------|
| **EstabResets** | 连接在 `ESTABLISHED` 或 `CLOSE-WAIT` 状态下被 RST 强制关闭的次数，等于以下两项之和：<br> a) 从 `ESTABLISHED` 变为 `CLOSED` 的次数。<br> b) 从 `CLOSE-WAIT` 变为 `CLOSED` 的次数。<br> 在 `tcp_set_state()` 函数中，如果之前的状态为 `TCP_CLOSE_WAIT` 或 `TCP_ESTABLISHED`，则计数增加 1。|

## 内存 Prune

在 TCP 连接中，内存 `Prune` 机制用于处理接收缓冲区（`rcv_buf`）内存不足的情况。当无法通过压缩（`collapse`）方式节省内存时，系统可能会丢弃部分乱序（`out-of-order`, `ofo`）或接收队列中的数据包，以释放内存空间。不过，这类丢包操作通常是异常情况，`OfoPruned` 和 `RcvPruned` 计数器通常应保持为 0，表示系统能够正常管理内存。

| **名称**| **含义**|
|----------------------|-----------------------------------|
| **PruneCalled**      | 由于 socket 缓冲区溢出导致接收队列中的数据包被清理的次数：在慢速路径中，如果无法将数据直接复制到用户空间且需要加入 `sk_receive_queue` 时，会检查接收端内存是否允许。若 `rcv_buf` 不足，可能需要清理乱序队列（`ofo queue`），此时计数器加 1。|
| **RcvPruned**        | 已废弃（自 2.2.0 版本起不再使用）。在缓冲区严重不足的情况下，从接收队列清除数据包的次数：在慢速路径中，若内存压力过大且清理乱序队列也不足以缓解，系统会丢弃接收到的数据包。此情况极为严重。|
| **OfoPruned**        | 由于 `socket` 缓冲区溢出，导致乱序队列（`ofo queue`）中的数据包被丢弃的次数：当 `socket` 使用过多接收内存（`rmem`）时，内核会优先丢弃已排队的乱序数据包（带 `SACK`）。若 `rcv_buf` 不足以加入新数据包，计数器加 1。|
| **TCPMemoryPressures** | 由于非致命内存分配失败（如减少发送缓冲区大小等）而导致 socket 进入“内存压力”状态的次数：在 `tcp_enter_memory_pressure()` 函数中，若从“无压力状态”切换到“有压力状态”，计数器加 1。触发情况包括：<br>- `tcp_sendmsg()`<br>- `tcp_sendpage()`<br>- `tcp_fragment()`<br>- `tso_fragment()`<br>- `tcp_mtu_probe()`<br>- `tcp_data_queue()`|

## PAWS 相关

`PAWS`（`Plausible Connection Verification`）是用于保护 TCP 连接免受时间戳攻击的一种机制。当收到的时间戳信息不符合预期时，会拒绝连接请求或数据包。以下是与 `PAWS` 相关的计数器及其含义：

| **名称**| **含义**|
|----------------------|------------------------|
| **PAWSPassive**      | 被时间戳检查拒绝的**被动**连接数：该计数器记录在三路握手的最后一个 `ACK` 包时，`PAWS` 检查失败的次数。触发点：`tcp_v4_conn_request()` |
| **PAWSActive**       | 被时间戳检查拒绝的**主动**连接数：该计数器记录在发送 `SYN` 后收到 `ACK` 包时，`PAWS` 检查失败的次数。触发点：`tcp_rcv_synsent_state_process()` |
| **PAWSEstab**        | 在已建立连接中，因时间戳检查失败被拒绝的包数：该计数器记录在已建立连接状态下，由于 `PAWS` 检查失败导致的数据包拒绝次数。 |
| **DelayedACKLocked** | 在 `ESTABLISHED` 状态下，因时间戳检查失败被拒绝的包数：该计数器记录输入包因 PAWS 检查失败而被拒绝的次数。触发点：<br> a) `tcp_validate_incoming()`<br> b) `tcp_timewait_state_process()`<br> c) `tcp_check_req()` |

## Listen 相关

`Listen` 相关计数器用于监控和统计 TCP 连接的监听队列（`accept queue`）溢出和丢包的情况。这些计数器帮助分析因资源限制或网络问题导致无法处理连接请求的情况。

| **名称**| **含义**|
|-----------------------|----------------------|
| **ListenOverflows**    | 监听队列溢出的次数：当三路握手的最后一步完成后，如果无法将 socket 添加到 `accept queue`，则会丢弃连接，计数器加 1。<br><br> **只要有数值就代表 accept queue 发生过溢出**。触发点：`tcp_v4_syn_recv_sock()` 和 `tcp_v4_conn_request()` |
| **ListenDrops**        | 丢弃 SYN 包到 LISTEN 状态的 socket 的次数：由于以下原因无法接受连接时，计数器加 1：<br> a) 无法找到指定应用（例如监听端口已经不存在）；<br> b) 创建 `socket` 失败；<br> c) 分配本地端口失败。<br><br> 该计数器也包含 `ListenOverflows` 发生的次数。触发点：`tcp_v4_syn_recv_sock()`。 |

## undo 相关

在 TCP 协议的拥塞控制中，**Undo**（撤销）机制用于处理在恢复（`Recovery`）、丢失（`Loss`）和乱序（`Disorder`）状态下由于错误的重传而导致的拥塞窗口（`CWND`）减少。具体来说，`Undo` 机制的目的是在检测到某些不必要或错误的重传后，恢复先前的拥塞窗口大小，避免过度减少带宽或性能损失。

`Undo` 机制分为不同的情况，根据 `TCP` 连接的状态和接收到的确认信息的类型，分为以下几种类型：

| **名称**| **含义**|
|-----------------|------------|
| **TCPFullUndo** | 检测到错误的重传，并撤销了 `CWND` 减小：在 `Recovery` 状态下，当接收到所有确认（`snd_una >= high_seq`）并且撤销操作完成（`undo_retrans == 0`）时，计数器加 1。触发点：`tcp_ack()` -> `tcp_fastretrans_alert()` -> `tcp_try_undo_recovery()`。|
| **TCPPartialUndo** | 检测到错误的重传，在快速重传过程中接收到部分确认，从而部分撤销了 `CWND` 减小：在 `Recovery` 状态下，当接收到部分确认（`snd_una < high_seq`）并且撤销操作完成（`undo_retrans == 0`）时，计数器加 1。触发点：`tcp_ack()` -> `tcp_fastretrans_alert()` -> `tcp_undo_partial()`。|
| **TCPDSACKUndo** | 检测到错误的重传，`D-SACK` 到达并确认了所有重传数据，从而撤销了 CWND 减小：在 Disorder 状态下，当撤销操作完成（`undo_retrans == 0`）时，计数器加 1。触发点：`tcp_ack()` -> `tcp_fastretrans_alert()` -> `tcp_try_undo_dsack()`。|
| **TCPLossUndo** | 检测到错误的重传，部分确认到达后恢复了 CWND，且未进入慢启动：在 Loss 状态下，当接收到所有确认（`snd_una >= high_seq`）并且撤销操作完成（`undo_retrans == 0`）时，计数器加 1。触发点：`tcp_ack()` -> `tcp_fastretrans_alert()` -> `tcp_try_undo_loss()`。|

## 快速路径与慢速路径

快速路径和慢速路径是 `TCP` 协议栈中两种数据包处理方式。
	•	**快速路径**：用于简单、常见的操作，能够快速处理数据包，减少延迟，通常用于纯 `ACK` 包和预测的 `ACK` 包等。
	•	**慢速路径**：用于需要更复杂处理的场景，如带有有效载荷的数据包或需要重传、流量控制等操作的包，处理时间较长。
快速路径优先处理简单的包以提高性能，而慢速路径处理更复杂的情况。

| **名称**| **含义**|
|-----------------------|----------------------|
| **TCPHPHits**     | 包头被预测的次数：如果有 `skb` 通过**快速路径**进入到 `sk_receive_queue` 上，计数器加 1；特别地，纯 `ACK` 以及直接复制到用户空间的包不算在此计数器内。触发点：`tcp_rcv_established()`。 |
| **TCPHPHitsToUser** | 通过**快速路径**直接复制到用户空间的 `skb` 数量。触发点：`tcp_rcv_established()`。 |
| **TCPPureAcks**   | 接收到不包含数据有效负载的 `ACK` 包的数量（**慢速路径**中的纯 `ACK`）。触发点：`tcp_ack()`。 |
| **TCPHPAcks**     | 预测的 `ACK` 包数量：接收到包并进入**快速路径**时，计数器加 1。触发点：`tcp_ack()`。 |

## SACK 相关

`SACK`（`Selective Acknowledgment`）是 TCP 协议的扩展，旨在提高丢包情况下的传输效率。通过 `SACK`，接收方可以告知发送方哪些数据包已经成功接收，哪些数据包丢失，从而避免不必要的重传。标准 TCP 协议只能确认接收方成功接收的最后一个连续数据段，而 `SACK` 允许接收方指出多个不连续的数据段，这样发送方只需要重传丢失的数据。

| **名称**| **含义**|
|---------------------------|-------------|
| **TCPSACKReneging**        | 当 `snd_una`（即 `skb->ack`）之后，`sk_write_queue` 中第一个 `skb` 的最小序号（SACK标记）出现 `TCPCB_SACKED_ACKED` 标志时，说明接收方丢弃了它之前已经通过 SACK 确认的数据。这个计数器增加的情况通常发生在接收方重新考虑了已经确认的数据。触发点：`tcp_clean_rtx_queue()`。 |
| **TCPSACKDiscard**         | 收到一个无效的 `SACK` 块并将其丢弃。这个计数器增加的原因通常是 `SACK` 中的序号太旧或不符合协议要求（不包括 D-SACK）。触发点：`tcp_sacktag_write_queue()`。                                                                                       |
| **TCPSackShifted**         | 在执行 `tcp_sacktag_walk()` 时，接收到的 SACK 块可能会导致一个 `skb` 被切割，切割后的新 `skb` 会被放到原 `skb` 之后。如果切割后的旧 `skb` 可以与之前的 `skb` 合并，这会触发计数器增加。此过程称为“shift”。触发点：`tcp_ack() → tcp_sacktag_write_queue() → tcp_sacktag_walk() → tcp_shift_skb_data() → tcp_shifted_data()`。|
| **TCPSackMerged**          | 在上述 `shift` 过程中，如果切割后的 `skb` 被它之前的 `skb` 完全包含（即“吃掉”），则该计数器增加。这通常发生在合并过程中，新的 `skb` 被完全整合到先前的 `skb` 中。触发点：`tcp_ack() → tcp_sacktag_write_queue() → tcp_sacktag_walk() → tcp_shift_skb_data() → tcp_shifted_data()`。 |
| **TCPSackShiftFallback**   | 如果 `shift` 操作无法完成，计数器增加。无法执行 `shift` 的原因包括：<br>1) 不支持 GSO（大段分割）；<br>2) 上一个 `skb` 不是完全分页的；<br>3) `SACK` 的序号已经被确认（ACK）。<br>触发点：`tcp_ack() → tcp_sacktag_write_queue() → tcp_sacktag_walk() → tcp_shift_skb_data()`。 |


## TCP Others

| **名称**| **含义**|
|------------------------------|------------|
| **OutOfWindowIcmps**          | 由于接收到的 ICMP 报文中的 TCP 序号不在接收窗口内而丢弃的 ICMP 包数量。可能的情况包括：<br>1) 在 `LISTEN` 状态时，序号不等于初始序号 ISN；<br>2) 在其他状态时，序号不在 `SND_UNA` 和 `SND_NXT` 之间。<br>触发点：`tcp_v4_err()`。|
| **LockDroppedIcmps**         | 由于 TCP socket 被用户锁定，导致丢弃的 ICMP 包数量。<br>触发点：`tcp_v4_err()`。|
| **ArpFilter**                 | 与 TCP 无关，接收到 ARP 包时，执行一次输出路由查找，如果路由项的设备与输入设备不同，则计数器加 1。<br>触发点：`arp_rcv()` -> `NETFILTER(ARP_IN)` -> `arp_process()`。|
| **TCPPrequeued**              | `tcp_recvmsg()` 发现并从预队列（prequeue）接收到报文时，计数器加 1。此计数器并不针对每个 `skb`，而是针对可以从预队列接收的数据。<br>触发点：`tcp_recvmsg()` -> `tcp_prequeue_process()`。|
| **TCPDirectCopyFromBacklog**  | 如果数据在软中断（softirq）处理中直接从 backlog 队列复制到用户空间内存中，则计数器加 1。<br>触发点：`tcp_recvmsg()`。|
| **TCPDirectCopyFromPrequeue** | 如果数据在当前系统调用（syscall）中直接从预队列（prequeue）复制到用户空间内存中，则计数器加 1。<br>触发点：`tcp_recvmsg()`。|
| **TCPPrequeueDropped**        | 当由于内存不足（`ucopy.memory < sk->rcv_buf`）导致预队列加入失败，数据重新由 backlog 处理时，计数器加 1。<br>触发点：`tcp_v4_rcv()` -> `tcp_prequeue()`。|
| **TCPRcvCollapsed**           | 由于接收缓冲区不足而合并了接收队列中的多个数据包时，计数器加 1。每当合并 `sk_receive_queue(ofo_queue)` 中的连续报文时，该计数器加 1。<br>触发点：<br>a) `tcp_prune_queue()` -> `tcp_collapse()` -> `tcp_collapse_one()`<br>b) `tcp_prune_ofo_queue()` -> `tcp_collapse()`。 |
| **TCPBacklogDrop**            | 由于接收队列已满，无法将接收到的数据包放入 socket，导致丢包的次数。<br>当 socket 被用户锁定时，内核会将包放入 `sk_backlog_queue`，但如果由于 `sk_rcv_buf` 不足导致入队失败，则丢弃数据包。<br>触发点：`tcp_v4_rcv()`。 |
| **TCPMinTTLDrop**             | 在接收到 TCP 报文或相关 ICMP 报文时，检查 IP 的 TTL。如果 TTL 小于 socket 配置的最小值，则丢包。此功能遵循 RFC5082（GTSM）。<br>触发点：`tcp_v4_err()` / `tcp_v4_rcv()`。|
| **TCPDeferAcceptDrop**        | 启用 `TCP_DEFER_ACCEPT` 后，丢弃的“Pure ACK”包的数量。`TCP_DEFER_ACCEPT` 允许在连接上有数据时才创建新 socket，以抵御 SYN-flood 攻击。<br>触发点：`tcp_check_req()`。|
| **IPReversePathFilter**       | 由于反向路径过滤机制丢弃的 IP 数据包数量。可能是由于反向路由查找失败，或是查找出的输出接口与输入接口不匹配。<br>触发点：`ip_rcv_finish()` -> `ip_route_input_noref()`。|
| **TCPReqQFullDoCookies**      | 当 `syn_table` 过载并触发 SYN cookie 机制时，计数器增加。这与是否启用 `sysctl_tcp_syncookies` 参数有关。<br>触发点：`tcp_rcv_state_process()` -> `tcp_v4_conn_request()` -> `tcp_syn_flood_action()`。|
| **TCPReqQFullDrop**           | 当 `syn_table` 过载并丢弃 SYN 包时，计数器增加。<br>触发点：`tcp_rcv_state_process()` -> `tcp_v4_conn_request()` -> `tcp_syn_flood_action()`。|


### 补充

| **名称**| **含义**|
|--------------------------|----------------------------------|
| **TCPRcvCoalesce**              | 当多个接收包被合并到同一个队列中时，计数器加 1。此过程有助于提高效率，减少内存操作次数。合并的包通常是在接收端网络拥塞或延迟的情况下发生。|
| **TCPOFOQueue**                 | 记录 TCP 的 **out-of-order**（OFO）数据包的数量，这些包尚未按序到达，但被暂时存储在队列中等待后续的数据包来完成顺序。|
| **TCPOFODrop**                  | 记录由于接收缓冲区已满或未能及时接收而导致的 **out-of-order**（OFO）数据包的丢弃数量。这种情况发生时，接收到的包不能合并或处理，导致丢弃。|
| **TCPOFOMerge**                 | 记录成功合并 **out-of-order** 数据包的次数。当接收的多个乱序包被重新排序并合并为一个连续的数据流时，计数器增加。|
| **TCPChallengeACK**             | 记录由于收到 **Challenge ACK** 请求而触发的计数器。**Challenge ACK** 是一种特殊的 ACK，用于测试远端是否能够接受新的 TCP 窗口或带宽。此功能可以帮助检测网络问题或延迟，防止丢失。|
| **TCPSYNChallenge**             | 记录在进行 **SYN Challenge** 操作时的次数。**SYN Challenge** 是一种防止 SYN Flood 攻击的机制，通常在连接请求过载时启动，通过发送特殊的 SYN 包来验证连接的有效性。|
| **TCPSpuriousRtxHostQueues**    | 记录在主机队列中由于误判而进行的 **TCP 重传** 数量。通常当网络拥塞或 RTT 过长时，发送端可能错误地判断包丢失，从而触发不必要的重传。|
| **BusyPollRxPackets**           | 记录使用 **忙碌轮询（Busy Polling）** 技术接收的包数量。这是一种优化技术，可以减少等待中断的时间，通过不断轮询接收队列来加速数据的接收。|
| **TCPAutoCorking**              | 记录因启用 **TCP 自动打包（Auto-Corking）** 功能而对发送的数据进行合并的次数。当内核检测到在短时间内发送多个小数据包时，它会将它们合并为一个包来提高效率并减少网络负载。|
| **TCPFromZeroWindowAdv**        | 记录从接收端发送到发送端的 **零窗口通告（Zero Window Advertisement）** 数量。零窗口通告表示接收端的缓冲区已满，发送端应暂停数据传输，直到接收端的缓冲区有足够空间。|
| **TCPToZeroWindowAdv**          | 记录发送端向接收端发送 **零窗口通告（Zero Window Advertisement）** 的数量。当发送端检测到接收端的缓冲区满时，会向接收端发送一个零窗口通告，要求它暂停发送数据。|
| **TCPWantZeroWindowAdv**        | 记录发送端希望接收到 **零窗口通告（Zero Window Advertisement）** 的次数。这通常表示发送端的数据发送速率已达到接收端缓冲区的限制，导致发送端希望通过零窗口通告暂停数据传输。|
| **TCPSynRetrans**               | 记录因 **SYN 重传** 而发生的连接请求重发次数。通常当首次发送的 SYN 包没有收到确认时，会触发 SYN 重传。|
| **TCPOrigDataSent**             | 记录原始数据发送的总字节数。这个指标计数的是不包括重传的首次有效数据包的发送量。|
| **TCPHystartTrainDetect**       | 记录 TCP 拥塞控制算法 **Hystart** 在连接开始时训练（train）阶段的检测次数。Hystart 通过在拥塞窗口增加过程中检测延迟变化来避免早期的拥塞。|
| **TCPHystartTrainCwnd**         | 记录 **Hystart** 训练阶段中拥塞窗口（cwnd）增加的次数。此计数器有助于理解在训练阶段 TCP 如何调整窗口大小。|
| **TCPHystartDelayDetect**       | 记录 **Hystart** 在检测到延迟波动时的次数。Hystart 算法的目标是在检测到网络延迟时暂停窗口增加，以避免进入拥塞状态。|
| **TCPHystartDelayCwnd**         | 记录 **Hystart** 在延迟波动检测到后调整拥塞窗口（cwnd）的次数。当延迟波动过大时，TCP 会减少拥塞窗口的增幅，以避免在不稳定的网络环境中造成拥塞。|


## IpExt

| **名称**| **含义**|
|------------------------------|----------------------------|
| **InNoRoutes**| 统计接收到的没有匹配路由的包数：当一个数据包的目的地址没有匹配的路由表项时，会触发此计数器。通常发生在目标网络不可达的情况下，导致数据包无法路由到目的地。|
| **InTruncatedPkts**| 统计接收到的被截断的包数：当接收到的包由于超过接收缓冲区的大小而被截断时，计数器会增加。截断包通常发生在接收端的缓冲区无法容纳数据时。|
| **InMcastPkts**| 统计接收到的多播包数：当数据包的目标是多播地址时，会计数该数据包为多播包。|
| **OutMcastPkts**| 统计发送的多播包数：当数据包的目标是多播地址并成功发送时，计数器增加。|
| **InBcastPkts**| 统计接收到的广播包数：当数据包的目标是广播地址时，会计数该数据包为广播包。|
| **OutBcastPkts**| 统计发送的广播包数：当数据包的目标是广播地址并成功发送时，计数器增加。|
| **InOctets**| 统计接收到的字节数：每当数据包被接收时，接收到的字节数会增加，计算总的输入流量。|
| **OutOctets**| 统计发送的字节数：每当数据包被发送时，发送的字节数会增加，计算总的输出流量。|
| **InMcastOctets**| 统计接收到的多播字节数：每当接收到一个多播包时，增加该计数器。|
| **OutMcastOctets**| 统计发送的多播字节数：每当发送一个多播包时，增加该计数器。                                                                                                                                                       |
| **InBcastOctets**| 统计接收到的广播字节数：每当接收到一个广播包时，增加该计数器。|
| **OutBcastOctets**| 统计发送的广播字节数：每当发送一个广播包时，增加该计数器。                                                                                                                                                       |
| **InCsumErrors**| 统计接收到的 checksum 错误的数据包数：当接收到的数据包的 checksum 不匹配时，计数器会增加。通常这表明数据包在传输过程中损坏。|
| **InNoECTPkts**| 统计接收到的没有 ECN 标记的包数：当接收到的 TCP 包未启用显式拥塞通知（ECN）时，增加该计数器。ECN 是一种用于通知网络拥塞的机制，未启用 ECN 的包将计入此类。                                                      |
| **InECT1Pkts**| 统计接收到的 ECN 标记为 1 的包数：当接收到的 TCP 包启用了 ECN，并且标记为 ECN1（指示轻微拥塞）时，增加该计数器。|
| **InECT0Pkts**| 统计接收到的 ECN 标记为 0 的包数：当接收到的 TCP 包启用了 ECN，并且标记为 ECN0（指示没有拥塞）时，增加该计数器。|
| **InCEPkts**| 统计接收到的带有 ECN Congestion Experienced（CE）标记的包数：当接收到的 TCP 包启用了 ECN 并且标记为 CE（指示严重拥塞）时，增加该计数器。|

## 参考资料

- [RFC 2012: SNMPv2 Management Information Base for the Transmission Control Protocol using SMIv2](https://tools.ietf.org/html/rfc2012)
- [TCP Fast Open: expediting web services](https://lwn.net/Articles/508865/)
- [TCP SNMP counters一](http://blog.chinaunix.net/uid-20043340-id-2984198.html)
- [TCP SNMP counters二](http://blog.chinaunix.net/uid-20043340-id-3016560.html)
- [TCP SNMP counters三](http://blog.chinaunix.net/uid-20043340-id-3017972.html)
- [netstat -s输出解析(一)](http://perthcharles.github.io/2015/11/09/wiki-rfc2012-snmp-proc/)
- [netstat -st输出解析(二)](http://perthcharles.github.io/2015/11/10/wiki-netstat-proc/)
- [How To Read `netstat -s` Output](http://cromwell-intl.com/networking/netstat-s.html)
- [net-tools 源码](https://sourceforge.net/p/net-tools/code/ci/v1.60/tree/statistics.c) 
- [Investigating Linux Network Issues with netstat and nstat](https://perfchron.com/2015/12/26/investigating-linux-network-issues-with-netstat-and-nstat/)
- [TCPCollector](https://github.com/BrightcoveOS/Diamond/wiki/collectors-TCPCollector)


