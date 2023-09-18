/*

Author: Noaima Bari
Class: ECE6122
Last Date Modified: 6 Dec 2021
Description:
Implements member functions of ECE_UAV class and also includes the threadFunction

*/

#include<chrono>
#include<math.h>
#include<iostream>
#include<time.h>
#include<stdlib.h>


using namespace std;

#include "ECE_UAV.h"

extern ECE_UAV ece_uav[15];

bool collision(double* pos1, double* pos2)
{
	// Checks for collision
	// Parameters: pos1(x, y, z coordinates of 1st uav) pos2: (coordinates of the other uav)

	double diff1 = pos1[0] > pos2[0] ? pos1[0] - pos2[0] : pos2[0] - pos1[0];
	double diff2 = pos1[1] > pos2[1] ? pos1[1] - pos2[1] : pos2[1] - pos1[1];
	double diff3 = pos1[2] > pos2[2] ? pos1[2] - pos2[2] : pos2[2] - pos1[2];

	if (diff1 <= 0.01 && diff2 <= 0.01 && diff3 <= 0.01)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void threadFunction(ECE_UAV* pUAV)
{
	// Function to update the kinematic information of a UAV every 10 msec
	// Parameters: pUAV(pointer to ECE_UAV object)

	std::this_thread::sleep_for(std::chrono::seconds(5));
	double pos1[3], vel1[3], pos2[3], vel2[3];

	do
	{
		// Calculate the new location of the UAV

		pUAV->changePosition();
		pUAV->getPosition(pos1);
		pUAV->getVelocity(vel1);

		// Checking for collision
		
		int j;
		for (int i = 0; i < 15; i++)
		{
			if (&ece_uav[i] == pUAV)
			{
				j = i;
				break;
			}
		}
		
		for (int i = j+1; i < 15; i++)
		{
			if (&ece_uav[i] != pUAV)
			{
				ece_uav[i].getPosition(pos2);
				if (collision(pos1, pos2) && (pUAV->collide == true))
				{
					// swap velocities
					
					ece_uav[i].velocity(vel1);
					pUAV->velocity(vel2);

				}
			}
			
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	} while (!pUAV->m_bStop);

}

void ECE_UAV::start()
{
	m_KinematicsThread = std::thread(threadFunction, this);
}

void ECE_UAV::stop()
{
	m_bStop = true;
	if (m_KinematicsThread.joinable())
	{
		m_KinematicsThread.join();
	}
}

void ECE_UAV::changePosition()
{

	// Function to change position of UAV according to given motion equations and Hooke's law

	double x_old = m_Position[0];
	double y_old = m_Position[1];
	double z_old = m_Position[2];

	// Finding direction

	double Dx = 0 - x_old;
	double Dy = 0 - y_old;
	double Dz = 135 - z_old;



	// Finding force

	double D = sqrt(Dx * Dx + Dy * Dy + Dz * Dz);	// distance of UAV from center of sphere

	double Fx = Dx / D;
	double Fy = Dy / D;
	double Fz = Dz / D;

	//double Fs, Fsx, Fsy, Fsz, k;
	double Fs, k;
	double a, b, c;

	if (D < 10)
	{

		// Reinitialize velocity vector (a, b, c) as perpendicular to surface
	

		// a.Dx + b.Dy + c.Dz = 0

		// Randomly generating a and b

		srand(m_Position[0]);
		a = rand() % 500;
		srand(m_Position[1]);
		b = rand() % 500;

		c = -(a * Dx + b * Dy) / Dz;

		double d = sqrt(a * a + b * b + c * c);

		a /= d;
		b /= d;
		c /= d;


		m_Velocity[0] = 5*a;
		m_Velocity[1] = 5*b;
		m_Velocity[2] = 5*c;


		// Applying Hooke's law

		k = 5;
		//cout <<"F = "<< F << endl;
		Fs = k * (10 - D);
		Fx = Fs * (-Dx / D);
		Fy = Fs * (-Dy / D);
		Fz = Fs * (-Dz / D);

		//cout << D << endl;

	}

	// Finding acceleration


	double ax = Fx / m_mass;
	double ay = Fy / m_mass;
	double az = Fz / m_mass;

	// Finding velocity

	double t = 0.045;

	double vx = m_Velocity[0] + ax * t;
	double vy = m_Velocity[1] + ay * t;
	double vz = m_Velocity[2] + az * t;
	m_Velocity[0] = vx;
	m_Velocity[1] = vy;
	m_Velocity[2] = vz;
	

	// Finding position

	double x_new = x_old + vx * t + 0.5 * ax * t * t;
	double y_new = y_old + vy * t + 0.5 * ay * t * t;
	double z_new = z_old + vz * t + 0.5 * az * t * t;


	// Updating the position

	m_Position[0] = x_new;
	m_Position[1] = y_new;
	m_Position[2] = z_new;

	//cout<< x_new <<", "<<y_new<<", "<<z_new<<endl;


}