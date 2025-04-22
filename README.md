

# glog阅读

## 测试环境 源码阅读方式

### 环境
- Ubuntu 24 wsl2 版本

- cmake 后端生成器为 Ninja 

- 编译器g++

- gdb调试

### 源码阅读方式
依据官方文档给出的函数使用方式，使用程序调试进行了解追溯函数的调用流程，以及总结每个类之间的关系，了解特殊技巧等 
开始！

## 源码阅读开始！

### 严重级别 

- INFO
- WARNING
- ERROR
- FATAL

对于安全等级是从```INFO```到```FATAL```从低到高，并且当触发了```FATAL```严重级别时程序会被终止.

对于从低到高的不同严重级别他们不只会单单记录在自己级别的文件当中，他们会被记录到自己以及高于自己严重级别的文件当中

e.g.:当你使用```WARNING```严重级别时，触发记录日志之后他会被记录在 自己严重等级的文件中: ```WARNING```文件，以及高于自己严重等级的文件中 :```ERROR , FATAL``` 

### 日志文件命名方式

``` 
<tmp>/<program name>.<hostname>.<user name>.log.<severity level>.<date>-<time>.<pid>
<tmp>/<程序名>.<主机名>.<用户名>.log.<严重级别>.<日期>-<时间>.<进程号>
  |
  临时路径
```

对于linux系统glog会通过环境变量确定目录

- ```TMPDIR```

- ```TMP```

如果都未设置就会退回到```/tmp/```

### 日志的前缀格式

``` 
Lyyyymmdd hh:mm:ss.uuuuuu threadid file:line] msg...
```

| 占位符            | 含义                                      |
| ----------------- | ----------------------------------------- |
| `L`               | 单个字符，表示日志级别（如`I`代表`INFO`） |
| `yyyy`            | 年份                                      |
| `mm`              | 月份(补零 5月就是 05)                     |
| `dd`              | 日期(补零)                                |
| `hh:mm:ss.uuuuuu` | 时间（小时、分钟和秒的小数部分）          |
| `threadid`        | 空格填充的线程ID                          |
| `file             | 文件名                                    |
| `line`            | 行号                                      |
| `msg`             | 用户提供的信息                            |

``` 
e.g.:
I1103 11:57:31.739339 24395 google.cc:2341] Command line: ./some_prog
I1103 11:57:31.739403 24395 google.cc:2342] Process id 24395   
```

### 格式的自定义

我们可以```google::InstallPrefixFormatter(&MyPrefixFormatter);```来自定义自己的日志格式

```c++
// 源函数实现
void InstallPrefixFormatter(PrefixFormatterCallback callback, void* data) {
  if (callback != nullptr) {
    g_prefix_formatter = std::make_unique<PrefixFormatter>(callback, data);
  } else {
    g_prefix_formatter = nullptr;
  }
}
// 对于PrefixFormatterCallback的using
// 我们可以看到要求的回调函数是一个返回void 参数列表为 
// std::ostream&, const LogMessage&, void*
using PrefixFormatterCallback = void (*)(std::ostream&, const LogMessage&,
                                         void*);
```

回调函数会在要格式化字符时被调用,并传入三种参数类型 最后一个void* 可以使我们自定义自己传入的参数

```c++
// 回调函数的例子
void MyPrefixFormatter(std::ostream& s, const google::LogMessage& m, void* /*data*/) {
   s << google::GetLogSeverityName(m.severity())[0]
   << setw(4) << 1900 + m.time().year()
   << setw(2) << 1 + m.time().month()
   << setw(2) << m.time().day()
   << ' '
   << setw(2) << m.time().hour() << ':'
   << setw(2) << m.time().min()  << ':'
   << setw(2) << m.time().sec() << "."
   << setw(6) << m.time().usec()
   << ' '
   << setfill(' ') << setw(5)
   << m.thread_id() << setfill('0')
   << ' '
   << m.basename() << ':' << m.line() << "]";
}

```

## 在使用glog的初始化

```c++
#include <glog/logging.h>

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    return 0;
}

```

调用 ```google::InitGoogleLogging(argv[0]);```函数对glog初始化

## 条件/偶尔日志记录

### LOG_IF

```c++
// 满足condition条件下记录日志
LOG_IF(INFO, condition > 10) << "满足condition,并以INFO严重等级log";
```
```c++
// 测试程序
#include <glog/logging.h>

int main(int argc, char* argv[]){
  google::InitGoogleLogging(argv[0]);
  int condition = 11;
  LOG_IF(INFO,condition > 10) << "condition满足,启动日志";
return 0;
}
```

```
// log文件中的内容
Log file created at: 2025/04/22 16:46:41
Running on machine: rzzrz
Running duration (h:mm:ss): 0:00:00
Log line format: [IWEF]yyyymmdd hh:mm:ss.uuuuuu threadid file:line] msg
I20250422 16:46:41.439733 140149972471360 main.cpp:6] condition满足,启动日志
```

改宏的实现源码为

```c++
#define LOG_IF(severity, condition) \
  static_cast<void>(0),             \
      !(condition)                  \
          ? (void)0                 \
          : google::logging::internal::LogMessageVoidify() & LOG(severity)
```

我们看到改宏展开后会根据condition来展开

在这里要是用将```0```转换成static_cast<void>(0)呢,且为什么后面的```0```也要```(void)0```转换

- 有些编译器会在```condition```为```flase```时展开宏为```0```时报错 ```未使用结果```,这里转换类型告诉编译器不用管
- void(0)的类型转换是因为 三目表达式要求 ```:``` 冒号左右两边的参数应当是相同的类型

### LOG_EVERY_N

```c++
// 每间隔n次执行
LOG_EVERY_N(INFO, 10) << "获得第" << google::COUNTER << "个cookie";
// 该行在第1、11、21...次执行时输出日志消息
// 也就是调用的第一次,并间隔n次执行

```

```c++
// 测试程序
#include <glog/logging.h>

int main(int argc, char* argv[]){
  google::InitGoogleLogging(argv[0]);
  for (int i = 0; i < 30; i++) {
    LOG_EVERY_N(INFO, 10) << "会在执行第1, 11, 21次log";
  }
    
return 0;
}
```

```
// 文件内容
Log file created at: 2025/04/22 17:06:04
Running on machine: rzzrz
Running duration (h:mm:ss): 0:00:00
Log line format: [IWEF]yyyymmdd hh:mm:ss.uuuuuu threadid file:line] msg
I20250422 17:06:04.221853 139899694952000 main.cpp:6] 会在执行第1, 11, 21次log
I20250422 17:06:04.222337 139899694952000 main.cpp:6] 会在执行第1, 11, 21次log
I20250422 17:06:04.222343 139899694952000 main.cpp:6] 会在执行第1, 11, 21次log
```

```c++
// 源码实现
#define LOG_IF_EVERY_N(severity, condition, n)            \
  SOME_KIND_OF_LOG_IF_EVERY_N(severity, (condition), (n), \
                              google::LogMessage::SendToLog)
// 首先将severity, condition, n参数传给SOME_KIND_OF_LOG_IF_EVERY_N
// 并将sinks设置为google::LogMessage::SendToLog

#define SOME_KIND_OF_LOG_IF_EVERY_N(severity, condition, n, what_to_do)       \
// 定义静态原子int记录调用次数以及调用次数模N后的值
// 保证只定义一次变量并且使用原子变量保证线程安全
  static std::atomic<int> LOG_OCCURRENCES(0), LOG_OCCURRENCES_MOD_N(0);       \
// GLOG_IFDEF_THREAD_SANITIZER适用于启用ThreadSanitizer标记上面两个变量为良性竞争的
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                        \
      __FILE__, __LINE__, &LOG_OCCURRENCES, sizeof(int), ""));                \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                        \
      __FILE__, __LINE__, &LOG_OCCURRENCES_MOD_N, sizeof(int), ""));          \
// 每次调用就会增加计数
  ++LOG_OCCURRENCES;                                                          \
  if ((condition) &&                                                          \
      ((LOG_OCCURRENCES_MOD_N = (LOG_OCCURRENCES_MOD_N + 1) % n) == (1 % n))) \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity,             \
                     LOG_OCCURRENCES, &what_to_do/*定义sink行为*/)                            \
      .stream()
```

以上就是LOG_IF_EVERY_N实现方式

其中要注意的是使用LOG_OCCURRENCES , LOG_OCCURRENCES_MOD_N这两个宏来实现定义不重名的变量

``` c++
#define LOG_OCCURRENCES LOG_EVERY_N_VARNAME(occurrences_, __LINE__)
#define LOG_EVERY_N_VARNAME(base, line) LOG_EVERY_N_VARNAME_CONCAT(base, line)
#define LOG_EVERY_N_VARNAME_CONCAT(base, line) base##line
// 最终生成occurrences_xx的变量饼子
```

他为什么要进行很多次转换呢?

根本原因就是我们需要实现将```occurrences_```和```__LINE__```这两个字符串进行合并,
我们需要考虑到当只用一步宏展开时会出现下面的问题

```c++
#define MY_CONTACT_DEF occurrences_##__LINE__
// 最终的结果并不是occurrences_xx而是只有一种结果的occurrences__LINE__
// 也就是说__LINE__并没有被展开而是直接被作为字符换进行处理了,没有达到使用行号定义独立变量的目的
// 所以我们考虑到将__LINE__宏展开,也就是一下过程
#define LOG_OCCURRENCES LOG_EVERY_N_VARNAME(occurrences_, __LINE__)
#define LOG_EVERY_N_VARNAME(base, line) LOG_EVERY_N_VARNAME_CONCAT(base, line)
// 最后粘合变量名
#define LOG_EVERY_N_VARNAME_CONCAT(base, line) base##line
```

也就是通过宏的```##```和```__LINE__```操作 以及static修饰定义原子变量实现计数
并判断condition以及两个原子变量之间的关系进行比较结果决定是否进行log
