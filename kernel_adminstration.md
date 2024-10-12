# Managing Kernel Drivers and Modules on Linux

## Introduction

Kernel modules are essential components in the Linux operating system that extend the functionality of the kernel without the need to reboot the system. These modules can be dynamically loaded and unloaded, allowing for a flexible and efficient way to manage hardware drivers, file systems, and other kernel features. This article will guide you through the various commands used to manipulate kernel modules, complete with examples to illustrate their usage. 

### Benefits of Using Kernel Modules

*   **Flexibility**: Modules can be loaded and unloaded as needed, without restarting the system.
*   **Efficiency**: Only necessary modules are loaded, saving memory.
*   **Customization**: Users can tailor the system to their specific hardware and software needs.


## Listing Modules

To list all kernel modules in Linux, including both loaded and non-loaded modules, you can use a combination of commands and system directories. Here’s how you can do it:

1.  List loaded modules: Use the `lsmod` command to list currently loaded kernel modules:

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

1. **Module**：
   - 这一列表示模块的名称。内核模块是加载到 Linux 内核中的可扩展组件，用于提供额外的功能，例如文件系统支持、网络功能或硬件驱动程序。
   - 每个模块都有唯一的名称，通常与其提供的功能相关联。

2. **Size**：
   - 这一列表示模块的大小，单位是字节。它指的是该模块在内存中占用的空间。
   - 该大小并不直接与磁盘上的文件大小相关，而是内核加载模块后分配给它的内存量。

3. **Used by**：
   - 这一列显示有多少个其他模块或进程正在使用该模块。
   - 数值为 0 表示模块已加载，但当前没有进程或其他模块依赖它。
   - 如果有非零值，则表明有其他模块或进程在使用该模块。例如，一个模块可能被多个程序或模块引用，如网络堆栈中的防火墙规则使用网络相关的模块。

2\. List **all available modules** (including non-loaded ones): You can find all available modules by looking in the `/lib/modules/$(uname -r)/` directory. Here's how to list them:

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
This command will list all kernel object files (`*.ko` or `*.ko.xz`) in the modules directory for your current kernel version.

我们可以通过如下命令统计内核模块的数量：

```bash
find /lib/modules/$(uname -r) -type f -name '*.ko*'  | wc -l
2513
```

3\. To get a more readable list of module names **without the full path**, you can use:

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

> **_Remember that the exact list of modules can vary depending on your Linux distribution and kernel version. Also, some modules might be built into the kernel rather than being loadable modules, in which case they won’t appear in these lists._**

## Displaying Module Information

The `modinfo` command provides detailed information about a kernel module, including its author, description, license, and parameters.

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
输出典型信息中，每个字段的含义如下（以 `modinfo arptable_filter `为例）：

1. **filename**: 内核模块文件的路径和名称。这里指的是`arptable_filter.ko.xz`，`.xz`表示该文件是压缩过的。
2. **description**: 内核模块的简短描述。这里说明这是一个arptables的过滤表。
3. **author**: 内核模块的作者。这里作者是David S. Miller，并且给出了他的电子邮件地址。
4. **license**: 内核模块使用的许可证类型。这里使用的是GPL（通用公共许可证）。
5. **retpoline**: 表示该模块是否支持retpoline，这是一种用于缓解Spectre攻击的编译器技术。这里标记为`Y`，表示支持。
6. **rhelversion**: 表示该模块兼容的Red Hat Enterprise Linux（RHEL）版本。这里指的是7.9版本。
7. **srcversion**: 内核模块的源代码版本。这里是一个特定的版本标识符。
8. **depends**: 该模块依赖的其他内核模块。这里`arptable_filter`依赖于`arp_tables`模块。
9. **intree**: 表示该模块是否是内核源代码树的一部分。`Y`表示是。
10. **vermagic**: 内核模块的版本魔术字符串，包含了内核版本、是否支持SMP（对称多处理）、模块卸载、模块版本信息等。
11. **signer**: 内核模块的签名者。这里指的是CentOS Linux的内核签名密钥。
12. **sig_key**: 签名密钥的指纹，用于验证模块的来源和完整性。
13. **sig_hashalgo**: 用于生成签名的哈希算法。这里使用的是SHA-256算法。

> _NOTE: This command works for both loaded and non loaded modules_

## Check module dependencies

*   **Direct Dependencies**: These are the immediate dependencies that a module requires to function. They are listed in the `modules.dep` file.
*   **Indirect Dependencies**: These are dependencies of the direct dependencies. In other words, they are the modules required by the modules that the original module depends on.

### — Using “grep” on “modules.dep”

The `modules.dep` file contains a list of **direct dependencies** for each kernel module. You can use the `grep` command to search for a specific module and see its direct dependencies.

```bash
grep <module_name>  /lib/modules/$(uname -r)/modules.dep

grep arptable_filter /lib/modules/$(uname -r)/modules.dep
kernel/net/ipv4/netfilter/arptable_filter.ko.xz: kernel/net/ipv4/netfilter/arp_tables.ko.xz
```

> **_Note:_** _This command may also produce additional outputs if the specified word exists in other entries._

### Using “modprobe --show-depends”

The `modprobe --show-depends` command provides a more comprehensive view by resolving **both direct and indirect dependencies**. It ensures that all necessary modules are loaded in the correct order.

```bash
modprobe --show-depends <module_name>

modprobe --show-depends arptable_filter
insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arp_tables.ko.xz
insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arptable_filter.ko.xz
```

**说明**

`insmod` 是 Linux 中用于直接加载内核模块的命令，它会将指定的模块文件（通常为 `.ko` 或 `.ko.xz` 格式）插入内核。`insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arp_tables.ko.xz` 表示，在加载 `arptable_filter` 模块之前，必须先加载它依赖的 `arp_tables` 模块。

`insmod /lib/modules/3.10.0-1160.119.1.el7.x86_64/kernel/net/ipv4/netfilter/arptable_filter.ko.xz` 表示加载目标模块 `arptable_filter`。它依赖于 `arp_tables`，扩展了 ARP 数据包的过滤功能，用于在 ARP 表上应用特定的过滤规则。

- **依赖关系**：`arptable_filter` 模块依赖于 `arp_tables` 模块，这意味着在加载 `arptable_filter` 模块时，必须先加载 `arp_tables` 模块。
- **加载顺序**：
  1. 先加载 `arp_tables` 模块。
  2. 然后加载 `arptable_filter` 模块。

## Loading Modules

### Loading Modules using “modprobe”

The `modprobe` command is the most commonly used tool for loading kernel modules. **It automatically handles module dependencies**, making it easier to manage complex module hierarchies.

```bash
sudo modprobe <module_name>
```

### Loading Modules using “insmod”

Alternatively, you can use the `insmod` command, which requires the full path to the module file and does not handle dependencies automatically.

```bash
sudo insmod /<path-to-the-module>
```

#### Automating Module Loading

*   **Persistent Module Loading**:  
    To load modules automatically at boot, add them to `/etc/modules` or `/etc/modules-load.d/` in some distributions.

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
*   **Creating Udev Rules for Hardware**:  
    Udev rules can be created to load modules automatically when specific hardware is detected.Udev rules are placed in `/etc/udev/rules.d/`.

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

## Removing Modules

### Using “modprobe -r”

To remove a module, you can use the `modprobe -r` command, which also handles dependencies.

```bash
sudo modprobe -r <module_name>
```

### Using “rmmod”

The `rmmod` command can also be used to remove a module, **but it does not handle dependencies**.

```bash
sudo rmmod <module_name>
```

## Troubleshooting Common Driver Issues

1\. **Module Fails to Load**:

- **Symptom**: Error messages when attempting to load a module.
- **Solution**: Check `dmesg` for error logs. Ensure the module is compatible with your kernel version.


2\. **Hardware Not Recognized**:

-  **Symptom**: A device is not working or not detected.
- **Solution**: Load the appropriate driver module. Check hardware compatibility.

3\. **Conflicting Modules**:

- **Symptom**: System instability or hardware not functioning correctly.
- **Solution**: Identify and remove conflicting modules using `lsmod` and `modprobe -r`.

4\. **Module Dependencies**:

- **Issue**: Some modules depend on others to function correctly.
- **Solution**: Use `modinfo [module_name]` to view module dependencies and load them accordingly.

## Generating Module Dependencies

The `depmod` command is used to generate `modules.dep` and map files, which are essential for the `modprobe` command to resolve dependencies correctly.

```bash
depmod -a
```
This command is typically run automatically during system boot and after installing new kernel modules.

## Conclusion

Managing kernel modules is a fundamental skill for anyone working with the Linux operating system. From listing and displaying module information to loading and removing modules, the commands and techniques covered in this article provide a comprehensive guide to kernel module management. By mastering these commands, you can ensure that your system runs smoothly and efficiently, with the flexibility to adapt to new hardware and software requirements. Whether you’re troubleshooting issues or optimizing performance, a solid understanding of kernel modules will serve you well in your Linux journey.