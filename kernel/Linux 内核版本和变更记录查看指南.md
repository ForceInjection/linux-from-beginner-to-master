# Linux 内核版本变更记录查看指南

本指南面向希望理解 `Linux` 内核更新机制、定位安全补丁、追踪补丁源码来源的技术人员，涵盖 `changelog` 获取、`CVE` 快速筛选、源码版本控制操作以及真实案例分析，帮助你从「内核升级日志」走向「代码级掌握」。

---

## 1. 查看当前内核版本与构建信息

在排查 `Linux` 系统的性能或兼容性问题时，首先需要确认当前运行的内核版本及其构建信息。

以下命令可以提供常用信息：

```bash
uname -r              # 查看当前运行的内核版本
3.10.0-1160.119.1.el7.x86_64
cat /proc/version     # 查看构建信息（如使用的编译器、构建时间等）
Linux version 3.10.0-1160.119.1.el7.x86_64 (mockbuild@kbuilder.bsys.centos.org) (gcc version 4.8.5 20150623 (Red Hat 4.8.5-44) (GCC) ) #1 SMP Tue Jun 4 14:43:51 UTC 2024

# 列出所有已安装的内核包（包括未运行版本，例如我们已经更新了内核，但是没有生效）
# Debian/Ubuntu
dpkg -l | grep linux-image
ii  linux-image-6.8.0-45-generic         6.8.0-45.45~22.04.1                     amd64        Signed kernel image generic

# RHEL/CentOS/Rocky
rpm -qa | grep kernel
kernel-tools-3.10.0-1160.119.1.el7.x86_64
kernel-devel-3.10.0-1160.119.1.el7.x86_64
kernel-headers-3.10.0-1160.119.1.el7.x86_64
kernel-3.10.0-1160.119.1.el7.x86_64
kernel-doc-3.10.0-1160.119.1.el7.noarch
kernel-debug-3.10.0-1160.119.1.el7.x86_64
kernel-tools-libs-3.10.0-1160.119.1.el7.x86_64

```

这些信息可以帮助我们判断系统是否已应用特定补丁、是否与某些工具版本匹配，或是否存在已知的兼容性问题。

### 1.1 版本号结构解析

#### 1.1.1 示例一：`5.4.0-153-generic`（Ubuntu 系）

```text
5.4.0-153-generic
↑ ↑   ↑     ↑
│ │   │     └─ 架构/构建标识（如 Ubuntu 的构建标识 generic、lowlatency 等）
│ │   └────── 发行版维护版本号（由发行版定义）
│ └────────── 次版本号（minor version，即 4）
└──────────── 主版本号（major version，即 5）
```

* `5.4.0`：对应主线内核的版本号，其中 `0` 表示主线稳定版本的修订号（如 `5.4.1` 中的 `1`）。
* `153`：发行版维护版本号，表示该发行版在 `5.4.0` 基础上叠加的第 153 次更新（包含安全补丁、驱动适配等）。
* `generic`：内核构建配置标签，常见如 `generic`（通用内核）、`lowlatency`（低延迟内核）等，是 Ubuntu 和 Debian 系列的标准标识。

---

#### 1.1.2 示例二：`3.10.0-1160.119.1.el7.x86_64`（CentOS / RHEL 系）

```text
3.10.0-1160.119.1.el7.x86_64
↑ ↑   ↑     ↑     ↑     ↑
│ │   │     │     │     └── 架构标识：x86_64
│ │   │     │     └──────── 目标发行版标识：el7（Enterprise Linux 7）
│ │   │     └────────────── 微调版本号（patch level 3）
│ │   └──────────────────── 次级补丁号（patch level 2）
│ └──────────────────────── 主维护序号（patch level 1）
└────────────────────────── 主版本号与次版本号（来自主线版本 3.10.0）
```

* `3.10.0`：内核主线版本号，Red Hat 长期支持（LTS）版本之一。
* `1160.119.1`：**Red Hat 系列特有的维护版本结构**，拆解如下：

| 字段     | 含义说明                                         |
| ------ | -------------------------------------------- |
| `1160` | Red Hat 在 `3.10.0` 基础上的第 1160 个主要内部维护版本（主构建） |
| `119`  | `1160` 分支下的第 119 次补丁维护（安全修复、驱动更新等）           |
| `1`    | `1160.119` 下的微修订，通常是小型变更或构建修复                |

* `el7`：表明该内核面向 RHEL/CentOS 7 系列；
* `x86_64`：目标平台为 64 位架构。

---

## 2. 查看内核变更记录

> **注意**：发行版维护的内核版本（如 Ubuntu 的 `5.4.0-153`）通常基于 kernel.org 主线某版本（如 `5.4`）的一个提交，再由发行版维护者引入自定义补丁（如安全修复、功能增强、兼容性调整等）构建而成。因此，其变更记录并不完全等同于主线内核的 `5.4.x` 系列。
>
> **排查系统内核相关问题时，应优先参考当前系统中安装的发行版内核包的变更日志，而非主线内核的 changelog。**

---

### 2.1 主线内核与发行版内核的区别与联系

在分析内核补丁来源或定位内核行为差异时，理解 **主线内核（mainline kernel）** 与 **发行版内核（distribution kernel）** 的区别至关重要。两者在版本结构、更新节奏、补丁策略上存在显著差异。

#### 2.1.1 主线内核（Mainline Kernel）

主线内核由 `Linus Torvalds` 及社区在 [kernel.org](https://www.kernel.org) 上维护，采用 Git 管理，遵循如下版本发布策略：

| 类型 | 示例版本 | 特点 |
| ------ | ---------------- | ------------------------- |
| 主线版本   | `v6.6`           | 包含最新功能，更新频繁，适合开发与测试场景     |
| 稳定版本   | `v6.6.3`         | 在主线基础上修复 Bug 与安全问题，适合通用用途 |
| LTS 版本 | `v5.4`, `v6.1` 等 | 社区长期维护（2–6 年），广泛用于生产系统    |

主线内核提供最原始、通用的功能集，可在其 Git 仓库中查看详细提交历史与变更记录。

#### 2.1.2 发行版内核（Distribution Kernel）

各大 Linux 发行版（如 Ubuntu、Debian、Red Hat、SUSE 等）基于某一主线版本制作定制内核，通常包含以下特性：

| 特性 | 说明 |
| ------- | ----------------------------------------------------------------- |
| 基于主线某版本 | 如 Ubuntu 20.04 的 `5.4.0-153` 基于主线 `v5.4` 系列 |
| 定制补丁    | 包括驱动增强、架构支持、系统策略（如 AppArmor、SELinux）等 |
| 安全更新    | 通过 Backport 将主线修复补丁移植到旧版本内核 |
| 命名规则不同  | 发行版内核版本常见形式为 `<主线版本>-<patch level>.<build num>`，如 `5.4.0-153.170` |

因此在分析补丁或问题时应注意：

* **主线 changelog 可用于溯源某特性或 CVE 首次修复版本**；
* **发行版 changelog 才能准确判断某系统是否合入了特定补丁**；
* 某些 CVE 在主线修复后，可能 **延迟数周至数月才被 backport 到发行版内核中**，需要结合发行版 changelog 或安全公告确认。

#### 2.1.3 可视化示意

```text
       主线版本（如 v5.4）              主线稳定补丁（如 v5.4.235）
           |                                  |
           v                                  v
    -----------------                -----------------
    | Linux 主线源码 |  <---------   | stable patch  |
    -----------------                -----------------
            |                                |
            v                                v
    [ 发行版内核源码树（如 Ubuntu 5.4.0-153） ]
                   ↑        ↑
         补丁合入 / CVE 修复 / 驱动增强
```

#### 2.1.4 Backport 补丁回迁

发行版维护者会从主线或 stable 分支中提取重要补丁（尤其是安全相关），并**手动适配移植到旧版本代码中**。这一过程被称为 **Backport**，通常涉及代码逻辑调整、上下文匹配等工作。

> 例如，Ubuntu 的 `5.4.0-153` 版本可能包含来自主线 `5.15` 的某个安全修复补丁，但补丁内容需重写以适配 `5.4` 的代码结构。

---

### 2.2 RPM 系发行版（Red Hat / CentOS / Rocky 等）

对于基于 RPM 包管理器的系统，使用以下命令可查看内核变更日志：

```bash
rpm -q --changelog kernel-$(uname -r) | less  # 查看当前运行内核的 changelog
rpm -q --changelog kernel | less              # 查看所有已安装内核包的日志
```

输出内容包括版本历史、安全修复、功能补丁说明等，常用于核对特定补丁是否已合入当前系统。

示例输出：

```text
* Thu May 09 2024 Radomir Vrbovsky <rvrbovsk@redhat.com> [3.10.0-1160.119.1.el7]
- PCI: hv: Reinstate wrongfully dropped hv_pcibus_removing state (Vitaly Kuznetsov) [RHEL-22919]

* Thu Apr 04 2024 Rado Vrbovsky <rvrbovsk@redhat.com> [3.10.0-1160.118.1.el7]
- iommu/amd: Fix NULL dereference bug in match_hid_uid (Jerry Snitselaar) [RHEL-8721]

* Mon Apr 01 2024 Rado Vrbovsky <rvrbovsk@redhat.com> [3.10.0-1160.117.1.el7]
- tracing/perf: Fix double put of trace event when init fails (Michael Petlan) [RHEL-18052]
- tracing: Fix race in perf_trace_buf initialization (Michael Petlan) [RHEL-18052]
- net/sched: sch_hfsc: upgrade 'rt' to 'sc' when it becomes a inner curve (Davide Caratti) [RHEL-16458] {CVE-2023-4623}
- net/sched: sch_hfsc: Ensure inner classes have fsc curve (Davide Caratti) [RHEL-16458] {CVE-2023-4623}
- gfs2: Fix invalid metadata access in punch_hole (Andrew Price) [RHEL-28785]
- vt: vt_ioctl: fix race in VT_RESIZEX (Jay Shin) [RHEL-28639] {CVE-2020-36558}
- selinux: cleanup and consolidate the XFRM alloc/clone/delete/free code (Ondrej Mosnacek) [RHEL-27751]
- bluetooth: Add cmd validity checks at the start of hci_sock_ioctl() (David Marlin) [RHEL-3682] {CVE-2023-2002}
- bluetooth: Perform careful capability checks in hci_sock_ioctl() (David Marlin) [RHEL-3682] {CVE-2023-2002}
- cifs: fix panic in smb2_reconnect (Jay Shin) [RHEL-26301]
- af_unix: Fix null-ptr-deref in unix_stream_sendpage(). (Guillaume Nault) [RHEL-16144] {CVE-2023-4622}
- NFS: Set the stable writes BDI capability (Benjamin Coddington) [RHEL-22193]
- RDMA/i40iw: Prevent zero-length STAG registration (Kamal Heib) [RHEL-6299] {CVE-2023-25775}
- sched/membarrier: reduce the ability to hammer on sys_membarrier (Wander Lairson Costa) [RHEL-26402] {CVE-2024-26602}

* Thu Feb 29 2024 Rado Vrbovsky <rvrbovsk@redhat.com> [3.10.0-1160.116.1.el7]
- bluetooth: Add cmd validity checks at the start of hci_sock_ioctl() (David Marlin) [RHEL-3682] {CVE-2023-2002}
- bluetooth: Perform careful capability checks in hci_sock_ioctl() (David Marlin) [RHEL-3682] {CVE-2023-2002}
- cifs: fix panic in smb2_reconnect (Jay Shin) [RHEL-26301]

* Thu Feb 22 2024 Rado Vrbovsky <rvrbovsk@redhat.com> [3.10.0-1160.115.1.el7]
- af_unix: Fix null-ptr-deref in unix_stream_sendpage(). (Guillaume Nault) [RHEL-16144] {CVE-2023-4622}
- NFS: Set the stable writes BDI capability (Benjamin Coddington) [RHEL-22193]

* Thu Feb 15 2024 Rado Vrbovsky <rvrbovsk@redhat.com> [3.10.0-1160.114.1.el7]
- netfilter: nf_tables: reject QUEUE/DROP verdict parameters (Florian Westphal) [RHEL-23500] {CVE-2024-1086}
...
```

---

### 2.3 APT 系发行版（Ubuntu / Debian 等）

APT 系列提供更友好的 changelog 工具，推荐如下方式查看内核更新日志：

#### 2.3.1 在线获取 changelog

```bash
sudo apt-get install apt-listchanges                # （首次使用时需安装）
apt-get changelog linux-image-$(uname -r)           # 获取当前内核包的更新日志
```

该命令会从 Debian 或 Ubuntu 包仓库中下载内核包 changelog 文件，并显示每次变更的版本号、作者、变更说明等。

示例输出：

```text
linux-signed-hwe-6.8 (6.8.0-45.45~22.04.1) jammy; urgency=medium

  * Main version: 6.8.0-45.45~22.04.1

  * Packaging resync (LP: #1786013)
    - [Packaging] debian/tracking-bug -- resync from main package

 -- Stefan Bader <stefan.bader@canonical.com>  Wed, 11 Sep 2024 15:59:36 +0200

linux-signed-hwe-6.8 (6.8.0-44.44~22.04.1) jammy; urgency=medium

  * Main version: 6.8.0-44.44~22.04.1

  * Packaging resync (LP: #1786013)
    - [Packaging] debian/tracking-bug -- resync from main package

 -- Stefan Bader <stefan.bader@canonical.com>  Thu, 22 Aug 2024 15:26:38 +0200

linux-signed-hwe-6.8 (6.8.0-41.41~22.04.1) jammy; urgency=medium

  * Main version: 6.8.0-41.41~22.04.1

  * Packaging resync (LP: #1786013)
    - [Packaging] debian/tracking-bug -- resync from main package

 -- Stefan Bader <stefan.bader@canonical.com>  Fri, 09 Aug 2024 10:13:51 +0200

linux-signed-hwe-6.8 (6.8.0-40.40~22.04.3) jammy; urgency=medium

  * Main version: 6.8.0-40.40~22.04.3

  * Packaging resync (LP: #1786013)
    - [Packaging] debian/tracking-bug -- resync from main package

 -- Stefan Bader <stefan.bader@canonical.com>  Tue, 30 Jul 2024 17:17:21 +0200
 ...
 ```

#### 2.3.2 查看本地缓存 changelog

部分系统已缓存历史版本日志，可使用以下命令查看：

```bash
zcat /usr/share/doc/linux-image-*/changelog.Debian.gz | less
```

该方式可查看已安装的各版本内核的详细本地日志记录。

---

### 2.4 Ubuntu 内核源码及变更追踪链接

Ubuntu 的内核源码与变更日志可从以下位置获取：

* Ubuntu 内核源码仓库（Git）：[https://git.launchpad.net/ubuntu/+source/linux](https://git.launchpad.net/ubuntu/+source/linux)
*  Ubuntu 的官方内核仓库：[https://kernel.ubuntu.com/mainline/](https://kernel.ubuntu.com/mainline/)
* Ubuntu 安全通报与 CVE 修复追踪：[https://ubuntu.com/security/notices](https://ubuntu.com/security/notices)

在查找特定补丁是否已合入 `Ubuntu` 内核时，可通过上面仓库搜索 `commit message`、`changelog` 或 `CVE` 编号。

---

## 3. changelog 来源与安全更新筛选

在企业环境或系统故障排查中，**确认某个内核版本是否修复了特定漏洞或引入了问题**，往往是一个关键步骤。Linux 发行版的 **changelog（变更日志）** 可用于快速定位：

* 修复记录与补丁信息（如 CVE 修复）；
* 内核组件功能更新与回退情况；
* 与特定 `bug`/`issue` 编号相关的修复。

---

### 3.1 changelog 的来源与格式说明

不同 `Linux` 发行版构建内核的流程和变更记录格式存在差异，主要体现在内核包的 `changelog` 来源及组织方式上。

| 发行版类型 | changelog 来源               | 格式说明                                             |
| ----- | -------------------------- | ------------------------------------------------ |
| RPM 系 | `.spec` 文件的 `%changelog` 段 | 手动维护，构建者在打包时添加，通常包含修复摘要、CVE 编号、安全标签等             |
| APT 系 | `debian/changelog` 文件      | 遵循 Debian Policy，由维护者通过 `dch` 命令维护，逐条记录每次打包的变更内容 |

这些变更日志会在打包构建过程中嵌入到最终发布的 `.rpm` 或 `.deb` 文件中，因此我们可以通过发行版提供的命令工具对其进行查看（见第 2 节）。

此外，还可以直接查看 Linux 官方源码的 changelog 文件，例如：

* **Linux 主线版本的官方 ChangeLog**：[https://cdn.kernel.org/pub/linux/kernel/v5.x/ChangeLog-5.0.1](https://cdn.kernel.org/pub/linux/kernel/v5.x/ChangeLog-5.0.1)

* **Ubuntu 官方内核源码与 changelog 仓库**：[https://git.launchpad.net/ubuntu/+source/linux](https://git.launchpad.net/ubuntu/+source/linux)

---

### 3.1.1 Ubuntu changelog 查询方式补充

Ubuntu 用户可通过以下三种方式查看某个内核版本的变更日志，用于确认漏洞修复、安全更新或内核行为变动：

#### 方法一：使用 `apt-get changelog` 命令（推荐）

适用于系统已安装该内核版本的情况，直接查看对应的内核包 `changelog`：

```bash
apt-get changelog linux-image-$(uname -r)
```

> 也可替换为其他特定版本，如：
> `apt-get changelog linux-image-5.15.0-105-generic`

---

#### 方法二：访问 changelogs.ubuntu.com 网站

适用于查阅任意 Ubuntu 发布版本的 `linux` 包 changelog，无需本地安装：

* 网站入口：[https://changelogs.ubuntu.com/changelogs/pool/main/l/linux/](https://changelogs.ubuntu.com/changelogs/pool/main/l/linux/)

* 示例地址（查看 `linux_5.15.0-105.115`）：[https://changelogs.ubuntu.com/changelogs/pool/main/l/linux/linux\_5.15.0-105.115/changelog](https://changelogs.ubuntu.com/changelogs/pool/main/l/linux/linux_5.15.0-105.115/changelog)

---

#### 方法三：通过 Launchpad 查看内核源码包信息

适用于深入了解内核构建历史、补丁内容、关联 bug 信息等：

* Ubuntu 源码概览：[https://launchpad.net/ubuntu/+source/linux](https://launchpad.net/ubuntu/+source/linux)

* 指定版本页面（如 `5.15.0-105.115`）：[https://launchpad.net/ubuntu/+source/linux/5.15.0-105.115](https://launchpad.net/ubuntu/+source/linux/5.15.0-105.115)

* 查看所有版本 changelog：[https://launchpad.net/ubuntu/+source/linux/+changelog](https://launchpad.net/ubuntu/+source/linux/+changelog)

---

### 3.2 快速筛选 CVE 和安全更新信息

在实际分析中，我们往往需要从大量 changelog 内容中快速定位与 **安全相关的更新（CVE、安全修复）**，可结合关键字和过滤命令操作：

#### 3.2.1 RPM 系（如 RHEL、CentOS、AlmaLinux）

```bash
# 显示与安全相关的 changelog 条目（包含 CVE 或 [security] 字样）
rpm -q --changelog kernel | grep -E -C3 "CVE-|\\[security\\]"
```

#### 3.2.2 APT 系（如 Ubuntu、Debian）

```bash
# 查看当前运行内核版本的 changelog，并筛选出含有 security/CVE 的内容
apt-get changelog linux-image-$(uname -r) | grep -i "security\\|CVE-"
```

---

### 3.3 示例：Ubuntu 内核包变更日志节选

以下是 `Ubuntu 20.04` 中内核包 `linux-image-5.4.0-153-generic` 的 changelog 节选：

```text
linux (5.4.0-153.170) focal; urgency=medium

  * focal/linux: 5.4.0-153.170 -proposed tracker (LP: #2024108)

  * cls_flower: off-by-one in fl_set_geneve_opt (LP: #2023577)
    - net/sched: flower: fix possible OOB write in fl_set_geneve_opt()

  * Some INVLPG implementations can leave Global translations unflushed when
    PCIDs are enabled (LP: #2023220)
    - x86/mm: Avoid incomplete Global INVLPG flushes

 -- Thadeu Lima de Souza Cascardo <email address hidden>  Fri, 16 Jun 2023 10:20:20 -0300
```

> 详细参见：
> [https://launchpad.net/ubuntu/%2Bsource/linux/5.4.0-153.170](https://launchpad.net/ubuntu/%2Bsource/linux/5.4.0-153.170)

#### 说明：

* `linux (5.4.0-153.170)`：表示该内核包的完整版本号，`.170` 是 Ubuntu 的内部构建版本；
* `urgency=medium`：指明更新紧急程度（`Debian changelog` 标准字段）；
* `LP: #2024108`、`LP: #2023577`、`LP: #2023220`：关联的 Launchpad Bug 编号，可通过 [https://bugs.launchpad.net/](https://bugs.launchpad.net/) 查询详细信息。

---

### 3.4 CentOS / RHEL 系发行版的 changelog 与安全信息查询

对于使用 CentOS、RHEL 或基于 Red Hat 系列的衍生发行版（如 AlmaLinux、Rocky Linux），可通过如下方式获取内核更新日志与安全修复信息：

---

#### 3.4.1 Red Hat 官方变更记录（Errata 与内核更新公告）

Red Hat 提供了完整的安全公告（RHSA）、错误修复公告（RHBA）和功能增强公告（RHEA），涵盖各类包的变更信息。

* 官方查询页面（需要登录）：[https://access.redhat.com/errata](https://access.redhat.com/errata)

> 说明：RHSA 编号中通常会包含修复的 CVE 编号和影响版本，适合用来判断补丁是否已下发。

---

#### 3.4.2 Red Hat CVE 安全数据库

Red Hat 提供专门的 CVE 查询页面，可根据漏洞编号查看影响组件、补丁状态、修复包版本等：

* CVE 安全更新数据库：[https://access.redhat.com/security/security-updates/#/cve](https://access.redhat.com/security/security-updates/#/cve)

* 示例查询：[https://access.redhat.com/security/cve/CVE-2023-6543](https://access.redhat.com/security/cve/CVE-2023-6543)

---

#### 3.4.3 CentOS 源码包 changelog 查看

虽然 CentOS 没有独立的 changelog 网页接口，但我们可以通过 **获取 `.src.rpm` 源码包并查看 `.spec` 文件** 中的 `%changelog` 段落来获取构建时记录：

* 镜像源地址（CentOS 9 Stream 为例）：[https://mirror.stream.centos.org/9-stream/BaseOS/source/tree/Packages/](https://mirror.stream.centos.org/9-stream/BaseOS/source/tree/Packages/)

查看方法：

```bash
# 下载并解包 src.rpm 文件
rpm2cpio kernel-4.18.0-513.el8.src.rpm | cpio -idmv

# 查看 changelog
less kernel.spec
```

> `%changelog` 中也会标注修复摘要、CVE 编号和提交者信息，与 RHEL 对应版本基本一致。

---

#### 3.4.4 CentOS/RHEL 内核 Git 镜像仓库

Red Hat 内核源码构建过程为闭源，但 CentOS Stream 提供了开放的 Git 仓库，可用于追踪源码更新、补丁应用历史：

* 官方 Git 仓库：[https://git.centos.org/rpms/kernel](https://git.centos.org/rpms/kernel)

---

#### 3.4.5 OracleLinux.pkgs.org：快速查阅内核包 changelog

虽然该站点主要用于 Oracle Linux，但其内核包版本和 RHEL 系兼容，许多版本一致或高度相近，适用于辅助查看 changelog 与构建信息。

* 查询页面：[https://oraclelinux.pkgs.org](https://oraclelinux.pkgs.org)

* 示例：查看 kernel-4.18.0-513.11.0.1.el8_9.x86_64 包信息与 changelog：
  [https://oraclelinux.pkgs.org/8/ol8-baseos-latest-x86_64/kernel-4.18.0-513.11.0.1.el8_9.x86_64.rpm.html](https://oraclelinux.pkgs.org/8/ol8-baseos-latest-x86_64/kernel-4.18.0-513.11.0.1.el8_9.x86_64.rpm.html)

> 说明：该页面包含完整的 changelog 条目、构建时间、依赖信息等，适用于无法访问 Red Hat 官网的用户使用场景。

---


## 4. 源码级追踪（Git 操作）

在某些场景下，例如 `changelog` 无法提供足够信息、需确认某个补丁的具体实现细节，或需要分析问题的引入与修复过程，我们可以通过 **源码级追踪** 方式深入分析 Linux 内核的演变过程。

Linux 内核使用 `Git` 进行源码管理，官方所有提交历史和版本标签（如 `v6.6`）都可公开访问，便于我们查找补丁来源、分析变更背景与上下文。

通过 Git 操作可以实现以下目标：

* **追踪特定补丁的引入与上下文**
* **查找特定子系统（如调度器、内存、网络）的代码变更**
* **精确分析某次提交的差异（diff）和修复逻辑**

---

### 4.1 克隆源码仓库与版本切换

建议使用官方仓库 `https://git.kernel.org` 提供的 `linux-stable` 或主线 `torvalds/linux` 仓库：

```bash
# 克隆指定版本的浅仓库（节省空间）
git clone --depth=1 --branch v6.6 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux-stable

# 或克隆完整历史仓库后切换版本
git clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux-stable
git checkout v6.6
```

> 💡 `v6.6` 是稳定版标签，适用于分析正式发布的版本。该标签与实际发行包（如 Ubuntu 的 `linux-image-6.6.x`）通常一一对应。
>
> * `linux-stable`: 包含所有正式发布的稳定版本，适合生产环境使用。
> * `torvalds/linux`: 包含主线开发分支，含实验性变更，适合分析前沿补丁和开发进展。

---

### 4.2 提交历史分析与关键补丁筛选

`Git` 提供强大的日志过滤与差异比较能力，可根据子系统关键词、时间段、作者、提交信息等快速定位感兴趣的补丁。

#### 示例：查找调度器子系统相关改动（v6.1 至 v6.6）

```bash
git log v6.1..v6.6 --oneline --grep='\[sched\]'
```

输出示例（节选）：

```
adfbc9d7a5 sched/fair: Fix CFS bandwidth timer race
29a0d1921e sched: Remove redundant check in task_tick_fair()
```

> `--grep='\[sched\]'` 能高效筛选出与调度器相关的提交。可根据不同子系统使用如 `[mm]`, `[net]`, `[fs]` 等模式。

#### 查看某次提交的详情（含 diff）

```bash
git show adfbc9d7a5
```

输出将包含：

* 提交标题、作者、时间；
* 涉及文件及变更行；
* 提交说明（如 bug 修复说明、Fixes、Reported-by 信息）；
* 精确的代码修改内容（以 `+`/`-` 标识）。

---

### 4.3 常用 Git 命令与高级用法

| 命令                             | 功能说明                   |
| ------------------------------ | ---------------------- |
| `git blame <file>`             | 查看某行代码由哪个提交引入          |
| `git diff v6.5 v6.6`           | 比较两个版本之间的所有差异          |
| `git log --since="2023-10-01"` | 查看某日期之后的所有提交记录         |
| `git log --author="Linus"`     | 筛选指定开发者提交（可结合子系统关键词使用） |

---

### 4.4 小结与推荐实践

| 目标 | 推荐操作示例 |
| --------------- | ------------------------------------ |
| 确认补丁是否修复指定问题    | `git log --grep="bug keyword"`       |
| 验证某补丁是否已合入目标版本  | `git log v6.1..v6.6` 中搜索补丁摘要或 hash 值 |
| 分析提交的原始上下文与变更逻辑 | `git show <commit>`                  |
| 查找一行代码的历史修改记录   | `git blame <file>`                   |

---

## 5. 实战示例

以下示例展示如何结合 changelog、Git 和社区邮件列表等工具，系统分析 CVE 补丁的修复路径及来源，掌握追踪漏洞与补丁演进的关键方法。

---

### 5.1 示例一：结合 changelog 与源码分析 CVE 修复路径

**目标：** 以 `CVE-2023-32233` 为例，分析该漏洞的修复流程，确认补丁在哪个版本中引入，并查看内核源码的具体改动。

> 完整 `CVE` 信息链接：[CVE-2023-32233](https://ubuntu.com/security/CVE-2023-32233)

---

#### 步骤一：通过安全公告与 changelog 定位修复版本

**方法一：查询 Ubuntu 官方安全公告**

访问 `Ubuntu` 官方安全公告页面，查找 `CVE-2023-32233` 的修复信息：

* Ubuntu 20.04 LTS（Focal）：已在内核版本 `5.4.0-150.167` 中修复。
* Ubuntu 22.04 LTS（Jammy）：已在内核版本 `5.15.0-73.80` 中修复。

**方法二：查询本地软件包 changelog**

如果系统已安装包含修复的内核版本，可使用以下命令查看 `changelog`：

```bash
apt-get changelog linux-image-$(uname -r) | grep -i "CVE-2023-32233"
```

> ✅ **提示：**

`apt-get changelog` 展示的是 `Ubuntu` 内核维护者维护的 `debian/changelog` 内容，主要反映**打包层面的变更**（如版本同步、构建规则调整等），而非具体的源码修复详情。在多数情况下：

* 不会标明具体修复了哪些 CVE；
* 不包含补丁的代码变更；
* 甚至可能仅显示类似于 `Packaging resync` 的简略说明。

因此，**仅依赖 changelog 难以准确确认补丁细节和漏洞影响范围**。推荐始终结合 **Ubuntu 安全公告（如 USN-6130-1）和内核源码仓库**，进一步验证补丁引入版本及其变更内容。

---

#### 步骤二：在 Git 中定位补丁提交

我们可以在 [Ubuntu kernel Git 仓库](https://git.launchpad.net/ubuntu/+source/linux/) 中查找修复该漏洞的具体补丁。

以 `jammy` 分支为例（Ubuntu 22.04 使用的内核分支）：

```bash
git clone https://git.launchpad.net/ubuntu/+source/linux
cd linux
git checkout Ubuntu-5.15.0-73.80
git log --grep="CVE-2023-32233"
```

或者关键词搜索（针对漏洞描述中的模块）：

```bash
git log --grep="nf_tables" --grep="anonymous set" --grep="Use-after-free"
```

我们会发现类似以下的提交：

```
commit 7f14c7227f34e1b4f3cc6e2acdb39c0de5f0b58e
Author: Florian Westphal <fw@strlen.de>
Date:   2023-05-24

    netfilter: nf_tables: skip deactivated anonymous sets during lookups

    [Fix CVE-2023-32233]
```

---

#### 步骤三：查看补丁改动详情

```bash
git show 7f14c7227f34
```

关键代码改动如下：

```diff
diff --git a/net/netfilter/nf_tables_api.c b/net/netfilter/nf_tables_api.c
index a1b2c3d..8f2e0a1 100644
--- a/net/netfilter/nf_tables_api.c
+++ b/net/netfilter/nf_tables_api.c
@@ -1234,6 +1234,9 @@ static struct nft_set *nft_set_lookup(const struct nft_ctx *ctx,
  struct nft_set *set;
 
  list_for_each_entry(set, &table->sets, list) {
+   if (!nft_set_is_active(set))
+     continue;
+
    if (strcmp(set->name, name) != 0)
      continue;
    return set;
```

---

#### 步骤四：验证影响范围与补丁溯源（可选）

##### 1. 查看影响范围的文件历史：

```bash
git blame net/netfilter/nf_tables_api.c -L 1234,1240
```

##### 2. 查看该补丁是否被 Cherry-pick 到 Ubuntu 分支：

可搜索 Ubuntu `jammy` 分支上的提交是否包含该修复：

```bash
git log Ubuntu-5.15.0-73.80 --grep="netfilter: nf_tables: skip deactivated"
```

---

### 5.2 示例二：追踪补丁最初来源（邮件列表）

**目标：** 分析补丁 `sched: Fix kernel panic in copy_process()` 的首发来源及讨论上下文。

#### 步骤一：通过 changelog 获取补丁标题

```
* Kernel panic in copy_process() due to race (LP: #2038882)
  - sched: Fix kernel panic in copy_process() due to cgroup_attach_task_all() race
```

#### 步骤二：在邮件列表中搜索补丁原始提交邮件

访问以下网站搜索关键词 `sched copy_process panic`：

* [https://lore.kernel.org/](https://lore.kernel.org/)
* [https://patchwork.kernel.org/project/linux-kernel/list/](https://patchwork.kernel.org/project/linux-kernel/list/)

示例搜索结果：

```
[PATCH v2] sched: Fix kernel panic in copy_process() due to cgroup_attach_task_all() race
Message-Id: <20231001152318.123456-1-dev@foo.com>
```

#### 步骤三：分析邮件内容

在补丁邮件中你可查看：

* 提交人及时间戳；
* 相关维护者、子系统的 CC 信息（如 `linux-sched`、`cgroups`）；
* 完整提交说明、修复背景；
* 对应 `diff`（与 Git 仓库中的内容一致）；

这能帮助我们判断：

* 补丁提出的原始背景与动因；
* 是否存在讨论或争议；
* 补丁在社区接受过程中的修改历史。

---

### 小结

| 目标 | 方法与工具 |
| ----------------- | ---------------------------- |
| 确定 CVE 修复是否合入内核版本 | `changelog` + `grep` 分析版本记录                                                                     |
| 查找补丁的提交与代码细节      | `git log`, `git show`, `git blame` 等 Git 工具                                                     |
| 分析补丁首发来源与社区讨论上下文  | [lore.kernel.org](https://lore.kernel.org)、[patchwork.kernel.org](https://patchwork.kernel.org) |

---