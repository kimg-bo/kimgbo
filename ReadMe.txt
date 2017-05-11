    kimgbo由muduo网络库改写而来。去除了muduo对于Boost库的依赖，转而采用C++11替代，
优化了muduo网络I/O库一处将来有可能会出现race condition隐患的代码，几乎具备了muduo
网络库全部的功能。
    kimgbo/base目录下存放了整个网络库的基础代码，kimgbo/net目录下存放了网络库的核
心代码，kimgbo/example目录下存放了一些网络库基础的使用示列程序。
    除去除了对boost库的依赖外，还对其线程池和Buffer进行了优化。
    1）用无锁队列（moodeycamel::ConcurrentQueue）替换了原有的任务队列，性能提升168%，
       同时还提供了一种多队列运行模式。
    2）实现了一种新的环形缓冲区替换了muduo的线性缓冲区，新的环形缓冲区避免了muduo缓
       冲区数据的内部腾挪，兼容除内部腾挪外的所有特性，同时在部分场景下性能提升106%。
