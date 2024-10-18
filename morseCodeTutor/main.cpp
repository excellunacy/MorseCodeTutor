/*
	MIT License

	Copyright(c) 2024 Evan Peterson, Jeffrey Saied, Katherine Stadnyk

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files(the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions :

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

/**
 * Main code file containing main function and functions for each
 * screen in the program, plus callbacks.
*/

#include "Global.h"

//Global variables used in other files
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Color Pen = { 0,0,0,255 }; //Always opaque black
uint8_t fontSize = 0;
TTF_Font* myFont;
char tempStr[strLength] = { 0 };
char text[strLength] = ""; //Stores ASCII
uint16_t text_i = 0;
uint32_t msPerDit;
uint32_t msPerFDit;
uint16_t wpm;
uint16_t fwpm;
bool fEnable = false; //Determines whether to use Farnsworth timing
uint32_t tolerance;   //3/4 of msPerDit, used in interpreting length of beeps and pauses
uint16_t lastRandomIndex = -1; //Prevents consecutive identical random calls
uint32_t totalDitDuration = 0; //Used for calculating averages
uint32_t totalDahDuration = 0;
uint32_t totalShortPauseDuration = 0;
uint32_t totalCharPauseDuration = 0;
uint32_t totalWordPauseDuration = 0;
uint16_t ditCount = 0;
uint16_t dahCount = 0;
uint16_t shortPauseCount = 0;
uint8_t  charPauseCount = 0;
uint8_t  wordPauseCount = 0;
int straightKey = 'M'; //Key designated as straight key
uint32_t lastChange;
uint16_t downDuration;
uint16_t upDuration;
char input[input_length] = { 0 }; //Stores the Morse Code sequence for a single char
uint8_t input_i = 0;
uint8_t error[8] = { 0 }; //Used for automatically tuning WPM
uint8_t error_i = 0;
bool morseDelete = false; //Flag for deleting the previous word when tapping Morse Code
bool state = UP; //Straight key state
uint8_t choice; //For options
uint16_t beepFreq = 784; //Hz
uint16_t ditSamples[numSamples] = { 0 }; //Used for calculating std dev
uint8_t ditSamples_i = 0;
uint8_t ditSamplesSize = 0;
uint16_t dahSamples[numSamples] = { 0 };
uint8_t dahSamples_i = 0;
uint8_t dahSamplesSize = 0;
uint16_t shortPauseSamples[numSamples] = { 0 };
uint8_t shortPauseSamples_i = 0;
uint8_t shortPauseSamplesSize = 0;
uint16_t charPauseSamples[numSamples] = { 0 };
uint8_t charPauseSamples_i = 0;
uint8_t charPauseSamplesSize = 0;
uint16_t wordPauseSamples[numSamples] = { 0 };
uint8_t wordPauseSamples_i = 0;
uint8_t wordPauseSamplesSize = 0;

//Global variables only used in this file
SDL_AudioSpec audio_spec;
uint8_t* audio_buf;
uint32_t audio_len;
uint32_t audio_time;
SDL_AudioDeviceID audio_device;
SDL_Event event; //Handles all user input
uint8_t mode = start; //Current screen/mode
SDL_TimerID timerID;
//Used for interpreting Morse Code from straight key
enum Action
{
	START = 0, //Initial
	READ_DASHDOT = 1, //Tapping char
	READ_CHARACTER = 2, //Translate char to ASCII
	READ_WORD = 3, //Add space
	PAUSE = 4 //Permanent wait
} action;
char QCodeKeyLong[40] = { 0 }; //Stores prompt
char QCodeKeyShort[7] = { 0 }; //Stores abbreviation answer
uint8_t gameType = 0; //Signifes specific game mode
uint8_t tutorial_level;
char test[] = { ".-,-...,-.-.\0" }; //Morse Code for "ABC" for calibration
uint16_t elapsedUpDuration = 0; //Updates within loop, to detect the type of pause
uint16_t osuDots[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }; //Charts exact length of each Morse Code tap
uint8_t osuDots_i = 0;
volatile bool waiting = false; //Used to track state for playMorse
volatile bool beeping = false;
uint16_t nextTimer = 0;
char playback[strLength * 3] = { 0 }; //Stores Morse Code to be played audibly by playMorse()
uint16_t playbackIndex = 0;


int main(int argc, char* args[])
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		while (true);
	}
	window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		while (true);
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	TTF_Init();
	setFontSize(2);
	if (myFont == NULL)
	{
		printf("Font not loaded: %s\n", TTF_GetError());
	}

	audio_spec.freq = 44100; //Hz data freqency
	audio_spec.format = AUDIO_S16SYS; //Built-in SDL_AudioFormat
	audio_spec.channels = 1;
	audio_spec.samples = 512; //Low to reduce latency on pausing/unpausing
	audio_spec.callback = audioCallback;
	audio_spec.userdata = NULL;
	audio_time = 0; //Used for sine equation. Risks overflowing after ~25 hours of constantly beeping
	SDL_AudioSpec obtained; //Will reflect hardware audio capabilities (output)
	audio_device = SDL_OpenAudioDevice(NULL, false, &audio_spec, &obtained, 0);
	if (audio_device <= 0)
	{
		printf("%s", SDL_GetError());
		while (true);
	}
	if (audio_spec.format != obtained.format)
	{
		while (true);
	}

	setWPM(15); //Default options
	fEnable = false;
	setFWPM(10);
	mode = start; //Initial state values
	srand(time(NULL));

	loop(); //Main loop

	//Close everything
	SDL_PauseAudioDevice(audio_device, true);
	SDL_DestroyWindow(window);
	TTF_CloseFont(myFont);
	SDL_FreeWAV(audio_buf);
	SDL_CloseAudioDevice(audio_device);
	SDL_Quit();
	return 0;
}

void loop()
{
	while (mode != quit)
	{
		switch (mode)
		{
		case start:
			startScreen();
			break;
		case mainMenu:
			mainMenuScreen();
			break;
		case typeAtoM:
			typeAtoMScreen();
			break;
		case transMtoA:
			keyMtoAScreen();
			break;
		case options:
			optionsScreen();
			break;
		case gameModes:
			GameModeSelectScreen();
			break;
		case qCodes:
			QCodesPractice();
			break;
		case hearMorse:
			hearingMorsegame();
			break;
		case mToAGameChoice:
			mToAGameChoiceScreen();
			break;
		case mToAGame:
			MorsetoASCIIGames();
			break;
		case explainMorse:
			explainMorseScreen();
			break;
		case chooseTutorial:
			chooseTutorialScreen();
			break;
		case tutorial:
			tutorialScreen();
			break;
		case tutorialFinish:
			tutorialFinishScreen();
		}
	}
}

void startScreen()
{
	background(255, 255, 255);
	setFontSize(3);
	drawTextCentered("MORSE CODE TUTOR", 0, 120);
	setFontSize(2);
	drawTextCentered("Welcome to the", 0, 90);
	drawTextCentered("Press enter to continue", 0, 300);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_RETURN)
			{
				mode = mainMenu;
				return;
			}
		}
	}
}

void mainMenuScreen()
{
	background(255, 255, 255);
	setFontSize(3);
	drawTextCentered("Main Menu", 0, titleY);
	setFontSize(2);
	drawTextWrapped("[1] Translate Text to Morse\n[2] Translate Morse to Text\n[3] Learn Morse Code\n[4] Game Modes\n[5] Options", listX, listY, SCREEN_WIDTH, lineSpace2);
	SDL_RenderPresent(renderer);

	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case '1':
				mode = typeAtoM;
				return;
			case '2':
				mode = transMtoA;
				calibrationQueryScreen();
				return;
			case '3':
				mode = explainMorse;
				return;
			case '4':
				mode = gameModes;
				return;
			case '5':
				mode = options;
				return;
			}
		}
	}
}

void typeAtoMScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit/stop audio", escX, escY);
	drawTextCenteredWrapped("Type text to be translated\nPress enter to play\nPress tab to reset", 0, instructY + lineSpace2, SCREEN_WIDTH, lineSpace2);
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
	text_i = 0;
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit) //mode == quit in case quit was pressed during playMorse
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				mode = mainMenu;
				return;
			}
			else if (event.key.keysym.sym == SDLK_RETURN) //Play sequence
			{
				text[text_i] = '\0';
				asciiToMorse(text, playback);
				playMorse();
			}
			else if (event.key.keysym.sym == SDLK_BACKSPACE && text_i != 0)
			{
				bkspcHighTextBox();
			}
			else if (event.key.keysym.sym == SDLK_TAB && text_i != 0)
			{
				return; //Reset screen
			}
			else if (text_i < charsPerLine*5) //If screen isn't full & input is a valid char
			{
				typeHighTextBox(keysymToValidChar(&event.key.keysym));
			}
		}
	}
}

void playMorse()
{
	playbackIndex = 0;
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			SDL_RemoveTimer(timerID);
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE) //Stop audio
			{
				SDL_PauseAudioDevice(audio_device, true);
				SDL_RemoveTimer(timerID);
				drawFilledCircle(flashX, flashY, flashR - 3, Pen.r, Pen.g, Pen.b); //Turn off flash
				SDL_RenderPresent(renderer);
				audio_time = 0;
				beeping = false;
				waiting = false;
				return;
			}
			if (mode == typeAtoM || mode == hearMorse) //Allow typing during playback on these modes
			{
				if (event.key.keysym.sym == SDLK_BACKSPACE && text_i != 0)
					bkspcHighTextBox();
				else if ((mode == typeAtoM && text_i < charsPerLine * 5) || (mode == hearMorse && text_i < charsPerLine * 2)) //If screen isn't full & input is a valid char
					typeHighTextBox(keysymToValidChar(&event.key.keysym));
			}
		}
		if (!waiting) //nextTimer must be set. Then the program can wait
		{
			if (!beeping) //Set timer for next beep
			{
				drawFilledCircle(flashX, flashY, flashR - 3, Pen.r, Pen.g, Pen.b); //Turn off flash
				SDL_RenderPresent(renderer);
				if (playbackIndex == 0) //Start
				{
					timerID = SDL_AddTimer(100, timerCallback, NULL); //Begin timer chain. When timerCallback runs, the value it returns sets the next timer length
				}
				switch (playback[playbackIndex])
				{
				case '\0': //End
					SDL_RemoveTimer(timerID);
					return;
				case '.':
					nextTimer = msPerDit;
					break;
				case '-':
					nextTimer = msPerDit * 3;
				}
				playbackIndex++; //Increment index after beep timer is set
			}
			else //beeping. Set timer for next pause
			{
				drawFilledCircle(flashX, flashY, flashR - 3, 255, 255, 255); //Turn on flash
				SDL_RenderPresent(renderer);
				switch (playback[playbackIndex])
				{
				case '\0':
					nextTimer = 0; //Setting a timer of 0 ms cancels the timer
					break;
				case '.':
				case '-':
					nextTimer = msPerDit;
					break;
				case ',':
					if (fEnable)
						nextTimer = msPerFDit * 3;
					else
						nextTimer = msPerDit * 3;
					playbackIndex++; //No beep duration for ','
					break;
				case ' ':
					if (fEnable)
						nextTimer = msPerFDit * 7;
					else
						nextTimer = msPerDit * 7;
					playbackIndex++; //No beep duration for ' '
				}
				//playbackIndex is not incremented if it points to the next dit/dah to be played
			}
			waiting = true;
		}
	}
}

void calibrationQueryScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCenteredWrapped("Do you wish to calibrate the Straight Key WPM?\n\n(y/n)", 0, 175, 300, lineSpace2);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case 'y':
				calibrationScreen();
			case 'n': //Skip to game mode
				return;
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			}
		}
	}
}

void calibrationScreen()
{
RestartCal:
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Key in \'ABC\'", 0, 150);
	//drawMorse(".-,-...,-.-."), but centered
	drawFilledRect(200, 230, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16, 232, 24, 4, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 4, 232, 24, 4, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 6, 230, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 7, 230, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 8, 230, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 10, 232, 24, 4, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 12, 230, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 13, 232, 24, 4, Pen.r, Pen.g, Pen.b);
	drawFilledRect(200 + 16 * 15, 230, 8, 8, Pen.r, Pen.g, Pen.b);
	drawTextCentered("Straight Key set to:", -fontPixels[fontSize], instructY);
	drawTextCentered((char*)&straightKey, fontPixels[fontSize] * 21 / 2, instructY);
	drawTextCentered("Press bkspc to restart", 0, instructY + lineSpace2); //Allows user to retry calibration
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
	uint8_t test_i = 0;
	clearStats(); //Stats are cleared every time straight key input begins
	state = UP; //Reset straight key
	action = START;
	while (test_i < 12)
	{
		SDL_PollEvent(&event);
		checkStraightKey(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				SDL_PauseAudioDevice(audio_device, true);
				mode = mainMenu;
				return;
			case SDLK_BACKSPACE:
				goto RestartCal; //Go to start of function
			}
		}
		if (state == DOWN) //Plays the speaker when the straight key is held down
		{
			drawFilledCircle(flashX, flashY, flashR - 3, 255, 255, 255); //Turn on flash
			SDL_PauseAudioDevice(audio_device, false);
		}
		else
		{
			drawFilledCircle(flashX, flashY, flashR - 3, Pen.r, Pen.g, Pen.b); //Turn off flash
			SDL_PauseAudioDevice(audio_device, true);
		}
		SDL_RenderPresent(renderer);
		if (state == UP) //If we are at a pause
		{
			if (action == READ_DASHDOT) //The key was pressed just before now
			{
				if (test[test_i] == '.') //Checks what input should be and adjusts stats accordingly
					totalDitDuration += downDuration;
				else if (test[test_i] == '-')
					totalDahDuration += downDuration;
				test_i++; //Increment index after each tap
				action = PAUSE;
			}
		}
		else //(state == DOWN)
		{
			if (test_i > 0 && action == PAUSE) //This isn't the first tap
			{
				if (test[test_i] == ',') //Skip ',' for inter-char pause
					test_i++;
				else
					totalShortPauseDuration += upDuration; //Ignore pauses between characters
			}
			action = READ_DASHDOT;
		}
	}
	//Check the stats
	float dahDitRatio = 3.25 * totalDahDuration / (totalShortPauseDuration + totalDitDuration); //3.25 = (1/4 dahs)/(1/13 dits and intra-char pauses)
	if (2.25 < dahDitRatio && dahDitRatio < 4.0) //Ratio of dah length to dit/intra-char pause length must be between 2.25-4.0. This ensures the user correctly tapped "ABC" with somewhat accurate rhythm.
	{
		setWPM(round(30000.0 / (totalShortPauseDuration + totalDitDuration + totalDahDuration))); //25 time units make up ABC, not counting inter char pauses. This formula corverts the total time into the WPM (30000 = 1200*25)
		drawTextCentered("WPM set to:", fontPixels[fontSize] * -3 / 2, instructY + lineSpace2 * 2);
		sprintf(tempStr, "%2d", wpm);
		drawTextCentered(tempStr, 6 * fontPixels[fontSize], instructY + lineSpace2 * 2);
		SDL_RenderPresent(renderer);
		SDL_Delay(500); //Slight delay before audio demo
		strcpy(playback, test); //Load Morse Code into playback
		playMorse(); //Demo the set WPM
		drawTextCentered("Press enter to continue", 0, instructY + lineSpace2 * 3);
		SDL_RenderPresent(renderer);
		while (true)
		{
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT || mode == quit)
			{
				mode = quit;
				return;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					mode = mainMenu;
				case SDLK_RETURN:
					return; //Unlike most screens, this function is called by another screen
				case SDLK_BACKSPACE:
					goto RestartCal; //Repeat entire function
				}
			}
		}
	}
	else
	{
		drawTextCentered("Invalid calibration", 0, instructY + lineSpace2 * 2);
		SDL_RenderPresent(renderer);
		while (true)
		{
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT || mode == quit)
			{
				mode = quit;
				return;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym) //User must repeat calibration or escape
				{
				case SDLK_ESCAPE:
					mode = mainMenu;
					return;
				case SDLK_BACKSPACE:
					goto RestartCal; //Repeat entire function
				}
			}
		}
	}
}

void keyMtoAScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCenteredWrapped("Tap Morse Code to be translated\n\nPress space to show stats\nPress bkspc to clear", 0, instructY - lineSpace2, SCREEN_WIDTH, lineSpace2);
	drawTextCentered("Straight Key set to:", -fontPixels[fontSize], instructY);
	drawTextCentered((char*)&straightKey, fontPixels[fontSize] * 21 / 2, instructY);
	drawOsuChart();
	setFontSize(1);
	drawTextWrapped("Use prosign \"HH\" to delete the last word", 500, escY, 140, lineSpace2);
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	action = START;
	text_i = 0;
	input_i = 0;
	SDL_RenderPresent(renderer);
	setFontSize(2);
	clearStats(); //Stats are cleared every time straight key input begins
	state = UP; //Reset straight key
	while (true)
	{
		SDL_PollEvent(&event);
		checkStraightKey(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				SDL_PauseAudioDevice(audio_device, true);
				mode = mainMenu;
				return;
			case SDLK_SPACE: //Enter displays stats for straight key input (average & standard deviation for dits & dahs, etc.)
				SDL_PauseAudioDevice(audio_device, true);
				showStats();
				return;
			case SDLK_BACKSPACE: //Clear the screen
				if (text_i != 0)
					return;
				break;
			}
		}
		if (state == DOWN) //Plays the speaker when the straight key is held down
		{
			drawFilledCircle(flashX, flashY, flashR - 3, 255, 255, 255); //Turn on flash
			SDL_PauseAudioDevice(audio_device, false);
		}
		else
		{
			drawFilledCircle(flashX, flashY, flashR - 3, Pen.r, Pen.g, Pen.b); //Turn off flash
			SDL_PauseAudioDevice(audio_device, true);
		}
		SDL_RenderPresent(renderer);
		if (text_i < charsPerLine * 5) //Straight key input stops being read once the screen fills up
		{
			if (state == UP) //If we are at a pause
			{
				elapsedUpDuration = (SDL_GetTicks() - lastChange); //A variable separate from upDuration is needed so that this never updates upDuration immediately after the falling edge
				if (action == READ_DASHDOT && elapsedUpDuration >= (msPerDit - tolerance)) //If an intra-character pause occurs (>= 0.25 time units)
				{
					readDashDot();
					//Visual indicator on bottom of screen shows exact length of each tap, inspired by Osu!
					if (osuDots[osuDots_i] <= SCREEN_WIDTH - 2)
						drawFilledRect(osuDots[osuDots_i] - 2, 442, 4, 8, 255, 255, 255); //Remove old dot
					if (120 * downDuration / msPerDit + 40 < SCREEN_WIDTH - 2) //If new dot would appear on screen
					{
						osuDots[osuDots_i] = 120 * downDuration / msPerDit + 40;
						drawFilledRect(osuDots[osuDots_i] - 2, 442, 4, 8, Pen.r, Pen.g, Pen.b); //Draw new dot
					}
					else
						osuDots[osuDots_i] = -1; //Default offscreen pos, prevents overflow when setting value
					osuDots_i++;
					osuDots_i = osuDots_i & 15; //Cheap mod 16 operation
					action = READ_CHARACTER; //Check for inter-char pause next
					SDL_RenderPresent(renderer);
				}
				else if (action == READ_CHARACTER && elapsedUpDuration >= 2 * msPerDit) //If an inter-character pause occurs (>= 2 time units)
				{
					input[input_i] = '\0';
					int c = morseToAscii(); //Determine character that was tapped. int here ensures null termination when cast as a char*
					if (c != 0) //If a valid character is detected, add to screen
					{
						typeHighTextBox(c);
						//Automatic tuning of WPM is checked every time a char is successfully tapped
						bool consistentError = true; //True if every value in error is the same
						for (uint8_t i = 1; i < 8; i++)
						{
							if (error[i] != error[0])
							{
								consistentError = false;
								break;
							}
						}
						if (consistentError)
						{
							if (error[0] == fast) //User is tapping consistently fast
								setWPM(wpm + 1);
							else //User is tapping consistently slow
								setWPM(wpm - 1);
							error[0] = 0; //Reset record after WPM update to avoid multiple adjacent changes
							error[1] = 0;
							error[2] = 0;
							error[3] = 0;
							error[4] = 0;
							error[5] = 0;
							error[6] = 0;
							error[7] = 0;
						}
					}
					input_i = 0;
					action = READ_WORD; //Check for inter-word pause next
				}
				else if (action == READ_WORD && elapsedUpDuration >= 5 * msPerDit) //If an inter-word pause occurs (>= 5 time units)
				{
					if(text[text_i-1]!=' ') //Prevents consecutive spaces
					{
						text[text_i] = ' ';
						text_i++;
					}
					action = PAUSE; //Stop reading pauses
				}
				if (morseDelete) //Delete previous word
				{
					//Delete last char (in case last is ' '), then keep deleting until a ' ' is found
					do
					{
						bkspcHighTextBox();
					} while (text_i != 0 && text[text_i - 1] != ' ');
					action = PAUSE; //Stop reading pauses
					morseDelete = false;
				}
			}
			else //(state == DOWN) If the straight key is being pressed, a dit/dah is being played
			{
				if (action == READ_CHARACTER) //An intra-char pause was recorded
				{
					totalShortPauseDuration += upDuration; //Stats are taken
					shortPauseCount++;
					if (upDuration < msPerDit) //error only updates for dits, dahs, and intra-char pauses
						error[error_i] = fast;
					else
						error[error_i] = slow;
					error_i++;
					error_i = error_i & 7; //Cheap mod 8 operation
					shortPauseSamples[shortPauseSamples_i] = upDuration;
					shortPauseSamples_i++;
					if (shortPauseSamples_i == numSamples)
						shortPauseSamples_i = 0;
					if (shortPauseSamplesSize < numSamples + 1)
						shortPauseSamplesSize++;
				}
				if (action == READ_WORD) //A char pause was recorded
				{
					totalCharPauseDuration += upDuration;
					charPauseCount++;
					charPauseSamples[charPauseSamples_i] = upDuration;
					charPauseSamples_i++;
					if (charPauseSamples_i == numSamples)
						charPauseSamples_i = 0;
					if (charPauseSamplesSize < numSamples + 1)
						charPauseSamplesSize++;
				}
				if (action == PAUSE) //A space was recorded
				{
					totalWordPauseDuration += upDuration;
					wordPauseCount++;
					wordPauseSamples[wordPauseSamples_i] = upDuration;
					wordPauseSamples_i++;
					if (wordPauseSamples_i == numSamples)
						wordPauseSamples_i = 0;
					if (wordPauseSamplesSize < numSamples + 1)
						wordPauseSamplesSize++;
				}
				action = READ_DASHDOT;
			}
		}
	}
}

void showStats()
{
	background(255, 255, 255);
	setFontSize(2);
	drawTextCentered("Morse Code Tapping Stats", 0, 60);
	drawText("Esc to exit", escX, escY);
	drawText("Average Dit:", listX, listY);
	uint16_t tempAvg;
	if (ditCount != 0) //If average is ever NaN, it defaults to 0
		tempAvg = totalDitDuration / ditCount; //tempAvg is in units of ms
	else
		tempAvg = 0;
	sprintf(tempStr, "%d.%03ds", (tempAvg / 1000) % 10, tempAvg % 1000); //Display four digits. Value shouldn't exceed 10 seconds. If it does, it will display inaccurately
	drawText(tempStr, listX + fontPixels[fontSize] * 20, listY);
	drawStdDev(ditSamples, ditSamplesSize, tempAvg, listY + lineSpace2);

	drawText("Average Dah:", listX, listY + lineSpace3 + lineSpace2);
	if (dahCount != 0)
		tempAvg = totalDahDuration / dahCount;
	else
		tempAvg = 0;
	sprintf(tempStr, "%d.%03ds", (tempAvg / 1000) % 10, tempAvg % 1000);
	drawText(tempStr, listX + fontPixels[fontSize] * 20, listY + lineSpace3 + lineSpace2);
	drawStdDev(dahSamples, dahSamplesSize, tempAvg, listY + lineSpace3 + lineSpace2 * 2);

	drawText("Average Tap Pause: ", listX, listY + lineSpace3 * 2 + lineSpace2 * 2);
	if (shortPauseCount != 0)
		tempAvg = totalShortPauseDuration / shortPauseCount;
	else
		tempAvg = 0;
	sprintf(tempStr, "%d.%03ds", (tempAvg / 1000) % 10, tempAvg % 1000);
	drawText(tempStr, listX + fontPixels[fontSize] * 20, listY + lineSpace3 * 2 + lineSpace2 * 2);
	drawStdDev(shortPauseSamples, shortPauseSamplesSize, tempAvg, listY + lineSpace3 * 2 + lineSpace2 * 3);

	drawText("Average Char Pause: ", listX, listY + lineSpace3 * 3 + lineSpace2 * 3);
	if (charPauseCount != 0)
		tempAvg = totalCharPauseDuration / charPauseCount;
	else
		tempAvg = 0;
	sprintf(tempStr, "%d.%03ds", (tempAvg / 1000) % 10, tempAvg % 1000);
	drawText(tempStr, listX + fontPixels[fontSize] * 20, listY + lineSpace3 * 3 + lineSpace2 * 3);
	drawStdDev(charPauseSamples, charPauseSamplesSize, tempAvg, listY + lineSpace3 * 3 + lineSpace2 * 4);

	drawText("Average Space: ", listX, listY + lineSpace3 * 4 + lineSpace2 * 4);
	if (wordPauseCount != 0)
		tempAvg = totalWordPauseDuration / wordPauseCount;
	else
		tempAvg = 0;
	sprintf(tempStr, "%d.%03ds", (tempAvg / 1000) % 10, tempAvg % 1000);
	drawText(tempStr, listX + fontPixels[fontSize] * 20, listY + lineSpace3 * 4 + lineSpace2 * 4);
	drawStdDev(wordPauseSamples, wordPauseSamplesSize, tempAvg, listY + lineSpace3 * 4 + lineSpace2 * 5);

	drawText("Actual WPM: ", listX, listY + lineSpace3 * 5 + lineSpace2 * 5); //This value is calculated based on total time units over total time
	if (ditCount + dahCount != 0)
		tempAvg = 120000 * (ditCount + dahCount * 3 + shortPauseCount + charPauseCount * 3 + wordPauseCount * 7) / (totalDitDuration + totalDahDuration + totalShortPauseDuration + totalCharPauseDuration + totalWordPauseDuration); //WPM * 100
	else
		tempAvg = 0;
	sprintf(tempStr, "%2d.%02d", (tempAvg / 100) % 100, tempAvg % 100);
	drawText(tempStr, listX + fontPixels[fontSize] * 20, listY + lineSpace3 * 5 + lineSpace2 * 5);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
		{
			return;
		}
	}
}

void optionsScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextWrapped("PARIS WPM:\nEnable Farnsworth Timing\nFarnsworth WPM:\nSpeaker Frequency:\n    (Default Freq. = 784)\nPlayback speed presets:", listX, 50, SCREEN_WIDTH, lineSpace3);
	drawTextCentered("Beginner", -150, 50 + 6 * lineSpace3);
	drawTextCentered("Medium", 0, 50 + 6 * lineSpace3);
	drawTextCentered("Expert", 150, 50 + 6 * lineSpace3);
	drawText("Straight Key Key:", listX, 50 + lineSpace3 * 7);
	drawTextCenteredWrapped("Use arrow keys to navigate\nEnter to choose/toggle\nModify #s with bkspc & 0-9", 0, 380, SCREEN_WIDTH, lineSpace2);
	drawRect(500, 90, 28, 28, Pen.r, Pen.g, Pen.b);
	drawOption(0);
	if (!fEnable) //Weird rule: On initialization if fEnable, drawOption(2) need not be called. Else, drawOption(2) must be called before drawOption(1)
		drawOption(2);
	drawOption(1);
	drawOption(3);
	drawOption(7);
	updateOptionChoice(0);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				drawFilledRect(0, 345, 640, lineSpace2, 255, 255, 255); //Delete invalid settings to prevent overlapped drawing
				if (wpm == 0 || (fEnable && (fwpm == 0 || wpm < fwpm)) || beepFreq < 150 || beepFreq > 3000)
				{
					drawTextCentered("Invalid settings", 0, 340);
					SDL_RenderPresent(renderer);
				}
				else
				{
					mode = mainMenu;
					return;
				}
				break;
			case SDLK_UP:
				if (choice == 0)
					updateOptionChoice(7);
				else if (choice == 5 || choice == 6)
					updateOptionChoice(3);
				else if (choice == 3 && !fEnable)
					updateOptionChoice(choice - 2);
				else if (choice == 7)
					updateOptionChoice(4);
				else //if (choice == 1 || choice == 2 || (choice == 3 && fEnable) || choice == 4)
					updateOptionChoice(choice - 1);
				break;
			case SDLK_DOWN:
				if (choice == 4 || choice == 5)
					updateOptionChoice(7);
				else if (choice == 1 && !fEnable)
					updateOptionChoice(choice + 2);
				else if (choice == 7)
					updateOptionChoice(0);
				else //if(choice == 0 || (choice == 1 && fEnable) || choice == 2 || choice == 3 || choice == 6)
					updateOptionChoice(choice + 1);
				break;
			case SDLK_LEFT:
				if (choice == 5 || choice == 6)
					updateOptionChoice(choice - 1);
				break;
			case SDLK_RIGHT:
				if (choice == 4 || choice == 5)
					updateOptionChoice(choice + 1);
				break;
			case SDLK_RETURN: //Used to toggle Farnsworth timing or select a preset speed
				if (choice == 1)
				{
					fEnable = !fEnable;
					drawOption(choice);
				}
				else if (choice == 4)
				{
					setWPM(12);
					fEnable = true;
					setFWPM(6);
					drawOption(0);
					drawOption(1);
				}
				else if (choice == 5)
				{
					setWPM(18);
					fEnable = true;
					setFWPM(9);
					drawOption(0);
					drawOption(1);
				}
				else if (choice == 6)
				{
					setWPM(25);
					fEnable = false;
					drawOption(0);
					drawOption(2);
					drawOption(1);
				}
				break;
			case SDLK_BACKSPACE:
				if (choice == 0 && wpm > 0)
				{
					setWPM(wpm / 10); //Delete least significant digit
					drawOption(choice);
				}
				else if (choice == 2 && fwpm > 0)
				{
					setFWPM(fwpm / 10); //Delete least significant digit
					drawOption(choice);
				}
				else if (choice == 3 && beepFreq > 0)
				{
					beepFreq /= 10; //Delete least significant digit
					drawOption(choice);
				}
				break;
			default:
				if (event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9')
				{
					if (choice == 0 && wpm < 10) //WPM must be 2 digits or less
					{
						setWPM(wpm * 10 + (event.key.keysym.sym - '0')); //Shift numbers left and add new significant digit (char converted to int)
						drawOption(choice);
					}
					else if (choice == 2 && fwpm < 10) //FWPM must be 2 digits or less
					{
						setFWPM(fwpm * 10 + (event.key.keysym.sym - '0'));
						drawOption(choice);
					}
					else if (choice == 3 && beepFreq < 1000) //Beep frequency must be 4 digits or less
					{
						beepFreq = beepFreq * 10 + (event.key.keysym.sym - '0');
						drawOption(choice);
					}
				}
				if (choice == 7 && event.key.keysym.sym < 256 && event.key.keysym.sym != SDLK_TAB && event.key.keysym.sym != SDLK_SPACE) //straightKey cannot interfere with other controls
				{
					straightKey = (char)toupper(event.key.keysym.sym); //Straight key is displayed uppercase, but set to lowercase inside checkStraightKey
					drawOption(7);
				}
			}
		}
	}
}

void GameModeSelectScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Select Game Mode", 0, titleY);
	drawTextWrapped("[1] Interpreting Morse Code\n[2] Tapping Morse Code\n[3] Learn Q-codes & Abbreviations", listX, listY, SCREEN_WIDTH, lineSpace2);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			case '1':
				mode = hearMorse;
				return;
			case '2':
				mode = mToAGameChoice;
				return;
			case '3':
				mode = qCodes;
				return;
			}
		}
	}
}

void QCodesPractice()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Select an Option", 0, titleY);
	drawTextWrapped("[1] Learn Q-codes\n[2] Learn Popular Abbreviations", listX, listY, SCREEN_WIDTH, lineSpace2);
	SDL_RenderPresent(renderer);
	uint8_t randomValue;
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			//'1' and '2' are used for gameType to distinguish from MORSE_CHARACTERS, MORSE_WORDS, and MORSE_PHRASES when it comes to preventing duplicate RNG calls
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = gameModes;
				return;
			case '1':
			qCodepick:
				if (gameType != '1') //If game has changed, we don't need lastRandomIndex
					lastRandomIndex = -1;
				gameType = '1';
				do
				{
					randomValue = rand() % 16;
				} while (randomValue == lastRandomIndex);
				lastRandomIndex = randomValue;
				strcpy(QCodeKeyLong, qCodePhrases[randomValue]);
				strcpy(QCodeKeyShort, qCodeCodes[randomValue]);
				goto Exit;
				break;
			case '2':
			abbrevpick:
				if (gameType != '2')
					lastRandomIndex = -1;
				gameType = '2';
				do
				{
					randomValue = rand() % 16;
				} while (randomValue == lastRandomIndex);
				lastRandomIndex = randomValue;
				strcpy(QCodeKeyLong, PopAbb[randomValue]);
				strcpy(QCodeKeyShort, PopAbbKey[randomValue]);
				goto Exit;
				break;
			}
		}
	}
Exit:
	background(255, 255, 255);
	drawText("What's the code for", listX, titleY);
	drawTextWrapped(QCodeKeyLong, listX + 20, 100, fontPixels[fontSize] * charsPerLine + 2, lineSpace2);
	drawText("Esc to exit", escX, escY);
	SDL_RenderPresent(renderer);
	text_i = 0;
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				mode = mainMenu;
				return;
			}
			else if (event.key.keysym.sym == SDLK_RETURN)
			{
				text[text_i] = '\0';
				mode = seeResult; //Prevents tapping during accuracy check
				QCodeCheckAccuracy(QCodeKeyShort, text, QCodeKeyLong);
				mode = qCodes;
				break;
			}
			else if (event.key.keysym.sym == SDLK_BACKSPACE && text_i != 0)
			{
				bkspcLowTextBox();
			}
			else if (text_i < 6 && 'a' <= event.key.keysym.sym && event.key.keysym.sym <= 'z') //Only letters can be typed, and no more than 6
			{
				typeLowTextBox(toupper(event.key.keysym.sym));
			}
		}
	}
	while (true) //QCodeCheckAccuracy has been called and the answer has been shown
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_RETURN:
				if (gameType == '1')
					goto qCodepick;
				else //gameType == '2'
					goto abbrevpick;
			case SDLK_TAB:
				asciiToMorse(QCodeKeyShort, playback);
				playMorse();
				break;
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			}
		}
	}
}

void QCodeCheckAccuracy(char* QCodeKey, char* QCodeAns, char*QCodePhrase)
{
	bool incorrect = strcmp(QCodeKey, QCodeAns) != 0;
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit/stop audio", escX, escY);
	if (incorrect)
		drawColorTextCentered("Incorrect!", 0, titleY, 255, 0, 0);
	else
		drawColorTextCentered("Correct!", 0, titleY, 12, 150, 49);
	drawColorText("The phrase was", listX, 80, 57, 57, 57);
	drawTextWrapped(QCodePhrase, listX, 80 + lineSpace2, SCREEN_WIDTH - (listX * 2), lineSpace2);
	drawColorText("You entered", listX, 80 + lineSpace2 * 3, 57, 57, 57);
	drawTextWrapped(QCodeAns, listX, 80 + lineSpace2 * 4, SCREEN_WIDTH - (listX * 2), lineSpace2);
	drawColorText("The correct answer is", listX, 80 + lineSpace2 * 6, 57, 57, 57);
	drawColorText(QCodeKey, listX, 80 + lineSpace2 * 7, 57, 57, 57);
	drawTextCenteredWrapped("Press tab to hear Morse Code\nPress enter to continue", 0, 80 + lineSpace2 * 11, SCREEN_WIDTH, lineSpace2);
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
}

void hearingMorsegame()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Select an Option", 0, titleY);
	drawTextWrapped("[1] Interpret letters\n[2] Interpret words\n[3] Interpret phrases", listX, listY, SCREEN_WIDTH, lineSpace2);
	SDL_RenderPresent(renderer);
	uint8_t randomValue;
	char ASCIIRANDOM[31];
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		if (event.type == SDL_KEYDOWN)
		{

			switch (event.key.keysym.sym)
			{
			case '1':
				if (gameType != MORSE_CHARACTERS) //If game has changed, we don't need lastRandomIndex
					lastRandomIndex = -1;
				gameType = MORSE_CHARACTERS;
			charpick:
				do
				{
					randomValue = (rand() % 26) + 65;  //Picks A-Z
				} while (randomValue == lastRandomIndex);
				lastRandomIndex = randomValue;
				ASCIIRANDOM[0] = randomValue;
				ASCIIRANDOM[1] = '\0';
				strcpy(playback, myMorse[randomValue-32]); //Shift char to proper index in myMorse
				goto leave;
			case '2':
				if (gameType != MORSE_WORDS)
					lastRandomIndex = -1;
				gameType = MORSE_WORDS;
			wordpick:
				do
				{
					randomValue = rand() % numWords;
				} while (randomValue == lastRandomIndex);
				lastRandomIndex = randomValue;
				strcpy(ASCIIRANDOM, words[randomValue]);
				asciiToMorse(ASCIIRANDOM, playback);
				goto leave;
			case '3':
				if (gameType != MORSE_PHRASES)
					lastRandomIndex = -1;
				gameType = MORSE_PHRASES;
			phrasepick:
				do
				{
					randomValue = rand() % numPhrases;
				} while (randomValue == lastRandomIndex);
				lastRandomIndex = randomValue;
				strcpy(ASCIIRANDOM, phrases[randomValue]);
				asciiToMorse(ASCIIRANDOM, playback);
				goto leave;
			case SDLK_ESCAPE:
				mode = gameModes;
				return;
			}
		}
	}
leave:
	mode = hearMorse;
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit/stop audio", escX, escY);
	drawTextCenteredWrapped("Type the Heard Morse\nPress tab to repeat\nPress enter to check", 0, instructY, SCREEN_WIDTH, lineSpace2);
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
	text_i = 0;
	playMorse();
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit) //mode == quit in case quit was pressed during playMorse
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				mode = mainMenu;
				return;
			}
			else if (event.key.keysym.sym == SDLK_TAB)
			{
				playMorse();
			}
			else if (event.key.keysym.sym == SDLK_RETURN)
			{
				text[text_i] = '\0';
				mode = seeResult; //Prevents tapping during accuracy check
				CheckAccuracy(playback, text, ASCIIRANDOM);
				break;
			}
			else if (event.key.keysym.sym == SDLK_BACKSPACE && text_i != 0)
			{
				bkspcHighTextBox();
			}
			else if (text_i < charsPerLine*2 && (text_i < 1 || gameType != MORSE_CHARACTERS)) //If screen isn't full & input is a valid char
			{
				typeHighTextBox(keysymToValidChar(&event.key.keysym));
			}
		}
	}
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_RETURN:
				switch (gameType) //Go to the respective case where the random value is picked to get a new prompt
				{
				case 1:
					goto charpick;
				case 2:
					goto wordpick;
				case 3:
					goto phrasepick;
				}
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			case SDLK_TAB:
				playMorse();
			}
		}
	}
}

void CheckAccuracy(char* keyMorse, char* enteredASCII, char* keyASCII)
{
	int j = 0;
	int correct = 0;
	int incorrect = 0;
	int total = 0;
	float percentCorrect = 0;
	for (int i = 0; keyASCII[i] != '\0'; i++) //Accuracy is based on ASCII characters, not Morse Code beeps
	{
		if (keyASCII[i] == enteredASCII[i])
		{
			correct++;
		}
		else
		{
			incorrect++;
			if (gameType == MORSE_CHARACTERS)
				break;
		}
		if (enteredASCII[i] == '\0')
		{
			enteredASCII[i + 1] = '\0'; //Clears leftover chars from after the null terminator
		}
	}
	if (gameType != MORSE_CHARACTERS)
	{
		total = correct + incorrect;
		percentCorrect = ((float)correct / (float)total) * 100;
		sprintf(tempStr, "%.2f%% accuracy", percentCorrect);
	}
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit/stop audio", escX, escY);
	drawTextCenteredWrapped("Press enter to continue\nPress tab to repeat Morse", 0, instructY + lineSpace2, SCREEN_WIDTH, lineSpace2);
	drawColorText("You Entered:", listX, 80, 57, 57, 57);
	drawTextWrapped(enteredASCII, listX, 80 + lineSpace2, SCREEN_WIDTH - (listX * 2), lineSpace2);
	drawColorText("Played Morse: ", listX, 80 + lineSpace2 * 3, 57, 57, 57);
	drawTextWrapped(keyASCII, listX, 80 + lineSpace2 * 4, SCREEN_WIDTH - (listX * 2), lineSpace2);
	drawMorse(keyMorse);
	if (incorrect >= 1)
	{
		drawColorTextCentered("Incorrect!", 0, titleY, 255, 0, 0);
		if (gameType != MORSE_CHARACTERS)
			drawText(tempStr, 100, 80 + lineSpace2 * 10); //Display accuracy percentage
	}
	else
	{
		drawColorTextCentered("Correct!", 0, titleY, 12, 150, 49);
	}
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
}

void mToAGameChoiceScreen()
{
	clearStats(); //Stats are cleared when a new game is selected, not when the user repeatedly plays a game
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Select an Option", 0, titleY);
	drawTextWrapped("[1] Tap out letters\n[2] Tap out words\n[3] Tap out phrases", listX, listY, SCREEN_WIDTH, lineSpace2);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = gameModes;
				return;
			case '1':
				if (gameType != MORSE_CHARACTERS) //If game has changed, we don't need lastRandomIndex
					lastRandomIndex = -1;
				gameType = MORSE_CHARACTERS;
				mode = mToAGame;
				calibrationQueryScreen();
				return;
			case '2':
				if (gameType != MORSE_WORDS)
					lastRandomIndex = -1;
				gameType = MORSE_WORDS;
				mode = mToAGame;
				calibrationQueryScreen();
				return;
			case '3':
				if (gameType != MORSE_PHRASES)
					lastRandomIndex = -1;
				gameType = MORSE_PHRASES;
				mode = mToAGame;
				calibrationQueryScreen();
				return;
			}
		}
	}
}

void MorsetoASCIIGames()
{
	char targetInput[charsPerLine * 2 + 1] = { 0 };
	int randomIndex;
	if (gameType == MORSE_CHARACTERS)
	{
		do
		{
			randomIndex = (rand() % 26) + 65; //Picks A-Z
		} while (randomIndex == lastRandomIndex);
		lastRandomIndex = randomIndex;
		targetInput[0] = randomIndex;
		targetInput[1] = '\0';
	}
	else if (gameType == MORSE_WORDS)
	{
		do
		{
			randomIndex = rand() % numWords;
		} while (randomIndex == lastRandomIndex);
		lastRandomIndex = randomIndex;
		strcpy(targetInput, words[randomIndex]);
	}
	else
	{
		do
		{
			randomIndex = rand() % numPhrases;
		} while (randomIndex == lastRandomIndex);
		lastRandomIndex = randomIndex;
		strcpy(targetInput, phrases[randomIndex]);
	}
	uint8_t targetLength = 1;
	while (targetInput[targetLength] != '\0') //The length of targetInput is used to know when input is complete for words and phrases
		targetLength++;
Retry:
	input_i = 0;
	text_i = 0;
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit/stop audio", escX, escY);
	if (gameType == MORSE_CHARACTERS)
	{
		setFontSize(3);
		drawTextCentered(targetInput, 0, titleY);
		setFontSize(2);
	}
	else
	{
		drawTextWrapped(targetInput, listX, titleY, fontPixels[fontSize] * charsPerShortLine + 2, lineSpace2);
	}
	drawTextCentered("Tap out the text above", 0, instructY - lineSpace2);
	drawTextCentered("Straight Key set to:", -fontPixels[fontSize], instructY);
	drawTextCentered((char*)&straightKey, fontPixels[fontSize] * 21 / 2, instructY);
	if (gameType != MORSE_CHARACTERS)
	{
		drawTextCentered("Press enter to Skip", 0, instructY + lineSpace2);
		drawTextCentered("Press bkspc to reset", 0, instructY + lineSpace2 * 2);
	}
	if (gameType == MORSE_WORDS)
	{
		setFontSize(1);
		drawTextWrapped("Use prosign \"HH\" to reset", 500, escY, 140, lineSpace2);
	}
	else if (gameType == MORSE_PHRASES)
	{
		setFontSize(1);
		drawTextWrapped("Use prosign \"HH\" or tap \"II\" to delete the last word", 500, escY, 140, lineSpace2);
	}
	drawOsuChart();
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
	action = START;
	bool inputComplete = false;
	setFontSize(2);
	state = UP; //Reset straight key
	while (!inputComplete)
	{
		SDL_PollEvent(&event);
		checkStraightKey(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				SDL_PauseAudioDevice(audio_device, true);
				mode = mainMenu;
				return;
			case SDLK_BACKSPACE: //Clear the screen
				if (text_i != 0 && gameType == MORSE_PHRASES || MORSE_WORDS)
					goto Retry;
				break;
			case SDLK_RETURN: //Skip the current word or phrase, so that the user doesn't get stuck
				if (gameType == MORSE_PHRASES || MORSE_WORDS)
					return;
			}
		}
		if (state == DOWN) //Plays the speaker when the straight key is held down
		{
			drawFilledCircle(flashX, flashY, flashR - 3, 255, 255, 255); //Turn on flash
			SDL_PauseAudioDevice(audio_device, false);
		}
		else
		{
			drawFilledCircle(flashX, flashY, flashR - 3, Pen.r, Pen.g, Pen.b); //Turn off flash
			SDL_PauseAudioDevice(audio_device, true);
		}
		SDL_RenderPresent(renderer);
		if (text_i < charsPerLine * 2) //Straight key input stops being read once 2 lines are full
		{
			if (state == UP) //If we are at a pause
			{
				elapsedUpDuration = (SDL_GetTicks() - lastChange); //A variable separate from upDuration is needed so that this never updates upDuration immediately after the falling edge
				if (action == READ_DASHDOT && elapsedUpDuration >= (msPerDit - tolerance)) //If an intra-character pause occurs (>= 0.25 time units)
				{
					readDashDot();
					//Visual indicator on bottom of screen shows exact length of each tap, inspired by Osu!
					if (osuDots[osuDots_i] <= SCREEN_WIDTH - 2)
						drawFilledRect(osuDots[osuDots_i] - 2, 442, 4, 8, 255, 255, 255); //Remove old dot
					if (120 * downDuration / msPerDit + 40 < SCREEN_WIDTH - 2) //If new dot would appear on screen
					{
						osuDots[osuDots_i] = 120 * downDuration / msPerDit + 40;
						drawFilledRect(osuDots[osuDots_i] - 2, 442, 4, 8, Pen.r, Pen.g, Pen.b); //Draw new dot
					}
					else
						osuDots[osuDots_i] = -1; //Default offscreen pos, prevents overflow when setting value
					osuDots_i++;
					osuDots_i = osuDots_i & 15; //Cheap mod 16 operation
					action = READ_CHARACTER; //Check for inter-char pause next
					SDL_RenderPresent(renderer);
				}
				else if (action == READ_CHARACTER && elapsedUpDuration >= 2 * msPerDit) //If an inter-character pause occurs (>= 2 time units)
				{
					input[input_i] = '\0';
					int c = morseToAscii(); //Determine character that was tapped. int here ensures null termination when cast as a char*
					if (gameType == MORSE_WORDS || MORSE_PHRASES)
					{
						if (c != 0) //If a valid character is detected, add to screen
						{
							typeLowTextBox(c);
							//Automatic tuning of WPM is checked every time a char is successfully tapped
							bool consistentError = true; //True if every value in error is the same
							for (uint8_t i = 1; i < 8; i++)
							{
								if (error[i] != error[0])
								{
									consistentError = false;
									break;
								}
							}
							if (consistentError)
							{
								if (error[0] == fast) //User is tapping consistently fast
									setWPM(wpm + 1);
								else //User is tapping consistently slow
									setWPM(wpm - 1);
								error[0] = 0; //Reset record after WPM update to avoid multiple adjacent changes
								error[1] = 0;
								error[2] = 0;
								error[3] = 0;
								error[4] = 0;
								error[5] = 0;
								error[6] = 0;
								error[7] = 0;
							}
							inputComplete = text_i == targetLength; //End input if the users has tapped as many letters as the length of the prompt
						}
						input_i = 0;
						action = READ_WORD; //Check for inter-word pause next
					}
					else //(gameMode == MORSE_CHARACTERS) Input ends after a single char is tapped, even if it is unrecognized
					{
						text[text_i] = c;
						text_i++;
						inputComplete = true;
					}
				}
				else if (action == READ_WORD && elapsedUpDuration >= 5 * msPerDit) //If an inter-word pause occurs (>= 5 time units)
				{
					if (gameType == MORSE_PHRASES)
					{
						if (text[text_i - 3] == ' ' && text[text_i - 2] == 'I' && text[text_i - 1] == 'I') //Check if " II" (error/I say again) has been tapped
						{
							//Delete "II"
							bkspcLowTextBox();
							bkspcLowTextBox();
							morseDelete = true;
						}
						else
						{
							if (text[text_i - 1] != ' ') //Prevents consecutive spaces
							{
								text[text_i] = ' ';
								text_i++;
							}
							inputComplete = text_i == targetLength; //Check if input should be ended
						}
					}
					//For MORSE_WORDS, don't draw a space. User can wait forever between characters in this mode, emulating Farnsworth timing
					action = PAUSE; //Stop reading pauses
				}
				if (morseDelete) //Delete previous word
				{
					//Delete last char (in case last is ' '), then keep deleting until a ' ' is found
					do
					{
						bkspcLowTextBox();
					} while (text_i != 0 && text[text_i - 1] != ' ');
					morseDelete = false;
					action = PAUSE; //Stop reading pauses
				}
			}
			else //(state == DOWN) If the straight key is being pressed, a dit/dah is being played
			{
				if (action == READ_CHARACTER) //An intra-char pause was recorded
				{
					totalShortPauseDuration += upDuration; //Stats are taken
					shortPauseCount++;
					if (upDuration < msPerDit) //error only updates for dits, dahs, and intra-char pauses
						error[error_i] = fast;
					else
						error[error_i] = slow;
					error_i++;
					error_i = error_i & 7; //Cheap mod 8 operation
					shortPauseSamples[shortPauseSamples_i] = upDuration;
					shortPauseSamples_i++;
					if (shortPauseSamples_i == numSamples)
						shortPauseSamples_i = 0;
					if (shortPauseSamplesSize < numSamples + 1)
						shortPauseSamplesSize++;
				}
				if (action == READ_WORD) //A char pause was recorded
				{
					totalCharPauseDuration += upDuration;
					charPauseCount++;
					charPauseSamples[charPauseSamples_i] = upDuration;
					charPauseSamples_i++;
					if (charPauseSamples_i == numSamples)
						charPauseSamples_i = 0;
					if (charPauseSamplesSize < numSamples + 1)
						charPauseSamplesSize++;
				}
				if (action == PAUSE) //A space was recorded
				{
					totalWordPauseDuration += upDuration;
					wordPauseCount++;
					wordPauseSamples[wordPauseSamples_i] = upDuration;
					wordPauseSamples_i++;
					if (wordPauseSamples_i == numSamples)
						wordPauseSamples_i = 0;
					if (wordPauseSamplesSize < numSamples + 1)
						wordPauseSamplesSize++;
				}
				action = READ_DASHDOT;
			}
		}
	}
	//inputComplete == true
	text[text_i] = '\0'; //Ensure text is null terminated
	drawFilledRect(80, instructY - lineSpace2 + 5, 480, lineSpace2 * 4 + 5, 255, 255, 255); //Clear instructions
	bool correct;
	if (strcmp(text, targetInput) == 0)
	{
		correct = true;
		drawColorTextCentered("Correct!", 0, instructY - lineSpace2, 12, 150, 49);
		drawTextCentered("Press enter to continue", 0, instructY);
	}
	else
	{
		correct = false;
		drawColorTextCentered("Incorrect!", 0, instructY - lineSpace2, 255, 0, 0);
		if (text[0] == '\0')
		{
			drawTextCentered("Morse Code sequence", 0, 250);
			drawTextCentered("unrecognized", 0, 250 + lineSpace2);
		}
		drawTextCentered("Press enter to retry", 0, instructY);
		drawTextCentered("Press tab to hear Morse Code", 0, instructY + lineSpace2 * 2);
	}
	drawTextCentered("Press space to show stats", 0, instructY + lineSpace2);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			case SDLK_SPACE:
				showStats();
			case SDLK_RETURN: //If successful, a new prompt is chosen. Else, the user retries the previous prompt
				if (correct)
					return;
				else
					goto Retry;
			case SDLK_TAB:
				if (!correct)
				{
					asciiToMorse(targetInput, playback);
					playMorse();
				}
			}
		}
	}
}

void explainMorseScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Understanding Morse Code", 0, titleY);
	drawTextCentered("Press enter to continue", 0, instructY + lineSpace2 * 4);
	setFontSize(1);
	//String literals cannot have a length of over ~256, so this paragraph must be split up
	drawTextWrapped("Morse Code encodes every letter or number into a sequence of beeps, measured in units of time. Dots last 1 time unit and Dashes last 3 time units. Between each dot/dash is a pause of 1 time unit,", listX, 100, SCREEN_WIDTH - (listX * 2), lineSpace1);
	drawTextWrapped("between each character is a pause of 3 time units, and between each word is a pause of 7 time units. However, this tutorial only covers individual characters.", listX, 100 + lineSpace1 * 4, SCREEN_WIDTH - (listX * 2), lineSpace1);
	drawTextWrapped("The Morse Code sequence for each character will be played audibly. Then, you must repeat the sequence back by tapping on the straight key. After 1 successful attempt, you may continue on to the next character.", listX, 100 + lineSpace1 * 9, SCREEN_WIDTH - (listX * 2), lineSpace1);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			case SDLK_RETURN:
				mode = chooseTutorial;
				return;
			}
		}
	}
}

void chooseTutorialScreen()
{
	tutorial_level = 0;
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Set Speed:", 0, titleY);
	drawTextWrapped("[1] Slow (10 WPM)\n[2] Fast (16 WPM)\n[3] Custom WPM", listX, listY, SCREEN_WIDTH, lineSpace2);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			case '1':
				setWPM(10);
				mode = tutorial;
				return;
			case '2':
				setWPM(16);
				mode = tutorial;
				return;
			case '3':
				mode = tutorial;
				return;
			}
		}
	}
}

void tutorialScreen()
{
	int tutorial_ascii; //int here ensures null termination when cast as a char*
	if (tutorial_level < 26)
		tutorial_ascii = tutorial_level + 'A'; //Convert to ascii letter
	else
		tutorial_ascii = tutorial_level + 22; //Convert to ascii number
Again:
	background(255, 255, 255);
	setFontSize(1);
	drawText("pause = 1", 590 - fontPixels[fontSize] * 6, escY);
	drawFilledCircle(570, escY + lineSpace2 + 12, 7, Pen.r, Pen.g, Pen.b);
	drawText("= 1", 590, escY + lineSpace2);
	drawFilledRect(540, escY + lineSpace2 * 2 + 11, 40, 4, Pen.r, Pen.g, Pen.b);
	drawText("= 3", 590, escY + lineSpace2 * 2);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	setFontSize(4);
	drawTextCentered((char*)&tutorial_ascii, 0, titleY);
	uint8_t draw_i = 0; //For drawing dits & dahs and feedback numbers
	int morse_length = 0;
	while (myMorse[tutorial_ascii - 32][morse_length] != '\0') //Draw big Morse Code sequence
	{
		if (myMorse[tutorial_ascii - 32][morse_length] == '.') //Dit
		{
			drawFilledCircle(32 + draw_i * 64, 160, 16, Pen.r, Pen.g, Pen.b);
			draw_i++;
		}
		else //Dah
		{
			drawFilledRect(16 + draw_i * 64, 156, 96, 8, Pen.r, Pen.g, Pen.b);
			draw_i += 2;
		}
		morse_length++;
	}
	//morse_length now equals length of myMorse[tutorial_ascii - 32]
	input_i = 0;
	action = START;
	bool fail = false;
	draw_i = 0;
	uint8_t deciTimeUnits;
	char feedbackString[5] = { 0 };
	strcpy(playback, myMorse[tutorial_ascii - 32]);
	drawFilledCircle(flashX, flashY, flashR, Pen.r, Pen.g, Pen.b);
	SDL_RenderPresent(renderer);
	SDL_Delay(500);
	playMorse(); //Demo the letter
	setFontSize(2);
	state = UP; //Reset straight key
	drawTextCentered("Tap out the Morse Code", 0, instructY - lineSpace2);
	drawTextCentered("Straight Key set to:", -fontPixels[fontSize], instructY);
	drawTextCentered((char*)&straightKey, fontPixels[fontSize] * 21 / 2, instructY);
	drawTextCentered("Press bkspc to reset", 0, instructY + lineSpace2);
	SDL_RenderPresent(renderer);
	setFontSize(1);
	while (input_i < morse_length)
	{
		SDL_PollEvent(&event);
		checkStraightKey(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				SDL_PauseAudioDevice(audio_device, true);
				mode = mainMenu;
				return;
			case SDLK_BACKSPACE: //Reset function
				SDL_PauseAudioDevice(audio_device, true);
				goto Again;
			}
		}
		if (state == DOWN) //Plays the speaker when the straight key is held down
		{
			drawFilledCircle(flashX, flashY, flashR - 3, 255, 255, 255); //Turn on flash
			SDL_PauseAudioDevice(audio_device, false);
		}
		else
		{
			drawFilledCircle(flashX, flashY, flashR - 3, Pen.r, Pen.g, Pen.b); //Turn off flash
			SDL_PauseAudioDevice(audio_device, true);
		}
		SDL_RenderPresent(renderer);
		if (state == UP) //If we are at a pause
		{
			if (action == READ_DASHDOT) //The key was previously pressed
			{
				if (myMorse[tutorial_ascii - 32][input_i] == '.')
				{
					if (!(msPerDit - tolerance <= downDuration && downDuration <= msPerDit + tolerance)) //To pass the tutorial, every dit must be between 0.25 & 1.75 time units
						fail = true;
					if (downDuration / msPerDit < 99) //Prohibits more than 2 digits left of the decimal
					{
						sprintf(feedbackString, "%.1f", (float)downDuration / (float)msPerDit);
						drawTextCentered(feedbackString, -288 + draw_i * 64, 200); //Centered on dot
						SDL_RenderPresent(renderer);
					}
					draw_i++;
				}
				else if (myMorse[tutorial_ascii - 32][input_i] == '-')
				{
					if (!(3 * msPerDit - tolerance <= downDuration & downDuration <= 3 * msPerDit + 2 * tolerance)) //To pass the tutorial, every dah must be between 2.25 & 4.5 time units
						fail = true;
					if (downDuration / msPerDit < 99)
					{
						sprintf(feedbackString, "%.1f", (float)downDuration / (float)msPerDit);
						drawTextCentered(feedbackString, -256 + draw_i * 64, 200); //Centered on dash
						SDL_RenderPresent(renderer);
					}
					draw_i += 2;
				}
				input_i++; //Increment index after each tap
				action = PAUSE;
			}
		}
		else //if (state == DOWN) If the straight key is being pressed, a dit/dah is being played
		{
			if (input_i > 0 && action == PAUSE) //This isn't the first tap
			{
				if (!(msPerDit - tolerance <= upDuration && upDuration <= msPerDit + tolerance)) //To pass the tutorial, every pause must be between 0.25 & 1.75 time units
					fail = true;
				if (upDuration / msPerDit < 99)
				{
					sprintf(feedbackString, "%.1f", (float)upDuration / (float)msPerDit);
					drawTextCentered(feedbackString, -320 + draw_i * 64, 240); //Centered between dots/dashes
					SDL_RenderPresent(renderer);
				}
			}
			action = READ_DASHDOT;
		}
	}
	//Input is complete
	setFontSize(2);
	if (!fail)
	{
		drawFilledRect(80, 5 + instructY - lineSpace2, 480, lineSpace2 * 3, 255, 255, 255); //Delete instructions
		drawColorTextCentered("Correct!", 0, instructY, 12, 150, 49);
		drawTextCenteredWrapped("Press enter to continue\nPress bkspc to practice again", 0, instructY + lineSpace2, SCREEN_WIDTH, lineSpace2);
		SDL_RenderPresent(renderer);
		while (true)
		{
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT || mode == quit)
			{
				mode = quit;
				return;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					mode = mainMenu;
					return;
				case SDLK_RETURN:
					tutorial_level++;
					if (tutorial_level == 36) //Tutorial completed
						mode = tutorialFinish;
					return;
				case SDLK_BACKSPACE: //Even on success, the user can retry
					goto Again;
				}
			}
		}
	}
	else //fail
	{
		drawFilledRect(80, 5 + instructY - lineSpace2, 480, lineSpace2 * 2, 255, 255, 255); //Delete instructions except for "Press bkspc to reset"
		drawColorTextCentered("Incorrect!", 0, instructY, 255, 0, 0);
		SDL_RenderPresent(renderer);
		while (true)
		{
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT || mode == quit)
			{
				mode = quit;
				return;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					mode = mainMenu;
					return;
				case SDLK_BACKSPACE:
					goto Again;
				}
			}
		}
	}
}

void tutorialFinishScreen()
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Congratulations!", 0, titleY);
	drawTextWrapped("You have completed the Morse Code tutorial. Next, try honing your skills in the other practice modes. Check out the Options to change the WPM and other settings.", listX, listY, SCREEN_WIDTH - (listX * 2), lineSpace2);
	if (wpm < 13) //Only suggest the Fast tutorial if the user was going signficantly slower
		drawTextWrapped("Or try playing the Fast tutorial by pressing enter.", listX, listY + lineSpace2 * 6, SCREEN_WIDTH - (listX * 2), lineSpace2);
	SDL_RenderPresent(renderer);
	while (true)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT || mode == quit)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				mode = mainMenu;
				return;
			case SDLK_RETURN:
				if (wpm < 13)
				{
					setWPM(16);
					tutorial_level = 0;
					mode = tutorial;
					return;
				}
			}
		}
	}
}

uint32_t timerCallback(uint32_t interval, void* params)
{
	if (beeping)
	{
		SDL_PauseAudioDevice(audio_device, true);
		audio_time = 0; //Reset start of sine wave
	}
	else
	{
		SDL_PauseAudioDevice(audio_device, false);
		audio_time = 0;
	}
	beeping = !beeping;
	waiting = false; //Prepare to calculate the next timer upon return
	return nextTimer; //The returned value sets the new timer duration
}

void SDLCALL audioCallback(void* userdata, uint8_t* stream, int bytes)
{
	int16_t* buffer = (int16_t*)stream;
	for (int i = 0; i < bytes / 2; i++) //bytes / 2 due to 16-bit depth
	{
		//Fills buffer with a sine wave of 44100 samples/sec and a frequency of beepFreq
		buffer[i] = (int16_t)(20000 * sin(2.0f * M_PI * (float)beepFreq * (float)audio_time / 44100.0f)); //20000 = amplitude (out of 32767)
		audio_time++;
	}
}