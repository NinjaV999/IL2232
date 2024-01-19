#Python 既可以面向对象也可以面向过程

#一.基础语法

#1.标识符：_ 开头，其他部分由字母 数字和下划线组成
#        对大小写敏感
#2.保留字
'''
3.用缩进来表示代码块
4.\: 来连接多行语句
total=a+\
      b+\
      c
5.数据类型：int ,bool,float,complex（复数类型）
6.字符串string：1.引号完全相同
              2. 0 表示字符串第一个开始索引， -1 表示字符串最后一个开始索引， str[0:-1]表示从第一个到倒数第二个，因为py中左闭右开
              3.str[head:tail:stride] py没有单独的字符变量，每一个字符都是长度为1的字符串，可以认为每次输出都是将指针移动了stride个
              4.用+连接字符串，*表示复制该字符串
              5.\表示转义字符，如果不想转义则在开头加r
7.print输出时自动换行的，如果不想还行则需要在末尾加上end=""
8.import导入整个块，from module1 import f1, import function1 from module1 , imporyt * represent import all function in the module1

二.基本数据类型
1.Number,string,tuple（元组）, =》不可变
bool
list(列表）,set（集合）,dictionary（字典）=
》可变

2.可用type或者isinstance来判断数据类型
二者的区别在与type不会认为子类是一种父类类型，而isinstance 会

3.bool：可以被时为1或者 0，所有非0数字或者非空list tuple，string都被视为ture

4.List：存储在[] 用，隔开
        截取列表 list[head:tail],省略则表示从头截取或截取到最后
         列表可以嵌套的存储多种数据类型的元素
         py中字符串是不可以改变的，而列表是可以改变的，与string有些相似处
         inputWords = input.split(" ")
            #按照空格将字符串分割并构成列表
         inputWords=inputWords[-1::-1]
            #从列表最后一个开始倒着取，形成新的列表
         output = ' '.join(inputWords)
            #将列表每个元素之间填充上空格形成新的字符串

5.Tuple：与列表类似但是元组的中的元素不能更改 ，以存储在（）中，以，隔开，可以认为string is a special format tuple
        tuple=（）
        tuple=（1，）构造一个元素的元组比较特殊

6.set：一种无序、可变的数据类型，用于存储唯一的元素，元素不会重复且可以进行集合常见的集合操作
        集合使用一个{}表示，元素之间用，分割， 创建空集合必须要用set(),因为{}表示空字典
        a-b差集 可以理解为只存在a中的元素
        a|b并
        a&b交
        a^b ab不同同时存在的

7.dictionary：列表是有序的对象集合而字典是无序的对象集合，其是通过键来存取的而不是通过偏移来存取的，用{}表示，空的{}表示空字典而不是空集合
                键值必须与一个元素对应，键必须是不可变的元素，在同一个字典中，键(key)必须是唯一的，它是一个无序的 键(key) : 值(value) 的集合
                dict['one'] = "1 - 菜鸟教程"，“one”为键 "1 - 菜鸟教程"为元素

                tinydict = {'name': 'runoob','code':1, 'site': 'www.runoob.com'}， ：前为键，：后为值

                8.bytes：表示不可变的二进制序列，
                x = b"hello"，以b开头创建和创建二进制序列

四.py operator
1.算术运算符：//取整除运算符，用于除法取整往小的方向取
2.比较运算符
3.逻辑运算符
4.
'''
