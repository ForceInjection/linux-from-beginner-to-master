# 使用 `autofs` 实现 `mount --bind ` 自动挂载

Autofs 是一项按需挂载目录的服务，通过使用 Autofs，当我们访问 NFS/Samba 服务的时候，自动挂载到事先指定好的目录，在一段时间后（默认300秒）会自动断开，相较于直接将 `mount` 命令写入到开机启动或者 `/etc/fstab` 中的做法，Autofs 更加智能，同时也能够节约网络的带宽以及减少对服务器的资源占用。

更多参考文档：

- [Linux Kernel 文档:《autofs - how it works》](https://docs.kernel.org/filesystems/autofs.html)
- [Redhat 存储管理指南:《autofs》](https://docs.redhat.com/zh_hans/documentation/red_hat_enterprise_linux/7/html/storage_administration_guide/nfs-autofs)

## 1. 安装 `autofs`

可以通过以下命令安装：

### CentOS / RHEL：

```bash
sudo yum install autofs
```

### Ubuntu / Debian：

```bash
sudo apt-get install autofs
```

## 2. 配置 `/etc/auto.master`

`/etc/auto.master` 是 `autofs` 的主配置文件，用于定义挂载点以及挂载规则文件。

编辑 `/etc/auto.master` 文件，在其中添加以下内容：

```ini
/transwarp/hadoop /etc/auto.bind --timeout=0
```

- `/transwarp/hadoop`：这是我们希望绑定的目标目录。
- `/etc/auto.bind`：这是具体挂载规则的配置文件。
- `--timeout=0`：设置超时时间为 0，表示永远不自动卸载挂载点。

保存并退出编辑器。

## 3. 配置 `/etc/auto.bind`

接下来，创建并编辑挂载规则文件 `/etc/auto.bind`。

```bash
sudo vi /etc/auto.bind
```

在文件中添加以下内容：

```ini
disk1 -fstype=bind :/mnt/disk1
```

- `disk1`：这是目标路径 `/transwarp/hadoop/disk1` 的子目录名称。`autofs` 将在访问 `/transwarp/hadoop/disk1` 时挂载 `/mnt/disk1`。
- `-fstype=bind`：表示使用 `bind mount` 类型。
- `:/mnt/disk1`：这是要绑定的源目录路径。

保存并退出编辑器。

## 4. 启动并启用 `autofs` 服务

完成配置后，启动并启用 `autofs` 服务：

```bash
sudo systemctl restart autofs
sudo systemctl enable autofs
```

## 5. 测试 `bind mount`

配置完成后，访问 `/transwarp/hadoop/disk1` 目录时，`autofs` 将自动执行 `bind mount`，并且挂载点不会自动卸载。

可以通过以下命令验证是否成功挂载：

```bash
ls /transwarp/hadoop/disk1
```

我们应该可以看到 `/mnt/disk1` 中的内容。如果内容正确显示，则配置完成。

## 6. 验证挂载点不会自动卸载

由于我们在 `/etc/auto.master` 中设置了 `--timeout=0`，挂载点将一直保持挂载状态，不会在空闲时自动卸载。

你可以通过以下命令检查挂载点的状态：

```bash
mount | grep /transwarp/hadoop
```

确保挂载点 `/transwarp/hadoop/disk1` 一直存在。

## 7. 常见问题排查

- **如果挂载失败**，请检查：
  - `/mnt/disk1` 是否存在，并且内容可以访问。
  - `/etc/auto.master` 和 `/etc/auto.bind` 文件是否配置正确。
  - 重新启动 `autofs` 服务后，配置是否已生效。

- **如果挂载点未生效**，可以使用以下命令查看 `autofs` 服务日志：
  
  ```bash
  sudo journalctl -u autofs
  ```

  日志可以帮助诊断 `autofs` 服务的启动和挂载问题。

## 总结

通过 `autofs`，我们可以实现 `mount --bind /mnt/disk1 /transwarp/hadoop`，并通过设置 `--timeout=0` 禁止自动卸载。这样就可以确保挂载点将一直保持活动状态，如果失效消失，无需手动重新挂载。