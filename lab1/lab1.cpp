#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <list>
#include <condition_variable>

using namespace std;

mutex globalMutex;
condition_variable condVar;

bool ready = false;
bool predT() { return ready == true; }
int hub;
list<int> startList = {1, 2, 3, 4, 5, 6, 7, 8, 9};
list<int> endList;

void Provide(list<int> x) {
    while (!x.empty()) {
        unique_lock<mutex> uniqueLock1(globalMutex);
        while (ready) {
            condVar.wait(uniqueLock1);
        }
        this_thread::sleep_for(chrono::seconds(1));
        int temp = *x.begin();
        x.pop_front();
        hub = temp;
        cout << "Sended element: " << hub << "   Thread id: " \
        << this_thread::get_id() << "\n";
        ready = true;
        condVar.notify_all();
        uniqueLock1.unlock();
    }
}

void Consume() {
    while (endList.size() != startList.size()) {
        unique_lock<mutex> uniqueLock2(globalMutex);
        condVar.wait(uniqueLock2, predT);
        endList.push_front(hub);
        cout << "Recieved element: " << *endList.begin() << "   Thread id: " \
        << this_thread::get_id() << "\n";
        ready = false;
        condVar.notify_all();
    }
}

int main() {
    thread t1(Provide, startList);
    thread t2(Consume);
    t1.join();
    t2.join();
}
