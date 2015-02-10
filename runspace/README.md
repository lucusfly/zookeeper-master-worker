Require:boost zookeeper

启动顺序：先开启work和takslist再启动master

一个可执行程序只能启动一次，想关闭该执行程序启动的进程可以直接命令-stop

clearDir：递归删除一个zookeeper目录,使用方式例："./clearDir /assign"
