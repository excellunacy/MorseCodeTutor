/*This source code copyrighted by Lazy Foo' Productions 2004-2024
and may not be redistributed without written permission.*/

#include "Global.h"
//Global variables used in other files
SDL_Window* window = NULL; //Must be outside main function. Workaround exists if several global variables are needed.
SDL_Renderer* renderer = NULL;
SDL_Color Pen = { 0,0,0,255 }; //Opaque black by default
uint8_t textSize = 0;
TTF_Font* myFont;
char tempStr[200] = { 0 };
char text[strLength] = ""; //Stores ASCII
uint16_t text_i = 0;
uint32_t msPerDit;
uint32_t msPerFDit;
uint16_t wpm;
uint16_t fwpm;
bool fEnable = false; //Determines whether to use Farnsworth timing
uint32_t tolerance;     // Duration that dash, dot, or gap may deviate from the ideal length to still be recognized
uint16_t lastRandomIndex=-1;
uint32_t totalDitDuration = 0;
uint32_t totalDahDuration = 0;
uint32_t totalShortPauseDuration = 0;
uint32_t totalCharPauseDuration = 0;
uint32_t totalWordPauseDuration = 0;
uint16_t ditCount = 0;
uint16_t dahCount = 0;
uint16_t shortPauseCount = 0;
uint8_t  charPauseCount = 0;
uint8_t  wordPauseCount = 0;
int straightKey = 'm'; //Key designated as straight key
uint32_t lastChange;
uint16_t downDuration;
uint16_t upDuration;
char input[input_length] = { 0 }; //Stores the Morse Code sequence for a single char
uint8_t input_i = 0;
bool error[8] = { fast,slow,fast,slow,fast,slow,fast,slow };
uint8_t error_i = 0;
bool morseDelete = false; //Flag for deleting the previous word when tapping Morse Code
bool state = false;

//Global variables only used here
SDL_AudioSpec audio_spec;
uint8_t* audio_buf;
uint32_t audio_len;
uint32_t audio_time;
SDL_AudioDeviceID audio_device;
SDL_Event event;
uint8_t mode = start;
SDL_TimerID timerID;
//Action for interpreting Morse Code from straight key
enum Action {
	START = 0,
	READ_DASHDOT = 1,
	READ_CHARACTER = 2,
	READ_WORD = 3,
	PAUSE = 4
} action;

char test[] = { ".-,-...,-.-.\0" }; //Morse Code for "ABC" for calibration
uint16_t elapsedUpDuration = 0; //Accumulates within loop, to detect the type of pause
uint16_t osuDots[16] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
uint8_t osuDots_i = 0;

//Timing state
uint16_t beepFreq = 784;
volatile bool waiting = false;
volatile bool beeping = false;
uint16_t nextTimer = 0;
uint16_t playbackIndex = 0;

int keyInput = -1;
char playback[strLength * 3] = { 0 }; //Stores Morse Code sequence

int main(int argc, char* args[])
{
	
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		while (1);
	}
	window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		while (1);
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	TTF_Init();
	setFontSize(2);
	if (myFont == NULL)
	{
		printf("Font not loaded: %s\n", TTF_GetError());
	}

	//SDL_LoadWAV("defaultBeep.wav", &audio_spec, &audio_buf, &audio_len);
	audio_spec.freq = 44100; //Hz data freqency
	audio_spec.format = AUDIO_S16SYS; //Built-in SDL_AudioFormat
	audio_spec.channels = 1;
	audio_spec.samples = 1024; //Low to reduce latency on pausing?
	audio_spec.callback = audioCallback;
	audio_spec.userdata = NULL;
	audio_time = 0; //Used for sine equation. Risks overflowing after ~25 hours of constantly beeping
	SDL_AudioSpec obtained; //Will reflect hardware audio capabilities (output)
	audio_device = SDL_OpenAudioDevice(NULL, false, &audio_spec, &obtained, 0);
	if (audio_device <= 0)
	{
		printf("%s", SDL_GetError());
		while (1);
	}
	if (audio_spec.format != obtained.format)
	{
		while (1);
	}
	//Use SDL_PauseAudioDevice(audio_device,false) instead of tone(freq)

	setWPM(20); //Default WPM
	fEnable = false;
	setFWPM(10);
	mode = start; //Initial state values

	/*
	//This block is for debugging event behaviors
	SDL_PollEvent(&event);
	int prevEvent=256;
	while (event.type != SDL_QUIT)
	{
		if (event.type != prevEvent)
		{
			//printf("%d %d\n", event.type, event.key.repeat);
			if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
			{
				printf("Key pressed\n");
			}
			else if (event.type == SDL_KEYUP && event.key.repeat == 0)
			{
				printf("Key released\n");
			}
			prevEvent = event.type;
		}
		SDL_PollEvent(&event);
	}
	*/

	SDL_PollEvent(&event);
	while (mode != quit)
		loop();

	//Close everything
	SDL_DestroyWindow( window );
	TTF_CloseFont(myFont);
	SDL_FreeWAV(audio_buf);
	SDL_CloseAudioDevice(audio_device);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}

void loop()
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
	case options:
		//optionsScreen();
		break;
	case transMtoA:
		keyMtoAScreen();
		break;
	case hearMorse:
		//hearingMorsegame();
		break;
	case mToAGameChoice:
		//mToAGameChoiceScreen();
		break;
	case mToAGame:
		//MorsetoASCIIGames();
		break;
	case explainMorse:
		//explainMorseScreen();
		break;
	case chooseTutorial:
		//chooseTutorialScreen();
		break;
	case tutorial:
		//tutorialScreen();
		break;
	case tutorialFinish:
		//tutorialFinishScreen();
		break;
	case gameModes:
		//GameModeSelectScreen();
		break;
	case qCodes:
		//QCodesPractice();
		break;
	}
}

void startScreen()
{
	background(255,180,255);
	setFontSize(2);
	drawTextCentered("Welcome to the", 0, 100);
	drawTextCentered("MORSE CODE TUTOR", 0, 125);
	setFontSize(1);
	drawTextCentered("PC Port", 0, 150);
	drawTextCentered("Press enter to continue", 0, 175);
	SDL_RenderPresent(renderer);
	while (1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT)
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
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //Color guide = RGB, opacity, 8-bit each
	SDL_RenderClear(renderer); //Set background
	setFontSize(2);
	drawTextCentered("Main Menu", 0, 10);
	setFontSize(1);
	drawText("[1]Translate Text to Morse", 10, 50);
	drawText("[2]Translate Morse to Text", 10, 70);
	drawText("[3]Game Modes", 10, 90);
	drawText("[4]Learn Morse Code", 10, 110);
	drawText("[5]Options", 10, 130);
	SDL_RenderPresent(renderer);

	while (1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT)
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
				mode = gameModes;
				return;
			case '4':
				mode = explainMorse;
				//tutorial_level = 0; //Always go to start of tutorial
				return;
			case '5':
				mode = options;
				//choice = 0; //If options is selected from the main menu, reset choice
				//prevChoice = -1;
				return;
			}
		}
	}
	do
	{
		SDL_PollEvent(&event);
	} while (event.type != SDL_QUIT);
}

void typeAtoMScreen()
{
	background(255, 255, 255);
	setFontSize(1);
	drawText("Esc to exit/stop audio", escX, escY);
	drawTextCentered("Type sentence", 0, 425);
	drawTextCentered("Press enter to play", 0, 450);
	SDL_RenderPresent(renderer);
	text_i = 0;
	while (1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT||mode==quit) //mode == quit in case quit was pressed during playMorse
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym==SDLK_ESCAPE)
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
			else if (event.key.keysym.sym == SDLK_BACKSPACE && text_i!=0)
			{
				bkspcTextbox();
			}
			else if (text_i < strLength && isValidChar(event.key.keysym.sym)) //If screen isn't full & input is a valid char
			{
				typeTextbox(toupper(event.key.keysym.sym));
			}
		}
		
	}
}

void playMorse() //Play Morse Code sequence contained in playback through speaker
{
	playbackIndex = 0;
	while (1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT)
		{
			mode = quit;
			SDL_PauseAudioDevice(audio_device, true);
			SDL_RemoveTimer(timerID);
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				SDL_PauseAudioDevice(audio_device, true);
				SDL_RemoveTimer(timerID);
				audio_time = 0;
				beeping = false;
				waiting = false;
				return;
			}
			if (mode == typeAtoM || mode == hearMorse)
			{
				if (event.key.keysym.sym == SDLK_BACKSPACE && text_i != 0)
					bkspcTextbox();
				else if (text_i < strLength && isValidChar(event.key.keysym.sym)) //If screen isn't full & input is a valid char
					typeTextbox(toupper(event.key.keysym.sym));
			}
		}
		if (!waiting) //If not waiting on current timer
		{
			if (!beeping)
			{
				if (playbackIndex == 0) //Start
				{
					timerID = SDL_AddTimer(100, timerCallback, NULL); //Begin timer chain
				}
				switch (playback[playbackIndex]) //Find time to beep based on current index
				{
				case '\0': //End
					SDL_RemoveTimer(timerID);
					return;
				case '.':
					nextTimer=msPerDit;
					break;
				case '-':
					nextTimer=msPerDit * 3;
				}
				playbackIndex++;
			}
			else //beeping
			{
				switch (playback[playbackIndex]) //Find time to pause based on next index
				{
				case '\0':
					nextTimer = 0;
					break;
				case '.':
				case '-':
					nextTimer = msPerDit;
					break;
				case ',':
					if (fEnable)
						nextTimer=msPerFDit * 3;
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
			}
			waiting = true;
		}
	}
}

void calibrationQueryScreen() //Asks user if straight key WPM should be calibrated. Currently only called when Translate "Morse to ASCII" is selected
{
	background(255, 255, 255);
	setFontSize(2);
	drawTextCenteredWrapped("Do you wish to calibrate the Straight Key WPM?\n(y/n)", 0, 200, 150, lineSpace2);
	SDL_RenderPresent(renderer);
	while (1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT)
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
			case 'n':
				return;
			}
		}
	}
}

void calibrationScreen() //Straight key is calibrated by user inputting a specified Morse Code sequence.
{
RestartCal:
	background(255, 255, 255);
	setFontSize(1);
	drawText("Esc to exit", escX, escY);
	drawTextCentered("Key in \'ABC\'", 0, 150);
	//Draw Morse Code
	//screen.text(".- -... -.-.", 44, 40);
	drawTextCentered("Press Bkspc to restart", 0, 400); //Allows user to retry calibration
	SDL_RenderPresent(renderer);
	uint8_t test_i = 0;
	clearStats(); //Stats are cleared every time straight key input begins
	action = START;
	while (test_i < 12)
	{
		SDL_PollEvent(&event);
		pollStraightKey(&event);
		if (event.type == SDL_QUIT)
		{
			mode = quit;
			SDL_PauseAudioDevice(audio_device, true);
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
				goto RestartCal;
			}
		}
		if (state == DOWN) //Plays the speaker when the straight key is held down
			SDL_PauseAudioDevice(audio_device, false);
		else
			SDL_PauseAudioDevice(audio_device, true);
		if (state == UP) //If we are at a pause
		{
			if (action == READ_DASHDOT) //The key was previously pressed
			{
				if (test[test_i] == '.') //Checks what input should be and adjusts stats accordingly
					totalDitDuration += downDuration;
				else if (test[test_i] == '-')
					totalDahDuration += downDuration;
				test_i++; //Increment index after each tap
				action = PAUSE;
			}
		}
		else //if (state == DOWN) When the straight key is pressed
		{
			if (test_i > 0 && action == PAUSE) //This isn't the first tap. action == PAUSE maybe can be removed?
			{
				if (test[test_i] == ',') //Skip ',' for inter-char pause
					test_i++;
				else
					totalShortPauseDuration += upDuration; //Record intra-char pauses only
			}
			action = READ_DASHDOT;
		}
	}
	//Check the stats
	float dahDitRatio = 3.25 * totalDahDuration / (totalShortPauseDuration + totalDitDuration); //Ratio of dah length to dit/intra-char pause length must be between 2.25-4.0. This ensures the user correctly tapped "ABC" with somewhat accurate rhythm. //(1/4)/(1/13)
	if (2.25 < dahDitRatio && dahDitRatio < 4.0) //If valid calibration
	{
		setWPM(round(30000.0 / (totalShortPauseDuration + totalDitDuration + totalDahDuration))); //25 time units make up ABC, not counting inter char pauses. This formula corverts the total time into the WPM (1200*25)
		drawTextCentered("WPM set to:", fontPixels[textSize] * -3 / 2, 400 + lineSpace2);
		char temp[] = { (wpm / 10) + 48, (wpm % 10) + 48, '\0' }; //Display two digits for wpm //The 48s convert the digits into ASCII values
		drawText(temp, 6 * fontPixels[textSize] , 400 + lineSpace2);
		SDL_Delay(500); //Slight delay before audio demo
		strcpy(playback, test); //Load Morse Code into playback
		playMorse(); //Demo the set WPM
		drawTextCentered("Press enter to continue", 0, 400 + 2 * lineSpace2);
		SDL_RenderPresent(renderer);
		while (1)
		{
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT)
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
					return;
				case SDLK_BACKSPACE:
					goto RestartCal; //Repeat entire function
				}
			}
		}
	}
	else
	{
		drawTextCentered("Invalid calibration", 0, 400 + lineSpace2);
		SDL_RenderPresent(renderer);
		while (1)
		{
			SDL_PollEvent(&event);
			if (event.type == SDL_QUIT)
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
					goto RestartCal; //Repeat entire function
				}
			}
		}
	}
}

void keyMtoAScreen()
{
	if (mode == quit)
		return;
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawTextCenteredWrapped("Key in Morse Code\nPress Space to show stats\nPress Bkspc to clear", 0, 350,SCREEN_WIDTH,lineSpace2);
	//Draw osu chart
	drawFilledRect(40, 414, 8, 32,Pen.r,Pen.g,Pen.b);
	drawFilledRect(180, 414, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(180, 438, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(460, 414, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(460, 438, 8, 8, Pen.r, Pen.g, Pen.b);
	//40 chars fit on a line
	action = START; //Determines what pause duration to look for
	text_i = 0;
	input_i = 0;
	SDL_RenderPresent(renderer);
	setFontSize(1);
	clearStats(); //Stats are cleared every time straight key input begins
	while (1)
	{
		SDL_PollEvent(&event);
		pollStraightKey(&event);
		if (event.type == SDL_QUIT)
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
			case SDLK_SPACE: //Enter displays stats for straight key input (average & standard deviation for dits & dahs, etc.)
				showStats();
				return;
			case SDLK_BACKSPACE: //When Bkspc is pressed, clear the screen
				if (text_i != 0)
					return;
				break;
			}
		}
		if (state == DOWN) //Plays the speaker when the straight key is held down
			SDL_PauseAudioDevice(audio_device, false);
		else
			SDL_PauseAudioDevice(audio_device, true);
		if (text_i < textBoxSize)
		{
			if (state == UP) //If we are at a pause
			{
				elapsedUpDuration = (SDL_GetTicks() - lastChange); //A variable separate from upDuration is needed so that this never updates upDuration immediately after the falling edge
				if (action == READ_DASHDOT && elapsedUpDuration >= (msPerDit - tolerance)) //If an intra-character pause occurs //Try changing (msPerDit-tolerance) to 10
				{
					readDashDot();
					//Visual indicator on bottom of screen shows exact length of each tap, inspired by Osu!
					//Remove old dot
					drawFilledRect(osuDots[osuDots_i], 426, 8, 8, Pen.r, Pen.g, Pen.b);
					if (35 * downDuration / msPerDit + 10 < 255) //Replace dot value
						osuDots[osuDots_i] = 35 * downDuration / msPerDit + 10;
					else
						osuDots[osuDots_i] = -1; //Default offscreen pos, prevents overflow
					//Draw new dot
					drawFilledRect(osuDots[osuDots_i], 426, 8, 8, Pen.r, Pen.g, Pen.b);
					osuDots_i++;
					osuDots_i = osuDots_i & 15; //Cheap mod 16 operation
					action = READ_CHARACTER; //Check for inter-char pause next
					SDL_RenderPresent(renderer);
				}
				else if (action == READ_CHARACTER && elapsedUpDuration >= 2 * msPerDit) //If an inter-character pause occurs
				{
					input[input_i] = '\0';
					int c = morseToAscii(); //Determine character that was tapped //Int here ensures null termination when cast as a char*
					if (c != 0) //If a valid character is detected, add to screen
					{
						typeTextbox(toupper(c));
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
							{
								setWPM(wpm + 1);
							}
							else //User is tapping consistently slow
							{
								setWPM(wpm - 1);
							}
							error[0] = fast; //Reset record after WPM update to avoid multiple adjacent changes
							error[1] = slow;
							error[2] = fast;
							error[3] = slow;
							error[4] = fast;
							error[5] = slow;
							error[6] = fast;
							error[7] = slow;
						}
					}
					input_i = 0;
					action = READ_WORD; //Check for inter-word pause next
				}
				else if (action == READ_WORD && elapsedUpDuration >= 5 * msPerDit) //If an inter-word pause occurs
				{
					text[text_i] = ' ';
					text_i++;
					action = PAUSE; //Stop looking for pauses
				}
				if (morseDelete)
				{
					//Deletes previous word
					//Algorithm: Delete last char (in case last is ' '), then keep deleting until a ' ' is found
					bkspcTextbox();
					while (text_i != 0 && text[text_i - 1] != ' ')
					{
						bkspcTextbox();
					}
					action = PAUSE;
					morseDelete = false;
				}
			}
			else //(state == DOWN) When the straight key is pressed, a dit/dah is being played
			{
				if (action == READ_CHARACTER) //An intra-char pause was recorded
				{
					totalShortPauseDuration += upDuration;
					shortPauseCount++;
					error[error_i] = upDuration < msPerDit;
					error_i++;
					error_i = error_i & 7; //Cheap mod 8 operation
				}
				if (action == READ_WORD) //A char pause was recorded
				{
					totalCharPauseDuration += upDuration;
					charPauseCount++;
				}
				if (action == PAUSE) //A space was recorded
				{
					totalWordPauseDuration += upDuration;
					wordPauseCount++;
				}
				action = READ_DASHDOT;
			}
		}
	}
}

void showStats() //Displays stats, i.e. averages times of each dit/dah/pause
{
	background(255, 255, 255);
	setFontSize(2);
	drawText("Esc to exit", escX, escY);
	drawText("Average Dit:", 10, 100);
	uint16_t tempAvg;
	if (ditCount != 0) //If average is ever NaN, it defaults to 0
		tempAvg = totalDitDuration / ditCount;
	else
		tempAvg = 0;
	char temp[] = { ((tempAvg / 1000) % 10) + 48, '.', ((tempAvg / 100) % 10) + 48, ((tempAvg / 10) % 10) + 48, (wpm % 10) + 48, 's', '\0' }; //Display four digits //The 48s convert the digits into corresponding ASCII values
	drawText(temp, 10 + fontPixels[textSize]*20, 100);
	drawText("Average Dah:", 10, 100+lineSpace2);
	if (dahCount != 0)
		tempAvg = totalDahDuration / dahCount;
	else
		tempAvg = 0;
	temp[0] = ((tempAvg / 1000) % 10) + 48;
	temp[2] = ((tempAvg / 100) % 10) + 48;
	temp[3] = ((tempAvg / 10) % 10) + 48;
	temp[4] = (tempAvg % 10) + 48;
	drawText(temp, 10 + fontPixels[textSize] * 20, 100+lineSpace2);
	drawText("Average Tap Pause: ", 10, 100+lineSpace2*2);
	if (shortPauseCount != 0)
		tempAvg = totalShortPauseDuration / shortPauseCount;
	else
		tempAvg = 0;
	temp[0] = ((tempAvg / 1000) % 10) + 48;
	temp[2] = ((tempAvg / 100) % 10) + 48;
	temp[3] = ((tempAvg / 10) % 10) + 48;
	temp[4] = (tempAvg % 10) + 48;
	drawText(temp, 10 + fontPixels[textSize] * 20, 100+lineSpace2*2);
	drawText("Average Char Pause: ", 10, 100 + lineSpace2*3);
	if (charPauseCount != 0)
		tempAvg = totalCharPauseDuration / charPauseCount;
	else
		tempAvg = 0;
	temp[0] = ((tempAvg / 1000) % 10) + 48;
	temp[2] = ((tempAvg / 100) % 10) + 48;
	temp[3] = ((tempAvg / 10) % 10) + 48;
	temp[4] = (tempAvg % 10) + 48;
	drawText(temp, 10 + fontPixels[textSize] * 20, 100+lineSpace2*3);
	drawText("Average Space: ", 10, 100 + lineSpace2*4);
	if (wordPauseCount != 0)
		tempAvg = totalWordPauseDuration / wordPauseCount;
	else
		tempAvg = 0;
	temp[0] = ((tempAvg / 1000) % 10) + 48;
	temp[2] = ((tempAvg / 100) % 10) + 48;
	temp[3] = ((tempAvg / 10) % 10) + 48;
	temp[4] = (tempAvg % 10) + 48;
	drawText(temp, 10 + fontPixels[textSize] * 20, 100+lineSpace2*4);
	drawText("Actual WPM: ", 10, 100 + lineSpace2*5); //This value is calculated based on total time units over total time
	if (ditCount + dahCount != 0)
		tempAvg = 120000 * (ditCount + dahCount * 3 + shortPauseCount + charPauseCount * 3 + wordPauseCount * 7) / (totalDitDuration + totalDahDuration + totalShortPauseDuration + totalCharPauseDuration + totalWordPauseDuration); //WPM * 100
	else
		tempAvg = 0;
	temp[0] = ((tempAvg / 1000) % 10) + 48;
	temp[1] = ((tempAvg / 100) % 10) + 48;
	temp[2] = '.';
	temp[3] = ((tempAvg / 10) % 10) + 48;
	temp[4] = (tempAvg % 10) + 48;
	temp[5] = '\0';
	drawText(temp, 10 + fontPixels[textSize] * 19, 100+lineSpace2*5);
	while (1)
	{
		SDL_PollEvent(&event);
		pollStraightKey(&event);
		if (event.type == SDL_QUIT)
		{
			mode = quit;
			return;
		}
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE)
		{
			return;
		}
	}
}

uint32_t timerCallback(uint32_t interval, void* params)
{
	if (beeping)
	{
		//noTone
		SDL_PauseAudioDevice(audio_device, true);
		audio_time = 0;
		

	}
	else
	{
		//tone
		SDL_PauseAudioDevice(audio_device, false);
		audio_time = 0;
	}
	beeping = !beeping;
	waiting = false;
	return nextTimer; //The returned value sets the new timer length
}

void SDLCALL audioCallback(void* userdata, uint8_t* stream, int bytes)
{
	int16_t* buffer = (int16_t*)stream;
	int16_t temp = 0;
	for (int i = 0; i < bytes / 2; i++) //bytes/2 due to 16-bit depth
	{
		buffer[i] = (int16_t)(24000 * sin(2.0f*M_PI*(float)beepFreq*(float)audio_time / 44100.0f)); //28000 = amp, 44100 = sample rate
		audio_time++;
	}
}