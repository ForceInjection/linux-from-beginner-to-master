# 管理 Linux 上的内核驱动程序和模块

## 介绍

内核模块是 Linux 操作系统中必不可少的组件，可以扩展内核的功能，而无需重新启动系统。这些模块可以动态加载和卸载，从而可以灵活高效地管理硬件驱动程序、文件系统和其他内核功能。本文将指导用于操作内核模块的各种命令，并附有示例来说明它们的用法。

### 使用内核模块的好处

* **灵活性**：可以根据需要加载和卸载模块，无需重新启动系统。
* **效率**：只加载必要的模块，节省内存。
* **定制**：用户可以根据其特定的硬件和软件需求定制系统。


## 列出模块

要列出 Linux 中的所有内核模块（包括已加载和未加载的模块），我们可以使用命令和系统目录的组合。操作方法如下：

1\. 列出已加载的模块：使用`lsmod`命令列出当前已加载的内核模块：

```bash
lsmod
Module                  Size  Used by
arptable_filter        12702  1
xt_nat                 12681  1
macvlan                19239  0
veth                   13458  0
xt_CT                  12956  2
nf_log_ipv4            12767  0
nf_log_common          13317  1 nf_log_ipv4
xt_LOG                 12690  0
xt_limit               12711  0
iptable_raw            12678  1
ip_set_hash_net        36021  2
vxlan                  53857  0
ip6_udp_tunnel         12755  1 vxlan
udp_tunnel             14423  1 vxlan
...
```

**说明**：`Module`、`Size` 和 `Used by` 列分别有以下含义：

**模块**：
   - 此列表示模块的名称。内核模块是加载到Linux内核中的可扩展组件，用于提供额外的功能，例如文件系统支持、网络功能或驱动硬件程序。
   - 每个模块都有唯一的名称，通常提供相关的功能。

**大小**：
   - 此列表示模块的大小，单位是字节。它指的是在内存中占用的空间。
   - 大小并不直接与磁盘上的文件大小相关，而是内核加载模块分配给它的内存量。

**被谁（其它模块）使用**：
   -此列显示有多少个其他模块或进程正在使用该模块。
   - 数值为0表示模块已加载，但当前没有进程或其他依赖模块。
   - 如果有非零值，则表明有其他模块或进程在使用该模块。例如，一个模块可能被多个程序或模块引用，如网络堆栈中的防火墙规则使用网络相关的模块。

2\. 列出**所有可用模块**（包括未加载的模块）：我们可以通过查看 `/lib/modules/$(uname -r)/` 目录来找到所有可用模块，方法如下：

```bash
find /lib/modules/$(uname -r) -type f -name '*.ko*'
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/acerhdf.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/amilo-rfkill.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/apple-gmux.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/asus-laptop.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/asus-nb-wmi.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/asus-wmi.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/chromeos_laptop.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/classmate-laptop.ko.xz
/lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/drivers/platform/x86/compal-laptop.ko.xz
...
```
此命令将列出当前内核版本的模块目录中的所有内核目标文件（`*.ko` 或 `*.ko.xz`）。

我们可以通过如下命令统计内核模块的数量：

```bash
查找 /lib/modules/$(uname -r) -type f -name '*.ko*' | wc -l
2513
```

3\. 要获取更易读的模块名称列表（不含完整路径），我们可以使用方式：

```bash
find /lib/modules/$(uname -r) -type f -name '*.ko*' | sed 's/.*\/\([^\/]*\)\.ko.*/\1/'
snd-util-mem
snd-usb-6fire
snd-bcd2000
snd-usb-caiaq
snd-usb-hiface
snd-usb-line6
snd-usb-pod
snd-usb-podhd
snd-usb-toneport
snd-usb-variax
snd-ua101
snd-usb-audio
snd-usbmidi-lib
snd-usb-us122l
snd-usb-usx2y
snd-hdmi-lpe-audio
irqbypass
...
```

> **_请记住，确切的模块列表可能因 Linux 发行版和内核版本而异。此外，某些模块可能内置于内核中，而不是可加载模块，在这种情况下，它们不会出现在这些列表中。_**

## 显示模块信息

`modinfo` 命令提供有关内核模块的详细信息，包括其作者、描述、许可证和参数。

```bash
modinfo <module_name>

modinfo arptable_filter
filename:       /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arptable_filter.ko.xz
description:    arptables filter table
author:         David S. Miller <davem@redhat.com>
license:        GPL
retpoline:      Y
rhelversion:    7.9
srcversion:     B4EA00DA2CA37FA761AEE7F
depends:        arp_tables # <------- 依赖 arp_tables
intree:         Y
vermagic:       3.10.0-1160.119.1.el7.x86_64 SMP mod_unload modversions
signer:         CentOS Linux kernel signing key
sig_key:        68:EA:10:3F:2C:90:A8:DC:0B:B0:44:6C:06:D1:45:61:F2:9E:11:72
sig_hashalgo:   sha256
```
输出信息典型中，每个字段的含义如下（以 `modinfo arptable_filter` 为例）：

1. **filename**: 内核模块文件的路径和名称。这里是指`arptable_filter.ko.xz`，`.xz`表示该文件是压缩过的。
2. **描述**：内核模块的简要描述。这里说明这是一个arptables的过滤表。
3. **作者**：内核模块的作者。这里是David S. Miller，并且给出了他的电子邮件地址。
4. **license**：内核模块使用的许可证类型。这里使用的是GPL（通用公共）许可证。
5. **retpoline**: 表示该模块是否支持retpoline，这是一种用于解除Spectre攻击的编译器技术。这里标记为`Y`，表示支持。
6. **rhelversion**: 表示该模块兼容的Red Hat Enterprise Linux（RHEL）版本。这里指的是7.9版本。
7. **srcversion**：内核模块的源代码版本。这里是一个特定的版本标识符。
8. **depends**: 该模块依赖于`arp_tables`模块。这里`arptable_filter`依赖于`arp_tables`模块。
9. **intree**: 表示该模块是否是内核源代码树的一部分。`Y`表示是。
10. **vermagic**: 内核模块的版本魔术字符串，包含内核版本、是否支持SMP（便宜多处理）、模块卸载、模块版本信息等。
11. **signer**：内核模块的签名者。这里指的是CentOS Linux的内核签名密钥。
12. **sig_key**：签名密钥的指纹，用于验证模块的来源和区别。
13. **sig_hashalgo**：用于生成签名的哈希算法。这里使用的是SHA-256算法。

> _注意：此命令适用于已加载和未加载的模块_

## 检查模块依赖关系

* **直接依赖项**：这些是模块运行所需的直接依赖项。它们列在 `modules.dep` 文件中。
* **间接依赖**：这些是直接依赖的依赖。换句话说，它们是原始模块所依赖的模块所需的模块。

### 在 `modules.dep` 中使用 `grep` 查找

`modules.dep` 文件包含每个内核模块的 **直接依赖项** 列表。我们可以使用 `grep` 命令搜索特定模块并查看其直接依赖项。

```bash
grep <module_name>  /lib/modules/$(uname -r)/modules.dep

grep arptable_filter /lib/modules/$(uname -r)/modules.dep
kernel/net/ipv4/netfilter/arptable_filter.ko.xz: kernel/net/ipv4/netfilter/arp_tables.ko.xz
```

> **注意：**如果指定的单词存在于其他条目中，此命令也可能会产生其他输出。

### 使用“modprobe --show-depends”

`modprobe --show-depends` 命令通过解决**直接和间接依赖关系**提供更全面的视图。它确保所有必要的模块都以正确的顺序加载。

```bash
modprobe --show-depends <module_name>

modprobe --show-depends arptable_filter
insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arp_tables.ko.xz
insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arptable_filter.ko.xz
```

**说明**

`insmod` 是 Linux 中用于直接加载内核模块的命令，它插入指定的模块文件（通常为 `.ko` 或 `.ko.xz` 格式）插入内核。`insmod /lib/modules/3.10. 0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arp_tables.ko.xz` 表示，在加载 `arptable_filter` 模块之前，必须先加载其依赖的`arp_tables` 模块。

`insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arptable_filter.ko.xz` 表示加载目标模块 `arptable_filter`。它依赖于 `arp_tables`，扩展了ARP数据包的过滤功能，用于在ARP表上应用特定的过滤规则。

- **依赖关系**：`arptable_filter`模块依赖于`arp_tables`模块，这意味着在加载`arptable_filter`模块时，必须先加载`arp_tables`模块。
- **加载顺序**：
  1. 先加载 `arp_tables` 模块。
  2. 然后加载`arptable_filter`模块。

## 加载模块

### 使用 `modprobe` 加载模块

`modprobe` 命令是加载内核模块最常用的工具。**它会自动处理模块依赖关系**，让管理复杂的模块层次结构变得更加容易。

```bash
sudo modprobe <module_name>
```

### 使用 `insmod` 加载模块

或者，您可以使用“insmod”命令，它需要模块文件的完整路径并且不会自动处理依赖关系。

```bash
sudo insmod /<path-to-the-module>
```

#### 自动模块加载

**持久模块加载**：要在启动时自动加载模块，请将它们添加到 `/etc/modules` 或某些发行版中的 `/etc/modules-load.d/`。

```bash
echo "[module_name]" | sudo tee -a /etc/modules
 
 # 在/etc/modules-load.d/中添加配置文件，包含需要加载的模块。例如：
ll /etc/modules-load.d/
total 36
-rw-r--r--. 1 root root 11 Mar 11  2024 arp_tables.conf
-rw-r--r--. 1 root root 13 Mar 11  2024 br_netfilter.conf
-rw-r--r--. 1 root root 13 Mar 11  2024 dm_thin_pool.conf
-rw-r--r--. 1 root root  6 Mar 11  2024 ip_vs.conf
-rw-r--r--. 1 root root  9 Mar 11  2024 ip_vs_rr.conf
-rw-r--r--. 1 root root  9 Mar 11  2024 ip_vs_sh.conf
-rw-r--r--. 1 root root 10 Mar 11  2024 ip_vs_wrr.conf
-rwxr-xr-x  1 root root  8 Mar 19  2024 kiodump.conf
-rw-r--r--. 1 root root 18 Mar 11  2024 nf_conntrack_ipv4.conf

cat /etc/modules-load.d/arp_tables.conf
arp_tables
```

**为硬件创建 Udev 规则**：可以创建 Udev 规则，当检测到特定硬件时自动加载模块。Udev 规则放在 `/etc/udev/rules.d/` 中。

```bash
cat /etc/udev/rules.d/70-persistent-ipoib.rules
# This is a sample udev rules file that demonstrates how to get udev to
# set the name of IPoIB interfaces to whatever you wish.  There is a
# 16 character limit on network device names.
#
# Important items to note: ATTR{type}=="32" is IPoIB interfaces, and the
# ATTR{address} match must start with ?* and only reference the last 8
# bytes of the address or else the address might not match the variable QPN
# portion.
#
# Modern udev is case sensitive and all addresses need to be in lower case.
#
# ACTION=="add", SUBSYSTEM=="net", DRIVERS=="?*", ATTR{type}=="32", ATTR{address}=="?*00:02:c9:03:00:31:78:f2", NAME="mlx4_ib3"
```

## 删除模块

### 使用 `modprobe -r`

要删除模块，您可以使用“modprobe -r”命令，该命令也可以处理依赖项。

```bash
sudo modprobe -r <module_name>
```

### 使用 `rmmod`

`rmmod` 命令也可用于删除模块，但它不处理依赖关系**。

```bash
sudo rmmod <module_name>
```

## 排除常见驱动程序问题

1\. **模块加载失败**：

- **症状**：尝试加载模块时出现错误消息。
- **解决方案**：检查 `dmesg` 中的错误日志。确保模块与您的内核版本兼容。


2\. **硬件无法识别**：

- **症状**：设备不工作或未被检测到。
- **解决方案**：加载适当的驱动模块。检查硬件兼容性。

3\. **冲突的模块**：

- **症状**：系统不稳定或硬件无法正常运行。
- **解决方案**：使用 `lsmod` 和 `modprobe -r` 识别并删除冲突的模块。

4\. **模块依赖关系**:

- **问题**：一些模块依赖其他模块才能正常运行。
- **解决方案**：使用 `modinfo [module_name]` 查看模块依赖关系并相应地加载它们。

## 生成模块依赖关系

`depmod` 命令用于生成 `modules.dep` 和 map 文件，这些文件对于 `modprobe` 命令正确解析依赖关系至关重要。

```bash
depmod -a
```
此命令通常在系统启动期间和安装新内核模块后自动运行。

## 结论

大多数情况下，如果发行版不支持开箱即用的硬件，或者正在使用提供自己的内核模块的第三方应用程序（如 VMware），那么我们才需要处理内核模块。学习和掌握如何添加或删除内核模块技能不会让你吃亏。即使现在不需要它们，有可能在将来的某一天会用上本文中介绍的技巧。