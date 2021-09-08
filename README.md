# Windows-Hook-攻防学习
学习Windows系统及其Hook技术中的学习和思考

第一章 SetWindowsHookEx实现键盘记录器
> “无忌，我教你的还记得多少？”
> “回太师傅，我只记得一大半。”
> “那，现在呢？”
> “已经剩下一小半了。”
> “那，现在呢？”
> “我已经把所有的全忘记了！”
> “好，你可以上了！”

羡慕张无忌，忘记了武功，打败了敌人；而我自己，忘记了武功，菜哭了自己。

已经一年多没有接触这个了，忘得干干净净，幸亏肌肉记忆还在，学起来十分亲切，废话不多说，进入正题。

# 前言

Hook是程序设计中最为灵活多变的技巧之一，在windows下，Hook有两种含义：
1、系统提供的消息Hook机制，也就是我们今天要实现的hook，在后面会讲到。
2、自定义的Hook编程技巧

​		自定义的Hook编程技巧则是基于特定系统结构、文件结构、汇编语言的一种高级技术，运用自如后犹如手握屠龙刀倚天剑。

操作环境：

visual studio 2015  

Windows 10

# 消息Hook

Windows消息hook主要通过SetWindowsHookEx去下钩，通过UnhookWindowsHookEx()去断钩，所以我们接下来主要学会用这俩个函数。

我们之所以可以进行消息Hook，是因为**Windows的消息机制**；简单来说：

Windows系统是建立在事件驱动的机制上的，每一个事件就是一个消息，每个运行中的程序，也就是所谓的进程，都维护者一个或多个消息队列，消息队列的个数取决于进程内包含的线程的个数。由于一个进程至少要拥有一个线程，所以进程至少要有一个消息队列。虽然Windows系统的消息分派是以线程为单位的，但并不是所有的线程都有消息队列，一个新创建的线程是没有消息队列的，只有当线程第一次调用GDI或USER32库函数的时候Windows才为线程创建消息队列。消息最终由属于线程的窗口来处理，普通的应用程序只能获取本线程的消息队列中的消息，也就是只能获得系统分派的、属于本线程的消息，换句话说，一个线程在运行过程中是不知道其它线程发生了什么事情的。**但是有一类特殊的程序却可以访问其他线程的消息队列，那就是钩子程序。**

(Windows消息机制还是十分重要的，这里我不多赘述，了解这些内容，足够咱们接下来的学习了，想要了解更多，建议看这篇文章https://zhuanlan.zhihu.com/p/42992978)

 编写钩子程序是Windows系统提供给用户的一种对Windows运行过程进行干预的机制，通过钩子程序，Windows将内部流动的消息暴露给用户，使用户能够在消息被窗口管理器分派之前对其进行特殊的处理，比如在调试程序的时候跟踪消息流程。但是，任何事情都有其两面性，一些密码窃取工具就是利用系统键盘钩子截获其他程序的键盘消息，从而获取用户输入的密码，可见非法的钩子程序对计算机信息安全具有极大的危害性。

（那么我们今天就通过消息hook实现一个简单的键盘记录器，嘿嘿，知己知彼，百战不殆）！

## 消息Hook流程

第一步：安装钩子

最后一步：卸载钩子

中间步骤：我们要实现回调函数，实现我们自己的操作。例如获得键盘输入信息，并将其保存到txt文件中

# 通过setWindowsHookEx()实现键盘记录器

## **实现原理**

当按下键盘，产生一个消息，按键消息加入到系统消息队列  操作系统从消息队列中取出消息，添加到相应的程序的消息队列中 ；

应用程序使用消息Hook从自身的消息队列中取出消息WM_KEYDOWN，调用消息处理函数。  我们可以在系统消息队列之间添加消息钩子，从而使得在系统消息队列消息发给应用程序之前捕获到消息。  

可以多次添加钩子，从而形成一个钩子链，可以依次调用函数。

## **安装钩子**

SetWindowsHookEx

```c++
WINUSERAPI
HHOOK
WINAPI
SetWindowsHookEx(
     //钩子类型
    _In_ int idHook,
    //回调函数地址
    _In_ HOOKPROC lpfn,
    //实例句柄(包含有钩子函数)
    _In_opt_ HINSTANCE hmod,
    //线程ID，欲勾住的线程（为0则不指定，全局）
    _In_ DWORD dwThreadId);
```

能设置的hook类型如下：

| 宏值              | 含义                                                         |
| ----------------- | ------------------------------------------------------------ |
| WH_MSGFILTER      | 截获用户与控件交互的消息                                     |
| WH_KEYBOARD       | 截获键盘消息                                                 |
| WH_GETMESSAGE     | 截获从消息队列送出的消息                                     |
| WH_CBT            | 截获系统基本消息，激活，建立，销毁，最小化，最大化，移动，改变尺寸等窗口事件 |
| WH_MOUSE          | 截获鼠标消息                                                 |
| WH_CALLWNDPROCRET | 截获目标窗口处理完毕的消息                                   |

我们这里hook键盘消息：

```
SetWindowsHookEx(
		WH_KEYBOARD_LL, //  low-level keyboard input events
        HookProcedure, //  回调函数地址
        GetModuleHandle(NULL), // A handle to the DLL containing the hook procedure 
        NULL //线程ID，欲勾住的线程（为0则不指定，全局）
    );
```

为钩子链中的下一个子程序设置钩子。在钩子子程中调用得到控制权的钩子函数在完成对消息的处理后，如果想要该消息继续传递，那么它必须调用另外一个 SDK中的API函数CallNextHookEx来传递它，以执行钩子链表所指的下一个钩子子程。

```C++
WINUSERAPI
LRESULT
WINAPI
CallNextHookEx(
    //钩子句柄，由SetWindowsHookEx()函数返回。
    _In_opt_ HHOOK hhk,
    
    //钩子事件代码，回调函数的钩子过程的事件代码
    _In_ int nCode,
    
    //传给钩子子程序的wParam值
    _In_ WPARAM wParam,
    
    //传给钩子子程序的lParam值
    _In_ LPARAM lParam);
```

## 卸载钩子

卸载钩子API，钩子在使用完之后需要用UnhookWindowsHookEx()卸载，否则会造成麻烦。

```C++
WINUSERAPI
BOOL
WINAPI
UnhookWindowsHookEx(
     //要删除的钩子的句柄。这个参数是上一个函数SetWindowsHookEx的返回值.
    _In_ HHOOK hhk);
```

## 键盘记录

主要的功能我们通过SetwindowsHookEx()参数中的回调函数HookProcedure()来实现。

```C++
LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam)
```

在里面我们会实现获得256个虚拟键的状态，并将其转换为真实字符

![image-20210908224511747](C:\Users\11073\AppData\Roaming\Typora\typora-user-images\image-20210908224511747.png)

输出：这里我们写了俩种方式

1.输出到控制台

2.输出到文本，并保存

## 一些逻辑

获得当前窗口和当前时间；

将记录的键盘消息保存到文件中；

## 效果

x86效果：

![image-20210908222139479](C:\Users\11073\AppData\Roaming\Typora\typora-user-images\image-20210908222139479.png)

x64位

![image-20210908222222498](C:\Users\11073\AppData\Roaming\Typora\typora-user-images\image-20210908222222498.png)

保存在txt文件中

![image-20210908222315186](C:\Users\11073\AppData\Roaming\Typora\typora-user-images\image-20210908222315186.png)





# 参考资料

[【windows核心编程】系统消息与自定义钩子（Hook）使用](https://www.cnblogs.com/17bdw/p/6533065.html)

[HOOK教程一_使用SetWindowsHookEx进行Windows消息HOOK](https://wenku.baidu.com/view/7b14e144767f5acfa1c7cd08.html)

