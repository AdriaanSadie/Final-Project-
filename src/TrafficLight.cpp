#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  
  	std::unique_lock<std::mutex> rec_Lock(q_mutex);
  	q_cond.wait(rec_Lock, [this] { return !_queue.empty(); });
  
  	T msg = std::move(_queue.back());
  	_queue.clear();
  
  	return msg;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  
  	//std::cout << "DOES IT EVEN GET TO THIS PART??????????" << std::endl; 	
  
  	std::lock_guard<std::mutex> send_Lock(q_mutex);
  
  	_queue.push_back(std::move(msg));
  	q_cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
  	phase_queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	
  	std::lock_guard<std::mutex> greenLock(_mutex);
  
  	while(true){
    	TrafficLightPhase message = phase_queue->receive();
     
      	if(message == TrafficLightPhase::green){
         	break; 
        }
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}


void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
  
}


// virtual function which is executed in a thread
[[noreturn]]
void TrafficLight::cycleThroughPhases(){
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  

  	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  	srand(time(0));
  
  	float rand_time = rand() % 2000 + 4000; // Random number in milliseconds between 4000 and 6000
  
  	while(true){
      	std::this_thread::sleep_for(std::chrono::milliseconds(1));  
      
      	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
      
      	//Check time interval
      	if (duration >= rand_time){
          	          	
        	if (_currentPhase == TrafficLightPhase::red){
           		_currentPhase = TrafficLightPhase::green;
            }
          	else{
            	_currentPhase = TrafficLightPhase::red;  
            }
          	
          	// Add this part once task FP.3 is done

          	auto this_phase = _currentPhase;
          	auto ftr = std::async(std::launch::async,&MessageQueue<TrafficLightPhase>::send, phase_queue, std::move(this_phase));
         	ftr.wait();

          
          	//Reset timer:
          	t1 = std::chrono::high_resolution_clock::now();
          	//Get new random number:
          	rand_time = rand() % 2000 + 4000;
        }
    }
}

