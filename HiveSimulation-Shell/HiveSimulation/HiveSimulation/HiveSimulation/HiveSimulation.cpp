// HiveSimulation.cpp : Defines the entry point for the console application.
//


//naming standard set so that any _ following a variable name will normal show the unit 
//ie distTrav_km is measured in km

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>	
#include <string>
#include <cmath>

class workerEgg {

public:
	int age = 0;
};

class queenEgg {

public:
	int age = 0; //Queen emerged in mid may of previous year (365d-70d)
	int maxAge = 28;
	bool active = false;
};


struct measurables
{
	int population = -1;
	double PollenStore_g = -1;
};

class queenBee {

public: 
	double age = 0;
	double maxEggLaying = 1600;
};


class droneBee {

public:
	int age = 0;
	int maxAge = 37; //days; (Fukuda Ohtani, 1977) Full ref: Fukuda Ohtani 1977 life span drones:  summer: 14d, autumn: 32-42d
};

class workerBee {

public:
	int age = 0;
	int maxAge = 290; //days; (Sakagami & Fukuda, 1968) Full ref: max life span of worker, Sakagami & Fukuda 1968; Fukuda Ohtani 1977 life span drones:  summer: 14d, autumn: 32-42d
	int distTrav_km = 0;
	int exp = 0;
	int maxDistTrav_km = 800; // 838km: max flight performance in a foragers life (Neukirch 1982)
	double pollenPerTrip_mg = 1.5;

	//inidividualism
	double greed = 0;
	double greedBias = 0;
	double laziness = 0;
	double lazyBias = 0;
};

struct hive 
{
	bool colonyDied = false;

	//bees
	int totalBees = 0;
	int queens = 1;
	queenBee queenBee;
	int drones = 10;
	int workers = 0;
	//overall population (could be a variable here, could be calculated later)

	//eggs
	int workerEggCount = 0;
	int droneEggCount = 0;
	//pollen store
	double PollenStore_g = 100; //100g of pollen on start up
	double idealPollenStore_g = 1000; //1000g of pollen is ideal
	double minimumPollenStore_g = 50;
	//honey
	double honeyStore_g = 100; //100g of honey on start up

	double mortalilityInHive = 0.004; //derived from Martin 2001 (healthy winter based on 50% mortalility p.230)
};

void colonyAlive(int dayNumber, hive &Hive) 
{
	if (Hive.PollenStore_g <= 0)
	{
		std::cout << "No pollen remaining! Colony has starved on day: " << dayNumber << std::endl;
		Hive.colonyDied = true;

	}
	else if (Hive.totalBees <= 0) {
		std::cout << "No bees left! Colony died on day: " << dayNumber << std::endl;
		Hive.colonyDied = true;
	}
}


void countingBees(std::vector<workerEgg> workerEggs, std::vector<workerBee> workerBees, hive &Hive)
{
	//eggs
	Hive.workerEggCount = workerEggs.size();

	//bees
	Hive.workers = workerBees.size();
	Hive.totalBees = workerBees.size() + Hive.drones + Hive.queens;
	std::cout << "Total population today: " << Hive.totalBees << std::endl;
}

int joinForageValue(hive Hive){
	int joinForage = 50;

	if (Hive.PollenStore_g < Hive.idealPollenStore_g) 
	{
		joinForage = 5;
	}
	else if (Hive.PollenStore_g > Hive.idealPollenStore_g + 3000) //If way more than the ideal pollen count (ie 4Kg of pollen stored)
	{
		joinForage = 75;
	}
	else 
	{
		joinForage = 50;
	}
	return joinForage; //will either return 5, 50 or 80.
}

double seasonalMaxFoodAvailable(int dayNumber, int i) 
{
//we're gonna start on the first day of spring for ease of usage. 
//No of days for each season -spring 92, summer 92, autumn 91, winter 90.
//each day will have a semi random amount of food available based on the season's average.

	double maxPollenToday_g = 0;
	double sunlightToday = 0;
	std::string season = "";
	//sunlight per season -(Statista, 2018)  //full	Bibliography Statista. (2018). Monthly average of sun hours in the UK 2013-2017 | Statistic. [online] Available at: https://www.statista.com/statistics/322602/monthly-average-daily-sun-hours-in-the-united-kingdom-uk/ [Accessed 31 Mar. 2018].

	int x = dayNumber % 365;
	if (x < 92) //spring
	{
		season = "Season: spring";
		maxPollenToday_g = 50;
		sunlightToday = 4.9; //uk average 2015 (Statista, 2018)
	}
	else if (x < 184) //summer
	{
		season = "Season: summer";
		maxPollenToday_g = 70;
		sunlightToday = 6.5; //uk average 2015 (Statista, 2018)
	}
	else if (x < 275) //autumn
	{
		season = "Season: autumn";
		maxPollenToday_g = 20;
		sunlightToday = 4.7; //uk average 2015 (Statista, 2018)
	}
	else if (x < 365) //winter
	{
		season = "Season: winter";
		maxPollenToday_g = 0;
		sunlightToday = 1.5; //uk average 2015 (Statista, 2018)
	}

	if (i == 1)
	{
		std::cout << season << std::endl;
		return maxPollenToday_g;
	}
	else {
		return sunlightToday;
	}

}

int compare(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}

bool mainTickNormal(int dayNumber, std::vector<workerEgg> &workerEggs, std::vector<workerBee> &workerBees, hive &Hive, queenEgg &qEgg, std::vector<workerBee> &allBees)
{

	//----------------------------- Alive Hive -----------------------------------------------
	colonyAlive(dayNumber, Hive);
	if (Hive.colonyDied == true)
	{
		return Hive.colonyDied;
	}
	//----------------------------- Alive Hive -----------------------------------------------
	//----------------------------- Worker Egg Development -----------------------------------------------

	bool empty = workerEggs.empty();
	if (empty == false)
	{
		auto len = workerEggs.size(); //Checks how many worker eggs are in the array

		for (int i = 0; i < len; i++)
		{
			workerEggs.at(i).age = workerEggs.at(i).age + 1;	//increases the age of each egg
		}

		for (int i = 0; i < len; i++) {
			if (workerEggs.at(i).age == 21)
			{
				workerBee newBee; //Create a new bee
				workerBees.push_back(newBee); //add her to the vector
			}
			else if (workerEggs.at(i).age > 21)
			{
				workerEggs.erase(workerEggs.begin() + i);		//remove the egg from the array ie DIE.
				i = 0; //reset back to the beginning of the loop
				len = workerEggs.size(); //reset the side of the loop
			}
		}
	}

	auto len = workerEggs.size();
	Hive.workerEggCount = len; //update the number of eggs

	//----------------------------- Worker Egg Development -----------------------------------------------
	//----------------------------- Queen Egg Development -----------------------------------------------
	if (qEgg.active == true) 
	{
		qEgg.age = qEgg.age + 1;
		if (qEgg.age == qEgg.maxAge) 
		{
			queenBee newQueen;
			Hive.queenBee = newQueen;
			Hive.queens = Hive.queens +1;
			std::cout << "New queen has emerged" << std::endl;
			qEgg.active = false;
			qEgg.age = 0;
		}
	}
	//----------------------------- Queen Egg Development -----------------------------------------------
	//----------------------------- Worker Egg Laying -----------------------------------------------
	double maxPollenToday_g = seasonalMaxFoodAvailable(dayNumber, 1);
	Hive.queenBee.age += 1;
	double potentialEggs = 1600;
	double eggPercentage = 0;
	double pollenPercentage = 0;

	potentialEggs = Hive.queenBee.maxEggLaying + (-0.0027 * pow(Hive.queenBee.age, 2) + (0.395 * Hive.queenBee.age)); // this number should slowly decrease
	pollenPercentage = Hive.PollenStore_g / Hive.idealPollenStore_g;
	if (pollenPercentage > 1)
	{
		pollenPercentage = 1;
	}

	if (potentialEggs < 900) 
	{
		if (potentialEggs < 400) 
		{
			if (qEgg.active == false) 
			{
				qEgg.active = true;
				std::cout << "New queen egg set to hatch" << std::endl;
			}
			
		}
		potentialEggs = 800; //queen drops off but not completely 
	}
	
	if (Hive.queenBee.age <= 10)
	{
		potentialEggs = 0; // Winston p203: 5-6 days untill full sexual maturity, 2-4 days for orientation and mating flights
	}

	//no eggs for the first two months of winter
	if ((dayNumber % 365) < 337) 
	{
		if ((dayNumber % 365) > 275)
		{
			potentialEggs = 0;
		}
	}
	double eggsToday = 0;
	if (maxPollenToday_g / 50 != 0) {
		eggsToday = pollenPercentage * potentialEggs * (maxPollenToday_g / 40); // should give us a nicer bell curve like egg laying
		if (eggsToday > 1600) {
			eggsToday = 1600;
		}
	}
	else
	{
		eggsToday = pollenPercentage * potentialEggs;
	}

	

	if (Hive.PollenStore_g > Hive.minimumPollenStore_g)
	{	
		std::cout << "Eggs today: " << eggsToday << std::endl;
		workerEgg newEggs;
		for (int i = 0; i < eggsToday; i++)
		{
			workerEggs.push_back(newEggs); //puts new eggs into the stack
		}
	}

	//----------------------------- Worker Egg Laying -----------------------------------------------
	//----------------------------- Aging Workers -----------------------------------------------
	int length = workerBees.size();
	for (int i = 0; i < length; i++) //whether individual bee joins for foraging.
	{
		workerBees.at(i).age = workerBees.at(i).age + 1; // age the bees
		if (workerBees.at(i).age > 295)
		{
			allBees.push_back(workerBees.at(i)); //Add the dying bee to the history stack
			workerBees.erase(workerBees.begin() + i);
			i = 0; //reset back to the beginning of the loop
			length = workerBees.size(); //reset the side of the loop
		}
	}
	//----------------------------- Aging Workers -----------------------------------------------
	//----------------------------- Foraging -----------------------------------------------
	 //set the maximum available food for the day - Seasonal
	double initialMaxPollen = maxPollenToday_g;
	double sunlightToday = seasonalMaxFoodAvailable(dayNumber, 2);
	std::vector<int> foragers = {}; //clear the forage array
	std::vector<int> dead = {};
	int joinForage = joinForageValue(Hive); //bees generate a random number. If randNum is higher than join forage then they join the forage.

	std::mt19937 rng;
	rng.seed(std::random_device()()); //set up for creating a random number using the Mersenne Twister engine
	
	if (initialMaxPollen != 0) {
		for (int i = 0; i < workerBees.size(); i++) //whether individual bee joins for foraging.
		{
			std::uniform_int_distribution<std::mt19937::result_type> randNum(1, 100); // generates a random number between 1 and 100
			if (randNum(rng) > joinForage)
			{
				foragers.push_back(i); //the bee becomes a forager
			}
		}
	}

	std::cout << "Number of foragers today: " << foragers.size() << std::endl;
	bool winter_flag = false;
	int FindPollen = 100 - maxPollenToday_g; //spring 50%, summer 70%, autumn 25%, winter 0%
	double totalTimeToday = 0;
	int pointlessTrips = 0;
	do { //while there is time left in the day
		if (initialMaxPollen == 0)
		{
			if (winter_flag == false) 
			{
				int exiledBeePop = 0;
				exiledBeePop = (10000 - Hive.workers) * -1;
				if (exiledBeePop < 0)
				{
					exiledBeePop = 0;
				}

				for (int i = 0; i < exiledBeePop; i++)
				{
					dead.push_back(i);
				}
				winter_flag = true;
			}
		}

		else {
			auto len = foragers.size();
			for (int i = 0; i < len; i++) //for each individual bee which is foraging.
			{
				std::uniform_int_distribution<std::mt19937::result_type> randDistNum(1, 1326); //average bee journey out and back to colony, bee will travel 6630m (Seeley 1983)
				double journey = randDistNum(rng);
				workerBees.at(foragers.at(i)).distTrav_km += journey/100; 
				workerBee currentBee = workerBees.at(foragers.at(i));
				if (currentBee.distTrav_km >= currentBee.maxDistTrav_km) //Bee died mid journey therefore did not make it back to the hive
				{
					dead.push_back(foragers.at(i)); //the current bee has died.
					foragers.erase(foragers.begin() + i);
					len = foragers.size();
				}
				else
				{
					std::uniform_int_distribution<std::mt19937::result_type> forageNum(1, 100); // generates a random number between 1 and 100
					if (forageNum(rng) + currentBee.exp > FindPollen) //if they find a plant with pollen
					{
						if (maxPollenToday_g - (currentBee.pollenPerTrip_mg / 1000) <= 0) //if the trip will mean there is no pollen left today 
						{
							if (currentBee.exp < 29)
							{
								workerBees.at(foragers.at(i)).exp += 1; //add some and come home
							}
							pointlessTrips++; //it was pointless
						}
						else //collect!
						{
							maxPollenToday_g = maxPollenToday_g - (currentBee.pollenPerTrip_mg / 1000);
							Hive.PollenStore_g += (currentBee.pollenPerTrip_mg / 1000);
							if (currentBee.exp < 29)
							{
								workerBees.at(foragers.at(i)).exp += 1;
							}
						}
					}
					else
					{
						if (currentBee.exp < 29)
						{
							workerBees.at(foragers.at(i)).exp += 1;
						}
					}
				}
			}
		}
		totalTimeToday += 0.28; //Average Journey = 17mins : search trip : 17min(+-11). 17/60 = 0.283333333333, number has been rounded for simplisity 	
								//create death loop for today here.
	} while (totalTimeToday < sunlightToday);
	if (initialMaxPollen == 0) 
	{
		//nothing
		int x = 1;
	}
	else if (maxPollenToday_g == 0)
	{
		std::cout << "All pollen collected" << std::endl;
	}
	if (dayNumber == 640) 
	{
		std::cout << "WOOOOO" << std::endl;
	}

	std::sort(dead.begin(), dead.end()); //sort the vector, required to avoid OoB errors
	for (int i = dead.size() - 1; i > 0; i--)
	{
		allBees.push_back(workerBees.at(dead.at(i))); //Add the dying bee to the history stack
		workerBees.erase(workerBees.begin() + dead.at(i)); //death.
		
	}

	//use the worker numbers to work out how much foraging can be done today
	//bring the food back to the hive, add the distance travelled for all of the workers, kill off some of the workers during foraging.
	//add the pollen to the store
	//----------------------------- Foraging -----------------------------------------------
	//----------------------------- Mortality -----------------------------------------------


	//----------------------------- Mortality -----------------------------------------------
	countingBees(workerEggs, workerBees, Hive);
	//----------------------------- foodConsumption -----------------------------------------------

	double dailyPNeedAdult_mg = 1.5;	//1.5mg based on Pernal, Currio 2000, value for 14d old bees, fig 3
	double dailyPNeedDrone_mg = 2;		//estimate

	double totalConsump = (Hive.workers * dailyPNeedAdult_mg) + (Hive.drones * dailyPNeedDrone_mg);
	double dailyComp_g = totalConsump / 1000; //mg to g

	Hive.PollenStore_g = Hive.PollenStore_g - dailyComp_g;
	if (Hive.PollenStore_g < 0)
	{
		Hive.PollenStore_g = 0;
	}
	
	//----------------------------- foodConsumption -----------------------------------------------

	return Hive.colonyDied;
	
}

bool mainTickIndividual(int dayNumber, std::vector<workerEgg> &workerEggs, std::vector<workerBee> &workerBees, hive &Hive, queenEgg &qEgg, bool massExpulsion, std::vector<workerBee> &allBees)
{
	std::cout << "Individualism active" << std::endl;
	//----------------------------- Alive Hive -----------------------------------------------
	colonyAlive(dayNumber, Hive);
	if (Hive.colonyDied == true)
	{
		return Hive.colonyDied;
	}
	//----------------------------- Alive Hive -----------------------------------------------
	//----------------------------- Worker Egg Development -----------------------------------------------

	bool empty = workerEggs.empty();
	if (empty == false)
	{
		auto len = workerEggs.size(); //Checks how many worker eggs are in the array

		for (int i = 0; i < len; i++)
		{
			workerEggs.at(i).age = workerEggs.at(i).age + 1;	//increases the age of each egg
		}

		for (int i = 0; i < len; i++) {
			if (workerEggs.at(i).age == 21)
			{
				workerBee newBee; //Create a new bee
				workerBees.push_back(newBee); //add her to the vector
			}
			else if (workerEggs.at(i).age > 21)
			{
				workerEggs.erase(workerEggs.begin() + i);		//remove the egg from the array ie DIE.
				i = 0; //reset back to the beginning of the loop
				len = workerEggs.size(); //reset the side of the loop
			}
		}
	}

	auto len = workerEggs.size();
	Hive.workerEggCount = len; //update the number of eggs

	//----------------------------- Worker Egg Development -----------------------------------------------
	//----------------------------- Queen Egg Development -----------------------------------------------
	if (qEgg.active == true)
	{
		qEgg.age = qEgg.age + 1;
		if (qEgg.age == qEgg.maxAge)
		{
			queenBee newQueen;
			Hive.queenBee = newQueen;
			Hive.queens = Hive.queens + 1;
			std::cout << "New queen has emerged" << std::endl;
			qEgg.active = false;
			qEgg.age = 0;
		}
	}
	//----------------------------- Queen Egg Development -----------------------------------------------
	//----------------------------- Worker Egg Laying -----------------------------------------------
	double maxPollenToday_g = seasonalMaxFoodAvailable(dayNumber, 1);
	Hive.queenBee.age += 1;
	double potentialEggs = 1600;
	double eggPercentage = 0;
	double pollenPercentage = 0;

	potentialEggs = Hive.queenBee.maxEggLaying + (-0.0027 * pow(Hive.queenBee.age, 2) + (0.395 * Hive.queenBee.age)); // this number should slowly decrease
	pollenPercentage = Hive.PollenStore_g / Hive.idealPollenStore_g;
	if (pollenPercentage > 1)
	{
		pollenPercentage = 1;
	}

	if (potentialEggs < 900)
	{
		if (potentialEggs < 400)
		{
			if (qEgg.active == false)
			{
				qEgg.active = true;
				std::cout << "New queen egg set to hatch" << std::endl;
			}

		}
		potentialEggs = 800; //queen drops off but not completely 
	}

	if (Hive.queenBee.age <= 10)
	{
		potentialEggs = 0; // Winston p203: 5-6 days untill full sexual maturity, 2-4 days for orientation and mating flights
	}

	//no eggs for the first two months of winter
	if ((dayNumber % 365) < 337)
	{
		if ((dayNumber % 365) > 275)
		{
			potentialEggs = 0;
		}
	}
	double eggsToday = 0;
	if (maxPollenToday_g / 50 != 0) {
		eggsToday = pollenPercentage * potentialEggs * (maxPollenToday_g / 40); // should give us a nicer bell curve like egg laying
		if (eggsToday > 1600) {
			eggsToday = 1600;
		}
	}
	else
	{
		eggsToday = pollenPercentage * potentialEggs;
	}



	if (Hive.PollenStore_g > Hive.minimumPollenStore_g)
	{
		std::cout << "Eggs today: " << eggsToday << std::endl;
		workerEgg newEggs;
		for (int i = 0; i < eggsToday; i++)
		{
			workerEggs.push_back(newEggs); //puts new eggs into the stack
		}
	}

	//----------------------------- Worker Egg Laying -----------------------------------------------
	//----------------------------- Individualism -----------------------------------------------
	std::mt19937 rng;
	rng.seed(std::random_device()());

	for (int i = 0; i < workerBees.size(); i++) 
	{
		std::uniform_int_distribution<std::mt19937::result_type> randGreed(1, 100);
		workerBees.at(i).greed = randGreed(rng) + workerBees.at(i).greedBias;
		//std::cout << "Greed bias: " << workerBees.at(i).greedBias << " Greed: " << workerBees.at(i).greed << std::endl;
		if (workerBees.at(i).greed < 10) 
		{
			workerBees.at(i).greedBias = -5;
		}
		else if (workerBees.at(i).greed > 90) 
		{
			workerBees.at(i).greedBias = +5; //more greedy
		}
		std::uniform_int_distribution<std::mt19937::result_type> randLazy(1, 100);
		workerBees.at(i).laziness = randLazy(rng) + workerBees.at(i).lazyBias;
		if (workerBees.at(i).laziness < 10)
		{
			workerBees.at(i).lazyBias = -5; 
		}
		else if (workerBees.at(i).laziness > 90)
		{
			workerBees.at(i).lazyBias = +5; //less lazy
		}
	}
	

	//----------------------------- Individualism -----------------------------------------------
	//----------------------------- Aging Workers -----------------------------------------------
	int length = workerBees.size();
	for (int i = 0; i < length; i++) //whether individual bee joins for foraging.
	{
		workerBees.at(i).age = workerBees.at(i).age + 1; // age the bees
		if (workerBees.at(i).age > 295) 
		{
			allBees.push_back(workerBees.at(i)); //Add the dying bee to the history stack
			workerBees.erase(workerBees.begin() + i);
			i = 0; //reset back to the beginning of the loop
			length = workerBees.size(); //reset the side of the loop
		}
	}
	//----------------------------- Aging Workers -----------------------------------------------
	//----------------------------- Foraging -----------------------------------------------
	//set the maximum available food for the day - Seasonal
	double initialMaxPollen = maxPollenToday_g;
	double sunlightToday = seasonalMaxFoodAvailable(dayNumber, 2);
	std::vector<int> foragers = {}; //clear the forage array
	std::vector<int> dead = {};
	int joinForage = joinForageValue(Hive); //bees generate a random number. If randNum is higher than join forage then they join the forage.


	rng.seed(std::random_device()()); //set up for creating a random number using the Mersenne Twister engine

	if (initialMaxPollen != 0) {
		for (int i = 0; i < workerBees.size(); i++) //whether individual bee joins for foraging.
		{
			std::uniform_int_distribution<std::mt19937::result_type> randNum(1, 100); // generates a random number between 1 and 100
			if (randNum(rng)/2 + (workerBees.at(i).laziness/2) > joinForage)
			{
				foragers.push_back(i); //the bee becomes a forager
			}
		}
	}

	std::cout << "Number of foragers today: " << foragers.size() << std::endl;

	int FindPollen = 100 - maxPollenToday_g; //spring 50%, summer 70%, autumn 25%, winter 0%
	bool winter_flag = false;
	double totalTimeToday = 0;
	int pointlessTrips = 0;
	do { //while there is time left in the day
		if (initialMaxPollen == 0)
		{
			if (winter_flag == false)
			{
				int exiledBeePop = 0;
				exiledBeePop = (10000 - Hive.workers) * -1;
				if (exiledBeePop < 0)
				{
					exiledBeePop = 0;
				}

				for (int i = 0; i < exiledBeePop; i++)
				{
					dead.push_back(i);
				}
				winter_flag = true;
			}
			
		}
		else {
			auto len = foragers.size();
			for (int i = 0; i < len; i++) //for each individual bee which is foraging.
			{
				std::uniform_int_distribution<std::mt19937::result_type> randDistNum(1, 1326); //average bee journey out and back to colony, bee will travel 6630m (Seeley 1983)
				double journey = randDistNum(rng);
				workerBees.at(foragers.at(i)).distTrav_km += journey / 100;
				workerBee currentBee = workerBees.at(foragers.at(i));
				if (currentBee.distTrav_km >= currentBee.maxDistTrav_km) //Bee died mid journey therefore did not make it back to the hive
				{
					dead.push_back(foragers.at(i)); //the current bee has died.
					foragers.erase(foragers.begin() + i);
					len = foragers.size();
				}
				else
				{
					std::uniform_int_distribution<std::mt19937::result_type> forageNum(1, 100); // generates a random number between 1 and 100
					if (forageNum(rng) + currentBee.exp > FindPollen) //if they find a plant with pollen
					{
						if (maxPollenToday_g - (currentBee.pollenPerTrip_mg / 1000) <= 0) //if the trip will mean there is no pollen left today 
						{
							if (currentBee.exp < 29)
							{
								workerBees.at(foragers.at(i)).exp += 1; //add some and come home
							}
							pointlessTrips++; //it was pointless
						}
						else //collect!
						{
							maxPollenToday_g = maxPollenToday_g - (currentBee.pollenPerTrip_mg / 1000);
							Hive.PollenStore_g += (currentBee.pollenPerTrip_mg / 1000);
							if (currentBee.exp < 29)
							{
								workerBees.at(foragers.at(i)).exp += 1;
							}
						}
					}
					else
					{
						if (currentBee.exp < 29)
						{
							workerBees.at(foragers.at(i)).exp += 1;
						}
					}
				}
			}
		}
		totalTimeToday += 0.28; //Average Journey = 17mins : search trip : 17min(+-11). 17/60 = 0.283333333333, number has been rounded for simplisity 	
								//create death loop for today here.
	} while (totalTimeToday < sunlightToday);
	if (initialMaxPollen == 0)
	{
		//nothing
		int x = 1;
	}
	else if (maxPollenToday_g == 0)
	{
		std::cout << "All pollen collected" << std::endl;
	}


	std::sort(dead.begin(), dead.end()); //sort the vector, required to avoid OoB errors
	for (int i = dead.size() - 1; i > 0; i--)
	{
		allBees.push_back(workerBees.at(dead.at(i))); //Add the dying bee to the history stack
		workerBees.erase(workerBees.begin() + dead.at(i)); //death.
	}

	//use the worker numbers to work out how much foraging can be done today
	//bring the food back to the hive, add the distance travelled for all of the workers, kill off some of the workers during foraging.
	//add the pollen to the store
	//----------------------------- Foraging -----------------------------------------------
	//----------------------------- Mortality -----------------------------------------------


	//----------------------------- Mortality -----------------------------------------------
	countingBees(workerEggs, workerBees, Hive);
	//----------------------------- foodConsumption -----------------------------------------------

	double dailyPNeedAdult_mg = 1.5;	//1.5mg based on Pernal, Currio 2000, value for 14d old bees, fig 3
	double dailyPNeedDrone_mg = 2;		//estimate
	double totalConsump = 0;
	for (int i = 0; i < workerBees.size(); i++) 
	{
		double individualBeeConsump = (workerBees.at(i).greed / 50) + 0.5; //creates a number between 0.5-1.5
		totalConsump = dailyPNeedAdult_mg * individualBeeConsump + totalConsump;
	}

	double dailyComp_g = totalConsump / 1000; //mg to g

	Hive.PollenStore_g = Hive.PollenStore_g - dailyComp_g;
	if (Hive.PollenStore_g < 0)
	{
		Hive.PollenStore_g = 0;
	}

	//----------------------------- foodConsumption -----------------------------------------------

	return Hive.colonyDied;

}

hive hiveSetup(int len)
{
	hive newHive;
	newHive.workerEggCount = len;
	newHive.totalBees = newHive.queens + newHive.drones + newHive.workers;

	return newHive;
}

void clearspam() {
	std::cout << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;
}
//Spams stuff to clear the screen

void menu(int &dayNumberMax, bool &individualism, bool &massExpulsion){
	int x;
	
	std::cout << "Hive simulation program - Variable: (0 = false, 1 = true)" << std::endl;
	std::cout << "Individual is currently set to " << individualism << std::endl;
	std::cout << "Mass expulsion is currently set to " << massExpulsion << std::endl;
	std::cout << "Day program will run for: " << dayNumberMax << std::endl << std::endl;

	std::cout << "Menu:" << std::endl;
	std::cout << "1. Run" << std::endl;
	std::cout << "2. Variable options" << std::endl;

	std::cin >> x;

	if (x == 1) 
	{
		return ;
	}
	else if (x == 2)
	{
		std::cout << "Variable changes:" << std::endl;
		std::cout << "1. Run time" << std::endl;
		std::cout << "2. Individualism" << std::endl;
		
		std::cin >> x;

		if (x == 1)
		{
			std::cout << "Enter the number of days to run for:" << std::endl;
			std::cin >> x;
			dayNumberMax = x;
			clearspam();
			clearspam();
			clearspam();
			menu(dayNumberMax, individualism, massExpulsion);
		}
		else if (x == 2)
		{
			std::cout << "Individual is currently set to " << individualism << " (0 = false, 1 = true)"<<std::endl;
			std::cout << "1. set to true" << std::endl;
			std::cout << "2. set to false" << std::endl;
			std::cout << "3. return to menu" << std::endl;
			std::cin >> x;
			if (x == 1) 
			{
				individualism = true;
				std::cout << "Mass expulsion in winter on or off?" << std::endl;
				std::cout << "Mass expulsion is currently set to " << massExpulsion << " (0 = false, 1 = true)" << std::endl;
				std::cout << "1. set to true (default)" << std::endl;
				std::cout << "2. set to false " << std::endl;
				std::cin >> x;
				if (x == 1)
				{
					massExpulsion = true;
				}
				else if (x == 2) 
				{
					massExpulsion = false;
				}
			}
			else if (x == 2)
			{
				individualism = false;
			}
			else 
			{
				std::cout << "Returning to menu" << std::endl;
			}
			clearspam();
			clearspam();
			clearspam();
			menu(dayNumberMax, individualism, massExpulsion);
		}
	}

}
//displays the menu

void endMenu(measurables m)
{

}

int main() //starts the tick
{
	int dayNumberMax = 30; //default
	bool individualism = false; // off by default
	bool massExpulsion = true; // on by default
	int x;
	std::vector<workerBee> allBees = {};

	menu(dayNumberMax, individualism, massExpulsion);

	workerEgg starterEggs;
	starterEggs.age = 10;
	std::vector<workerEgg> workerEggs = {};
	for (int i = 0; i < 10000; i++) 
	{
		workerEggs.push_back(starterEggs); //puts 10000 eggs into the stack
	}

	workerBee starterBees;
	std::vector<workerBee> workerBees = {};
	for (int i = 0; i < 20000; i++)
	{
		workerBees.push_back(starterBees); //puts 1000 worker bees into the stack
	}

	//setup for new hive
	auto len = workerEggs.size();
	hive startHive; //= hiveSetup(len);
	startHive.queenBee.age = 295;
	countingBees(workerEggs, workerBees, startHive);

	std::cout << "Total bees at start: " << startHive.totalBees << std::endl;
	
	queenEgg qEgg;
	measurables Measurable[2000];
	bool colonyDied = false;
	for (int i = 0; i < dayNumberMax; i++)
	{
		if (colonyDied == false) 
		{
			if (individualism == false) 
			{
				std::cout << "---------------------------------------------------" << std::endl;
				colonyDied = mainTickNormal(i, workerEggs, workerBees, startHive, qEgg, allBees);
				Measurable[i].population = startHive.totalBees;				//update the measurables
				std::cout << "Population on day " << i << ": " << startHive.totalBees << std::endl;
				Measurable[i].PollenStore_g = startHive.PollenStore_g;		//update the measurables
				std::cout << "Pollen store on day " << i << ": " << startHive.PollenStore_g << std::endl;
			}
			else if (individualism == true) 
			{
				std::cout << "---------------------------------------------------" << std::endl;
				colonyDied = mainTickIndividual(i, workerEggs, workerBees, startHive, qEgg, massExpulsion, allBees);
				Measurable[i].population = startHive.totalBees;				//update the measurables
				std::cout << "Population on day " << i << ": " << startHive.totalBees << std::endl;
				Measurable[i].PollenStore_g = startHive.PollenStore_g;		//update the measurables
				std::cout << "Pollen store on day " << i << ": " << startHive.PollenStore_g << std::endl;
			}
		}
	}

	for (int i = 0; i < workerBees.size(); i++) 
	{
		allBees.push_back(workerBees.at(i)); //Add the remaining bees to the history stack
	}

	//endMenu, make this a function if we can.
	std::cout << "---------------------------------------------------" << std::endl;
	std::cout << "Simulation completed - results" << std::endl;
	std::cout << "1. Population" << std::endl;
	std::cout << "2. Pollen store amounts" << std::endl;
	std::cout << "3. Print all individual bee information" << std::endl;
	std::cout << "4. export data to text file" << std::endl;
	std::cin >> x;
	bool y = true;
	do {
		if (x == 1)
		{
			for (int i = 0; i < 2000; i++)
			{
				if (Measurable[i].population != -1)
				{
					std::cout << "Day " << i << ": " << Measurable[i].population << std::endl;
				}
			}
		}
		else if (x == 2)
		{
			for (int i = 0; i < 2000; i++)
			{
				if (Measurable[i].PollenStore_g != -1)
				{
					std::cout << "Day " << i << ": " << Measurable[i].PollenStore_g << std::endl;
				}
			}
		}
		else if (x == 3)
		{
			std::cout << "Bee number\tAge\tDistance travelled\tGreed Bias\tLaziness Bias" << std::endl;
			for (int i = 0; i < allBees.size(); i++) 
			{
				std::cout << i << "\t" << allBees.at(i).age << "\t" << allBees.at(i).distTrav_km << "\t" << allBees.at(i).greedBias << "\t" << allBees.at(i).lazyBias << std::endl;
			}
		}
		else if (x == 4) 
		{
			std::string fileRoot, fileName;
			std::ofstream myfile;
			std::cout << "Enter a name for the file:" << std::endl;
			std::cin >> fileRoot;
			fileName = "H:\\3rdYEARbby\\FYP\\Data\\" + fileRoot + "Population.txt";
			myfile.open(fileName);

			myfile << "Day\tPopulation" << std::endl;
			for (int i = 0; i < 2000; i++)
			{
				if (Measurable[i].population != -1)
				{
					myfile << i << "\t" << Measurable[i].population<< std::endl;
				}
			}
			

			std::cout << "Saved at: " << fileName << std::endl;
			myfile.close();

			fileName = "H:\\3rdYEARbby\\FYP\\Data\\" + fileRoot + "pollenStore_g.txt";
			myfile.open(fileName);

			myfile << "Day\tPollenStore (g)" << std::endl;
			for (int i = 0; i < 2000; i++)
			{
				if (Measurable[i].PollenStore_g != -1)
				{
					myfile << i << "\t" << Measurable[i].PollenStore_g << std::endl;
				}
			}
			std::cout << "Saved at: " << fileName << std::endl;
			myfile.close();

			fileName = "H:\\3rdYEARbby\\FYP\\Data\\" + fileRoot + "AllBees.txt";
			myfile.open(fileName);

			myfile << "Bee number\tAge\tDistance travelled\tGreed Bias\tLaziness Bias" << std::endl;
			for (int i = 0; i < allBees.size(); i++)
			{
				myfile << i << "\t" << allBees.at(i).age << "\t" << allBees.at(i).distTrav_km << "\t" << allBees.at(i).greedBias << "\t" << allBees.at(i).lazyBias << std::endl;
			}
			std::cout << "Saved at: " << fileName << std::endl;
			myfile.close();
		}
		
		std::cout << "---------------------------------------------------" << std::endl;
		std::cout << "Simulation completed - results" << std::endl;
		std::cout << "1. Population" << std::endl;
		std::cout << "2. Pollen store amounts" << std::endl;
		std::cout << "3. Print all individual bee information" << std::endl;
		std::cout << "4. export data to text file" << std::endl;
		std::cin >> x;

	} while (y == true);
	//endMenu, make this a function if we can.
    return 0;
}

