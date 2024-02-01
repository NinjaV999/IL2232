#this program is used to Runing the program
import subprocess
import  time

compile_process = subprocess.run(["cmake", "--build", "./cmake-build-debug"], stdout=subprocess.PIPE, text=True)
print("Please select the runnning mode. Input 0 will evaluate RL agent, Input 1 will train a new RL agent")

i=input("Please select the mode:")
if (i== '0'):
    print("Evaluation RL agent \n")
    print("Running RLevaluation program \n")
    evaluation_process=subprocess.Popen(["python", "RLevaluation.py"], stdout=subprocess.PIPE)
    time.sleep(5)
    print("Running C++ program...")
    working_directory = "./cmake-build-debug"

    cpp_process = subprocess.Popen(["./hello_cmake"], stdout=subprocess.PIPE, text=True, cwd=working_directory)
    for line in cpp_process.stdout:
        print(line.strip())
        if "#STOP_PROGRAM" in line:
            print("Detected stop signal. Stopping the program")
            evaluation_process.terminate()
            cpp_process.terminate()

            break

else:
    print("Training RL agent \n")
    trainning_process=subprocess.Popen(["python", "RLtrain_new.py"])
    time.sleep(5)
    print("Running C++ program")
    working_directory = "./cmake-build-debug"
    cpp_process = subprocess.Popen(["./hello_cmake"], cwd=working_directory, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    while True:
            finishReady=0
            finishfile=open("./cmake-build-debug/finish.txt",'r')
            finishReady=finishfile.read()

            if finishReady == '1' :
                #print(finishReady)
                finishfile1=open("./cmake-build-debug/finish.txt",'w+')
                finishfile1.write('0')
                print("Detected stop signal. Stopping the program")
                trainning_process.terminate()
                cpp_process.terminate()
                break


