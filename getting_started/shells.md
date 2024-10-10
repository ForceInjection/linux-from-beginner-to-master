# Shells

A variety of different shells are available for Linux and Unix, each with pros and cons. While **bash** (updated version of sh) and **tcsh** (descended from C-shell/csh) are the most common shells, the choice of shell is entirely up to user preference and availability on the system. In most Linux distributions, bash is the default shell.

The purpose of a shell is to interpret commands for the Operating System (OS) to execute. Since bash and other shells are scripting languages, a shell can also be used for programming via scripts. The shell is an interactive and customizable environment for the user.

All examples in this tutorial use the **bash shell**.


## Introduction to Shells

Tip: Don't type the prompt character

The shell prompt is where the characters you type will appear. In technical documentation, the prompt is typically represented by a `$` character. _Do not type_ the `$` when attempting to reproduce the examples.

The shell input to run a command typically follows a certain format:

1.  **command** – the executable (program or package) that is to be run.
    *   If you are running your own application, you must include either the full path or the relative path as part of the command.
    *   Most commands that come packaged with the OS or are installed by the package manager (executables often located in `/bin` or `/usr/bin`) do not need the path because they have already been added to the environment variable `$PATH`.
2.  **option(s)** – (A.K.A. "flags") optional arguments for the command that alter the behavior.
    *   Start with a `-` or `--` (example: `-h` or `--help` for help).
    *   Each command may have different options or no options at all.
    *   Some options require an argument immediately following.
    *   Explore options for commands by reading the [Manual Pages](#man-pages).
3.  **argument(s)** – depend on the command and the flags selected.
    *   Certain flags require an argument.
    *   Filename arguments must include a path unless located in the current directory.

### Basic Examples

To determine which shell you are currently using, you can type the `echo` command followed by the system environment variable `$SHELL` as follows:

```bash
$ echo $SHELL
/bin/bash
```

Here, `echo` is the command entered through the shell, and `$SHELL` is a command argument. The output is showing the location of the installed bash.

### Tab Completion

The bash shell includes an incredibly useful feature called [tab completion](https://en.wikipedia.org/wiki/Command-line_completion), which enables you to enter part of a command, press the `Tab` key, and it will complete the command if there is no ambiguity. If there are multiple options, hitting `Tab` again will display the possible options. Below is an example on Stampede2 where "py" was entered followed by `Tab`:

```bash
$ py
pydoc                  pygobject-codegen-2.0  pygtk-demo             pystuck                python2                python2.7-config       python-config
pygmentize             pygtk-codegen-2.0      pyjwt                  python                 python2.7              python2-config         
```
Use tab completion to finish commands, file names, and directory names. Try it out at any point and see how much it simplifies your workflow!

## Exercise: Working with the Shell

Try these shell commands at the prompt. Many of these commands have extensive optional arguments.

### Display `$PATH`, the value of the PATH variable

The **`PATH`** environment variable stores designated paths to executables; as a result, these executables can be executed without reference to their full paths. Commonly used paths are added to this environment variable by the system at startup. Later we will see how to add additional paths to the environment variable. Executables in directories included in `$PATH` are often referred to as being "in the path" of the current shell.

```bash
$ echo $PATH
/usr/local/bin:/usr/bin:/bin
```

### List the available shells in the system

The **`cat`** (concatenate) command is a standard Linux utility that concatenates and prints the content of a file to standard output (shell output). In this case, _shells_ is the name of the file, and _/etc/_ is the pathname of the directory where this file is stored.

```bash
$ cat /etc/shells
/bin/sh
/bin/bash
/sbin/nologin
/usr/bin/sh
/usr/bin/bash
/usr/sbin/nologin
/bin/tcsh
/bin/csh
/usr/bin/tmux
/bin/ksh
/bin/rksh
/bin/zsh
 ```   

### Find the current date and time of the system

Use the **`date`** command.

```bash
$ date
Fri Nov  9 19:23:23 CST 2018
```    

### List all of your own current running processes

Use the **`ps`** command (process status). In Linux, each process is associated with a PID or _process identifier_.

```bash
$ ps
  PID TTY          TIME CMD
  916 pts/58   00:00:00 bash
 1531 pts/58   00:00:00 ps
```    

## <span id="man-pages">Manual Pages</span>

The easiest way to get more information about a particular Linux command or program is to use the **`man`** command followed by the item you want information on:

This will bring up the manual page ("man page") for the program within the shell, which have been formatted from the online man pages. These pages can be referenced from any Linux or Unix shell where `man` is installed, which is most systems. Linux includes a built-in manual for nearly all commands, so these should be your go-to reference.

The manual is divided into a number of sections by type of topic, for example:

| **Section** | **Description**                                                                     |
|-------------|-------------------------------------------------------------------------------------|
| **1**       | Executable programs and shell commands                                              |
| **2**       | System calls (functions provided by the kernel)                                     |
| **3**       | Library calls (functions within program libraries)                                  |
| **4**       | Special files                                                                       |
| **5**       | File formats and conventions                                                        |
| **6**       | Games                                                                               |
| **7**       | Miscellaneous (including macro packages and conventions)                            |
| **8**       | System administration commands (usually only for root)                              |

If you specify a specific section when you issue the command, only that section of the manual will be displayed. For example, `man 2 mkdir` will display the Section 2 man page for the `mkdir` command. Section 1 for any command is displayed by default.

If your terminal does not support scrolling with the mouse, you can navigate the man pages by using the up and down arrow keys to scroll up and down or by using the enter key to advance a line and the space bar to advance a page. Use the q key to quit out of the manual.

The man pages follow a common layout. Within a man page, sections may include the following topics:

| **Topoc**       | **Description**                                                 |
|-----------------|-----------------------------------------------------------------|
| **NAME**        | a one-line description of what it does.                         |
| **DESCRIPTION** | describes the program's functionalities.                        |
| **OPTIONS**     | lists command line options that are available for this program. |
| **EXAMPLES**    | examples of some of the options available.                      |
| **SEE ALSO**    | list of related commands.                                       |

Example snippets from the man page for the `rm` (Remove) command:

```bash
$ man rm
RM(1)                            User Commands                           RM(1)
    
NAME
    rm - remove files or directories
    
SYNOPSIS
    rm [OPTION]... FILE...
    
DESCRIPTION
    This  man page documents the GNU version of rm.  rm removes each 
    specified file. By default, it does not remove directories.
    
     If the -I or --interactive=once option is given,  and  there  are  more
     than  three  files  or  the  -r,  -R, or --recursive are given, then rm
     prompts the user for whether to proceed with the entire operation.   If
     the response is not affirmative, the entire command is aborted.
```    

Depending on the command, the OPTIONS section can be quite lengthy:

```bash
OPTIONS
    Remove (unlink) the FILE(s).
    
       -f, --force
              ignore nonexistent files, never prompt
    
       -i     prompt before every removal
    
       -r, -R, --recursive
              remove directories and their contents recursively
    
       -v, --verbose
              explain what is being done
```   

**Fun fact:** there is even a manual entry for the `man` command. Try:

```bash
$ man man
```

Issuing the `man` command with the `-k` option will print the short man page descriptions for any pages that match the command. For example, if you are wondering if there is a manual entry for the `who` command:

```bash
$ man -k who
```
Since there is a man page listed, you can then display the man page for the `who` command with `man who`.

## Directory Navigation

### Directories

In a hierarchical file system like Linux, the root directory is the highest directory in the hierarchy, and in Linux this is the **`/`** directory. The home directory is created for the user, and is typically located at `/home/<username>`. Commonly used shorthands for the home directory are `~` or `$HOME`. The home directory is usually the initial default working directory when you open a shell.

The absolute path or full path details the entire path through the directory structure to get to a file, starting at `/`. A relative path is the path from where you are now (your present working directory) to the file in question. An easy way to know if a path is absolute is to check if it contains the `/` character at the very beginning of the path.

The "`.`" directory is a built-in shortcut for the current directory path and similarly the "`..`" directory is the directory above the current directory. These special shortcuts exist in every directory on the file system, except "`..`" does not exist in the root directory (`/`) because it is at the top. Files that begin with a dot "`.`" (i.e. `.bashrc`) are called dot files and are hidden by default during navigation (in the sense that the `ls` command will not display them), since they are usually used for user preferences or system configuration.

### Navigating

Here is a list of common commands used for navigating directories:


### pwd command

**`pwd`** – **p**rint **w**orking **d**irectory. Prints the full path to the directory you are in, starting with the root directory. On Stampede2 you might see:

```bash
$ pwd
/home1/05574/<username>
```

### ls command

**`ls`** – **l**i**s**ts the contents of a directory.

```bash
$ ls
test1.txt  test2.txt  test3.txt
```    

*   Displays the files in the current directory or any directory specified with a path.
*   Use the wildcard `*` followed by a file extension to view all files of a specific type (i.e. `ls *.c` to display all C code files).
*   Use the `-a` option to display **a**ll files, including dot files.
*   There are many options for this command, so be sure to check the man pages. A Stampede2 example:

```bash
$ ls -lha $SCRATCH
total 12K
drwx------ 3 <username> G-819251 4.0K Jul  9 14:57 .
drwxr-xr-x 5 root       root     4.0K May 14 16:13 ..
drwx------ 2 <username> G-819251 4.0K Jul  9 14:57 .slurm
```

### cd command

**`cd`** – **c**hange **d**irectory to the directory or path following the command. The following command will take you from your current directory to your home directory on most Linux systems:

```bash
$ cd ~ 
```

*   This example will take you up one directory, in this case to the root directory `/` , and then over to the `var` directory:

```bash
$ cd ../var
```    
*   With no arguments, `cd` will take you back to your home directory.

## Interacting with Files and Directories

### Interacting

Here is a list of common commands used for interacting with files and directories:

#### mkdir command

**`mkdir`** – **m**a**k**e a new **dir**ectory of the given name, as permissions allow.

```bash
$ mkdir Newdir
```

#### mv command

**`mv`** – **m**o**v**e files, directories, or both to a new location.

```bash
$ mv file1 Newdir
```

This can also be used to rename files:

```bash
$ mv file1 file2
```

Use wildcards like `*` to move all files of a specific type to a new location:

```bash
$ mv *.c ../CodeDir
```

You can always verify that the file was moved with `ls`.

#### cp command

**`cp`** – **c**o**p**y files, directories, or both to a new location.

```bash
$ cp file1 ~/
```

You can give the copy a different name than the original in the same command:

```bash
$ cp file1 file1_copy
```

To copy a directory, use the `-r` option (**r**ecursively). In this case, both the source and the destination are directories, and must already exist.

```bash
$ cp -r Test1 ~/testresults
```

#### rm command

**`rm`** – **r**e**m**oves files or directories **_permanently_** from the system.

> **Warning**: Linux does not typically have a "trash bin" or equivalent as on other OS

When you issue `rm` to remove a file, it is difficult to impossible to recover removed files without resorting to backup restore.

```bash
$ rm file1
```

With the `-r` or `-R` option, it will also remove/delete entire directories recursively and permanently.

```bash
$ rm -r Junk
```

> **Caution**: Avoid using wildcards like `*`.

For instance, the example below will remove all of the files and subdirectories within your current directory, so _**use with caution.**_

```bash
$ rm -r *
```

To remove an empty directory, use `rmdir`

#### touch command

**`touch`** – changes a file's modification timestamp without editing the contents of the file. It is also useful for creating an empty file when the filename given does not exist.

Try these commands to get more familiar with Linux files and directories.

## I/O and Redirection

### Input and Output (I/O)

As the title of this section suggests, I/O stands for input/output. Your commands or programs will often have input and/or output. It is important to know how to specify where your input is from or to redirect where output should go; for example, you may want your output to go to a file rather than printing to the screen. Inputs and outputs of a program are called streams in Linux. There are three types of streams:

| **Type**   | **Description**                                                       |
|-----------------|------------------------------------------------------------------|
| **stdin**  | standard input - the stream of data going into a program.  By default, this is input from the keyboard. |
| **stdout** | standard output - the output stream where data is written out by a program.  By default, this output is sent to the screen. |
| **stderr** | standard error - another output stream (independent of stdout) where programs output error messages.  By default, error output is sent to the screen. |

### Output Redirection

It is often useful to save the output (stdout) from a program to a file. This can be done with the redirection operator **`>`**.

```bash
$ example_program > my_output.txt
```

For another example, imagine that you run the `ls` command on a directory that has so many files that your screen scrolls and you cannot see all of the files listed. You might want to redirect that output to a file so you can open it up in a text editor and look more closely at the output:

```bash
$ ls > output_file.txt
```

Redirection of this sort will create the named file if it doesn't exist, or else overwrite the existing file of the same name. If you know the file already exists (or even if it does not), you can append the output file instead of rewriting it using the redirection operator **`>>`**.

```bash
$ ls >> output_file
```

### Input Redirection

Input can also be given to a command from a file instead of typing it in the shell by using the redirection operator **`<`**.

```bash
$ mycommand < programinput
```

Alternatively, you can use the pipe operator **`|`** like this:

```bash
$ cat programinput | mycommand
```

Using the pipe operator `|`, you can link commands together. _The pipe will link stdout from one command to stdin of another command_. In the above example, we use the `cat` command to print the file to the screen (stdout), and then we redirect that printing to the command `mycommand`.

### Error Redirection

When performing normal redirection of the standard output of a program (stdout), stderr will not be redirected because it is a separate stream. Many programmers find it useful to redirect only stderr to a separate file. You might do this to make it easier to find the error messages from your program. This can be accomplished in the shell with a redirection operator **`2>`**.

```bash
$ command 2> my_error_file
```

In addition, you can merge stderr with stdout by using **`2>&1`**.

```bash
$ command > combined_output_file 2>&1
```

### Redirect _and_ Save Output

Redirecting the output of a command to a file is useful, but it means that you will not see anything on the screen while it is running. This can be undesirable, especially for long-running commands. To have the output go to both a file _and_ the screen, use the `tee` command:

```bash
$ command | tee outputfile
```

You can also use `tee` to catch stderr with:

```bash
$ command 2>&1 | tee outputfile
```

## Exercise: Redirecting Output

### Viewing Output

Use **`ls`** (list files) and **`>`** (redirect) to create a file named "mylist" which contains a list of your files.

```bash
$ ls -l /etc > mylist
```

There are three main methods for viewing a file from the command prompt. Try each of these on your "mylist" file to get a feel for how they work:

*   **`cat`** shows the contents of the _entire file_ at the terminal and scrolls automatically to the end.

	```bash
	$ cat mylist
	```
*   **`more`** shows the contents of the file, pausing when it fills the screen.
    ```bash
    $ more mylist
    ```
    *   Note that it reads the entire file before displaying, so it could take a long time to load for large files.
    *   Use the spacebar to advance one page at a time.
*   **`less`** is similar to `more`, but with more features. It shows the contents of the file, pausing when it fills the screen.
    ```bash
    $ less mylist
    ```
    *   Note that `less` is **faster** than `more` on large files because it does not read the entire input file before displaying.
    *   Use the spacebar to advance one page at a time, or use the arrow keys to scroll one line at a time. Enter **q** to quit. Entering **g** or **G** will take you to the beginning or end of the file, respectively.
    *   You can also **search** within a file (similar to Vim) by typing **/** and the word or characters you are searching for (example: **/foo** will search for "foo"). `less` will jump to the first match for the word. Move between matches by using **n** and **?** keys.

> **Tip**: Reference the man pages

> It may also be useful to explore the man pages for `head` and `tail` and try them out, especially in conjunction with these viewing methods.

### Combining Redirection and Viewing

Now let's try an exercise where we enter the famous quote "Four score and seven years ago" from Lincoln's Gettysburg address into a file called "lincoln.txt". First, use `cat` to direct stdin to "lincoln.txt":

```bash
$ cat > lincoln.txt
```

Next, enter the quote above. To end the text input, press **Control-D**.

```bash
Four score and seven years ago
[Control-D]
```    

Finally, you can use `cat` to view the file you just created:

```bash
$ cat lincoln.txt
Four score and seven years ago
```    

Now try adding another line of the famous quote to the existing file:

```bash
$ cat >> lincoln.txt
our fathers brought forth on this continent, a new nation
[Control-D]
```

If you wish, you could try appending the rest of the speech to the file. Finally, try viewing the file in both `more` and `less` to test them out. Feel free to test navigation in both and try searching with `less`. If you have a longer file, try viewing that as well so you can get used to scrolling.

### Creating a Simple Script

We can also redirect input to a script file that we create and then run the script. First, we will create the script file called "tryme.sh" that contains the `cat` command without any arguments, forcing it to read from stdin.

```bash
$ cat > tryme.sh
#!/bin/sh
cat
[Control-D]
```

The first line of the script `#!/bin/sh` indicates which shell interpreter to use. `/bin/sh` is a special sort of file, called a _symlink_, which points at the default interpreter. You can see where it points by:

```bash
$ ls -l /bin/sh
```

The default is often `/bin/bash`, but you can also specify to use bash (or another shell) directly by replacing the line with the location of bash on your system, which is usually `#!/bin/bash`.

Next, we can execute the script using the `source` command, and redirect the "lincoln.txt" file to stdin. This will cause the script to execute the `cat` command with the contents of "lincoln.txt" as input, consequently printing it to the screen (via stdout):

```bash
$ source tryme.sh < lincoln.txt
Four score and seven years ago
our fathers brought forth on this continent, a new nation
```

If you omit the redirection character `<`, the script will try to read from stdin (keyboard input) and then immediately print it back out.

## Searching

### Locating Files With `find`

The `find` command provides a wide range of capabilities for searching through directory trees, including executing commands on found files, searching for files based on creation and modification times, and more. It will search any set of directories you specify for files that match the criteria. For example, you might have thousands of files in your home directory and be looking for a file named foo:

```bash
$ pwd
/home/jolo
    
$ find . -name foo 
./foo
```

In the example above, the first argument "`.`" indicates for `find` to start searching in the current directory (/home/jolo), and the flag `-name` with the argument `foo` means to search for a file named "foo". Find returns the relative path of the file "foo" when it finds it in the file system. In this case, the file was found right in the home directory.

You can also specify more than one location to search:

```bash
$ find /home/jolo/Project /home/jolo/Results/ . $HOME -name foo
```

This searches for the file name "foo" in the "/home/jolo/Project/", "/home/jolo/Results/", and the current directory.

### Locating Files With `locate`

Another command provided on most Linux systems is the `locate` command, which builds a file-based database of files and their locations and will match strings. `locate` is _usually faster_ than `find` because it searches the database, rather than looking in each directory and subdirectory. You can use `locate myfile` in order to find where the file is located. Try `locate -h` for a full list of options.

### Pattern Matching With `grep`

The `grep` (global regular expression print) command is another useful utility that searches the named input file for lines that match the given pattern and prints those matching lines. In the following example, `grep` searches for instances of the word "bar" in the file "foo":

```bash
$ cat foo
tool
bar
cats
dogs
    
$ grep bar foo
bar
```
If there are no matches, `grep` will not print anything to the screen.

## Exercise: Piping, Sorting, and Counting

### Piping

The pipe operator (`|`) _sends the output of the command to its left to the input of the command on its right_, thereby chaining simple commands together to perform more complex processing than a single command can do. It is similar to the redirection operator but directs the output to the next command instead of a file. Most Linux commands will read from stdin and write to stdout instead of only using a file, so `|` can be a very useful tool.

Another useful bash command is `history`, which will print all the bash commands you've entered in the shell on the system to stdout (up to a maximum set by the system administrator). This can be very useful when you only remember one part of a command or forget exact syntax, but it will quickly become more daunting to search the output the longer you use the shell . You can use `|` combined with `grep` to quickly and easily search this output. First try just the `history` command to view the normal output, then try searching for the `cat` command, which we've used several times, like this:

```bash
$ history | grep cat
1901  cat /etc/shells
1927  cat programinput | mycommand
1929  cat mylist
1930  cat > lincoln.txt
1931  cat lincoln.txt
1932  cat >> lincoln.txt
1933  cat > tryme.sh
1936  cat foo
1937  history | grep cat
```

As you can see, it is much easier to find specific past commands by this method. The numbers before the commands indicate the line number in the bash history file, which corresponds to when the command was entered, as long as your history has not been cleared. You can even redirect this output to a file by:

```bash
$ history | grep cat > cat_history.txt
```

This file can then be searched using `less`, a text editor (covered later), or by the grep command. For example, you can use grep to search for "lincoln":

```bash
$ cat cat_history.txt | grep lincoln
 1930  cat > lincoln.txt
 1931  cat lincoln.txt
 1932  cat >> lincoln.txt
```

Here's another example. Say you wanted to identify the processes using `bash`; you could use `ps -ef` (which outputs _all_ processes in full format) as input to `grep` like so:

```bash
$ ps -ef | grep bash
```

There are many ways to use `|`, as you have seen in these examples, but feel free to explore more options!

### Sorting

The `sort` command sorts the content of a file or any stdin, and prints the sorted list to the screen.

```bash
$ cat temp.txt 
cherry
apple
x-ray
clock
orange
bananna
    
$ sort temp.txt
apple
bananna
cherry
clock
orange
x-ray
```    

To see the sorted list in reverse order, use the `-r` option. `sort -n` will sort the output numerically rather than alphabetically.

```bash    
$ cat temp2.txt
7
48
1
56
8
32
    
$ sort -nr temp2.txt
56
48
32
8
7
1
```    

Note that the two options can be combined by `-nr`, and order does not matter unless there is an input for a particular option. If you were looking for a filename that began with a "w", you may try:

```bash
$ ls | sort -r
```

### Counting

The `wc` command reads either stdin or a list of files and generates a few different count summaries:

*   numbers of lines (by counting the number of newline characters)
*   numbers of words
*   numbers of bytes

Using the file temp.txt from the previous example, we can use `wc` to count the lines, words, and bytes (or characters):

```bash
$ wc temp.txt 
 6  6 40 temp.txt
```    

The output shows that there are **6** lines, **6** words, and **40** bytes (or characters) in the file temp.txt. You can also use the following options with `wc` to specify certain behavior:

*   Only display line count: `-l`

    ```bash
    $ wc -l temp.txt
    6 temp.txt
    ```
    
*   Only display word count: `-w`
    
    ```bash
    $ wc -w temp.txt
    6 temp.txt
    ```
    
*   Only display byte count: `-c`
    
    ```bash
        $ wc -c temp.txt
        40 temp.txt
    ```

You can pipe `ls` to `wc -l` to list the number of files in a directory:

```bash
$ ls | wc -l
```

## Job Control

In addition to starting commands, the shell provides basic job control functions for processes. For shell sessions, such as interactive sessions on Stampede2, it can be useful to see and control processes that run for longer times. Job control allows the user to stop, suspend, and resume jobs from within the shell. This is useful if you have a program that runs longer than desired, does not complete due to a bug, or has other problems. From within a shell session, the `ps` command will show the current processes running in your shell session.

```bash
$ ps
  PID TTY          TIME CMD
 4621 pts/7    00:00:00 bash
32273 pts/7    00:00:00 ps
```    

From within a running process, using Ctrl C will send an interrupt signal to the process, which will usually cause it to terminate (I/O and other factors can block the interrupt from taking effect immediately). If you have executed a long-running program that you want to complete, but you want to do other things in the same shell while it runs (rather than starting a new shell on the system), you can suspend the process by pressing Ctrl Z. It can be resumed in the **b**ack**g**round with `bg`. Similarly, a process can be invoked and immediately sent to the background by adding an `&` at the end of the command:

```bash
$ mylong-runningcode.o &
```

A running job can be brought to the **f**ore**g**round with `fg`:

```bash
$ ./longscript.sh &
$ fg
./longscript.sh
```

While using job control in the shell you can also use the `jobs` command to display currently running jobs, similar to `ps`.

```bash
$ ./longscript.sh &
$ jobs
[1]+  Running                 ./longscript.sh &
$ fg
./longscript.sh
```

In the example above, we invoke `longscript.sh &` and immediately send it to the background. The `jobs` command shows the list of running jobs under shell control. Using `fg`, we can bring "longscript.sh" back to the foreground. See the manual page (`man fg`) for details about how to specify which job will be brought to the foreground if there is more than one background job.

You can also use the `top` command to view details about running processes. The program [`htop`](https://hisham.hm/htop/) is a common, more interactive, alternative to `top` that is not installed by default on most Linux systems, but is worth exploring. And finally, use the `kill` command followed by a PID to stop a process. For more information on these job control commands, see their respective man pages.

## Environment Variables

As mentioned previously, the environment variable `PATH` stores a list of directory paths that the shell will search, in order, when you issue a command. `$PATH` refers to the _value assigned to_ this variable. The paths in `$PATH` may be modified by the OS, a system administrator, environment management tools, or directly by the user. You can view the current list of directories with `echo $PATH`.

If a command you issue is not found in any of the paths in `$PATH`, then the shell will not be able to execute it, unless it is prefaced by the full path to the command. You can change the directories in the list by assigning a new value to `PATH`. Since `PATH` is an environment variable, you typically do this through the `export` command, which ensures that the new `$PATH` value is passed down to subshells of the current shell.

### Add directory to path

To _add_ directories to your path, you can use:

```bash
$ export PATH=$PATH:/path/to/new/command
```

Again, the **`$PATH`** on the right-hand side of the `=` means "the current value of the `PATH` variable." The **`:`** joins directories in the path variable together. Inserting **`$PATH:`** before the directory you want to add has the effect of appending the new directory to the list of existing directory paths. If you want your additional directory to be the first one searched for commands, put it immediately after the `=` and follow it with **`:$PATH`**. Note that no spaces should appear on either side of the `=` sign.

### Redefine path

If you wanted to **completely replace** the list with a different path:

```bash
$ export PATH=/path/to/replacement/directory
```

> Warning: This command **completely replaces** the previous path.

> Please keep in mind that using `export PATH=/path/to/replacement/directory` will _erase_ the previous list of directories in your path.

Try `env` to list all environment variables. Examples such as `SHELL`, `HOME`, and `PATH` are built-in environment variables. Any variable can be declared to the shell, just locally, by typing

```bash
$ MYVAR=something
```

To make a shell variable into an environment variable, you need to `export` it. This places the variable into the environment of the current shell, plus any subshells that are started from that shell. You can `export` an existing shell variable, or you can create and export a shell variable in a single line:

```bash
$ export MYVAR
$ export MYVAR2=something_else
```    

Conventionally, shell variables are declared in all capital letters. The value of the variable can then be used in bash commands or scripts by inserting `$MYVAR`. Note again the use of a preceding **`$`** when using the variable and the absence of the **`$`** prefix when setting the variable value. If you want to insert the variable inside a string, it is safest to do this by using the form `${MYVAR}` and placing the beginning and ending portions of the string on either side. This cleanly distinguishes the name of the variable from any neighboring characters in the string. For example:

```bash
$ DATAPATH=/home/jolo/Project
$ ls $DATAPATH
$ cp newdatafile.txt ${DATAPATH}/todaysdatafile.txt
$ ls $DATAPATH
todaysdatafile.txt
```