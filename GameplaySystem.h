#pragma once

#pragma once
#include "cannon.h"
constexpr int BLAST_COST = 2;
constexpr int DAMAGE_BOOST_COST = 1;
constexpr int WIND_SHIELD_COST = 1;
constexpr int REPAIR_KIT_COST = 1;

constexpr int SIDEHIT_DAMAGE = 15;
constexpr int NORMAL_DAMAGE = 20;
constexpr int BOOST_DAMAGE = 30;
constexpr int REPAIR_AMOUNT = 40;

constexpr int BOOST_TURNS = 2;
constexpr int SHIELD_TURNS = 2;
enum class UpgradeType
{
	BlastRadius,
	DamageBoost,
	RepairKit,
	WindShield
};


struct Player
{
	int maxHealth = 100;
	int health = 100;
	int coins = 0;

	// position and firing state for integrated demo
	float posX = 0.0f;
	float posY = 0.0f;
	float angle = 45.0f; // relative 0..90
	float power = 300.0f;
	float explosionRadius = 30.0f;

	// tie visual cannon to gameplay player for integration
	Cannon cannon;

	// -----------------Upgrade effects--------------------------
	bool hasBlastUpgrade = false;
	int windShieldTurns = 0;
	int damageBoostTurns = 0;

	//-------------- Remaining purchases for this player----------------
	int blastRadiusLeft = 1;
	int damageBoostLeft = 2;
	int windShieldLeft = 2;
	int repairKitLeft = 2;
};

class GamePlaySystem
{
private:
	int currentTurn = 1;
public:
	void applyDamage(Player &player,int damage);
	void switchTurn();
	void awardCoin(Player &player);
	bool buyUpgrade(Player &player,UpgradeType upgradeType);
	int checkWinner(const Player &player1,const Player &player2);
	int getCurrentTurn() const;
};

