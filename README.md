### OrbWeaver running Caffe Machine Learning Framework

an example application of OrbWeaver scripting language running caffe machine learning framework, caffe APIs are accessed through runtime native wrapper which translate function call between OrbWeaver scripting application function call and operating system function call (windows prochedure call on windows OS or system V ABI on POSIX)

[![Watch the video](https://img.youtube.com/vi/hnfQDXEFkuo/hqdefault.jpg)](https://www.youtube.com/embed/hnfQDXEFkuo)

### OrbWeaver running OpenCV library, face landmark algorithm running at realtime performance

an example application of OrbWeaver scripting language running OpenCV library, OpenCV APIs are accessed through runtime native wrapper which translate function call between OrbWeaver scripting application function call and operating system function call (windows prochedure call on windows OS or system V ABI on POSIX)

[![Watch the video](https://img.youtube.com/vi/6t9gJXFUahs/hqdefault.jpg)](https://www.youtube.com/embed/6t9gJXFUahs)

# README

this is the sourcecode for OrbWeaver scripting language (weak, static type, object oriented, imperative), OrbWeaver support native call to external library based on x64 calling convention for Windows x64 and System V EABI for Linux, just add the library to /modules path of the OrbWeaver binary and declare the interface on /includes directory, see example modules for detail (modules.zip)

    interface gtk lib "libgtk_lib.so" {
        int MessageBox(string text, string title, int type);
        var Button(string text);
        var Label(string text);
        var Window(string text);
        var Run();
        var BuilderFromFile(string path);
        var BuilderFromString(string content);
        var Application(string name, string callback);
        void setup();
    }

OrbWeaver act as intermediate interface between user script and native library, enabling user to access native library through scripting language, declared interface are loaded by interpreter automatically and can be accessed like accessing static class
every objects in OrbWeaver are treated as byte array, the content determined by the API which created the object (can be string or pointer), type punning or dependency injection is possible using this mechanism
default API supports array, object, threading, stream, serializer/deserializer
syntax supports lazy expression, lambda expression, external API access, internal API access

## system requirements
* CMake
* gcc (Linux) or MSVC (Windows)
* glib library (Mandatory, Linux)
* SQLite3 (Mandatory, Linux) for session web server API
* Jansson (Mandatory, Linux) for JSON API
* OpenSSL (Mandatory, Linux) for https (webserver)
* PostgreSQL (Optional, Linux) for DAL (Database Abstraction Layer example)

## build
    cmake --config .
    cmake --build .

## generated build (/bin)
    glome - compiler
    torus - a script interpreter can be configured as web-server

## install modules and includes inside /bin
download modules and includes
https://github.com/kuriel07/pandora/blob/master/modules.zip

## download sample server.txt file
https://github.com/kuriel07/pandora/blob/master/server_config.zip, includes and modules must exist first, check server.txt for detail configuration, server port and certificates


## usage :
    glome -ca [filename]     --> compiler (use -ca to compile-assembler bytecodes)
    torus [filename]     --> start executing script file
    torus -c server.txt  --> running as webserver (need www directory)


## additional references
might need to install these packages
    sudo apt install libssl-dev
    sudo apt install libglib2.0-dev
    sudo apt install libjansson-dev
    sudo apt install libpq-dev
    sudo apt install libsqlite3-dev


if cmake cannot find sqlite3, might need to install cmake version >3.2
since version cmake 3.2 "cmake --config ." replaced by "cmake -S ."

## install sqlite3 from sourcecode if necessary
    git clone https://github.com/sqlite/sqlite.git 
    cd sqlite
    cmake --config .
    cmake --build .
    cmake install

## =====  WSL ====
case of WSL environment
cmake need modules FindSQLite3.cmake /usr/share/cmake , need newer version > 3.2
    sudo apt-get update
    sudo apt-get install apt-transport-https ca-certificates gnupg software-properties-common wget
    After that is finished
    
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
Next add the updated repository by typing in the following

    sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
    sudo apt-get update

    sudo apt install cmake


## install cmake from sourcecode (build)
    wget http://www.sqlite.org/sqlite-autoconf-3070603.tar.gz
    tar xvfz sqlite-autoconf-3070603.tar.gz
    cd sqlite-autoconf-3070603
    ./configure
    make
    make install

![OrbWeaver](https://raw.githubusercontent.com/kuriel07/pandora/master/orbweaver_final.png "OrbWeaver logo") 




