# Linux 挂载传播机制原理解析

## 1. 背景与概念

**挂载传播**（`mount propagation`）机制是 `Linux` kernel 中用于控制挂载/卸载（`mount`/`umount`）事件在不同挂载点及命名空间之间传播的机制。它建立在"**挂载命名空间**"（`mount namespace`）与"**共享子树**"（`shared subtree`）机制之上[1]。

### 1.1 挂载命名空间（`Mount namespace`）

通过系统调用 `clone(CLONE_NEWNS)` 或 `unshare(CLONE_NEWNS)` 可以创建一个新的挂载命名空间。新的命名空间会从当前命名空间复制一份挂载列表（`mount list`）作为初始状态。默认情况下，两个不同的挂载命名空间中，后续的 `mount`／`umount` 操作互不影响。

### 1.2 共享子树（`Shared Subtree`）与传播类型

为了解决"在多个 `namespace` 中重复做 `mount`"的问题，`Linux` 在 2.6.15 版本引入了"共享子树"机制[2]。该机制的核心在于每个挂载点（`mount point`）都带有一个**传播类型（`propagation type`）**，主要包括以下四种类型[1]:

- **`MS_SHARED`**：该挂载属于一个"**同级挂载组**"（`peer group`），在组内，对任意挂载点的新增或移除挂载（`mount`/`umount`）事件都会传播至组内其他成员，实现双向传播。
- **`MS_PRIVATE`**：不参与任何传播；挂载与卸载事件既不会从此挂载传播出去，也不会接收来自其他挂载的传播事件。
- **`MS_SLAVE`**：单向接收传播；只接收来自"主"共享组的传播事件，但自身在其下的挂载操作不会被传播回去，形成单向从属关系。
- **`MS_UNBINDABLE`**：类似 `MS_PRIVATE`，但额外禁止将其作为 `bind mount` 的源，主要用于**防止循环绑定**。

说明与来源：共享子树机制与传播类型定义参考[1,2,4]。

### 1.3 传播相关参数速查（mount）

- `mount --bind SRC DST`：绑定挂载；在 `DST` 位置呈现 `SRC` 树的一个视图，不改变传播类型。
- `mount --rbind SRC DST`：递归绑定；复制 `SRC` 子树所有可绑定挂载，跳过 `unbindable` 节点。
- `mount --make-private PATH`：将挂载标记为 `private`；不参与传播。
- `mount --make-shared PATH`：将挂载标记为 `shared`；加入或创建 `peer group`，事件在同组复制。
- `mount --make-slave PATH`：将挂载标记为 `slave`；只接收来自"主"共享组的传播，事件不回流。
- `mount --make-unbindable PATH`：标记为 `unbindable`；禁止作为 `bind` 源。
- 递归变更（作用于子树）：`mount --make-rprivate PATH`、`--make-rshared PATH`、`--make-rslave PATH`、`--make-runbindable PATH`。

```bash
# 参数速查用法演示（示例，不直接执行）

# 将 /path 设为 shared／rshared（加入或创建 peer group）
mount --make-shared /path
mount --make-rshared /path

# 切换为 private／rprivate（不参与传播）
mount --make-private /path
mount --make-rprivate /path

# 标记为 slave／rslave（单向从主组到此挂载）
mount --make-slave /path
mount --make-rslave /path

# 禁止绑定源（unbindable／runbindable）
mount --make-unbindable /path
mount --make-runbindable /path

# 绑定挂载与递归绑定（不改变传播类型）
mount --bind /src /dst
mount --rbind /src /dst
```

说明与来源：

- 参数含义与用法参考 `mount(8)` 与 `Linux` 内核文档 `Documentation/filesystems/sharedsubtree.txt`[4]；
- 传播类型定义参见 `mount_namespaces(7)`。

---

## 2. 机制原理及源码简析

下面从挂载机制流程 + 对应源码片段（简化逻辑）来分析挂载传播的关键环节。

### 2.1 挂载（`mount`）或绑定挂载（`bind mount`）触发传播

当执行 `mount()` 或 `mount --bind` 操作时，内核会判断当前目标挂载点（`destination`）所在挂载树（`vfsmount`）所属的传播类型。如果该挂载是 `MS_SHARED`，那么挂载操作不仅在本命名空间发生，还会"克隆"/"传播"至其它属于同一同行组（`peer group`）或从属组（`slave group`）中的挂载点。

在 `Documentation/filesystems/sharedsubtree.txt`[4]中，对于 `bind` 操作，分别列出 "源 A 的状态"与 "目标 B 的状态" 所对应的传播结果。技术原理可以参考以下伪代码（参照内核 `vfsmount` 结构与 `shared-subtree` 实现）：

```c
struct vfsmount {
    struct mount *mnt;       /* 指向 mount 结构 */
    struct list_head mnt_peer;/* peer group 列表 */
    enum mnt_propagation_type mnt_propagation; /* shared/private/slave/unbindable */
    /* … 其他字段 … */
};

/* 在 mount_recursion() 或 do_mount() 内部，当出现 bind 或 mount 操作时： */
if (dest->mnt_propagation == MNT_SHARED) {
    /* 对于 dest 所在的 peer group 中所有成员 g:
       在每一个 g 上也执行 clone/source 挂载操作 */
    for_each_peer_member(g) {
        clone_mount(source, g_target_path);
    }
}
```

（实际内核实现更复杂，涉及 `mnt_clone_tree()`、`propagate_mount()`、`peer_create()` 等函数）参考 `Documentation/filesystems/sharedsubtree.txt`[4]中的规则。

下面给出两个挂载传播机制的实践示例，分别在单命名空间内和跨命名空间场景下演示传播行为。所有演示均在隔离的 mount 命名空间内进行，使用安全的临时文件系统（tmpfs）路径，确保对系统影响最小。

**示例 1：单命名空间内的共享传播演示**：

```bash
# ====================================================================
# 示例 1：单命名空间内的共享传播演示（安全路径版）
# 目标：在 /tmp 下使用受控挂载演示 shared 复制，避免触碰 /
# 重要：在隔离的 mount 命名空间中进行（不会影响其他进程）
# ====================================================================
# 1）前置检查（在宿主机执行）：使用 /tmp 路径，避免系统风险
# 说明：不再使用 / 与 /host；所有演示均在 /tmp 下进行。
sudo mkdir -p /tmp/demo_root /tmp/mp1 /tmp/mp2
mountpoint -q /tmp/mp1 && echo "/tmp/mp1 已是挂载点" || echo "/tmp/mp1 非挂载点"
#-----------期望输出-----------
/tmp/mp1 非挂载点
#-----------END-----------

mountpoint -q /tmp/mp2 && echo "/tmp/mp2 已是挂载点" || echo "/tmp/mp2 非挂载点"
#-----------期望输出-----------
/tmp/mp2 非挂载点
#-----------END-----------

# 2）进入隔离的 mount 命名空间
# 说明：使用 --propagation private 保持新命名空间为私有，避免跨命名空间传播；退出后命名空间销毁，挂载随之消失。
sudo unshare -m --propagation private bash

# 3）顶层受控挂载与对等视图
# 挂载点说明：
# - 在 /tmp/demo_root 上创建一个独立的 tmpfs 顶层挂载（受控、可写、不会影响 /）
# - 将该挂载递归绑定到 /tmp/mp1，随后设置 /tmp/mp1 为 shared
# - 再将 /tmp/mp1 的挂载树递归绑定到 /tmp/mp2，形成同组对等挂载（peer group）
mount -t tmpfs tmpfs /tmp/demo_root
mount --rbind /tmp/demo_root /tmp/mp1
mount --make-shared /tmp/mp1
mount --rbind /tmp/mp1 /tmp/mp2

# 4）验证传播属性与共享组（shared:N 应相同）
# 数据来源：findmnt(8) 读取 /proc/self/mountinfo；/proc/self/mountinfo 为权威来源。
findmnt -no TARGET,PROPAGATION /tmp/mp1
#-----------期望输出-----------
/tmp/mp1 shared
#-----------END-----------

findmnt -no TARGET,PROPAGATION /tmp/mp2
#-----------期望输出-----------
/tmp/mp2 shared
#-----------END-----------

# 查看两个顶层挂载点（均应 shared）
grep -E '(/tmp/mp1 |/tmp/mp2 )' /proc/self/mountinfo
#-----------期望输出（示例）-----------
56 3719 0:38 / /tmp/mp1 rw,relatime shared:35 - tmpfs tmpfs rw
60 3719 0:38 / /tmp/mp2 rw,relatime shared:35 - tmpfs tmpfs rw
#-----------END-----------

# 提取共享组 ID（shared:N），验证两者属于同一共享组
grep -E '(/tmp/mp1 |/tmp/mp2 )' /proc/self/mountinfo | sed -n 's/.*shared:\([0-9]\+\).*/shared-id=\1/p'
#-----------期望输出-----------
shared-id=35
shared-id=35
#-----------END-----------

# 5）受控挂载演示复制：在 /tmp/mp1 下创建新的挂载，应复制到 /tmp/mp2
mkdir -p /tmp/mp1/tmpfs_demo
mount -t tmpfs tmpfs /tmp/mp1/tmpfs_demo

findmnt -no TARGET,FSTYPE,PROPAGATION /tmp/mp1/tmpfs_demo
#-----------期望输出-----------
/tmp/mp1/tmpfs_demo tmpfs  shared
#-----------END-----------

findmnt -no TARGET,FSTYPE,PROPAGATION /tmp/mp2/tmpfs_demo
#-----------期望输出-----------
/tmp/mp2/tmpfs_demo tmpfs  shared
#-----------END-----------

grep -E '(/tmp/mp1|/tmp/mp2)/tmpfs_demo' /proc/self/mountinfo
#-----------期望输出（示例）-----------
3735 3727 0:672 / /tmp/mp1/tmpfs_demo rw,relatime shared:39 - tmpfs tmpfs rw
3736 3731 0:672 / /tmp/mp2/tmpfs_demo rw,relatime shared:39 - tmpfs tmpfs rw
#-----------END-----------

umount /tmp/mp1/tmpfs_demo || true

# 6）（可选）对比 rslave：仅从“源”到“视图”单向传播（安全路径）
#    若需单向传播演示，可先移除对等挂载，再将 /tmp/mp1 设为 rslave：
#    umount /tmp/mp2
#    mount --make-rslave /tmp/mp1
#    mount -t tmpfs tmpfs /tmp/mp1/another_demo
#    说明：此时仅能观察到 “从 /tmp/demo_root → /tmp/mp1” 的传播；在 rslave 下，/tmp/mp1 的事件不会回流到其它视图。

# 7）清理并退出命名空间（安全）
cd /
exit

# 验证不再是挂载点后再删除目录（避免 Device or resource busy）
mountpoint -q /tmp/mp2 || sudo rm -rf /tmp/mp2
mountpoint -q /tmp/mp1 || sudo rm -rf /tmp/mp1
mountpoint -q /tmp/demo_root || sudo rm -rf /tmp/demo_root

# 最终确认：不应再有挂载记录
grep -E ' /tmp/(mp1|mp2|demo_root) ' /proc/self/mountinfo || echo "cleaned"
```

**示例 2**：跨两个命名空间的挂载传播演示（双终端操作版）

```bash
# ====================================================================
# 示例 2：跨两个命名空间的挂载传播演示（双终端操作版）
# 目标：使用两个终端演示挂载事件在命名空间之间的传播
# ====================================================================

# ========================================
# 终端 1 - 创建父命名空间并设置共享挂载
# ========================================

# 1）进入隔离的 mount 命名空间
# 说明：使用 --propagation private 保持新命名空间为私有，避免跨命名空间传播；退出后命名空间销毁，挂载随之消失。
sudo unshare -m --propagation private bash

# 2）在父命名空间中操作
mkdir -p /mnt/shared

# 挂载点说明：在 /mnt/shared 挂载独立 tmpfs 并设置为共享传播类型
# - 文件系统类型：tmpfs（内存文件系统，隔离且更安全）
# - 传播类型：shared（双向传播）
# - 效果：在此挂载点下的操作将传播到同组其他挂载点
# - 共享组 ID：内核自动生成唯一标识
mount -t tmpfs tmpfs /mnt/shared
mount --make-shared /mnt/shared

# 3）显示当前挂载状态
# 挂载点说明：查看 /mnt/shared 挂载点的传播属性
# - 预期输出：shared（表示已成功设置为共享传播类型）
findmnt -no TARGET,PROPAGATION /mnt/shared
#-----------期望输出-----------
/mnt/shared shared
#-----------END-----------

# 4）获取共享组 ID 和进程 ID
# 挂载点说明：从 /proc/self/mountinfo 中提取共享组标识
# - shared:[N]：N 为共享组唯一标识，用于跨命名空间传播识别
# - 同一共享组内的挂载点共享相同的组ID
# 从 mountinfo 的第 5 列精确匹配目标路径，并从可选字段提取 shared:N
SHARED_ID=$(awk '$5=="/mnt/shared"{ for(i=1;i<=NF;i++) if($i ~ /^shared:[0-9]+$/){ split($i,a,":"); print a[2]; exit } }' /proc/self/mountinfo)
echo "共享组 ID: $SHARED_ID"
#-----------期望输出-----------
共享组 ID: 37
#-----------END-----------

# PID 用于子命名空间通过 nsenter 进入父命名空间
echo "父命名空间 PID: $$"

#-----------期望输出-----------
父命名空间 PID: 372822
#-----------END-----------

# 注意：请保持 terminal 打开，确保父命名空间运行，等待子命名空间操作

# ========================================
## 终端 2：子命名空间（接收传播）
# ========================================

# 1）首先获取父命名空间的 PID（替换为终端 1 显示的 PID）
PARENT_PID=372822

# 2）进入父命名空间创建子命名空间
## 保留父侧传播属性（unchanged），随后将 /mnt/shared 切换为 slave
sudo nsenter -m -t $PARENT_PID unshare -m --propagation unchanged bash
findmnt -no TARGET,PROPAGATION /mnt/shared
##-----------期望输出-----------
/mnt/shared shared
##-----------END-----------

# 3）设置为 slave 模式以接收传播（可选）
# 说明：
#   仅接收父侧事件但不反向传播（findmnt 显示为 "slave" 或 "private,slave"）。
mount --make-slave /mnt/shared
findmnt -no TARGET,PROPAGATION /mnt/shared
##-----------期望输出-----------
/mnt/shared private,slave
##-----------END-----------

# 4）验证 slave 状态
# 挂载点说明：检查 /proc/self/mountinfo 中的 master 标识
# - master:[N]：N 为主共享组标识，表示此挂载点从属于哪个共享组（父侧的 shared 组）
# - 预期输出：master:XXX（XXX 通常与父命名空间的 shared 组 ID 相同，如 shared:37 → master:37）
awk '$5=="/mnt/shared"{ for(i=1;i<=NF;i++) if($i ~ /^master:[0-9]+$/){ print $i; exit } }' /proc/self/mountinfo
##-----------期望输出-----------
master:37
##-----------END-----------

# ========================================
# 终端 1 - 创建挂载，观察传播效果
# ========================================

# 1）在终端 1 的父命名空间中操作
mkdir -p /mnt/shared/test-from-parent

# 2）创建新挂载（将自动传播到 slave 子命名空间）
# 挂载点说明：在父命名空间的 /mnt/shared/test-from-parent 创建 tmpfs 挂载
# - 文件系统类型：tmpfs（内存文件系统）
# - 挂载位置：/mnt/shared/test-from-parent（共享挂载点下的子目录）
# - 传播行为：由于父挂载点 /mnt/shared 为 shared 类型，此挂载将自动传播
mount -t tmpfs tmpfs /mnt/shared/test-from-parent
findmnt -no TARGET,FSTYPE,PROPAGATION /mnt/shared/test-from-parent
##-----------期望输出-----------
/mnt/shared/test-from-parent tmpfs  shared
##-----------END-----------

# 3）等待传播完成
sleep 2

# ========================================
# 终端 2 - 验证传播结果
# ========================================

# 1）在子命名空间中检查
# 挂载点说明：检查子命名空间中是否出现了传播过来的挂载点
# - 目标路径：/mnt/shared/test-from-parent（与父命名空间相同路径）
# - 预期结果：由于 slave 传播机制，此挂载点应自动出现在子命名空间
if findmnt /mnt/shared/test-from-parent >/dev/null 2>&1; then
    echo "✅ 成功：子命名空间接收到来自父命名空间的挂载传播"
    echo "传播过来的挂载信息："
    findmnt -no TARGET,FSTYPE,PROPAGATION /mnt/shared/test-from-parent
else
    echo "❌ 失败：未接收到传播"
fi
##-----------期望输出-----------
✅ 成功：子命名空间接收到来自父命名空间的挂载传播
传播过来的挂载信息：
/mnt/shared/test-from-parent tmpfs  private,slave
##-----------END-----------

# ========================================
# 清理操作（在两个终端中执行）
# ========================================

# 终端 1：清理父命名空间
umount /mnt/shared/test-from-parent
umount /mnt/shared
mountpoint -q /mnt/shared/ || rm -rf /mnt/shared/
exit

# 终端 2：退出子命名空间
exit
```

这个双终端版本更加直观，便于实际操作和观察跨命名空间的挂载传播过程。以下是对示例设计原理的一个说明：

- **示例 1**：单命名空间内的共享传播演示，展示同组挂载点之间的传播机制
- **示例 2**：跨两个命名空间的挂载传播演示，展示：
  - 父命名空间（ns1）创建共享挂载并设置 `shared` 传播类型
  - 子命名空间（ns2）通过 `nsenter` 进入父命名空间后创建新命名空间
  - 子命名空间将继承的挂载设置为 `slave` 模式以接收单向传播
  - 父命名空间创建新挂载（tmpfs）时，事件传播到子命名空间
  - 验证跨命名空间传播的成功与否

### 2.2 卸载（`umount`）传播

相似地，当在一个挂载点下执行 `umount()` 操作且其上层挂载为 `shared group`，则卸载也会被"广播"到 `peer group` 中对应的位置。文档说明：

> "a mount/unmount event generated on a vfsmount leads to mount or unmount actions in other vfsmounts."

源码中，`do_umount()` 等函数最终会调用传播逻辑，比如 `propagate_mount()`，从 `peer group` 中移除／卸载相应挂载。

说明与来源：原理说明参考共享子树文档与源码实现[4]。

### 2.3 命名空间切分与传播类型继承

当新的挂载命名空间被创建（`unshare -m`）时，系统默认会 **将该 `namespace` 内所有挂载点标记为 `private`**（即 `MS_PRIVATE` 传播类型），以保证完全隔离。

若需要在子命名空间接收来自父命名空间的挂载事件，应首先使用 `unshare -m --propagation unchanged` 创建子命名空间以保持传播属性继承，然后显式将相应挂载设置为 `MS_SLAVE`（单向传播；常用 `mount --make-rslave`）或 `MS_SHARED`（双向传播；常用 `mount --make-rshared`）。跨命名空间的传播关系由 `master/slave` 维持，并非"保留相同 `peer group`"。group"。

说明与来源：命名空间默认传播与 `master/slave` 关系参考[1,4]。

### 2.4 常见代码路径关键词

查找源码时，可关注以下函数／结构：

- `fs/namespace.c` 中的 `do_new_mount()`（新建挂载）、`do_loopback()`（`bind`）、`copy_tree()`（递归绑定）、`attach_recursive_mnt()`／`propagate_mount()`（传播）、`do_mount()`／`do_add_mount()`（安装）与 `clone_mnt()` 等核心实现（来源：`Linux` 内核源码）。
- `fs/namei.c` 中的自动挂载相关路径解析：`follow_automount()`、`link_path_walk()`
- 结构定义与标志位：`include/linux/mount.h`（`MNT_SHARED`、`MNT_PRIVATE`、`MNT_SLAVE`、`MNT_UNBINDABLE` 等），以及 `struct mount`／`struct vfsmount`

### 2.5 源码分析摘要

- `struct mount` 中有标志 `MNT_SHARED`, `MNT_PRIVATE`, `MNT_SLAVE`, `MNT_UNBINDABLE`，这些标识决定挂载点传播类型。
- 当一个新的挂载在 `shared` 挂载内部被创建时，内核会：

  1. 在该挂载点本地执行操作；
  2. 查找它所属 `peer group`，通过 `mnt_peer` 链表找到其他成员；
  3. 在每个成员相同路径位置上 `clone` 一个挂载（或按需复用）以保持同步；

- 在 `bind` 或 `rbind` 操作时，还要考虑源树下是否包含 `unbindable` 挂载，如果有则 `prune`（剪除）对应复制 `branch`。

说明与来源：传播与克隆的实现细节参考共享子树文档与内核源码[4,5]。

---

## 3. 应用场景与误区

### 3.1 容器／`Kubernetes` 场景

在容器化环境中（如 `Docker` + `containerd` 或 `Kubernetes` 的 `Pod`），常见的挂载传播相关注意点：

- 宿主机 `/` 通常为 `shared`，这意味着容器中如果挂载了宿主机目录，有可能产生挂载传播。
- 容器默认新 `namespace` 若未额外调整通常会被标为 `private`，以避免传播。
- 若在容器内将宿主机目录 `bind` 到容器内部，再加上 `shared propagation`，就可能出现递归挂载、挂载膨胀等问题。

说明与来源：容器场景下的传播行为与默认隔离参考 `mount_namespaces(7)` 与共享子树文档[1,5]。

### 3.2 典型误区

- 误以为 "挂载在容器里就完全隔离" —— 若传播类型是 `shared`，可能仍影响宿主机／其他 `namespace`。
- 误设 `--make-shared /` 而忽略子挂载传播逻辑，导致 `bind mount` 意外传播。
- 认为 "只要是 `bind` 就不会传播" —— 实际上 `propagation` 类型决定是否传播，不仅仅 `bind` 本身。

说明与来源：以上实践与误区说明参考 `mount_namespaces(7)` 与共享子树文档[1,5]。

---

## 4. 实践指导与建议

### 4.1 如何查看当前挂载传播类型

```bash
findmnt -o TARGET,PROPAGATION
## 或
cat /proc/self/mountinfo | grep shared:
```

在 `mountinfo` 中会看到类似 `shared:23`、`master:23 propagate_from:23` 等标识。

在 `/proc/self/mountinfo` 文件中，传播相关的标识具有以下语义含义：

- **`shared:N`**：表示该挂载点属于**同级挂载组**（`peer group`）`N`。在该挂载点下方发生的挂载（`mount`）或卸载（`umount`）操作，会在同组内的所有挂载点之间相互传播。组内所有成员显示相同的 `N` 值，该标识符由内核自动分配，用于唯一标识一个传播组。
- **`master:N`**：表示该挂载点作为**从属挂载**（`slave`），其主共享组为 `N`。它仅接收来自主组的传播事件，但不会将自身事件反向传播到主组。一个挂载点可以同时具有 `shared:M` 和 `master:N` 标识，表示它既属于某个传播组，又从属于另一个主组。
- **`propagate_from:N`**：当进程的视角无法直接访问从属挂载的主组时（例如由于 `chroot` 或命名空间隔离导致主组路径不可达），此标识表示在当前视角下最近可达的传播起点为共享组 `N`。这通常出现在复杂的命名空间嵌套场景中。
- **`unbindable`**：表示该挂载点不可作为绑定挂载（`bind mount`）的源。其传播语义与 `private` 类似，但额外增加了"不可绑定"的限制，递归绑定操作会剪除（`prune`）其子树中的 `unbindable` 节点。
- **无传播标识**：表示该挂载点为 `private` 类型，不参与任何传播机制。事件既不向外传播，也不接收来自外部的传播。

注意：上述 `N` 均为共享组标识（`peer group ID`），并非 `mount ID`（字段 1）。此外同一同级挂载组在不同 `mount` 命名空间中显示相同的 `N`（即 `peer group ID` 在跨 `namespace` 的视角下保持一致）。

示例（带注释）：

```bash
# /proc/self/mountinfo 示例（省略部分字段）
239 61 8:2 / /mnt         rw,relatime shared:102         - ext4 /dev/sda1 rw
# 该挂载点在 peer group 102 中；组内成员互相传播事件

273 239 8:2 /etc /tmp/etc rw,relatime master:105 propagate_from:102 - ext4 /dev/sda1 rw
# 该挂载是共享组 105 的从属（master:105），但直接主组在当前视角不可达，
# 因此显示 propagate_from:102，提示“可见的传播起点”为组 102

77  61 8:2  / /home/cecilia rw,relatime unbindable       - ext4 /dev/sda1 rw
# 该挂载点不可作为 bind 源；递归绑定会剪除（prune）其子树中的 unbindable 节点
```

说明与来源：`mountinfo` 字段语义参考 [1]、[5]；传播模型参考 [3]；常用命令参考 [4]。

### 4.2 如何设置挂载传播类型

```bash
# 设置传播类型
mount --make-shared <mountpoint>
mount --make-private <mountpoint>
mount --make-slave <mountpoint>
mount --make-unbindable <mountpoint>

# 递归设置传播类型（影响子挂载）
mount --make-rshared <mountpoint>
mount --make-rprivate <mountpoint>
mount --make-rslave <mountpoint>
mount --make-runbindable <mountpoint>
```

注意：`--make-*` 系列参数仅对已存在的挂载点生效，不适用于新挂载操作。

### 4.3 容器环境下防止传播问题

#### 4.3.1 容器运行时配置

- 启动容器时显式指定传播类型：

  ```bash
  docker run --mount type=bind,propagation=private ...
  podman run --mount type=bind,propagation=slave ...
  ```

如需完全隔离，建议将容器内根挂载点设为 `private`：

```bash
mount --make-private /
```

#### 4.3.2 Kubernetes mountPropagation 配置

> **注意**：挂载传播是一个低级功能，并非在所有卷类型上都一致工作。Kubernetes 项目建议仅对 `hostPath` 或基于内存的 `emptyDir` 卷使用挂载传播。有关更多上下文，请参阅 [Kubernetes issue #95049](https://github.com/kubernetes/kubernetes/issues/95049)。

挂载传播允许将由容器挂载的卷共享给同一 Pod 中的其他容器，甚至共享给同一节点上的其他 Pod。在 `Kubernetes` 中，通过 `mountPropagation` 字段控制挂载传播行为：

- **`None`**（默认值）

  - 此卷挂载不会接收主机挂载到此卷或其任何子目录的任何后续挂载
  - 容器创建的挂载在主机上不可见
  - 等同于 `mount(8)` 中描述的 `rprivate` 挂载传播类型

  > **注意**：当 `rprivate` 传播不适用时，CRI 运行时可能会选择 `rslave` 挂载传播（即 `HostToContainer`）。已知 `cri-dockerd`（Docker）在挂载源包含 Docker 守护进程的根目录（`/var/lib/docker`）时会选择 `rslave` 挂载传播。

- **`HostToContainer`**

  - 此卷挂载将接收挂载到此卷或其任何子目录的所有后续挂载
  - 如果主机在卷挂载内部挂载任何内容，容器将在那里看到它已挂载
  - 如果任何具有 `Bidirectional` 挂载传播到同一卷的 Pod 在那里挂载任何内容，具有 `HostToContainer` 挂载传播的容器将看到它
  - 等同于 `mount(8)` 中描述的 `rslave` 挂载传播

- **`Bidirectional`**

  - 此卷挂载的行为与 `HostToContainer` 挂载相同
  - 容器创建的所有卷挂载将传播回主机和使用相同卷的所有 Pod 的所有容器
  - 典型用例：使用 `FlexVolume` 或 `CSI` 驱动程序的 Pod，或需要使用 `hostPath` 卷在主机上挂载某些内容的 Pod
  - 等同于 `mount(8)` 中描述的 `rshared` 挂载传播

  > **警告**：`Bidirectional` 挂载传播可能很危险。它可能损坏主机操作系统，因此仅在特权容器中允许。此外，Pod 中容器创建的任何卷挂载必须在终止时由容器销毁（卸载）。

说明与来源：容器传播控制参考 `Docker`、`Podman` 及 `Kubernetes` 官方文档[7,8]。

### 4.4 调试 & 排查

- 使用 `findmnt -o TARGET,PROPAGATION` 快速查看传播类型。
- 使用 `cat /proc/self/mountinfo | grep -E "(shared|slave|private|unbindable)"` 查看详细传播信息。
- 使用 `strace -e mount,umount` 跟踪挂载操作，观察传播行为。
- 使用 `nsenter -t <pid> -m` 进入目标 `namespace` 查看挂载状态。

说明与来源：调试工具参考 `findmnt(8)`、`strace(1)`、`nsenter(1)` 等手册页。

---

## 5. 总结

`Linux` 的挂载传播机制通过"共享子树"（`shared subtree`）和"挂载命名空间"（`mount namespace`）实现了灵活的挂载事件传播控制。理解 `shared`、`slave`、`private`、`unbindable` 四种传播类型及其对应的 `mount --make-*` 命令，对于容器化环境、系统管理和调试都至关重要。

- `shared`：双向传播，组内所有挂载点相互影响
- `slave`：单向传播，仅接收来自主组的传播
- `private`：不传播，完全隔离
- `unbindable`：不传播且不可绑定

该机制在容器化环境中尤为重要，正确的传播类型设置可以避免挂载泄露、递归挂载等问题。通过 `mount --make-*` 系列命令可以灵活控制传播行为，而 `/proc/self/mountinfo` 提供了详细的传播状态信息。

---

## 6. 参考资料

1. man7.org — mount_namespaces(7) - [Linux manual page](https://man7.org/linux/man-pages/man7/mount_namespaces.7.html)
2. LWN.net — [Mount namespaces and shared subtrees](https://lwn.net/Articles/689856/)
3. Linux Kernel Documentation — [Shared Subtrees](https://docs.kernel.org/filesystems/sharedsubtree.html)
4. Linux Kernel Source — Documentation/filesystems/sharedsubtree.txt - [Shared Subtree Implementation](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/filesystems/sharedsubtree.txt)
5. man7.org — mount(8) - [mount filesystem](https://man7.org/linux/man-pages/man8/mount.8.html)
6. man7.org — proc(5) - [process information pseudo-filesystem](https://man7.org/linux/man-pages/man5/proc.5.html)
7. Kubernetes Documentation — [Volumes - mountPropagation](https://kubernetes.io/docs/concepts/storage/volumes/#mount-propagation)
8. Kubernetes API Reference — [VolumeMount - mountPropagation](https://kubernetes.io/docs/reference/kubernetes-api/workload-resources/pod-v1/#volumemount)

---
