What is inotify in Linux?
=========================
**[原文地址](https://www.scaler.com/topics/linux-inotify/)**

### 目录（Table of Contents）

1. **Overview**
2. **inotify Installation**
3. **How to Implement inotify**
4. **Example**
5. **Conclusion**

<!-- 重新开始编号 -->
1. **概述**
2. **inotify 安装**
3. **如何实现 inotify**
4. **示例**
5. **结论**

概述（Overview）
--------

Inotify is a Linux kernel subsystem that provides file and directory monitoring capabilities. It enables applications to receive notifications about diverse events occurring in the file system, including file modifications, creations, deletions, and attribute changes. Inotify does real-time monitoring of file system activities, making it a valuable tool for many applications and system utilities.

inotify 是 Linux 内核的一个子系统，提供了文件和目录监控的能力。它使应用程序能够接收到文件系统中发生的各种事件的通知，包括文件修改、创建、删除和属性变化。inotify对文件系统活动进行实时监控，使其成为许多应用程序和系统实用程序的宝贵工具。

The vital components of Linux Inotify are as follows:

Linux inotify 的关键组件如下：

1.  **Inotify Instance**: To use Inotify, an application first creates an instance of Inotify by calling the inotify\_init system call. It returns a file descriptor that represents the Inotify instance.
2.  **Watch Descriptors**: After creating the Inotify instance, an application can add watches to monitor specific files or directories. Calling the inotify\_add\_watch system call retrieves a watch descriptor. This descriptor represents a watch. The watch descriptor is linked to both the Inotify instance and the file or directory under monitoring.
3.  **Events**: Inotify notifies about events happening on the files or directories being watched. Events can include modifications, creations, deletions, attribute changes, and more. Each event is represented by a data structure containing relevant information such as the event type, the name of the file or directory, and additional data if needed.
4.  **Reading Events**: To retrieve events, an application reads from the file descriptor associated with the Inotify instance. The read system call is used to obtain the event data. Multiple events can be read in a single read call, as they are stored in a buffer within the kernel.
5.  **Handling Events**: Once an application has read the events, it can process them accordingly. It may involve performing specific actions based on the event type, such as updating a file, sending notifications, or triggering other operations. The application can differentiate between different event types using the information provided by the event data structure.


<!-- 重新开始编号 -->
1. **inotify 实例**：要使用 inotify，应用程序首先通过调用 inotify_init 系统调用来创建一个 inotify 实例。它返回一个代表 inotify实例的文件描述符。
2. **监视描述符**：创建 inotify 实例后，应用程序可以添加监视来监控特定的文件或目录。调用 inotify_add_watch 系统调用会获取一个监视描述符。此描述符代表一个监视，并与 inotify 实例和被监视的文件或目录相关联。
3. **事件**：inotify 会通知监视的文件或目录上发生的事件。事件可以包括修改、创建、删除、属性变化等。每个事件都由一个数据结构表示，包含相关的信息，如事件类型、文件或目录的名称以及需要的附加数据。
4. **读取事件**：为了检索事件，应用程序从与 inotify 实例关联的文件描述符中读取。使用 read 系统调用来获取事件数据。可以在单个 read 调用来读取多个事件，因为它们存储在内核中的缓冲区里。
5. **处理事件**：应用程序读取事件后，可以相应地处理它们。它可能涉及根据事件类型执行特定操作，如更新文件、发送通知或触发其他操作。应用程序可以使用事件数据结构提供的信息来区分不同的事件类型。

Linux Inotify provides a flexible and powerful mechanism for monitoring file system events, allowing applications to respond to real-time changes. It is used in various domains, including system monitoring, synchronization, and automation.

Linux inotify 提供了一个灵活而强大的机制来监控文件系统事件，允许应用程序实时响应变化。它在包括系统监控、同步和自动化在内的各个领域中都有应用。

### 它是如何工作的？（How Does It Work?）

Inotify operates based on a simple and efficient mechanism within the Linux kernel. Here is an overview of how it works:

inotify 基于 Linux 内核中的一个简单而高效的机制运行。以下是它的工作原理概述：

1.  **Initialization**: When an application creates an instance of Inotify using the inotify\_init system call, the Linux kernel sets up the necessary data structures and resources to manage the Inotify subsystem. This initialization returns a file descriptor representing the Inotify instance.
2.  **Watch Registration**: Once you establish the Inotify instance, applications can add watches to monitor specific files or directories of interest. The application specifies the Inotify instance's file descriptor, the path to the file or directory to be watched, and the types of events to be monitored by calling the inotify\_add\_watch system call. The kernel allocates and associates a watch descriptor with the given file or directory, storing relevant information in the internal data structures.
3.  **Event Reporting**: Kernel detects file system events on the watched files or directories and generates corresponding Inotify events. These events are stored in an internal event queue associated with the Inotify instance. The kernel tracks events such as file modifications, creations, deletions, and attribute changes.
4.  **Event Retrieval**: To obtain the reported events, the application reads from the file descriptor linked to the Inotify instance. By using the read system call, the application fetches one or more Inotify events from the event queue and transfers them to a buffer provided by the application.
5.  **Event Handling**: Once the application obtains the Inotify events, it processes them based on the event type and associated data. The application can examine event structures to determine the specific event that occurred, access the name of the file or directory involved, and perform appropriate actions accordingly. It may involve updating files, triggering notifications, initiating further operations, or any other desired behavior.
6.  **Event Monitoring**: The application can maintain event monitoring by iterating the process of reading events from the file descriptor. By integrating this event loop mechanism, the application remains responsive and receives real-time updates on the watched files or directories.


<!-- 重新开始编号 -->
1. **初始化**：当应用程序使用 inotify_init 系统调用创建 inotify 实例时，Linux 内核设置必要的数据结构和资源来管理 inotify 子系统。此初始化返回代表 inotify 实例的文件描述符。
2. **监视注册**：一旦建立 inotify 实例，应用程序可以添加监视来监控特定的文件或目录。应用程序通过调用 inotify_add_watch 系统调用，指定 inotify 实例的文件描述符、要监视的文件或目录的路径以及要监视的事件类型。内核为给定的文件或目录分配并关联一个监视描述符，将相关信息存储在内部数据结构中。
3. **事件报告**：内核检测监视的文件或目录上的文件系统事件，并生成相应的 inotify 事件。这些事件存储在与 inotify 实例关联的内部事件队列中。内核跟踪文件修改、创建、删除和属性变化等事件。
4. **事件检索**：为了获取报告的事件，应用程序从与 inotify 实例链接的文件描述符中读取。通过使用read系统调用，应用程序从事件队列中获取一个或多个 inotify 事件，并将它们传输到应用程序提供的缓冲区中。
6. **事件处理**：一旦应用程序获取了 inotify 事件，它就可以根据事件类型和相关数据进行处理。应用程序可以检查事件结构以确定发生的特定事件，访问涉及的文件或目录的名称，并相应地执行适当的操作。它可能涉及更新文件、触发通知、启动进一步的操作或任何其他所需行为。

It's important to note that Inotify operates at the kernel level, providing a streamlined and efficient means of monitoring file system activities. By minimizing the need for polling and actively notifying applications of relevant events, Inotify enables efficient event-driven programming and real-time responsiveness to changes in the file system. 

重要的是要注意，inotify在内核级别运行，提供了一种流线型和高效的监控文件系统活动的方式。通过最小化轮询的需求，并积极通知应用程序相关事件，inotify 使事件驱动的编程和对文件系统变化的实时响应成为可能。

![iNotify_Working](https://www.scaler.com/topics/images/inotify-working.webp) ![iNotify_Working_2](https://www.scaler.com/topics/images/inotify-working-2.webp)

### inotify API

The Inotify API in Linux provides functions that applications use to interact with the Inotify subsystem. These functions allow applications to create iNotify instances, add watches, read events, and handle file system activities. Here are the vital functions of the Inotify API:

Linux 中的 inotify API 提供了应用程序与 inotify 子系统交互的函数。这些函数允许应用程序创建 iNotify 实例，添加监视，读取事件和处理文件系统活动。以下是 inotify API 的关键函数：

1.  **inotify\_init**: This function initializes an iNotify instance and returns a file descriptor representing the instance. It creates the necessary data structures and resources within the kernel.
2.  **inotify\_add\_watch**: Applications use this function to add a watch to a specific file or directory. It takes the Inotify instance's file descriptor, the path to the file or directory, and a set of events to monitor. It returns a watch descriptor that uniquely identifies the watch.
3.  **read**: Applications utilize this function to read events from the file descriptor associated with the Inotify instance. It retrieves one or more events from the event queue and stores them in a buffer provided by the application. The function returns the number of bytes read.
4.  **inotify\_event**: This structure represents an Inotify event. It contains information about the event type, the name of the file or directory associated with the event, and additional data if needed. Applications can access the event data through this structure.


<!-- 重新开始编号 -->
1. **inotify_init**：此函数初始化一个 iNotify 实例并返回代表该实例的文件描述符。它在内核内创建必要的数据结构和资源。
2. **inotify_add_watch**：应用程序使用此函数向特定文件或目录添加监视。它采用iNotify实例的文件描述符、要监视的文件或目录的路径和要监视的事件集。它返回一个监视描述符，唯一标识监视。
3. **read**：应用程序使用此函数从与 iNotify 实例关联的文件描述符中读取事件。它从事件队列中检索一个或多个事件，并将它们存储在应用程序提供的缓冲区中。该函数返回读取的字节数。
4. **inotify_event**：此结构表示一个 iNotify 事件。它包含有关事件类型、与事件关联的文件或目录的名称以及需要的附加数据的信息。应用程序可以通过此结构访问事件数据。

By using these functions and working with the Inotify API, applications can effectively set up event monitoring, receive notifications, and respond to file system changes in a dynamic and real-time manner.

通过使用这些函数和使用 inotify API，应用程序可以有效地设置事件监视，接收通知，并以动态和实时的方式响应文件系统的变化。

### inotify 事件（inotify Events）

The Linux kernel generates Inotify events to notify applications about various file system activities. These events provide valuable information about changes occurring on the watched files or directories. Here are the vital aspects of inotify events:

Linux 内核生成 inotify 事件以通知应用程序有关各种文件系统活动。这些事件提供了有关在监视的文件或目录上发生的更改的宝贵信息。以下是 inotify 事件的关键方面：

1.  **Event Types**: Inotify events represent different file system activities. These include:
    *   **File modifications** (**IN\_MODIFY**): This event happens when a file undergoes through some modification, such as a change in its content.
    *   **File creations** (**IN\_CREATE**): This event is triggered when a newly created file appears in the monitored directory.
    *   **File deletions** (**IN\_DELETE**): This event occurs when a file is deleted from the watched directory.
    *   **Attribute changes** (**IN\_ATTRIB**): Signifies changes in file metadata or attributes, such as permissions or timestamps.
    *   **Directory modifications** (**IN\_ISDIR**): Indicates that the event pertains to a directory.
2.  **Event Data**: Each iNotify event is associated with specific data that provides additional information. The relevant event data includes:
    *   **Watch Descriptor**: The unique identifier (watch descriptor) of the watch associated with the file or directory generating the event.
    *   **Event Mask**: A bit mask representing the type of event that occurred.
    *   **Cookie**: A cookie is assigned to related events, allowing grouping or tracking of the events.
    *   **File or Directory Name**: The name of the file or directory associated with the event.
3.  **Event Retrieval**: Applications utilize the read system call to retrieve iNotify events from the file descriptor associated with the iNotify instance. The Linux kernel stores events in a buffer when they occur. The read call fetches one or more events from the buffer into the application's provided buffer.
4.  **Event Handling**: Upon receiving iNotify events, applications can process them accordingly. By examining the event data, applications can determine the event type, the file or directory involved, and other relevant details. Based on this information, applications can take appropriate actions, such as updating files, triggering notifications, or performing specific operations based on the event type.


<!-- 重新开始编号 -->
1. **事件类型**：inotify事件代表不同的文件系统活动。这些包括：
  *   **文件修改（IN_MODIFY）**：当文件经历一些修改时，例如其内容发生变化时，会发生此事件。
  *   **文件创建（IN_CREATE）**：当在监视的目录中出现新创建的文件时，会触发此事件。
  *   **文件删除（IN_DELETE）**：当监视目录中的文件被删除时，会发生此事件。
  *   **属性变化（IN_ATTRIB）**：表示文件元数据或属性的更改，例如权限或时间戳。
  *   **目录修改（IN_ISDIR）**：表示事件与目录有关。
2. **事件数据**：每个iNotify事件都与特定数据相关联，提供额外的信息。相关的事件数据包括：
  *   **监视描述符**：生成事件的文件或目录的监视描述符的唯一标识符。
  *   **事件掩码**：表示发生的事件类型的位掩码。
  *   **Cookie**：分配给相关事件的 Cookie，允许对事件进行分组或跟踪。
  *   **文件或目录名称**：与事件关联的文件或目录的名称。

3. **事件检索**：应用程序使用 read 系统调用来从与 iNotify 实例关联的文件描述符中检索 iNotify 事件。Linux 内核在事件发生时将事件存储在缓冲区中。read 调用来从缓冲区中获取一个或多个事件到应用程序提供的缓冲区。

4. **事件处理**：接收到 iNotify 事件后，应用程序可以相应地处理它们。通过检查事件数据，应用程序可以确定事件类型，涉及的文件或目录以及其他相关细节。基于这些信息，应用程序可以采取适当的行动，例如更新文件，触发通知或根据事件类型执行特定操作。

Inotify events enables real-time monitoring and responsiveness to file system changes. By leveraging the event information provided by iNotify, applications can effectively track and respond to modifications, creations, deletions, and attribute changes occurring in the watched files and directories.

inotify 事件使文件系统更改的实时监控和响应成为可能。通过利用 inotify 提供的事件信息，应用程序可以有效地跟踪并响应监视的文件和目录中发生的修改、创建、删除和属性变化。

### 版本（Versions）

The integration of inotify into the Linux kernel occurred with the merging of the 2.6.13 version. Glibc version 2.4 introduced the necessary library interfaces. Features such as IN\_DONT\_FOLLOW, IN\_MASK\_ADD, and IN\_ONLYDIR were included in version 2.5 of glibc.

inotify 集成到 Linux 内核是在 **2.6.13** 版本合并时发生的。Glibc 版本 2.4 引入了必要的库接口。像 IN_DONT_FOLLOW、IN_MASK_ADD 和 IN_ONLYDIR 这样的特性被包含在 glibc 的 2.5 版本中。

### 注意事项（Notes）

Here are some additional points to consider regarding inotify in Linux:

以下是一些关于 Linux 中 inotify 的额外要点：

1.  **Availability**: Inotify has been a part of the Linux kernel since version 2.6.13, making it widely available on modern Linux distributions.
2.  **Event Limit**: A single process using inotify can create only a limited number of watches. This limit can vary depending on the system configuration, but it is typically a high value to accommodate various use cases.
3.  **Efficiency**: Inotify optimizes resource usage and operates efficiently. It uses a file descriptor to communicate events, and multiple events can be read in a single read call, reducing the overhead of event handling.
4.  **Recursive Monitoring**: Inotify can monitor directories recursively. When you add a watch to a directory, you can receive events for changes within that specific directory and its subdirectories.
5.  **Event Ordering**: The order of events delivered by inotify may not always match the exact order of the changes. The delivery of events can occur in batches, and interleaving events from different files or directories is possible.
6.  **Buffer Size**: The kernel utilizes a buffer with a limited size to store events. If events exceed the reading capacity, the buffer may overflow, causing the events to be lost. Proper handling of event reading and buffer management is necessary to ensure all events are processed correctly.
7.  **Permissions and Access**: Inotify events are subject to file permissions and access restrictions. Insufficient permissions can prevent an application from receiving events for specific files or directories.
8.  **Integration with Programming Languages**: Besides C language, various programming languages provide bindings and libraries for inotify, making it accessible and usable in different development environments.


<!-- 重新开始编号 -->
1. **可用性**：inotify 自 Linux 内核版本 2.6.13 以来一直是 Linux 内核的一部分，使其在现代 Linux 发行版上广泛可用。
2. **事件限制**：使用 inotify 的单个进程只能创建有限数量的监视。此限制可能因系统配置而异，但通常是一个高值，以适应各种用例。
3. **效率**：inotify 优化了资源使用并有效运行。它使用文件描述符来通信事件，并且可以在单个 read 调用来读取多个事件，减少了事件处理的开销。
4. **递归监控**：inotify 可以递归地监视目录。当我们向目录添加监视时，我们可以接收到该特定目录及其子目录中更改的事件。
5. **事件排序**：inotify 交付的事件顺序可能不总是与更改的确切顺序相匹配。事件的交付可以批量进行，不同文件或目录的事件交错是可能的。
6. **缓冲区大小**：内核使用有限大小的缓冲区来存储事件。如果事件超出读取能力，缓冲区可能会溢出，导致事件丢失。适当的事件读取和缓冲区管理是必要的，以确保所有事件都被正确处理。
7. **权限和访问**：inotify 事件受文件权限和访问限制的约束。权限不足可能会阻止应用程序接收特定文件或目录的事件。
8. **与编程语言的集成**：除了 C 语言，各种编程语言提供了 inotify 的绑定和库，使其在不同的开发环境中可访问和可用。

It's vital to consult the official documentation and resources specific to your Linux distribution for detailed information and best practices when using inotify in your applications.

使用 inotify 时，重要的是要查阅特定于 Linux 发行版或 Linux 内核社区的官方文档和资源，以获取详细信息和最佳实践。

### Bugs

Like any software, inotify is not exempt from encountering bugs or issues. Here are some points to be aware of when working with inotify in Linux:

像任何软件一样，inotify 也不是免于遇到错误或问题。以下是在使用 Linux 中的 inotify 时需要注意的一些要点：

1.  **Kernel Version Compatibility**: Confirm that your kernel version is compatible with inotify and that you usea a compatible version of the inotify API. Using an outdated or incompatible kernel or API may lead to unexpected behavior or bugs.
2.  **Bug Reports and Fixes**: If you encounter a bug or issue related to inotify, check the bug tracking system of your Linux distribution or the official Linux kernel bug tracker for any reported issues or fixes. Keeping your system up-to-date with the kernel patches and updates can help resolve known bugs.
3.  **Buffer Overflow**: Inotify stores events in a buffer. If the events are generated faster than they can be processed, the buffer may overflow. It can result in missed events or incomplete event handling. Proper management of the event buffer and efficient event processing is essential to avoid potential issues.
4.  **Race Conditions**: Race conditions can occur when multiple processes or threads use inotify simultaneously. These conditions may lead to inconsistent behavior or unexpected results. Proper synchronization mechanisms and careful handling of shared resources can help mitigate race condition-related bugs.
5.  **Resource Limitations**: Inotify imposes certain limits on the number of watches and the size of the event buffer. Exceeding these limits may cause unexpected behavior or failures. It is vital to be aware of these limitations and design your applications accordingly to avoid potential bugs.
6.  **Edge Cases**: Inotify may have specific edge cases or corner scenarios that could trigger unexpected behavior. Thorough testing and understanding of the inotify API and its behavior can help identify and address potential bugs in such situations.
7.  **Third-Party Libraries**: When using third-party libraries or frameworks that rely on inotify, be mindful of their compatibility and potential bugs that may arise from their usage. Stay informed about any updates, patches, or bug reports related to those libraries.
8.  **Bug Reporting**: If you encounter a bug in the inotify subsystem, it is encouraged to report it to the appropriate channels. Provide detailed information about the issue, steps to reproduce, and any relevant system configurations to help developers address and resolve the bug effectively.


<!-- 重新开始编号 -->
1. **内核版本兼容性**：请确认内核版本与 inotify 兼容性，并且使用的是兼容版本的 inotify API。
使用过时或不兼容的内核或 API 可能会导致意外行为或错误。
2. **错误报告和修复**：如果我们遇到与 inotify 相关的错误或问题，请检查 Linux 发行版的错误跟踪系统或官方 Linux 内核错误跟踪器是否有任何报告的问题或修复。通过保持系统与内核补丁和更新的最新状态，可以帮助解决已知的错误。
3. **缓冲区溢出**：inotify 在缓冲区中存储事件。如果事件生成速度超过它们可以被处理的速度，缓冲区可能会溢出。这可能导致错过事件或不完整的事件处理。适当的事件缓冲区管理和高效的事件处理对于避免潜在问题至关重要。
4. **竞态条件**：当多个进程或线程同时使用 inotify 时，可能会发生竞态条件。这些条件可能导致不一致的行为或意外结果。适当的同步机制和仔细处理共享资源可以帮助减轻与竞态条件相关的错误。
5. **资源限制**：inotify 对监视的数量和事件缓冲区的大小施加了某些限制。超出这些限制可能会导致意外行为或失败。了解这些限制并相应地设计应用程序以避免潜在的错误至关重要。
6. **边缘情况**：inotify 可能具有特定的边缘情况或角落场景，可能会触发意外行为。对 inotify API 及其行为的全面测试和理解可以帮助识别和解决这些情况下的潜在错误。
7. **第三方库**：当使用依赖 inotify 的第三方库或框架时，请留意它们的兼容性和可能出现的错误。了解与这些库相关的任何更新、补丁或错误报告。
8. **错误报告**：如果在 inotify 子系统中遇到错误，鼓励大家向适当的渠道报告。提供有关问题的详细信息、重现步骤和任何相关的系统配置，以帮助开发人员有效解决和解决错误。

Remember to consult official documentation, bug trackers, and support channels specific to your Linux distribution or the Linux kernel community for the latest information on reported bugs and their resolutions.

请查阅特定于 Linux发 行版或 Linux 内核社区的官方文档、错误跟踪器和支持渠道，以获取有关报告的错误及其解决方案的最新信息。

inotify 工具安装（inotify Installation）
--------------------

To use inotify, install the necessary components in your Linux system. Here are the steps to install and set up inotify:

要使用 inotify，请在 Linux 系统中安装必要的组件。以下是安装和设置 inotify 的步骤：

1. **检查内核支持**：验证当前 Linux 内核版本是否支持 inotify。我们可以通过运行以下命令来检查内核版本（**Check Kernel Support**: Verify that your Linux kernel version supports inotify. You can check the kernel version by running the command）：

```bash
uname -r
```
Ensure that the kernel version is 2.6.13 or later, as this version is introduced inotify.

确保内核版本至少为 **2.6.13**，因为这是引入 inotify 的版本。

2. **包管理器**：使用 Linux 发行版的包管理器来安装 inotify 工具和库。包名称可能因发行版而异。例如，在基于 Ubuntu 或 Debian 的系统上，我们可以使用 apt 包管理器（**Package Manager**: Use your Linux distribution's package manager to install the inotify tools and libraries. The package name may vary depending on your distribution. For example, on Ubuntu or Debian-based systems, you can use the apt package manager）：

```bash
sudo apt install inotify-tools
```

3.  **验证安装**：现在是时候验证 inotify 工具的安装了。运行以下命令以检查 inotifywait 工具的版本：（**Verify Installation**: It's time to verify the installation of inotify tools. Run the following command to check the version of the inotifywait tool）：

```bash
inotifywait --version
```
Executing this command will display the version information.

执行此命令将显示版本信息。

4.  **从源代码编译**：如果发行版没有提供预打包的 inotify 工具或库，我们可以从源代码编译它们。从官方 Linux 内核存储库或其他可信来源获取源代码，并按照随源代码提供的说明编译和安装 inotify 组件（**Compile from Source**: If your distribution does not provide pre-packaged inotify tools or libraries, you can compile them from the source. Obtain the source code from the official Linux kernel repository or other trusted sources, and follow the instructions provided with the source code to compile and install the inotify components）。

5.  **API集成**：要在程序中使用 inotify，请在代码中包含 inotify.h 头文件。确保在编译过程中链接到适当的 inotify 库。请查看编程语言或框架的文档或示例，以获取有关集成 inotify API 的更多详细信息（**API Integration**: To use inotify in your programs, include the inotify.h header file in your code. Make sure to link against the appropriate inotify libraries during the compilation process. Consult the documentation or examples specific to your programming language or framework for more details on integrating the inotify API）。

Once the installation and integration steps are complete, you can use the inotify features and capabilities in applications. Remember to refer to the official documentation and resources for further guidance on using inotify effectively in your Linux system.

安装和集成步骤完成后，我们可以在应用程序中使用 inotify 的功能和能力。请记住，参考官方文档和资源，以获取有关在 Linux 系统中有效使用 inotify 的进一步指导。

如何实现 inotify（How to Implement inotify）
------------------------

Implementing inotify in your Linux application involves the following steps:

在 Linux 应用程序中实现 inotify 涉及以下步骤：

1.  **Initialize an inotify Instance**: Begin by creating an inotify instance using the inotify\_init() system call. This call returns a file descriptor that represents the inotify instance.
2.  **Add Watches**: Add watches to the inotify instance to monitor specific files or directories using the inotify\_add\_watch() system call. To make this call, provide the file descriptor of the inotify instance and the path to the file or directory you want to monitor. It returns a watch descriptor that represents the added watch.
3.  **Read Events**: Continuously read events from the inotify instance's file descriptor using the read() system call. The buffer stores the events, allowing multiple events to be read in a single read() call. Each event provides information about the type of change, the file or directory involved, and additional data if necessary.
4.  **Handle Events**: Process the events according to your application's logic. Determine the event type, extract the necessary information from the event data, and perform the appropriate actions based on the event. It may involve updating files, triggering specific functions, or sending notifications, among other possibilities.
5.  **Remove Watches**: When you no longer need to monitor a file or directory, remove the associated watch using the inotify\_rm\_watch() system call. You need to provide the file descriptor of the inotify instance and the watch descriptor of the watch you want to remove.
6.  **Cleanup**: Once finished with the inotify instance, close its file descriptor using the close() system call to release system resources.


<!-- 重新开始编号 -->
1. **初始化 inotify 实例**：首先使用 inotify_init()系统调用来创建一个 inotify 实例。此调用返回代表 inotify 实例的文件描述符。
2. **添加监视**：使用 inotify_add_watch()系统调用来向 inotify 实例添加监视，以监视特定的文件或目录。提供 inotify 实例的文件描述符和要监视的文件或目录的路径。它返回代表添加监视的监视描述符。
3. **读取事件**：使用 read()系统调用来连续从 inotify 实例的文件描述符读取事件。事件存储在缓冲区中，允许在单个 read()调用中读取多个事件。每个事件提供有关更改类型的信息，涉及的文件或目录以及如果需要的附加数据。
4. **处理事件**：根据应用程序的逻辑处理事件。确定事件类型，从事件数据中提取必要信息，并根据事件执行适当的操作。可能涉及更新文件、触发特定功能或发送通知等。
5. **移除监视**：当您不再需要监视文件或目录时，使用 inotify_rm_watch()系统调用来移除相关监视。您需要提供inotify实例的文件描述符和要移除的监视的监视描述符。
6. **清理**：完成 inotify 实例的使用后，使用close()系统调用来关闭其文件描述符以释放系统资源。

Refer to the official documentation and resources for detailed information and examples specific to your programming language or framework.

请参考官方文档和资源，以获取特定于编程语言或框架的详细信息和示例。

示例（Example）
-------

To illustrate the usage of inotify in a practical scenario, consider the following example:

为了说明 inotify 在实际场景中的使用，考虑以下示例：

Suppose you have a directory called /var/logs that contains log files for your application. You intend to monitor this directory for modifications or newly created log files. When a log file is modified or created, you want to display a notification on the console.

假设我们有一个名为 **/var/logs** 的目录，其中包含应用程序的日志文件。我们打算监视此目录以查找修改或新创建的日志文件。当日志文件被修改或创建时，就可以在控制台上显示通知。

Here's how you can implement this using inotify in a Linux application:

以下是如何使用 Linux 应用程序中的 inotify 实现此目的：

1.  Initialize an inotify instance by calling inotify\_init(), which returns a file descriptor representing the inotify instance.
2.  Add a watch for the /var/logs directory using inotify\_add\_watch(). Provide the inotify instance's file descriptor and the path to the directory. Obtain the watch descriptor for the added watch.
3.  Enter a loop to read events from the inotify instance. Use read() to read the events from the file descriptor. Handle multiple events if they are available in the event buffer.
4.  For each event, determine the type of change and the file or directory involved. In this case, we are interested in IN\_MODIFY and IN\_CREATE events for log file modifications and creations, respectively.
5.  If an event matches the desired criteria, display a notification on the console indicating the name of the modified or created log file.
6.  Continue reading and processing events until you exit the loop.
7.  Before terminating the application, remove the watch for the /var/logs directory using inotify\_rm\_watch(). Provide the inotify instance's file descriptor and the watch descriptor of the directory watch.


<!-- 重新开始编号 -->
1. 通过调用 inotify_init()初始化 inotify 实例，它返回代表 inotify 实例的文件描述符。
2. 使用 inotify_add_watch()为 /var/logs 目录添加监视。提供 inotify 实例的文件描述符和目录的路径。获取添加监视的监视描述符。
3. 进入循环，从 inotify 实例读取事件。使用 read()从文件描述符读取事件。如果事件缓冲区中有多个事件，请处理它们。
4. 对于每个事件，确定更改的类型和涉及的文件或目录。在这种情况下，我们对日志文件修改和创建的 IN_MODIFY 和 IN_CREATE 事件感兴趣。
5. 如果事件符合所需条件，请在控制台上显示通知，指示被修改或创建的日志文件的名称。
6. 继续读取和处理事件，直到退出循环。
7. 在应用程序终止之前，使用 inotify_rm_watch()移除 /var/logs 目录的监视。提供 inotify 实例的文件描述符和目录监视的监视描述符。

```bash
#!/bin/bash

# Directory to watch
WATCH_DIR="/var/log"

# Monitor the directory and its subdirectories for changes using inotifywait
inotifywait -m -r -e create -e modify --format '%w%f' "$WATCH_DIR" | while read FILE
do
  echo "Log file $FILE has been modified or created"
done
```

By implementing this example, you can create a Linux application that actively monitors log file changes in the /var/logs directory and displays notifications in real-time.

通过实现此示例，我们可以创建一个 Linux 应用程序，该程序积极监视 /var/logs 目录中的日志文件更改，并实时显示通知。

This example demonstrates the versatility and usefulness of inotify for real-time monitoring of file system changes in various applications and scenarios.

这个示例展示了 inotify 在各种应用程序和场景中实时监控文件系统更改的多样性和有用性。

总结（Conclusion）
----------

Inotify is a powerful Linux kernel feature that provides file and directory monitoring capabilities. It allows applications to efficiently and in real-time receive notifications about various events occurring in the file system, such as file modifications, creations, deletions, and attribute changes. With its flexible API, inotify enables developers to implement robust and responsive file system monitoring solutions.

inotify 是一个强大的Linux内核特性，提供了文件和目录监控的能力。它允许应用程序高效地实时接收有关文件系统中发生的各种事件的通知，例如文件修改、创建、删除和属性变化。凭借其灵活的 API，inotify 使开发人员能够实现强大且响应迅速的文件系统监控解决方案。

Key takeaways from this article on inotify in Linux:

本文关于 Linux 中 inotify 的关键要点：

*   Inotify, a Linux kernel subsystem, empowers applications to receive real-time notifications of file and directory events.
*   The vital components of inotify include initializing an inotify instance, adding watches for specific files or directories, reading and processing events, and handling the events according to application logic.
*   Inotify events represent changes, such as file modifications, creations, deletions, attribute changes, and more.
*   Implementing inotify requires integrating the inotify API into your application and managing event buffering, error handling, and synchronization.
*   Inotify can be used for various purposes, including automatic directory views, configuration file reloading, change logging, and file synchronization.

<!-- 重新开始编号 -->
* inotify 是一个 Linux 内核子系统，使应用程序能够接收文件和目录事件的实时通知。
* inotify 的关键组件包括初始化 inotify 实例、为特定文件或目录添加监视、读取和处理事件以及根据应用程序逻辑处理事件。
* inotify 事件代表更改，例如文件修改、创建、删除、属性变化等。
* 实现 inotify 需要将 inotify API 集成到应用程序中，并管理事件缓冲、错误处理和同步。
* inotify 可以用于自动目录视图、配置文件重新加载、更改日志记录和文件同步等多种目的。

By leveraging inotify, developers can create applications that respond promptly to file system changes, enhancing functionality, automation, and monitoring capabilities. Understanding the inotify API and its usage opens up possibilities for efficient file system event handling in Linux environments.

通过利用 inotify，开发人员可以创建及时响应文件系统更改的应用程序，增强功能、自动化和监控能力。了解 inotify API 及其用法为在 Linux 环境中有效处理文件系统事件提供了可能性。