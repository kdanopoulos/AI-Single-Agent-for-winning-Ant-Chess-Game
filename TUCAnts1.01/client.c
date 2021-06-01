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

state * myHead;

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove2;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "test!";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
typed

struct state {
    char board[ BOARD_ROWS ][ BOARD_COLUMNS ];
	int scoreRaise;
	
    struct listOfSons * sons;
	struct listOfSons * myListNode;
};

struct listOfSons {
	struct state * myState;
	struct state * father;
	
	struct listOfSons * nextNode;
	struct listOfSons * prevNode;
};

struct moves {
	char board[ BOARD_ROWS ][ BOARD_COLUMNS ];
	int move[5][2];
	int lenght;
};

struct function {
	list * firstNode;
	int doIHaveCaptureMoves;
};

typedef struct node2 {
	int capture;
	int i;
	int j;
	int direction;
    struct node2 * next;
} list;
/**********************************************************/

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

int alphaBeta(state * possition,int depth,int alpha,int beta,int maximazingPlayer,int score){
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
			eval = minimax(currList->myState,depth-1,alpha,beta,0,score);
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
			eval = minimax(currList->myState,depth-1,alpha,beta,1,score);
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

void stateGenerattion(){
	state * head = (state *) malloc(sizeof(state));
	myHead = head;
	state * curState;
	head->board = gamePosition->board;
	head->scoreRaise = 0;
	head->sons = NULL;
	head->myListNode = NULL;
	
	int doIHaveCaptureMoves = 0;
	int temp;

	//-------------------------- Creating a list of all possible moves in this round --------------------------//
	function * myFunc;
	curState = head; // current State
    myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(myColor,curState);
	list * firstNode = myFunc->firstNode;
	doIHaveCaptureMoves = myFunc->doIHaveCaptureMoves;
	
	//-------------------------- Creating a list of all possible moves in this round --------------------------//
	
	createStatesFromList(curState,firstNode->next,doIHaveCaptureMoves);
	//------------------------------------------------------------------------
	char color = myColor;
		if (color == WHITE)
			color == BLACK;
		else 
			color == WHITE;
		
		listOfSons * currList = curState->sons;
		while(currList!=NULL){
			
			curState = currList->myState;
			myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(color,curState);
			firstNode = myFunc->firstNode;
			doIHaveCaptureMoves = myFunc->doIHaveCaptureMoves;
			createStatesFromList(curState,firstNode->next,doIHaveCaptureMoves);
			
			currList = currList->nextNode;
		}
		//------------------------------------------------------------------------
		if (color == WHITE)
			color == BLACK;
		else 
			color == WHITE;
		currList = head->sons;
		listOfSons * listOfList; 
		while(currList!=NULL){
			 listOfList = currList->myState->sons;
			 while(listOfList!=NULL){
				 curState = listOfList->myState;
				 myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(color,curState);
				 firstNode = myFunc->firstNode;
				 doIHaveCaptureMoves = myFunc->doIHaveCaptureMoves;
				 createStatesFromList(curState,firstNode->next,doIHaveCaptureMoves);
				 listOfList = listOfList->nextNode;
			 }
			 currList = currList->nextNode;
		}
		//------------------------------------------------------------------------
		if (color == WHITE)
			color == BLACK;
		else 
			color == WHITE;
		currList = head->sons;
		listOfSons * listOfListOfList;
		while(currList!=NULL){
			listOfList = currList->myState->sons;
			while(listOfList!=NULL){
				listOfListOfList = listOfList->myState->sons;
				while(listOfListOfList!=NULL){
					curState = listOfListOfList->myState;
					curState = listOfList->myState;
				    myFunc = findAllPossibleMovesForThisRoundAndAddToTheList(color,curState);
				    firstNode = myFunc->firstNode;
				    doIHaveCaptureMoves = myFunc->doIHaveCaptureMoves;
				    createStatesFromList(curState,firstNode->next,doIHaveCaptureMoves);
					listOfListOfList = listOfListOfList->nextNode;
				}
				listOfList = listOfList->nextNode;
			}
			currList = currList->nextNode;
		}
		//------------------------------------------------------------------------
 
}

void createStatesFromList(state * curState,list * firstNodeOfList,int doIHaveCaptureMoves){
	list * curr;
	list * prevNodeList;
	curr = firstNodeOfList;
	state * newState; // the variable used for generating new states
	if(doIHaveCaptureMoves==1){  // if I have at least one capture move (we must always choose capture moves instead of others)
		while(curr!= NULL){ // run until we reach the final possible movement 
			if (curr->capture==1){
				//an direction iso me 3 tote exw 2 states
				// epishs an ginetai prepei na kanw mexti 5 captures sto idio state
				struct moves myMove;
				myMove.board = curState->board;
				myMove.move[0][0] = -1;
				myMove.move[0][1] = -1;
				myMove.move[1][0] = -1;
				myMove.move[1][1] = -1;
				myMove.move[2][0] = -1;
				myMove.move[2][1] = -1;
				myMove.move[3][0] = -1;
				myMove.move[3][1] = -1;
				myMove.move[4][0] = -1;
				myMove.move[4][1] = -1;
				myMove.lenght = -1;
				
				myMove = findMyMaximumMoves(curr->i,cur->j,myMove,1); // we find the best possible movement in possition i,j
				newState = (state *) malloc(sizeof(state));
				newState->board = myMove.board;
				newState->scoreRaise = ;
				newState->sons = NULL;
				newState->myListNode = NULL;
				
				addASonToThisState(curState,newState); // add the new state as son of the current state 
			}
			prevNodeList = curr;
			curr=curr->next;
			free(prevNodeList);
		}
	}
	else { // if we don't have at all capture moves
		// edw den exoume panw apo ena vhma
		//an direction iso me 3 tote exw 2 states
		while(curr!=NULL){
			if (curr->direction==3){  // move to both directions
			        if(myColor == WHITE){	//white color
				        // right				   
				        newState = (state *) malloc(sizeof(state));
				        newState->board = moveWhiteRight(curr->i,curr->j,curState->board);
				        newState->scoreRaise = ;
				        newState->sons = NULL;
				        newState->myListNode = NULL;				   
				        addASonToThisState(curState,newState);				   
				        // left		   				   
				        newState = (state *) malloc(sizeof(state));
				        newState->board = moveWhiteLeft(curr->i,curr->j,curState->board);
				        newState->scoreRaise = ;
				        newState->sons = NULL;
				        newState->myListNode = NULL;			   
				        addASonToThisState(curState,newState);
				    }
				    else{	  //black color				
				        // right				   
				        newState = (state *) malloc(sizeof(state));
				        newState->board = moveBlackRight(curr->i,curr->j,curState->board);
				        newState->scoreRaise = ;
				        newState->sons = NULL;
				        newState->myListNode = NULL;			   
				        addASonToThisState(curState,newState);				   
				        // left
				        newState = (state *) malloc(sizeof(state));
				        newState->board = moveBlackLeft(curr->i,curr->j,curState->board);
				        newState->scoreRaise = ;
				        newState->sons = NULL;
				        newState->myListNode = NULL;				   
				        addASonToThisState(curState,newState);
				    }
			}
			else if (curr->direction==2){   // move to the right			
				    if( myColor == WHITE ){	//white color				
				         newState = (state *) malloc(sizeof(state));
				         newState->board = moveWhiteRight(curr->i,curr->j,curState->board);
				         newState->scoreRaise = ;
				         newState->sons = NULL;
				         newState->myListNode = NULL;				   
				         addASonToThisState(curState,newState);
				    }
				    else {   //black color
				         newState = (state *) malloc(sizeof(state));
				         newState->board = moveBlackRight(curr->i,curr->j,curState->board);
				         newState->scoreRaise = ;
				         newState->sons = NULL;
				         newState->myListNode = NULL;  
				         addASonToThisState(curState,newState);
				    }
			}
			else{   // move to the left			
			        if( myColor == WHITE ){	//white color				   
				        newState = (state *) malloc(sizeof(state));
				        newState->board = moveWhiteLeft(curr->i,curr->j,curState->board);
				        newState->scoreRaise = ;
				        newState->sons = NULL;
				        newState->myListNode = NULL;			   
				        addASonToThisState(curState,newState);
				    }
				    else {   //black color			   
				        newState = (state *) malloc(sizeof(state));
				        newState->board = moveBlackLeft(curr->i,curr->j,curState->board);
				        newState->scoreRaise = ;
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

struct function * findAllPossibleMovesForThisRoundAndAddToTheList(char color, state * currentState){
	//struct function {
	      //list * firstNode;
	      //int doIHaveCaptureMoves;
    //};
	int temp;
	function * myFunc = (function *) malloc(sizeof(function));
    myFunc->doIHaveCaptureMoves = 0;
    myFunc->firstNode = (list *) malloc(sizeof(list));
	myFunc->firstNode->capture = -1;
	myFunc->firstNode->i = -1;
	myFunc->firstNode->j = -1;
	myFunc->firstNode->direction = -1;
	myFunc->firstNode->next = NULL;
	list * curr = myFunc->firstNode;
	
	for( i = 0; i < BOARD_ROWS; i++ )
	{
		for( j = 0; j < BOARD_COLUMNS; j++ )
		{
			if( currentState->board[i][j]== color )	//when we find a piece of ours
			{
				if( color == WHITE )	//white color
				{
					if(i+1 < BOARD_ROWS )
					{
						if( j-1>=0 )
							if( (currentState->board[i+1][j-1]==EMPTY)||(currentState->board[i+1][j-1]==RTILE)){//check if we can move to the left
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
								if (curr->i = i && curr->j = j){
									curr->direction = 3;
								}
								else {
									curr->next = (list *) malloc(sizeof(list));
									curr = curr->next;
								    curr->capture = 0;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
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
								if (curr->i = i && curr->j = j){
									curr->direction = 3;
								}
								else {
									curr->next = (list *) malloc(sizeof(list));
									curr = curr->next;
								    curr->capture = 0;
								    curr->i = i;
								    curr->j = j;
								    curr->next = NULL;
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

int canCapture( char row, char col, char player, char[][] board )
{
	int returnValue = 0;

	assert( ( player ==  WHITE ) || ( player == BLACK ) );
	assert( row >= 0 && row < BOARD_ROWS );
	assert( col >= 0 && col < BOARD_COLUMNS );

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


struct moves findMyMaximumMoves(int i ,int j ,struct moves myMove,int numberOfStep){
	struct moves leftMoves;
	struct moves rightMoves;
	struct moves temp;
	int leftBoolean = 0;
	int rightBoolean = 0;
	
	if (numberOfStep==5)
		return myMove;
	
	if ( myColor == WHITE ){ // white
	    if(i + 2 < BOARD_ROWS){
		    if (j - 2 >= 0){
				if( myMove.board[i+1][j-1] == BLACK &&(( myMove.board[i+2][j-2] == EMPTY) || ( myMove.board[i+2][j-2] == RTILE)))
				{
					temp.board = myMove.board;
					temp.board[i+1][j-1] = EMPTY;
					temp.board[i+2][j-2] = WHITE;
					temp.move[(numberOfStep-1)][0] = i+2;
					temp.move[(numberOfStep-1)][1] = j-2;
					temp.lenght = numberOfStep;
					leftMoves = findMyMaximumMoves(i+2,j-2,temp,numberOfStep++);	//left jump possible
					leftBoolean = 1;
				}
		    }
	        if (j + 2 < BOARD_COLUMNS){
				if( myMove.board[i+1][j+1] == BLACK && ((myMove.board[i+2][j+2] == EMPTY ) || ( myMove.board[i+2][j+2] == RTILE)))
				{
					temp.board = myMove.board;
					temp.board[i+1][j+1] = EMPTY;
					temp.board[i+2][j+2] = WHITE;
					temp.move[(numberOfStep-1)][0] = i+2;
					temp.move[(numberOfStep-1)][1] = j+2;
					temp.lenght = numberOfStep;
					rightMoves = findMyMaximumMoves(i+2,j+2,temp,numberOfStep++);	//right jump possible
					rightBoolean = 1;
				}
		    }
	    }
	}
	else{  // black
	    if(i - 2 >= 0){
			if( j - 2 >= 0 )
			{
				if( myMove.board[i-1][j-1] == WHITE && ((myMove.board[i-2][j-2] == EMPTY )||(myMove.board[i-2][j-2] == RTILE)))
				{
					temp.board = myMove.board;
					temp.board[i-1][j-1] = EMPTY;
					temp.board[i-2][j-2] = WHITE;
					temp.move[(numberOfStep-1)][0] = i-2;
					temp.move[(numberOfStep-1)][1] = j-2;
					temp.lenght = numberOfStep;
					leftMoves = findMyMaximumMoves(i-2,j-2,temp,numberOfStep++);	//left jump possible
					leftBoolean = 1;
				}
			}

			if( j + 2 < BOARD_COLUMNS )
			{
				if( myMove.board[i-1][j+1] == WHITE && ((myMove.board[i-2][j+2] == EMPTY )||(myMove.board[i-2][j+2] == RTILE)))
				{
					temp.board = myMove.board;
					temp.board[i-1][j+1] = EMPTY;
					temp.board[i-2][j+2] = WHITE;
					temp.move[(numberOfStep-1)][0] = i-2;
					temp.move[(numberOfStep-1)][1] = j+2;
					temp.lenght = numberOfStep;
					rightMoves = findMyMaximumMoves(i-2,j+2,temp,numberOfStep++);	//right jump possible
					rightBoolean = 1;
				}
			}
		}
	}
    if (leftBoolean==1 && rightBoolean==1){
		if (leftMoves.lenght>rightMoves.lenght){
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
		return myMove;
	}
    	
}

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

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will send opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				printPosition( &gamePosition );
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove2.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove2.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{

/**********************************************************/
// random player - not the most efficient implementation

					if( myColor == WHITE )		// find movement's direction
						playerDirection = 1;
					else
						playerDirection = -1;

					jumpPossible = FALSE;		// determine if we have a jump available
					for( i = 0; i < BOARD_ROWS; i++ )
					{
						for( j = 0; j < BOARD_COLUMNS; j++ )
						{
							if( gamePosition.board[ i ][ j ] == myColor )
							{
								if( canJump( i, j, myColor, &gamePosition ) )
									jumpPossible = TRUE;
							}
						}
					}

					while( 1 )
					{
						i = rand() % (BOARD_ROWS);
						j = rand() % BOARD_COLUMNS;

						if( gamePosition.board[ i ][ j ] == myColor )		//find a piece of ours
						{

							myMove2.tile[ 0 ][ 0 ] = i;		//piece we are going to move
							myMove2.tile[ 1 ][ 0 ] = j;

							if( jumpPossible == FALSE )
							{
								myMove2.tile[ 0 ][ 1 ] = i + 1 * playerDirection;
								myMove2.tile[ 0 ][ 2 ] = -1;
								if( rand() % 2 == 0 )	//with 50% chance try left and then right
								{
									myMove2.tile[ 1 ][ 1 ] = j - 1;
									if( isLegal( &gamePosition, &myMove2 ))
										break;

									myMove2.tile[ 1 ][ 1 ] = j + 1;
									if( isLegal( &gamePosition, &myMove2 ))
										break;
								}
								else	//the other 50%...try right first and then left
								{
									myMove2.tile[ 1 ][ 1 ] = j + 1;
									if( isLegal( &gamePosition, &myMove2 ))
										break;

									myMove2.tile[ 1 ][ 1 ] = j - 1;
									if( isLegal( &gamePosition, &myMove2 ))
										break;
								}
							}
							else	//jump possible
							{
								if( canJump( i, j, myColor, &gamePosition ) )
								{
									k = 1;
									while( canJump( i, j, myColor, &gamePosition ) != 0 )
									{
										myMove2.tile[ 0 ][ k ] = i + 2 * playerDirection;
										if( rand() % 2 == 0 )	//50% chance
										{

											if( canJump( i, j, myColor, &gamePosition ) % 2 == 1 )		//left jump possible
												myMove2.tile[ 1 ][ k ] = j - 2;
											else
												myMove2.tile[ 1 ][ k ] = j + 2;

										}
										else	//50%
										{

											if( canJump( i, j, myColor, &gamePosition ) > 1 )		//right jump possible
												myMove2.tile[ 1 ][ k ] = j + 2;
											else
												myMove2.tile[ 1 ][ k ] = j - 2;

										}

										if( k + 1 == MAXIMUM_MOVE_SIZE )	//maximum tiles reached
											break;

										myMove2.tile[ 0 ][ k + 1 ] = -1;		//maximum tiles not reached

										i = myMove2.tile[ 0 ][ k ];		//we will try to jump from this point in the next loop
										j = myMove2.tile[ 1 ][ k ];


										k++;
									}
									break;
								}
							}
						}

					}
// end of random
/**********************************************************/

				}

				sendMove( &myMove2, mySocket );			//send our move
				//printf("i chose to go from (%d,%d), to (%d,%d)\n",myMove.tile[0][0],myMove.tile[1][0],myMove.tile[0][1],myMove.tile[1][1]);
				doMove( &gamePosition, &myMove2 );		//play our move on our position
				printPosition( &gamePosition );
				
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 

	return 0;
}






