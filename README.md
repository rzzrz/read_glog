

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

### 条件/偶尔日志记录

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

### LOG_IF_EVERY_N

```c++
// 该宏可以实现每n次并满足条件时进行log
LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "获得第" << google::COUNTER
                                       << "个大cookie";
```

```c++
// 最后结果
GNU nano 7.2                    main.rzzrz.rzz-rz.log.INFO.20250424-122715.27680                              Log file created at: 2025/04/24 12:27:15
Running on machine: rzzrz
Running duration (h:mm:ss): 0:00:00
Log line format: [IWEF]yyyymmdd hh:mm:ss.uuuuuu threadid file:line] msg
I20250424 12:27:15.815013 140094245674560 main.cpp:6] 获得第1026个大cookie
I20250424 12:27:15.815419 140094245674560 main.cpp:6] 获得第1036个大cookie
I20250424 12:27:15.815426 140094245674560 main.cpp:6] 获得第1046个大cookie
I20250424 12:27:15.815429 140094245674560 main.cpp:6] 获得第1056个大cookie
I20250424 12:27:15.815431 140094245674560 main.cpp:6] 获得第1066个大cookie
I20250424 12:27:15.815433 140094245674560 main.cpp:6] 获得第1076个大cookie
I20250424 12:27:15.815435 140094245674560 main.cpp:6] 获得第1086个大cookie
I20250424 12:27:15.815438 140094245674560 main.cpp:6] 获得第1096个大cookie
I20250424 12:27:15.815440 140094245674560 main.cpp:6] 获得第1106个大cookie
I20250424 12:27:15.815442 140094245674560 main.cpp:6] 获得第1116个大cookie
```

```c++
// 源码实现和上面的差不多只是加了一些条件判断
#define SOME_KIND_OF_LOG_IF_EVERY_N(severity, condition, n, what_to_do)       \
  static std::atomic<int> LOG_OCCURRENCES(0), LOG_OCCURRENCES_MOD_N(0);       \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                        \
      __FILE__, __LINE__, &LOG_OCCURRENCES, sizeof(int), ""));                \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                        \
      __FILE__, __LINE__, &LOG_OCCURRENCES_MOD_N, sizeof(int), ""));          \
  ++LOG_OCCURRENCES;                                                          \
  if ((condition) &&                                                          \
      ((LOG_OCCURRENCES_MOD_N = (LOG_OCCURRENCES_MOD_N + 1) % n) == (1 % n))) \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity,             \
                     LOG_OCCURRENCES, &what_to_do)                            \
      .stream()
```

### LOG_FIRST_N

``` c++

LOG_FIRST_N(INFO, 20) << "获得第" << google::COUNTER << "个cookie";

// 结果 只log前20次

  GNU nano 7.2                    main.rzzrz.rzz-rz.log.INFO.20250424-123212.28832                              Log file created at: 2025/04/24 12:32:12
Running on machine: rzzrz
Running duration (h:mm:ss): 0:00:00
Log line format: [IWEF]yyyymmdd hh:mm:ss.uuuuuu threadid file:line] msg
I20250424 12:32:12.623370 140164973207104 main.cpp:6] 获得第1个cookie
I20250424 12:32:12.623833 140164973207104 main.cpp:6] 获得第2个cookie
I20250424 12:32:12.623840 140164973207104 main.cpp:6] 获得第3个cookie
I20250424 12:32:12.623843 140164973207104 main.cpp:6] 获得第4个cookie
I20250424 12:32:12.623846 140164973207104 main.cpp:6] 获得第5个cookie
I20250424 12:32:12.623848 140164973207104 main.cpp:6] 获得第6个cookie
I20250424 12:32:12.623850 140164973207104 main.cpp:6] 获得第7个cookie
I20250424 12:32:12.623853 140164973207104 main.cpp:6] 获得第8个cookie
I20250424 12:32:12.623855 140164973207104 main.cpp:6] 获得第9个cookie
I20250424 12:32:12.623857 140164973207104 main.cpp:6] 获得第10个cookie
I20250424 12:32:12.623860 140164973207104 main.cpp:6] 获得第11个cookie
I20250424 12:32:12.623862 140164973207104 main.cpp:6] 获得第12个cookie
I20250424 12:32:12.623864 140164973207104 main.cpp:6] 获得第13个cookie
I20250424 12:32:12.623867 140164973207104 main.cpp:6] 获得第14个cookie
I20250424 12:32:12.623869 140164973207104 main.cpp:6] 获得第15个cookie
I20250424 12:32:12.623871 140164973207104 main.cpp:6] 获得第16个cookie
I20250424 12:32:12.623874 140164973207104 main.cpp:6] 获得第17个cookie
I20250424 12:32:12.623876 140164973207104 main.cpp:6] 获得第18个cookie
I20250424 12:32:12.623878 140164973207104 main.cpp:6] 获得第19个cookie
I20250424 12:32:12.623881 140164973207104 main.cpp:6] 获得第20个cookie
```

### LOG_EVERY_T
### LOG_EVERY_T

```c++
// 与时间操作有关的log宏
LOG_EVERY_T(INFO, 0.01) << "获得一个cookie";  // 每10ms
LOG_EVERY_T(INFO, 2.35) << "获得一个cookie";  // 每2.35秒
```

```c++
// 
#define SOME_KIND_OF_LOG_EVERY_T(severity, seconds)                            \
								// 和LOG_EVERY_N中的LOG_OCCURRENCES一样也是使用
								// 使用__LINE__实现定义独特的变量名
// 记录持续时间
  constexpr std::chrono::nanoseconds LOG_TIME_PERIOD =                         \
      std::chrono::duration_cast<std::chrono::nanoseconds>(                    \
          std::chrono::duration<double>(seconds));                             \
  
  // 静态的原子变量,避免重复定义并保证线程安全
  static std::atomic<google::int64> LOG_PREVIOUS_TIME_RAW;                     \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                         \
      __FILE__, __LINE__, &LOG_TIME_PERIOD, sizeof(google::int64), ""));       \
  GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(                         \
      __FILE__, __LINE__, &LOG_PREVIOUS_TIME_RAW, sizeof(google::int64), "")); \
  const auto LOG_CURRENT_TIME =                                                \
      std::chrono::duration_cast<std::chrono::nanoseconds>(                    \
                                          // 获取时钟元年之后的数据
          std::chrono::steady_clock::now().time_since_epoch());                \
  const auto LOG_PREVIOUS_TIME =                                               \
      // 使用松散内存序加载LOG_PREVIOUS_TIME,但可能会导致其他缓存中的值未更新到内存中导致语序问题
      LOG_PREVIOUS_TIME_RAW.load(std::memory_order_relaxed);                   \
  const auto LOG_TIME_DELTA =                                                  \
      LOG_CURRENT_TIME - std::chrono::nanoseconds(LOG_PREVIOUS_TIME);          \
  if (LOG_TIME_DELTA > LOG_TIME_PERIOD)                                        \
    LOG_PREVIOUS_TIME_RAW.store(                                               \
        std::chrono::duration_cast<std::chrono::nanoseconds>(LOG_CURRENT_TIME) \
            .count(),                                                          \
        std::memory_order_relaxed);                                            \
  if (LOG_TIME_DELTA > LOG_TIME_PERIOD)                                        \
  google::LogMessage(__FILE__, __LINE__, google::GLOG_##severity).stream()
```

上面代码实现要注意的问题如下

- 使用constexpr提高性能,且确保是编译器常量

  限定使用者输入的时间间隔的变量应该是编译器常量,并且能在编译器进行编译,给编译器提供优化空间提高性能

- 使用nanoseconds(纳秒)类型提高精度

  首先让用户输入的可能的double类型转换成统一的duration类型,然后转换为纳秒的高精度类型最后进行比较,调高精度

- 对于memory_order_relaxed内存序的使用

  了性能提升,但是会导致多个线程之间语序的问题 

### 在调试复杂问题时，详细的日志消息非常有用。glog提供`VLOG`宏定义自定义数字日志级别。

```bash
// 在程序运行开始时可以使用下面的语法
./your_app --vmodule=mapreduce=2,file=1,gfs*=3 --v=0
// 这就是实现了 
// 从mapreduce.{h,cc}记录VLOG(2)及以下级别
// 从file.{h,cc}记录VLOG(1)及以下级别
// 从"gfs"前缀文件记录VLOG(3)及以下级别
// 其他位置记录VLOG(0)及以下级别

// 也就是可以做到分模块(文件)设置不同的log上限等级
```

### 数字严重等级LOG使用

```c++
VLOG(1) << "当--v=1或更高时打印此消息";
VLOG(2) << "当--v=2或更高时打印此消息";
```

```c++
// 源码实现
// 该处实现服用了 LOG_IF的实现,使用VLOG_IS_ON的宏来判断是否log
#define VLOG(verboselevel) LOG_IF(INFO, VLOG_IS_ON(verboselevel))

// --------------------------------

// glog的源码解释:首先使用GUN C的扩展来
#if defined(__GNUC__)
#efine VLOG_IS_ON(verboselevel)                                       \
// 这里使用了GNU的复合语句表达式
    __extension__({                                                      \
    // 定义一个本文件的静态变量，注意当前的静态变量被"{}"包裹随意,可见范围就仅是当前展开的语句
    // 这样实现的每一个调用位置都有自己独立的站点信息,可以尽量的实现细粒度
      static google::SiteFlag vlocal__ = {nullptr, nullptr, 0, nullptr}; \
      GLOG_IFDEF_THREAD_SANITIZER(AnnotateBenignRaceSized(               \
          __FILE__, __LINE__, &vlocal__, sizeof(google::SiteFlag), "")); \
      google::int32 verbose_level__ = (verboselevel);                    \
      (vlocal__.level == nullptr
            // 该位置的函数是将刚刚定义的站点进行绑定
      	    // 就是查询当前文件用户要求的数字严重等级的上限等
           ? google::InitVLOG3__(&vlocal__, &FLAGS_v, __FILE__,          \
                                 verbose_level__)                        \
           : *vlocal__.level >= verbose_level__);                        \
    })
#else
   //不支持GUN C扩展标准的编译器就不会支持模块化的log
// GNU extensions not available, so we do not support --vmodule.
// Dynamic value of FLAGS_v always controls the logging level.
#  define VLOG_IS_ON(verboselevel) (FLAGS_v >= (verboselevel))
#endif

//--------------------------------

// 查询某个文件用户所要求的数字严重等级上限
bool InitVLOG3__(SiteFlag* site_flag, int32* level_default, const char* fname,
                 int32 verbose_level) {
  std::lock_guard<std::mutex> l(vmodule_mutex);
  bool read_vmodule_flag = inited_vmodule;
  if (!read_vmodule_flag) {// 查看是否初始化
    VLOG2Initializer();// 初始化VLOG系统
  }

  // protect the errno global in case someone writes:
  // VLOG(..) << "The last error was " << strerror(errno)
  int old_errno = errno;

  // site_default normally points to FLAGS_v
  int32* site_flag_value = level_default;

  // Get basename for file
  const char* base = strrchr(fname, '/');

#ifdef _WIN32
  if (!base) {
    base = strrchr(fname, '\\');
  }
#endif

  base = base ? (base + 1) : fname;
  const char* base_end = strchr(base, '.');
  size_t base_length =
      base_end ? static_cast<size_t>(base_end - base) : strlen(base);

  // Trim out trailing "-inl" if any
  if (base_length >= 4 && (memcmp(base + base_length - 4, "-inl", 4) == 0)) {
    base_length -= 4;
  }

  // TODO: Trim out _unittest suffix?  Perhaps it is better to have
  // the extra control and just leave it there.

  // find target in vector of modules, replace site_flag_value with
  // a module-specific verbose level, if any.
  for (const VModuleInfo* info = vmodule_list; info != nullptr;
       info = info->next) {
    if (SafeFNMatch_(info->module_pattern.c_str(), info->module_pattern.size(),
                     base, base_length)) {
      site_flag_value = &info->vlog_level;// 站点的数字严重级别指针指向用户定义的严重级别
      // value at info->vlog_level is now what controls
      // the VLOG at the caller site forever
      break;
    }
  }

  // Cache the vlog value pointer if --vmodule flag has been parsed.
  ANNOTATE_BENIGN_RACE(site_flag,
                       "*site_flag may be written by several threads,"
                       " but the value will be the same");
  if (read_vmodule_flag) {
    site_flag->level = site_flag_value;
    // If VLOG flag has been cached to the default site pointer,
    // we want to add to the cached list in order to invalidate in case
    // SetVModule is called afterwards with new modules.
    // The performance penalty here is neglible, because InitVLOG3__ is called
    // once per site.
    if (site_flag_value == level_default && !site_flag->base_name) {
      site_flag->base_name = base;
      site_flag->base_len = base_length;
      site_flag->next = cached_site_list;
      cached_site_list = site_flag;
    }
  }

  // restore the errno in case something recoverable went wrong during
  // the initialization of the VLOG mechanism (see above note "protect the..")
  errno = old_errno;
  return *site_flag_value >= verbose_level;
}
```
源码关键点解释
- 复合语句表达式

  这是一种属于GNU C的扩展语法,允许将一个或者多个语句合成为一个表达式,最后表达式的值会是最后一个语句的值

  在上面的实现就会是三目表达式的值
  
  解释一下为什么要用复合语句表达式来实现，而不是原版标准c++标准的那种宏展开呢？
  
  首先我们需要一个bool值告诉当前宏到底要不要进行log，且想要实现分文件的日志等级的限制，根据上述的限制推像到，宏展开的最后应当返回bool值，并且能够定义一个仅当前文件可见的静态变量(glog实现的是当前调用出可见的,但最后结果还是分文件可见)来记录指导当前文件下的日志记录行文。
  
### 还有在`DEBUG`模式下才会生效的```log```，避免```release```下的程序由于过多的日志记录降低性能

```c++
DLOG(INFO) << "找到cookies";
DLOG_IF(INFO, num_cookies > 10) << "获得大量cookies";
DLOG_EVERY_N(INFO, 10) << "获得第" << google::COUNTER << "个cookie";
DLOG_FIRST_N(INFO, 10) << "获得第" << google::COUNTER << "个cookie";
DLOG_EVERY_T(INFO, 0.01) << "获得一个cookie";

```

### 运行时检查

`CHECK`宏提供了在条件不满足时中止程序的能力，类似于标准C库中的`assert`宏。

与`assert`不同，`CHECK`不受`NDEBUG`控制，无论编译模式如何都会执行检查。因此以下示例中的`fp->Write(x)`总是会执行：

```c++
CHECK(fp->Write(x) == 4) << "写入失败!";

// 实现
#define CHECK(condition)                                       \
			// GOOGLE_PREDICT_BRANCH_NOT_TAKEN分支预测的优化宏
  LOG_IF(FATAL, GOOGLE_PREDICT_BRANCH_NOT_TAKEN(!(condition))) \
      << "Check failed: " #condition " "
```

提供多种辅助宏用于相等/不等检查：

- `CHECK_EQ`、`CHECK_NE`、`CHECK_LE`、`CHECK_LT`、`CHECK_GE`和`CHECK_GT`。它们比较两个值，当结果不符合预期时记录包含这两个值的`FATAL`消息。这些值必须支持`operator<<(ostream, ...)`。

  ```c++
  // 可以看到每个比较的检查宏都是一个统一的CHECK_OP宏实现的
  #define CHECK_EQ(val1, val2) CHECK_OP(_EQ, ==, val1, val2)
  #define CHECK_NE(val1, val2) CHECK_OP(_NE, !=, val1, val2)
  #define CHECK_LE(val1, val2) CHECK_OP(_LE, <=, val1, val2)
  #define CHECK_LT(val1, val2) CHECK_OP(_LT, <, val1, val2)
  #define CHECK_GE(val1, val2) CHECK_OP(_GE, >=, val1, val2)
  #define CHECK_GT(val1, val2) CHECK_OP(_GT, >, val1, val2)
  
  // CHECK_OP的实现
  #if defined(STATIC_ANALYSIS)
  // Only for static analysis tool to know that it is equivalent to assert
  #  define CHECK_OP_LOG(name, op, val1, val2, log) CHECK((val1)op(val2))
  #elif DCHECK_IS_ON()
  using _Check_string = std::string;
  #  define CHECK_OP_LOG(name, op, val1, val2, log)                              \
  		// std::unquie_ptr来管理会在堆上申请内存
      while (std::unique_ptr<google::logging::internal::_Check_string> _result = \
      			// 根据传入的name(标记是哪种比较)来调用对应的实现函数
                 google::logging::internal::Check##name##Impl(                   \
                     google::logging::internal::GetReferenceableValue(val1),     \
                     google::logging::internal::GetReferenceableValue(val2),     \
                     #val1 " " #op " " #val2))                                   \
      log(__FILE__, __LINE__,                                                    \
          google::logging::internal::CheckOpString(std::move(_result)))          \
          .stream()
  #else
  // In optimized mode, use CheckOpString to hint to compiler that
  // the while condition is unlikely.
  #  define CHECK_OP_LOG(name, op, val1, val2, log)                          \
  		// 直接在栈上构造提高release模式下提高性能
      while (google::logging::internal::CheckOpString _result =              \
                 google::logging::internal::Check##name##Impl(               \
                     google::logging::internal::GetReferenceableValue(val1), \
                     google::logging::internal::GetReferenceableValue(val2), \
                     #val1 " " #op " " #val2))                               \
      log(__FILE__, __LINE__, _result).stream()
  #endif  // STATIC_ANALYSIS, DCHECK_IS_ON()
  ```

  
