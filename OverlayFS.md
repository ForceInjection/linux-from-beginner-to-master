Introduction to the OverlayFS
=============================

The OverlayFS pseudo-filesystem was first included in the Linux kernel 3.18 release: it allows us to combine two directory trees or filesystems (an “upper” and a “lower one”) in a way that is completely transparent to the user, which is able to access files and directories on the “merged” layer just like he would do on a standard filesystem.

OverlayFS 伪文件系统首次包含在 Linux 内核 3.18 版本中：它允许我们将两个目录树或文件系统（一个“上层”和一个“下层”）以对用户完全透明的方式结合起来，用户可以像在标准文件系统上一样访问“合并”层上的文件和目录。

简介（Introduction）
------------

We all should be familiar with the standard behavior the Linux kernel adopts when mounting a filesystem: files and directories existing in the directory used as mountpoint are masked, and become unavailable to the user, while those existing on the mounted filesystem are displayed. The original files can be accessed again only when the filesystem is unmounted. This also happens when we mount multiple filesystems on the same directory. When the OverlayFS pseudo-filesystem is used, instead, files existing on the different layers are **combined**, and the resulting filesystem can be mounted itself.

我们都应该熟悉 Linux 内核在挂载文件系统时采用的标准行为：存在于用作挂载点的目录中的文件和目录被屏蔽，用户无法访问，而存在于挂载文件系统中的文件和目录则被显示。只有当文件系统被卸载后，原始文件才能再次被访问。当我们在同一个目录上挂载多个文件系统时，也会发生这种情况。当使用 OverlayFS 伪文件系统时，相反，不同层上的文件被合并，生成的文件系统可以自己挂载。

The OverlayFS is typically used on systems running on embed devices, like [OpenWRT,](https://openwrt.org/) where is useful to preserve a basic set of configurations and at the same time allowing the user to perform modifications. OverlayFS is also at the base of the “overlay” and “overlay2” Docker storage drivers. Let’s see what is the main logic behind it.

OverlayFS 通常用于运行在嵌入式设备上的系统，如 [OpenWRT,](https://openwrt.org/)，在这里它有助于保留一组基本配置，同时允许用户进行修改。OverlayFS 也是 Docker 的“overlay”和“overlay2”存储驱动程序的基础。让我们看看它背后的主要逻辑。

OverlayFS：基本概念（OverlayFS: basic concepts）
-------------------------

Two layers are involved in the working of the OverlayFS: a lower and an upper one. The lower layer is typically mounted in **read only mode.** When this setup is used, since no direct changes to files and directories hosted on it are possible, it can be used as a safe fallback. The upper layer, instead, can be mounted in read/write mode. The file existing on the two layers are combined and become accessible in what we can call the “merged” layer, which can be mounted itself as a standard filesystem.

OverlayFS 的工作涉及两个层：一个**下层**和一个**上层**。下层通常以**只读模式**挂载。当使用这种设置时，由于不可能直接更改托管在其上的文件和目录，它可以用作安全的后备。相反，上层可以以读写模式挂载。存在于两个层上的文件被合并，并变得可以在我们所谓的“合并”层上访问，该层可以作为标准文件系统自己挂载。

As we can observe in the picture below, a file existing in the lower layer is “obscured” or “masked” by a file with the same name existing in the upper one. When the user modifies a file which belongs to the former, a copy of it is created in the latter, writable layer (this strategy is called: “copy-up”): this file masks the original one, in a process which is transparent to the user. A similar thing happens for directories, which are **merged,** instead:

正如我们在下图中观察到的，下层中存在的文件被上层中同名的文件“遮挡”或“屏蔽”。当用户修改属于前者的文件时，会在后者，可写层中创建它的副本（这种策略称为：“copy-up”）：这个文件遮挡了原始文件，这个过程对用户是透明的。对于目录，也会发生类似的事情，它们被合并：

![The diagram illustrates how the OverlayFS works](https://linuxconfig.org/wp-content/uploads/2022/09/01-introduction-to-the-overlayfs.avif)

上图说明了 OverlayFS 的工作原理（The diagram illustrates how the OverlayFS works——

What happens when **delete** a file or a directory? If the file we delete belongs to the upper layer, it is deleted in place; if it belongs to the lower layer, instead, the deletion is simulated by using a **whiteout file** (or an opaque directory – a directory with the `trusted.overlay.opaque` extended attribute), which is created in the writable layer and obscures the original element.

当**删除**文件或目录时会发生什么？如果我们删除的文件属于上层，它将被就地删除；如果它属于下层，相反，删除将通过在可写层中创建一个**白出文件**（或一个不透明目录——一个带有 `trusted.overlay.opaque` 扩展属性的目录）来模拟，它遮挡了原始元素。

The OverlayFS is the base of the Docker **overlay** and **overlay2** drivers. On such implementation the lower, read-only layer is represented by **images**; the writable, upper layer, instead, is represented by the **containers** based on them. Images are immutable: all changes happen inside containers and are lost when containers are deleted (that is why volumes are used for persistence):

OverlayFS 是 Docker **overlay** 和 **overlay2** 驱动程序的基础。在这种实现中，下层，只读层由镜像表示；相反，可写层，上层由基于它们的容器表示。镜像是不可变的：所有更改都发生在容器内，并且当容器被删除时会丢失（这就是为什么使用卷进行持久性的原因）：

![Docker and OverlayFS constructs](https://docs.docker.com/engine/storage/drivers/images/overlay_constructs.webp)

Docker 和 OverlayFS 构建（Docker and OverlayFS constructs）

OverlayFS 使用方式（OverlayFS usage）
---------------

Let’s see how to use the OverlayFS. For the sake of this example I will assume we want to combine two filesystems: the lower one existing on the `/dev/sda1` partition, and the one to be used in read-write mode to be on the `/dev/sda2` partition. The first thing we want to do, is to create the directories which we will use as mount points:

让我们看看如何使用 OverlayFS。为了这个例子，我假设我们想要结合两个文件系统：一个存在于 `/dev/sda1` 分区上的下层，以及一个用于读写模式的上层，它位于 `/dev/sda2` 分区上。我们首先要做的就是创建我们将用作挂载点的目录：

```bash
$ sudo mkdir /lower /overlay
```

Now, let’s mount the `/dev/sda1` filesystem on the `/lower` directory, in read-only mode:

现在，让我们将 `/dev/sda1` 文件系统以只读模式挂载到 /lower 目录上：

```bash
$ sudo mount -o ro /dev/sda1 /lower
```

The `ls` command reveals the filesystem contains just one file:

`ls` 命令显示文件系统只包含一个文件：

```bash
$ ls -l /lower
total 20
-rw-r--r--. 1 root root 23 Sep 1 10:43 file1.txt
drwx------. 2 root root 16384 Sep 1 10:40 lost+foun
```

The files contain just one line:

文件只包含一行：

```bash
$ cat /lower/file1.txt 
this is the first line
```

Now, let’s proceed further. As a next step, we mount the `/dev/sda2` filesystem on the `/overlay` directory:

现在，让我们继续。作为下一步，我们将 `/dev/sda2` 文件系统挂载到 `/overlay` 目录上：

```bash
$ sudo mount /dev/sda2 /overlay
```

Once the filesystem is mounted, we create two directories on it: `upper` and `work`. The former will host the files which are part of the upper layer, the latter will be used internally to prepare files when they are switched from one layer to the other: **it must be empty and on the same filesystem of the upper one**:

一旦文件系统挂载，我们在它上面创建两个目录：`upper` 和 `work`。前者将托管上层文件，后者将被内部用于准备文件，当它们从一层切换到另一层时：**它必须是空的，并且位于上层相同的文件系统上**：

```bash
$ sudo mkdir /overlay/{upper,work}
```

Now we can “assemble” and mount the overlay. To accomplish the task, we use the following command:

现在我们可以“组装”并挂载覆盖层。为了完成这项任务，我们使用以下命令：

```bash
$ sudo mount overlay -t overlay -o lowerdir=/lower,upperdir=/overlay/upper,workdir=/overlay/work  /media
```

We invoked `mount` passing “overlay” as argument to the `-t` option (short for `--types`), thus specifying the type of filesystem we want to mount (a pseudo filesystem in this case), than we used `-o` flag to list the mount options: in this case “lowerdir”, “upperdir” and “workdir” to specify: the directory on which the read-only filesystem is mounted, the directory hosting the files of the upper, writable layer, and the location of the “work” directory, respectively. Finally we specified the mountpoint for the “merged” filesystem: `/media`, in this case.

我们调用 `mount` 传递“overlay”作为 `-t` 选项（即 `--types`的缩写），从而指定我们想要挂载的文件系统类型（在这种情况下是一个伪文件系统），然后我们使用 `-o` 标志列出挂载选项：在这种情况下是“lowerdir”，“upperdir”和“workdir”以指定：在其中挂载只读文件系统的目录，托管上层可写层文件的目录，以及“work”目录的位置。最后我们指定了“合并”文件系统的挂载点：在这种情况下是 `/media`。

We can see a summary of the overlay setup using the `mount` command:

我们可以使用 `mount` 命令查看覆盖层设置的摘要：

```bash
$ mount | grep -i overlay
overlay on /media type overlay (rw,relatime,seclabel,lowerdir=/lower,upperdir=/overlay/upper,workdir=/overlay/work)
```

If we list the files in the `/media` directory we see only the `file1.txt` exists, which, as we know, belongs to the lower layer:

如果我们列出 `/media` 目录中的文件，我们只会看到 `file1.txt `存在，正如我们所知，它属于下层：

```bash
$ ls -l /media
total 20
-rw-r--r--. 1 root root 23 Sep 1 10:43 file1.txt
drwx------. 2 root root 16384 Sep 1 10:40 lost+found
```

Now let’s try to add a line to the file and see what happens:

现在让我们尝试向文件中添加一行，看看会发生什么：

```bash
$ echo "this is the second line" | sudo tee -a /media/file1.txt
```

If we inspect the content of the file, we can see the line was added successfully:

如果我们检查文件的内容，我们可以看到该行已被成功添加：

```bash
$ cat /media/file1.txt
this is the first line
this is the second line
```

The original `file1.txt` file, however, was not altered:

然而，原始的 ·file1.txt· 文件并未更改：

```bash
$ cat /lower/file1.txt
this is the first line
```

Instead, a file with the same name was added in the upper layer:

相反，在上层中添加了同名的文件：

```bash
$ ls -l /overlay/upper
-rw-r--r--. 1 root root 47 Sep 1 14:36 file1.txt
```

Now, if we delete the `/media/file1.txt` file,`/overlay/upper/file1.txt` will become a whiteout file:

现在，如果我们删除 `/media/file1.txt` 文件，`/overlay/upper/file1.txt` 将变成一个白出文件：

```bash
$ sudo rm /media/file1.txt
$ ls -l /overlay/upper
**c**\---------. 2 root root 0, 0 Sep 1 14:45 file1.txt
```

As stated in the [official documentation](https://docs.kernel.org/filesystems/overlayfs.html), a whiteout file is a **character device** (this is reflected in the output of ls – see the highlighted leading “c”) with the 0/0 device number.

正如[官方文档](https://docs.kernel.org/filesystems/overlayfs.html)中所述，白出文件是**一个字符设备**（这在 ls 的输出中反映出来——看到突出显示的“c”），具有 0/0 设备号。

使用 OverlayFS 存储驱动程序（Use the OverlayFS storage driver）
================================

Docker has a storage driver called `overlay2`.

Docker 有一种叫做 `overlay2` 的存储驱动。

> **注意（Note）**
> 
> 对于 `fuse-overlayfs 驱动程序`，请查看无根模式文档）（For `fuse-overlayfs` driver, check [Rootless mode documentation](https://docs.docker.com/engine/security/rootless/)）。

前置条件（Prerequisites）
-------------------------------

OverlayFS is the recommended storage driver, and supported if you meet the following prerequisites:

*   Version 4.0 or higher of the Linux kernel, or RHEL or CentOS using version 3.10.0-514 of the kernel or higher.
    
*   The `overlay2` driver is supported on `xfs` backing filesystems, but only with `d_type=true` enabled.
    
    Use `xfs_info` to verify that the `ftype` option is set to `1`. To format an `xfs` filesystem correctly, use the flag `-n ftype=1`.
    
*   Changing the storage driver makes existing containers and images inaccessible on the local system. Use `docker save` to save any images you have built or push them to Docker Hub or a private registry before changing the storage driver, so that you don't need to re-create them later.
    

Configure Docker with the `overlay2` storage driver
---------------------------------------------------------------------------------------------------------

Before following this procedure, you must first meet all the prerequisites.

The following steps outline how to configure the `overlay2` storage driver.

1.  Stop Docker.
    ```bash
        $ sudo systemctl stop docker
    ```
    
2.  Copy the contents of `/var/lib/docker` to a temporary location.
    ```bash
        $ cp -au /var/lib/docker /var/lib/docker.bk
    ```    
    
3.  If you want to use a separate backing filesystem from the one used by `/var/lib/`, format the filesystem and mount it into `/var/lib/docker`. Make sure to add this mount to `/etc/fstab` to make it permanent.
    
4.  Edit `/etc/docker/daemon.json`. If it doesn't yet exist, create it. Assuming that the file was empty, add the following contents.
    ```yaml
    
        {
          "storage-driver": "overlay2"
        }
    ```
    Docker doesn't start if the `daemon.json` file contains invalid JSON.
    
5.  Start Docker.
    ```bash
        $ sudo systemctl start docker
    ```    
    
6.  Verify that the daemon is using the `overlay2` storage driver. Use the `docker info` command and look for `Storage Driver` and `Backing filesystem`.
    ```bash
        $ docker info
        
        Containers: 0
        Images: 0
        Storage Driver: overlay2
         Backing Filesystem: xfs
         Supports d_type: true
         Native Overlay Diff: true
        <...>
    ```    
    

Docker is now using the `overlay2` storage driver and has automatically created the overlay mount with the required `lowerdir`, `upperdir`, `merged`, and `workdir` constructs.

Continue reading for details about how OverlayFS works within your Docker containers, as well as performance advice and information about limitations of its compatibility with different backing filesystems.

How the `overlay2` driver works
-----------------------------------------------------------------

OverlayFS layers two directories on a single Linux host and presents them as a single directory. These directories are called layers, and the unification process is referred to as a union mount. OverlayFS refers to the lower directory as `lowerdir` and the upper directory a `upperdir`. The unified view is exposed through its own directory called `merged`.

The `overlay2` driver natively supports up to 128 lower OverlayFS layers. This capability provides better performance for layer-related Docker commands such as `docker build` and `docker commit`, and consumes fewer inodes on the backing filesystem.

### Image and container layers on-disk

After downloading a five-layer image using `docker pull ubuntu`, you can see six directories under `/var/lib/docker/overlay2`.

> **Warning**
> 
> Don't directly manipulate any files or directories within `/var/lib/docker/`. These files and directories are managed by Docker.

```bash

    $ ls -l /var/lib/docker/overlay2
    
    total 24
    drwx------ 5 root root 4096 Jun 20 07:36 223c2864175491657d238e2664251df13b63adb8d050924fd1bfcdb278b866f7
    drwx------ 3 root root 4096 Jun 20 07:36 3a36935c9df35472229c57f4a27105a136f5e4dbef0f87905b2e506e494e348b
    drwx------ 5 root root 4096 Jun 20 07:36 4e9fa83caff3e8f4cc83693fa407a4a9fac9573deaf481506c102d484dd1e6a1
    drwx------ 5 root root 4096 Jun 20 07:36 e8876a226237217ec61c4baf238a32992291d059fdac95ed6303bdff3f59cff5
    drwx------ 5 root root 4096 Jun 20 07:36 eca1e4e1694283e001f200a667bb3cb40853cf2d1b12c29feda7422fed78afed
    drwx------ 2 root root 4096 Jun 20 07:36 l
```    

The new `l` (lowercase `L`) directory contains shortened layer identifiers as symbolic links. These identifiers are used to avoid hitting the page size limitation on arguments to the `mount` command.

```bash
    $ ls -l /var/lib/docker/overlay2/l
    
    total 20
    lrwxrwxrwx 1 root root 72 Jun 20 07:36 6Y5IM2XC7TSNIJZZFLJCS6I4I4 -> ../3a36935c9df35472229c57f4a27105a136f5e4dbef0f87905b2e506e494e348b/diff
    lrwxrwxrwx 1 root root 72 Jun 20 07:36 B3WWEFKBG3PLLV737KZFIASSW7 -> ../4e9fa83caff3e8f4cc83693fa407a4a9fac9573deaf481506c102d484dd1e6a1/diff
    lrwxrwxrwx 1 root root 72 Jun 20 07:36 JEYMODZYFCZFYSDABYXD5MF6YO -> ../eca1e4e1694283e001f200a667bb3cb40853cf2d1b12c29feda7422fed78afed/diff
    lrwxrwxrwx 1 root root 72 Jun 20 07:36 NFYKDW6APBCCUCTOUSYDH4DXAT -> ../223c2864175491657d238e2664251df13b63adb8d050924fd1bfcdb278b866f7/diff
    lrwxrwxrwx 1 root root 72 Jun 20 07:36 UL2MW33MSE3Q5VYIKBRN4ZAGQP -> ../e8876a226237217ec61c4baf238a32992291d059fdac95ed6303bdff3f59cff5/diff
```    

The lowest layer contains a file called `link`, which contains the name of the shortened identifier, and a directory called `diff` which contains the layer's contents.

```bash
    $ ls /var/lib/docker/overlay2/3a36935c9df35472229c57f4a27105a136f5e4dbef0f87905b2e506e494e348b/
    
    diff  link
    
    $ cat /var/lib/docker/overlay2/3a36935c9df35472229c57f4a27105a136f5e4dbef0f87905b2e506e494e348b/link
    
    6Y5IM2XC7TSNIJZZFLJCS6I4I4
    
    $ ls  /var/lib/docker/overlay2/3a36935c9df35472229c57f4a27105a136f5e4dbef0f87905b2e506e494e348b/diff
    
    bin  boot  dev  etc  home  lib  lib64  media  mnt  opt  proc  root  run  sbin  srv  sys  tmp  usr  var
```    

The second-lowest layer, and each higher layer, contain a file called `lower`, which denotes its parent, and a directory called `diff` which contains its contents. It also contains a `merged` directory, which contains the unified contents of its parent layer and itself, and a `work` directory which is used internally by OverlayFS.

```bash
    $ ls /var/lib/docker/overlay2/223c2864175491657d238e2664251df13b63adb8d050924fd1bfcdb278b866f7
    
    diff  link  lower  merged  work
    
    $ cat /var/lib/docker/overlay2/223c2864175491657d238e2664251df13b63adb8d050924fd1bfcdb278b866f7/lower
    
    l/6Y5IM2XC7TSNIJZZFLJCS6I4I4
    
    $ ls /var/lib/docker/overlay2/223c2864175491657d238e2664251df13b63adb8d050924fd1bfcdb278b866f7/diff/
    
    etc  sbin  usr  var
```    

To view the mounts which exist when you use the `overlay` storage driver with Docker, use the `mount` command. The output below is truncated for readability.

```bash
    $ mount | grep overlay
    
    overlay on /var/lib/docker/overlay2/9186877cdf386d0a3b016149cf30c208f326dca307529e646afce5b3f83f5304/merged
    type overlay (rw,relatime,
    lowerdir=l/DJA75GUWHWG7EWICFYX54FIOVT:l/B3WWEFKBG3PLLV737KZFIASSW7:l/JEYMODZYFCZFYSDABYXD5MF6YO:l/UL2MW33MSE3Q5VYIKBRN4ZAGQP:l/NFYKDW6APBCCUCTOUSYDH4DXAT:l/6Y5IM2XC7TSNIJZZFLJCS6I4I4,
    upperdir=9186877cdf386d0a3b016149cf30c208f326dca307529e646afce5b3f83f5304/diff,
    workdir=9186877cdf386d0a3b016149cf30c208f326dca307529e646afce5b3f83f5304/work)
```    

The `rw` on the second line shows that the `overlay` mount is read-write.

The following diagram shows how a Docker image and a Docker container are layered. The image layer is the `lowerdir` and the container layer is the `upperdir`. If the image has multiple layers, multiple `lowerdir` directories are used. The unified view is exposed through a directory called `merged` which is effectively the containers mount point.

![How Docker constructs map to OverlayFS constructs](https://docs.docker.com/engine/storage/drivers/images/overlay_constructs.webp)

Where the image layer and the container layer contain the same files, the container layer (`upperdir`) takes precedence and obscures the existence of the same files in the image layer.

To create a container, the `overlay2` driver combines the directory representing the image's top layer plus a new directory for the container. The image's layers are the `lowerdirs` in the overlay and are read-only. The new directory for the container is the `upperdir` and is writable.

### Image and container layers on-disk

The following `docker pull` command shows a Docker host downloading a Docker image comprising five layers.

```bash
    $ docker pull ubuntu
    
    Using default tag: latest
    latest: Pulling from library/ubuntu
    
    5ba4f30e5bea: Pull complete
    9d7d19c9dc56: Pull complete
    ac6ad7efd0f9: Pull complete
    e7491a747824: Pull complete
    a3ed95caeb02: Pull complete
    Digest: sha256:46fb5d001b88ad904c5c732b086b596b92cfb4a4840a3abd0e35dbb6870585e4
    Status: Downloaded newer image for ubuntu:latest
```    

#### The image layers

Each image layer has its own directory within `/var/lib/docker/overlay/`, which contains its contents, as shown in the following example. The image layer IDs don't correspond to the directory IDs.

> **Warning**
> 
> Don't directly manipulate any files or directories within `/var/lib/docker/`. These files and directories are managed by Docker.

```bash
    $ ls -l /var/lib/docker/overlay/
    
    total 20
    drwx------ 3 root root 4096 Jun 20 16:11 38f3ed2eac129654acef11c32670b534670c3a06e483fce313d72e3e0a15baa8
    drwx------ 3 root root 4096 Jun 20 16:11 55f1e14c361b90570df46371b20ce6d480c434981cbda5fd68c6ff61aa0a5358
    drwx------ 3 root root 4096 Jun 20 16:11 824c8a961a4f5e8fe4f4243dab57c5be798e7fd195f6d88ab06aea92ba931654
    drwx------ 3 root root 4096 Jun 20 16:11 ad0fe55125ebf599da124da175174a4b8c1878afe6907bf7c78570341f308461
    drwx------ 3 root root 4096 Jun 20 16:11 edab9b5e5bf73f2997524eebeac1de4cf9c8b904fa8ad3ec43b3504196aa3801
```   

The image layer directories contain the files unique to that layer as well as hard links to the data shared with lower layers. This allows for efficient use of disk space.

```bash
    $ ls -i /var/lib/docker/overlay2/38f3ed2eac129654acef11c32670b534670c3a06e483fce313d72e3e0a15baa8/root/bin/ls
    
    19793696 /var/lib/docker/overlay2/38f3ed2eac129654acef11c32670b534670c3a06e483fce313d72e3e0a15baa8/root/bin/ls
    
    $ ls -i /var/lib/docker/overlay2/55f1e14c361b90570df46371b20ce6d480c434981cbda5fd68c6ff61aa0a5358/root/bin/ls
    
    19793696 /var/lib/docker/overlay2/55f1e14c361b90570df46371b20ce6d480c434981cbda5fd68c6ff61aa0a5358/root/bin/ls
```    

#### The container layer

Containers also exist on-disk in the Docker host's filesystem under `/var/lib/docker/overlay/`. If you list a running container's subdirectory using the `ls -l` command, three directories and one file exist:

```bash
    $ ls -l /var/lib/docker/overlay2/<directory-of-running-container>
    
    total 16
    -rw-r--r-- 1 root root   64 Jun 20 16:39 lower-id
    drwxr-xr-x 1 root root 4096 Jun 20 16:39 merged
    drwxr-xr-x 4 root root 4096 Jun 20 16:39 upper
    drwx------ 3 root root 4096 Jun 20 16:39 work
```    

The `lower-id` file contains the ID of the top layer of the image the container is based on, which is the OverlayFS `lowerdir`.

```bash
    $ cat /var/lib/docker/overlay2/ec444863a55a9f1ca2df72223d459c5d940a721b2288ff86a3f27be28b53be6c/lower-id
    
    55f1e14c361b90570df46371b20ce6d480c434981cbda5fd68c6ff61aa0a5358
```    

The `upper` directory contains the contents of the container's read-write layer, which corresponds to the OverlayFS `upperdir`.

The `merged` directory is the union mount of the `lowerdir` and `upperdirs`, which comprises the view of the filesystem from within the running container.

The `work` directory is internal to OverlayFS.

To view the mounts which exist when you use the `overlay2` storage driver with Docker, use the `mount` command. The following output is truncated for readability.

```bash
    $ mount | grep overlay
    
    overlay on /var/lib/docker/overlay2/l/ec444863a55a.../merged
    type overlay (rw,relatime,lowerdir=/var/lib/docker/overlay2/l/55f1e14c361b.../root,
    upperdir=/var/lib/docker/overlay2/l/ec444863a55a.../upper,
    workdir=/var/lib/docker/overlay2/l/ec444863a55a.../work)
```    

The `rw` on the second line shows that the `overlay` mount is read-write.

How container reads and writes work with `overlay2`
---------------------------------------------------------------------------------------------------------

### Reading files

Consider three scenarios where a container opens a file for read access with overlay.

#### The file does not exist in the container layer

If a container opens a file for read access and the file does not already exist in the container (`upperdir`) it is read from the image (`lowerdir`). This incurs very little performance overhead.

#### The file only exists in the container layer

If a container opens a file for read access and the file exists in the container (`upperdir`) and not in the image (`lowerdir`), it's read directly from the container.

#### The file exists in both the container layer and the image layer

If a container opens a file for read access and the file exists in the image layer and the container layer, the file's version in the container layer is read. Files in the container layer (`upperdir`) obscure files with the same name in the image layer (`lowerdir`).

### Modifying files or directories

Consider some scenarios where files in a container are modified.

#### Writing to a file for the first time

The first time a container writes to an existing file, that file does not exist in the container (`upperdir`). The `overlay2` driver performs a `copy_up` operation to copy the file from the image (`lowerdir`) to the container (`upperdir`). The container then writes the changes to the new copy of the file in the container layer.

However, OverlayFS works at the file level rather than the block level. This means that all OverlayFS `copy_up` operations copy the entire file, even if the file is large and only a small part of it's being modified. This can have a noticeable impact on container write performance. However, two things are worth noting:

*   The `copy_up` operation only occurs the first time a given file is written to. Subsequent writes to the same file operate against the copy of the file already copied up to the container.
    
*   OverlayFS works with multiple layers. This means that performance can be impacted when searching for files in images with many layers.
    

#### Deleting files and directories

*   When a _file_ is deleted within a container, a _whiteout_ file is created in the container (`upperdir`). The version of the file in the image layer (`lowerdir`) is not deleted (because the `lowerdir` is read-only). However, the whiteout file prevents it from being available to the container.
    
*   When a _directory_ is deleted within a container, an _opaque directory_ is created within the container (`upperdir`). This works in the same way as a whiteout file and effectively prevents the directory from being accessed, even though it still exists in the image (`lowerdir`).
    

#### Renaming directories

Calling `rename(2)` for a directory is allowed only when both the source and the destination path are on the top layer. Otherwise, it returns `EXDEV` error ("cross-device link not permitted"). Your application needs to be designed to handle `EXDEV` and fall back to a "copy and unlink" strategy.

OverlayFS and Docker Performance
---------------------------------------------------------------------

`overlay2` may perform better than `btrfs`. However, be aware of the following details:

### Page caching

OverlayFS supports page cache sharing. Multiple containers accessing the same file share a single page cache entry for that file. This makes the `overlay2` drivers efficient with memory and a good option for high-density use cases such as PaaS.

### Copyup

As with other copy-on-write filesystems, OverlayFS performs copy-up operations whenever a container writes to a file for the first time. This can add latency into the write operation, especially for large files. However, once the file has been copied up, all subsequent writes to that file occur in the upper layer, without the need for further copy-up operations.

### Performance best practices

The following generic performance best practices apply to OverlayFS.

#### Use fast storage

Solid-state drives (SSDs) provide faster reads and writes than spinning disks.

#### Use volumes for write-heavy workloads

Volumes provide the best and most predictable performance for write-heavy workloads. This is because they bypass the storage driver and don't incur any of the potential overheads introduced by thin provisioning and copy-on-write. Volumes have other benefits, such as allowing you to share data among containers and persisting your data even if no running container is using them.

Limitations on OverlayFS compatibility
---------------------------------------------------------------------------------

To summarize the OverlayFS's aspect which is incompatible with other filesystems:

[`open(2)`](https://linux.die.net/man/2/open)

OverlayFS only implements a subset of the POSIX standards. This can result in certain OverlayFS operations breaking POSIX standards. One such operation is the copy-up operation. Suppose that your application calls `fd1=open("foo", O_RDONLY)` and then `fd2=open("foo", O_RDWR)`. In this case, your application expects `fd1` and `fd2` to refer to the same file. However, due to a copy-up operation that occurs after the second calling to `open(2)`, the descriptors refer to different files. The `fd1` continues to reference the file in the image (`lowerdir`) and the `fd2` references the file in the container (`upperdir`). A workaround for this is to `touch` the files which causes the copy-up operation to happen. All subsequent `open(2)` operations regardless of read-only or read-write access mode reference the file in the container (`upperdir`).

`yum` is known to be affected unless the `yum-plugin-ovl` package is installed. If the `yum-plugin-ovl` package is not available in your distribution such as RHEL/CentOS prior to 6.8 or 7.2, you may need to run `touch /var/lib/rpm/*` before running `yum install`. This package implements the `touch` workaround referenced above for `yum`.

[`rename(2)`](https://linux.die.net/man/2/rename)

OverlayFS does not fully support the `rename(2)` system call. Your application needs to detect its failure and fall back to a "copy and unlink" strategy.