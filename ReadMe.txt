kimgbo是一个基于Reactor模式的网络I/O库，优先考虑易用性，由muduo网络库改写而来。保留了muduo基于对象
的编程风格，支持 oneloop per thread + threadpool 模型。并去除了muduo对于Boost库的依赖，转而采用C++11
和tr1标准替代，优化了muduo网络I/O库一处存在racecondition隐患的代码，几乎具备了muduo网络库全部的功能。
kimgbo网络I/O库保留了与muduo类似的文件夹归类方式，kimgbo/base目录下存放了整个网络库的基础代码，
kimgbo/net目录下存放了网络库的核心代码，kimgbo/example目录下存放了一些网络库基础的使用示列程序。
后期准备继续对其进行优化，主要的优化点有三处：1、将继续完善网络库的细节实现，提升安全性。2、由于muduo
的历史遗留问题，其线程安全性还有待提高。3、将有可能改进网络库Buffer的实现，提升内存使用的效率。