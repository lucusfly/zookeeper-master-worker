# Introduction
Zookeeper master-worker implementation using c++ synchronized zookeeper client interfaces

# Zookeeper C++ client
The c++ client implementation bases zookeeper c-binding client and boost. comparing with c-binding client, some convenience is added:
(1) all client requests send and get synchronously. 
(2) if return code is retryable, it will auto retry.
(3) one class instance handle all interactions with zookeeper server.
 
But the lose benifit is asynchronization, which will cause application slow if network is not good.

# Master Worker framwork
It is the similiar with the common master-worker job handle framwork, which has been descripted in the book "ZooKeeper" writed by Flavio Junqueira & Benjamin Reed. 

The true master start after leader selection from all master process. It takes watchs on tasks and workers and assigns tasks on worker balanced.

Woker is simple, as a process to handle tasks assigned to it. when necessary, worker should update task state.
