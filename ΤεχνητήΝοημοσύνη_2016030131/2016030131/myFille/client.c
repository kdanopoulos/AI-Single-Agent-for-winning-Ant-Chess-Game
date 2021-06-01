#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/**********************************************************/
Position gamePosition;		// Position we are going to use

//Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

int dieNextRoundScore = 5;
int foodScore = 4;
int normalMoveScore = 1;
int scorePerCapture = 5;

char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "MyAgent123!";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)



typedef struct {
    char board[ BOARD_ROWS ][ BOARD_COLUMNS ];
    int scoreRaise;
    char tile[ 2 ][ MAXIMUM_MOVE_SIZE ];
	
    struct listOfSons * sons;
    struct listOfSons * myListNode;
}state;

state * myHead;

typedef struct  {
	struct state * myState;
	struct state * father;
	
	struct listOfSons * nextNode;
	struct listOfSons * prevNode;
}listOfSons;



typedef struct  {
	struct list * firstNode;
	int doIHaveCaptureMoves;
}function;

typedef struct  {
	int capture;
	int i;
	int j;
	int direction;
	int food;
        struct list * next;
} list;
/**********************************************************/

function * findAllPossibleMovesForThisRoundAndAddToTheList(char , state *);
int minimax(state *,int,int,int);
int alphaBeta(state *,int,int,int,int,int);
int max(int a,int b);
int min(int a,int b);
int stateGenerattion();
void createStatesFromList(state *,list *,int,char );
int canCapture( char, char, char, char[ BOARD_ROWS ][ BOARD_COLUMNS ]);
moves findMyMaximumMoves(int,int,moves,int,char);
void addASonToThisState(state *,state *);
void copyBoardStateP(state *,char [ BOARD_ROWS ][ BOARD_COLUMNS ]);
void copyMoveStateP(state *,char [ 2 ][ MAXIMUM_MOVE_SIZE ]);
moves copyBoardMoves(moves,char [ BOARD_ROWS ][ BOARD_COLUMNS ]);
moves copyMoveMoves(moves ,char [2][MAXIMUM_MOVE_SIZE]);
int canIDieInTheNextRound(int ,int ,char ,char ,char [BOARD_ROWS][BOARD_COLUMNS]);

void copyBoardStateP(state * st,char temp[ BOARD_ROWS ][ BOARD_COLUMNS ]){
    int i,j;
    for(i=0;i<BOARD_ROWS;i++){
        for(j=0;j<BOARD_COLUMNS;j++){
                st->board[i][j] = temp[i][j];
        }
    }
}
void copyMoveStateP(state * st,char temp[ 2 ][ MAXIMUM_MOVE_SIZE ]){
	int i,j;
	for(i=0;i<2;i++){
        for(j=0;j<MAXIMUM_MOVE_SIZE;j++){
                st->tile[i][j] = temp[i][j];
        }
    }
}
moves copyBoardMoves(moves mv,char temp[ BOARD_ROWS ][ BOARD_COLUMNS ]){
	int i,j;
    for(i=0;i<BOARD_ROWS;i++){
        for(j=0;j<BOARD_COLUMNS;j++){
                mv.board[i][j] = temp[i][j];
        }
    }
	return mv;
}
moves copyMoveMoves(moves mv,char temp[2][MAXIMUM_MOVE_SIZE]){
    int i,j;
	for(i=0;i<2;i++){
        for(j=0;j<MAXIMUM_MOVE_SIZE;j++){
                mv.move[i][j] = temp[i][j];
        }
    }
    return mv;
}

int minimax(state * possition,int depth,int maximazingPlayer,int score){
	int minEval,maxEval,eval;
	if (depth==1){
		score+=possition->scoreRaise;
		return score;
	}
	if (maximazingPlayer == 1){
		maxEval = -100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		while(currList!=NULL){
			eval = minimax(currList->myState,depth-1,0,score);
			maxEval = max(maxEval,eval);
			currList = currList->nextNode;
		}
		return maxEval;
	}
	else{
		minEval = 100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		while(currList!=NULL){
			eval = minimax(currList->myState,depth-1,1,score);
			minEval = min(minEval,eval);
			currList = currList->nextNode;
		}
		return minEval;
	}
}

int alphaBeta(state * possition,int depth,int alpha,int beta,int maximazingPlayer,int score){ // alpha = -100000 , beta = 100000
	int minEval,maxEval,eval;
	if (depth==1){
		score+=possition->scoreRaise;
		return score;
	}
	if (maximazingPlayer == 1){
		maxEval = -100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		while(currList!=NULL){
			eval = alphaBeta(currList->myState,depth-1,alpha,beta,0,score);
			maxEval = max(maxEval,eval);
			alpha = max(alpha,eval);
			if (beta <= alpha)
				currList = NULL;
			else
				currList = currList->nextNode;
			
		}
		return maxEval;
	}
	else{
		minEval = 100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		while(currList!=NULL){
			eval = alphaBeta(currList->myState,depth-1,alpha,beta,1,score);
			minEval = min(minEval,eval);
			beta = min(beta,eval);
			if (beta <= alpha)
				currList = NULL;
			else
				currList = currList->nextNode;
		}
		return minEval;
	}
}

int minimaxForwardPruning(state * possition,int depth,int maximazingPlayer,int score,int dif,int waiting){
	int minEval,maxEval,eval;
	if (depth==1){
		score+=possition->scoreRaise;
		return score;
	}
	if (maximazingPlayer == 1){
		maxEval = -100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		int mean = 0;
		int number =0;
		int difference;
		while(currList!=NULL){
            number++;
			eval = minimaxForwardPruning(currList->myState,depth-1,0,score);
			mean += eval;
			maxEval = max(maxEval,eval);
			currList = currList->nextNode;
			if (number>waiting)	{
			difference = eval - (mean/number);
			if (difference>dif){
                    return eval;
			}
			}
		}
		return maxEval;
	}
	else{
		minEval = 100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		int mean = 0;
		int number =0;
		int difference;
		while(currList!=NULL){
            number++;
			eval = minimaxForwardPruning(currList->myState,depth-1,1,score);
			mean += eval;
			minEval = min(minEval,eval);
			currList = currList->nextNode;
			if (number>waiting)	{
			difference = eval - (mean/number);
			if (difference<-dif){
                    return eval;
			}
            }
		}
		return minEval;
	}
}


int alphaBetaForwardPruning(state * possition,int depth,int alpha,int beta,int maximazingPlayer,int score,int dif,int waiting){ // alpha = -100000 , beta = 100000
	int minEval,maxEval,eval;
	if (depth==1){
		score+=possition->scoreRaise;
		return score;
	}
	if (maximazingPlayer == 1){
		maxEval = -100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		int mean = 0;
		int number =0;
		int difference;
		while(currList!=NULL){
            number++;
			eval = alphaBetaForwardPruning(currList->myState,depth-1,alpha,beta,0,score,dif,waiting);
            mean += eval;
			maxEval = max(maxEval,eval);
			alpha = max(alpha,eval);
			if (beta <= alpha)
				currList = NULL;
			else
				currList = currList->nextNode;
			if (number>waiting)	{
			difference = eval - (mean/number);
			if (difference>dif){
                    return eval;
			}
			}

		}
		return maxEval;
	}
	else{
		minEval = 100000;
		listOfSons * currList = possition->sons;
		score+= possition->scoreRaise;
		int mean = 0;
		int number =0;
		int difference;
		while(currList!=NULL){
            number++;
			eval = alphaBetaForwardPruning(currList->myState,depth-1,alpha,beta,1,score,dif,waiting);
            mean += eval;
			minEval = min(minEval,eval);
			beta = min(beta,eval);
			if (beta <= alpha)
				currList = NULL;
			else
				currList = currList->nextNode;
            if (number>waiting)	{
			difference = eval - (mean/number);
			if (difference<-dif){
                    return eval;
			}
            }
		}
		return minEval;
	}
}


int max(int a,int b){
	if(a>=b)
		return a;
	else 
		return b;
}
int min(int a,int b){
	if(a<=b)
		return a;
	else 
		return b;
}

int stateGenerattion(){
function * myFunc;
state * head = (state *) malloc(sizeof(state));
myHead = head;
copyBoardStateP(head,gamePosition.board);
head->scoreRaise = 0;
head->sons = NULL;
head->myListNode = NULL;
myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(myColor,head);
list * smallList = myFunc->firstNode;
smallList = smallList->next;  // because the first node does not contain information
createStatesFromList(head,smallList,myFunc->doIHaveCaptureMoves,myColor);
//-------------------------------------------------------
char opponentColor;
if (myColor==WHITE)
    opponentColor = BLACK;
else
    opponentColor = WHITE;
listOfSons * sonsOfHead = head->sons;
state * cState;
while(sonsOfHead!=NULL){
    cState = sonsOfHead->myState;
    myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(opponentColor,cState);
    smallList = myFunc->firstNode;
    smallList = smallList->next;
    if (smallList==NULL)
        return 1;
    createStatesFromList(cState,smallList,myFunc->doIHaveCaptureMoves,opponentColor);
    sonsOfHead = sonsOfHead->nextNode;
}
//-------------------------------------------------------
listOfSons * sonsOfSons;
state * cState2;
sonsOfHead = head->sons;
while(sonsOfHead!=NULL){
    cState = sonsOfHead->myState;
    sonsOfSons = cState->sons;
    while(sonsOfSons!=NULL){
        cState2 = sonsOfSons->myState;
        myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(myColor,cState2);
        smallList = myFunc->firstNode;
        smallList = smallList->next;
        if (smallList==NULL)
        return 2;
        createStatesFromList(cState2,smallList,myFunc->doIHaveCaptureMoves,myColor);
        sonsOfSons = sonsOfSons->nextNode;
    }
    sonsOfHead = sonsOfHead->nextNode;
}
//-------------------------------------------------------
listOfSons * sonsOfSonsOfSons;
state * cState3;
sonsOfHead = head->sons;
while(sonsOfHead!=NULL){
    cState = sonsOfHead->myState;
    sonsOfSons = cState->sons;
    while(sonsOfSons!=NULL){
        cState2 = sonsOfSons->myState;
        sonsOfSonsOfSons = cState2->sons;
        while(sonsOfSonsOfSons!=NULL){
            cState3 = sonsOfSonsOfSons->myState;
            myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(opponentColor,cState3);
            smallList = myFunc->firstNode;
            smallList = smallList->next;
            if (smallList==NULL)
            return 3;
            createStatesFromList(cState3,smallList,myFunc->doIHaveCaptureMoves,opponentColor);
            sonsOfSonsOfSons = sonsOfSonsOfSons->nextNode;
        }
        sonsOfSons = sonsOfSons->nextNode;
    }
    sonsOfHead = sonsOfHead->nextNode;
}
//-------------------------------------------------------
listOfSons * fourthSons;
state * cState4;
sonsOfHead = head->sons;
while(sonsOfHead!=NULL){
    cState = sonsOfHead->myState;
    sonsOfSons = cState->sons;
    while(sonsOfSons!=NULL){
        cState2 = sonsOfSons->myState;
        sonsOfSonsOfSons = cState2->sons;
        while(sonsOfSonsOfSons!=NULL){
            cState3 = sonsOfSonsOfSons->myState;
            fourthSons = cState3->sons;
            while(fourthSons!=NULL){
                cState4 = fourthSons->myState;
                myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(myColor,cState4);
                smallList = myFunc->firstNode;
                smallList = smallList->next;
                if (smallList==NULL)
                return 4;
                createStatesFromList(cState4,smallList,myFunc->doIHaveCaptureMoves,myColor);
                fourthSons = fourthSons->nextNode;
            }
            sonsOfSonsOfSons = sonsOfSonsOfSons->nextNode;
        }
        sonsOfSons = sonsOfSons->nextNode;
    }
    sonsOfHead = sonsOfHead->nextNode;
}
//-------------------------------------------------------
/*
listOfSons * fifthSons;
state * cState5;
sonsOfHead = head->sons;
while(sonsOfHead!=NULL){
    cState = sonsOfHead->myState;
    sonsOfSons = cState->sons;
    while(sonsOfSons!=NULL){
        cState2 = sonsOfSons->myState;
        sonsOfSonsOfSons = cState2->sons;
        while(sonsOfSonsOfSons!=NULL){
            cState3 = sonsOfSonsOfSons->myState;
            fourthSons = cState3->sons;
            while(fourthSons!=NULL){
                cState4 = fourthSons->myState;
                fifthSons = cState4->sons;
                while(fifthSons!=NULL){
                    cState5 = fifthSons->myState;
                    myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(opponentColor,cState5);
                    smallList = myFunc->firstNode;
                    smallList = smallList->next;
                    if (smallList==NULL)
                         return 5;
                    createStatesFromList(cState5,smallList,myFunc->doIHaveCaptureMoves,opponentColor);
                    fifthSons = fifthSons->nextNode;
                }
                fourthSons = fourthSons->nextNode;
            }
            sonsOfSonsOfSons = sonsOfSonsOfSons->nextNode;
        }
        sonsOfSons = sonsOfSons->nextNode;
    }
    sonsOfHead = sonsOfHead->nextNode;
}*/
return 5;
}
void freeTheMallocStates(){
listOfSons * sonsOfHead;
state * cState;
listOfSons * sonsOfSons;
state * cState2;
listOfSons * sonsOfSonsOfSons;
state * cState3;
listOfSons * fourthSons;
state * cState4;
listOfSons * fifthSons;
sonsOfHead = myHead->sons;
free(sonsOfHead->father);
while(sonsOfHead!=NULL){
    cState = sonsOfHead->myState;
    sonsOfSons = cState->sons;
    while(sonsOfSons!=NULL){
        cState2 = sonsOfSons->myState;
        sonsOfSonsOfSons = cState2->sons;
        while(sonsOfSonsOfSons!=NULL){
            cState3 = sonsOfSonsOfSons->myState;
            fourthSons = cState3->sons;
            while(fourthSons!=NULL){
                cState4 = fourthSons->myState;
                fifthSons = cState4->sons;
                while(fifthSons!=NULL){
                free(fifthSons->myState);
                fifthSons = fifthSons->nextNode;
                free(fifthSons->prevNode);
                }
                free(cState4);
                fourthSons = fourthSons->nextNode;
                free(fourthSons->prevNode);
            }
            free(cState3);
            sonsOfSonsOfSons = sonsOfSonsOfSons->nextNode;
            free(sonsOfSonsOfSons->prevNode);
        }
        free(cState2);
        sonsOfSons = sonsOfSons->nextNode;
        free(sonsOfSons->prevNode);
    }
    free(cState);
    sonsOfHead = sonsOfHead->nextNode;
    free(sonsOfHead->prevNode);
}
}

int canIDieInTheNextRound(int new_i,int new_j,char currentColor,char currentEnemyColor,char curBoard[BOARD_ROWS][BOARD_COLUMNS]){
    if(currentColor == WHITE){
        if(new_i+1<=BOARD_ROWS && new_j+1<=BOARD_COLUMNS && new_i-1>=0 && new_j-1>=0){
            if(curBoard[new_i+1][new_j+1]==currentEnemyColor && curBoard[new_i-1][new_j-1]==EMPTY){ // I can die from the right in the next round
                    if(currentColor==myColor){
                        return -5;
                    }
                    else{
                        return +5;
                    }
            }
            if(curBoard[new_i-1][new_j+1]==currentEnemyColor && curBoard[new_i+1][new_j-1]== EMPTY){ // I can die from the left in the next round
                    if(currentColor==myColor){
                        return -5;
                    }
                    else{
                        return +5;
                    }
            }
        }
    }
    else{  // currentColor = BLACK
        if(new_i+1<=BOARD_ROWS && new_j+1<=BOARD_COLUMNS && new_i-1>=0 && new_j-1>=0){
            if(curBoard[new_i+1][new_j-1]==currentEnemyColor && curBoard[new_i-1][new_j+1]==EMPTY){ // I can die from the right in the next round
                    if(currentColor==myColor){
                        return -5;
                    }
                    else{
                        return +5;
                    }
            }
            if(curBoard[new_i-1][new_j-1]==currentEnemyColor && curBoard[new_i+1][new_j+1]==EMPTY){ // I can die from the left in the next round
                    if(currentColor==myColor){
                        return -5;
                    }
                    else{
                        return +5;
                    }
            }
        }
    }
    return 0;
}
void createStatesFromList(state * curState,list * firstNodeOfList,int doIHaveCaptureMoves,char color){
	int m,n;
	list * curr;
	list * prevNodeList;
	curr = firstNodeOfList;
	state * newState; // the variable used for generating new states
	moves myMove2;
	if(doIHaveCaptureMoves==1){  // if I have at least one capture move (we must always choose capture moves instead of others)
		while(curr!= NULL){ // run until we reach the final possible movement
			if (curr->capture==1){
				myMove2 = copyBoardMoves(myMove2,curState->board);
				myMove2.move[0][0] = curr->i;
				myMove2.move[1][0] = curr->j;
				myMove2.move[0][1] = -1;
				myMove2.move[1][1] = -1;
				myMove2.move[0][2] = -1;
				myMove2.move[1][2] = -1;
				myMove2.move[0][3] = -1;
				myMove2.move[1][3] = -1;
				myMove2.move[0][4] = -1;
				myMove2.move[1][4] = -1;
				myMove2.move[0][5] = -1;
				myMove2.move[1][5] = -1;
				myMove2.length = -1;
				myMove2 = findMyMaximumMoves(curr->i,curr->j,myMove2,1,color); // we find the best possible movement in possition i,j
				newState = (state *) malloc(sizeof(state));
				copyBoardStateP(newState,myMove2.board); // copy best board
				copyMoveStateP(newState,myMove2.move); // copy best movement
				if (color==myColor){
				   newState->scoreRaise =  (myMove2.length * 5);
				}
				else{
				   newState->scoreRaise = - (myMove2.length * 5);
				}
				if(color==WHITE){
				   if(myMove2.move[1][myMove2.length-1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(myMove2.move[1][myMove2.length-1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				newState->sons = NULL;
				newState->myListNode = NULL;
				addASonToThisState(curState,newState); // add the new state as son of the current state
			}
			prevNodeList = curr;
			curr=curr->next;
			free(prevNodeList);
		}
	}
	else {   // if we don't have at all capture moves
		char movement[2][MAXIMUM_MOVE_SIZE];
		char tempBoard[ BOARD_ROWS ][ BOARD_COLUMNS ];
		char opponent;
		int canDie;
		if (myColor==WHITE)
            opponent=BLACK;
        else
            opponent=WHITE;
		movement[0][0] = -1;
		movement[1][0] = -1;
		movement[0][1] = -1;
		movement[1][1] = -1;
		movement[0][2] = -1;
		movement[1][2] = -1;
		movement[0][3] = -1;
		movement[1][3] = -1;
		movement[0][4] = -1;
		movement[1][4] = -1;
		movement[0][5] = -1;
		movement[1][5] = -1;
		while(curr!=NULL){
			if (curr->direction==3){  // move to both directions
			        if(color == WHITE){	//white color
				        // right
				        newState = (state *) malloc(sizeof(state));
						for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				        tempBoard[curr->i][curr->j] = EMPTY;
				        tempBoard[curr->i+1][curr->j+1] = WHITE;
					copyBoardStateP(newState,tempBoard);
					movement[0][0] = curr->i;
				        movement[1][0] = curr->j;
				        movement[0][1] = curr->i+1;
				        movement[1][1] = curr->j+1;
				        movement[0][2] = -1;
						copyMoveStateP(newState,movement);
						canDie = canIDieInTheNextRound(curr->i+1,curr->j+1,color,opponent,newState->board);
                        if (canDie==0){
				          if (curr->food == 3 || curr->food == 2){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				          }
				          else{
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				          }
                        }
                        else{
                          newState->scoreRaise = canDie;
                        }
                                        if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}                                       
				        newState->sons = NULL;
				        newState->myListNode = NULL;
				        addASonToThisState(curState,newState);
				        // left
				        newState = (state *) malloc(sizeof(state));
						for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				        tempBoard[curr->i][curr->j] = EMPTY;
				        tempBoard[curr->i+1][curr->j-1] = WHITE;
					copyBoardStateP(newState,tempBoard);
					movement[0][0] = curr->i;
				        movement[1][0] = curr->j;
				        movement[0][1] = curr->i+1;
				        movement[1][1] = curr->j-1;
				        movement[0][2] = -1;
						copyMoveStateP(newState,movement);
						canDie = canIDieInTheNextRound(curr->i+1,curr->j-1,color,opponent,newState->board);
						if (canDie==0){
				          if (curr->food == 3 || curr->food == 1){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				          }
				          else{
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				          }
						}
						else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				        newState->sons = NULL;
				        newState->myListNode = NULL;
				        addASonToThisState(curState,newState);
				    }
				    else{	  //black color
				        // right
				        newState = (state *) malloc(sizeof(state));
						for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				        tempBoard[curr->i][curr->j] = EMPTY;
				        tempBoard[curr->i-1][curr->j+1] = BLACK;
					copyBoardStateP(newState,tempBoard);
					movement[0][0] = curr->i;
				        movement[1][0] = curr->j;
				        movement[0][1] = curr->i-1;
				        movement[1][1] = curr->j+1;
				        movement[0][2] = -1;
						copyMoveStateP(newState,movement);
						canDie = canIDieInTheNextRound(curr->i-1,curr->j+1,color,opponent,newState->board);
						if (canDie==0){
				          if (curr->food == 3 || curr->food == 2){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				          }
				          else{
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				          }
						}
						else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				        newState->sons = NULL;
				        newState->myListNode = NULL;
				        addASonToThisState(curState,newState);
				        // left
				        newState = (state *) malloc(sizeof(state));
						for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				        tempBoard[curr->i][curr->j] = EMPTY;
				        tempBoard[curr->i-1][curr->j-1] = BLACK;
					copyBoardStateP(newState,tempBoard);
					movement[0][0] = curr->i;
				        movement[1][0] = curr->j;
				        movement[0][1] = curr->i-1;
				        movement[1][1] = curr->j-1;
				        movement[0][2] = -1;
						copyMoveStateP(newState,movement);
						canDie = canIDieInTheNextRound(curr->i-1,curr->j-1,color,opponent,newState->board);
						if (canDie==0){
				          if (curr->food == 3 || curr->food == 1){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				          }
				          else{
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				          }
						}
						else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				        newState->sons = NULL;
				        newState->myListNode = NULL;
				        addASonToThisState(curState,newState);
				    }
			}
			else if (curr->direction==2){   // move to the right
				    if( color == WHITE ){	//white color
				         newState = (state *) malloc(sizeof(state));
						 for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				         tempBoard[curr->i][curr->j] = EMPTY;
				         tempBoard[curr->i+1][curr->j+1] = WHITE;
					 copyBoardStateP(newState,tempBoard);
					 movement[0][0] = curr->i;
				         movement[1][0] = curr->j;
				         movement[0][1] = curr->i+1;
				         movement[1][1] = curr->j+1;
				         movement[0][2] = -1;
						 copyMoveStateP(newState,movement);
						 canDie = canIDieInTheNextRound(curr->i+1,curr->j+1,color,opponent,newState->board);
						 if (canDie==0){
				           if (curr->food == 2 ){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				           }
				           else {
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				           }
						 }
						 else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				         newState->sons = NULL;
				         newState->myListNode = NULL;
				         addASonToThisState(curState,newState);
				    }
				    else {   //black color
				         newState = (state *) malloc(sizeof(state));
						 for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				         tempBoard[curr->i][curr->j] = EMPTY;
				         tempBoard[curr->i-1][curr->j+1] = BLACK;
					 copyBoardStateP(newState,tempBoard);
					 movement[0][0] = curr->i;
				         movement[1][0] = curr->j;
				         movement[0][1] = curr->i-1;
				         movement[1][1] = curr->j+1;
				         movement[0][2] = -1;
						 copyMoveStateP(newState,movement);
						 canDie = canIDieInTheNextRound(curr->i-1,curr->j+1,color,opponent,newState->board);
						 if (canDie==0){
				           if (curr->food == 2 ){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				           }
				           else {
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				           }
						 }
						 else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				         newState->sons = NULL;
				         newState->myListNode = NULL;
				         addASonToThisState(curState,newState);
				    }
			}
			else{   // move to the left
			        if( color == WHITE ){	//white color
				        newState = (state *) malloc(sizeof(state));
						for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				        tempBoard[curr->i][curr->j] = EMPTY;
				        tempBoard[curr->i+1][curr->j-1] = WHITE;
					copyBoardStateP(newState,tempBoard);
					movement[0][0] = curr->i;
				        movement[1][0] = curr->j;
				        movement[0][1] = curr->i+1;
				        movement[1][1] = curr->j-1;
				        movement[0][2] = -1;
						copyMoveStateP(newState,movement);
						canDie = canIDieInTheNextRound(curr->i+1,curr->j-1,color,opponent,newState->board);
						if(canDie==0){
				          if (curr->food == 1 ){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				           }
				           else {
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				           }
						}
						else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				        newState->sons = NULL;
				        newState->myListNode = NULL;
				        addASonToThisState(curState,newState);
				    }
				    else {   //black color
				        newState = (state *) malloc(sizeof(state));
						for(m=0;m<BOARD_ROWS;m++){
							for(n=0;n<BOARD_COLUMNS;n++){
								tempBoard[m][n] = curState->board[m][n];
							}
						}
				        tempBoard[curr->i][curr->j] = EMPTY;
				        tempBoard[curr->i-1][curr->j-1] = BLACK;
					copyBoardStateP(newState,tempBoard);
					movement[0][0] = curr->i;
				        movement[1][0] = curr->j;
				        movement[0][1] = curr->i-1;
				        movement[1][1] = curr->j-1;
				        movement[0][2] = -1;
						copyMoveStateP(newState,movement);
						canDie = canIDieInTheNextRound(curr->i-1,curr->j-1,color,opponent,newState->board);
						if(canDie==0){
				          if (curr->food == 1 ){
				              if (color==myColor)
				                  newState->scoreRaise = 4;
				              else
				                  newState->scoreRaise = -4;
				           }
				           else {
				              if (color==myColor)
				                  newState->scoreRaise = 1;
				              else
				                  newState->scoreRaise = -1;
				           }
						}
						else {
                          newState->scoreRaise = canDie;
						}
						if(color==WHITE){
				   if(movement[1][1]==(BOARD_COLUMNS-1)){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				else{
				    if(movement[1][1]==0){
				      if (color==myColor){
				          newState->scoreRaise = newState->scoreRaise + 5;
				      }
				      else{
				         newState->scoreRaise = newState->scoreRaise - 5;
                                      }				    
				   }
				}
				        newState->sons = NULL;
				        newState->myListNode = NULL;
				        addASonToThisState(curState,newState);
				    }
			}
			prevNodeList = curr;
			curr=curr->next;
			free(prevNodeList);
		}
	}
}
/*
char[][] moveWhiteLeft(int i ,int j , char[][] curBoard){
	curBoard[i][j] = EMPTY;
	curBoard[i+1][j-1] = WHITE;
	return curBoard;
}
char[][] moveWhiteRight(int i ,int j , char[][] curBoard){
	curBoard[i][j] = EMPTY;
	curBoard[i+1][j+1] = WHITE;
	return curBoard;
}
char[][] moveBlackLeft(int i ,int j , char[][] curBoard){
	curBoard[i][j] = EMPTY;
	curBoard[i-1][j-1] = BLACK;
	return curBoard;
}
char[][] moveBlackRight(int i ,int j , char[][] curBoard){
	curBoard[i][j] = EMPTY;
	curBoard[i-1][j+1] = BLACK;
	return curBoard;
}*/



function * findAllPossibleMovesForThisRoundAndAddToTheList(char color, state * currentState){
	int temp;
	function * myFunc = (function *) malloc(sizeof(function));
	list * temporaryList = (list *) malloc(sizeof(list));
	temporaryList->capture = -1;
	temporaryList->i = -1;
	temporaryList->j = -1;
	temporaryList->direction = -1;
	temporaryList->next = NULL;

        myFunc->doIHaveCaptureMoves = 0;
        myFunc->firstNode = temporaryList;
	list * curr = myFunc->firstNode;

	for(int i = 0; i < BOARD_ROWS; i++ )
	{
		for(int j = 0; j < BOARD_COLUMNS; j++ )
		{
			if( currentState->board[i][j]== color )	//when we find a piece of ours
			{
				if( color == WHITE )	//white color
				{
					if(i+1 < BOARD_ROWS )
					{
						if( j-1>=0 )
							if( (currentState->board[i+1][j-1]==EMPTY)||(currentState->board[i+1][j-1]==RTILE)){//check if we can move to the left
			 	                                if (currentState->board[i+1][j-1]==RTILE){
			 	                                   curr->food = 1;
			 	                                }
								curr->next = (list *) malloc(sizeof(list));
								curr = curr->next;
								curr->capture = 0;
								curr->i = i;
								curr->j = j;
								curr->next = NULL;
								curr->direction = 1;
							}
						if( j+1 < BOARD_COLUMNS )
							if( (currentState->board[i+1][j+1]==EMPTY)||(currentState->board[i+1][j+1]==RTILE)){//check if we can move to the right
								if (curr->i == i && curr->j == j){
									curr->direction = 3;
									if (currentState->board[i+1][j+1]==RTILE && curr->food==1){
									   curr->food = 3;
									}
								}
								else {
								    curr->next = (list *) malloc(sizeof(list));
								    curr = curr->next;
								    curr->capture = 0;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
								    if (currentState->board[i+1][j+1]==RTILE){
								       curr->food = 2;
								    }
                                                                    curr->direction = 2;
								}
							}
						temp = canCapture(i,j,WHITE,currentState->board);
						if( temp != 0 )	{	//check if we can make a jump - capture
						    myFunc->doIHaveCaptureMoves = 1;
						    if (temp == 3){
								curr->next = (list *) malloc(sizeof(list));
								curr = curr->next;
								curr->capture = 1;
								curr->i = i;
								curr->j = j;
								curr->next = NULL;
                                                                curr->direction = 3;
							}
							else {
								if (temp==1){
								    curr->next = (list *) malloc(sizeof(list));
								    curr = curr->next;
								    curr->capture = 1;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
                                                                    curr->direction = 1;
								}
								else{
								    curr->next = (list *) malloc(sizeof(list));
								    curr = curr->next;
								    curr->capture = 1;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
                                                                    curr->direction = 2;
								}
							}
						}
					}
				}
				else		//black color
				{
					if(i-1 >= 0 )
					{
						if(j-1 >= 0 )
							if( (currentState->board[i-1][j-1]==EMPTY )||(currentState->board[i-1][j-1]==RTILE)){//check if we can move to the left
							        if (currentState->board[i-1][j-1]==RTILE){
							           curr->food = 1;
							        }
								curr->next = (list *) malloc(sizeof(list));
								curr = curr->next;
								curr->capture = 0;
								curr->i = i;
								curr->j = j;
								curr->next = NULL;
								curr->direction = 1;
							}

						if(j+1 < BOARD_COLUMNS )
							if( (currentState->board[i-1][j+1]==EMPTY )||(currentState->board[i-1][j+1]==RTILE)){//check if we can move to the right
								if (curr->i == i && curr->j == j){
									curr->direction = 3;
									if (currentState->board[i-1][j+1]==RTILE && curr->food == 1){
									   curr->food = 3;
									}
								}
								else {
								    curr->next = (list *) malloc(sizeof(list));
								    curr = curr->next;
								    curr->capture = 0;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
								    if (currentState->board[i-1][j+1]==RTILE){
								       curr->food = 2;
								    }
                                                                    curr->direction = 2;
								}
							}

						temp = canCapture(i,j,BLACK,currentState->board);
						if( temp != 0 )	{	//check if we can make a jump - capture
						    myFunc->doIHaveCaptureMoves = 1;
						    if (temp == 3){
								curr->next = (list *) malloc(sizeof(list));
								curr = curr->next;
								curr->capture = 1;
								curr->i = i;
								curr->j = j;
								curr->next = NULL;
                                                                curr->direction = 3;
							}
							else {
								if (temp==1){
								    curr->next = (list *) malloc(sizeof(list));
								    curr = curr->next;
								    curr->capture = 1;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
                                                                    curr->direction = 1;
								}
								else{
								    curr->next = (list *) malloc(sizeof(list));
								    curr = curr->next;
								    curr->capture = 1;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
                                                                    curr->direction = 2;
								}
							}
						}
					}
				}
			}
		}
	}


   return myFunc;
}
int canCapture( char row, char col, char player, char board[ BOARD_ROWS ][ BOARD_COLUMNS ])
{
	int returnValue = 0;

	if( player == WHITE )	//white player
	{
		if(row+2 < BOARD_ROWS)
		{
			if(col-2 >= 0 )
			{
				if(board[row+1][col-1] == BLACK && ((board[row+2][col-2]==EMPTY)||(board[row+2][col-2]==RTILE)))
				{
					returnValue = returnValue + 1;	//left jump possible
				}
			}

			if(col+2 < BOARD_COLUMNS )
			{
				if( board[row+1][col+1] == BLACK && ((board[row+2][col+2]==EMPTY)||(board[row +2][col+2]==RTILE)))
				{
					returnValue = returnValue + 2;	//right jump possible
				}
			}

		}
	}
	else	//black player
	{
		if(row-2 >= 0 )
		{
			if(col-2 >= 0 )
			{
				if( board[row-1][col-1] == WHITE && ((board[row-2][col-2]==EMPTY)||(board[row-2][col-2]==RTILE)))
				{
					returnValue = returnValue + 1;	//left jump possible
				}
			}

			if(col+2 < BOARD_COLUMNS )
			{
				if( board[row-1][col+1] == WHITE && ((board[row-2][col+2]==EMPTY)||(board[row-2][col+2]==RTILE)))
				{
					returnValue = returnValue + 2;	//right jump possible
				}
			}

		}
	}
	return returnValue;
}



moves findMyMaximumMoves(int i ,int j ,moves myMove2,int numberOfStep,char color){ // at the beginning numberOfStep = 1;
	moves leftMoves;
	moves rightMoves;
	moves temp;
	int leftBoolean = 0;
	int rightBoolean = 0;

	if (numberOfStep==6){
	        return myMove2;
	}
	if ( color == WHITE ){ // white
	    if(i + 2 < BOARD_ROWS){
		    if (j - 2 >= 0){
				if( myMove2.board[i+1][j-1] == BLACK &&(( myMove2.board[i+2][j-2]==EMPTY)||( myMove2.board[i+2][j-2]==RTILE)))
				{
					temp = copyBoardMoves(temp,myMove2.board);
					temp = copyMoveMoves(temp,myMove2.move);
					temp.board[i][j] = EMPTY;
					temp.board[i+1][j-1] = EMPTY;
					temp.board[i+2][j-2] = WHITE;
					temp.move[0][(numberOfStep-1)] = i;
					temp.move[1][(numberOfStep-1)] = j;
					temp.move[0][numberOfStep] = i+2;
					temp.move[1][numberOfStep] = j-2;
					temp.length = numberOfStep;
					leftMoves = findMyMaximumMoves(i+2,j-2,temp,numberOfStep+1,color);	//left jump possible
					leftBoolean = 1;
				}
		    }
	        if (j + 2 < BOARD_COLUMNS){
				if( myMove2.board[i+1][j+1]==BLACK && ((myMove2.board[i+2][j+2]==EMPTY)||(myMove2.board[i+2][j+2]==RTILE)))
				{
					temp = copyBoardMoves(temp,myMove2.board);
					temp = copyMoveMoves(temp,myMove2.move);
					temp.board[i][j] = EMPTY;
					temp.board[i+1][j+1] = EMPTY;
					temp.board[i+2][j+2] = WHITE;
					temp.move[0][(numberOfStep-1)] = i;
					temp.move[1][(numberOfStep-1)] = j;
					temp.move[0][numberOfStep] = i+2;
					temp.move[1][numberOfStep] = j+2;
					temp.length = numberOfStep;
					rightMoves = findMyMaximumMoves(i+2,j+2,temp,numberOfStep+1,color);	//right jump possible
					rightBoolean = 1;
				}
		    }
	    }
	}
	else{  // black
	    if(i - 2 >= 0){
			if( j - 2 >= 0 )
			{
				if( myMove2.board[i-1][j-1]==WHITE && ((myMove2.board[i-2][j-2]==EMPTY)||(myMove2.board[i-2][j-2]==RTILE)))
				{
					temp = copyBoardMoves(temp,myMove2.board);
					temp = copyMoveMoves(temp,myMove2.move);
					temp.board[i][j] = EMPTY;
					temp.board[i-1][j-1] = EMPTY;
					temp.board[i-2][j-2] = BLACK;
					temp.move[0][(numberOfStep-1)] = i;
					temp.move[1][(numberOfStep-1)] = j;
					temp.move[0][numberOfStep] = i-2;
					temp.move[1][numberOfStep] = j-2;
					temp.length = numberOfStep;
					leftMoves = findMyMaximumMoves(i-2,j-2,temp,numberOfStep+1,color);	//left jump possible
					leftBoolean = 1;
				}
			}

			if( j + 2 < BOARD_COLUMNS )
			{
				if( myMove2.board[i-1][j+1]==WHITE && ((myMove2.board[i-2][j+2]==EMPTY)||(myMove2.board[i-2][j+2]==RTILE)))
				{
					temp = copyBoardMoves(temp,myMove2.board);
					temp = copyMoveMoves(temp,myMove2.move);
					temp.board[i][j] = EMPTY;
					temp.board[i-1][j+1] = EMPTY;
					temp.board[i-2][j+2] = BLACK;
					temp.move[0][(numberOfStep-1)] = i;
					temp.move[1][(numberOfStep-1)] = j;
					temp.move[0][numberOfStep] = i-2;
					temp.move[1][numberOfStep] = j+2;
					temp.length = numberOfStep;
					rightMoves = findMyMaximumMoves(i-2,j+2,temp,numberOfStep+1,color);	//right jump possible
					rightBoolean = 1;
				}
			}
		}
	}
    if (leftBoolean==1 && rightBoolean==1){
		if (leftMoves.length>rightMoves.length){
			return leftMoves;
		}
		else {
			return rightMoves;
		}
	}
	else if (leftBoolean==1){
		return leftMoves;
	}
	else if (rightBoolean==1){
		return rightMoves;
	}
	else {
        myMove2.move[0][numberOfStep] = -1;
        myMove2.move[1][numberOfStep] = -1;
		return myMove2;
	}
		return myMove2;
}




void addASonToThisState(state * myState,state * sonState){
	if(myState->sons == NULL){  // we add the first son
	
	  listOfSons * mySons = (listOfSons *) malloc(sizeof(listOfSons));
	  mySons->myState = sonState;
	  mySons->father = myState;
	  mySons->nextNode = NULL;
	  mySons->prevNode = NULL;
	
	  myState->sons = mySons;
	  
	  sonState->myListNode = mySons;
	}
	else {    // we have already at least 1 son 
	
	  listOfSons * curNodeOfSons = myState->sons;
	  while(curNodeOfSons->nextNode!=NULL){
		curNodeOfSons = curNodeOfSons->nextNode;
	  }
	  // curNodeOfSons == with last node of the list Sons
	  listOfSons * newSon = (listOfSons *) malloc(sizeof(listOfSons));
	  newSon->myState = sonState;
	  newSon->father = myState;
	  newSon->nextNode = NULL;
	  newSon->prevNode = curNodeOfSons;
	  
	  curNodeOfSons->nextNode = newSon;
	  
	  sonState->myListNode = newSon;
	}
}

int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}



	connectToTarget( port, ip, &mySocket );

	char msg;

/**********************************************************/
// used in random
	srand( time( NULL ) );
	int i, j, k;
	int jumpPossible;
	int playerDirection;
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );
	
		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				
				break;

			case NM_COLOR_W:			//server indorms us that we have WHITE color
				myColor = WHITE;
				printf("My color is %d\n",myColor);
				break;

			case NM_COLOR_B:			//server indorms us that we have BLACK color
				myColor = BLACK;
				printf("My color is %d\n",myColor);
				break;



			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;
				

				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else{
				   int depth;
				   depth = stateGenerattion();
				   //depth = 5;
				   listOfSons * myList = myHead->sons;
				   state * bestMove;
				   int bestScore = -10000;
				   int curScore;
				   //int curScore1;
				   int oneMove = 0;
				   if (myList->nextNode==NULL)
				      oneMove = 1;
				   if(oneMove==0){   
				   while(myList!=NULL){
				       curScore = alphaBeta(myList->myState,depth,-100000,100000,1,0);
				       //curScore1 = minimax(myList->myState,depth,1,0);
				       //if(curScore == curScore1){
				       if(bestScore<curScore){
				          bestScore = curScore;
				          bestMove = myList->myState;
				       }
				       myList = myList->nextNode;
				   }
				   }
				   else{
				   bestMove = myList->myState;
				   }
				   int m,n;
				   for(m=0;m<2;m++){
					   for(n=0;n<MAXIMUM_MOVE_SIZE;n++){
						   myMove.tile[m][n] = bestMove->tile[m][n];
					   }
				   }
				}				
				sendMove( &myMove, mySocket );			//send our move
				//freeTheMallocStates();
				
				
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;

			default:
				printf("Wrong Signal!\n");
				return 0;

			
		}

	} 

	return 0;
}






