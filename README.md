# simfind - A find program with simple functionalities

compile: make    

usage: `./main [-dir <dir>] [-name <reg>] [-type f|d]`

    <dir>   directory to search for. if not provided, use current work directory instead. 
    <reg>   name patterns. be sure to escape special characters in shell command line. 
    f|d     f - search for file, d - search for directory. 

---

编译：make

用法：`./main [-dir <dir>] [-name <reg>] [-type f|d]`

    <dir>   搜索的目录，如果没有提供该选项，默认使用当前工作目录 
    <reg>   文件名匹配模式. 使用时请保证特殊字符在shell中进行转义. 
    f|d     f - 搜索文件, d - 搜索目录. 
