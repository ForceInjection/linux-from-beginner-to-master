# Linux 中 x86 上下文切换的演变
>* **发表时间**：_2018 年 9 月_
* **原文地址**：[Evolution of the x86 context switch in Linux](https://www.maizure.org/projects/evolution_x86_context_switch_linux/)

![Linux x86 上下文切换](https://www.maizure.org/projects/evolution_x86_context_switch_linux/context_switch_flow.png)

上周末在研究 80386 硬件上下文切换的古老事实时，我想起了早期版本的 `Linux` 内核依赖它。我立刻被它吸引了，花了几个小时阅读多年未见的代码。这个周末，我决定将这段旅程记录下来，整理出我在过程中发现的所有有趣的内容。

**练习**：从最早的（0.01）到最新的 LTS 版本（4.14.67）追踪 Linux 内核中的上下文切换——特别强调第一个和最后一个版本。

##  目录

- **早期 Linux - 古代历史**
- **Linux 1.0 - 概念验证**
- **Linux 2.0 - 竞争者**
  - **2.2 - 软件切换**
  - **2.4 - 最后的传统内核**
- **Linux 2.6 - Linux 走向主流**
- **Linux 3.0 - 现代操作系统**
- **Linux 4.14.67 - 最新的 LTS 版本**

真正的故事并不是上下文切换本身，而是 `Linux` 如何从一个小项目演变为现代操作系统的过程。

## 我们讨论的是什么上下文切换？

虽然许多事情都可以被视为上下文切换（例如切换到内核模式，跳转到中断处理程序），但我将采用公认的含义：**从一个进程切换到另一个进程**。在 `Linux` 中，这意味着 `switch_to()` 宏及其内部的所有内容。

这个切换宏是一个平淡无奇的机械序列，夹在两个更有趣的系统之间：**任务调度器**和 **CPU**。操作系统开发者在混合和匹配调度策略方面有很大的自由度。`CPU` 架构也是一个广阔的领域，正如 `Linux` 支持的数十种不同类型所证明的那样。然而，上下文切换是它们之间的齿轮。“**设计**”是其他地方做出选择的结果，使得上下文切换成为操作系统中**最不**有趣的部分。用一句同义反复的话来说：**唯一要做的事情就是做需要做的事情**。

上下文切换的简短任务列表：

1. 重新指向工作空间：恢复堆栈（`SS:SP`）
2. 找到下一条指令：恢复 IP（`CS:IP`）
3. 重建任务状态：恢复通用寄存器
4. 交换内存地址空间：更新页目录（`CR3`）
5. ...以及更多：`FPU`、操作系统数据结构、调试寄存器、硬件解决方法等。

虽然所有这些任务在另一个进程接管 `CPU` 时都会发生，但并不总是显而易见它们何时何地发生。例如，在 `Linux 2.2` 之前的硬件上下文切换中，任务 2、3 和 4 被隐藏了。任务 3 是有限的，因为切换发生在内核模式之间。恢复用户线程操作是调度器返回后 `iret` 的工作。许多这些任务在内核版本之间在 `switch_to()` 和调度器之间浮动。我能保证的是，我们总能在每个版本中看到堆栈交换和 FPU 切换。

**这篇文章适合谁？**  
没有特定的人。你需要熟悉 x86 汇编语言，可能还需要一两个学期的操作系统设计课程才能欣赏（**译者**：读懂）这篇文章。

**完全披露**：我不是[内核维护者](https://www.kernel.org/doc/linux/MAINTAINERS)或贡献者。任何来自这些人员或[Linux 内核邮件列表](https://lkml.org/)的信息如果与我的信息相矛盾，应该认真对待。这是一个随意的个人项目，不是同行评审的文章。

## Linux 1.0 之前 - 古代历史（1991 年）

早期的 Linux 内核简单而功能齐全，具有以下关键特性：

- **单一架构（80386/i386）**- 只有一种“**风味**”的上下文切换。许多 `80386` 的细节在内核中硬编码。我将使用“**Intel 80386 程序员手册（1986）**”作为这些部分的参考。
- **硬件上下文切换** - 内核依赖于 `80386` 的内置机制来切换任务。
- **单处理器抢占式多任务处理** - 只有一个 `CPU` 和一个进程在某一时刻是活动的。然而，另一个进程可以随时接管。因此，通常的同步规则适用：锁定共享资源（无自旋锁）。禁用中断可以应急，但应首先考虑让步（互斥锁）。

![早期 Linux 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/early_linux_context_switch_flow.png)

事不宜迟，让我们看看两个早期的上下文切换。代码已经格式化以便显示：每行一个元素，并删除了续行符（\\）。

- **Linux 0.01**

```c
/** include/linux/sched.h */
#define switch_to(n) {
struct {long a,b;} __tmp;
__asm__("cmpl %%ecx,_current\n\t"
  "je 1f\n\t"
  "xchgl %%ecx,_current\n\t"
  "movw %%dx,%1\n\t"
  "ljmp %0\n\t"
  "cmpl %%ecx,%2\n\t"
  "jne 1f\n\t"
  "clts\n"
  "1:"
  ::"m" (*&__tmp.a),
  "m" (*&__tmp.b),
  "m" (last_task_used_math),
  "d" _TSS(n),
  "c" ((long) task[n]));
}
```
- **Linux 0.11**

```c
/** include/linux/sched.h */
#define switch_to(n) {
struct {long a,b;} __tmp;
__asm__("cmpl %%ecx,_current\n\t"
  "je 1f\n\t"
  "movw %%dx,%1\n\t"
  "xchgl %%ecx,_current\n\t"
  "ljmp %0\n\t"
  "cmpl %%ecx,_last_task_used_math\n\t"
  "jne 1f\n\t"
  "clts\n"
  "1:"
  ::"m" (*&__tmp.a),
  "m" (*&__tmp.b),
  "d" (_TSS(n)),
  "c" ((long) task[n]));
}
```

首先要注意的是，这段代码很短！短到可以逐行进行代码解析：

```c
#define switch_to(n) {
```

所以 `switch_to()` 是一个宏。它只出现在一个地方：`schedule()` 的最后一行。因此，宏在预处理后与调度器共享作用域。检查像 `current` 和 `last_task_used_math` 这样的未知引用是否在全局作用域中。输入参数 `n` 是下一个任务的索引号（介于 0 和 63 之间）。

```c
struct {long a,b;} __tmp;
```

在堆栈上保留 8 字节（64 位），可通过两个 4 字节成员 `a` 和 `b` 访问。我们稍后会设置这些字节中的一些，用于远跳转操作。

```c
__asm__("cmpl %%ecx,_current\n\t"
```

上下文切换是一个长的内联汇编块。第一条指令确定目标任务是否已经是当前任务。这是存储在 ECX 寄存器中的值与调度器中的 `current` 全局值的减法比较。两者都包含指向某个进程的 `task\_struct` 的指针。ECX 包含目标任务指针，作为下面的给定输入：`"c" ((long) task[n])`。比较结果设置 `EFLAGS` 状态寄存器：如果两个指针匹配（x - x = 0），则 ZF = 1。

```c
"je 1f\n\t"
```

如果下一个任务是当前任务，则无需上下文切换，因此我们应该跳过（跳转）整个过程。`je` 指令测试 ZF = 1。如果是，则向前跳转到代码中第一个标签 '1' 处，即代码中的 8 行之后。

```c
"xchgl %%ecx,_current\n\t"
```

更新 `current` 全局变量以反映新任务。ECX（task\[n\]）中的指针与 current 交换。不更新标志。

```c
"movw %%dx,%1\n\t"
```

将目标任务描述符（TSS）的段选择器索引移动到之前保留的空间中。技术上，这将 DX 寄存器中的值移动到 `__tmp.b` 中，这是我们保留的 8 字节结构的第 5 到第 8 字节。DX 的值是给定的输入：`"d" (_TSS(n))`。`_TSS` 多级宏扩展为有效的 TSS 段选择器，我将在下面几行中介绍。底线是 `__tmp.b` 的最高两个字节现在将保存指向下一个任务的段指针。

```c
"ljmp %0\n\t"
```

通过跳转到 TSS 描述符来调用 80386 硬件上下文切换。这个简单的跳转可能会让人困惑，因为有三个不同的概念需要解包：首先，`ljmp` 是一个间接远跳转，期望一个 6 字节（48 位）的操作数。其次，提供的操作数 %0 引用未初始化的变量 `__tmp.a`。最后，跳转到 GDT 中的段选择器在 x86 中有特殊含义。让我们分解这些点。

### 间接远跳转

重要的是，这个跳转有一个 6 字节的操作数。80386 程序员指南描述了这种跳转：

![80386 中的长跳转](https://www.maizure.org/projects/evolution_x86_context_switch_linux/ljmp.png)

### 跳转到 \_\_tmp.a

回想一下，\_\_tmp 结构体包含两个 4 字节的值，`a` 成员是结构体的基址。然而，如果我们使用该成员作为 6 字节操作数的基地址，我们将到达 `__tmp.b` 整数的两个字节**内部**。这两个字节组成了远地址的“段选择器”部分。当处理器读取该段是 GDT 中的 TSS 时，偏移部分被完全忽略。`__tmp.a` 未初始化并不重要，因为 `__tmp.b` 确实有一个有效的值，这要归功于之前的 `movw` 指令。现在，让我们用跳转地址更新图表：

![__tmp 的远跳转格式](https://www.maizure.org/projects/evolution_x86_context_switch_linux/ljmp_addr.png)

我们如何知道这个地址引用了 GDT？我将在下面几行代码中详细介绍，但简短的版本是选择器中的四个 0 位触发了 GDT 查找。`_TSS(n)` 宏确保这四个零存在。最低两位是段的特权级别（00 是超级用户/内核），接下来的 0 位表示使用 GDT 表（在系统启动时存储在 GDTR 中）。第四个零是段索引的一部分，强制所有 TSS 查找在 GDT 表的偶数条目上进行。

### 硬件上下文切换

\_\_tmp 中的跳转地址标识了 GDT 中的 TSS 描述符。对于 80386 来说，这意味着：

![在 80386 中调用硬件任务切换](https://www.maizure.org/projects/evolution_x86_context_switch_linux/80386_task_switch.png)

处理器为我们自动执行以下操作：

- 检查当前特权级别是否允许（我们在内核模式下，所以继续）
- 检查 TSS 是否有效（应该是）
- 将所有当前任务状态保存到仍然存储在任务寄存器（TR）中的旧 TSS 中。这意味着保存 EAX、ECX、EDX、EBX、ESP、EBP、ESI、EDI、ES、CS、SS、DS、FS、GS 和 EFLAGS。EIP 递增到下一条指令并保存
- 更新 TR 为新任务
- 恢复所有通用寄存器、EIP 和 PDBR（地址空间交换）。任务切换完成，因此在 CR0 寄存器中设置 TS 标志

这条单指令 `"ljmp %0\n\t"` 刚刚执行了我们上下文切换的所有步骤。剩下的只是一些清理工作。

```c
"cmpl %%ecx,%2\n\t"
```

检查我们刚刚切换出的任务是否已经恢复了数学协处理器。参数是 `last_task_used_math` task\_struct 指针。这个想法是管理 TS 标志以跟踪协处理器是否具有不同的进程上下文。硬件上下文切换不管理协处理器。

```c
"jne 1f\n\t"
```

如果上一个任务没有恢复协处理器，跳转到上下文切换的末尾。我们希望保持 TS 标志设置，以便下次使用协处理器时，我们可以捕获以执行一些延迟清理。它是“延迟”的，因为我们推迟任务直到绝对必要。

```c
"clts\n"
```

清除 TS 标志，因为上一个进程恢复了协处理器状态。

```c
"1:"
```

上下文切换结束的标签。所有跳转到此标签的指令都会跳过部分或全部过程。

```c
::"m" (*&__tmp.a),
```

这个汇编块没有输出，第一个输入（%0）是 GDT 中 TSS 描述符的远指针的前 4 个字节的内存位置。仅用作地址引用，值未初始化。

```c
"m" (*&__tmp.b),
```

第二个输入（%1）是 TSS 描述符的远指针的第 5 和第 6 个字节的内存位置。技术上，这个内存位置是 4 个字节，但只检查并使用前两个字节。

```c
"m" (last_task_used_math),
```

第三个输入（%2）是指向上一个恢复协处理器状态的 task\_struct 的指针的内存位置。

```c
"d" (_TSS(n)),
```

第四个输入（%3 / %%edx）是 GDT 中 TSS 描述符的段选择器地址。让我们分解这个宏：

```c
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define FIRST_TSS_ENTRY 4
```

用英语来说，这表示第一个 TSS 描述符是第 4 个条目（索引计数从段选择器的第 4 位开始）。每个后续的 TSS 占据 GDT 的每隔一个条目 - 4、6、8 等。前八个任务如下所示：

| 任务编号 | 16 位段选择器 |
|--------|-------------------------|
| 0      | 0000000000100  0  00    |
| 1      | 0000000000110  0  00    |
| 2      | 0000000001000  0  00    |
| 3      | 0000000001010  0  00    |
| 4      | 0000000001100  0  00    |
| 5      | 0000000001110  0  00    |
| 6      | 0000000010000  0  00    |
| 7      | 0000000010010  0  00    |

地址中的位已按 80386 预期的字段格式分隔：

![80386 段选择器格式](https://www.maizure.org/projects/evolution_x86_context_switch_linux/segment_selector_format.png)

最低的 4 位始终为 0，表示超级用户模式、GDT 表，并强制 GDT 索引为偶数。

```c
"c" ((long) task[n]));
```

最后一个输入（%4 / %ecx）是指向我们正在切换的新 task\_struct 的指针。请注意，%%ecx 的值在上下文切换之前更改为上一个任务。

### 0.01 和 0.11 之间的差异

两个上下文切换之间有两个差异。一个是简单的代码清理，另一个是部分错误修复。

- `_last_task_used_math` 被移除作为输入变量，因为该符号已经在全局作用域中可用。相关的比较操作更改为直接引用。
- `xchgl` 指令与 `movw` 交换，以便将其更接近硬件上下文切换（`ljmp`）。问题是这些操作不是原子的——在 `xchgl` 和 `ljmp` 之间可能会发生中断，导致另一个上下文切换，错误的 `current` 任务和真实任务的状态未保存。交换这些指令将这种情况从“不太可能”变为“非常不可能”。在长时间运行的系统中，“非常不可能”是“不可避免”的代名词。

---

## Linux 1.x - 概念验证（1994 年）

在 0.11 和 1.0 之间的约 2 年中，发生了一些变化 - 大约 20 个开发补丁。大部分工作集中在驱动程序和用户/开发者功能上。最大任务数增加到 128。但没有太多会影响上下文切换的根本变化。

### Linux 1.0

Linux 1.0 仍然是单一架构和单处理器，使用硬件上下文切换。

![Linux 1.0 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux1_context_switch_flow.png)

**Linux 1.0**

```c
/** include/linux/sched.h */
#define switch_to(tsk)
__asm__("cmpl %%ecx,_current\n\t"
	"je 1f\n\t"
	"cli\n\t"
	"xchgl %%ecx,_current\n\t"
	"ljmp %0\n\t"
	"sti\n\t"
	"cmpl %%ecx,_last_task_used_math\n\t"
	"jne 1f\n\t"
	"clts\n"
	"1:"
	: /* no output */
	:"m" (*(((char *)&tsk->tss.tr)-4)),
	 "c" (tsk)
	:"cx")
```

最显著的变化是输入参数不再是 task\_struct 数组的任务编号索引。现在 `switch_to()` 接受指向新任务的指针。我们现在可以移除 `__tmp` 结构体，而是直接引用 TSS。让我们逐行解析。

```c
#define switch_to(tsk)
```

输入现在是指向下一个任务的 `task_struct` 的指针。

```c
"__asm__("cmpl %%ecx,_current\n\t"
```

未更改。检查输入任务是否已经是当前任务，无需切换。

```c
"je 1f\n\t"
```

未更改。如果没有切换，则跳过上下文切换。

```c
"cli\n\t"
```

禁用中断以防止计时器（或其他可屏蔽中断）在全局任务更新和硬件上下文切换之间切入。这个中断大锤通过使接下来的两条指令（伪）原子化来解决早期内核版本中的错误。

```c
"xchgl %%ecx,_current\n\t"
"ljmp %0\n\t"
```

未更改 - 交换当前进程以反映新任务并调用硬件上下文切换。

```c
"sti\n\t"
```

启用中断。

```c
"cmpl %%ecx,_last_task_used_math\n\t"
"jne 1f\n\t"
"clts\n"
"1:"
```

所有未更改自 Linux 0.11。处理 TS 寄存器以管理脏的数学协处理器。

```c
: /* no output */
```

此内联汇编没有输出效果 - 显然有人对早期内核版本缺乏注释感到恼火。

```c
:"m" (*(((char *)&tsk->tss.tr)-4)),
```

加载新任务的 TSS 描述符的段选择器，现在可以直接从 task\_struct 指针中获得。`tss.tr` 成员保存 GDT/TSS 内存引用的 \_TSS(task\_number)，这在 1.0 之前的内核中使用。我们仍然回退 4 个字节并加载 6 字节的段选择器以获取最高的两个字节。有趣！

```c
"c" (tsk)
```

某种程度上未更改 - 我们现在直接加载传递的指针，而不是查找索引。

```c
:"cx")
```

上下文切换会破坏 ECX 寄存器。

### Linux 1.3

内核现在支持几种新架构：Alpha、MIPS 和 SPARC。因此，现在有四个不同版本的 `switch_to()`，其中一个在内核编译时包含。架构相关代码已从内核主体中分离出来，因此我们必须移动到一个新位置来找到我们的 x86 版本。

**Linux 1.3**

```c
/** include/asm-i386/system.h */
#define switch_to(tsk) do {
__asm__("cli\n\t"
	"xchgl %%ecx,_current\n\t"
	"ljmp %0\n\t"
	"sti\n\t"
	"cmpl %%ecx,_last_task_used_math\n\t"
	"jne 1f\n\t"
	"clts\n"
	"1:"
	: /* no output */
	:"m" (*(((char *)&tsk->tss.tr)-4)),
	 "c" (tsk)
	:"cx");
	/* Now maybe reload the debug registers */
	if(current->debugreg[7]){
		loaddebug(0);
		loaddebug(1);
		loaddebug(2);
		loaddebug(3);
		loaddebug(6);
	}
} while (0)
```

一些小变化：整个上下文切换被包装在一个假的 do-while 循环中 - 假的因为它永远不会重复。检查我们是否切换到新任务的操作从 `switch_to()` 移到了调度器的 C 代码中。一些调试任务从 C 代码移到了 `switch_to()` 中，可能是为了避免它们被分离。让我们逐步了解这些变化。

```c
#define switch_to(tsk) do {
```

现在 `switch_to()` 被包装在一个 do-while(0) 循环中。这种结构防止宏在条件语句（如果是“if”中的“then”）中扩展为多个语句时出错。目前这似乎不是这种情况，但考虑到调度器内部的变化，我怀疑这在早期版本中是真实的，并保留下来以备将来使用。我的猜测：

- **实际的 1.3 调度器**:

```c
...within schedule()...

    if (current == next)
       return;
    kstat.context_swtch++;
    switch_to(next);
```
- **稍微改变以“中断” switch_to()**: 

```c
...within schedule()...

    if (current != next)
        switch_to(next);
		
 /* do-while(0) 'captures' entire 
  * block to ensure proper parse */
```

```c
__asm__("cli\n\t"
"xchgl %%ecx,_current\n\t"
"ljmp %0\n\t"
"sti\n\t"
"cmpl %%ecx,_last_task_used_math\n\t"
"jne 1f\n\t"
"clts\n"
"1:"
": /* no output */"
:"m" (*(((char *)&tsk->tss.tr)-4)),"
"c" (tsk)"
:"cx");"
```

自 Linux 1.0 之后没有变动。仍然在交换 `current` \*task\_struct 之前禁用中断，硬件上下文切换，并验证协处理器使用情况。

```c
/* 现在可能重新加载调试寄存器 */
if(current->debugreg[7]){
```

检查新进程的调试控制，看看是否启用了 ptrace（这里的一些非空地址意味着启用了 ptrace）。调试跟踪移到了 `switch_to()` 中。在 1.0 中，完全相同的 C 序列跟在宏后面。我想他们想确保：1）调试尽可能接近上下文切换 2）switch\_to 是 `schedule()` 中的最后一件事。

```c
loaddebug(0);
loaddebug(1);
loaddebug(2);
loaddebug(3);
```

从保存的 ptrace 状态恢复断点调试寄存器。

```c
loaddebug(6);
```

从保存的 ptrace 状态恢复状态调试寄存器。

```c
} while (0)
```

关闭 `switch_to()` 块。虽然条件永远不会超过一次评估，但这确保了解析器/词法分析器将函数视为一个基本块，不会与 `schedule()` 中的相邻条件语句交互。注意缺少尾随逗号 - 这由宏调用后的逗号填充：`switch_to(next)**;**`

## Linux 2.x - Linux 成为竞争者（1996 年）

1996 年 6 月，内核更新到 2.0 版本，开始了 15 年的旅程，最终获得了广泛的商业支持。几乎内核中的每个基本系统在 2.x 中都发生了巨大变化。让我们逐步了解每个初始次要版本，直到 2.6。2.6 版本开发了很长时间，因此值得单独一节。

### Linux 2.0

Linux 2.0 以一声巨响开始了这个系列：**多处理**！两个或更多的处理器可以同时处理用户/内核代码。自然，这需要一些改造。例如，每个处理器现在都有一个专用的中断控制器，一个 APIC，因此需要在每个处理器的基础上管理中断。像计时器中断这样的机制需要重新设计（禁用中断只影响一个处理器）。同步变得复杂，特别是当试图将其应用于已经庞大且分散的代码库时。Linux 2.0 为将成为大内核锁（BKL）的东西奠定了基础...总得有个开始。

我们现在有两个版本的 `switch_to()`：从 1.x Linux 扩展的单处理器（UP）版本，以及新的和改进的对称多处理（SMP）版本。我们将首先查看遗留的变化，因为它包含一些也包含在 SMP 版本中的变化。

### Linux 2.0.1 - 单处理器版（UP）

![Linux 2.0 UP 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux2_context_switch_UP_flow.png)

**Linux 2.0.1 (UP)**

```c
/** include/asm-i386/system.h */
#else  /* Single process only (not SMP) */
#define switch_to(prev,next) do {
__asm__("movl %2,"SYMBOL_NAME_STR(current_set)"\n\t"
	"ljmp %0\n\t"
	"cmpl %1,"SYMBOL_NAME_STR(last_task_used_math)"\n\t"
	"jne 1f\n\t"
	"clts\n"
	"1:"
	: /* no outputs */
	:"m" (*(((char *)&next->tss.tr)-4)),
	 "r" (prev), "r" (next));
	/* Now maybe reload the debug registers */
	if(prev->debugreg[7]){
		loaddebug(prev,0);
		loaddebug(prev,1);
		loaddebug(prev,2);
		loaddebug(prev,3);
		loaddebug(prev,6);
	}
} while (0)
#endif
```

两个立即明显的变化：

- `switch_to()` 有一个新参数：我们正在切换的进程 `*task_struct`
- 一个宏来正确处理 C 和内联汇编之间的符号标识

像往常一样，我们将逐行讨论变化。

```c
#define switch_to(prev,next) do {
```

`prev` 参数标识我们正在切换的任务（一个 `*task_struct`）。我们仍然将宏包装在 do-while(0) 循环中，以帮助解析宏周围的单行“if”。

```c
__asm__("movl %2,"SYMBOL_NAME_STR(current_set)"\n\t"
```

更新当前活动的任务为新选择的任务。这在功能上等同于 `xchgl %%ecx,_current`，只是现在我们有一个多个 task\_struct 的数组和一个宏（`SYMBOL_NAME_STR`）来管理内联汇编符号名称。为什么要使用预处理器来管理内联汇编名称？一些汇编器（GAS）需要在 C 变量名前加下划线（\_）。其他汇编器没有这个约定。与其硬编码这个约定，不如允许你在编译时为你的工具链配置它。

```c
"ljmp %0\n\t"
"cmpl %1,"SYMBOL_NAME_STR(last_task_used_math)"\n\t"
"jne 1f\n\t"
"clts\n"
"1:"
": /* no outputs */"
:"m" (*(((char *)&next->tss.tr)-4)),
```

没有尚未指出的变化。

```c
"r" (prev), "r" (next));
```

现在我们携带两个任务作为内联汇编的输入。一个小的变化是现在允许任何寄存器使用。以前，`next` 被硬编码为 ECX。

```c
/* 现在可能重新加载调试寄存器 */
if(prev->debugreg[7]){
    loaddebug(prev,0);
    loaddebug(prev,1);
    loaddebug(prev,2);
    loaddebug(prev,3);
    loaddebug(prev,6);
    }
} while (0)
```

与内核 v1.3 完全相同。

### Linux 2.0.1 - 对称多处理版（SMP）

![Linux 2.0 SMP 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux2_context_switch_SMP_flow.png)

**Linux 2.0.1 (SMP)**

```c
/** include/asm-i386/system.h */
#ifdef __SMP__   /* Multiprocessing enabled */
#define switch_to(prev,next) do {
    cli();
    if(prev->flags&PF_USEDFPU)
    {
        __asm__ __volatile__("fnsave %0":"=m" (prev->tss.i387.hard));
        __asm__ __volatile__("fwait");
        prev->flags&=~PF_USEDFPU;
    }
    prev->lock_depth=syscall_count;
    kernel_counter+=next->lock_depth-prev->lock_depth;
    syscall_count=next->lock_depth;
__asm__("pushl %%edx\n\t"
    "movl "SYMBOL_NAME_STR(apic_reg)",%%edx\n\t"
    "movl 0x20(%%edx), %%edx\n\t"
    "shrl $22,%%edx\n\t"
    "and  $0x3C,%%edx\n\t"
    "movl %%ecx,"SYMBOL_NAME_STR(current_set)"(,%%edx)\n\t"
    "popl %%edx\n\t"
    "ljmp %0\n\t"
    "sti\n\t"
    : /* no output */
    :"m" (*(((char *)&next->tss.tr)-4)),
     "c" (next));
    /* Now maybe reload the debug registers */
    if(prev->debugreg[7]){
        loaddebug(prev,0);
        loaddebug(prev,1);
        loaddebug(prev,2);
        loaddebug(prev,3);
        loaddebug(prev,6);
    }
} while (0)
```

变得混乱了吗？我希望我能说它会变得更好，但在 SMP 领域永远不会。为了节省空间，我将不再列出未更改的行。

SMP 上下文切换的三个新增功能：1）改变我们如何在多个处理器之间存储单个协处理器；2）由于内核锁是递归的，因此锁深度管理；3）引用 APIC 以获取 CPU ID 以关联当前的 \*task\_struct。

```c
if(prev->flags&PF_USEDFPU)
```

检查我们正在切换的任务是否在此时间片内使用了协处理器。如果是，我们需要在切换之前将上下文抓取到 FPU 中。

```c
__asm__ __volatile__("fnsave %0":"=m" (prev->tss.i387.hard));
```

将 FPU 状态存储在 TSS 中。FNSAVE 用于跳过异常处理。`__volatile__` 应该防止优化器更改此指令。

```c
__asm__ __volatile__("fwait");
```

在 FPU 处理上一个保存时忙等待 CPU。

```c
prev->flags&=~PF_USEDFPU;
```

强制此任务的协处理器使用标志关闭。补码的 AND 始终为 0。

```c
prev->lock_depth=syscall_count;
```

存储旧任务的嵌套使用内核锁的计数。

```c
kernel_counter+=next->lock_depth-prev->lock_depth;
```

将全局内核锁计数器更新为新任务减去旧任务。有效地从正在休眠的旧任务中移除锁，新任务可以从中断处继续。

```c
syscall_count=next->lock_depth;
```

恢复新任务的锁状态。应该在上一个时间片中断的地方继续。

```c
__asm__("pushl %%edx\n\t"
```

我们即将使用 EDX，因此存储它当前持有的任何值。

```c
"movl "SYMBOL_NAME_STR(apic_reg)",%%edx\n\t"
```

将 APIC I/O 地址移动到 EDX 中。我们需要使用 APIC 来获取 CPU ID，因为我们不知道哪个处理器正在运行。`apic_reg` 在操作系统初始化期间映射。

```c
"movl 0x20(%%edx), %%edx\n\t"
```

将 APIC ID 寄存器值解引用到 EDX 中。实际 ID 在 24-27 位中。

![APIC 寄存器值的格式](https://www.maizure.org/projects/evolution_x86_context_switch_linux/apic_id_register.png)

```c
"shrl $22,%%edx\n\t"
```

将 APIC ID 移位到 2-5 位。

```c
"and $0x3C,%%edx\n\t"
```

仅屏蔽出 2-5 位中的 APIC ID，留下 CPU 编号 \* 4。

```c
"movl %%ecx,"SYMBOL_NAME_STR(current_set)"(,%%edx)\n\t"
```

更新当前 CPU 的任务指针为新任务。UP 版本已经移除了使用 ECX 来存储当前任务的特定用途，但这个 SMP 版本仍然使用它。EDX 持有存储在 2-5 位中的 CPU 编号，这是 CPU 编号乘以 4 以缩放指针大小偏移量从 \_current\_set。

```c
"popl %%edx\n\t"
```

我们完成了 EDX 的使用，因此恢复它在此过程之前的值。

其余行与 UP 对应部分相同。

### Linux 2.2 (1999 年)

Linux 2.2 值得等待：**软件上下文切换**！我们仍然使用任务寄存器（TR）来引用 TSS。SMP 和 UP 程序合并，统一处理 FPU 状态。大部分上下文切换现在在 C 中执行。

![Linux 2.2 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux22_context_switch_flow.png)

**Linux 2.2.0 (内联汇编)**

```c
/** include/asm-i386/system.h */
#define switch_to(prev,next) do {
    unsigned long eax, edx, ecx;
    asm volatile("pushl %%ebx\n\t"
                 "pushl %%esi\n\t"
                 "pushl %%edi\n\t"
                 "pushl %%ebp\n\t"
                 "movl %%esp,%0\n\t" /* save ESP */
                 "movl %5,%%esp\n\t" /* restore ESP */
                 "movl $1f,%1\n\t"   /* save EIP */
                 "pushl %6\n\t"      /* restore EIP */
                 "jmp __switch_to\n"
                 "1:\t"
                 "popl %%ebp\n\t"
                 "popl %%edi\n\t"
                 "popl %%esi\n\t"
                 "popl %%ebx"
                 :"=m" (prev->tss.esp),"=m" (prev->tss.eip),
                  "=a" (eax), "=d" (edx), "=c" (ecx)
                 :"m" (next->tss.esp),"m" (next->tss.eip),
                  "a" (prev), "d" (next));
} while (0)
```

这个新的 `switch_to()` 是一个与所有先前版本截然不同的版本：它很容易理解！我们使用内联汇编交换堆栈指针和指令指针（上下文切换任务 #1 和 #2）。其余的工作在跳转到 C 代码（`__switch_to()`）后发生。

```c
asm volatile("pushl %%ebx\n\t"
"pushl %%esi\n\t"
"pushl %%edi\n\t"
"pushl %%ebp\n\t"
```

将 EBX、ESI、EDI 和 EBP 存储在我们即将切换出的进程的堆栈上。（...为什么是 EBX？）

```c
"movl %%esp,%0\n\t" /* 保存 ESP */
"movl %5,%%esp\n\t" /* 恢复 ESP */
```

如注释所示，我们在旧进程和新进程之间交换堆栈指针。旧进程是操作数 %0（`prev->tss.esp`），新进程是操作数 %5（`next->tss.esp`）。

```c
"movl $1f,%1\n\t" /* 保存 EIP */
```

存储旧任务在我们上下文切换回来后的下一条指令的指令指针值。请注意，下一条指令的值使用标签 `1:`。

```c
"pushl %6\n\t" /* 恢复 EIP */
```

为新任务准备下一条指令。由于我们刚刚切换到新堆栈，此 IP 取自新任务的 TSS 并推送到堆栈顶部。执行将在我们从即将执行的 C 代码返回后的下一条指令开始。

```c
"jmp **__switch_to**\n"
```

跳转到我们新的和改进的软件上下文切换。见下文。

```c
"popl %%ebp\n\t"
"popl %%edi\n\t"
"popl %%esi\n\t"
"popl %%ebx"
```

以相反的顺序从堆栈中恢复寄存器，大概是在我们在新的时间片中切换回旧任务之后。

| Linux 2.2.0 (C)                                       |
|------------------------------------------------------|
| urltomarkdowncodeblockplaceholder100.03770148408241081 |

软件上下文切换将旧的跳转到 TSS 描述符替换为跳转到新的 C 函数：`__switch_to()`。此函数用 C 编写，并包含一些熟悉的组件，例如曾经在内联汇编之后的调试寄存器。跳转到 C 代码使我们能够将其更接近切换。

```c
unlazy_fpu(prev);
```

检查是否使用了 FPU，如果是，则保存 FPU 状态。现在，每次切换出使用 FPU 的进程时都会发生这种情况（因此称为“非延迟”）。该过程与 2.0.1 中的 SMP 例程相同，只是现在我们有一个干净的宏，包括手动设置 TS。

```c
gdt_table[next->tss.tr >> 3].b &= 0xfffffdff;
```

清除即将到来的任务描述符的 BUSY 位。使用任务的编号索引 GDT。回想一下 v0.01，tss.tr 保存任务的段选择器值，其中最低 3 位用于权限。我们只需要索引，因此右移这些位。TSS 的第二个字节被修改以移除第 10 位。

![手动清除 TSS 中的 BUSY](https://www.maizure.org/projects/evolution_x86_context_switch_linux/tss_clear_busy.png)

```c
asm volatile("ltr %0": :"g" (*(unsigned short *)&next->tss.tr));
```

将任务寄存器加载为指向下一个任务段选择器的指针。

```c
asm volatile("movl %%fs,%0":"=m" (*(int *)&prev->tss.fs));
asm volatile("movl %%gs,%0":"=m" (*(int *)&prev->tss.gs));
```

将 FS 和 GS 段寄存器存储在前一个任务的 TSS 中。此步骤在硬件上下文切换中是自动的，但现在我们需要手动执行此操作。但为什么？Linux 如何使用 FS 和 GS？

在 Linux 2.2（1999 年）中，没有一个一致的答案，除了它们被使用，因此必须存储它们以保持可用。内核模式代码将“借用”这些段来指向内核或用户数据段。声音/网络驱动程序也是如此。在最近的时代（~2.6 以后），FS 和 GS 通常分别支持线程本地存储和每处理器数据区域。

```c
if (next->mm->segments != prev->mm->segments)
    asm volatile("lldt %0": :"g" (*(unsigned short *)&next->tss.ldt));
```

如果它们与旧进程不匹配，则恢复本地描述符表的段。这是通过加载 LDT 寄存器来完成的。

```c
if (new_cr3 != prev->tss.cr3)
    asm volatile("movl %0,%%cr3": :"r" (new_cr3));
```

更新新任务的虚拟内存状态。机械地，这设置了 CR3 寄存器，其中包含新上下文中所有内存访问使用的页目录。

```c
loadsegment(fs,next->tss.fs);
loadsegment(gs,next->tss.gs);
```

为新任务恢复 FS 和 GS。强制执行正确的对齐，并在出现问题时加载零段。

```c
loaddebug(prev,7);
```

最后，调试控制寄存器现在从 TSS 中存储和切换。以前，这只是检查，而不是存储。

### Linux 2.4 (2001 年)

2.4 内核系列带来了许多新功能，例如内核线程和任务队列。尽管如此，以及调度器的一些变化，上下文切换从 2.2 开始基本保持不变，尽管我们停止更新 TR，而是替换其所有数据。我非正式地称此为“最后的传统”内核，因为所有后续版本都包括 64 位 x86 架构。

![Linux 2.4 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux24_context_switch_flow.png)

**Linux 2.4.0 (内联汇编)**

```c
/** include/asm-i386/system.h */

#define switch_to(prev,next,last) do {
    asm volatile("pushl %%esi\n\t"
        "pushl %%edi\n\t"
        "pushl %%ebp\n\t"
        "movl %%esp,%0\n\t"	/* save ESP */
        "movl %3,%%esp\n\t"	/* restore ESP */
        "movl $1f,%1\n\t"		/* save EIP */
        "pushl %4\n\t"		/* restore EIP */
        "jmp __switch_to\n"
        "1:\t"
        "popl %%ebp\n\t"
        "popl %%edi\n\t"
        "popl %%esi\n\t"
        :"=m" (prev->thread.esp),"=m" (prev->thread.eip),
         "=b" (last)
        :"m" (next->thread.esp),"m" (next->thread.eip),
         "a" (prev), "d" (next),
         "b" (prev));
} while (0)
```

2.4 内核上下文切换带来了一些小的变化：EBX 不再被推入/弹出，但它现在包含在内联汇编的输出中。我们有一个新的输入参数 `last`，它包含与 prev 相同的值，并通过 EBX 传递出去，但不使用。

```c
:"=m" (prev->thread.esp),"=m" (prev->thread.eip),
:"m" (next->thread.esp),"m" (next->thread.eip),
```

I/O 操作数现在引用内核线程的堆栈/指令指针。以前的上下文切换引用 TSS 中的堆栈指针。

**Linux 2.4.0 (C)**
```c
/** arch/i386/kernel/process.c */
void __switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
	struct thread_struct *prev = &prev_p->thread,
				 *next = &next_p->thread;
				 
	struct tss_struct *tss = init_tss + smp_processor_id();

	unlazy_fpu(prev_p);

	tss->esp0 = next->esp0;

	asm volatile("movl %%fs,%0":"=m" (*(int *)&prev->fs));
	asm volatile("movl %%gs,%0":"=m" (*(int *)&prev->gs));

	/* Restore %fs and %gs. */
	loadsegment(fs, next->fs);
	loadsegment(gs, next->gs);

	/* Now maybe reload the debug registers */
	if (next->debugreg[7]){
		loaddebug(next, 0);
		loaddebug(next, 1);
		loaddebug(next, 2);
		loaddebug(next, 3);
		/* no 4 and 5 */
		loaddebug(next, 6);
		loaddebug(next, 7);
	}

	if (prev->ioperm || next->ioperm) {
		if (next->ioperm) {
			memcpy(tss->io_bitmap, next->io_bitmap,
				 IO_BITMAP_SIZE*sizeof(unsigned long));
			tss->bitmap = IO_BITMAP_OFFSET;
		} else
			tss->bitmap = INVALID_IO_BITMAP_OFFSET;
	}
}
```

C 部分的上下文切换有一些变化。不再提及 TR 寄存器，而是直接更改当前处理器的活动 TSS。与内联 asm 代码一样，每个任务引用其 `task_struct` 内的 `thread_struct` 中的 TSS 数据。每个活动的 CPU 使用 GDT 中的专用 TSS 并直接更新这些字段。

```c
void __switch_to(struct task_struct *prev_p, struct task_struct *next_p)
```

前一个和下一个任务指针现在在符号后附加了 `_p`。这很微妙但很重要，因为 `prev` 和 `next` 即将被重新用于内核线程。

```c
struct thread_struct *prev = &prev_p->thread,
         *next = &next_p->thread;
```

定义指向每个任务的 TSS 数据的指针。

```c
tss->esp0 = next->esp0;
```

将环 0 堆栈偏移替换为新任务的偏移。还不强制页表重新加载...

```c
asm volatile("movl %%fs,%0":"=m" (*(int *)&prev->fs));
asm volatile("movl %%gs,%0":"=m" (*(int *)&prev->gs));
```

存储旧任务的 FS 和 GS。这些段的用途仍然不一致，但它们被使用。最终在 2.6 中，FS 用于线程本地存储，GS 用于每处理器数据区域。

```c
if (prev->ioperm || next->ioperm) {
    if (next->ioperm) {
      memcpy(tss->io_bitmap, next->io_bitmap,
        IO_BITMAP_SIZE*sizeof(unsigned long));
      tss->bitmap = IO_BITMAP_OFFSET;
```

设置即将到来的任务的端口映射 I/O 权限在活动 TSS 中。

```c
} else
tss->bitmap = INVALID_IO_BITMAP_OFFSET;
```

将活动 TSS 的 I/O 权限指向已知的无效位图（0x8000）。


## Linux 2.6 - Linux 走向主流（2003 年）

随着 2.5 内核的结束，O(n) 调度器已经达到了实际使用的极限，AMD 展示了 x86 的扩展，需要内核开发人员立即关注：x86-64。

### Linux 2.6.0

2.6.0 内核带来了恒定时间调度器。虽然这是线性时间调度器的一个进步，但它最终在 2.6.23 中被完全公平调度器（CFS）取代。在上下文切换的另一边，新的 64 位架构推动了迄今为止最显著的变化。

### Linux 2.6.0 - i386 版

这将是本文中最后一次出现 32 位上下文切换。

![Linux 2.6 i386 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux26_i386_context_switch_flow.png)

**Linux 2.6.0 (i386 内联汇编)**

```c
/** include/asm-i386/system.h */
#define switch_to(prev,next,last) do {
    unsigned long esi,edi;
    asm volatile("pushfl\n\t"
            "pushl %%ebp\n\t"
            "movl %%esp,%0\n\t"	 /* save ESP */
            "movl %5,%%esp\n\t" /* restore ESP */
            "movl $1f,%1\n\t"   /* save EIP */
            "pushl %6\n\t"	     /* restore EIP */
            "jmp __switch_to\n"
            "1:\t"
            "popl %%ebp\n\t"
            "popfl"
            :"=m" (prev->thread.esp),"=m" (prev->thread.eip),
             "=a" (last),"=S" (esi),"=D" (edi)
            :"m" (next->thread.esp),"m" (next->thread.eip),
             "2" (prev), "d" (next));
} while (0)
```

删除了四行。ESI 和 EDI 曾经被推入/弹出堆栈，但现在它们通过 I/O 操作数传递。

**Linux 2.6.0 (i386 C)**

```c
/** arch/i386/kernel/process.c */
struct task_struct * __switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
    struct thread_struct *prev = &prev_p->thread,
                 *next = &next_p->thread;
    int cpu = smp_processor_id();
    struct tss_struct *tss = init_tss + cpu;

    __unlazy_fpu(prev_p);

    load_esp0(tss, next->esp0);

    /* Load the per-thread Thread-Local Storage descriptor. */
    load_TLS(next, cpu);

    asm volatile("movl %%fs,%0":"=m" (*(int *)&prev->fs));
    asm volatile("movl %%gs,%0":"=m" (*(int *)&prev->gs));

    /* Restore %fs and %gs if needed. */
    if (unlikely(prev->fs | prev->gs | next->fs | next->gs)) {
        loadsegment(fs, next->fs);
        loadsegment(gs, next->gs);
    }

    /* Now maybe reload the debug registers */
    if (unlikely(next->debugreg[7])) {
        loaddebug(next, 0);
        loaddebug(next, 1);
        loaddebug(next, 2);
        loaddebug(next, 3);
        /* no 4 and 5 */
        loaddebug(next, 6);
        loaddebug(next, 7);
    }

    if (unlikely(prev->io_bitmap_ptr || next->io_bitmap_ptr)) {
        if (next->io_bitmap_ptr) {
            memcpy(tss->io_bitmap, next->io_bitmap_ptr,
                IO_BITMAP_BYTES);
            tss->io_bitmap_base = IO_BITMAP_OFFSET;
        } else
            tss->io_bitmap_base = INVALID_IO_BITMAP_OFFSET;
    }
    return prev_p;
}
```

一些有趣的变化：一个函数返回值和一个 `unlikely()` 宏的出现。我不会考虑将现有代码包装在 \[un\]likely 中作为重新解释其工作原理的原因。宏只是告诉代码生成器哪个基本块情况应该首先出现以帮助流水线。

```c
struct task_struct *__switch_to(...)
```

\_\_switch\_to 现在返回指向 _旧_ 任务的指针。这在任何地方都没有处理，所以要么这个变化被移除了，要么他们想遵循一个约定。例如，当一个函数改变状态时，我们返回先前的状态以便稍后保存和恢复。目前，这没有效果。

```c
load_TLS(next, cpu);
```

更新新任务的线程本地存储描述符在 GDT 中。Linux 2.6 在 GDT 中引入了三个 TLS 条目，用于线程特定的段。TLS 段 1 由 glibc 使用，Wine 使用段 2。我们现在一致使用 FS 段寄存器来引用线程本地存储。

```c
if (unlikely(prev->fs | prev->gs | next->fs | next->gs)) {
```

现在，我们仅在先前或下一个任务使用了 FS 和 GS 时恢复它们。

### Linux 2.6.0 - x86\_64 版

x86\_64 上下文切换与 32 位对应部分有显著不同。我们将更仔细地研究这一点，因为我们将在本文的剩余部分中只关注 64 位。

![Linux 2.6 x86-64 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux26_x8664_context_switch_flow.png)

**Linux 2.6.0 (x86_64 内联汇编)**

```c
/** include/asm-x86_64/system.h */

#define SAVE_CONTEXT    "pushfq ; pushq %%rbp ; movq %%rsi,%%rbp\n\t"
#define RESTORE_CONTEXT "movq %%rbp,%%rsi ; popq %%rbp ; popfq\n\t" 
#define __EXTRA_CLOBBER
    ,"rcx","rbx","rdx","r8","r9","r10","r11","r12","r13","r14","r15"

#define switch_to(prev,next,last)
    asm volatile(SAVE_CONTEXT
          "movq %%rsp,%P[threadrsp](%[prev])\n\t" /* save RSP */
          "movq %P[threadrsp](%[next]),%%rsp\n\t" /* restore RSP */
          "call __switch_to\n\t"
          ".globl thread_return\n"
          "thread_return:\n\t"
          "movq %%gs:%P[pda_pcurrent],%%rsi\n\t"
          "movq %P[thread_info](%%rsi),%%r8\n\t"
          "btr  %[tif_fork],%P[ti_flags](%%r8)\n\t"
          "movq %%rax,%%rdi\n\t"
          "jc   ret_from_fork\n\t"
          RESTORE_CONTEXT
          : "=a" (last)
          : [next] "S" (next), [prev] "D" (prev),
            [threadrsp] "i" (offsetof(struct task_struct, thread.rsp)),
            [ti_flags] "i" (offsetof(struct thread_info, flags)),
            [tif_fork] "i" (TIF_FORK),
            [thread_info] "i" (offsetof(struct task_struct, thread_info)),
            [pda_pcurrent] "i" (offsetof(struct x8664_pda, pcurrent))
          : "memory", "cc" __EXTRA_CLOBBER)
```
x86\_64 带来了 `_switch_to()` 宏的全新外观，因此我们需要逐行解析。许多变化只是寄存器名称（r.. vs e..）。还有一些我上面包含的助手。

```c
asm volatile(SAVE_CONTEXT
```

使用上面显示的助手宏保存内核上下文到堆栈上。与 32 位版本非常相似，只是寄存器名称不同。这与内联 asm 块末尾的 RESTORE\_CONTEXT 帮助配对。

```c
"movq %%rsp,%P[threadrsp](%[prev])\n\t" /* 保存 RSP */
```

将当前堆栈指针存储在旧任务的 TSS 中。注意输入操作数部分中定义的新符号：`[threadrsp]` 是 task\_struct 中 thread.rsp 的立即偏移量。`%P` 解引用 prev:threadsp 指针以确保正确存储更新的 SP。

```c
"movq %P[threadrsp](%[next]),%%rsp\n\t" /* 恢复 RSP */
```

恢复新任务的堆栈指针。

```c
"call __switch_to\n\t"
```

调用 C 部分的上下文切换，在下一节中描述。

```c
".globl thread_return\n"
```

（检查）定义一个名为 `thread_return` 的全局标签。

```c
"thread_return:\n\t"
```

`thread_return` 的跳转点。机械地，使用这将指令指针设置为下一条指令。实际上，这似乎在内核或库（如 glibc）中都没有使用。我假设 pthreads 可能会使用它...但看起来并非如此。

```c
"movq %%gs:%P[pda_pcurrent],%%rsi\n\t"
```

使用每进程数据区域（PDA）引用将源索引设置为当前任务。GS 在内核模式下应始终指向每处理器数据。我们需要。

```c
"movq %P[thread_info](%%rsi),%%r8\n\t"
```

将 `thread_info` 结构移动到 r8 中。我们即将测试此结构中的一个位。`thread_info` 是 Linux 2.6 的新功能，它本质上是 `task_struct`\-lite，并且很容易适合堆栈。

```c
"btr %[tif_fork],%P[ti_flags](%%r8)\n\t"
```

将 `thread_info->flags` 中的 TIF\_FORK 位值存储到 CF 中，并将该位在结构中设置为零。此位将在 fork/clone 后设置，并用于在几行后触发 ret\_from\_fork。

```c
"movq %%rax,%%rdi\n\t"
```

将前一个线程 `task_struct` 存储到 RDI 中。最后一条接触 EAX 的指令是调用 C 函数 \_\_switch\_to，它在 EAX 中返回 `prev`。

```c
"jc ret_from_fork\n\t"
```

如果此线程是新的 fork/clone，则跳转到 ret\_from\_fork 以启动其新生命。

```c
: "=a" (last)
```

前一个线程在 EAX 中返回。

```c
: [next] "S" (next), [prev] "D" (prev),
  [threadrsp] "i" (offsetof(struct task_struct, thread.rsp)),
  [ti_flags] "i" (offsetof(struct thread_info, flags)),
  [tif_fork] "i" (TIF_FORK),
  [thread_info] "i" (offsetof(struct task_struct, thread_info)),
  [pda_pcurrent] "i" (offsetof(struct x8664_pda, pcurrent))
```

内联汇编的输入引用。大多数是包含数据结构偏移量的立即操作数。我们已经在上面运行了它们的用法。

```c
: "memory", "cc" __EXTRA_CLOBBER)
```

这些寄存器在内联 asm 调用中无法存活。

**Linux 2.6.0 (x86_64 C)**

```c
/** arch/x86_64/kernel/process.c */

struct task_struct *__switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
    struct thread_struct *prev = &prev_p->thread,
                 *next = &next_p->thread;
    int cpu = smp_processor_id();  
    struct tss_struct *tss = init_tss + cpu;

    unlazy_fpu(prev_p);

    tss->rsp0 = next->rsp0;

    asm volatile("movl %%es,%0" : "=m" (prev->es)); 
    if (unlikely(next->es | prev->es))
        loadsegment(es, next->es); 
	
    asm volatile ("movl %%ds,%0" : "=m" (prev->ds)); 
    if (unlikely(next->ds | prev->ds))
        loadsegment(ds, next->ds);

    load_TLS(next, cpu);

    /* Switch FS and GS. */
    { 
        unsigned fsindex;
        asm volatile("movl %%fs,%0" : "=g" (fsindex)); 

        if (unlikely(fsindex | next->fsindex | prev->fs)) {
            loadsegment(fs, next->fsindex);
            if (fsindex)
                prev->fs = 0;				
        }
        /* when next process has a 64bit base use it */
        if (next->fs) 
            wrmsrl(MSR_FS_BASE, next->fs); 
        prev->fsindex = fsindex;
    }
    { 
        unsigned gsindex;
        asm volatile("movl %%gs,%0" : "=g" (gsindex)); 
        if (unlikely(gsindex | next->gsindex | prev->gs)) {
            load_gs_index(next->gsindex);
            if (gsindex)
                prev->gs = 0;				
        }
        if (next->gs)
            wrmsrl(MSR_KERNEL_GS_BASE, next->gs); 
        prev->gsindex = gsindex;
    }

    /* Switch the PDA context. */
    prev->userrsp = read_pda(oldrsp); 
    write_pda(oldrsp, next->userrsp); 
    write_pda(pcurrent, next_p); 
    write_pda(kernelstack, (unsigned long)next_p->thread_info + THREAD_SIZE - PDA_STACKOFFSET);

    /* Now maybe reload the debug registers */
    if (unlikely(next->debugreg7)) {
        loaddebug(next, 0);
        loaddebug(next, 1);
        loaddebug(next, 2);
        loaddebug(next, 3);
        /* no 4 and 5 */
        loaddebug(next, 6);
        loaddebug(next, 7);
    }

    /* Handle the IO bitmap */ 
    if (unlikely(prev->io_bitmap_ptr || next->io_bitmap_ptr)) {
        if (next->io_bitmap_ptr) {
            memcpy(tss->io_bitmap, next->io_bitmap_ptr, IO_BITMAP_BYTES);
            tss->io_bitmap_base = IO_BITMAP_OFFSET;
        } else {
            tss->io_bitmap_base = INVALID_IO_BITMAP_OFFSET;
        }
    }

    return prev_p;
}
```
x86\_64 变体的 C 上下文切换增加了一些变化。我不会重复简单的寄存器变化（即 EAX 到 RAX）。

```c
asm volatile("movl %%es,%0" : "=m" (prev->es));
if (unlikely(next->es | prev->es))
    loadsegment(es, next->es);
```

存储旧任务的 ES 段，然后如果合适，加载新的 ES 段。

```c
asm volatile ("movl %%ds,%0" : "=m" (prev->ds));
if (unlikely(next->ds | prev->ds))
    loadsegment(ds, next->ds);
```

存储旧任务的数据段，然后如果合适，加载新的数据段。

```c
unsigned fsindex;
asm volatile("movl %%fs,%0" : "=g" (fsindex));
if (unlikely(fsindex | next->fsindex | prev->fs)) {
    loadsegment(fs, next->fsindex);
    if (fsindex)
        prev->fs = 0;
}
```

将 FS 段移动到 `fsindex` 中，然后如果合适，为新任务加载 FS。基本上，如果旧任务或新任务具有 FS 的有效值，则加载某些内容（可能是 NULL）。FS 通常用于线程本地存储，但根据切换发生的时间，还有其他用途。完全相同的块用于 GS，因此无需重复。GS 通常是 `thread_info` 的段。

```c
if (next->fs)
    wrmsrl(MSR_FS_BASE, next->fs);
```

如果下一个任务使用 FS 寄存器，则确保我们写入整个 64 位值。回想一下，段寄存器是 16/32 位时代的产物，我们需要使用一个特殊函数来确保写入高 32 位。

```c
prev->fsindex = fsindex;
```

存储旧任务的 FS。

```c
prev->userrsp = read_pda(oldrsp);
write_pda(oldrsp, next->userrsp);
write_pda(pcurrent, next_p);
write_pda(kernelstack, (unsigned long)next_p->thread_info +
                THREAD_SIZE - PDA_STACKOFFSET);
```

更新即将到来的任务的 PDA，其中包括存储旧任务的旧 RSP（系统调用）。PDA 使用线程和堆栈信息进行更新。

## Linux 3.0 - 现代操作系统（2011 年）

不要被版本号所迷惑：3.0 在 2.6.0 之后**近 8 年**才发布。变化的数量足以写一本书，我无法在这里涵盖所有内容。至于上下文切换代码的组织，i386 和 x86\_64 合并为仅 x86，并具有单独的进程文件（process\_32.c 和 process\_64.c）。这篇文章已经太长了，所以我将**只继续阅读 x86\_64** 的剩余部分。v3.0 的一些细节将被忽略，因为我们将在最新的 LTS 中更详细地介绍。

![Linux 3.0.1 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux30_context_switch_flow.png)

**Linux 3.0.1 (x86_64 内联汇编)**

```c
/** arch/x86/include/asm/system.h */

#define SAVE_CONTEXT    "pushf ; pushq %%rbp ; movq %%rsi,%%rbp\n\t"
#define RESTORE_CONTEXT "movq %%rbp,%%rsi ; popq %%rbp ; popf\t"
#define __EXTRA_CLOBBER  \
    ,"rcx","rbx","rdx","r8","r9","r10","r11","r12","r13","r14","r15"
	  
#define switch_to(prev, next, last)
    asm volatile(SAVE_CONTEXT
         "movq %%rsp,%P[threadrsp](%[prev])\n\t" /* save RSP */
         "movq %P[threadrsp](%[next]),%%rsp\n\t" /* restore RSP */
         "call __switch_to\n\t"
         "movq "__percpu_arg([current_task])",%%rsi\n\t"
         __switch_canary
         "movq %P[thread_info](%%rsi),%%r8\n\t"
         "movq %%rax,%%rdi\n\t"
         "testl  %[_tif_fork],%P[ti_flags](%%r8)\n\t"
         "jnz   ret_from_fork\n\t"
         RESTORE_CONTEXT
         : "=a" (last)
           __switch_canary_oparam
         : [next] "S" (next), [prev] "D" (prev),
           [threadrsp] "i" (offsetof(struct task_struct, thread.sp)),
           [ti_flags] "i" (offsetof(struct thread_info, flags)),
           [_tif_fork] "i" (_TIF_FORK),
           [thread_info] "i" (offsetof(struct task_struct, stack)),
           [current_task] "m" (current_task)
           __switch_canary_iparam
         : "memory", "cc" __EXTRA_CLOBBER)
```
八年过去了，`switch_to()` 宏只有 4 个变化。其中两个是相关的，没有一个具有震撼性。

```c
movq "__percpu_arg([current_task])",%%rsi\n\t
```

将新的 `task_struct` 移动到 RSI 中。这是访问每个 CPU 的任务信息的“新”方式。以前，这是通过 GS:\[pda offset\] 访问的。随后的 RSI 操作与 2.6 相同。

```c
__switch_canary
```

如果在内核构建期间启用了 `CONFIG_CC_STACKPROTECTOR` 宏，则此宏启用进一步的检查。我不会深入探讨这个兔子洞，只是说这种机制可以防止[为了乐趣和利益而粉碎堆栈](http://www-inst.eecs.berkeley.edu/~cs161/fa08/papers/stack_smashing.pdf)。基本上，存储一个随机值，然后稍后检查它。如果它改变了，坏事发生了。

```c
testl %[_tif_fork],%P[ti_flags](%%r8)\n\t
jnz ret_from_fork\n\t
```

测试新任务是否刚刚通过 clone/fork 创建，然后跳转到 `ret_from_fork()`。以前，这是一个 `btr` 指令，但现在我们在调用后推迟位重置。跳转测试更改为 JNZ，因为测试的变化：如果设置了该位，TEST（AND）将为正。

```c
__switch_canary_oparam
```

`CONFIG_CC_STACKPROTECTOR` 的输出堆栈 canary。

```c
__switch_canary_iparam
```

`CONFIG_CC_STACKPROTECTOR` 的输入堆栈 canary。

**Linux 3.0.1 (x86_64 C)**

```c
/** arch/x86/kernel/process_64.c */

__notrace_funcgraph struct task_struct *
__switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
    struct thread_struct *prev = &prev_p->thread;
    struct thread_struct *next = &next_p->thread;
    int cpu = smp_processor_id();
    struct tss_struct *tss = &per_cpu(init_tss, cpu);
    unsigned fsindex, gsindex;
    bool preload_fpu;

    preload_fpu = tsk_used_math(next_p) && next_p->fpu_counter > 5;

    /* we're going to use this soon, after a few expensive things */
    if (preload_fpu)
        prefetch(next->fpu.state);

    /* Reload esp0, LDT and the page table pointer: */
    load_sp0(tss, next);

    savesegment(es, prev->es);
    if (unlikely(next->es | prev->es))
        loadsegment(es, next->es);

    savesegment(ds, prev->ds);
    if (unlikely(next->ds | prev->ds))
        loadsegment(ds, next->ds);

    savesegment(fs, fsindex);
    savesegment(gs, gsindex);

    load_TLS(next, cpu);

    __unlazy_fpu(prev_p);

    /* Make sure cpu is ready for new context */
    if (preload_fpu)
        clts();

    arch_end_context_switch(next_p);

    /* Switch FS and GS. */
    if (unlikely(fsindex | next->fsindex | prev->fs)) {
        loadsegment(fs, next->fsindex);
        if (fsindex)
            prev->fs = 0;
    }
    /* when next process has a 64bit base use it */
    if (next->fs)
        wrmsrl(MSR_FS_BASE, next->fs);
    prev->fsindex = fsindex;

    if (unlikely(gsindex | next->gsindex | prev->gs)) {
        load_gs_index(next->gsindex);
        if (gsindex)
            prev->gs = 0;
    }
    if (next->gs)
        wrmsrl(MSR_KERNEL_GS_BASE, next->gs);
    prev->gsindex = gsindex;

    /* Switch the PDA and FPU contexts. */
    prev->usersp = percpu_read(old_rsp);
    percpu_write(old_rsp, next->usersp);
    percpu_write(current_task, next_p);

    percpu_write(kernel_stack,
        (unsigned long)task_stack_page(next_p) +
        THREAD_SIZE - KERNEL_STACK_OFFSET);

    /* Now maybe reload the debug registers and handle I/O bitmaps */
    if (unlikely(task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT ||
            task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV))
        __switch_to_xtra(prev_p, next_p, tss);

    /* Preload the FPU context - task is likely to be using it. */
    if (preload_fpu)
        __math_state_restore();

    return prev_p;
}
```

C 代码部分有更多的变化，但考虑到时间跨度，仍然相对较少。有些是表面上的，例如将所有声明移到顶部。以下是新内容：

```c
__notrace_funcgraph struct task_struct * __switch_to(...)
```

新的签名 `__notrace_funcgraph` 防止活动的 ftrace 使用 `switch_to` 的函数图。

```c
preload_fpu = tsk_used_math(next_p) && next_p->fpu_counter > 5;
if (preload_fpu)
        prefetch(next->fpu.state);
```

检查任务是否在过去 5 个时间片中使用了 FPU，然后尝试抓取数据以确保它在缓存中以便稍后使用。

```c
load_sp0(tss, next);
```

加载内核空间堆栈指针，强制页表更新，并现在通知虚拟机管理程序（如果适用）。

```c
savesegment(es, prev->es);
```

存储 ES 段。并不是真正的变化，只是替换了 2.6 中的内联汇编：`asm volatile("movl %%es,%0" : "=m" (prev->es));`。

```c
if (preload_fpu)
        clts();
```

如果使用可能性很高，则立即重新加载 FPU。`clts()` 的使用与我们从 Linux 的第一个版本中看到的相同：`"cmpl %%ecx,%2\n\t     jne 1f\n\t     clts\n"`。

```c
arch_end_context_switch(next_p);
```

这仅在[虚拟化](https://elixir.bootlin.com/linux/v3.0.1/source/arch/x86/include/asm/paravirt.h#L718)下才有意义。在正常情况下，此函数[不执行任何操作](https://elixir.bootlin.com/linux/v3.0.1/source/arch/x86/include/asm/pgtable.h#L82)。更多细节在最终内核版本中。

```c
if (unlikely(task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT ||
        task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV))
    __switch_to_xtra(prev_p, next_p, tss);
```

处理曾经在 `switch_to` 尾部的管理工作，包括调试寄存器和 I/O 位图设置。我将为 4.14.62 详细的代码解析保留这一点。

```c
if (preload_fpu)
        __math_state_restore();
```

在验证高水平的 FPU 使用后执行 FPU 恢复。希望它现在已经通过之前的预取进入了缓存。

## Linux 4.14.67 - 最新的 LTS 内核（2018 年）

这将是我们对上下文切换内部工作原理的最深入探讨。自 3.0 以来，该过程受到了很多关注，包括代码组织。总体而言，这段代码感觉比以往任何时候都更干净、更有条理。对于 x86\_64：

![Linux 4.14.67 上下文切换流程](https://www.maizure.org/projects/evolution_x86_context_switch_linux/linux41467_context_switch_flow.png)

- 顶级 `switch_to()` 宏自 ~v3.4 以来在 [arch/x86/include/asm/switch\_to.h](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/include/asm/switch_to.h#L71) 中有自己的头文件。该宏在 [kernel/sched/core.c](https://elixir.bootlin.com/linux/v4.14.67/source/kernel/sched/core.c#L2807) 中的 `context_switch()` 末尾被调用一次。
- 自 4.9 以来，`switch_to()` 已分为两部分：`prepare_switch_to()` 宏和内联汇编部分已移至实际的汇编文件（[arch/x86/entry/entry\_64.S](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/entry/entry_64.S#L333)）。
- 切换的 C 部分仍然通过汇编代码中间的跳转发生。自 2.6.24 以来，源代码位于 [arch/x86/kernel/process\_64.c](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/kernel/process_64.c#L395)。

**Linux 4.14.67**

```c
/** arch/x86/include/asm/switch_to.h */

#define switch_to(prev, next, last)
do {
    prepare_switch_to(prev, next);

    ((last) = __switch_to_asm((prev), (next)));	
} while (0)
```

与旧内核相比，看起来很简单。这种重组是 Andy Lutomirski 在实现[虚拟映射内核堆栈](https://lwn.net/Articles/692208/)时出现的问题的结果。

```c
prepare_switch_to(prev, next);
```

确保内核堆栈在尝试上下文切换之前已就位并可访问。这避免了在使用虚拟映射内核堆栈时尝试上下文切换时可能出现的双重故障/内核恐慌。

```c
((last) = __switch_to_asm((prev), (next)));
```

启动实际的上下文切换。

我们将快速查看 `prepare_switch_to`，定义在同一源文件中。

**Linux 4.14.67**

```c
/** arch/x86/include/asm/switch_to.h */

static inline void prepare_switch_to(struct task_struct *prev,
				     struct task_struct *next)
{
#ifdef CONFIG_VMAP_STACK
	READ_ONCE(*(unsigned char *)next->thread.sp);
#endif
}
```

```c
#ifdef CONFIG_VMAP_STACK
```

当堆栈使用虚拟内存时定义。我们只需要在使用虚拟堆栈时准备上下文切换。这是内核构建时的配置选项，默认情况下在许多现代发行版中为 yes。

```c
READ_ONCE(*(unsigned char *)next->thread.sp);
```

现在触摸下一个堆栈以修复页表（pgd）。核心问题是我们试图访问一个不仅被分页出去，而且由于 vmalloc 区域的延迟加载，甚至不在此任务的内存上下文中的指针。不在这里且不可用[意味着内核恐慌](https://lkml.org/lkml/2016/8/11/169)，如果在我们需要它之前没有处理。

**Linux 4.16.67**

```c
/** arch/x86/entry/entry_64.S */

ENTRY(__switch_to_asm)
    UNWIND_HINT_FUNC
	
    /* Save callee-saved registers */
    pushq	%rbp
    pushq	%rbx
    pushq	%r12
    pushq	%r13
    pushq	%r14
    pushq	%r15

    /* switch stack */
    movq	%rsp, TASK_threadsp(%rdi)
    movq	TASK_threadsp(%rsi), %rsp

#ifdef CONFIG_CC_STACKPROTECTOR
    movq	TASK_stack_canary(%rsi), %rbx
    movq	%rbx, PER_CPU_VAR(irq_stack_union)+stack_canary_offset
#endif

#ifdef CONFIG_RETPOLINE
    FILL_RETURN_BUFFER %r12, RSB_CLEAR_LOOPS, X86_FEATURE_RSB_CTXSW
#endif

    /* restore callee-saved registers */
    popq	%r15
    popq	%r14
    popq	%r13
    popq	%r12
    popq	%rbx
    popq	%rbp

    jmp	__switch_to
END(__switch_to_asm)
```

[entry\_64.S](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/entry/entry_64.S#L333) 包含前 25 年 Linux 中通过内联汇编完成的工作。

```c
UNWIND_HINT_FUNC
```

生成由过去一年引入的 objtool 堆栈跟踪工具使用的提示。这对于不符合通常 C 调用约定的特殊汇编过程是必需的。像这样的提示是 [ORC 展开器](https://lwn.net/Articles/728339/)自 v4.6 以来成功采用的原因。

```c
pushq %rbp, %rbx, %r12, %r13, %r14, %r15
```

将调用者保存的寄存器存储到我们即将切换的**旧堆栈**中。

```c
movq %rsp, TASK_threadsp(%rdi)
movq TASK_threadsp(%rsi), %rsp
```

在旧任务和新任务之间交换堆栈指针。从周围的汇编中并不直接清楚，但 RDI 和 RSI 包含输入参数 `task_struct *`，**prev** 和 **next**。这遵循 System V ABI 约定。以下是寄存器及其用法的子集：

![Linux x86 上下文切换](https://www.maizure.org/projects/evolution_x86_context_switch_linux/systemV_abi_registers.png)  

```c
#ifdef CONFIG_CC_STACKPROTECTOR
    movq TASK_stack_canary(%rsi), %rbx
    movq %rbx, PER_CPU_VAR(irq_stack_union)+stack_canary_offset
```

如果启用了堆栈保护，则将此任务的 canary 值移动到当前 CPU 的中断堆栈中的适当位置。堆栈保护通常默认启用，因此这很可能会发生。

```c
#ifdef CONFIG_RETPOLINE
    FILL_RETURN_BUFFER %r12, RSB_CLEAR_LOOPS, X86_FEATURE_RSB_CTXSW
```

返回蹦床是[缓解](https://lkml.org/lkml/2018/1/4/724)分支预测（Spectre 漏洞）可能被利用的[措施](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/include/asm/nospec-branch.h#L36)。纯[巫术](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/include/asm/nospec-branch.h#L36)！

```c
popq %r15, %r14, %r13, %r12, %rbx, %rbp
```

从**新堆栈**中恢复所有寄存器，以相反的顺序推入：（r15, r14, r13, r12, rbx, rbp）。

**Linux 4.16.67 (带注释的源代码)**

```c
/** arch/x86/kernel/process_64.c */

__visible __notrace_funcgraph struct task_struct *
__switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
    struct thread_struct *prev = &prev_p->thread;
    struct thread_struct *next = &next_p->thread;
    struct fpu *prev_fpu = &prev->fpu;
    struct fpu *next_fpu = &next->fpu;
    int cpu = smp_processor_id();
    struct tss_struct *tss = &per_cpu(cpu_tss_rw, cpu);

    WARN_ON_ONCE(IS_ENABLED(CONFIG_DEBUG_ENTRY) &&
            this_cpu_read(irq_count) != -1);

    switch_fpu_prepare(prev_fpu, cpu);

    save_fsgs(prev_p);

    load_TLS(next, cpu);

    arch_end_context_switch(next_p);

    savesegment(es, prev->es);
    if (unlikely(next->es | prev->es))
        loadsegment(es, next->es);

    savesegment(ds, prev->ds);
    if (unlikely(next->ds | prev->ds))
        loadsegment(ds, next->ds);

    load_seg_legacy(prev->fsindex, prev->fsbase,
            next->fsindex, next->fsbase, FS);
    load_seg_legacy(prev->gsindex, prev->gsbase,
            next->gsindex, next->gsbase, GS);

    switch_fpu_finish(next_fpu, cpu);

    /* Switch the PDA and FPU contexts. */
    this_cpu_write(current_task, next_p);
    this_cpu_write(cpu_current_top_of_stack, task_top_of_stack(next_p));

    /* Reload sp0. */
    update_sp0(next_p);

    /* Now maybe reload the debug registers and handle I/O bitmaps */
    if (unlikely(task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT ||
            task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV))
        __switch_to_xtra(prev_p, next_p, tss);

#ifdef CONFIG_XEN_PV
    if (unlikely(static_cpu_has(X86_FEATURE_XENPV) &&
            prev->iopl != next->iopl))
        xen_set_iopl_mask(next->iopl);
#endif

    if (static_cpu_has_bug(X86_BUG_SYSRET_SS_ATTRS)) {
        unsigned short ss_sel;
        savesegment(ss, ss_sel);
        if (ss_sel != __KERNEL_DS)
            loadsegment(ss, __KERNEL_DS);
	}

    /* Load the Intel cache allocation PQR MSR. */
    intel_rdt_sched_in();

    return prev_p;
}
```

这个最后的[代码](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/kernel/process_64.c#L395)块使我们了解了今天的上下文切换！如果你直接跳到这里，别担心 - 我会（重新）涵盖大部分要点，并更详细地介绍。注意上下文切换中泄漏的角落案例数量。

```c
__visible __notrace_funcgraph struct task_struct * __switch_to(struct task_struct *prev_p, struct task_struct *next_p)
```

C 上下文切换的签名有几个部分需要解包：

- **\_\_visible** - 此属性确保[链接时优化](https://lwn.net/Articles/512548/)不会移除 `__switch_to()` 符号。
- **\_\_notrace\_funcgraph** - 过滤 `__switch_to()` 使其不出现在 ftrace 函数图中。此功能在 v2.6.29 左右添加，并在此后不久包含此函数。
- 输入参数是指向旧任务和新任务的指针，它们分别在 RDI 和 RSI 中传递。

```c
struct thread_struct *prev = &prev_p->thread;
struct thread_struct *next = &next_p->thread;
struct fpu *prev_fpu = &prev->fpu;
struct fpu *next_fpu = &next->fpu;
```

从两个输入 `task_struct *` 中收集一些信息。`thread_struct` [包含](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/include/asm/processor.h#L451) 任务的 TSS 数据（寄存器等）。`fpu>` 结构体 [包含](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/include/asm/fpu/types.h#L281) fpu 数据，例如上次使用的 CPU、初始化和寄存器值。

```c
int cpu = smp_processor_id();
```

获取我们需要的处理器编号，以便管理 CPU 特定的 GDT 以获取 TSS 数据、线程本地存储和比较 fpu 状态。

```c
struct tss_struct *tss = &per_cpu(cpu_tss_rw, cpu);
```

指向[当前 CPU TSS](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/kernel/process.c#L51)。

```c
WARN_ON_ONCE(IS_ENABLED(CONFIG_DEBUG_ENTRY) &&
        this_cpu_read(irq_count) != -1);
```

检测上下文切换期间是否使用了 IRQ 堆栈，并在每次启动时报告一次。这是在 4.14 开发早期添加的，对上下文切换没有实际影响。

```c
switch_fpu_prepare(prev_fpu, cpu);
```

在我们仍在旧任务中时保存当前的 FPU 状态。

```c
save_fsgs(prev_p);
```

现在存储 FS 和 GS，然后我们更改线程本地存储。

```c
load_TLS(next, cpu);
```

为新任务的线程本地存储重新加载 GDT。机械地，这将新线程中的 tls\_array 复制到 GDT 条目 6、7 和 8 中。

```c
arch_end_context_switch(next_p);
```

此函数仅在虚拟化下定义。更改虚拟化模式并清理任何剩余的批处理工作。在 2.6 系列后期引入。这不是我的强项，所以我将留给读者[研究](https://en.wikipedia.org/wiki/Paravirtualization#Linux_paravirtualization_support)。

```c
savesegment(es, prev->es);
    if (unlikely(next->es | prev->es))
    loadsegment(es, next->es);
```

存储 ES 段并在适当时加载新的 ES 段。类似的 DS 调用被省略。即使新任务不使用 DS/ES，它仍然清除任何旧值。

```c
load_seg_legacy(prev->fsindex, prev->fsbase,
next->fsindex, next->fsbase, FS);
```

加载新的 FS 段（GS 省略）。这将检测并加载 32 和 64 位寄存器类型的寄存器。新任务现在已准备好进行 TLS 操作。

```c
switch_fpu_finish(next_fpu, cpu);
```

为新任务初始化 FPU 状态。

```c
this_cpu_write(current_task, next_p);
```

更新 CPU 的当前任务（`task_struct *`。有效地更新新的 FPU 和 PDA（每处理器数据区域）状态。

```c
this_cpu_write(cpu_current_top_of_stack,
    task_top_of_stack(next_p));
```

更新 CPU 的堆栈顶部指针，这实际上是 sp1 作为[入口蹦床](https://elixir.bootlin.com/linux/v4.14.67/source/arch/x86/entry/entry_64.S#L162)的安全性的重载使用。

```c
update_sp0(next_p);
```

点击新堆栈以验证它。这似乎不是 sp0，而是 sp1？可能应该重命名。

```c
if (unlikely(task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT ||
        task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV))
    __switch_to_xtra(prev_p, next_p, tss);
```

更新调试寄存器和 I/O 位图。这两个任务曾经直接在上下文切换中处理，但现在已移至 `__switch_to_xtra()`。

```c
#ifdef CONFIG_XEN_PV
if (unlikely(static_cpu_has(X86_FEATURE_XENPV) &&
        prev->iopl != next->iopl))
    xen_set_iopl_mask(next->iopl);
```

手动交换 Xen 虚拟化的 I/O 特权位。显然，通常的标志切换[无法正常工作](https://people.canonical.com/~ubuntu-security/cve/2016/CVE-2016-3157.html)，因此必须直接屏蔽当前位。

```c
if (static_cpu_has_bug(X86_BUG_SYSRET_SS_ATTRS)) {
    unsigned short ss_sel;
    savesegment(ss, ss_sel);
    if (ss_sel != __KERNEL_DS)
        loadsegment(ss, __KERNEL_DS);
```

掩盖 AMD CPU 中[意外的 SYSRET 行为](https://lore.kernel.org/patchwork/patch/564016/)，该行为无法正确更新段描述符。

```c
intel_rdt_sched_in();
```

一些 Intel 资源管理内务处理。更新 [RMID 和 CLOSid](https://software.intel.com/en-us/blogs/2014/12/11/intel-s-cache-monitoring-technology-software-visible-interfaces)。

```c
return prev_p;
```

完成！

## 常见问题

**为什么选择这些内核版本？**  
第一个和最后一个版本是明显的书挡。我最初计划了 4 个中间版本（2.1、2.3、2.5 和 2.6.26），但其中一些变化不足以使文章膨胀。它已经太长了。

**这项研究花了多长时间？**  
两周。一周的代码阅读、笔记记录和技术手册。然后一周的转录笔记、制作图表和格式化这篇文章。

**4.14.67 不是最新的 LTS 版本吗？**  
我在 9 月 1 日开始阅读代码，并拉取了 4.14.67 的源代码。4.14.68 在四天后最终确定。

**更多问题将随着它们的到来而添加**