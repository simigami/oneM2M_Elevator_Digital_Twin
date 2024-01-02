#pragma once
#include <thread>
#include <iostream>

class my_class
{
public:
    my_class(int num);
    void run();
    void t_run();
	int num;
};