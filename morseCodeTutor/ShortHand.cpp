#include "Global.h"

void drawText(const char* str, int x, int y)
{
	if (*str == '\0')
		return;
	//Text render guide = Solid is fast quality, Shaded or Blended is high quality, LCD is subpixel quality
	//    Use UTF8. Text, UNICODE, and Glyh32 also exist.
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(myFont, str, Pen);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	//Here dstrect is an SDL_Rect with format x,y,w,h with origin at upper left
	const SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

void drawTextCentered(const char* str, int x, int y)
{
	if (*str == '\0')
		return;
	//Text render guide = Solid is fast quality, Shaded or Blended is high quality, LCD is subpixel quality
	//    Use UTF8. Text, UNICODE, and Glyh32 also exist.
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(myFont, str, Pen);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	//Here dstrect is an SDL_Rect with format x,y,w,h with origin at upper left
	const SDL_Rect textRect = { SCREEN_WIDTH / 2 - textSurface->w / 2 + x, y, textSurface->w, textSurface->h };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

void setFontSize(uint8_t in)
{
	textSize = in;
	myFont = TTF_OpenFont("source-code-pro.ttf", fontSizes[textSize]);
}

void drawTextWrapped(const char* str, int x, int y, int w, int lineSpace)
{
	strcpy(tempStr, str);
	//maxCharWidth = w / fontPixels[textSize]
	uint8_t startIndex = 0;
	uint8_t lastIndex=0;
	while (tempStr[lastIndex] != '\0')
	{
		if (lastIndex-startIndex == w / fontPixels[textSize])
		{
			while (tempStr[lastIndex] != '\n'&&tempStr[lastIndex] != ' ')
				lastIndex--;
			if (lastIndex == startIndex) //In case a word is too big too fit on a single line
				return;
			tempStr[lastIndex] = '\0';
			drawText(&tempStr[startIndex], x, y);
			y += lineSpace;
			startIndex = lastIndex+1;
			lastIndex = startIndex;
		}
		lastIndex++;
	}
    drawText(&tempStr[startIndex], x, y);
}

void drawTextCenteredWrapped(const char* str, int x, int y, int w, int lineSpace)
{
	strcpy(tempStr, str);
	//maxCharWidth = w / fontPixels[textSize]
	uint8_t startIndex = 0;
	uint8_t lastIndex = 0;
	while (tempStr[lastIndex] != '\0')
	{
		if (tempStr[lastIndex]=='\n'||(lastIndex - startIndex == w / fontPixels[textSize])) //Time to draw a line
		{
			while (tempStr[lastIndex] != '\n'&&tempStr[lastIndex] != ' ')
				lastIndex--;
			if (lastIndex == startIndex) //In case a word is too big too fit on a single line
				return;
			tempStr[lastIndex] = '\0';
			drawTextCentered(&tempStr[startIndex], x, y);
			y += lineSpace;
			startIndex = lastIndex + 1;
			lastIndex = startIndex;
		}
		lastIndex++;
	}
	drawTextCentered(&tempStr[startIndex], x, y);
}

void background(uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(renderer,r,g,b,255); //Color guide = RGB, opacity, 8-bit each
	SDL_RenderClear(renderer);
}

void asciiToMorse(char* ascii, char* morse) //Translate char* ascii into morse code and insert into char* morse
{
	for (uint16_t i = 0; morse[i] != '\0'; i++) //First clear morse so that strcat works properly (this might not be necessary)
		morse[i] = '\0';
	if (ascii[0] == '\0')
		return; //Failsafe for empty strings
	strcat(morse, myMorse[toupper(ascii[0]) - 32]);
	for (uint16_t i = 1; ascii[i] != '\0'; i++) {
		//char* currentMorse = myMorse[toupper(example[i]) - 32];    //prints the index minus 32, because the array starts at " " which is 32, and ends at "_" which is 95
		//no need to include the lowercase ascii because the toupper() sets everything to upper case.
		if (ascii[i] != ' ' && ascii[i - 1] != ' ')
			strcat(morse, charDelimiter);
		strcat(morse, myMorse[toupper(ascii[i]) - 32]);
	}
}

void typeTextbox(int keyInput)
{
	text[text_i] = keyInput;
	SDL_SetRenderDrawColor(renderer, Pen.r, Pen.g, Pen.b, Pen.a);
	drawText((char*)&keyInput, 10 + (text_i % 40)*fontPixels[textSize], 100 + (text_i / 40) * lineSpace1);
	text_i++;
	SDL_RenderPresent(renderer);
}

void bkspcTextbox()
{
	text_i--;
	text[text_i] = '\0';
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect bkspcRect = { 10 + (text_i % 40)*fontPixels[textSize], 100 + (text_i / 40) * lineSpace1, fontPixels[textSize], lineSpace1 };
	SDL_RenderFillRect(renderer, &bkspcRect);
	SDL_RenderPresent(renderer);
}

bool isValidChar(char c)
{
	return (c >= 32 && c <= 34) || c == 36 || (c >= 38 && c <= 41) || (c >= 43 && c <= 59) || c == 61 || c == 63 || c == 64 || c == 95 || (c >= 97 && c <= 122);
}

void setWPM(int newWPM) //Ensures wpm and time unit length are both set
{
	wpm = newWPM;
	if (newWPM != 0)
	{
		msPerDit = round(1200.0 / wpm); //Can be set to NaN, but screen will be stuck on Options
		tolerance = msPerDit * 3 / 4;
	}
}

void setFWPM(int newFWPM) //Ensures fwpm and f time unit length are both set
{
	if (newFWPM != 0)
	{
		fwpm = newFWPM;
		msPerFDit = round(1200.0 * (50.0 * wpm - 31.0 * fwpm) / 19.0 / wpm / fwpm); //Can be set to NaN, but screen will be stuck on Options
	}
}

void clearStats() //Resets all stats
{
	totalDitDuration = 0;
	totalDahDuration = 0;
	totalShortPauseDuration = 0;
	totalCharPauseDuration = 0;
	totalWordPauseDuration = 0;
	ditCount = 0;
	dahCount = 0;
	shortPauseCount = 0;
	charPauseCount = 0;
	wordPauseCount = 0;
}

void pollStraightKey(SDL_Event* event) //Check "straight key" and update downDuration, upDuration & lastChange
{
	if (event->key.keysym.sym == straightKey && event->key.repeat == 0) //The straight key has changed & it is not from a repeated keystroke
	{
		if (event->type == SDL_KEYUP) //Rising edge
		{
			state = UP;
			downDuration = (event->key.timestamp - lastChange); //Use event->key.timestamp for time
			upDuration = 0;
			lastChange = event->key.timestamp;
		}
		else if (event->type == SDL_KEYDOWN) //Falling edge
		{
			state = DOWN;
			upDuration = (event->key.timestamp - lastChange);
			downDuration = 0;
			lastChange = event->key.timestamp;
		}
	}
}

void drawFilledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint8_t r, uint8_t g,uint8_t b)
{
	SDL_SetRenderDrawColor(renderer,r,g,b,255);
	SDL_Rect rect = { x,y,w,h };
	SDL_RenderFillRect(renderer, &rect);
}


void readDashDot() // Read the next dash or dot in the dash-dot sequence and add to input
{
	if (input_i != input_length - 1) //Prevents indexing out of bounds
	{
		if (downDuration >= 2 * msPerDit && downDuration < (3 * msPerDit + 2 * tolerance))
		{
			input[input_i] = '-';
			input_i++;
			totalDahDuration += downDuration;
			dahCount++;
			error[error_i] = downDuration < 3 * msPerDit;
			error_i++;
			error_i = error_i & 7; //Cheap mod 8 operation
		}
		else if (downDuration >= (msPerDit - tolerance) && downDuration < (msPerDit + tolerance))
		{
			input[input_i] = '.';
			input_i++;
			totalDitDuration += downDuration;
			ditCount++;
			error[error_i] = downDuration < msPerDit;
			error_i++;
			error_i = error_i & 7; //Cheap mod 8 operation
		}
		else
			input[input_i] = '\0';
	}
}

char morseToAscii() // Convert Morse code (contained in input) to ASCII character
{
	uint8_t alphabet_i = 0;
	uint8_t sequence_length = input_i;
	input_i = 0;
	//Break when myMorse[alphabet_i][input_i] == '\0' (correct) or alphabet_i == 64 (not found)
	while ((myMorse[alphabet_i][input_i] != '\0' || input[input_i] != '\0') && alphabet_i != 64) //If both strings completely match, the char is returned
	{
		if (myMorse[alphabet_i][input_i] == input[input_i])
		{
			input_i++;
		}
		else
		{
			input_i = 0;
			alphabet_i++;
		}
	}
	if (alphabet_i == 64)
	{
		//Check if 8 dits are at the end
		if (sequence_length >= 8 && strcmp((char*)(input + sequence_length - 8), "........\0") == 0)
		{
			//Error has been tapped
			morseDelete = true;
		}
		return '\0'; //Returns null char if unrecognized
	}
	return (char)(alphabet_i + 32);
}