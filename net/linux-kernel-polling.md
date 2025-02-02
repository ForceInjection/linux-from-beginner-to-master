# Linux 6.13：30 行代码如何节省 30% 数据中心能耗？


## Cheriton School of Computer Science researchers’ update to Linux kernel could cut energy use in data centres by up to 30 per cent（Cheriton 计算机科学学院的研究人员更新了 Linux 内核，使数据中心的能耗最多可降低 30%）

> 原文地址：`https://uwaterloo.ca/computer-science/news/cheriton-school-computer-science-researchers-update-linux`

> 原文内容：Researchers at the Cheriton School of Computer Science have developed a small modification to the Linux kernel that could reduce energy consumption in data centres by as much as 30 per cent. The update has the potential to cut the environmental impact of data centres significantly, as computing accounts for as much as 5 per cent of the world’s daily energy use. Nearly all web traffic is routed through data centres, the majority of which use the open-source operating system Linux.
> Cheriton 计算机科学学院的研究人员开发出一种 Linux 内核的小修改，可以将数据中心的能耗降低多达 30%。这一更新有可能大大减少数据中心对环境的影响，因为计算占全球每日能源消耗的 5%。几乎所有网络流量都通过数据中心路由，其中​​大多数使用开源操作系统 Linux。

> “Information arrives at data centres in ‘packets,’ and then the data centre’s frontend, kind of like a receptionist at a front desk, figures out where to send those packets,” says Cheriton School of Computer Science Professor Martin Karsten.
> 切瑞顿计算机科学学院教授马丁·卡斯滕 (Martin Karsten) 表示：“信息以‘数据包’的形式到达数据中心，然后数据中心的前端，就像前台的接待员一样，确定将这些数据包发送到哪里。”

> Professor Karsten, along with his former master’s student Peter Cai, identified inefficiencies in how network traffic is processed for communications-heavy server applications. By making a small change to the Linux kernel’s networking stack, they achieved a significant improvement in both performance and energy efficiency.
> Karsten 教授与他之前的硕士生 Peter Cai 一起发现了通信量大的服务器应用程序在处理网络流量时效率低下的问题。通过对 Linux 内核的网络堆栈进行小幅改动，他们实现了性能和能效的显著提升。

> Their research, presented at ACM SIGMETRICS 2024, introduces a solution that rearranges operations within the Linux networking stack, improving the efficiency and performance of traditional kernel-based networking. According to their research, this modification increases throughput in some situations by up to 45 per cent without compromising tail latency.
> 他们的研究成果在 ACM SIGMETRICS 2024 上发表，介绍了一种解决方案，可以重新安排 Linux 网络堆栈内的操作，从而提高传统基于内核的网络的效率和性能。根据他们的研究，这种修改在某些情况下可将吞吐量提高 45%，而不会影响尾部延迟。

> “We didn’t add anything,” Professor Karsten said. “We just rearranged what is done when, which leads to a much better usage of the data centre’s CPU caches. It’s kind of like rearranging the pipeline at a manufacturing plant, so that you don’t have people running around all the time.”
> “我们没有添加任何东西，”卡尔斯滕教授说。“我们只是重新安排了什么时候做什么，这样可以更好地利用数据中心的 CPU 缓存。这有点像重新安排制造厂的流水线，这样就不需要人们一直跑来跑去。”

>It’s one thing to propose an improvement, but quite another to see it adopted by industry. To this end, Professor Karsten teamed up with Joe Damato, distinguished engineer at Fastly, to develop a small section of code — a non-intrusive kernel change of just 30 lines — that would improve Linux’s network traffic processing. When adopted, the new method could reduce the energy consumption of important data centre operations by as much as 30 per cent, Professor Karsten said.
>提出改进方案是一回事，但让其被业界采用又是另一回事。为此，卡尔斯滕教授与 Fastly 杰出工程师乔·达马托 (Joe Damato) 合作开发了一小段代码——仅 30 行的非侵入式内核更改——这将改善 Linux 的网络流量处理。卡尔斯滕教授表示，一旦采用，新方法可以将重要数据中心运营的能耗降低多达 30%。

>They tested their solution’s effectiveness and submitted it to Linux for consideration. The code has now been published as part of Linux’s newest kernel, **release version 6.13**(`https://lore.kernel.org/lkml/CAHk-=wiprabAQcCwb3qNhrT5P50MJNqunC9JU5v99kdvM-2rsg@mail.gmail.com/`).
>他们测试了该解决方案的有效性，并将其提交给 Linux 进行审核。该代码现已作为 Linux 最新内核版本 6.13的一部分发布。

>Central to their innovation is a feature called IRQ or interrupt request suspension, which balances CPU power usage with efficient data processing. By reducing unnecessary CPU interruptions during high-traffic periods, IRQ suspension enhances network performance while maintaining low latency during low-traffic conditions.
>他们的创新核心是名为IRQ或中断请求暂停的功能，该功能可在 CPU 功耗与高效数据处理之间实现平衡。通过减少高流量期间不必要的 CPU 中断，IRQ 暂停可提高网络性能，同时在低流量条件下保持低延迟。

>“All these big companies — Amazon, Google, Meta — use Linux in some capacity, but they’re very picky about how they decide to use it,” Professor Karsten said. “If they choose to ‘switch on’ our method in their data centres, it could save gigawatt-hours of energy worldwide. Almost every single service request that happens on the Internet could be positively affected by this.”
>“所有这些大公司——亚马逊、谷歌、Meta——都在某种程度上使用 Linux，但他们对如何使用它非常挑剔，”卡尔斯滕教授说。“如果他们选择在数据中心‘启用’我们的方法，那么全球可以节省数千兆瓦时的能源。互联网上发生的几乎每一个服务请求都可以受到积极影响。”

>Professor Karsten points to this research, as well as the construction of a cutting-edge green computer server room in Waterloo’s upcoming Mathematics 4 building, as examples of the kind of sustainability research that computer scientists need to prioritize. “We all have a part to play in building a greener future,” he said.
>卡尔斯滕教授指出，这项研究以及滑铁卢即将建成的数学 4 号大楼中最先进的绿色计算机服务器机房的建设，都是计算机科学家需要优先考虑的可持续性研究的例子。“我们所有人都应为建设更绿色的未来做出贡献，”他说。

>The Linux kernel **code addition**(`https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80b6f094756f`) developed by Professor Karsten and Joe Damato was based on a research paper titled “**Kernel vs. User-Level Networking: Don’t Throw Out the Stack with the Interrupts**,”(`https://dl.acm.org/doi/abs/10.1145/3626780`) written by Cheriton master’s graduate Peter Cai and Professor Karsten and published in the Proceedings of the ACM on Measurement and Analysis of Computing Systems.
>Karsten 教授和 Joe Damato 开发的Linux 内核代码附加内容基于一篇研究论文《内核与用户级网络：不要用中断扔掉堆栈》，该论文由 Cheriton 硕士毕业生 Peter Cai 和 Karsten 教授撰写，并发表在《ACM 计算机系统测量与分析论文集》上。

**30%** - 这可是巨大的成果，那就让我们来抽丝剥茧，分析一下到底是**吹牛**还是真有**干货**吧！ 

### 关键内容 
文章中核心的有两大部分内容：

1. 论文《**Kernel vs. User-Level Networking: Don’t Throw Out the Stack with the Interrupts**》；
2. 代码提交：`https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=80b6f094756f`。

那我们就针对这两部分内容做一个深入分析吧。

## 2. Kernel vs. User-Level Networking: Don’t Throw Out the Stack with the Interrupts 总结

### **研究背景**
近年来，许多研究表明，用户态网络（User-Level Networking）和内核旁路（Kernel Bypass）可以显著提升网络性能。然而，这些改进的具体来源尚未得到充分的量化分析。本文指出，**异步硬件中断（IRQ）处理的直接和间接开销是造成传统内核网络栈低效的主要原因**，并通过实验进行验证。

### **核心贡献**
#### **发现 IRQ 处理对性能的影响**
- 传统内核网络栈依赖 **异步 IRQ 处理网络数据**，但 `IRQ` 会影响 `CPU` 流水线效率，增加开销。
- 用户态网络栈通过 **轮询（polling）** 代替中断，提高处理效率。

#### **实验验证 IRQ 影响**
- 设计了 **IRQ Packing**（绑定 `IRQ` 到特定 `CPU`）和 **IRQ Suppression**（减少 `IRQ` 触发）两种实验方法，间接证明 `IRQ` 处理带来的开销。
- 通过修改 `Linux` 内核，使传统内核网络栈性能提升 **最多 45%**，且不影响尾延迟（`tail latency`）。

#### **提出 Kernel Polling 方案**
- **在 `epoll_wait()` 返回时动态控制 IRQ 屏蔽**，确保应用程序和网络协议栈同步执行，提高效率。
- 该方法相比用户态网络栈：
  - **无需专门的 CPU 绑定（CPU pinning），不影响内核调度，且更易于部署**。
  - **不需要修改应用程序代码，只需升级内核**。

论文提出的 **Kernel Polling** 方案，即在 `epoll_wait()` 处理事件时，**动态控制 IRQ 屏蔽**，避免在应用程序处理数据时收到异步中断，关键修改点：在 `epoll_wait()` 的返回路径上，检查是否有新数据可用：

- **如果有数据，则屏蔽 IRQ**，确保应用程序在处理数据时不受中断影响。
- **如果没有数据，则解除 IRQ 屏蔽**，允许系统通过中断唤醒等待的应用程序。

**伪代码**

```c
procedure ep_poll(ep) ⊲ epoll_wait() 实现
  avail ← are_events_available(ep)  // 检查是否有新数据
  loop
    if avail then
      events ← get_events(ep)
      new_napi_id ← events[0].napi_id
      if new_napi_id ≠ ep.last_napi_id then
        unmask_interrupts(ep.last_napi_id)  // 解除上次使用的 IRQ 屏蔽
      end if
      ep.last_napi_id ← new_napi_id
      mask_interrupts(ep.last_napi_id)  // 屏蔽当前 IRQ
      return events
    end if
    avail ← do_busy_poll(ep)  // 进入内核轮询模式
    if avail then
      continue
    end if
    unmask_interrupts(ep.last_napi_id)  // 没有数据时，恢复 IRQ
    sleep_until_notified(ep)  // 等待新的网络事件
  end loop
end procedure
```

### **4. 实验结果**

性能对比（**Memcached，8核心**）:

| **方案**            | **吞吐量（QPT）** | **IPC提升** | **尾部延迟（99%）** |
|-----------------|--------------|---------|----------------|
| 默认内核（Vanilla） | 724k         | 基准    | 高且波动大      |
| `IRQ` 打包          | 847k (+17%)  | +18%    | 较低但吞吐受限  |
| `IRQ` 抑制          | 967k (+33%)  | +31%    | 高波动性        |
| **内核轮询**     | **947k (+30%)** | **+26%** | **最优且稳定**  |

> 论文原文：`Kernel polling increases throughput by up to 30% in an UMA and 45% in a NUMA configuration without compromising tail latency`.


### **5. 结论**
1. **IRQ是主要瓶颈**：减少异步中断可显著提升`IPC`和吞吐量，尾部延迟更稳定。
2. **内核轮询的优势**：
   - 无需专用核心或复杂调优，灵活性优于用户级方案。
   - 兼容现有内核功能，未来可整合为生产级方案（如通过特权`Socket`选项）。
3. **研究意义**：
   - 强调实验需明确`IRQ`配置与局部性影响。
   - 用户级网络的性能优势部分源于简化协议栈（如`Caladan`未实现拥塞控制），可能牺牲功能。

## 3. Commit 分析
### Commit 基本信息  

- **提交标题**: `Suspend IRQs during application busy periods`  
- **代码行数**: 核心逻辑仅 30 行（不含驱动适配代码）
- **作者**: `Martin Karsten` 和 `Joe Damato`
-  **sign-off**: `Jakub Kicinski`（`Linux` 网络子系统核心维护者）  

### 核心内容

该 `commit` 主要引入了一种新的中断屏蔽（`IRQ suspension`）机制，以优化高流量网络应用的 `CPU` 负载与网络处理效率之间的平衡。以下是该提交的主要内容及其影响：

#### 引入 `IRQ Suspension` 机制

* 该机制允许网络应用在 `epoll` 轮询期间屏蔽中断，以减少高负载情况下的中断开销，同时在低负载情况下降低尾部延迟（`tail latency`）;
* 通过新增 `irq_suspend_timeout` 参数，应用可以在调用 `epoll_wait` 时屏蔽 `NAPI` 相关的 `IRQ` 和 `softirq` 处理，直到 `epoll_wait` 没有从 `NAPI` 读取到数据。
* `irq_suspend_timeout` 作为一个安全机制，确保当用户态处理数据时间过长时，可以超时恢复正常的 `NAPI` 处理中断。

#### 与现有中断管理机制的比较

* 目前 `Linux` 内核已有多种中断优化机制：
* 通过 `napi_defer_hard_irqs` 和 `gro_flush_timeout` 进行软中断（`softirq`）延迟处理。
* 通过 `NIC` 硬件中断合并（`interrupt coalescing`）减少中断频率。
* 采用 `NAPI` 轮询模式（`busy polling`），可以提高吞吐，但会导致 **100% CPU** 占用。
* `irq_suspend_timeout` 通过智能切换 `busy poll` 和 `irq deferral`，在高负载下减少干扰，在低负载下保持低延迟。

#### 具体实现方式

* 在 `napi_struct` 结构中新增 `irq_suspend_timeout` 变量，并提供 `napi_suspend_irqs() `和 `napi_resume_irqs()` 以便在合适的时机屏蔽或恢复 `IRQ`。
* 在 `epoll_wait` 执行过程中：
* 若 `epoll_wait` 读取到数据，则调用 `napi_suspend_irqs()` 继续屏蔽中断。
* 若 `epoll_wait` 没有数据，则调用 `napi_resume_irqs()` 解除 `IRQ` 屏蔽，并恢复 `napi_defer_hard_irqs` 机制。
* 该参数可通过 `netdev-genl`（`netlink`）接口进行设置。

#### 性能测试

* 测试环境：单路 `AMD EPYC 7662 64` 核处理器 + `100Gbps Mellanox ConnectX-5 NIC`。
* 采用 `memcached` + `mutilate` 进行基准测试，测试了不同 `irq_suspend_timeout` 配置的影响。
	
### 测试结果

* `irq_suspend_timeout` 机制能在高负载下减少中断开销，提高吞吐。
* 在低负载下，其表现接近 `irq deferral` 机制，且优于普通 `busy polling`。

**对比**:

- `Throughput (MAX)` and latencies of `base` vs `suspend`.
- `CPU usage` of napibusy and fullbusy during lower load (200K, 400K for
  example) vs suspend.
- Latency of the defer variants vs suspend as timeout and load
  increases.
- `suspend0`, which sets defer_hard_irqs and gro_flush_timeout to 0, has
  nearly the same performance as the base case (this is FAQ item #1).
  
> 根据作者以往工作的经验（做过实验，也分析过 SNIA - https://www.snia.org/ 的服务器能耗数据），**服务器的能耗**主要和 **`CPU` 利用率**成**正比**，与网络以及硬盘（当然如果有磁盘阵列的情况下，可能要另外再做实验了）利用率关系不大。 而 `commit` 的测试结果中，在处于**正常负载**（200k/400k）的时候，`suspend` 的 `CPU` 利用率要比 `napibusy` 和 `fullbusy` 普遍**低 30%** 以上。
> 
> 因此这里节省 **30%** 能源大概来源于此！

### 应用场景

* 适用于 高吞吐、低延迟的网络应用，如高性能数据服务（`memcached`、`redis`）或高并发网络服务器。
* 该机制可以减少 `CPU` 在网络处理上的开销，提高整体系统的响应能力。
* 但需要仔细调整 `irq_suspend_timeout` 和 `gro_flush_timeout`，以避免影响低负载情况下的实时性。

#### 相关文件修改

| **文件** | **增加行数** | **删除行数** | **说明** |
|------|--------|--------|----|
| `fs/eventpoll.c` | **36** | **2** | `epoll_wait` 逻辑改动，增加 `IRQ` 屏蔽/恢复 |
| `net/core/dev.c` | **39** | 0 | `napi_suspend_irqs()` & `napi_resume_irqs()` 实现 |
| `include/linux/netdevice.h` | **2** | 0 | `napi_struct` 结构体新增 `irq_suspend_timeout` |
| `include/net/busy_poll.h` | **3** | 0 | `NAPI` 相关` API` 增加 |
| `net/core/dev.h` | **25** | 0 | `irq_suspend_timeout` `getter/setter` 实现 |
| `net/core/netdev-genl.c` | **12** | 0 | Netlink 交互，允许用户态配置 `irq_suspend_timeout` |

核心的 `Kernel Polling` 方案 只需修改大约 `30` 行代码，主要涉及：

* `epoll_wait()` 逻辑（修改 `fs/eventpoll.c`）
* `NAPI` 处理逻辑（修改 `net/core/dev.c`）
* 添加 `IRQ` 屏蔽控制标志位

## 总结 
这一提交通过极简的代码改动（**30 行核心逻辑**），证明了内核优化的巨大潜力。在规模化部署的场景下，其为数据中心节约 **30%** 能源的目标是合理且可实现的。此类优化若扩展到更多驱动和硬件平台（如 `NVIDIA BlueField DPU`），节能效果将进一步放大。  