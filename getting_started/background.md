# Background

This topic describes the history and motivation behind Linux, identifies its fundamental design paradigm, and explains how modern Linux supports scientific computing.

## Introduction

The Linux operating system is an extremely versatile Unix-like operating system, and has taken a clear lead in the High Performance Computing (HPC) and scientific computing community. Linux is a multi-user, preemptive multitasking operating system that provides a number of facilities including management of hardware resources, directories, and file systems, as well as the loading and execution of programs. A vast number of utilities and libraries have been developed (mostly free and open source as well) to accompany or extend Linux.

There are two major components of Linux, the kernel and the shell:

1.  The kernel is the core of the Linux operating system that schedules processes and interfaces directly with the hardware. It manages system and user I/O, processes, devices, files, and memory.
2.  The shell is a text-only interface to the kernel. Users input commands through the shell, and the kernel receives the tasks from the shell and performs them. The shell tends to do four jobs repeatedly: display a prompt, read a command, process the given command, then execute the command. After which it starts the process all over again.

It is important to note that users of a Linux system typically _do not_ interact with the kernel directly. Rather, most user interaction is done through the shell or a desktop environment.

## History

### Unix

The Unix operating system got its start in 1969 at Bell Laboratories and was written in assembly language. In 1973, Ken Thompson and Dennis Ritchie succeeded in rewriting Unix in their new language C. This was quite an audacious move; at the time, system programming was done in assembly in order to extract maximum performance from the hardware. The concept of a portable operating system was barely a gleam in anyone's eye.

The creation of a portable operating system was very significant in the computing industry, but then came the problem of licensing each type of Unix. Richard Stallman, an American software freedom activist and programmer recognized a need for open source solutions and launched the GNU project in 1983, later founding the [Free Software Foundation](https://www.fsf.org/). His goal was to create a completely free and open source operating system that was Unix-compatible or [Unix-like](https://en.wikipedia.org/wiki/Unix-like).

### Linux

In 1987, the source code to a minimalistic Unix-like operating system called [MINIX](https://en.wikipedia.org/wiki/MINIX) was released by Andrew Tanenbaum, a professor at Vrije Universiteit, for academic purposes. Linus Torvalds began developing a new operating system [based on MINIX](https://groups.google.com/forum/#!msg/comp.os.minix/dlNtH7RRrGA/SwRavCzVE7gJ) while a student at the University of Helsinki in 1991. In September of 1991, Torvalds released the first version (0.1) of the Linux kernel.

Torvalds greatly enhanced the open source community by releasing the Linux kernel under the [GNU General Public License](https://en.wikipedia.org/wiki/GNU_General_Public_License) so that everyone has access to the source code and can freely make modifications to it. Many components from the GNU project, such as the [GNU Core Utilities](https://en.wikipedia.org/wiki/GNU_Core_Utilities), were then integrated with the Linux kernel, thus completing the first free and open source operating system.

Linux has been adapted to a variety of computer systems of many sizes and purposes. Furthermore, different variants of Linux (called Linux distributions) have been [developed over time](https://en.wikipedia.org/wiki/Linux_distribution#History) to meet various needs. There are now hundreds of different Linux distributions available, with a wide variety of features. The most popular operating system in the world is actually [Android](https://en.wikipedia.org/wiki/Android_(operating_system)), which is built on the Linux kernel.

## Why Linux?

Linux has been so heavily utilized in the HPC and scientific computing community that it has become the standard in many areas of academic and scientific research, particularly those requiring HPC. There have been over 40 years of development in Unix and Linux, with many academic, scientific, and system tools. In fact, as of November 2017, _all of the [TOP500](https://www.top500.org/statistics/list/) supercomputers in the world run Linux!_

Linux has four essential properties which make it an excellent operating system for the science community:

*   **Performance** – Performance of the operating system can be optimized for specific tasks such as running small portable devices or large supercomputers.
*   **Functionality** – A number of community-driven scientific applications and libraries have been developed under Linux such as molecular dynamics, linear algebra, and fast-Fourier transforms.
*   **Flexibility** – The system is flexible enough to allow users to build applications with a wide array of support tools such as compilers, scientific libraries, debuggers, and network monitors.
*   **Portability** – The operating system, utilities, and libraries have been ported to a wide variety of devices including desktops, clusters, supercomputers, mainframes, embedded systems, and smart phones.

## Files and Processes

Everything in Linux is considered to be either a _file_ or a _process_:

*   A process is an executing program identified by a unique process identifier, called a PID. Processes may be short in duration, such as a process that prints a file to the screen, or they may run indefinitely, such as a monitor program.
*   A file is a collection of data, with a location in the file system called a path. Paths will typically be a series of words (directory names) separated by forward slashes, /. Files are generally created by users via text editors, compilers, or other means.
*   A directory is a special type of file. Linux uses a directory to hold information about other files. You can think of a directory as a container that holds other files or directories; it is equivalent to a folder in Windows or macOS.

A file is typically stored on physical storage media such as a disk (hard drive, flash disk, etc.). Every file must have a name because the operating system identifies files by their name. File names may contain any characters, although some special characters (such as spaces, quotes, and parenthesis) can make it difficult to access the file, so you should avoid them in filenames. On most common Linux variants, file names can be as long as 255 characters, so it is convenient to use descriptive names.

Files can hold any sequence of bytes; it is up to the user to choose the appropriate application to correctly interpret the file contents. Files can be human readable text organized line by line, a structured sequence only readable by a specific application, or a machine-readable byte sequence. Many programs interpret the contents of a file as having some special structure, such as a pdf or postscript file. In scientific computing, binary files are often used for efficiency storage and data access. Some other examples include scientific data formats like NetCDF or HDF which have specific formats and provide application programming interfaces (APIs) for reading and writing.

The Linux kernel is responsible for organizing processes and interacting with files; it allocates time and memory to each process and handles the file system and communications in response to system calls. The Linux system uses files to represent everything in the system: devices, internals to the kernel, configurations, etc.