# Text Editors

A text editor is a tool to assist the user with creating and editing files. There is no "best" text editor; it depends on personal preferences. Regardless of your typical workflow, you will likely need to be proficient in using at least one common text editor if you are using Linux for scientific computing or similar work. Two of the most widely used command-line editors are **Vim** and **Emacs**, both of which are available on TACC systems via the `vim` and `emacs` commands, respectively.

## Overview

This topic is a brief introduction to the basics of both Vim and Emacs. It is recommended that you try each one and work through testing each of the commands to select which one works best for your workflow. Additional editors on Stampede2 you may want to consider are [nano](https://www.nano-editor.org/), a simple text editor designed for new users, or [gedit](https://help.gnome.org/users/gedit/stable/), a general-purpose text editor focused on simplicity and ease of use, with a simple GUI. There are also many more, so feel free to explore other options.

Each text editor in Linux has a designed workflow to assist you in editing, and some workflows work better than others depending on your preferences. For example, Emacs relies heavily on key-chords (or multiple key strokes), while Vim uses distinct editing [modes](https://en.wikibooks.org/wiki/Learning_the_vi_Editor/Vim/Modes#Modes). Vim users tend to enter and exit the editor repeatedly and use the shell for complex tasks, while Emacs users typically remain within the editor and use Emacs itself for [complex tasks](https://www.gnu.org/software/emacs/manual/html_node/emacs/Shell.html). Most users develop a preference for one text editor and stick with it.

### What about Vi?

The Vi editor (command is `vi`) is the predecessor of Vim. Most modern systems actually alias `vi` to `vim` so that you are using `vim` whenever you enter the `vi` command. You can determine if this is the case by entering the following:

```bash
$ which vi
alias vi='vim'
    /bin/vim
```    

The line `alias vi='vim'` tells you that `vim` will be executed whenever the command `vi` is entered. The above output is actually from Stampede2. For this reason, we will focus on Vim in this tutorial and not Vi. We will also provide a basic overview of Emacs.

## Vim

![logo for Vim: The editor](img/vim_editor.gif)

Source: [vim.org](https://www.vim.org/logos.php)

### Basic Functions

*   **Open** an existing file by entering **`vim`** in the shell followed by the name of the file.
*   **Create** a new file in the same way as opening a file by specifying the new filename. The new file will not be saved unless specified.
*   **Save** a file that is currently open by entering the **`:w`** command.
*   **Quit** an open file by entering the **`:q`** command. If you have made any edits without saving, you will see an error message. If you wish to _quit without saving_ the edits, use **`:q!`**.
*   **Save and Quit** at the same time by combining the commands: **`:wq`**.
*   **Edit** the file by entering insert mode to add and remove text. Entering into normal mode will allow you to easily copy, paste, and delete (as well as other functionality).
*   **Cancel** a command before completely entering it by hitting **`Esc`** twice.

### Normal (Command) Mode

Vim starts in normal mode, and returns to normal mode whenever you exit another mode. When in normal mode, there is no text at the bottom of the shell, except the commands you are entering.

### Navigation

Navigation in normal mode has a large number of shortcuts and extra features, which we will only cover some of here. Basic movement can be done using the arrow keys or using the letter keys as shown in the table.

Keys for basic movement.

| **Move** | **Key** |
|---|---|
| **&larr;** | h |
| **&darr;** | j |
| **&uarr;** | k |
| **&rarr;** | l |

The benefits of using the alternate keys is that you do not have to move your hand back-and-forth to the arrow keys while in this mode, and can more effectively enter Vim commands (once you are practiced). Some other examples of navigation shortcuts include:

*   Move to the **beginning of the line**: `0`
*   Move to the **end of the line**: `$`
*   Move to the **beginning of the next word**: `w` This can also be used with a number to move multiple words at once (i.e. `5w` moves 5 words forward).
*   Move to the **end of the current word**: `e` This can be used with a number in the same way that `w` can to move multiple words at once.

These extra navigation shortcuts become powerful when combined with other Vim functions, allowing you to edit text and navigate through the file without changing modes.

### Insert Mode

When you first open a document, you will always start in normal mode and have to enter insert mode. To enter insert mode where the cursor is currently located in the file, press the letter **`i`** or the **`Insert`** key. Additionally, you can press the letter **`a`** (for append) if you would like to enter insert mode at the character after the cursor. To exit insert mode, press the **`Esc`** key. When in insert mode, `-- INSERT --` will be visible at the bottom of the shell. Navigation in insert mode is done with the standard arrow keys.

### Editing Features

Here are some important commands to know:

*   **Undo** the previous command, even the last edit in insert mode, with the command **`u`**
*   **Redo** the previous command (after undo) with **`Ctrl-R`**
*   **Copy** (yank) characters, words, or lines:
    *   **`yl`** to copy a single character under the cursor
    *   **`yy`** to copy the current line
    *   **`y#y`** or `#yy` where `#` is replaced with the number of lines you want to copy (i.e. `y25y` will copy 25 lines).
*   **Paste** (put) characters, words, or lines:
    *   **`p`** will paste after the cursor for characters and words, or on the next line (regardless of the cursor location within a line) if you are pasting lines.
    *   **`P`** will paste before the cursor for characters and words, or on the preceding line (regardless of the cursor location within a line) if you are pasting lines.
*   **Delete** or **Cut** characters, words, or lines (that can then be pasted elsewhere):
    *   **`x`** to delete a single character under the cursor
    *   **`dd`** to delete the current line
    *   **`d#d`** or `#dd` where `#` is replaced with the number of lines you want to delete (similar to copy).
*   **Search** for strings throughout a file and optionally **replace**:
    *   A basic search for a word is simply **`/word`** followed by `Enter`. This will jump to the first occurrence of the word after the cursor. Phrases can also be used.
    *   Once a search is active, you can use **`n`** to jump to the next occurrence and **`N`** to jump to the previous occurrence.
    *   [Search and replace](https://vim.wikia.com/wiki/Search_and_replace) has many options, but one example is to find all occurrences of "foo" in the file and replace (substitute) them with "bar" with the command: **`:%s/foo/bar/g`**
*   **Split** the screen _vertically_ or _horizontally_ to view multiple files at once in the same shell:
    *   **`:sp <filename>`** will open the specified file above the current active file and split the screen horizontally.
    *   **`:vsp <filename>`** will open the specified file to the left of the current active file and split the screen vertically
    *   Navigate between split-screen files by pressing **`Ctrl-W`** followed by navigation keys (i.e. `Ctrl-W h` or `Ctrl-W ←` to move to the left file)
    *   Also note that you can open several documents at once from the shell using appropriate flags. See `man vim` for more information.

Any of the editing commands can easily be combined with navigation commands. For example, `5de` will delete the next 5 words, or `y$` will copy from the current cursor location to the end of the line. There are a large number of combinations and possible commands. Note that copying, pasting, and deleting can also be done efficiently using visual mode.

### Visual Mode

From normal mode, press the **`v`** key to enter visual mode. This mode enables you to highlight words in sections to perform commands on them, such as copy or delete. Navigation in visual mode is done with the normal mode navigation keys or the standard arrow keys. For example, if you are in normal mode and you want to copy a few words from a single line and paste them on another line:

1.  Navigate to the first character of the first word you want to copy
2.  Enter visual mode by `v`
3.  Navigate to the last character of the last word you want to copy (this should highlight all the words you want)
4.  Enter `y` to copy the words
5.  Navigate to where you want to paste the words
6.  Enter `p` to paste

Note that step 6 will paste after the cursor instead of on the next line even if you have copied several lines. You can also replace that step with `P` to paste before the cursor.

### Vim and the Shell

Working with Vim regularly can mean switching back and forth between it and the shell, but there are two ways to simplify this. From normal mode, you can use the command **`:!`** followed by any shell command to execute a single command without closing the file. For example, `:!ls` will display the contents of the current directory. This will appear to background Vim while executing the command (so you can see the shell and output), and display the following message upon completion:

```bash
Press ENTER or type command to continue
```

Pressing `Enter` will return you to your open file. Alternatively, you can simply background the file while in normal mode with **`Ctrl-Z`** to view the shell and issue commands. When you want to return to the file, use the foreground command **`fg`**. In this way, you can actually have a number of files open (with or without splitting the screen) all in the same shell, and easily switch between them. Note that if you background multiple files, the foreground command will bring them up in reverse order (most recent file accessed first).

### Customization

Vim uses a `.vimrc` file for customizations. Essentially, this file is to consist of Vim commands that you would like issued each time you open Vim to customize your experience. One example of a command you will likely want is `syntax on`, which provides syntax highlighting for programming languages. There are also a number of commands you can explore to customize the coloring of the syntax. Here is an example of a simple `.vimrc` file that you may use:

```bash
syntax on
set tabstop=4
set expandtab
set number
set hls
```

In addition to syntax highlighting, the above customizations will set tabs to be 4 characters wide, replace tabs with spaces, show line numbers along the left-hand side of the screen, and highlight matching words when searching. There is a global `vimrc` file that sets system-wide Vim initializations (you will not have access to this on Stampede2), and each user has their own located at `~/.vimrc` wich can be used for personal initializations.

### A Hands-On Tutorial

One of the most effective ways to learn Vim is through the built-in hands-on tutorial that can be accessed via the shell by the command **`vimtutor`**. This command will open a text file in Vim that will walk you through all the major functionalities of Vim as well as a few useful tips and tricks. If you plan to use Vim even occasionally, it is a great resource. Furthermore, the above list of features and commands is not exhaustive, and the interested new Vim user should certainly explore the man pages and online resources to discover more Vim features.

## Emacs

![Emacs logo](img/emacs.png)

Source: [gnu.org](https://www.gnu.org/software/emacs/emacs.html)

### Basic Functionality

*   **Open** an existing file by entering **`emacs`** in the shell followed by the name of the file. This will default to running Emacs in a GUI, but it can also be run within the shell (`emacs -nw`). Note that to use the GUI with a remote connection such as Stampede2, you must use X11 forwarding (covered in the Remote Connections section), otherwise the `emacs` command will open within the shell. If you want to run the GUI and keep the shell free, you can open and background Emacs with `emacs &`. Use `Ctrl-x f` to open a file from within Emacs.
*   **Create** a new file in the same way as opening a file by specifying the new filename. The new file will not be saved unless specified.
*   **Save** a file that is currently open by entering the **`Ctrl-x Ctrl-s`** command.
*   **Quit** by entering **`Ctrl-x Ctrl-c`**.
*   **Save and Quit** is the same command as quitting, except that when you have unsaved files it will ask if you would like to save each one. To save, enter `y`.
*   **Edit** a file by simply entering and removing text.
*   **Cancel** a command before completely entering it or a command that is executing with **`Ctrl-g`** or by hitting **`Esc`** 3 times.

### Navigation

Similar to Vim, navigation in Emacs has shortcuts and extra features. Basic movement can be done using the arrow keys or using the letter keys in the following table:

Keys for basic movement.

| **Move** | **Command** |
|---|---|
| **&larr;** | Ctrl-b |
| **&darr;** | Ctrl-n |
| **&uarr;** | Ctrl-p |
| **&rarr;** | Ctrl-f |

The benefits of using the alternate keys is that you do not have to move your hand back-and-forth to the arrow keys, and can more effectively enter Emacs commands (once you are practiced). Some other examples of navigation shortcuts include:

*   Move to the **next screen view**: `Ctrl-v`
*   Move to the **previous screen view**: `Alt-v`
*   Move to the **next word**: `Alt-f` This can also be used with a number to move multiple words at once (i.e. `Alt-5f` moves 5 words forward).
*   Move to the **previous word**: `Alt-b` This can be used with a number in the same way to move multiple words at once.
*   Move to the **beginning of the line**: `Ctrl-a`
*   Move to the **end of the line**: `Ctrl-e`
*   Move to the **beginning of a sentence**: `Alt-a`
*   Move to the **end of a sentence**: `Alt-e`

Note that the more customary keys `Page Up`, `Page Down`, `Home`, and `End` all work as expected.

### Editing Features

Here are some important commands to know:

*   **Undo** the previous command with the command **`Ctrl-x u`**
*   **Redo** the previous command (after undo) by performing a non-editing command (such as `Ctrl-f`), and then undo the undo with `Ctrl-x u`
*   **Delete** or **Cut** characters, words, or lines (that can then be pasted elsewhere):
    *   **`Backspace`** to delete a single character before the cursor
    *   **`Ctrl-d`** to delete a single character after the cursor
    *   **`Alt-Backspace`** to delete the word before the cursor
    *   **`Alt-d`** to delete the word after the cursor
    *   **`Ctrl-k`** to delete from the cursor to end of the line
    *   **`Alt-k`** to delete from the cursor to end of the sentence
*   **Paste** characters, words, or lines:
    *   **`Ctrl-y`** pastes the most recent deleted text
    *   **`Alt-y`** pastes the deleted text before the most recent
*   **Copy** characters, words, or lines: The easiest way to copy is actually to _cut_ the text and then paste it back where it was. Then it can be pasted in a new location also.
*   **Search** for strings throughout a file and optionally **replace**:
    *   **`Ctrl-s`** starts a forward search that is incremental (each character you enter updates the search). Entering `Ctrl-s` again skips to the next occurrence. `Enter` ends the search.
    *   **`Ctrl-r`** starts a backwards search that behaves similarly to the forward search.
    *   [Search and replace](https://www.gnu.org/software/emacs/manual/html_node/emacs/Search.html#Search) has many options, but one example is to find all occurrences of "foo" in the file and replace them with "bar" with the command: **`Alt-x replace-string foo Enter bar`**
    *   You can use tab-completion for entering commands after typing `Alt -x`. For example, type `Alt-x`, then `rep`, then hit `Tab` twice to see a list of matching commands. Since similar commands are named similarly, you will find other useful related commands, such as `replace-regexp`.
*   **Split** the screen _vertically_ or _horizontally_ to view multiple files at once in emacs:
    *   **`Ctrl-x 3`** will split the screen horizontally
    *   **`Ctrl-x 2`** will split the screen vertically
    *   **`Ctrl-x 1`** closes all panes except the active one
    *   **`Ctrl-x 0`** closes a pane

### Highlighting Mode

This mode enables you to highlight words in sections to perform commands on them, such as copy or delete. For example, if you want to copy a few words from a single line and paste them on another line:

1.  Navigate to the first character of the first word you want to copy
2.  Enter highlighting mode by `Ctrl-Space`
3.  Navigate to the last character of the last word you want to copy (this should highlight all the words you want)
4.  Enter `Alt-w` to copy the words
5.  Navigate to where you want to paste the words
6.  Enter `Ctrl-y` to paste

### Emacs and the Shell

There are several options for running shell commands from Emacs. To execute a single shell command while in Emacs, use the command **`Alt-!`** followed by the shell command and hit `Enter`. The output of the command will display in a portion of the screen called an [echo area](https://www.gnu.org/software/emacs/manual/html_node/emacs/Echo-Area.html). There are several more features for running shell commands, including running an interactive shell inside Emacs (we recommend [ansi-term](https://www.emacswiki.org/emacs/AnsiTerm)). For more about these features, please see the [Emacs documentation](https://www.gnu.org/software/emacs/manual/html_node/emacs/Shell.html) on the topic. Alternatively, you can suspend Emacs with the command **`Ctrl-z`**. As with suspending Vim, you can execute commands in the shell, and then return to Emacs with the foreground command **`fg`**.

### Customization

Emacs is customizable in many ways including changing the [key bindings](https://www.gnu.org/software/emacs/manual/html_node/emacs/Key-Bindings.html) for commands, the color scheme (themes), and more. Due to the breadth of options, we refer you to existing documentation on [customization](https://www.gnu.org/software/emacs/manual/html_node/emacs/Customization.html).

### A Hands-On Tutorial

One of the most effective ways to learn Emacs is through the built-in hands-on tutorial that can be accessed by opening Emacs without any filename input. It will walk you through all the major functionalities of Emacs as well as a few useful tips and tricks. If you plan to use Emacs even occasionally, it is a great resource. Furthermore, the above list of features and commands is not exhaustive, and the interested new Emacs user should certainly explore the man pages and online resources to discover more Emacs features. In particular, [buffers](https://www.gnu.org/software/emacs/manual/html_node/emacs/Buffers.html) are a useful concept to understand when using Emacs, but are not covered here.

Additionally, you may want to consider looking into [spacemacs](https://www.spacemacs.org/) if you are familiar with Vim key bindings or would like to continue using emacs with more customization. Users can install spacemacs to their local directory on Stampede2 using git.