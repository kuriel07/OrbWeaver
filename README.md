this is the sourcecode for OrbWeaver scripting language

## system requirements
* CMake
* gcc (Linux) or MSVC (Windows)
* glib library (Mandatory, Linux)
* SQLite3 (Mandatory, Linux) for session web server API
* Jansson (Mandatory, Linux) for JSON API
* OpenSSL (Mandatory, Linux) for https (webserver)
* PostgreSQL (Optional, Linux) for DAL (database abstraction layer example)

## build
    cmake --config .
    cmake --build .

## generated build (/bin)
    glome - compiler
    torus - a script interpreter can be configured as web-server

## install modules and includes inside /bin
download modules and includes
https://github.com/kuriel07/pandora/blob/master/modules.zip

## download sample server.txt file, includes and modules must exist first
https://github.com/kuriel07/pandora/blob/master/server_config.zip
check server.txt for detail configuration, server port and certificates


## usage :
    glome -ca [filename]     --> compiler (use -ca to compile-assembler bytecodes)
    torus [filename]     --> start executing script file
    torus -c server.txt  --> running as webserver (need www directory)

![OrbWeaver](https://raw.githubusercontent.com/kuriel07/pandora/master/orbweaver_final.png "OrbWeaver logo") 




