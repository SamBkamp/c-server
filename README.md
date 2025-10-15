# C_server

this is a web server written in C to serve an arduino project im working on. The purpose of this server is for my arduino to get up-to-date data (such as current epoch, stock quotes, and user defined notes) while offloading the processing away from the low-power arduino esp32 nano and on to my computer or a server or whatever.

This server is single threaded (for now), and has a listening socket as well as a request socket. The listening socket is for the arduino to connect to, and the request (outbound) socket is for my computer to request data from other sources.

The api secret for 12data (the api I use to get stock quotes) is in api_secret.h. It looks like this:

```c
#ifndef API_SECRET
#define API_SECRET "SECRET_KEY_HERE"
#endif
```

Simple as. 