/*

Author: Noaima Bari
Class: ECE6122
Last Date Modified: 6 Dec 2021
Description:
Covers point (8) of the project: Creates a class called ECE_UAV 
that has member variables to contain the mass, (x, y, z) position, 
(vx, vy, yz) velocities, and (ax, ay, az) accelerations of the UAV. 
Also contains a member variable of type std::thread.
A member function start() causes the thread member variable to run an external function called threadFunction

*/

#pragma once
#include<thread>
#include<string>
#include<atomic>
//#include<unordered_map>
#include<mutex>

class ECE_UAV
{
public:
	void start();
	void stop();
	void changePosition();
	void position(double* inPos)
	{
		// use a mutex
		m_mutex.lock();
		memcpy(m_Position, inPos, 3 * sizeof(double));
		m_mutex.unlock();
	} // this has to be thread safe

	void getPosition(double* inPos)
	{
		m_mutex.lock();
		memcpy(inPos, m_Position, 3 * sizeof(double));
		m_mutex.unlock();
	}
	void velocity(double* inVel)
	{
		m_mutex.lock();
		memcpy(m_Velocity, inVel, 3 * sizeof(double));
		m_mutex.unlock();
	}
	void getVelocity(double* inVel)
	{
		m_mutex.lock();
		memcpy(inVel, m_Velocity, 3 * sizeof(double));
		m_mutex.unlock();
	}
	std::atomic<bool> m_bStop = false;
	bool collide = true;
	//unordered_map<int, int> collide;


private:


	double m_mass = 1.0; // each uav has a mass of 1 kg
	double m_Acceleration[3] = {};
	double m_Position[3] = {};
	double m_Velocity[3] = { 0 };
	//bool flag = false;



	std::thread m_KinematicsThread;
	std::mutex m_mutex;

};

