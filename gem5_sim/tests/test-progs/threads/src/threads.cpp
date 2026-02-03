#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

std::mutex mtx;

void thread_function(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Hello from Thread " << id << " running on CPU!" << std::endl;
}

int main() {
    const int num_threads = 4;
    std::vector<std::thread> threads;

    std::cout << "Master thread starting..." << std::endl;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(thread_function, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All threads finished." << std::endl;
        return 0;
}
