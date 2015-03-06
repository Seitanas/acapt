acapt
=====
program used to capture files from http media streams with exact stream length of 1 hour. 
Files are created on hour basis with format filename_yyyy-mm-dd_hh.00.00

compile options -pthread -lcurl

gcc -oacapt acapt.c -lpthread -lcurl

Needs libcurl-dev package


