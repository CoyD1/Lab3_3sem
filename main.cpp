#include <iostream>
#include "myvector.h"
#include "slist.h"
#include "dlist.h"

int main()
{
    std::cout << "myvector: " << std::endl;
    myvector<int> v1;
    for (int i = 0; i < 10; ++i)
    {
        v1.push_back(i);
    }
    v1.print();
    std::cout << v1[2] << std::endl;
    std::cout << v1.size() << std::endl;
    v1.erase(6);
    v1.erase(4);
    v1.erase(2);
    v1.print();
    v1.insert(10, 0);
    v1.print();
    v1.insert(20, v1.size() / 2);
    v1.print();
    v1.insert(30, v1.size());
    v1.print();

    myvector<int> v2 = std::move(v1);
    v1.print();
    v2.print();

    myvector<int> v3;
    v3 = v2;
    v2.print();
    v3.print();

    myvector<int> v4;
    v4 = std::move(v3);
    v3.print();
    v4.print();

    for (auto i = v4.begin(); i != v4.end(); ++i)
    {
        std::cout << *i << " ";
    }
    std::cout << '\n';

    std::cout << "\nslist: " << std::endl;
    slist<int> s1;

    for (int i = 0; i < 10; ++i)
    {
        s1.push_back(i);
    }
    s1.print();
    std::cout << s1[2] << std::endl;
    std::cout << s1.size() << std::endl;
    s1.erase(6);
    s1.erase(4);
    s1.erase(2);
    s1.print();
    s1.insert(10, 0);
    s1.print();
    s1.insert(20, s1.size() / 2);
    s1.print();
    s1.insert(30, s1.size());
    s1.print();

    slist<int> s2 = std::move(s1);
    s1.print();
    s2.print();

    slist<int> s3;
    s3 = s2;
    s2.print();
    s3.print();

    slist<int> s4;
    s4 = std::move(s3);
    s3.print();
    s4.print();

    for (auto i = s4.begin(); i != s4.end(); ++i)
    {
        std::cout << *i << " ";
    }
    std::cout << '\n';

    std::cout << "\ndlist: " << std::endl;
    dlist<int> d1;

    for (int i = 0; i < 10; ++i)
    {
        d1.push_back(i);
    }
    d1.print();
    std::cout << d1[2] << std::endl;
    std::cout << d1.size() << std::endl;
    d1.erase(6);
    d1.erase(4);
    d1.erase(2);
    d1.print();
    d1.insert(10, 0);
    d1.print();
    d1.insert(20, d1.size() / 2);
    d1.print();
    d1.insert(30, d1.size());
    d1.print();

    dlist<int> d2 = std::move(d1);
    d1.print();
    d2.print();

    dlist<int> d3;
    d3 = d2;
    d2.print();
    d3.print();

    dlist<int> d4;
    d4 = std::move(d3);
    d3.print();
    d4.print();

    for (auto i = d4.begin(); i != d4.end(); ++i)
        std::cout << *i << " ";
    std::cout << '\n';

    return 0;
}