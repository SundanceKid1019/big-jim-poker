#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cmath>
#include <list>
#include <random>

using namespace std;

//Struct for a given hand, holds info about the hand
typedef struct Hand {
	int myCards[2][2];
	int flop[3][2];
	int turn[1][2];
	int river[1][2];
	int position,callSize,potSize,stage;
	bool aggressor;
	int oppRange[1326][4];
	int myRange[1326][4];
	//Constructor, sets defaults for beginning of a hand
	Hand(int small=0,int big=0) {
		int deck[52][2] = {
			{2,1},{3,1},{4,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},{11,1},{12,1},{13,1},{14,1},
			{2,2},{3,2},{4,2},{5,2},{6,2},{7,2},{8,2},{9,2},{10,2},{11,2},{12,2},{13,2},{14,2},
			{2,3},{3,3},{4,3},{5,3},{6,3},{7,3},{8,3},{9,3},{10,3},{11,3},{12,3},{13,3},{14,3},
			{2,4},{3,4},{4,4},{5,4},{6,4},{7,4},{8,4},{9,4},{10,4},{11,4},{12,4},{13,4},{14,4},
		};
		stage=0;
		potSize=big+small;
		callSize=big-small;
		int n=52,r=2;
		vector<bool> v(n);
		int j=0;
		int k=0;
	    fill(v.end() - r, v.end(), true);
	    do {
	        for (int i=0;i<n;i++) {
	            if (v[i]) {
	            	oppRange[j][k]=deck[i][0];
	            	oppRange[j][k+1]=deck[i][1];
	            	myRange[j][k]=deck[i][0];
	            	myRange[j][k+1]=deck[i][1];
	            	k+=2;
	            }
	        }
	        j++;
	        k=0;
	    } while (next_permutation(v.begin(), v.end()));
	    for(int i=0;i<6;i++) {
	    	flop[i/2][i%2]=0;
	    }for(int i=0;i<2;i++) {
	    	turn[i/2][i%2]=0;
	    }for(int i=0;i<2;i++) {
	    	river[i/2][i%2]=0;
	    }
	}
} Hand;

//Method Headers/Prototypes and Descriptions:

//updateRange: removes the every hand in range1 that has an equity of <e against range2
void updateRange(float e, int (*range1)[1326][4], int (*range2)[1326][4], int deck[52][2],Hand& hand);
//updateRange2: removes every hand in range1 that has an equity of >e against range2
void updateRange2(float e, int (*range1)[1326][4], int (*range2)[1326][4], int deck[52][2],Hand& hand);
//foldEquity: calculates the percentage of hands in range1 that have an equity of <e against range2
float foldEquity(float e, int (*range1)[1326][4], int (*range2)[1326][4], int deck[52][2],Hand& hand);
//potential: returns whether or not the hand has potential, ie it can hit a flush or straight on a later street
bool potential(int hand[2][2],Hand& h);
//charToInt: used for converting user input into readable card values
int charToInt(char c);
//resetRange: resets range of hands to starting range, used as safety for when a range is narrowed to 0 hands
void resetRange(int (*range)[1326][4],int deck[52][2]);
//strengthOfHand: returns a long long (the superior variable type) representing a 5 card hand's strength from Straight flush to high card
long long int strengthOfHand(int hand[5][2]);
//action: called when it's Jim's turn to act, makes Jim perform an action and print to console
void action(int deck[52][2],Hand& hand,int *myStack,int *oppStack,int big);
//simHands: simulates a hand against a range a given amount of times to find the hand's winrate, can also find a range's winrate against another range
float simHands(int myCards[52][2],int n,int deck[n][2],int trials,Hand& hand,int (*o)[1326][4],int (*m)[1326][4],bool a);
//Fold: action function for when Jim folds
void Fold(int *myStack,int *oppStack,Hand& hand);
//Check: for when Jim checks
void Check(int *myStack,int *oppStack,Hand& hand,int deck[52][2]);
//Call: for when Jim calls
void Call(int *myStack,int *oppStack,Hand& hand,int deck[52][2]);
//Raise: for when Jim raises
void Raise(int *myStack,int *oppStack,Hand& hand,int value,int deck[52][2]);


//main method for user commands and main management of game
int main() {

	//declare and initialize variables for the game

	int deck[52][2] = {
		{2,1},{3,1},{4,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},{11,1},{12,1},{13,1},{14,1},
		{2,2},{3,2},{4,2},{5,2},{6,2},{7,2},{8,2},{9,2},{10,2},{11,2},{12,2},{13,2},{14,2},
		{2,3},{3,3},{4,3},{5,3},{6,3},{7,3},{8,3},{9,3},{10,3},{11,3},{12,3},{13,3},{14,3},
		{2,4},{3,4},{4,4},{5,4},{6,4},{7,4},{8,4},{9,4},{10,4},{11,4},{12,4},{13,4},{14,4},
	};

	int stackSize;
	int small,big;
	string command;
	/*
	Stage:
	0 - Preflop
	1 - Flop
	2 - Turn
	3 - River
	*/

	//Get key information from user:
	cout<<"What are the stack sizes?\n";
	cin>>stackSize;
	int myStack=stackSize,oppStack=stackSize;
	cout<<"Small and big blind?\n";
	cin>>small>>big;

	//loop for while game is in progress
	bool playing=true;
	while(playing) {

		//declare hand variable for each hand
		Hand hand = Hand(small,big);

		//Display information to user:
		cout<<"Me: "<<myStack<<"     "<<"Him: "<<oppStack<<endl;
		cout<<"Pot: "<<hand.potSize<<"     "<<hand.callSize<<" to call\n";
		//Ask user for position, where 0 means Jim is small blind and 1 means Jim is big blind
		cout<<"What's my position?\n";
		cin>>hand.position;
		//Set stacks according to who is which blind
		if(hand.position==0) {
			myStack-=small;
			oppStack-=big;
		} else {
			myStack-=big;
			oppStack-=small;
		}
		//Get Jim's starting hand
		cout<<"What's my hand?\n";
		char info[4] = {' ',' ',' ',' '};
		cin>>info[0]>>info[1]>>info[2]>>info[3];
		for(int i=0;i<4;i++) {
			hand.myCards[i/2][i%2]=charToInt(info[i]);
		}

		char hold[6];
		//loop for while hand is taking place:
		bool handInProgress=true;
		while(handInProgress) {
			//loop to make sure user enters valid command
			bool validCommand=false;
			while(!validCommand) {
				validCommand=true;
				//display info to user and ask for command
				cout<<"Me: "<<myStack<<"     "<<"Him: "<<oppStack<<endl;
				cout<<"Pot: "<<hand.potSize<<"     "<<hand.callSize<<" to call\n";
				cout<<"Now what?\n";
				cin>>command;
				//flop command - for inputing the flop cards
				if (command=="flop") {
					cin>>hold[0]>>hold[1]>>hold[2]>>hold[3]>>hold[4]>>hold[5];
					for(int i=0;i<6;i++) {
						hand.flop[i/2][i%2]=charToInt(hold[i]);
					}
					hand.stage=1;
				//turn command - for inputing the turn card
				} else if (command=="turn") {
					cin>>hold[0]>>hold[1];
					for(int i=0;i<2;i++) {
						hand.turn[i/2][i%2]=charToInt(hold[i]);
					}
					hand.stage=2;
				//river command - for inputing the river card
				} else if (command=="river") {
					cin>>hold[0]>>hold[1];
					for(int i=0;i<2;i++) {
						hand.river[i/2][i%2]=charToInt(hold[i]);
					}
					hand.stage=3;
				//fold command - Jim's opponent folds
				} else if (command=="fold") {
					myStack+=hand.potSize;
					hand.aggressor=false;
					hand.potSize=0;
					hand.callSize=0;
					hand.stage=0;
					handInProgress=false;
				//check command - Jim's opponent checks
				} else if (command=="check") {
					hand.callSize=0;
					float equity=.8;
					updateRange2(equity,&hand.oppRange,&hand.myRange,deck,hand);
				//call command - Jim's opponent calls
				} else if (command=="call") {
					oppStack-=hand.callSize;
					hand.potSize+=hand.callSize;
					float equity=(1.0*hand.callSize)/(1.0*hand.potSize);
					updateRange(equity,&hand.oppRange,&hand.myRange,deck,hand);
					hand.callSize=0;
					hand.stage++;
				//raise command - Jim's opponent raises by a given value
				} else if (command=="raise") {
					hand.potSize+=hand.callSize;
					oppStack-=hand.callSize;
					int pre=hand.callSize;
					cin>>hand.callSize;
					if(hand.callSize>myStack) {
						hand.callSize=myStack;
					}
					if(hand.callSize>oppStack) {
						hand.callSize=oppStack;
					}
					hand.potSize+=hand.callSize;
					oppStack-=hand.callSize;
					// float equity=(1.0*pre+callSize)/(1.0*potSize);
					// updateRange(equity,&myRange,&oppRange,stage,myCards,deck,flop,turn,river);
					float e=(1.0*hand.callSize)/(1.0*hand.potSize+hand.callSize);
					float foldEq=foldEquity(e,&hand.oppRange,&hand.myRange,deck,hand);
					float equity=foldEq+(1-foldEq)*(e);
					updateRange(equity,&hand.myRange,&hand.oppRange,deck,hand);
					action(deck,hand,&myStack,&oppStack,big);
				//win command - Jim wins the pot
				} else if (command=="win") {
					myStack+=hand.potSize;
					hand.potSize=0;
					hand.callSize=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//lose command - Jim loses the pot
				} else if (command=="lose") {
					oppStack+=hand.potSize;
					hand.potSize=0;
					hand.callSize=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//act command - tells Jim to take his turn and perform action
				} else if (command=="act") {
					action(deck,hand,&myStack,&oppStack,big);
				//reset command - Resets stack sizes in case of user mistake
				} else if (command=="reset") {
					cin>>myStack>>oppStack;
					hand.potSize=0;
					hand.callSize=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//blinds command - sets new blinds
				} else if (command=="blinds") {
					cin>>small>>big;
					hand.potSize=0;
					hand.callSize=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//range command - Jim displays the range of hands he puts his opponent on
				} else if (command=="range") {
					for(int i=0;i<1326;i++) {
						if(hand.oppRange[i][0]!=0) {
							cout<<hand.oppRange[i][0]<<" "<<hand.oppRange[i][1]<<"   "<<hand.oppRange[i][2]<<" "<<hand.oppRange[i][3]<<endl;
						}
					}
				//incase user inputs invalid command:
				} else {
					validCommand=false;
					cout<<"Invalid command... \n";
				}

			}
		}
	}

}

void updateRange(float e, int (*range1)[1326][4], int (*range2)[1326][4], int deck[52][2], Hand& hand) {
	int tempRange[1326][4];
	//multiply equity by 100 to compare to winrate percentage which is already multiplied by 100
	float equity = 100*e;
	//cout<<"eq:"<<equity<<endl;
	//fill tempRange with range1 values
	for(int i=0;i<1326;i++) {
		tempRange[i][0]=(*range1)[i][0];
		tempRange[i][1]=(*range1)[i][1];
		tempRange[i][2]=(*range1)[i][2];
		tempRange[i][3]=(*range1)[i][3];
	} 
	int cards[2][2];
	for(int i=0;i<1326;i++) {
		cards[0][0]=(*range1)[i][0];
		cards[0][1]=(*range1)[i][1];
		cards[1][0]=(*range1)[i][2];
		cards[1][1]=(*range1)[i][3];
		//loop for each hand in range1, calculate winrate against range2, and if winrate is <equity AND if hand has no potential, eliminate hand from range1 (set to 0)
		if(cards[0][0]!=0) {
			int winRate=simHands(cards,52,deck,100,hand,range2,&tempRange,false);
			if(winRate<equity && !potential(cards,hand)) {
				//cout<<winRate<<"<"<<equity<<"Eliminated "<<(*range1)[i][0]<<(*range1)[i][1]<<" "<<(*range1)[i][2]<<(*range1)[i][3]<<endl;
				for(int j=0;j<4;j++) {
					(*range1)[i][j]=0;
				}
			} 
		}
	}
}

void updateRange2(float e, int (*range1)[1326][4], int (*range2)[1326][4], int deck[52][2], Hand& hand) {
	int tempRange[1326][4];
	float equity = 100*e;
	//cout<<"eq:"<<equity<<endl;
	for(int i=0;i<1326;i++) {
		tempRange[i][0]=(*range1)[i][0];
		tempRange[i][1]=(*range1)[i][1];
		tempRange[i][2]=(*range1)[i][2];
		tempRange[i][3]=(*range1)[i][3];
	} 
	int cards[2][2];
	for(int i=0;i<1326;i++) {
		cards[0][0]=(*range1)[i][0];
		cards[0][1]=(*range1)[i][1];
		cards[1][0]=(*range1)[i][2];
		cards[1][1]=(*range1)[i][3];
		//loop for each hand in range1, calculate winrate against range2, if winrate is >equity, remove hand from range1 (set to 0)
		if(cards[0][0]!=0) {
			int winRate=simHands(cards,52,deck,100,hand,range2,&tempRange,false);
			if(winRate>equity) {
				//cout<<winRate<<"<"<<equity<<"Eliminated "<<(*range1)[i][0]<<(*range1)[i][1]<<" "<<(*range1)[i][2]<<(*range1)[i][3]<<endl;
				for(int j=0;j<4;j++) {
					(*range1)[i][j]=0;
				}
			} 
		}
	}
}

bool potential(int cards[2][2],Hand& hand) {
	int suits[4]={0};
	int sorted[5][2]={{cards[0][0],cards[0][1]},{cards[1][0],cards[1][1]},{hand.flop[0][0],hand.flop[0][1]},{hand.flop[1][0],hand.flop[1][1]},{hand.flop[2][0],hand.flop[2][1]}};
	suits[cards[0][1]]++;
	suits[cards[0][1]]++;
	suits[hand.flop[0][1]]++;
	suits[hand.flop[1][1]]++;
	suits[hand.flop[2][1]]++;
	//counts number of each suit and if hand has 4 of one suit after the flop, return true
	for(int i=0;i<4;i++) {
		if(suits[i]==4) {
			return true;
		}
	}
	//sort the hand to look for straight potential
	int key1,key2,j;
	for(int i=1;i<5;i++) {
		key1=sorted[i][0];
		key2=sorted[i][1];
		j=i-1;
		while(j>=0 && sorted[j][0]<key1) {
			sorted[j+1][0]=sorted[j][0];
			sorted[j+1][1]=sorted[j][1];
			j--;
		}
		sorted[j+1][0]=key1;
		sorted[j+1][1]=key2;
	}
	//return true if hand has open ended straight potential
	if(sorted[4][0]-sorted[1][0]==3 && sorted[4][0]!=sorted[1][0] && sorted[4][0]!=sorted[2][0] && sorted[4][0]!=sorted[3][0]) {
		return true;
	} else if(sorted[3][0]-sorted[0][0]==3 && sorted[0][0]!=sorted[1][0] && sorted[0][0]!=sorted[2][0] && sorted[0][0]!=sorted[3][0]) {
		return true;
	} //missing case where 4 cards in a row and 5th is a mid-pair
	return false;
}

float foldEquity(float e, int (*range1)[1326][4],int(*range2)[1326][4],int deck[52][2],Hand& hand) {
	int tempRange[1326][4];
	float equity = 100*e;
	//cout<<"eq:"<<equity<<endl;
	for(int i=0;i<1326;i++) {
		tempRange[i][0]=(*range1)[i][0];
		tempRange[i][1]=(*range1)[i][1];
		tempRange[i][2]=(*range1)[i][2];
		tempRange[i][3]=(*range1)[i][3];
	} 
	int cards[2][2];
	float folds=0;
	float trials;
	for(int i=0;i<1326;i++) {
		cards[0][0]=(*range1)[i][0];
		cards[0][1]=(*range1)[i][1];
		cards[1][0]=(*range1)[i][2];
		cards[1][1]=(*range1)[i][3];
		//loop for each hand in range1, tests winrate against range2 and counts the number of hands that are <equity
		if(cards[0][0]!=0) {
			int winRate=simHands(cards,52,deck,100,hand,range2,&tempRange,false);
			if(winRate<equity) {
				folds++;
			} 
			trials++;
		}
	}
	//returns percentage of hands folded
	return folds/trials;
}

void resetRange(int (*range)[1326][4],int deck[52][2]) {
	int n=52,r=2;
	vector<bool> v(n);
	int j=0;
	int k=0;
	//refills the given range with every possible hand in the deck
    fill(v.end() - r, v.end(), true);
    do {
        for (int i=0;i<n;i++) {
            if (v[i]) {
            	(*range)[j][k]=deck[i][0];
            	(*range)[j][k+1]=deck[i][1];
            	k+=2;
            }
        }
        j++;
        k=0;
    } while (next_permutation(v.begin(), v.end()));
}

int charToInt(char c) {
	switch(c) {
		case 'A':
		case 'a':
			return 14;
			break;
		case 'K':
		case 'k':
			return 13;
			break;
		case 'Q':
		case 'q':
			return 12;
			break;
		case 'J':
		case 'j':
			return 11;
			break;
		case 'T':
		case 't':
			return 10;
			break;
		case 'C':
		case 'c':
			return 1;
			break;
		case 'D':
		case 'd':
			return 2;
			break;
		case 'H':
		case 'h':
			return 3;
			break;
		case 'S':
		case 's':
			return 4;
			break;
		default:
			return c-'0';
			break;
	}
}

	/*
	Straight Flush		90,000,000,000
	Four of a Kind		8
	Full House			7
	Flush				6
	Straight			5
	Three of a Kind		4
	Two Pair			3
	Pair				2
	High Card 			1
	*/

long long int strengthOfHand(int cards[5][2]) {

	int key1,key2,j;
	for(int i=1;i<5;i++) {
		key1=cards[i][0];
		key2=cards[i][1];
		j=i-1;
		while(j>=0 && cards[j][0]<key1) {
			cards[j+1][0]=cards[j][0];
			cards[j+1][1]=cards[j][1];
			j--;
		}
		cards[j+1][0]=key1;
		cards[j+1][1]=key2;
	}

	//Four of a Kind
	if (cards[0][0]==cards[1][0] && cards[0][0]==cards[2][0] && cards[0][0]==cards[3][0]) {
		return 80000000000 + cards[4][0];
	} else if (cards[4][0]==cards[1][0] && cards[4][0]==cards[2][0] && cards[4][0]==cards[3][0]) {
		return 80000000000 + cards[0][0];
	}

	//Full House
	if (cards[0][0]==cards[1][0] && cards[0][0]==cards[2][0] && cards[3][0]==cards[4][0]) {
		return 70000000000 + cards[0][0] * 1000000 + cards[4][0] * 10000;
	} else if (cards[0][0]==cards[1][0] && cards[2][0]==cards[3][0] && cards[2][0]==cards[4][0]) {
		return 70000000000 + cards[4][0] * 1000000 + cards[0][0] * 10000;
	}

	bool flush=false;
	//Flush
	if (cards[0][1]==cards[1][1] && cards[0][1]==cards[2][1] && cards[0][1]==cards[3][1] && cards[0][1]==cards[4][1]) {
		flush=true;
	}

	//Straight and Straight Flush
	if (cards[0][0]-cards[4][0]==4 && cards[0][0]!=cards[1][0] && cards[1][0]!=cards[2][0] && cards[2][0]!=cards[3][0] && cards[3][0]!=cards[4][0]) {
		if(flush) {
			return 90000000000 + cards[0][0];
		} else {
			return 50000000000 + cards[0][0];
		}
	} else if (cards[0][0]==14 && cards[1][0]==5 && cards[2][0]==3 && cards[3][0]==3 && cards[4][0]==2) {
		if(flush) {
			return 90000000000 + cards[1][0];
		} else {
			return 50000000000 + cards[1][0];
		}
	} else if (flush) {
		return 60000000000 + cards[0][0] * 100000000 + cards[1][0] * 1000000 + cards[2][0] * 10000 + cards[3][0] * 100 + cards[4][0];
	}

	//Three of a Kind
	if (cards[0][0]==cards[1][0] && cards[0][0]==cards[2][0]) {
		return 40000000000 + cards[0][0]*100000000 + cards[3][0]*1000000 + cards[4][0]*10000;
	} else if (cards[1][0]==cards[2][0] && cards[1][0]==cards[3][0]) {
		return 40000000000 + cards[1][0]*100000000 + cards[0][0]*1000000 + cards[4][0]*10000;
	} else if (cards[2][0]==cards[3][0] && cards[2][0]==cards[4][0]) {
		return 40000000000 + cards[2][0]*100000000 + cards[0][0]*1000000 + cards[1][0]*10000;
	}

	//Two Pair
	if (cards[0][0]==cards[1][0] && cards[2][0]==cards[3][0]) {
		return 30000000000 + cards[0][0]*100000000 + cards[2][0]*1000000 + cards[4][0]*10000;
	} else if (cards[0][0]==cards[1][0] && cards[3][0]==cards[4][0]) {
		return 30000000000 + cards[0][0]*100000000 + cards[3][0]*1000000 + cards[2][0]*10000;
	} else if (cards[1][0]==cards[2][0] && cards[3][0]==cards[4][0]) {
		return 30000000000 + cards[1][0]*100000000 + cards[3][0]*1000000 + cards[0][0]*10000;
	}

	//Pair
	if (cards[0][0]==cards[1][0]) {
		return 20000000000 + cards[0][0]*100000000 + cards[2][0]*1000000 + cards[3][0]*10000 + cards[4][0]*100;
	} else if (cards[1][0]==cards[2][0]) {
		return 20000000000 + cards[1][0]*100000000 + cards[0][0]*1000000 + cards[3][0]*10000 + cards[4][0]*100;
	} else if (cards[2][0]==cards[3][0]) {
		return 20000000000 + cards[2][0]*100000000 + cards[0][0]*1000000 + cards[1][0]*10000 + cards[4][0]*100;
	} else if (cards[3][0]==cards[4][0]) {
		return 20000000000 + cards[3][0]*100000000 + cards[0][0]*1000000 + cards[1][0]*10000 + cards[2][0]*100;
	}

	//High Card
	return 10000000000 + cards[0][0]*100000000 + cards[1][0]*1000000 + cards[2][0]*10000 + cards[3][0]*100 + cards[4][0];
}

void action(int deck[52][2],Hand& hand,int *myStack,int *oppStack,int big) {
	int preflopDeck[50][2];
	int flopDeck[47][2];
	int turnDeck[46][2];
	int riverDeck[45][2];

	//long series of if statements to determine whether to fold, check, call, or raise, with a mix randomness and logic 
	if (hand.stage==0) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1]))) {
				preflopDeck[j][0]=deck[i][0];
				preflopDeck[j][1]=deck[i][1];
				j++;
			}
		}
		
		float winRate = simHands(hand.myCards,50,preflopDeck,10000,hand,&hand.oppRange,&hand.myRange,false);
		cout<<winRate<<endl;

		if(hand.position==0) {
			default_random_engine generator;
			uniform_int_distribution<int> distribution(0,20);
			int callUpTo=30+distribution(generator);
			int raiseAfter=70+distribution(generator);
			if(hand.callSize==big/2) {
				if(winRate<40) {
					Fold(myStack,oppStack,hand);
					return;
				} else if (winRate<callUpTo) {
					Call(myStack,oppStack,hand,deck);
					return;
				} else {
					Raise(myStack,oppStack,hand,2*big,deck);
					return;
				}
			} else {
				if(winRate>raiseAfter) {
					Raise(myStack,oppStack,hand,3*hand.callSize,deck);
					return;
				} else if ((hand.callSize<=hand.potSize-hand.callSize && winRate>100*hand.callSize/(hand.potSize+hand.callSize)) || winRate>60) {
					Call(myStack,oppStack,hand,deck);
					return;
				} else {
					Fold(myStack,oppStack,hand);
					return;
				}
			}
		} else {
			default_random_engine generator;
			uniform_int_distribution<int> distribution(0,15);
			int checkUpTo=60+distribution(generator);
			int raiseAfter=70+distribution(generator);
			if(hand.callSize==0) {
				if(winRate<checkUpTo) {
					Check(myStack,oppStack,hand,deck);
					return;
				} else {
					Raise(myStack,oppStack,hand,3*big,deck);
					return;
				}
			} else {
				if(winRate>raiseAfter) {
					Raise(myStack,oppStack,hand,3*hand.callSize,deck);
					return;
				} else if((hand.callSize<=hand.potSize-hand.callSize && winRate>100*hand.callSize/(hand.potSize+hand.callSize)) || winRate>60) {
					Call(myStack,oppStack,hand,deck);
					return;
				} else {
					Fold(myStack,oppStack,hand);
					return;
				}
			}
		} 

	} else if(hand.stage==1) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1])) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1])) {
				flopDeck[j][0]=deck[i][0];
				flopDeck[j][1]=deck[i][1];
				j++;
			}
		}

		float winRate = simHands(hand.myCards,47,flopDeck,10000,hand,&hand.oppRange,&hand.myRange,false);
		cout<<winRate<<endl;

		default_random_engine generator;
		uniform_int_distribution<int> distribution(0,15);
		if(hand.callSize==0) {
			if(hand.aggressor) {
				int checkUpTo=35+distribution(generator);
				int thirdPot=60+distribution(generator);
				int halfPot=thirdPot+20;
				if(winRate<checkUpTo) {
					default_random_engine gen;
					uniform_int_distribution<int> distr(-1,3);
					int bluff = distr(gen);
					if(bluff<=0) {
						Check(myStack,oppStack,hand,deck);
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/bluff),deck);
					}
					return;
				} else {
					if(winRate<thirdPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/3.0),deck);
						return;
					} else if (winRate<halfPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/2.0),deck);
						return;
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)),deck);
						return;
					}
				}
			} else {
				int checkUpTo=50+distribution(generator);
				int thirdPot=65+distribution(generator);
				int halfPot=thirdPot+20;
				if(winRate<checkUpTo) {
					default_random_engine gen;
					uniform_int_distribution<int> distr(0,5);
					uniform_int_distribution<int> distri(1,3);
					int bluff = distr(gen);
					int size = distri(gen);
					if(bluff==0) {
						Check(myStack,oppStack,hand,deck);
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/size),deck);
					}
					return;
				} else {
					if(winRate<thirdPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/3.0),deck);
						return;
					} else if (winRate<halfPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/2.0),deck);
						return;
					} else {
						default_random_engine gen;
						uniform_int_distribution<int> distr(0,1);
						int rand=distr(gen);
						if(rand==0) {
							Raise(myStack,oppStack,hand,(int)((hand.potSize)),deck);
						} else {
							Check(myStack,oppStack,hand,deck);
						}
						return;
					}
				}
			}
		} else {
			int raiseAfter=70+distribution(generator);
			if(winRate>raiseAfter) {
				Raise(myStack,oppStack,hand,3*hand.callSize,deck);
				return;
			} else if ((hand.callSize<=hand.potSize-hand.callSize && winRate>100*hand.callSize/(hand.potSize+hand.callSize)) || winRate>75) {
				Call(myStack,oppStack,hand,deck);
				return;
			} else {
				Fold(myStack,oppStack,hand);
				return;
			}
		}
	} else if (hand.stage==2) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1])) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.turn[1][0] && deck[i][1]==hand.turn[1][1])) {
				turnDeck[j][0]=deck[i][0];
				turnDeck[j][1]=deck[i][1];
				j++;
			}
		}

		float winRate = simHands(hand.myCards,46,turnDeck,10000,hand,&hand.oppRange,&hand.myRange,false);
		cout<<winRate<<endl;

		default_random_engine generator;
		uniform_int_distribution<int> distribution(0,15);
		if(hand.callSize==0) {
			if(hand.aggressor) {
				int checkUpTo=35+distribution(generator);
				int thirdPot=60+distribution(generator);
				int halfPot=thirdPot+20;
				if(winRate<checkUpTo) {
					default_random_engine gen;
					uniform_int_distribution<int> distr(-1,3);
					int bluff = distr(gen);
					if(bluff<=0) {
						Check(myStack,oppStack,hand,deck);
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/bluff),deck);
					}
					return;
				} else {
					if(winRate<thirdPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/3.0),deck);
						return;
					} else if (winRate<halfPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/2.0),deck);
						return;
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)),deck);
						return;
					}
				}
			} else {
				int checkUpTo=50+distribution(generator);
				int thirdPot=65+distribution(generator);
				int halfPot=thirdPot+20;
				if(winRate<checkUpTo) {
					default_random_engine gen;
					uniform_int_distribution<int> distr(0,5);
					uniform_int_distribution<int> distri(1,3);
					int bluff = distr(gen);
					int size = distri(gen);
					if(bluff==0) {
						Check(myStack,oppStack,hand,deck);
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/size),deck);
					}
					return;
				} else {
					if(winRate<thirdPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/3.0),deck);
						return;
					} else if (winRate<halfPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/2.0),deck);
						return;
					} else {
						default_random_engine gen;
						uniform_int_distribution<int> distr(0,1);
						int rand=distr(gen);
						if(rand==0) {
							Raise(myStack,oppStack,hand,(int)((hand.potSize)),deck);
						} else {
							Check(myStack,oppStack,hand,deck);
						}
						return;
					}
				}
			}
		} else {
			int raiseAfter=70+distribution(generator);
			if(winRate>raiseAfter) {
				Raise(myStack,oppStack,hand,3*hand.callSize,deck);
				return;
			} else if ((hand.callSize<=hand.potSize-hand.callSize && winRate>100*hand.callSize/(hand.potSize+hand.callSize)) || winRate>75) {
				Call(myStack,oppStack,hand,deck);
				return;
			} else {
				Fold(myStack,oppStack,hand);
				return;
			}
		}

	} else if (hand.stage==3) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1])) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.turn[1][0] && deck[i][1]==hand.turn[1][1]) || (deck[i][0]==hand.river[1][0] && deck[i][1]==hand.river[1][1])) {
				turnDeck[j][0]=deck[i][0];
				turnDeck[j][1]=deck[i][1];
				j++;
			}
		}

		float winRate = simHands(hand.myCards,45,turnDeck,10000,hand,&hand.oppRange,&hand.myRange,false);
		cout<<winRate<<endl;

		default_random_engine generator;
		uniform_int_distribution<int> distribution(0,15);
		if(hand.callSize==0) {
			if(hand.aggressor) {
				int checkUpTo=35+distribution(generator);
				int thirdPot=60+distribution(generator);
				int halfPot=thirdPot+20;
				if(winRate<checkUpTo) {
					default_random_engine gen;
					uniform_int_distribution<int> distr(-1,3);
					int bluff = distr(gen);
					if(bluff<=0) {
						Check(myStack,oppStack,hand,deck);
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/bluff),deck);
					}
					return;
				} else {
					if(winRate<thirdPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/3.0),deck);
						return;
					} else if (winRate<halfPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/2.0),deck);
						return;
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)),deck);
						return;
					}
				}
			} else {
				int checkUpTo=50+distribution(generator);
				int thirdPot=65+distribution(generator);
				int halfPot=thirdPot+20;
				if(winRate<checkUpTo) {
					default_random_engine gen;
					uniform_int_distribution<int> distr(0,5);
					uniform_int_distribution<int> distri(1,3);
					int bluff = distr(gen);
					int size = distri(gen);
					if(bluff==0) {
						Check(myStack,oppStack,hand,deck);
					} else {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/size),deck);
					}
					return;
				} else {
					if(winRate<thirdPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/3.0),deck);
						return;
					} else if (winRate<halfPot) {
						Raise(myStack,oppStack,hand,(int)((hand.potSize)/2.0),deck);
						return;
					} else {
						default_random_engine gen;
						uniform_int_distribution<int> distr(0,1);
						int rand=distr(gen);
						if(rand==0) {
							Raise(myStack,oppStack,hand,(int)((hand.potSize)),deck);
						} else {
							Check(myStack,oppStack,hand,deck);
						}
						return;
					}
				}
			}
		} else {
			int raiseAfter=70+distribution(generator);
			if(winRate>raiseAfter) {
				Raise(myStack,oppStack,hand,3*hand.callSize,deck);
				return;
			} else if ((hand.callSize<=hand.potSize-hand.callSize && winRate>100*hand.callSize/(hand.potSize+hand.callSize)) || winRate>75) {
				Call(myStack,oppStack,hand,deck);
				return;
			} else {
				Fold(myStack,oppStack,hand);
				return;
			}
		}

	}

}

float simHands(int mc[2][2],int n,int inDeck[n][2],int trials,Hand& hand,int (*opRange)[1326][4],int (*myRange)[1326][4],bool range) {
	random_device rd;
	mt19937 g(rd());
	int deck[52][2];
	for(int i=0;i<n;i++) {
		deck[i][0]=inDeck[i][0];
		deck[i][1]=inDeck[i][1];
	}
	int board[5][2]={{0,0},{0,0},{0,0},{0,0},{0,0}};
	float wins=0;
	default_random_engine generator;
	uniform_int_distribution<int> distribution(0,1325);
	int myCards[2][2];
	myCards[0][0]=mc[0][0];
	myCards[0][1]=mc[0][1];
	myCards[1][0]=mc[1][0];
	myCards[1][1]=mc[1][1];
	for(int b=0;b<trials;b++) {
		int counter=0,counter1=0;
		if(range) {
			int rnd;
			do {
				rnd=distribution(generator);
				counter1++;
				if(counter1>5000) {
					resetRange(myRange,deck);
					break;
				}
			} while((*opRange)[rnd][0]==0);
			myCards[0][0]=(*myRange)[rnd][0];
			myCards[0][1]=(*myRange)[rnd][1];
			myCards[1][0]=(*myRange)[rnd][2];
			myCards[1][1]=(*myRange)[rnd][3];
		}

		int oppCards[2][2];
		int rand;
		bool dupe=true;
		while(dupe) {
			do {
				rand=distribution(generator);
				counter++;
				if(counter>5000) {
					resetRange(opRange,deck);
					//updateRange(.5,opRange,myRange,deck,hand);
					break;
				}
			} while((*opRange)[rand][0]==0);
			oppCards[0][0]=(*opRange)[rand][0];
			oppCards[0][1]=(*opRange)[rand][1];
			oppCards[1][0]=(*opRange)[rand][2];
			oppCards[1][1]=(*opRange)[rand][3];
			for(int i=0;i<2;i++) {
				if(!((oppCards[i][0]==myCards[0][0] && oppCards[i][1]==myCards[0][1]) || (oppCards[i][0]==myCards[1][0] && oppCards[i][1]==myCards[1][1]))) {
					dupe=false;
					break;
				}
			}
		}

		shuffle(begin(deck),end(deck),g);
		while((deck[0][0]==oppCards[0][0] && deck[0][1]==oppCards[0][1]) || (deck[1][0]==oppCards[1][0] && deck[1][1]==oppCards[1][1])) {
			shuffle(begin(deck),end(deck),g);
		}
		for(int i=0;i<5;i++) {
			board[i][0]=deck[i][0];
			board[i][1]=deck[i][1];
		}
		if(hand.stage>=1) {
			for(int i=0;i<3;i++) {
				board[i][0]=hand.flop[i][0];
				board[i][1]=hand.flop[i][1];
			}
		}
		if(hand.stage>=2) {
			board[3][0]=hand.turn[0][0];
			board[3][1]=hand.turn[0][1];
		}
		if(hand.stage>=3) {
			board[4][0]=hand.river[0][0];
			board[4][1]=hand.river[0][1];
		}

		//cout<<"\n"<<myCards[0][0]<<"."<<myCards[0][1]<<" "<<myCards[1][0]<<"."<<myCards[1][1]<<" vs "<<oppCards[0][0]<<"."<<oppCards[0][1]<<" "<<oppCards[1][0]<<"."<<oppCards[1][1]<<"\n";
		//cout<<board[0][0]<<"."<<board[0][1]<<" "<<board[1][0]<<"."<<board[1][1]<<" "<<board[2][0]<<"."<<board[2][1]<<" "<<board[3][0]<<"."<<board[3][1]<<" "<<board[4][0]<<"."<<board[4][1]<<"\n";

		int n=7,r=5;
		vector<bool> v(n);
	    fill(v.end() - r, v.end(), true);
	    int k=0;
	    int l=0;
	    int myCombinations[21][5][2];
	    int oppCombinations[21][5][2];
	    do {
	        for (int i=0;i<n;i++) {
	            if (v[i]) {
	            	if (i<5) {
	            		myCombinations[k][l][0]=board[i][0];
	            		myCombinations[k][l][1]=board[i][1];
	            		oppCombinations[k][l][0]=board[i][0];
	            		oppCombinations[k][l][1]=board[i][1];
	            	} else {
	            		myCombinations[k][l][0]=myCards[i-5][0];
	            		myCombinations[k][l][1]=myCards[i-5][1];
	            		oppCombinations[k][l][0]=oppCards[i-5][0];
	            		oppCombinations[k][l][1]=oppCards[i-5][1];
	            	}
	            	l++;
	            }
	        }
	        l=0;
	        k++;
	    } while (next_permutation(v.begin(), v.end()));

	    long long myMaxStrength;
	    long long oppMaxStrength;
	    myMaxStrength=0;
	    oppMaxStrength=0;
	    for(int i=0;i<21;i++) {
	    	long long myTemp=strengthOfHand(myCombinations[i]);
	    	if(myTemp>myMaxStrength) {
	    		myMaxStrength=myTemp;
	    	}
	    	long long oppTemp=strengthOfHand(oppCombinations[i]);
	    	if(oppTemp>oppMaxStrength) {
	    		oppMaxStrength=oppTemp;
	    	}
	    }
	    //cout<<myMaxStrength<<" "<<oppMaxStrength<<"\n";
	    if(myMaxStrength>=oppMaxStrength) {
	    	wins++;
	    }/*
	    	cout<<"WIN:"<<endl;
		    for(int i=0;i<4;i++) {
				cout<<oppCards[i/2][i%2]<<" ";
			}
			cout<<" vs ";
			for(int i=0;i<4;i++) {
				cout<<myCards[i/2][i%2]<<" ";
			}
			cout<<endl;
			for(int i=0;i<10;i++) {
				cout<<board[i/2][i%2]<<" ";
			}
			cout<<endl;
	    
	    if(oppMaxStrength>=myMaxStrength&&!range) {
	    	cout<<"LOSS:"<<endl;
	    	for(int i=0;i<4;i++) {
	    		cout<<oppCards[i/2][i%2]<<" ";
	    	}
	    	cout<<" vs ";
	    	for(int i=0;i<4;i++) {
	    		cout<<myCards[i/2][i%2]<<" ";
	    	}
	    	cout<<endl;
	    	for(int i=0;i<10;i++) {
	    		cout<<board[i/2][i%2]<<" ";
	    	}
	    	cout<<endl;
	    } else if (myMaxStrength==oppMaxStrength) {
	    	wins+=.5;
	    }*/
	}
	float winRate = 100.0 * wins / trials;
	//if(!range) cout<<winRate<<endl;
	return winRate;
}

void Fold(int *myStack,int *oppStack,Hand& hand) {
	cout<<"Fold.\n";
	*oppStack+=hand.potSize;
	hand.potSize=0;
	hand.callSize=0;
	hand.aggressor=false;
}

void Check(int *myStack,int *oppStack,Hand& hand,int deck[52][2]) {
	cout<<"Check.\n";
	hand.aggressor=false;
	float equity=.8;
	updateRange2(equity,&hand.myRange,&hand.oppRange,deck,hand);
}

void Call(int *myStack,int *oppStack,Hand& hand,int deck[52][2]) {
	cout<<"Call.\n";
	hand.potSize+=hand.callSize;
	*myStack-=hand.callSize;
	float equity=(1.0*hand.callSize)/(1.0*hand.potSize);
	hand.callSize=0;
	hand.aggressor=false;
	updateRange(equity,&hand.myRange,&hand.oppRange,deck,hand);
}

void Raise(int *myStack,int *oppStack,Hand& hand,int value,int deck[52][2]) {
	hand.aggressor=true;
	if(value>*myStack) {
		value=*myStack;
	}
	if(value>*oppStack) {
		value=*oppStack;
	}
	if(value==0) {
		Call(myStack,oppStack,hand,deck);
		return;
	}
	cout<<"Raise by "<<value<<endl;
	hand.potSize+=value;
	int pre=hand.callSize;
	hand.callSize=value-hand.callSize;
	*myStack-=value;
	// float equity=(1.0*pre+value)/(1.0**potSize+value);
	// updateRange(equity,opRange,myRange,stage,myCards,deck,flop,turn,river);
	float e=(1.0*pre+hand.callSize)/(1.0*hand.potSize+hand.callSize);
	float foldEq=foldEquity(e,&hand.myRange,&hand.oppRange,deck,hand);
	float equity=foldEq+(1-foldEq)*(e);
	updateRange(equity,&hand.oppRange,&hand.myRange,deck,hand);
}




























