this is the sourcecode for OrbWeaver scripting language (weak, dynamic type, object oriented, imperative), OrbWeaver support native call to external library based on x64 calling convention for Windows x64 and System V EABI for Linux, just add the library to /modules path of the OrbWeaver binary and declare the interface on /includes directory, see example modules for detail (modules.zip)

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

![OrbWeaver](https://raw.githubusercontent.com/kuriel07/pandora/master/orbweaver_final.png "OrbWeaver logo") 




