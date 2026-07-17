/*
//Function to display player's name, HP and coins
void printPlayer(Player p)
{
	cout << "Name: " << p.name << " HP: " << p.health << " Coins: " << p.coins<< endl;
}


//Function to calculate consecutive hits, decrease health upon damage and display if player has been eliminated
bool applyDamage(Player& p, int damage)
{
	//To randomize if its direct hit or not until the files are connected with one another
	bool isDirect = (rand() % 2 == 0);

	if (isDirect) 
	{
		p.coins++;
		cout << p.name << " has recieved a coin!" << endl;
	}
	p.health -= damage;
	if (p.health <= 0)  // To check whether the player is alive if not and if not display eliminated on the screen 
	{
		cout << p.name << " has been eliminated" << endl;
		return true;
	}
	return false;
}


//Function to switch turn from player1 to player 2 and vice versa
void switchTurn(int &currentTurn)
{
	if (currentTurn == 1) currentTurn = 2;
	else currentTurn = 1;
}
*/

#include "GameplaySystem.h"
#include <iostream>
#include <cstdlib>
using namespace std;

void GamePlaySystem::applyDamage(Player& player, int damage)
{
	player.health -= damage;
	if (player.health < 0)
	{
		player.health = 0;
	}
	// keep visual cannon health in sync
	player.cannon.health = player.health;
}


void GamePlaySystem::awardCoin(Player& player)
{
	
		player.coins++;
}

void GamePlaySystem::switchTurn()
{
	if (currentTurn == 1)
	{
		currentTurn = 2;
	}
	else
	{
		currentTurn =1;
	}
}
int GamePlaySystem::getCurrentTurn() const 
{
	return currentTurn;
}


bool GamePlaySystem::buyUpgrade(Player &player,UpgradeType upgradeType) 
{
	switch (upgradeType)
	{
	case UpgradeType::BlastRadius:
		if (player.coins < BLAST_COST)
		{
			return false;
		}
		if (player.blastRadiusLeft <= 0)
		{
			return false;
		}

		player.blastRadiusLeft--;
		// increase the player's explosion radius by 1.5x so blast upgrades affect explosions
		player.explosionRadius = player.explosionRadius * 1.5f;
		player.hasBlastUpgrade = true;
		player.coins -= BLAST_COST;
		return true;

	case UpgradeType::DamageBoost:
		if (player.coins < DAMAGE_BOOST_COST)
		{
			return false;
		}
		if (player.damageBoostLeft <= 0)
		{
			return false;
		}

		player.damageBoostLeft--;
		player.damageBoostTurns = BOOST_TURNS;
		player.coins -= DAMAGE_BOOST_COST;
		return true;

	case UpgradeType::WindShield:
		if (player.coins < WIND_SHIELD_COST)
		{
			return false;
		}
		if (player.windShieldLeft <= 0)
		{
			return false;
		}

		player.windShieldLeft--;
		player.windShieldTurns = SHIELD_TURNS;
		player.coins -= WIND_SHIELD_COST;
		return true;

	case UpgradeType::RepairKit:
		if (player.health == player.maxHealth)
		{
			return false;
		}
		if (player.coins < REPAIR_KIT_COST)
		{
			return false;
		}
		if (player.repairKitLeft <= 0)
		{
			return false;
		}

		player.repairKitLeft--;
		player.health += REPAIR_AMOUNT;
		if (player.health > player.maxHealth)
		{
			player.health = player.maxHealth;
		}
		// sync cannon visual health as well
		player.cannon.health = player.health;
		player.coins -= REPAIR_KIT_COST;
		return true;
	default:
			return false;
	}

}

int GamePlaySystem::checkWinner(const Player& player1, const Player& player2)
{
	if (player1.health == 0)
	{
		return 2;
	}
	else if (player2.health == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

