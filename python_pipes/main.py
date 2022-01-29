import sys
import os
import pipe_utils
import subprocess
import threading
import time


def thread_function(name):

    # Process function
    while(1):
        print("hello")
        time.sleep(3)

        #process the array
        array = pipe.output_array
        print(array)
        print(len(array))


if __name__ == '__main__':

    if len(sys.argv) < 2:
        print("need s or c as argument")

    global pipe
    global c
    global flag

    # Initialize pipe network objects
    pipe = pipe_utils.Pipe_Network()
    c = threading.Condition()
    flag = 0  # shared between Thread_A and Thread_B

    # Call Pipe launcher executable to read from shared memory
    relPathToLaunch = '../../../Reading_Program_Shared_Memory/program_B/Release'
    origWD = os.getcwd()  # remember our original working directory
    os.chdir(os.path.join(os.path.abspath(sys.path[0]), relPathToLaunch))
    subprocess.Popen(['program.exe'])
    os.chdir(origWD)  # get back to our original working directory


    if sys.argv[1] == "s":
        pipe.server()
    elif sys.argv[1] == "c":

        x = threading.Thread(target=thread_function, args=(1,))
        x.start()

        pipe.client()
    else:
        print(f"no can do: {sys.argv[1]}")