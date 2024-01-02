#include "test.h"

using namespace std;

my_class::my_class(int num)
{
	this->num = num;
}
void my_class::t_run()
{
	thread temp(&my_class::run, this);
    temp.detach();
}
void my_class::run()
{
    cout << "MEM ADDR : " << this << endl;
	while(true)
	{
		cout << "ELEM : " << this->num << endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}