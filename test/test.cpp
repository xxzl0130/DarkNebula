#include <iostream>
#include <DarkNebula/Timer.h>

int main()
{
	dn::Timer timer(100, [&]()
		{
			std::cout << "Hello World!\n";
		});
	timer.start();
	Sleep(1000);
	return 0;
}
