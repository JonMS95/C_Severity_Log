# C_Severity_Log: a simple logging system 📝
A powerful and easy-to-use logging system designed to enhance the monitoring and debugging of your applications.


## Table of contents 🗂️
* [**Introduction** 📑](#introduction)
* [**Features** 🌟](#features)
* [**Prerequisites** 🧱](#prerequisites)
* [**Installation instructions** 📓](#installation-instructions)
  * [**Download and compile** ⚙️](#download-and-compile)
  * [**Compile and run test** 🧪](#compile-and-run-test)
* [**Usage** 🖱️](#usage)
* [**To do** ☑️](#to-do)
* [**Related documents** 🗄️](#related-documents)


## Introduction <a id="introduction"></a> 📑
When it comes to debugging, there are a couple of tools every developer should be accustomed to using: debuggers (such as [**GDB**](https://www.sourceware.org/gdb/)) and logging systems.
More than a debugging tool, the latter is a way to monitor the activity of a running application, providing developer-specified information.
The library's name refers to the fact that logs are displayed differently depending on their severity level.


## Features <a id="features"></a> 🌟
The resulting library comes with some remarkable features:
* Displaying different types of information (errors, warnings, generic information and debugging information)
* Being able to hide logs depending on their type (for instance, preventing CLI from showing debugging information)
* Adding a date to each line outputted by the log system

In order to get some knowledge about how to use the library alongside its options, go to [Usage](#usage).


## Prerequisites <a id="prerequisites"></a> 🧱
By now, the application has only been tested in POSIX-compliant Linux distros. In these, many of the dependencies below may already come installed in the OS.
In the following list, the minimum versions required (if any) by the library are listed.

| Dependency                   | Purpose                                 | Minimum version |
| :--------------------------- | :-------------------------------------- |:-------------: |
| [gcc][gcc-link]              | Compile                                 |11.4            |
| [Bash][bash-link]            | Execute Bash/Shell scripts              |4.4             |
| [Make][make-link]            | Execute make file                       |4.1             |
| [Git][git-link]              | Download GitHub dependencies            |2.34.1          |
| [Xmlstarlet][xmlstarlet-link]| Parse [configuration file](config.xml)  |1.6.1           |

[gcc-link]:        https://gcc.gnu.org/
[bash-link]:       https://www.gnu.org/software/bash/
[make-link]:       https://www.gnu.org/software/make/
[git-link]:        https://git-scm.com/
[xmlstarlet-link]: https://xmlstar.sourceforge.net/

Except for Make and Bash, the latest version of each of the remaining dependencies will be installed automatically if they have not been found beforehand.

In any case, installing **_Xmlstarlet_** before executing any of the commands below is strongly recommended. Otherwise, it can lead to error since make file
contains some calls to it at the top. If that happens, just repeat the process (Xmlstarlet would have been already installed).

## Installation instructions <a id="installation-instructions"></a> 📓
### Download and compile <a id="download-and-compile"></a> ⚙️
1. In order to download the repo, just clone it from GitHub to your choice path by using the [link](https://github.com/JonMS95/C_Severity_Log) to the project.

```bash
cd /path/to/repos
git clone https://github.com/JonMS95/C_Severity_Log
```

2. Then navigate to the directory in which the repo has been downloaded, and set execution permissions to every file just in case they have not been sent beforehand.

```bash
cd /path/to/repos/C_Severity_Log

find . -type f -exec chmod u+x {} +
```

3. For the library to be built (i.e., clean, download dependencies and compile), just type the following:

```bash
make
```

The result of the line above will be a new API directory (which will match the used version). Within it, a *.h* and a *.so* file will be found.
- **/path/to/repos/C_Severity_Log/API**
  - **vM_m**
    - **Dynamic_libraries**
      - **_libSeverityLog.so.M.m_**
    - **Header_files**
      - **_SeverityLog_api.h_**

Where **_M_** and **_m_** stand for the major and minor version numbers.
**_SeverityLog_api.h_** could also be found in **_/path/to/repos/C_Severity_Log/Source_files/SeverityLog_api.h_** although it may differ depending on the version.


### Compile and run test <a id="compile-and-run-test"></a> 🧪
For the test executable file to be compiled and executed, use:

```bash
make test
```

Again, the one below is the path to the generated executable file:
- **/path/to/repos/C_Severity_Log/Tests**
  - **Executable_files**
      - **_main_**
  - Source_files
  - Dependency_files


## Usage <a id="usage"></a> 🖱️
The following is the main logging function prototype as found in the **_header API file_** (_/path/to/repos/C_Severity_Log/API/vM_m/Header_files/SeverityLog_api.h_) or in the [repo file](https://github.com/JonMS95/C_Severity_Log/blob/main/Source_files/Severity_Log_api.h).

```c
C_SEVERITY_LOG_API int SeverityLog(int severity, const char* format, ...);
```

Where:
* **severity**: is the severity level (1 for errors, 2 for information, 3 for warnings and 4 for debugging information)
* **format**: is the format string for the output message
* **(variadic arguments)**: is a variable amount of arguments that are used alongside the format specifier

In exchange, the function returns:
* **0** if everything went OK, or **< 0** if any error happened

In order to simplify the usage of the **_SeverityLog_** function, some macros have been defined:

```c
#define SVRTY_LOG_ERR(...) SeverityLog(SVRTY_LVL_ERR, __VA_ARGS__)
#define SVRTY_LOG_INF(...) SeverityLog(SVRTY_LVL_INF, __VA_ARGS__)
#define SVRTY_LOG_WNG(...) SeverityLog(SVRTY_LVL_WNG, __VA_ARGS__)
#define SVRTY_LOG_DBG(...) SeverityLog(SVRTY_LVL_DBG, __VA_ARGS__)
```

On top of simply logging, the information to be displayed can be restricted depending on its nature:

```c
C_SEVERITY_LOG_API void SetSeverityLogMask(int mask);
```

Some of the mask values have already been defined in the API header file and can be the following:

```c
#define SVRTY_LOG_MASK_OFF  0b0000
#define SVRTY_LOG_MASK_ERR  0b0001
#define SVRTY_LOG_MASK_INF  0b0010
#define SVRTY_LOG_MASK_WNG  0b0100
#define SVRTY_LOG_MASK_DBG  0b1000
#define SVRTY_LOG_MASK_EIW  0b0111 // EIW stands for ERR, INF, WNG
#define SVRTY_LOG_MASK_ALL  0b1111
```

As for the date, **_SetSeverityLogPrintTimeStatus_** should be used:

```c
C_SEVERITY_LOG_API void SetSeverityLogPrintTimeStatus(bool time_status);
```

Where **time_status** is a boolean variable that tells whether or not the date is meant to be shown preceding every log message.

When it comes to file name logging, the name of the executable file calling log functions can be displayed as well by simpy using **SetSeverityLogPrintExeNameStatus**:

```c
C_SEVERITY_LOG_API void SetSeverityLogPrintExeNameStatus(bool exe_name_status);
```

Many parameters (logging to syslog, among others) can be set by using a single function too:

```c
C_SEVERITY_LOG_API int SeverityLogInit( const unsigned long buffer_size ,
                                        const int  severity_level_mask  ,
                                        const bool print_time           ,
                                        const bool print_exe_file       ,
                                        const bool print_TID            ,
                                        const bool log_to_syslog        );
```

Same can be done in a shorter function call (less input parameters) with SeverityLogInitWithMask function:

```c
C_SEVERITY_LOG_API int SeverityLogInitWithMask(const size_t buffer_size, const uint8_t init_mask);
```

For reference, a proper API usage example has been provided on the [test source file](https://github.com/JonMS95/C_Severity_Log/blob/main/Tests/Source_files/main.c).
An example of CLI usage is provided in the [**Shell_files/test.sh**](https://github.com/JonMS95/C_Severity_Log/blob/main/Shell_files/test.sh) file.


## To do <a id="to-do"></a> ☑️
- [ ] Fix known bug: format string may include unused formaters without causing any issue, while they should


## Related Documents <a id="related-documents"></a> 🗄️
* [LICENSE](LICENSE)
* [CONTRIBUTING.md](Docs/CONTRIBUTING.md)
* [CHANGELOG.md](Docs/CHANGELOG.md)

