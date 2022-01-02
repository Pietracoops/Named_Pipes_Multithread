#include <iostream>
#include "pipes.h"
#include <thread>
#include <string>

int broadcast_var = 0;


Pipes_Server server;

void thread1()
{
	//Launch broadcasting pipes

	server.launch_server();
}

void thread2()
{
	//Launch broadcasting pipes
	while (true)
	{
		std::cout << "broadcast string: " << std::to_string(broadcast_var) <<std::endl;
		broadcast_var++;
		server.pipe_broadcast_string = std::to_string(broadcast_var);
		Sleep(1000);
	}

}


int main()
{
	
	std::thread t1(thread1);
	std::thread t2(thread2);

	t1.join();
	t2.join();


	return 0;
}