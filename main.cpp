
#include <iostream>
#include "evicting_map.hpp"

void fun(const std::string& key, const std::string& value)
{
    cout<<"call_back:"<<"key:"<<key<<", value:" << value << std::endl;
}

int main()
{
    EvictingMap<std::string, std::string> M(5, 2);
    M.SetEvictingCallback(fun);
    M.Put("1","11");
    cout<<"----------------"<<endl;
    M.Put("2","22");
//    cout<<"head "<<M._list.head<<endl;
    cout<<M.ListDebugString()<<endl;
    cout<<"----------------"<<endl;
    M.Put("1","33");
//    cout<<"head "<<M._list.head<<endl;
    cout<<M.ListDebugString()<<endl;

    cout<<"----------------"<<endl;
    const std::string* value = M.Get("2");
    cout<<"Get 2:"<<*value<<endl;
    cout<<M.ListDebugString()<<endl;


    cout<<"----------------"<<endl;
    M.Put("3","33");
    cout<<M.ListDebugString()<<endl;
    for(int i = 4 ; i < 100; ++i)
    {
        M.Put(std::to_string(i),std::to_string(i)+"---");
    }
    cout<<"size:"<<M.Size()<<","<<M.ListSize()<<endl;
    return 0;
}
