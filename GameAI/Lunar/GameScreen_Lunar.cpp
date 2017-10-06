//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#include "GameScreen_Lunar.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "LunarConstants.h"
#include "LunarLander.h"
#include <time.h>
#include <fstream>

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::GameScreen_Lunar(SDL_Renderer* renderer) : GameScreen(renderer)
{
	srand(static_cast <unsigned> (time(0)));

	//Set up the platform to land on.
	mPlatform = new Texture2D(renderer);
	mPlatform->LoadFromFile("Images/Lunar/Platform.png");

	//Create a lunar lander to use for tests.
	mPlatformPosition = Vector2D(198,430);

	//Player controlled lander.
	mLunarLander = new LunarLander(renderer, Vector2D(400,125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));

	//AI controlled landers.
	mGeneration = 0;
	GenerateRandomChromosomes();
	for(int i = 0; i < kNumberOfAILanders; i++)
		mAILanders[i] = new LunarLander(renderer, Vector2D(400,125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));
	RestartGA();
	mAllowMutation = true;
	mAccumulatedDeltaTime = 0;
	//-------------------------------------

	//Start game in frozen state.
	mPause = true;

	//Start game in player control.
	mAIOn = false;
}

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::~GameScreen_Lunar()
{
	delete mPlatform;
	mPlatform = NULL;

	delete mLunarLander;
	mLunarLander = NULL;

	for(int i = 0; i < kNumberOfAILanders; i++)
		delete *mAILanders;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Render()
{
	mPlatform->Render(mPlatformPosition);

	if(mAIOn)
	{
		for(int i = 0; i < kNumberOfAILanders; i++)
			mAILanders[i]->Render();
	}
	else
		mLunarLander->Render();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Update(size_t deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch(e.type)
	{
		//Deal with mouse click input.
		case SDL_KEYUP:
			switch(e.key.keysym.sym)
			{
				case SDLK_SPACE:
					mPause = !mPause;
					if(mPause)
						cout << "Paused" << endl;
					else
						cout << "Unpaused" << endl;
				break;

				case SDLK_a:
					mAIOn = !mAIOn;
					if(mAIOn)
						cout << "AI on" << endl;
					else
						cout << "AI off" << endl;
				break;

				case SDLK_LEFT:
					if(!mAIOn)
						mLunarLander->TurnLeft(deltaTime);
				break;

				case SDLK_RIGHT:
					if(!mAIOn)
						mLunarLander->TurnRight(deltaTime);
				break;

				case SDLK_UP:
					if(!mAIOn)
						mLunarLander->Thrust();
				break;

				case SDLK_r:
					if (!mAIOn)
					{
						mLunarLander->Reset();
						return;
					}
				break;
			}
		break;
	
		default:
		break;
	}
	
	if(!mPause)
	{
		mAccumulatedDeltaTime += deltaTime;
		if (mAccumulatedDeltaTime > 50)
		{
			//Reset the count.
			mAccumulatedDeltaTime = 0;

			if (!mAIOn)
				mLunarLander->Update(deltaTime, e);
			else
				UpdateAILanders(deltaTime, e);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::GenerateRandomChromosomes()
{
	for(int lander = 0; lander < kNumberOfAILanders; lander++)
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[lander][action] = (LunarAction)(rand()%LunarAction_MaxActions);
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::ClearChromosomes()
{
	for(int lander = 0; lander < kNumberOfAILanders; lander++)
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[lander][action] = LunarAction_None;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::UpdateAILanders(size_t deltaTime, SDL_Event e)
{
	int numberDeadOrLanded = 0;

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		mAILanders[i]->Update(deltaTime, e);

		if (!mAILanders[i]->IsAlive() || mAILanders[i]->HasLanded())
			numberDeadOrLanded++;
	}

	//Are all finished?
	if (numberDeadOrLanded == kNumberOfAILanders)
	{
		CalculateFitness();
		return;
	}

	//Move on to the next action in the Chromosomes.
	mCurrentAction++;
	if (mCurrentAction >= kNumberOfChromosomeElements)
		return;

	//Assign all actions to landers.
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		switch (mChromosomes[i][mCurrentAction])
		{
			case LunarAction_LunarLeft:
				mAILanders[i]->TurnLeft(deltaTime);
			break;

			case LunarAction_LunarRight:
				mAILanders[i]->TurnRight(deltaTime);
			break;

			case LunarAction_LunarThrust:
				mAILanders[i]->Thrust();
			break;
		}
	}
}
//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::CalculateFitness()
{
	// Give each lander a score based on distance from platform centre and time taken to reach platform. Higher score is better

	double totalFitness = 0;
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		const double x_diff = mAILanders[i]->GetCentralPosition().x - mPlatformPosition.x;
		const double y_diff = mAILanders[i]->GetCentralPosition().y - mPlatformPosition.y;

		const double disFit = std::sqrt(x_diff * x_diff + y_diff * y_diff);
		const double rotFit = std::abs(360 - mAILanders[i]->GetRotationAngle());

		if (mAILanders[i]->IsAlive()) // If the lander survived
		{
			// The shortest time to reach the platform is best
			mFitnessValues[i] = 10000 - mAILanders[i]->GetSurvivalTime();
		}
		else
		{
			// A mixture of closest distance to platform, closest rotation to 0, longest time survived
			mFitnessValues[i] = (500 - disFit) + (100 - rotFit) + mAILanders[i]->GetSurvivalTime();
		}

		totalFitness += mFitnessValues[i];
	}

	cout << "Total fitness: " << std::to_string(totalFitness) << endl;

	Selection();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Selection()
{
	// Select the top N (EVEN NUMBER) landers based on their score.

	mSelectedAIChromosomes[kNumberOfAILanders][kNumberOfChromosomeElements] = { };

	double highestScore = 0;

	// Find the first highest score
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		if (mFitnessValues[i] > highestScore)
		{
			highestScore = mFitnessValues[i]; // Set it for future iterations

			for (int action = 0; action < kNumberOfChromosomeElements; action++)
			{
				mSelectedAIChromosomes[0][action] = mChromosomes[i][action];
			}
		}
	}

	double currentHighestScore = 0;

	// Acquire the top 'kChromosomesToEvolve' scores
	for (int i = 1; i < kChromosomesToEvolve; i++)
	{		
		int highestIndex = 0;

		for (int j = 0; j < kNumberOfAILanders; j++)
		{
			if (mFitnessValues[j] < highestScore && mFitnessValues[j] > currentHighestScore)
			{
				currentHighestScore = mFitnessValues[j];
				highestIndex = j;
			}
		}

		highestScore = currentHighestScore;

		for (int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			mSelectedAIChromosomes[i][action] = mChromosomes[highestIndex][action];
		}
	}

	Crossover();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Crossover()
{
	// Choose two random landers from the new pool. Combine half and half each of the landers chromosomes to create a new lander.
	// Do for every lander

	int count = 0;
	for (int i = 0; i < kChromosomesToEvolve; i += 2)
	{
		// Half the chromosomes will be come from i
		for (int action = 0; action < kNumberOfChromosomeElements/2; action++)
		{
			mChromosomes[count][action] = mSelectedAIChromosomes[i][action];
		}

		// Half the chromosomes will be come from i + 1
		for (int action = kNumberOfChromosomeElements / 2; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[count][action] = mSelectedAIChromosomes[i + 1][action];
		}

		count++;
	}

	for (int i = 0; i < kChromosomesToEvolve; i += 2)
	{
		for (int action = 0; action < kNumberOfChromosomeElements / 2; action++)
		{
			mChromosomes[count][action] = mSelectedAIChromosomes[i+1][action];
		}

		for (int action = kNumberOfChromosomeElements / 2; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[count][action] = mSelectedAIChromosomes[i][action];
		}

		count++;
	}

	Mutation();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Mutation()
{
	// Randomly change a single gene in the new landers.

	for (int i = 0; i < kChromosomesToEvolve; i++)
	{
		mChromosomes[i][(rand() % kNumberOfChromosomeElements)] = (LunarAction)(rand() % LunarAction_MaxActions);
	}

	// Fill the rest with random chromosomes
	for (int i = kChromosomesToEvolve; i < kNumberOfAILanders; i++)
	{
		for (int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			mChromosomes[i][action] = (LunarAction)(rand() % LunarAction_MaxActions);
		}
	}

	RestartGA();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::RestartGA()
{
	if(mGeneration >= 500)
	{
		GenerateRandomChromosomes();
		mGeneration = 0;
		mAllowMutation = true;
		cout << endl << "---FAILED (Restart)---" << endl << endl;
	}

	for(int i = 0; i < kNumberOfAILanders; i++)
	{
		mFitnessValues[i] = 0.0f;
		mAILanders[i]->Reset();
	}

	mAccumulatedDeltaTime = 0;
	mCurrentAction		  = 0;
	//mPause				  = true;
	//cout << "Paused" << endl;
	mGeneration++;
	cout << "---GENERATION " << mGeneration << "---" << endl;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::SaveSolution()
{
	//Output the solution of the first chromosome - They should all match at the point we output so it
	//shouldn't matter which chromosome is used for the solution.
	ofstream outFile("Solution.txt");
	if(outFile.is_open())
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			switch(mChromosomes[0][action])
			{
				case LunarAction_None:
					outFile << "0";
				break;
			
				case LunarAction_LunarThrust:
					outFile << "1";
				break;

				case LunarAction_LunarLeft:
					outFile << "2";
				break;

				case LunarAction_LunarRight:
					outFile << "3";
				break;
			}
		
			outFile << ",";
		}

		outFile << endl << "Generations taken: " << mGeneration;
		outFile << endl << "Platform start (" << mPlatformPosition.x << "," << mPlatformPosition.y << ")";
		outFile << endl << "Ship start (" << mAILanders[0]->GetStartPosition().x << "," << mAILanders[0]->GetStartPosition().y << ")";

		outFile.close();
	}
}

//--------------------------------------------------------------------------------------------------
