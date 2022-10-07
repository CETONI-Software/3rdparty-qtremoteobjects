# Qt Remote Objects Fork

This is a fork of the Qt Remote Objects library.

https://doc.qt.io/qt-5/qtremoteobjects-index.html

The intention of this fork is to modify the library in a way, that it is possible to enable remoting for existing QObject based classes. 

Currently only
the properties a derived class are exposed but not the properties of the base class. That means, in the following hierarchy, if remoting is enabled for the
Derived class, only the properties of this class are exposed but not the
properties of the base class:

                      +---------------------+
                      |       QObject       |
                      +---------------------+
                                 ^
                                 |
                      +---------------------+
                      |        Base         |
                      +---------------------+
                                 ^
                                 |
                      +---------------------+
                      |       Derived       |
                      +---------------------+

I found an explanation in the Qt mailing list:

https://www.mail-archive.com/interest@qt-project.org/msg34040.html

In the fork the `DynamicApiClass` is modified to expose all properties except
the properties of the base QObject class.

## Requirements

Qt 5.15.2

## Building

- Clone the repository
- switch to the `5.15.2_cetoni` branch
- create a `build` directory in the repository
- cd into the build directory
- call `qmake -recursive ..\src\remoteobjects`
- execute `make release`

The modified library builds a `QtRemoteObjects_CETONI.dll`. Copy this DLL to your libs folder on then copy the `remoteobjects` folder to your `include`
directory.
