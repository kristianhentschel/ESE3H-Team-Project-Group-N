This is the source code distribution for Team N's Team Project 3 submission.

The Linux Software can be compiled by invoking make from the ./src/ directory. The embedded software can not directly be compiled with this distribution as libraries installed with the development toolkit (Keil MDK, http://www.keil.com/arm/mdk.asp) are not available. A separate Keil-Project folder is supplied that includes these, however it will likely require environment setup. The ./src/ directory however includes all code written by the Team, including the main.c for the embedded software, for inspection.

A quick overview of the modules can be gained from the comments in the .h header files for the individual libraries. Notes on originality (or sources of third party code where applicable) can be found in the header file comments as well.

On the raspberry pi, the software can be invoked using the ./run.sh script. It
should be noted that the webserver must be invoked from the ./htdocs
directory, which run.sh does. This is so that the user interface source files
can be found by the webserver. Once the server is running, the application can
be accessed on the local network by visiting http://hostname:8080/ in a web
browser.
