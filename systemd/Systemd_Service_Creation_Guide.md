
# 如何在 Linux 中创建 Systemd 服务

本文介绍了在 Linux 系统中创建和管理 Systemd 服务的详细步骤。我们以 shell 脚本为例做一个简单的说明。

## 第一步：创建服务脚本

1. 在 `/usr/local/bin/` 或任何位置创建一个可执行脚本。例如，创建一个名为 `my_service.sh` 的脚本：

    ```bash
    sudo nano /usr/local/bin/my_service.sh
    ```

2. 在脚本中添加相关命令，并使用 `while true` 循环和 `sleep` 命令来实现脚本持续运行，例如：

    ```bash
    #!/bin/bash

    while true; do
        echo "Test Service is running"
        sleep 5  # 休眠 5 秒
    done
    ```

    这个脚本将不断输出 "Test Service is running"，每隔 5 秒重复一次。

3. 保存并退出编辑器，然后赋予脚本执行权限：

    ```bash
    sudo chmod +x /usr/local/bin/my_service.sh
    ```

## 第二步：创建 Systemd 单元文件

1. 在 `/etc/systemd/system/` 目录下创建一个新的服务文件。例如，创建一个名为 `my_service.service` 的文件：

    ```bash
    sudo nano /etc/systemd/system/my_service.service
    ```

2. 在文件中定义服务内容，格式如下：

    ```ini
    [Unit]
    Description=My Custom Service
    After=network.target

    [Service]
    ExecStart=/usr/local/bin/my_service.sh
    Restart=on-failure
    StandardOutput=journal
    StandardError=journal

    [Install]
    WantedBy=multi-user.target
    ```

### `[Unit]` 部分

- **`Description=My Custom Service`**  
  这行描述了服务的名称或功能。这是一个简短的文本描述，用于说明服务的用途或它所执行的任务。当使用 `systemctl status` 或 `journalctl` 时，这个描述会显示在输出中。

- **`After=network.target`**  
  这个选项指定服务应在特定的目标或服务之后启动。在这个例子中，`network.target` 是一个特殊的目标，表示网络已初始化。当服务依赖网络时，设置 `After=network.target` 可确保网络在服务启动前已就绪。注意，这只是表示顺序，而不是依赖关系，如果需要确保依赖关系可以使用 `Requires=` 指令。

### `[Service]` 部分

- **`ExecStart=/usr/local/bin/my_service.sh`**  
  这个选项定义了服务启动时执行的命令或脚本。在这个例子中，它指定了 `/usr/local/bin/my_service.sh` 脚本作为服务的主进程。每当我们启动服务时，Systemd 就会运行这个脚本。

- **`Restart=on-failure`**  
  这个选项定义了服务在何种情况下会自动重启。`on-failure` 表示如果服务由于非正常退出（非 0 返回码、被信号终止等）而失败，Systemd 会自动尝试重启服务。其他可选值包括 `always`（无论退出状态如何，始终重启），`on-abort`（只有在进程由于信号被杀死时重启），`no`（从不重启）等。

- **`StandardOutput=journal` 和 `StandardError=journal`**  
  这两个选项指定服务的标准输出和标准错误应记录到 Systemd 的日志系统（`journald`）中。这意味着任何通过 `echo`、`printf` 或其他输出命令生成的日志信息都会被记录到 Systemd 日志中，并可以通过 `journalctl` 查看。

### `[Install]` 部分

- **`WantedBy=multi-user.target`**  
  这个选项指定服务应在哪些目标（target）下启动。`multi-user.target` 是一个常见的运行级别目标，类似于传统的运行级别 3（多用户模式，没有图形界面）。将服务链接到 `multi-user.target` 使得它在系统进入多用户模式时自动启动。我们可以通过 `systemctl enable` 命令将服务添加到 `multi-user.target` 中，使其在系统引导时自动启动。

## 第三步：启动和管理服务

1. 重新加载 Systemd 配置，以便系统识别新的服务：

    ```bash
    sudo systemctl daemon-reload
    ```

2. 启动服务：

    ```bash
    sudo systemctl start my_service.service
    
    systemctl status my_service.service
	● my_service.service - My Custom Service
	   Loaded: loaded (/etc/systemd/system/my_service.service; disabled; vendor preset: disabled)
	   Active: active (running) since Fri 2024-08-23 14:06:13 CST; 7s ago
	 Main PID: 3199183 (my_service.sh)
	    Tasks: 2
	   Memory: 276.0K
	   CGroup: /system.slice/my_service.service
	           ├─3199183 /bin/bash /home/grissom/my_service.sh
	           └─3199356 sleep 5
	 
	Aug 23 14:06:13 tdc-55 systemd[1]: Started My Custom Service.
	Aug 23 14:06:13 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:18 tdc-55 my_service.sh[3199183]: Test Service is running...
    ```

3. 要使服务在系统启动时自动启动，可以启用它：

    ```bash
    sudo systemctl enable my_service.service
    ```

## 第四步：检查服务状态和日志

1. 检查服务的状态以确认其是否正常运行：

    ```bash
    sudo systemctl status my_service.service
    ```

2. 如果需要停止或重启服务，可以使用以下命令：

    - 停止服务：

        ```bash
        sudo systemctl stop my_service.service
        ```

    - 重启服务：

        ```bash
        sudo systemctl restart my_service.service
        ```

3. 获取服务的日志：

    由于日志输出已配置为使用 Systemd 的日志管理器 `journalctl`，我们可以通过以下命令查看服务的日志：

    ```bash
    sudo journalctl -u my_service.service
    
    journalctl -u my_service.service
	-- Logs begin at Sun 2024-08-04 02:34:19 CST, end at Fri 2024-08-23 14:20:29 CST. --
	Aug 23 14:06:13 tdc-55 systemd[1]: Started My Custom Service.
	Aug 23 14:06:13 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:18 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:23 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:28 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:33 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:38 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:43 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:48 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:53 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:06:58 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:07:03 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:07:08 tdc-55 my_service.sh[3199183]: Test Service is running...
	Aug 23 14:07:13 tdc-55 my_service.sh[3199183]: Test Service is running...
    ```
	这将显示服务的输出日志，包括启动、停止以及任何错误信息。

	通过这些步骤，我们就可以在 Linux 系统中创建并管理自定义的 Systemd 服务，并使用 `journalctl` 轻松获取服务日志。
