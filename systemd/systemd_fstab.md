# 为何挂载失败？搞懂 systemd 与 fstab 的管理机制与常见坑

## 一、问题描述

在执行如下命令挂载数据盘时：

```bash
mount /dev/sdb1 /registry-data
```

命令无报错，但挂载点瞬间被自动卸载，`dmesg` 显示：

```
XFS (sdb1): Mounting V5 Filesystem
XFS (sdb1): Ending clean mount
XFS (sdb1): Unmounting Filesystem
```

确认 `/registry-data` 是存在的目录，且设备 `/dev/sdb1` 可正常挂载到其他目录。初步判断为 systemd 自动卸载行为所致。

---

## 二、快速结论

该问题由 `/etc/fstab` 中的挂载定义引发，而修改后未执行 `systemctl daemon-reload`，导致 systemd 仍引用旧的挂载配置。当手动挂载与 systemd 维护的 `.mount` 单元状态不一致时，systemd 会尝试强制执行单元定义的状态，触发自动卸载。

相关日志佐证如下：

```text
systemd[1]: Unit registry\x2ddata.mount is bound to inactive unit dev-disk-by\x2duuid-390f3864\x2de526\x2d42ab\x2d84aa\x2da46584e13642.device. Stopping, too.
systemd[1]: Unmounting /registry-data...
systemd[1]: Unmounted /registry-data.
systemd[1]: Unit registry\x2ddata.mount entered failed state.
```

---

## 三、排查过程记录

### 3.1 检查挂载点状态

```bash
ls -ld /registry-data
```

输出为正常空目录，权限合规（`drwxr-xr-x`），非挂载残留。

```bash
lsof +D /registry-data
```

确认无进程占用挂载点。

---

### 3.2 检查 fstab 配置完整性

```bash
cat /etc/fstab
```

验证条目是否包含必要字段（设备、挂载点、文件系统类型、挂载选项、备份标记）。
示例有效条目：

```fstab
/dev/sdb1 /registry-data xfs defaults,nofail 0 0
```

**常见问题**：

* 缺少文件系统类型（如 `xfs`）
* 挂载选项未定义（如 `defaults`）
* 未使用容错参数（如 `nofail`）

---

### 3.3 检查 systemd 的挂载 unit 状态

```bash
systemctl list-units --type=mount | grep registry-data
```

发现 `registry-data.mount` 状态为 `failed`（注意：systemctl 会自动处理转义字符，无需输入 `\x2d`）。

进一步确认：

```bash
journalctl -u registry-data.mount
```

示例输出：

```
... Mount process exited, code=exited, status=32/n/a  
... Failed to mount /registry-data.
```

---

### 3.4 systemd 生成逻辑确认

systemd 在启动或 reload 时，通过 `/lib/systemd/system-generators/systemd-fstab-generator`（路径可能因发行版不同）根据 `/etc/fstab` 生成 `.mount` unit（如 `/run/systemd/generator/registry\x2ddata.mount`）。

`CentOS 7.9` 上的输出：

```bash
ll /var/run/systemd/generator
...
-rw-r--r-- 1 root root 250 Apr 23 20:08 -.mount
-rw-r--r-- 1 root root 254 Apr 23 20:08 boot.mount
drwxr-xr-x 2 root root 200 Apr 23 20:08 local-fs.target.requires
-rw-r--r-- 1 root root 260 Apr 23 20:08 mnt-disk1.mount
-rw-r--r-- 1 root root 260 Apr 23 20:08 mnt-disk2.mount
-rw-r--r-- 1 root root 224 Apr 23 20:08 opt-kubernetes-data.mount
-rw-r--r-- 1 root root 218 Apr 23 20:08 registry\x2ddata.mount
-rw-r--r-- 1 root root 250 Apr 23 20:08 var-lib-docker.mount
...
```

若修改 fstab 后未 reload，会导致：

* unit 未更新；
* unit 状态不一致；
* 手动挂载时被旧 unit 恢复失败状态，触发自动卸载。

---

### 3.5 修复流程

```bash
# 1. 修改 /etc/fstab 后执行 reload
sudo systemctl daemon-reload

# 2. 重置失败状态（如有必要）
sudo systemctl reset-failed registry-data.mount

# 3. 尝试重新挂载
sudo systemctl restart registry-data.mount

# 4. 验证挂载状态
mount | grep /registry-data
systemctl status registry-data.mount

# 可选：若无需 systemd 管理，注释 fstab 条目后 reload
```

---

非常好，这是理解 systemd 挂载机制的重要一环，建议将这部分作为单独小节补充，并加强与 `.mount` 单元之间的依赖关系说明。以下是合并修改后的内容，供你直接使用或插入：

---

## 四、原理说明：systemd 管理 fstab 的机制

* `/etc/fstab` 中的挂载条目在系统启动或执行 `systemctl daemon-reexec` / `daemon-reload` 时，会被 systemd 的生成器（如 `systemd-fstab-generator`）动态转换为对应的 `.mount` 单元（unit）；
* 每个 `.mount` unit 的命名规则如下：
  * 挂载路径中的 `/`（斜杠） → 替换为 `-`（短横线）；
  * 特殊字符（如空格） → 转义为 `\x<hex>` 十六进制形式；
  * 示例：
    * `/registry-data` → `registry-data.mount`
    * `/data/with space` → `data-with\x20space.mount`
* systemd 通过这些 `.mount` unit 来接管挂载点的生命周期管理，包括自动挂载、依赖关系控制、失败恢复等；
* **关键行为机制（需特别注意）**：
 * systemd 始终以 `.mount` unit 的定义为挂载状态的“真相源（source of truth）”。
 * 当 systemd 检测到实际挂载状态与 `.mount` unit 定义不一致时，会**强制同步**：
 	- **手动挂载未定义 unit** → `被 systemd 自动卸载`
	- **unit 设定应挂载但实际未挂载** → `systemd 自动重新挂载`
	- **可能导致 `.mount` 进入 failed 状态，需结合日志分析原因**

---

## 五、附录：常用命令清单

| 目的            | 命令                                                   |
| ------------- | ---------------------------------------------------- |
| 查看挂载 unit 状态  | `systemctl status registry-data.mount`               |
| 查看挂载日志        | `journalctl -u registry-data.mount`                  |
| 强制重新解析 fstab  | `systemctl daemon-reload`                            |
| 列出所有挂载 unit   | `systemctl list-units --type=mount`                  |
| 手动挂载并检查是否被卸载  | `mount /dev/sdb1 /registry-data && sleep 5 && mount` |
| 生成的 unit 存放目录 | `ls /run/systemd/generator/registry\\x2ddata.mount`  |
| 查看挂载点是否被进程占用  | `lsof +D /registry-data`                             |

---

## 六、systemd 版本注意事项

| 参数                       | 引入版本  | 说明            |
| ------------------------ | ----- | ------------- |
| x-systemd.automount      | ≥ 208 | 支持自动挂载        |
| x-systemd.mount-timeout  | ≥ 243 | 可设置挂载超时时间（秒）  |
| nofail                   | —     | 允许挂载失败不阻塞启动流程 |
| x-systemd.device-timeout | —     | 设备等待超时时间（秒）   |

建议通过 `systemctl --version` 确认版本，或查阅发行版文档。

---

## 七、推荐配置与运维建议

1. **必选操作**

   ```fstab
   /dev/sdb1 /registry-data xfs defaults,nofail,x-systemd.device-timeout=10 0 0
   ```

   * `nofail`：防止启动时因挂载失败导致系统卡死
   * `x-systemd.device-timeout=10`：设备等待超时（秒）

2. **目录规范**

   * 挂载目录应为空目录，权限建议 `755`
   * 避免使用 `/mnt` 等系统目录，推荐 `/data` 等专用路径

3. **排障优先级**

   ```
   fstab 完整性 → systemd unit 状态 → 文件系统检查 → 硬件故障
   ```