What is inotify in Linux?
=========================
**[原文地址](https://www.scaler.com/topics/linux-inotify/)**

### Table of Contents

1. **Overview**
2. **inotify Installation**
3. **How to Implement inotify**
4. **Example**
5. **Conclusion**

Overview
--------

Inotify is a Linux kernel subsystem that provides file and directory monitoring capabilities. It enables applications to receive notifications about diverse events occurring in the file system, including file modifications, creations, deletions, and attribute changes. Inotify does real-time monitoring of file system activities, making it a valuable tool for many applications and system utilities.

The vital components of Linux Inotify are as follows:

1.  **Inotify Instance**: To use Inotify, an application first creates an instance of Inotify by calling the inotify\_init system call. It returns a file descriptor that represents the Inotify instance.
2.  **Watch Descriptors**: After creating the Inotify instance, an application can add watches to monitor specific files or directories. Calling the inotify\_add\_watch system call retrieves a watch descriptor. This descriptor represents a watch. The watch descriptor is linked to both the Inotify instance and the file or directory under monitoring.
3.  **Events**: Inotify notifies about events happening on the files or directories being watched. Events can include modifications, creations, deletions, attribute changes, and more. Each event is represented by a data structure containing relevant information such as the event type, the name of the file or directory, and additional data if needed.
4.  **Reading Events**: To retrieve events, an application reads from the file descriptor associated with the Inotify instance. The read system call is used to obtain the event data. Multiple events can be read in a single read call, as they are stored in a buffer within the kernel.
5.  **Handling Events**: Once an application has read the events, it can process them accordingly. It may involve performing specific actions based on the event type, such as updating a file, sending notifications, or triggering other operations. The application can differentiate between different event types using the information provided by the event data structure.

Linux Inotify provides a flexible and powerful mechanism for monitoring file system events, allowing applications to respond to real-time changes. It is used in various domains, including system monitoring, synchronization, and automation.

### How Does It Work?

Inotify operates based on a simple and efficient mechanism within the Linux kernel. Here is an overview of how it works:

1.  **Initialization**: When an application creates an instance of Inotify using the inotify\_init system call, the Linux kernel sets up the necessary data structures and resources to manage the Inotify subsystem. This initialization returns a file descriptor representing the Inotify instance.
2.  **Watch Registration**: Once you establish the Inotify instance, applications can add watches to monitor specific files or directories of interest. The application specifies the Inotify instance's file descriptor, the path to the file or directory to be watched, and the types of events to be monitored by calling the inotify\_add\_watch system call. The kernel allocates and associates a watch descriptor with the given file or directory, storing relevant information in the internal data structures.
3.  **Event Reporting**: Kernel detects file system events on the watched files or directories and generates corresponding Inotify events. These events are stored in an internal event queue associated with the Inotify instance. The kernel tracks events such as file modifications, creations, deletions, and attribute changes.
4.  **Event Retrieval**: To obtain the reported events, the application reads from the file descriptor linked to the Inotify instance. By using the read system call, the application fetches one or more Inotify events from the event queue and transfers them to a buffer provided by the application.
5.  **Event Handling**: Once the application obtains the Inotify events, it processes them based on the event type and associated data. The application can examine event structures to determine the specific event that occurred, access the name of the file or directory involved, and perform appropriate actions accordingly. It may involve updating files, triggering notifications, initiating further operations, or any other desired behavior.
6.  **Event Monitoring**: The application can maintain event monitoring by iterating the process of reading events from the file descriptor. By integrating this event loop mechanism, the application remains responsive and receives real-time updates on the watched files or directories.

It's important to note that Inotify operates at the kernel level, providing a streamlined and efficient means of monitoring file system activities. By minimizing the need for polling and actively notifying applications of relevant events, Inotify enables efficient event-driven programming and real-time responsiveness to changes in the file system. ![iNotify_Working](https://www.scaler.com/topics/images/inotify-working.webp) ![iNotify_Working_2](https://www.scaler.com/topics/images/inotify-working-2.webp)

### inotify API

The Inotify API in Linux provides functions that applications use to interact with the Inotify subsystem. These functions allow applications to create iNotify instances, add watches, read events, and handle file system activities. Here are the vital functions of the Inotify API:

1.  **inotify\_init**: This function initializes an iNotify instance and returns a file descriptor representing the instance. It creates the necessary data structures and resources within the kernel.
2.  **inotify\_add\_watch**: Applications use this function to add a watch to a specific file or directory. It takes the Inotify instance's file descriptor, the path to the file or directory, and a set of events to monitor. It returns a watch descriptor that uniquely identifies the watch.
3.  **read**: Applications utilize this function to read events from the file descriptor associated with the Inotify instance. It retrieves one or more events from the event queue and stores them in a buffer provided by the application. The function returns the number of bytes read.
4.  **inotify\_event**: This structure represents an Inotify event. It contains information about the event type, the name of the file or directory associated with the event, and additional data if needed. Applications can access the event data through this structure.

By using these functions and working with the Inotify API, applications can effectively set up event monitoring, receive notifications, and respond to file system changes in a dynamic and real-time manner.

### inotify Events

The Linux kernel generates Inotify events to notify applications about various file system activities. These events provide valuable information about changes occurring on the watched files or directories. Here are the vital aspects of inotify events:

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

Inotify events enables real-time monitoring and responsiveness to file system changes. By leveraging the event information provided by iNotify, applications can effectively track and respond to modifications, creations, deletions, and attribute changes occurring in the watched files and directories.

### Versions

The integration of inotify into the Linux kernel occurred with the merging of the 2.6.13 version. Glibc version 2.4 introduced the necessary library interfaces. Features such as IN\_DONT\_FOLLOW, IN\_MASK\_ADD, and IN\_ONLYDIR were included in version 2.5 of glibc.

### Notes

Here are some additional points to consider regarding inotify in Linux:

1.  **Availability**: Inotify has been a part of the Linux kernel since version 2.6.13, making it widely available on modern Linux distributions.
2.  **Event Limit**: A single process using inotify can create only a limited number of watches. This limit can vary depending on the system configuration, but it is typically a high value to accommodate various use cases.
3.  **Efficiency**: Inotify optimizes resource usage and operates efficiently. It uses a file descriptor to communicate events, and multiple events can be read in a single read call, reducing the overhead of event handling.
4.  **Recursive Monitoring**: Inotify can monitor directories recursively. When you add a watch to a directory, you can receive events for changes within that specific directory and its subdirectories.
5.  **Event Ordering**: The order of events delivered by inotify may not always match the exact order of the changes. The delivery of events can occur in batches, and interleaving events from different files or directories is possible.
6.  **Buffer Size**: The kernel utilizes a buffer with a limited size to store events. If events exceed the reading capacity, the buffer may overflow, causing the events to be lost. Proper handling of event reading and buffer management is necessary to ensure all events are processed correctly.
7.  **Permissions and Access**: Inotify events are subject to file permissions and access restrictions. Insufficient permissions can prevent an application from receiving events for specific files or directories.
8.  **Integration with Programming Languages**: Besides C language, various programming languages provide bindings and libraries for inotify, making it accessible and usable in different development environments.

It's vital to consult the official documentation and resources specific to your Linux distribution for detailed information and best practices when using inotify in your applications.

### Bugs

Like any software, inotify is not exempt from encountering bugs or issues. Here are some points to be aware of when working with inotify in Linux:

1.  **Kernel Version Compatibility**: Confirm that your kernel version is compatible with inotify and that you usea a compatible version of the inotify API. Using an outdated or incompatible kernel or API may lead to unexpected behavior or bugs.
2.  **Bug Reports and Fixes**: If you encounter a bug or issue related to inotify, check the bug tracking system of your Linux distribution or the official Linux kernel bug tracker for any reported issues or fixes. Keeping your system up-to-date with the kernel patches and updates can help resolve known bugs.
3.  **Buffer Overflow**: Inotify stores events in a buffer. If the events are generated faster than they can be processed, the buffer may overflow. It can result in missed events or incomplete event handling. Proper management of the event buffer and efficient event processing is essential to avoid potential issues.
4.  **Race Conditions**: Race conditions can occur when multiple processes or threads use inotify simultaneously. These conditions may lead to inconsistent behavior or unexpected results. Proper synchronization mechanisms and careful handling of shared resources can help mitigate race condition-related bugs.
5.  **Resource Limitations**: Inotify imposes certain limits on the number of watches and the size of the event buffer. Exceeding these limits may cause unexpected behavior or failures. It is vital to be aware of these limitations and design your applications accordingly to avoid potential bugs.
6.  **Edge Cases**: Inotify may have specific edge cases or corner scenarios that could trigger unexpected behavior. Thorough testing and understanding of the inotify API and its behavior can help identify and address potential bugs in such situations.
7.  **Third-Party Libraries**: When using third-party libraries or frameworks that rely on inotify, be mindful of their compatibility and potential bugs that may arise from their usage. Stay informed about any updates, patches, or bug reports related to those libraries.
8.  **Bug Reporting**: If you encounter a bug in the inotify subsystem, it is encouraged to report it to the appropriate channels. Provide detailed information about the issue, steps to reproduce, and any relevant system configurations to help developers address and resolve the bug effectively.

Remember to consult official documentation, bug trackers, and support channels specific to your Linux distribution or the Linux kernel community for the latest information on reported bugs and their resolutions.

inotify Installation
--------------------

To use inotify, install the necessary components in your Linux system. Here are the steps to install and set up inotify:

1.  **Check Kernel Support**: Verify that your Linux kernel version supports inotify. You can check the kernel version by running the command:

```bash
uname -r
```
Ensure that the kernel version is 2.6.13 or later, as this version is introduced inotify.

2.  **Package Manager**: Use your Linux distribution's package manager to install the inotify tools and libraries. The package name may vary depending on your distribution. For example, on Ubuntu or Debian-based systems, you can use the apt package manager:

```bash
sudo apt install inotify-tools
```

3.  **Verify Installation**: It's time to verify the installation of inotify tools. Run the following command to check the version of the inotifywait tool:

```bash
inotifywait --version
```
Executing this command will display the version information.

4.  **Compile from Source**: If your distribution does not provide pre-packaged inotify tools or libraries, you can compile them from the source. Obtain the source code from the official Linux kernel repository or other trusted sources, and follow the instructions provided with the source code to compile and install the inotify components.
5.  **API Integration**: To use inotify in your programs, include the inotify.h header file in your code. Make sure to link against the appropriate inotify libraries during the compilation process. Consult the documentation or examples specific to your programming language or framework for more details on integrating the inotify API.

Once the installation and integration steps are complete, you can use the inotify features and capabilities in applications. Remember to refer to the official documentation and resources for further guidance on using inotify effectively in your Linux system.

How to Implement inotify
------------------------

Implementing inotify in your Linux application involves the following steps:

1.  **Initialize an inotify Instance**: Begin by creating an inotify instance using the inotify\_init() system call. This call returns a file descriptor that represents the inotify instance.
2.  **Add Watches**: Add watches to the inotify instance to monitor specific files or directories using the inotify\_add\_watch() system call. To make this call, provide the file descriptor of the inotify instance and the path to the file or directory you want to monitor. It returns a watch descriptor that represents the added watch.
3.  **Read Events**: Continuously read events from the inotify instance's file descriptor using the read() system call. The buffer stores the events, allowing multiple events to be read in a single read() call. Each event provides information about the type of change, the file or directory involved, and additional data if necessary.
4.  **Handle Events**: Process the events according to your application's logic. Determine the event type, extract the necessary information from the event data, and perform the appropriate actions based on the event. It may involve updating files, triggering specific functions, or sending notifications, among other possibilities.
5.  **Remove Watches**: When you no longer need to monitor a file or directory, remove the associated watch using the inotify\_rm\_watch() system call. You need to provide the file descriptor of the inotify instance and the watch descriptor of the watch you want to remove.
6.  **Cleanup**: Once finished with the inotify instance, close its file descriptor using the close() system call to release system resources.

Refer to the official documentation and resources for detailed information and examples specific to your programming language or framework.

Example
-------

To illustrate the usage of inotify in a practical scenario, consider the following example:

Suppose you have a directory called /var/logs that contains log files for your application. You intend to monitor this directory for modifications or newly created log files. When a log file is modified or created, you want to display a notification on the console.

Here's how you can implement this using inotify in a Linux application:

1.  Initialize an inotify instance by calling inotify\_init(), which returns a file descriptor representing the inotify instance.
2.  Add a watch for the /var/logs directory using inotify\_add\_watch(). Provide the inotify instance's file descriptor and the path to the directory. Obtain the watch descriptor for the added watch.
3.  Enter a loop to read events from the inotify instance. Use read() to read the events from the file descriptor. Handle multiple events if they are available in the event buffer.
4.  For each event, determine the type of change and the file or directory involved. In this case, we are interested in IN\_MODIFY and IN\_CREATE events for log file modifications and creations, respectively.
5.  If an event matches the desired criteria, display a notification on the console indicating the name of the modified or created log file.
6.  Continue reading and processing events until you exit the loop.
7.  Before terminating the application, remove the watch for the /var/logs directory using inotify\_rm\_watch(). Provide the inotify instance's file descriptor and the watch descriptor of the directory watch.

By implementing this example, you can create a Linux application that actively monitors log file changes in the /var/logs directory and displays notifications in real-time.

This example demonstrates the versatility and usefulness of inotify for real-time monitoring of file system changes in various applications and scenarios.

Conclusion
----------

Inotify is a powerful Linux kernel feature that provides file and directory monitoring capabilities. It allows applications to efficiently and in real-time receive notifications about various events occurring in the file system, such as file modifications, creations, deletions, and attribute changes. With its flexible API, inotify enables developers to implement robust and responsive file system monitoring solutions.

Key takeaways from this article on inotify in Linux:

*   Inotify, a Linux kernel subsystem, empowers applications to receive real-time notifications of file and directory events.
*   The vital components of inotify include initializing an inotify instance, adding watches for specific files or directories, reading and processing events, and handling the events according to application logic.
*   Inotify events represent changes, such as file modifications, creations, deletions, attribute changes, and more.
*   Implementing inotify requires integrating the inotify API into your application and managing event buffering, error handling, and synchronization.
*   Inotify can be used for various purposes, including automatic directory views, configuration file reloading, change logging, and file synchronization.

By leveraging inotify, developers can create applications that respond promptly to file system changes, enhancing functionality, automation, and monitoring capabilities. Understanding the inotify API and its usage opens up possibilities for efficient file system event handling in Linux environments.