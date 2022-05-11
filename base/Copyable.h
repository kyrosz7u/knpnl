//
// Created by 樱吹雪 on 2022/2/2.
//
#ifndef KHPNL_COPYABLE_H
#define KHPNL_COPYABLE_H

//类型库
#include <stdint.h>
/**
"《C++ Primer 第5版》15.7.2 合成拷贝控制与继承" 这一章节里有如下内容：
如果基类中的默认构造函数、拷贝构造函数、拷贝赋值运算符或析构函数是被删除的函数或者不可访问，
则派生类中对应的成员将是被删除的，原因是编译器不能使用基类成员来执行派生类对象基类部分的构造、赋值或销毁操作。
 即基类部分无法被构造
**/
namespace base{

class Copyable{
protected:
    //将构造函数申明为protected，可以防止直接构造这个类，但是能够构造其子类
    Copyable() {}
    ~Copyable() {}
};

class Noncopyable{
protected:
    Noncopyable() {}
    ~Noncopyable() {}
private:  // emphasize the following members are private
    //子类在构造的时候需要先构造父类，而这里通过private属性让子类无法调用父类的拷贝构造函数
    //所以子类也无法完成拷贝构造
    Noncopyable(const Noncopyable& );
    const Noncopyable& operator=(const Noncopyable& );
};

}




#endif
