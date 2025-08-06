# Linux `set` 命令详解

`set` 命令用于显示和设置 shell 及 Linux 环境中的各种变量、选项。它广泛适用于不同的 shell，例如 Bourne shell (sh)、C shell (csh) 和 Korn shell (ksh)，但在 Bash 中更常用。`set` 提供了强大的控制功能，使用户可以改变 shell 的行为，尤其适合调试、错误处理、环境配置等场景。

## 1. 基本语法

```bash
set [options] [arguments]
```

- **options**：影响 shell 操作方式的选项标志。
- **arguments**：设置位置参数（即 `$1`, `$2`, 等）的值。

## 2. 选项详解

### 2.1 常用选项

| 选项 | `-o` 标志 | 描述 |
|------|-----------|------|
| `-a` | `-o allexport` | 标记所有创建或修改的变量或函数为导出，以便子 shell 使用。 |
| `-b` | `-o notify` | 后台作业终止时立即通知用户。 |
| `-e` | `-o errexit` | 当命令失败（非零退出状态）时，使 shell 立即退出。 |
| `-f` | `-o noglob` | 禁用文件名通配符展开（globbing）。 |
| `-h` | `-o hashall` | 在定义函数时定位并保存函数命令。默认启用。 |
| `-k` | `-o keyword` | 将所有赋值参数放入命令的环境，而不仅限于命令名之前的参数。 |
| `-n` | `-o noexec` | 读取命令但不执行它们，常用于检查脚本语法。 |
| `-m` | `-o monitor` | 任务完成时显示通知。默认在交互模式中启用。 |
| `-p` | `-o privileged` | 禁用 `$ENV` 文件处理和 shell 函数导入。当真实用户 ID 和有效用户 ID 不同时，默认启用。关闭时将有效用户 ID 和组 ID 设置为真实用户 ID 和组 ID。 |
| `-t` | `-o onecmd` | 读取一条命令后退出。 |
| `-u` | `-o nounset` | 将未定义的变量视为错误，防止使用未设置的变量。 |
| `-v` | `-o verbose` | 逐行打印 shell 输入内容，便于调试。 |
| `-x` | `-o xtrace` | 执行时打印每条命令及其参数，常用于调试。 |
| `-B` | `-o braceexpand` | 启用 shell 大括号扩展（如 `{1..5}`）。 |
| `-C` | `-o noclobber` | 通过重定向时防止覆盖现有文件。 |
| `-E` | `-o errtrace` | 使 shell 函数继承 `ERR` 捕获（trap）。 |
| `-H` | `-o histexpand` | 启用历史替换（如 `!!`）。在交互模式下默认启用。 |
| `-P` | `-o physical` | 防止命令跟随符号链接，强制使用物理目录路径。 |
| `-T` | `-o functrace` | 使 shell 函数继承 `DEBUG` 捕获（trap）。 |
| `--` | 无 | 将剩余参数赋值给位置参数。如果没有参数，则取消设置位置参数。 |
| `-` | 无 | 将剩余参数赋值给位置参数，同时关闭 `-x` 和 `-v` 选项。 |
| 无 | `-o emacs` | 使用 `emacs` 风格的行编辑接口。 |
| 无 | `-o history` | 启用命令历史记录。 |
| 无 | `-o ignoreeof` | 读取到文件末尾（EOF）时，shell 不退出。 |
| 无 | `-o interactive-comments` | 允许在交互模式下使用注释。 |
| 无 | `-o nolog` | 不在历史文件中记录函数定义。 |
| 无 | `-o pipefail` | 使管道返回值为第一个失败命令的退出状态，而不是最后一个命令的状态。 |
| 无 | `-o posix` | 使 Bash 的行为与 POSIX 标准一致。 |
| 无 | `-o vi` | 使用类似 `vi` 的行编辑接口。 |

## 3. 常见用法与示例

1. **显示所有 shell 变量**  
   执行 `set` 不带任何参数，将输出当前 shell 中的所有变量及其值。

   ```bash
   set
   ```

2. **脚本调试**  
   使用 `-x` 选项逐行输出脚本执行的每条命令和参数。

   ```bash
   set -x
   var=5
   while [ $var -gt 0 ]; do
       echo $var
       var=$((var - 1))
   done
   ```

   输出将详细显示每个步骤的执行情况，有助于排查问题。

3. **错误处理：命令失败时退出**  
   使用 `-e` 选项使 shell 遇到错误命令时退出，避免继续执行后续指令。

   ```bash
   #!/bin/bash
   set -e
   echo "Trying to read a non-existing file"
   cat non_existing_file.txt
   echo "This will not print if the file doesn't exist"
   ```

4. **自动导出变量**  
   使用 `-a` 选项将每个新创建的变量自动标记为导出到子进程环境。

   ```bash
   set -a
   var1="Hello"
   var2="World"
   /bin/bash  # 启动子shell，所有变量都会导出
   echo "$var1 $var2"
   ```

5. **禁止文件重定向覆盖现有文件**  
   使用 `-C` 选项防止重定向 (`>`) 意外覆盖已存在的文件，确保文件内容不会丢失。

   ```bash
   echo "Initial data" > file.txt
   set -C
   echo "New data" > file.txt  # 这将失败，因为文件已经存在
   ```

6. **检测未定义变量**  
   `-u` 选项可确保使用未定义变量时引发错误，避免脚本因未设置变量而产生的逻辑错误。

   ```bash
   #!/bin/bash
   set -u
   echo $undefined_var  # 这会导致脚本立即退出
   ```

7. **设置位置参数**  
   使用 `set` 命令可以设置位置参数 `$1`, `$2` 等。

   ```bash
   set one two three
   echo $1  # 输出 one
   echo $2  # 输出 two
   ```

8. **字符串分割为多个变量**  
   `set --` 可以根据空格将字符串拆分为多个位置参数。

   ```bash
   mystring="hello world example"
   set -- $mystring
   echo $1  # 输出 hello
   echo $2  # 输出 world
   ```

9. **切换 shell 特性**  
   使用 `set -o` 可以切换不同的 shell 特性，例如 `allexport` 自动导出变量，`notify` 后台任务完成时立即通知。

   ```bash
   set -o allexport -o notify
   ```

## 4. 高级应用

- **防止脚本运行时误操作**  
   使用 `-C` 防止覆盖重要文件，结合 `-e` 在错误时退出，确保关键任务的健壮性。
  
- **调试复杂脚本**  
   结合使用 `-x` 选项（调试）与 `-v` 选项（逐行打印输入）可以帮助分析复杂脚本的行为。
