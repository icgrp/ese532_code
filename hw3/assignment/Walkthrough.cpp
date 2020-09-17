#include <iostream>
#include <thread>
#include "Utilities.h"

// gets the thread id of the main thread
std::thread::id main_thread_id = std::this_thread::get_id();

// checks if running on main thread using the id
void is_main_thread() {
  if ( main_thread_id == std::this_thread::get_id() )
    std::cout << "This is the main thread." << std::endl;
  else
    std::cout << "This is not the main thread." << std::endl;
}

void my_function(int a, int b, int&c) {
    c = a + b;
    std::cout << "From thread id:"
            << std::this_thread::get_id()
            << " a+b=" << c << std::endl;
}

int main() {
    // Assign main thread to cpu 0
    pin_main_thread_to_cpu0();

    int a = 2;
    int b = 3;
    int c;
    my_function(a, b, c);
    is_main_thread();

    // create a new thread, note it's not running
    // anything yet.
    std::thread th;

    // construct the thread to run is_main_thread
    // note, as soon as you construct it, the thread
    // starts running.
    // You could create and run at the same time
    // by writing: std::thread th(is_main_thread);
    th = std::thread(is_main_thread);

    // Assign our thread to cpu 1.
    pin_thread_to_cpu(th, 1); 

    // wait for the thread to finish.
    th.join();

    std::thread th2(&my_function, a, b, std::ref(c));
    th2.join();
}