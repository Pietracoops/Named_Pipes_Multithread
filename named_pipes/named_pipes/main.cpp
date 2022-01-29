#include <iostream>
#include "pipes.h"
#include <thread>
#include <string>
#include "shared_memory.h"
#include "SharedMemory.h"

int broadcast_var = 0;


Pipes_Server server;


void thread1()
{
	//Launch broadcasting pipes

	std::cout << "launching server" << std::endl;
	server.launch_server();
}

void thread2()
{

	int n = 100; // size of shared memory block (bytes)
	char name[] = "shared_memory1"; // name of shared memory block

	double A[1];
	char str[1000];

	SharedMemory SM(name, n, 1, 2);

	while (true)
	{
		SM.read_data(A, str);

		std::cout << "broadcast string: " << std::to_string(A[0]) << std::endl;

		server.pipe_broadcast_string = std::to_string(A[0]);

		//cout << A[0] << " Seconds";
		Sleep(1000);
	}



	//Launch broadcasting pipes
	//while (true)
	//{
	//	std::cout << "broadcast string: " << std::to_string(broadcast_var) <<std::endl;
	//	broadcast_var++;
	//	server.pipe_broadcast_string = std::to_string(broadcast_var);
	//	Sleep(1000);
	//}

}


int main()
{
	
	std::thread t1(thread1);
	std::thread t2(thread2);


	t1.join();
	t2.join();


	std::cout << "\ndone.\n";


	return 0;
}