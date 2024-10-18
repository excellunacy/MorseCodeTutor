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
 * Supplementary code file containing commonly used functions for
 * drawing, calculating with variables, and performing frequent
 * actions.
*/

#include "Global.h"
char wrapTextStr[strLength] = { 0 }; //Holds a copy of a string to be wrapped so that it can be modified

void drawText(const char* str, uint16_t x, uint16_t y)
{
	if (*str == '\0')
		return;
	//Text render guide = Solid is fast quality, Shaded or Blended is high quality, LCD is subpixel quality
	//Text formats include UTF8, text, UNICODE, and Glyh32
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(myFont, str, Pen);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	const SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

void drawTextCentered(const char* str, int16_t x, uint16_t y)
{
	if (*str == '\0')
		return;
	//Text render guide = Solid is fast quality, Shaded or Blended is high quality, LCD is subpixel quality
	//Text formats include UTF8, text, UNICODE, and Glyh32
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(myFont, str, Pen);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	const SDL_Rect textRect = { SCREEN_WIDTH / 2 - textSurface->w / 2 + x, y, textSurface->w, textSurface->h };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

void setFontSize(uint8_t in)
{
	if (in < 7)
	{
		fontSize = in;
		myFont = TTF_OpenFont("source-code-pro.ttf", fontSizes[fontSize]);
	}
}

void drawTextWrapped(const char* str, uint16_t x, uint16_t y, uint16_t w, uint16_t lineSpace)
{
	strcpy(wrapTextStr, str);
	uint16_t startIndex = 0;
	uint16_t lastIndex = 0;
	while (wrapTextStr[lastIndex] != '\0')
	{
		//Max chars on line = w / fontPixels[fontSize]
		if (wrapTextStr[lastIndex] == '\n' || (lastIndex - startIndex == w / fontPixels[fontSize])) //New line at lastIndex
		{
			while (wrapTextStr[lastIndex] != '\n'&&wrapTextStr[lastIndex] != ' ') //Back track so that a word isn't split
				lastIndex--;
			if (lastIndex == startIndex && wrapTextStr[startIndex] != '\n') //In case a word is too big too fit on a single line or there are consecutive new lines
				return;
			wrapTextStr[lastIndex] = '\0';
			drawText(&wrapTextStr[startIndex], x, y);
			y += lineSpace;
			startIndex = lastIndex + 1;
			lastIndex = startIndex;
		}
		else
			lastIndex++;
	}
	drawText(&wrapTextStr[startIndex], x, y);
}

void drawTextCenteredWrapped(const char* str, int16_t x, uint16_t y, uint16_t w, uint16_t lineSpace)
{
	strcpy(wrapTextStr, str);
	uint16_t startIndex = 0;
	uint16_t lastIndex = 0;
	while (wrapTextStr[lastIndex] != '\0')
	{
		//Max chars on line = w / fontPixels[fontSize]
		if (wrapTextStr[lastIndex] == '\n' || (lastIndex - startIndex == w / fontPixels[fontSize])) //New line at lastIndex
		{
			while (wrapTextStr[lastIndex] != '\n' && wrapTextStr[lastIndex] != ' ') //Back track so that a word isn't split
				lastIndex--;
			if (lastIndex == startIndex && wrapTextStr[startIndex] != '\n') //In case a word is too big too fit on a single line or there are consecutive new lines
				return;
			wrapTextStr[lastIndex] = '\0';
			drawTextCentered(&wrapTextStr[startIndex], x, y);
			y += lineSpace;
			startIndex = lastIndex + 1;
			lastIndex = startIndex;
		}
		else
			lastIndex++;
	}
	drawTextCentered(&wrapTextStr[startIndex], x, y);
}

void drawColorText(const char* str, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
	if (*str == '\0')
		return;
	//Text render guide = Solid is fast quality, Shaded or Blended is high quality, LCD is subpixel quality
	//Text formats include UTF8, text, UNICODE, and Glyh32
	SDL_Color color = { r,g,b,255 };
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(myFont, str, color);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	const SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

void drawColorTextCentered(const char* str, int16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
	if (*str == '\0')
		return;
	//Text render guide = Solid is fast quality, Shaded or Blended is high quality, LCD is subpixel quality
	//Text formats include UTF8, text, UNICODE, and Glyh32
	SDL_Color color = { r,g,b,255 };
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(myFont, str, color);
	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	const SDL_Rect textRect = { SCREEN_WIDTH / 2 - textSurface->w / 2 + x, y, textSurface->w, textSurface->h };
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

void background(uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_RenderClear(renderer);
}

void asciiToMorse(const char* ascii, char* morse)
{
	morse[0] = '\0'; //Clears morse. strcat will now treat morse like a fully empty string
	if (ascii[0] == '\0')
		return; //Failsafe for empty strings
	strcat(morse, myMorse[toupper(ascii[0]) - 32]); //Add char. -32 shifts char value to proper index
	for (uint16_t i = 1; ascii[i] != '\0'; i++)
	{
		if (ascii[i] != ' ' && ascii[i - 1] != ' ') //Place ',' between characters within each word
			strcat(morse, charDelimiter);
		strcat(morse, myMorse[toupper(ascii[i]) - 32]); //Spaces between words are accounted for here
	}
}

void typeHighTextBox(uint16_t keyInput)
{
	if (keyInput == '\0' || (keyInput == ' ' && (text_i == 0 || text[text_i - 1] == ' '))) //Prevents consecutive spaces or initial space
		return;
	text[text_i] = keyInput;
	SDL_SetRenderDrawColor(renderer, Pen.r, Pen.g, Pen.b, Pen.a);
	//Using an int and casting to char* produces a string with a single char followed by a '\0'
	drawText((char*)&keyInput, listX + (text_i % charsPerLine)*fontPixels[fontSize], 80 + lineSpace2 + (text_i / charsPerLine) * lineSpace2);
	text_i++; //Null termination must be ensured elsewhere
	SDL_RenderPresent(renderer);
}

void bkspcHighTextBox()
{
	if (text_i == 0)
		return;
	text_i--;
	text[text_i] = '\0';
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect bkspcRect = { listX + (text_i % charsPerLine)*fontPixels[fontSize], 85 + lineSpace2 + (text_i / charsPerLine) * lineSpace2, fontPixels[fontSize], lineSpace2 };
	SDL_RenderFillRect(renderer, &bkspcRect);
	SDL_RenderPresent(renderer);
}

void typeLowTextBox(uint16_t keyInput)
{
	if (keyInput == '\0' || (keyInput == ' ' && (text_i == 0 || text[text_i - 1] == ' '))) //Prevents consecutive spaces or initial space
		return;
	text[text_i] = keyInput;
	SDL_SetRenderDrawColor(renderer, Pen.r, Pen.g, Pen.b, Pen.a);
	//Using an int and casting to char* produces a string with a single char followed by a '\0'
	drawText((char*)&keyInput, listX + (text_i % charsPerShortLine)*fontPixels[fontSize], 200 + (text_i / charsPerShortLine) * lineSpace2);
	text_i++; //Null termination must be ensured elsewhere
	SDL_RenderPresent(renderer);
}

void bkspcLowTextBox()
{
	if (text_i == 0)
		return;
	text_i--;
	text[text_i] = '\0';
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect bkspcRect = { listX + (text_i % charsPerShortLine)*fontPixels[fontSize], 205 + (text_i / charsPerShortLine) * lineSpace2, fontPixels[fontSize], lineSpace2 };
	SDL_RenderFillRect(renderer, &bkspcRect);
	SDL_RenderPresent(renderer);
}

void setWPM(uint16_t newWPM)
{
	wpm = newWPM;
	if (newWPM != 0)
	{
		msPerDit = round(1200.0 / wpm);
		tolerance = msPerDit * 3 / 4;
	}
}

void setFWPM(uint16_t newFWPM)
{
	fwpm = newFWPM;
	if (newFWPM != 0)
	{
		msPerFDit = round(1200.0 * (50.0 * wpm - 31.0 * fwpm) / 19.0 / wpm / fwpm);
	}
}

void clearStats()
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
	ditSamples_i = 0;
	ditSamplesSize = 0;
	dahSamples_i = 0;
	dahSamplesSize = 0;
	shortPauseSamples_i = 0;
	shortPauseSamplesSize = 0;
	charPauseSamples_i = 0;
	charPauseSamplesSize = 0;
	wordPauseSamples_i = 0;
	wordPauseSamplesSize = 0;
}

void checkStraightKey(SDL_Event* event)
{
	if (event->key.keysym.sym == tolower(straightKey) && event->key.repeat == 0) //The straight key has changed & it is not from a repeated keystroke
	{
		if (event->type == SDL_KEYUP) //Rising edge
		{
			state = UP;
			downDuration = (event->key.timestamp - lastChange);
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

void drawFilledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_Rect rect = { x,y,w,h };
	SDL_RenderFillRect(renderer, &rect);
}

void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	SDL_Rect rect = { x,y,w,h };
	SDL_RenderDrawRect(renderer, &rect);
}

void readDashDot()
{
	if (input_i < input_length-1) //Prevents indexing out of bounds or overwriting the null terminator
	{
		if (downDuration >= 2 * msPerDit && downDuration < (3 * msPerDit + 2 * tolerance)) //Dah is interpreted as between 2 & 4.5 time units
		{
			input[input_i] = '-';
			input_i++;
			totalDahDuration += downDuration; //Stats are taken
			dahCount++;
			if (downDuration < 3 * msPerDit) //Used for automatically tuning WPM
				error[error_i] = fast;
			else
				error[error_i] = slow;
			error_i++;
			error_i = error_i & 7; //Cheap mod 8 operation
			dahSamples[dahSamples_i] = downDuration; //Used for calculating std dev
			dahSamples_i++;
			if (dahSamples_i == numSamples)
				dahSamples_i = 0;
			if (dahSamplesSize < numSamples + 1)
				dahSamplesSize++;
		}
		else if (downDuration >= (msPerDit - tolerance) && downDuration < (msPerDit + tolerance)) //Dit is interpreted as between 0.25 & 1.75 time units
		{
			input[input_i] = '.';
			input_i++;
			totalDitDuration += downDuration;
			ditCount++;
			if (downDuration < msPerDit) //Used for automatically tuning WPM
				error[error_i] = fast;
			else
				error[error_i] = slow;
			error_i++;
			error_i = error_i & 7; //Cheap mod 8 operation
			ditSamples[ditSamples_i] = downDuration; //Used for calculating std dev
			ditSamples_i++;
			if (ditSamples_i == numSamples)
				ditSamples_i = 0;
			if (ditSamplesSize < numSamples + 1)
				ditSamplesSize++;
		}
		else
			input[input_i] = '\0';
	}
}

char morseToAscii()
{
	uint8_t alphabet_i = 0;
	uint8_t sequence_length = input_i;
	input_i = 0;
	//Break if strings fully match or Morse sequence is not found in myMorse
	while (strcmp(myMorse[alphabet_i], input) != 0 && alphabet_i != 64)
	{
		alphabet_i++;
	}
	if (alphabet_i == 64)
	{
		//Check for prosign "HH" at the end
		//Should be 8 dits, but 7 is accepted as well
		if (sequence_length >= 7 && strcmp((char*)(input + sequence_length - 7), ".......\0") == 0)
		{
			//Error has been tapped
			morseDelete = true;
		}
		return '\0'; //Returns null char if unrecognized
	}
	return (char)(alphabet_i + 32);
}


void drawOption(uint8_t option)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	drawFilledRect(0, 345, 640, lineSpace2, 255, 255, 255); //Erases "Invalid Settings" upon update
	switch (option)
	{
	case 0: //WPM
		drawFilledRect(listX + fontPixels[fontSize] * 11, 55, fontPixels[fontSize] * 2, lineSpace2, 255, 255, 255); //Erase previous numbers
		sprintf(tempStr, "%2d", wpm);
		drawText(tempStr, listX + fontPixels[fontSize] * 11, 50);
		break;
	case 1: //Enable Farnsworth Timing
		if (fEnable) //Draw x in box, remove strikethrough & FWPM and rewrite FWPM line
		{
			SDL_SetRenderDrawColor(renderer, Pen.r, Pen.g, Pen.b, Pen.a);
			SDL_RenderDrawLine(renderer, 501, 91, 527, 117);
			SDL_RenderDrawLine(renderer, 501, 117, 527, 91);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			drawFilledRect(5, 55 + lineSpace3 * 2, 300, lineSpace2, 255, 255, 255); //Remove strikethrough
			drawText("Farnsworth WPM: ", listX, 50 + lineSpace3 * 2);
			//Note no break statement
		}
		else //Remove x & add strikethrough
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDrawLine(renderer, 501, 91, 527, 117);
			SDL_RenderDrawLine(renderer, 502, 116, 526, 92);
			SDL_SetRenderDrawColor(renderer, Pen.r, Pen.g, Pen.b, Pen.a);
			SDL_RenderDrawLine(renderer, listX - 4, 65 + lineSpace3 * 2, 300, 65 + lineSpace3 * 2);
			break;
		}
	case 2: //FWPM
		drawFilledRect(listX + fontPixels[fontSize] * 16, 55 + lineSpace3 * 2, fontPixels[fontSize] * 2, lineSpace2, 255, 255, 255); //Erase previous numbers
		sprintf(tempStr, "%2d", fwpm);
		drawText(tempStr, listX + fontPixels[fontSize] * 16, 50 + lineSpace3 * 2);
		break;
	case 3: //Beep frequency
		drawFilledRect(listX + fontPixels[fontSize] * 19, 55 + lineSpace3 * 3, fontPixels[fontSize] * 4, lineSpace2, 255, 255, 255); //Erase previous numbers
		sprintf(tempStr, "%4d", beepFreq);
		drawText(tempStr, listX + fontPixels[fontSize] * 19, 50 + lineSpace3 * 3);
		break;
	case 7: //Straight Key Key
		drawFilledRect(listX + fontPixels[fontSize] * 18, 55 + lineSpace3 * 7, fontPixels[fontSize], lineSpace2, 255, 255, 255); //Erase previous character
		sprintf(tempStr, "%c", straightKey);
		drawText(tempStr, listX + fontPixels[fontSize] * 18, 50 + lineSpace3 * 7);
	}
	SDL_RenderPresent(renderer);
}

void updateOptionChoice(uint8_t newChoice)
{
	switch (choice) //Erase box over previous selection
	{
	case 0:
		drawRect(listX - 5 + fontPixels[fontSize] * 11, 52, 10 + fontPixels[fontSize] * 2, 30, 255, 255, 255);
		break;
	case 1:
		drawRect(492, 82, 44, 44, 255, 255, 255);
		break;
	case 2:
		drawRect(listX - 5 + fontPixels[fontSize] * 16, 52 + lineSpace3 * 2, 10 + fontPixels[fontSize] * 2, 30, 255, 255, 255);
		break;
	case 3:
		drawRect(listX - 5 + fontPixels[fontSize] * 19, 52 + lineSpace3 * 3, 10 + fontPixels[fontSize] * 4, 30, 255, 255, 255);
		break;
	case 4:
		drawRect(SCREEN_WIDTH / 2 - fontPixels[fontSize] * 4 - 155, 50 + lineSpace3 * 6, 10 + fontPixels[fontSize] * 8, 30, 255, 255, 255);
		break;
	case 5:
		drawRect(SCREEN_WIDTH / 2 - fontPixels[fontSize] * 3 - 5, 50 + lineSpace3 * 6, 10 + fontPixels[fontSize] * 6, 30, 255, 255, 255);
		break;
	case 6:
		drawRect(SCREEN_WIDTH / 2 - fontPixels[fontSize] * 3 + 145, 50 + lineSpace3 * 6, 10 + fontPixels[fontSize] * 6, 30, 255, 255, 255);
		break;
	case 7:
		drawRect(listX - 5 + fontPixels[fontSize] * 18, 52 + lineSpace3 * 7, 10 + fontPixels[fontSize], 30, 255, 255, 255);
	}
	choice = newChoice;
	switch (choice) //Draw box over current selection
	{
	case 0:
		drawRect(listX - 5 + fontPixels[fontSize] * 11, 52, 10 + fontPixels[fontSize] * 2, 30, Pen.r, Pen.g, Pen.b);
		break;
	case 1:
		drawRect(492, 82, 44, 44, Pen.r, Pen.g, Pen.b);
		break;
	case 2:
		drawRect(listX - 5 + fontPixels[fontSize] * 16, 52 + lineSpace3 * 2, 10 + fontPixels[fontSize] * 2, 30, Pen.r, Pen.g, Pen.b);
		break;
	case 3:
		drawRect(listX - 5 + fontPixels[fontSize] * 19, 52 + lineSpace3 * 3, 10 + fontPixels[fontSize] * 4, 30, Pen.r, Pen.g, Pen.b);
		break;
	case 4:
		drawRect(SCREEN_WIDTH / 2 - fontPixels[fontSize] * 4 - 155, 50 + lineSpace3 * 6, 10 + fontPixels[fontSize] * 8, 30, Pen.r, Pen.g, Pen.b);
		break;
	case 5:
		drawRect(SCREEN_WIDTH / 2 - fontPixels[fontSize] * 3 - 5, 50 + lineSpace3 * 6, 10 + fontPixels[fontSize] * 6, 30, Pen.r, Pen.g, Pen.b);
		break;
	case 6:
		drawRect(SCREEN_WIDTH / 2 - fontPixels[fontSize] * 3 + 145, 50 + lineSpace3 * 6, 10 + fontPixels[fontSize] * 6, 30, Pen.r, Pen.g, Pen.b);
		break;
	case 7:
		drawRect(listX - 5 + fontPixels[fontSize] * 18, 52 + lineSpace3 * 7, 10 + fontPixels[fontSize], 30, Pen.r, Pen.g, Pen.b);
	}
	SDL_RenderPresent(renderer);
}

void drawMorse(const char* morseSequence)
{
	uint8_t draw_i = 0;
	for (int i = 0; morseSequence[i] != '\0'; i++)
	{
		switch (morseSequence[i]) //Each time unit is 8 pixels wide. Dits are a 8x8 square, dahs are a 24x4 rectangle that is 2 pixels lower
		{
		case '.':
			drawFilledCircle(listX + 4 + 16 * (draw_i % 37), 83 + lineSpace2 * 6 + lineSpace2 * (draw_i / 37), 4, Pen.r, Pen.g, Pen.b);
			draw_i += 1;
			break;
		case '-':
			if (draw_i % 37 == 36) //If a line ends on a dash, it won't fit, so it is moved to the next line
			{
				draw_i += 1;
			}
			drawFilledRect(listX + 16 * (draw_i % 37), 82 + lineSpace2 * 6 + lineSpace2 * (draw_i / 37), 24, 4, Pen.r, Pen.g, Pen.b);
			draw_i += 2;
			break;
		case ',': //Pause between characters
			draw_i += 1;
			break;
		case ' ': //Longer pause between words
			draw_i += 3;
			break;
		}
	}
}

void drawOsuChart()
{
	//Centered at x = 40, 160, & 400
	drawFilledRect(36, 430, 8, 32, Pen.r, Pen.g, Pen.b);
	drawFilledRect(156, 430, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(156, 454, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(396, 430, 8, 8, Pen.r, Pen.g, Pen.b);
	drawFilledRect(396, 454, 8, 8, Pen.r, Pen.g, Pen.b);
}

void drawFilledCircle(uint16_t xCenter, uint16_t yCenter, uint16_t radius, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
	//Draw point if (x-xCenter)^2 + (y-yCenter)^2 <= radius^2
	for (int y = yCenter - radius; y <= yCenter + radius; y++)
	{
		for (int x = xCenter - radius; x <= xCenter + radius; x++)
		{
			if ((x - xCenter)*(x - xCenter) + (y - yCenter)*(y - yCenter) <= radius * radius) //x^2 + y^2 <= r^2
				SDL_RenderDrawPoint(renderer, x, y);
		}
	}
}

char keysymToValidChar(SDL_Keysym* keysym)
{
	if (keysym->sym > 256) //If key pressed cannot be represented as a char
		return '\0';
	char c = '\0'; //Default is invalid
	if ('a' <= keysym->sym && keysym->sym <= 'z') //If letter (ignore case)
	{
		c = toupper(keysym->sym);
	}
	else //If number or symbol
	{
		//The ! casts to boolean. True if Lshift or Rshift are on. Caps lock does not affect numbers/symbols
		if (!!(keysym->mod & 3)) //Uppercase
		{
			switch (keysym->sym)
			{
			case '1':
				c = '!';
				break;
			case '\'':
				c = '"';
				break;
			case '4':
				c = '$';
				break;
			case '7':
				c = '&';
				break;
			case '9':
				c = '(';
				break;
			case '0':
				c = ')';
				break;
			case '=':
				c = '+';
				break;
			case ';':
				c = ':';
				break;
			case '/':
				c = '?';
				break;
			case '2':
				c = '@';
				break;
			case '-':
				c = '_';
				break;
			case ' ': //Spacebar has no capital
				c = ' ';
			}
		}
		else //Lowercase
		{
			c = keysym->sym;
		}
	}
	//Checks if c can be translated into Morse Code with myMorse
	if ((' ' <= c && c <= '"') || c == '$' || ('&' <= c && c <= ')') || ('+' <= c && c <= ';') || c == '=' || ('?' <= c && c <= 'Z') || c == '_')
	{
		return c;
	}
	return '\0';
}

void drawStdDev(uint16_t* samples, uint16_t totalSamples, uint16_t mean, uint16_t y)
{
	uint16_t tempStdDev;
	drawText("Standard Dev.:", listX + fontPixels[fontSize] * 2, y);
	if (totalSamples > numSamples)
		tempStdDev = calcStdDev(samples, numSamples, mean, false);
	else
		tempStdDev = calcStdDev(samples, totalSamples, mean, true);
	sprintf(tempStr, "%d.%03ds", (tempStdDev / 1000) % 10, tempStdDev % 1000);
	drawText(tempStr, listX + fontPixels[fontSize] * 20, y);
}

uint16_t calcStdDev(uint16_t* samples, uint16_t samplesSize, uint16_t mean, bool fullPop)
{
	if (samplesSize == 0)
		return 0;
	//Formula for std dev = sqrt(sum(differenceFromMean^2)/samples)
    //Subtract 1 from samples if samples does not represent full population
    //std dev = sqrt(variance)
	uint32_t sum = 0;
	for (int i = 0; i < samplesSize; i++)
	{
		sum += (samples[i] - mean)*(samples[i] - mean);
	}
	if (fullPop)
	{
		sum /= samplesSize;
	}
	else //Bessel's correction
		sum /= (samplesSize - 1);
	return (uint16_t)sqrt(sum);
}