#include <Logger.h>
#include <vector>
#include <thread>
#include <chrono>
#include <string>

int main()
{
    std::vector<std::thread> threadPool;

    int numThreads = 3;

    for (int i = 0; i < numThreads; i++)
    {
        threadPool.emplace_back([i](){
            
            for (int j = 0; j < 100; j++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 300));

                std::string mesg1 = "Thread " + std::to_string(i) + "Message " + std::to_string(j);
                LOG(LogLevel::INFO, mesg1);
            }

        });
    }
   
    for (auto& thread : threadPool)
        thread.join();

   return 1; 
}