//Physics engine of cannonballgame


#include <iostream>
#include <cmath>
using namespace std;

struct Player{
    int hp;
    float cannonX;
};

struct Cannon{
    float angle;
    float power;
};

struct Cannonball{
    float xpos;
    float ypos;
    float horivelocity;
    float vertivelocity;
    bool active=false;
};

struct Environment{
    float gravity;
    float wind;
};



bool checkhit(Player& target,
              Cannonball& ball)
{
    if(ball.xpos >= target.cannonX - 10 &&
       ball.xpos <= target.cannonX + 10 &&
       ball.ypos <= 5)
    {
        return true;
    }

    return false;
}



void fireProjectile(Player& shooter,Cannon& cannonData,Cannonball& ball,bool playerTurn){
float radians=cannonData.angle*(3.14159/180);
ball.xpos=shooter.cannonX;
ball.ypos=0;
ball.vertivelocity=cannonData.power*sin(radians);
if(playerTurn){
        ball.horivelocity=cannonData.power*cos(radians);
    }
    else{
        ball.horivelocity=-cannonData.power*cos(radians);
    }
    ball.active = true;
}



void updateProjectile(Cannonball& ball,Environment& effects,float deltaTime){
    ball.xpos+=ball.horivelocity*deltaTime;
    ball.ypos+=ball.vertivelocity*deltaTime;
    ball.vertivelocity-=effects.gravity*deltaTime;
    ball.horivelocity+=effects.wind*deltaTime;
    if(ball.ypos<=0){
        ball.ypos=0;
        ball.active=false;
    }
}



void applyDamage(Player& target){
    target.hp--;
}



int main(){
    Player player1;
    Player player2;

    player1.hp = 3;
    player2.hp = 3;

    player1.cannonX = 0;
    player2.cannonX = 100;

    Cannon cannonData;

    Cannonball ball;

    Environment effects;

    effects.gravity = 9.8;
    effects.wind = 0.2;

    bool playerTurn = true;

    while(true)
    {
        Player& shooter =
        playerTurn ? player1 : player2;

        Player& target =
        playerTurn ? player2 : player1;

        cout << "\n====================\n";

        if(playerTurn)
        {
            cout << "PLAYER 1 TURN\n";
        }
        else
        {
            cout << "PLAYER 2 TURN\n";
        }

        cout << "Current Wind: ";
        cout << effects.wind << "\n";

        cout << "Enter Power: ";
        cin >> cannonData.power;

        cout << "Enter Angle: ";
        cin >> cannonData.angle;

        fireProjectile(shooter,
                       cannonData,
                       ball,
                       playerTurn);

        bool hit = false;

        float deltaTime = 0.1;

        while(ball.active)
        {
            updateProjectile(ball,
                             effects,
                             deltaTime);

            cout << "X: ";
            cout << ball.xpos;

            cout << "\nY: ";
            cout << ball.ypos;

            cout << "\n\n";

            if(checkhit(target, ball))
            {
                cout << "\nDIRECT HIT!\n";

                applyDamage(target);

                hit = true;

                ball.active = false;
            }
        }

        if(!hit)
        {
            cout << "\nMissed Hit!\n";
        }

        cout << "\nPlayer 1 HP: ";
        cout << player1.hp;

        cout << "\nPlayer 2 HP: ";
        cout << player2.hp;

        cout << "\n";

        if(player1.hp <= 0)
        {
            cout << "\nPLAYER 2 WINS!\n";
            break;
        }

        if(player2.hp <= 0)
        {
            cout << "\nPLAYER 1 WINS!\n";
            break;
        }

        playerTurn = !playerTurn;
    }

    return 0;
}