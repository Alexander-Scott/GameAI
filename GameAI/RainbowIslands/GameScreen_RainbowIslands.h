//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _GAMESCREEN_RAINBOW_H   
#define _GAMESCREEN_RAINBOW_H

#include "../GameScreen.h"
#include "Commons_RainbowIslands.h"
#include "../Commons.h"
#include <SDL.h>
#include <vector>

#include "NeuralNetwork.h"
#include "GeneticAlgorithm.h"

using namespace::std;

class Texture2D;
class Character;
class CharacterAI;
class CharacterFruit;
class CharacterRainbow;
class CharacterChest;
class LevelMap;

class GameScreen_RainbowIslands : GameScreen
{
//--------------------------------------------------------------------------------------------------
public:
	GameScreen_RainbowIslands(SDL_Renderer* renderer);
	~GameScreen_RainbowIslands();

	bool SetUpLevel();
	void Render();
	void RenderBackground();
	void Update(size_t deltaTime, SDL_Event e);

//--------------------------------------------------------------------------------------------------
private:
	void SetLevelMap();
	void RestartLevel();
	void CreateStartingCharacters();

	void MakeEnemiesAngry();
	void UpdateEnemies(size_t deltaTime, SDL_Event e);
	void CreateCaterpillar(Vector2D position, FACING direction);

	void UpdateRainbows(size_t deltaTime, SDL_Event e);
	void CreateRainbow(Vector2D position, int numberOfRainbows);

	void UpdateFruit(size_t deltaTime, SDL_Event e);
	void CreateFruit(Vector2D position, bool bounce);

	void CreateChest(Vector2D position);
	void TriggerChestSpawns();

	Texture2D*				  mBackgroundTexture;
	CharacterAI*			  mCharacter;
	bool					  mCanSpawnRainbow;
	vector<Character*>		  mEnemies;
	vector<CharacterFruit*>	  mFruit;
	vector<CharacterRainbow*> mRainbows;
	CharacterChest*			  mChest;
	LevelMap*				  mLevelMap;

	size_t					  mTimeToCompleteLevel;
	bool					  mTriggeredAnger;

	bool					  mTriggeredChestSpawns;

//--------------------------------------------------------------------------------------------------
private:
	void					  UpdateNeuralNetwork();

	CharacterFruit *		  FindClosestFruit();
	Character *				  FindClosestEnemy();	

	void					  SaveWeightsToFile();
	bool					  ReadWeightsFromFile();

	NeuralNetwork*			  mNeuralNetwork;
	GeneticAlgorithm*		  mGeneticAlgorithm;

	int						  mGenerationCount; // Count the number of generations that have occured

	vector<Genome>	          mCurrGenGenomes; // Stores the genomes/weights belonging to this generation
	vector<double>			  mCurrGenAvgFitness; // Stores the average fitness per generation
	vector<double>			  mCurrGenBestFitness; // Stores the best fitness per generation
	int						  mCurrGenGenomeIndex; // The current genome of this generation
	int						  mCurrGenUpdateTimer; // Timer that restarts level if the current generation has gone on for too long

	CharacterFruit*			  mClosestFruit;
	Character*				  mClosestEnemy;
	int						  mRainbowsFired;
};

#endif //_GAMESCREEN_RAINBOW_H