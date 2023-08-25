# ZMQ Test Tool

## Description

This tool sends and receives messages over ZeroMQ sockets for testing purposes.

## Overview
========

Getting Started
---------------

### Download nzmqt

First of all, decide which version of nzmqt you will need. Your choice will depend on the version of 0MQ you are going to use. In order to make this choice easier a new versioning scheme for nzmqt has been introduced. nzmqt version numbers indicate compatibility with 0MQ version <0MQ-major>.<0MQ-minor>.<0MQ-bugfix> by adopting the major and minor part. For instance, nzmqt releases compatible with all 0MQ versions 2.2.x will have a corresponding version number 2.2.y, and an nzmqt version number 3.2.y indicates compatibility with all 0MQ versions 3.2.x. Note that there is no relation between nzmqt's and 0MQ's bugfix versions as bugfixes do not introduce incompatible API changes. Also note that release 0.7 is the last release NOT conforming to this scheme.

Once you know which version you need you can either clone the complete nzmqt code repository or you can directly download the sources of a release as a ZIP file from GitHub.

### Resolve dependencies

As nzmqt is a Qt binding for 0MQ there is obviously a dependency to these two libraries. Since 0MQ version 3 the C++ binding for 0MQ is not part of the core anymore. It has been externalized to a [separate Git repository][cppzmq].

So here is the complete list of dependencies:

* [Qt 4.8.x][] (or newer): You will need to download and install it yourself.
* [0MQ 3.2.x][zeromq 3.2.x]: You will need to download and install it yourself.
* [C++ binding for 0MQ][cppzmq]: A script delivered with nzmqt will download the source code and appropriate version itself.

### Setup nzmqt project

As already mentioned in the previous section nzmqt comes with a short script that will make sure a compatible version of 0MQ's C++ binding is available by downloading the correct version. In fact, the corresponding GitHub project repository is referenced as a Git submodule. So the script actually initializes the submodule. Furthermore, it will copy the C++ binding's include file to ``<path-to-nzmqt>/externals/include`` directory in the project root.

***Execute setup script***

In a console type:

    cd <path-to-nzmqt>
    ./setup-project.sh

***Test your installation***

In order to see if everything works well you can compile and run unit tests provided with nzmqt. Just compile ``zmqtesttool.pro`` project located under project root directory. If the build is successful you will find an ``zmqtesttool`` executable under ``build-xxx`` directory (Path is defined by your compiler setting in Qt Creator). Run this binary in a console and look if ui emerges normally. For future reference and bug tracking, please [file a bug][nzmqt issue tracker] using the nzmqt issue tracker on GitHub.

### Setup your own project to use nzmqt

There are different options for integrating nzmqt in your project. The following descriptions assume you use Qt's [QMake][] tool.

***Include File Only***

Like 0MQ's C++ binding nzmqt can be added to your project by including a single C++ header file which you need to include in your project. Consequently, using nzmqt in a Qt project is as simple as adding that single header file to your project's ``.pro`` file as follows.

    HEADERS += nzmqt/nzmqt.hpp

Adjust your include path:

    INCLUDEPATH += \
        <path-to-nzmqt>/include \
        <path-to-nzmqt>/externals/include

And if not already done, you also need to link against 0MQ library:

    LIBS += -lzmq

To specify the library path, we can use the following line (I assume that the `.dll` file is placed in the `lib` folder under the root path of the project):

    LIBS += -L$$PWD/lib -llibzmq

***Include and Source File***

If you don't like the everything is inlined as it is the case with *Include File Only* option you can use this approach. It will "move" the class method implementations to a separate ``nzmqt.cpp`` file. For this to work modify your ``.pro`` file as follows.

    # This define will "move" nzmqt class method
    # implementations to nzmqt.cpp file.
    DEFINES += NZMQT_LIB
    
    HEADERS += nzmqt/nzmqt.hpp
    SOURCES += nzmqt/nzmqt.cpp
    
    INCLUDEPATH += \
        <path-to-nzmqt>/include \
        <path-to-nzmqt>/externals/include
    
    LIBS += -lzmq
    
***Static Library***

You can also create an nzmqt static library. There is a project file ``nzmqt_staticlib.pro`` included in ``<path-to-nzmqt>/src`` directory which is preconfigured to do this.

***Shared Library***

Finally, there is the option to create an nzmqt shared library. A corresponding preconfigured project file ``nzmqt_sharedlib.pro`` can also be found in ``<path-to-nzmqt>/src``directory.

Documentation
-------------

* [API reference][]
* [changelog][]
* [software license][]
* [samples][]


 [cppzmq]:              https://github.com/zeromq/cppzmq                                        "C++ binding for 0MQ on GitHub"
 [Qt 5.15.x]:            http://download.qt-project.org/official_releases/qt/5.15/                "Qt 5.15.x download page"
 [zeromq 4.3.x]:        http://www.zeromq.org/intro:get-the-software                            "0MQ download page"
 [QMake]:               http://doc-snapshot.qt-project.org/qt5-stable/qmake/qmake-manual.html   "Latest QMake manual"
 [nzmqt issue tracker]: https://github.com/jonnydee/nzmqt/issues                                "nzmqt issue tracker on GitHub"
 [changelog]:           CHANGELOG.md                                                         "zmqtesttool software changelog"
 [software license]:    LICENSE.md                                                           "zmqtesttool software license"
 [User_Manual]:             User_Manual.md                                                              "zmqtesttool user manual"

## Author

lvruitao

## Project Hierarchy

- **main**
  - `main.cpp` - main entrance for program
- **mainwindow**
  - `mainwindow.cpp` - main window interface and related implementation
  - `mainwindow.h` - header file for the main window
- **aboutdialog**
  - `aboutdialog.cpp` - network about window interface and related implementation
  - `aboutdialog.h` - header file for about
- **include**
  - **cppzmq**
    - `zmq_util.h` - zeromq utility header file
    - `zmq.h` - zeromq fundamental header file for c (All of its headers corresponding to the implementations inside `libzmq-v142-mt-4_3_5.dll` dynamic link library)
    - `zmq.hpp` - zeromq header file for c++
  - **nzmqt**
    - `global.hpp` - global header file for NZMQT_API
    - `impl.hpp` - implementation header file for NZMQT_API
    - `nzmqt.hpp` - nzmqt header file
  - `Publisher.hpp` - publisher header file and implementation
  - `Subscriber.hpp` - subscriber header file and implementation
  - `SampleBase.hpp` - sample base header file and implementation

## Library Dependencies

- Qt-based libraries
- [libzmq V4.3.4 x64](https://github.com/zeromq/libzmq)

## Millstones

- [x] 2023.08.14: New project using Qt5.15 C++ environment, add ZeroMQ library.
- [x] 2023.08.21: Basic subscribe and publish function test.
- [x] 2023.08.23: Preliminary version.

## How to Create an Executable Package For a Qt Application

1. Build your application in Release mode:
    In Qt Creator, you can switch to Release mode by selecting "Release" from the drop-down menu in the lower-left corner of the screen, and then build your application by selecting "Build" -> "Rebuild All".

2. Deploy your application:
    After building your application in release mode, you'll need to make sure that your application can find the Qt libraries and plugins it needs to run. Qt provides a tool called `windeployqt` on Windows, `macdeployqt` on macOS and `linuxdeployqt` on Linux that automates the process of deploying an application.

    Example code to build the executable package for the register-tool application:

    Note: Recommend using "x64 Native Tools Command Prompt for VS 2022" when pre-building with Visual Studio 2022

    ```shell
    windeployqt .\zmqtesttool.exe --qmldir D:\Qt\5.15.2\msvc2019_64\qml
    ```

3. Copy the necessary files to the executable package directory:
    After successfully running the above command, the executable package will be generated in the current directory, however the configuration files such as "flightsdk_authority.json" is not included in the executable package, so we need to manually copy these files to the executable package directory.

    Third part libraries such as "libcrypto-1_1-x64.dll", "libssh2-1_11-x64.dll" and "libssl-1_1-x64.dll" are also not included in the executable package, so we need to manually copy these files in the bin folder to the executable package directory.

## Install-Package Encapsulation

To encapsulate the register-tool project into an install package, we can create an installer that packages all the necessary files and deploys them to the target system (win32 platform).

Distribute this installer to users who can run it on their Windows machines to install this register-tool application.

Inno Setup is a free installer for Windows programs. Download and install Inno Setup from the official website: https://www.jrsoftware.org/isinfo.php

## License

MIT License
