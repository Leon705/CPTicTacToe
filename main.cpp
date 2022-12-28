#include <appdef.hpp>
#include <sdk/os/debug.hpp>
#include <sdk/os/input.hpp>
#include <sdk/os/lcd.hpp>
#include <sdk/os/mem.hpp>

//320x520 resolution

APP_NAME("TicTacToe")
APP_DESCRIPTION("simple Tic Tac Toe application")
APP_AUTHOR("Leon705")
APP_VERSION("1.0.0")

struct Field 
{
	int32_t x, y, x2, y2;
	int8_t  letter;
};

Field* NULLFIELD = 0;

Field board[3][3];

int8_t counter = 0;

int8_t PIXELLENGTH = 4;

int8_t LETTER_X[5][5] = {
	{1, 0, 0, 0, 1},
	{0, 1, 0, 1, 0},
	{0, 0, 1, 0, 0},
	{0, 1, 0, 1, 0},
	{1, 0, 0, 0, 1},
};

int8_t LETTER_O[5][5] = {
	{0, 1, 1, 1, 0},
	{1, 0, 0, 0, 1},
	{1, 0, 0, 0, 1},
	{1, 0, 0, 0, 1},
	{0, 1, 1, 1, 0},	
};

int string_length(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = string_length(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void int_to_string(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}



void drawSquare(int x, int y, int color){
	for(int i = 0; i < PIXELLENGTH; i++){
		for(int j = 0; j < PIXELLENGTH; j++){
			LCD_SetPixel(x + i, y + j, color);
		}
	}
}

void drawLetter(int8_t letter[5][5], int x, int y){
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			if(letter[i][j] == 1){
				drawSquare(x+(i*PIXELLENGTH)-2, y+(j*PIXELLENGTH)-2, 0);
			}
		}
	}
}

void drawHorizontalLine(int x, int x2, int y){
	int ln = x2-x;
	int i = 0;
	while(i <= ln){
		LCD_SetPixel(x+i, y, 0);
		i++;
	}
}

void drawVerticalLine(int x, int y, int y2){
	int ln = y2-y;
	int i = 0;
	while(i <= ln){
		LCD_SetPixel(x, y+i, 0);
		i++;
	}
}

void drawSquare(int x, int y, int x2, int y2){
	drawHorizontalLine(x, x2, y);
	drawHorizontalLine(x, x2, y2);
	drawVerticalLine(x, y, y2);
	drawVerticalLine(x2, y, y2);
	LCD_Refresh();
}


void drawBoard(int8_t player){
	Field f;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			f = board[i][j];
			if(f.letter == 1){
				drawLetter(LETTER_X, f.x+(50-(3*PIXELLENGTH)/2)-1, f.y+(50-(3*PIXELLENGTH)/2)-1);
			}else if(f.letter == 2){
				drawLetter(LETTER_O, f.x+(50-(3*PIXELLENGTH)/2)-1, f.y+(50-(3*PIXELLENGTH)/2)-1);		
			}
			Debug_Printf(0,40,false,0,"Player %d's turn", player);
			LCD_Refresh();
		}
	}
}


void init(){
	counter = 0;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			Field f;

			f.x  = i*100+10;
			f.x2 = i*100+110;
			f.y  = j*100 + 110;
			f.y2 = j*100 + 210;
			f.letter = 0;
			board[i][j] = f;
			drawSquare(f.x, f.y, f.x2, f.y2);
		}
	}
}

int checkWin() {
	uint8_t ret = 10;
	for(int player = 1; player < 3; player++){
    	for (int i = 0; i < 3; i++) {
    	    if (board[i][0].letter == player && board[i][1].letter == player && board[i][2].letter == player) {
    	        return (ret+player);
    	    }
    	}
    	for (int i = 0; i < 3; i++) {
    	    if (board[0][i].letter == player && board[1][i].letter == player && board[2][i].letter == player) {
    	        return (ret+player);
    	    }
    	}
    	if (board[0][0].letter == player && board[1][1].letter == player && board[2][2].letter == player) {
    	    return (ret+player);
    	}
    	if (board[0][2].letter == player && board[1][1].letter == player && board[2][0].letter == player) {
    	    return (ret+player);
    	}
	}
    return 0;
}

bool isInField(Field field, int px, int py){
	return (px >= field.x && px <= field.x2 && py >= field.y && py <= field.y2);
}



Field* parseCoords(int px, int py){
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			if(isInField(board[i][j], px, py)){
				Field* rField;
				rField = &board[i][j];
				return rField;
			}
		}
	}
	return 0;
}

bool setField(int8_t player, Field field){
	if(field.letter != 0) return false;
	field.letter = player;
	return true;
}


void gameLoop(){
	struct InputEvent event;
	
	bool running = true;
	int32_t x = 0;
	int32_t y = 0;
	int8_t player = 1;
	int8_t lastBeginner = 1;
	int8_t won = 0; /*0->no win; 1 -> player 1; 2 -> player 2*/
	char playerNumberStr[8];

	Field* field;

	while(running){
		memset(&event, 0, sizeof(event));
		GetInput(&event, 0xFFFFFFFF, 0x10);
		switch(event.type){
		
			case EVENT_TOUCH:
				if(counter == 9){
					LCD_ClearScreen();
					init();
					counter--;
					if(lastBeginner == 1) player = 2; else player = 1;
					lastBeginner = player;
					break;
				}	

				x = event.data.touch_single.p1_x;
				y = event.data.touch_single.p1_y;

				if(event.data.touch_single.direction == TOUCH_UP){
					if(counter == -1){
						counter++;
						break;
					}
						
					field = parseCoords(x,y);
					if(field->letter == 0){
						field->letter = player;
						if(player == 1) player = 2; else player = 1;
						counter++;
					}
				}

			break;
		}

		Debug_Printf(0, 42, false, 0, "Tic Tac Toe by Leon705");

		drawBoard(player);
		/*check win*/
		won = checkWin();
		if(won!=0){
			won = won - 10;
			int_to_string(won, playerNumberStr);
			Debug_SetCursorPosition(1, 2);
			Debug_PrintString((char*)"Player ", false);
			Debug_SetCursorPosition(8, 2);
			Debug_PrintString(playerNumberStr, false);
			Debug_SetCursorPosition(9, 2);
			Debug_PrintString((char*)" won", false);
			LCD_Refresh();
			Debug_WaitKey();
			LCD_ClearScreen();
			if(won == 1) player = 2; else player = 1;
			lastBeginner = player;
			init();
		
		}else{
			if(Input_IsAnyKeyDown()) return;
		}


	}
}

void main() {
	LCD_VRAMBackup();
	LCD_ClearScreen();
	init();
	drawBoard(1);
	LCD_Refresh();
	gameLoop();
	LCD_VRAMRestore();
	LCD_Refresh();
}
