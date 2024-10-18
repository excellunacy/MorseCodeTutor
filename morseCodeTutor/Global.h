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
 * Header file for both main.cpp and ShortHand.cpp containing
 * libraries, constants, word banks, global variables and function
 * headers.
*/

#include <SDL.h> //SDL2
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

//Screens/modes
#define start           0
#define mainMenu        1
#define typeAtoM        2
#define transMtoA       3
#define hearMorse       4
#define options         5
#define mToAGameChoice  6
#define mToAGame        7
#define chooseTutorial  8
#define tutorial        9
#define tutorialFinish 10
#define explainMorse   11
#define gameModes      12
#define qCodes         13
#define seeResult      14
#define quit           15

//Game Modes for Practicing Morse Interpreting/Tapping
#define MORSE_CHARACTERS 1
#define MORSE_WORDS 2
#define MORSE_PHRASES 3

#define strLength        200
#define input_length      20 //Dot/dash sequence for single char. Extra long in case error is inputted with prosign "HH"
#define numOptions         8
#define numWords          50
#define numPhrases        30
#define excluded_length   10
#define fast               1
#define slow               2
#define DOWN              true
#define UP                false
#define charsPerLine      40
#define charsPerShortLine 30
#define numSamples        20

//GUI Common Values
#define lineSpace1    20
#define lineSpace2    25
#define lineSpace3    35
#define escX		  5
#define escY		  5
#define titleY		  50
#define listX	      40
#define listY	      120
#define instructY     340
#define flashX        590
#define flashY        400
#define flashR        15

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const char myMorse[64][8] = { " ", "-.-.--", ".-..-.", "", "...-..-", "", ".-...", ".----.", "-.--.", "-.--.-", "", ".-.-.", "--..--", "-....-", ".-.-.-", "-..-.", "-----", ".----",
						 "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "---...", "-.-.-.", "", "-...-", "", "..--..", ".--.-.", ".-", "-...", "-.-.", "-..", ".", "..-.",
						 "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "", "", "", "", "..--.-"
}; //Morse to ASCII table. Contains chars from ' ' (32) to '_' (95) in order
const char charDelimiter[2] = ","; //Used to separate characters when stored in Morse Code

//Array of words for both hearing and tapping games
const char words[numWords][11] = {
  "HELLO",
  "HI",
  "WORLD",
  "START",
  "END",
  "MESSAGE",
  "PRACTICE",
  "TODAY",
  "MORNING",
  "AFTER",
  "EVENING",
  "GOOD",
  "MORSE",
  "THE",
  "EASY",
  "WELCOME",
  "COMPUTER",
  "SUCCESS",
  "AND",
  "HOWEVER",
  "GREAT",
  "FOX",
  "ZEBRA",
  "QUEST",
  "PLAY",
  "JACK",
  "SUMMER",
  "TWIST",
  "OVER",
  "CHANGE",
  "GUESS",
  "SHARK",
  "AGAIN",
  "MANY",
  "ASK",
  "STRIKE",
  "BUILD",
  "SWEET",
  "CHIP",
  "FUN",
  "COMPANY",
  "TEAM",
  "WHERE",
  "HOW",
  "BIKE",
  "VIRTUE",
  "SLEEP",
  "STAND",
  "PHONE",
  "COOL"
};
// Array of phrases/sentences (Currently must be 30 characters or less with, so no wrapping needed)
const char phrases[numPhrases][31] = {
  "HELLO WORLD",
  "HAVE A NICE DAY",
  "SEND THIS MESSAGE",
  "HAPPY BIRTHDAY",
  "STOP BY AND RELAX",
  "HOW ARE YOU",
  "ON YOUR MARK",
  "UNTIL TOMORROW",
  "WHAT TIME IS IT",
  "ON MY WAY",
  "THANK YOU AGAIN",
  "BETTER LATE THAN NEVER",
  "I AM AT THE LIBRARY",
  "THE STORE IS OPEN",
  "APPLES AND ORANGES",
  "PAINT A PICTURE",
  "LIFTING WEIGHTS",
  "HOME SWEET HOME",
  "LEARNING MORSE CODE",
  "BIDING MY TIME",
  "LIVE AND LEARN",
  "FRIEND OF THE FAMILY",
  "RAISE THE STAKES",
  "START AT THE BEGINNING",
  "FOLLOW MY LEAD",
  "GIVE ME A SECOND",
  "BRING A SCREWDRIVER",
  "REACH A NEW GOAL",
  "THE TREES ARE RED",
  "TALK TO YOU LATER"
};
const char qCodePhrases[16][40] = {
  "Shall I stand by?",                  //QRX
  "Does my frequency vary?",            //QRH
  "What is the correct time?",          //QTR
  "Are you busy?",                      //QRL
  "Do you have interference?",          //QRM
  "Shall I stop sending?",              //QRT
  "Shall I increase power?",            //QRO
  "Shall I decrease power?",            //QRP
  "Shall I send faster?",               //QRQ
  "Shall I send more slowly?",          //QRS
  "Have you anything for me?",          //QRU
  "Are you ready?",                     //QRV
  "Who is calling me?",                 //QRZ
  "Are my signals fading?",             //QSB
  "Is my keying defective?",            //QSD
  "Can you acknowledge receipt?"        //QSL
};
const char qCodeCodes[16][4] = { "QRX", "QRH", "QTR", "QRL", "QRM", "QRT", "QRO", "QRP", "QRQ", "QRS", "QRU", "QRV", "QRZ", "QSB", "QSD", "QSL" };
const char PopAbb[16][20] = {
  "Again", //AGN
  "Break", //BK
  "Better", //BTR
  "Yes", //C
  "Confirm", //CFM
  "Callsign", //CS
  "Conditions", //CX
  "Emergency", //EMRG
  "For", //FER
  "From", //FM
  "Frequency", //FREQ
  "Go Ahead", //GA
  "Good", //GUD
  "I say again", //II
  "Over", //KN
  "No" //N
};
const char PopAbbKey[16][6] = { "AGN", "BK", "BTR", "C", "CFM", "CS", "CX", "EMRG", "FER", "FM", "FREQ", "GA", "GUD", "II", "KN", "N" };

const uint8_t fontSizes[7] = { 12, 18, 24, 36, 48, 60, 72 }; //"source-code-pro.ttf" contains these font sizes
const uint8_t fontPixels[7] = { 7, 11, 14, 22, 29, 36, 43 }; //The pixels that each char takes up when drawn, aligned with fontSizes

//Global variables
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Color Pen;
extern uint8_t fontSize;
extern TTF_Font* myFont;
extern char tempStr[strLength];
extern char text[strLength];
extern uint16_t text_i;
extern uint32_t msPerDit;
extern uint32_t msPerFDit;
extern uint16_t wpm;
extern uint16_t fwpm;
extern bool fEnable;
extern uint32_t tolerance;
extern uint32_t totalDitDuration;
extern uint32_t totalDahDuration;
extern uint32_t totalShortPauseDuration;
extern uint32_t totalCharPauseDuration;
extern uint32_t totalWordPauseDuration;
extern uint16_t ditCount;
extern uint16_t dahCount;
extern uint16_t shortPauseCount;
extern uint8_t  charPauseCount;
extern uint8_t  wordPauseCount;
extern int straightKey;
extern uint32_t lastChange;
extern uint16_t downDuration;
extern uint16_t upDuration;
extern bool state;
extern char input[input_length];
extern uint8_t input_i;
extern uint8_t error[8];
extern uint8_t error_i;
extern bool morseDelete;
extern uint8_t choice;
extern uint16_t beepFreq;
extern uint16_t ditSamples[numSamples];
extern uint8_t ditSamples_i;
extern uint8_t ditSamplesSize;
extern uint16_t dahSamples[numSamples];
extern uint8_t dahSamples_i;
extern uint8_t dahSamplesSize;
extern uint16_t shortPauseSamples[numSamples];
extern uint8_t shortPauseSamples_i;
extern uint8_t shortPauseSamplesSize;
extern uint16_t charPauseSamples[numSamples];
extern uint8_t charPauseSamples_i;
extern uint8_t charPauseSamplesSize;
extern uint16_t wordPauseSamples[numSamples];
extern uint8_t wordPauseSamples_i;
extern uint8_t wordPauseSamplesSize;


//Function headers for Shorthand.cpp

/**
 * Draw text on the screen.
 *
 * \param str the string to be printed
 * \param x the x-coordinate of the left edge of the text
 * \param y the y-cooridinate of the top of the text. Text will print
 *          slightly below y.
 *
 * \sa drawTextCentered
 * \sa setFontSize
 * \sa drawTextWrapped
 * \sa drawTextCenteredWrapped
 * \sa drawColorText
 * \sa drawColorTextCentered
 */
void drawText(const char* str, uint16_t x, uint16_t y);

/**
 * Draw text centered horizontally on the screen.
 *
 * \param str the string to be printed
 * \param x the horizontal offset of the text. 0 means that the text will
 *          be centered.
 * \param y the y-cooridinate of the top of the text. Text will print
 *          slightly below y.
 *
 * \sa drawText
 * \sa setFontSize
 * \sa drawTextWrapped
 * \sa drawTextCenteredWrapped
 * \sa drawColorText
 * \sa drawColorTextCentered
 */
void drawTextCentered(const char* str, int16_t x, uint16_t y);

/**
 * Change the size of text drawn on the screen. Takes an index of the
 * available font sizes, with 0 being the smallest.
 *
 * \param in must be 0-7, otherwise this function does nothing.
 *
 * \sa drawText
 * \sa drawTextCentered
 * \sa drawTextWrapped
 * \sa drawTextCenteredWrapped
 * \sa drawColorText
 * \sa drawColorTextCentered
 */
void setFontSize(uint8_t in);

/**
 * Draw text wrapped, with or without new line characters, with a set max width
 * and line spacing.
 *
 * \param str the string to be printed. If it contains a '\n', it will always
 *            create a new line there. Otherwise, it will break at spaces
 *            before the max width is exceeded.
 * \param x the x-coordinate of the left edge of the text
 * \param y the y-cooridinate of the top of the text. Text will print
 *          slightly below y.
 * \param w the max width in pixels. All lines drawn will be less than w pixels.
 * \param lineSpace the amount to add to y for each subsequent line.
 *
 * \sa drawText
 * \sa drawTextCentered
 * \sa setFontSize
 * \sa drawTextCenteredWrapped
 * \sa drawColorText
 * \sa drawColorTextCentered
 */
void drawTextWrapped(const char* str, uint16_t x, uint16_t y, uint16_t w, uint16_t lineSpace);

/**
 * Draw text wrapped, with or without new line characters, with a set max width
 * and line spacing, centered horizontally on the screen.
 *
 * \param str the string to be printed. If it contains a '\n', it will always
 *            create a new line there. Otherwise, it will break at spaces
 *            before the max width is exceeded.
 * \param x the horizontal offset of the text. 0 means that the text will
 *          be centered.
 * \param y the y-cooridinate of the top of the text. Text will print
 *          slightly below y.
 * \param w the max width in pixels. All lines drawn will be less than w pixels.
 * \param lineSpace the amount to add to y for each subsequent line.
 *
 * \sa drawText
 * \sa drawTextCentered
 * \sa setFontSize
 * \sa drawTextWrapped
 * \sa drawColorText
 * \sa drawColorTextCentered
 */
void drawTextCenteredWrapped(const char* str, int16_t x, uint16_t y, uint16_t w, uint16_t lineSpace);

/**
 * Draw text on the screen with a specified color.
 *
 * \param str the string to be printed
 * \param x the x-coordinate of the left edge of the text
 * \param y the y-cooridinate of the top of the text. Text will print
 *          slightly below y.
 * \param r the red value.
 * \param g the green value.
 * \param b the blue value.
 *
 * \sa drawText
 * \sa drawTextCentered
 * \sa setFontSize
 * \sa drawTextWrapped
 * \sa drawTextCenteredWrapped
 * \sa drawColorTextCentered
 */
void drawColorText(const char* str, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

/**
 * Draw text centered horizontally on the screen with a specified color.
 *
 * \param str the string to be printed
 * \param x the horizontal offset of the text. 0 means that the text will
 *          be centered.
 * \param y the y-cooridinate of the top of the text. Text will print
 *          slightly below y.
 * \param r the red value.
 * \param g the green value.
 * \param b the blue value.
 *
 * \sa drawText
 * \sa drawTextCentered
 * \sa setFontSize
 * \sa drawTextWrapped
 * \sa drawTextCenteredWrapped
 * \sa drawColorText
 */
void drawColorTextCentered(const char* str, int16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

/**
 * Clear the screen with one solid color.
 *
 * \param r the red value.
 * \param g the green value.
 * \param b the blue value.
 */
void background(uint8_t r, uint8_t g, uint8_t b);

/**
 * Translate ASCII characters in ascii into morse code and insert into
 * morse.
 *
 * \param ascii the string of text to be translated. It should not contain
 *              any characters that cannot be translated into Morse Code.
 * \param morse the string of text to hold the Morse Code sequence, In this
 *              string, '.' means dit, '-' means dah, ',' means inter-char
 *              pause, and ' ' means space.
 */
void asciiToMorse(const char* ascii, char* morse);

/**
 * Insert a char in "text" and draw in the text box at a preset
 * location high on the screen. This function includes the frame update.
 * Consecutive ' ' chars will not be stored or drawn. Only one text box can
 * be used at a time.
 *
 * \param keyInput the char to be typed. It is cast to a uint16_t in this
 *                 so that it can be cast to a char* with 2 valid indices.
 *
 * \sa bkspcHighTextBox
 * \sa typeLowTextBox
 * \sa bkspcLowTextBox
 */
void typeHighTextBox(uint16_t keyInput);

/**
 * Delete a char in "text" and remove it from the text box at a
 * preset location high on the screen. This function includes the frame
 * update. Even when text is empty, this function can be called safely.
 * Only one text box can be used at a time.
 *
 * \sa typeHighTextBox
 * \sa typeLowTextBox
 * \sa bkspcLowTextBox
 */
void bkspcHighTextBox();

/**
 * Insert a char in "text" and draw in the text box at a preset
 * location low on the screen. This function includes the frame update.
 * Consecutive ' ' chars will not be stored or drawn. Only one text box can
 * be used at a time.
 *
 * \param keyInput the char to be typed. It is cast to a uint16_t in this
 *                 so that it can be cast to a char* with 2 valid indices.
 *
 * \sa typeHighTextBox
 * \sa bkspcHighTextBox
 * \sa bkspcLowTextBox
 */
void typeLowTextBox(uint16_t keyInput);

/**
 * Delete a char in "text" and remove it from the text box at a
 * preset location low on the screen. This function includes the frame
 * update. Even when text is empty, this function can be called safely.
 * Only one text box can be used at a time.
 *
 * \sa typeHighTextBox
 * \sa bkspcHighTextBox
 * \sa typeLowTextBox
 */
void bkspcLowTextBox();

/**
 * Updates Morse Code words per minute with units of standard Paris WPM,
 * along with recalculating other variables used for timing. wpm should not
 * be changed directly.
 *
 * \param newWPM desired WPM. Setting this value to 0 will not change
 * the timing variables.
 *
 * \sa setFWPM
 */
void setWPM(uint16_t newWPM);

/**
 * Updates Morse Code Farnsworth words per minute with units of Paris WPM
 * Farnsworth timing, along with recalculating other variables used for
 * timing. fwpm should not be changed directly.
 *
 * \param newFWPM desired Farnsworth WPM. Setting this value to 0 will not
 * change the timing variables.
 *
 * \sa setWPM
 */
void setFWPM(uint16_t newFWPM);

/**
 * Resets all statistics collected from the straight key input.
 */
void clearStats();

/**
 * Check if the straight key has changed state based on the last polled
 * event. If so, update upDuration & downDuration accordingly. This function
 * should be called after every event is polled.
 *
 * \param event the previously polled event. Must not be NULL.
 */
void checkStraightKey(SDL_Event* event);

/**
 * Draw a filled in rectangle. This function essentially replaces
 * SDL_RenderFillRect with more intuitive parameters.
 *
 * \param x the x-coordinate of the left edge.
 * \param y the y-coordinate of the top edge.
 * \param w the width in pixels.
 * \param h the height in pixels.
 * \param r the red value.
 * \param g the green value.
 * \param b the blue value.
 *
 * \sa drawRect
 */
void drawFilledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

/**
 * Draw the outline of a rectangle. This function essentially replaces
 * SDL_RenderDrawRect with more intuitive parameters. Anything previously
 * drawn inside the rectangle will not be erased.
 *
 * \param x the x-coordinate of the left edge.
 * \param y the y-coordinate of the top edge.
 * \param w the width in pixels.
 * \param h the height in pixels.
 * \param r the red value.
 * \param g the green value.
 * \param b the blue value.
 *
 * \sa drawFilledRect
 */
void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);

/**
 * Determine whether a dit or dah was tapped on the straight key based on
 * downDuration and append to "input". If input is full, this function
 * does nothing.
 */
void readDashDot();

/**
 * Convert the Morse Code sequence contained in "input" into a single ASCII
 * character. This shoud be called during user straight key input after
 * every inter-char pause. If the Morse Code sequence is unrecognized,
 * return '\0'.
 *
 * \returns the ASCII character translated from input, or '\0' if the
 *          sequence is invalid.
 */
char morseToAscii();

/**
 * Draw or redraw one of the options in case it was updated.
 *
 * \param option the index of the option to be drawn. 0 = WPM, 1 = Enable
 *               Farnsworth timing, 2 = FWPM, 3 = Beep frequency, 4/5/6 =
 *               speed presets, 7 = Straight Key key
 *
 * \sa updateOptionChoice
 */
void drawOption(uint8_t option);

/**
 * Update which option is currently selected. Also, erase the previous
 * selection box and draw the new box.
 *
 * \param newChoice the index of the new option to be selected.
 *
 * \sa drawOption
 */
void updateOptionChoice(uint8_t newChoice);

/**
 * Draw the Morse Code sequence visually as dots and dashes. The location on
 * the screen is predetermined.
 *
 * \param morseSequence the Morse Code sequence. In this string, '.' means
 *                      dit, '-' means dah, ',' means inter-char pause, and
 *                      ' ' means space. After about 720 time units,
 *                      anything drawn will appear off screen.
 */
void drawMorse(const char* morseSequence);

/**
 * Draw the chart on the bottom of the screen that shows user feedback on
 * the length of the 16 most recent straight key taps. The markings from
 * left to right indicate 0, 1 and 3 time units. This is inspired by Osu!,
 * which has a similar chart to show timing.
 */
void drawOsuChart();

/**
 * Draw a filled in circle. This function could be optimized for speed, as
 * it currently checks every pixel in a square containing the circle.
 * However, for this program, the extra time wasted is insignificant.
 *
 * \param xCenter the x-coordinate of the center.
 * \param yCenter the y-coordinate of the center.
 * \param radius the radius of the circle.
 * \param r the red value.
 * \param g the green value.
 * \param b the blue value.
 */
void drawFilledCircle(uint16_t xCenter, uint16_t yCenter, uint16_t radius, uint8_t r, uint8_t g, uint8_t b);

/**
 * Convert SDL_Keysym* to a char and check if it can be translated into
 * Morse Code. SDL_Keycode usually represents a char, but it is always
 * lowercase (even for numbers/symbols) and can contain values beyond 255,
 * so the mod attribute of SDL_Keysym must be checked and SDL_Keycode
 * cannot be cast to a char. All letters are set to upper case since it
 * makes no difference for Morse Code.
 *
 * \param key the key pressed by the user.
 *
 * \returns a char that can be converted into Morse Code.          
 */
char keysymToValidChar(SDL_Keysym* key);

/**
 * Draw the standard deviation (including contextual text) based on a set of
 * samples and their mean. The constant numSamples is used to check if
 * "samples" contains the full dataset. If totalSamples > numSamples,
 * there are more samples than that which could be contained in "samples".
 * This function is designed for showStats.
 *
 * \param samples the dataset to calculate standard deviation from. This
 *                can't contain more indices than numSamples.
 * \param totalSamples the number of samples. This can exceed the length of
 *                     "samples", which indicates that the population of
 *                     data points is greater than the number of samples.
 * \param mean the arithmetic mean used in calculating standard deviation.
 * \param y the y-coordinate to draw the text.
 *
 * \sa calcStdDev
 */
void drawStdDev(uint16_t* samples, uint16_t totalSamples, uint16_t mean, uint16_t y);

/**
 * Calculate the standard deviation based on a set of samples and their
 * mean. All values are assumed to be unsigned.
 *
 * \param samples the dataset to calculate standard deviation from.
 * \param samplesSize the length of "samples".
 * \param mean the arithmetic mean used in calculating standard deviation.
 * \param fullPop if true, samples contains the entire population.
 *
 * \returns the standard deviation.
 *
 * \sa drawStdDev
 */
uint16_t calcStdDev(uint16_t* samples, uint16_t samplesSize, uint16_t mean, bool fullPop);


//Functions headers for main.cpp


/**
 * Call a function corresponding to which screen the program is currently
 * on until the program should closes. The function is returned to when
 * the screen is changing or a screen needs to be reset completely.
 */
void loop();

/**
 * Show the start screen to introduce the user to the program.
 */
void startScreen();

/**
 * Show the main menu with every mode. The main modes include
 * "Translate Text to Morse", "Translate Morse to Text", "Game
 * Modes", "Learn Morse Code", and "Options".
 */
void mainMenuScreen();

/**
 * Run the "Translate Text to Morse" mode where the user can type
 * text and hear it translated to Morse Code.
 */
void typeAtoMScreen();

/**
 * Audibly play the Morse Code sequence contained in playback
 * with customizable timing. setWPM and setFWPM should be called
 * and fEnable should be initialized beforehand to set the timing.
 */
void playMorse();

/**
 * Ask user if WPM should be calibrated for straight key input. This
 * isn't strictly necessary, but it is a simple and intuitive way to set
 * the speed.
 */
void calibrationQueryScreen();

/**
 * Calibrate the WPM based on the speed that the user can tap a
 * specified Morse Code sequence.
 */
void calibrationScreen();

/**
 * Run the "Translate Morse to Text" mode where the user can tap
 * Morse Code and see it translated into text.
 */
void keyMtoAScreen();

/**
 * Display statistics collected from the user's tapping on the straight
 * key, i.e. averages times and standard deviation of each dit/dah/pause.
 */
void showStats();

/**
 * Show all options for the user to configure. These options include
 * WPM (with standard Paris WPM units), Enable Farnsworth timing,
 * Farnsworth WPM (with Paris WPM Farnsworth timing units), frequency
 * of sound from speaker, speed presets, and the key corresponding to
 * the straight key.
 */
void optionsScreen();

/**
 * List the quiz style game modes including "Interpreting Morse Code",
 * "Tapping Morse Code", and "Learn Q-codes & Abbreviations".
 */
void GameModeSelectScreen();

/**
 * Quiz the user on Q-codes or common abbreviations used in Morse Code.
 */
void QCodesPractice();

/**
 * Show the results screen whenever the user submits an answer in "Learn
 * Q-codes & Abbreviations".
 *
 * \param QCodeKey the correct abbreviation.
 * \param QCodeAns the answer provided by the user.
 * \param QCodePhrase the meaning of the correct Q-code/abbbreviation.
 */
void QCodeCheckAccuracy(char* QCodeKey, char* QCodeAns, char*QCodePhrase);

/**
 * Quiz the user on Morse Code by playing a Morse Code sequence that
 * must be translated by the user into text.
 */
void hearingMorsegame();

/**
 * Show the results screen whenever the user submits an answer in
 * "Interpreting Morse Code".
 *
 * \param keyMorse the Morse Code sequence that was played by the
 *                 speakers.
 * \param enteredASCII the text provided by the user.
 * \param keyASCII the text correctly translated from the Morse Code
 *                 sequence.
 */
void CheckAccuracy(char* keyMorse, char* enteredASCII, char* keyASCII);

/**
 * List the specific game types for "Tapping Morse Code".
 */
void mToAGameChoiceScreen();

/**
 * Quiz the user on Morse Code by displaying text that must be
 * translated into Morse Code by tapping on the straight key.
 */
void MorsetoASCIIGames();

/**
 * Begin the "Learn Morse Code" mode by explaining the concept of
 * Morse Code and how the subsequent tutorial works.
 */
void explainMorseScreen();

/**
 * Ask the user to set the speed of the tutorial. The speed can be
 * either slow (10 WPM), fast (16 WPM), or a custom speed based on the
 * WPM set in the options.
 */
void chooseTutorialScreen();

/**
 * Run the tutorial where the Morse Code sequence for each letter/number
 * letter/number is shown and played audibly, so that the user can
 * repeat the sequence back and learn each character in Morse Code.
 */
void tutorialScreen();

/**
 * Show the ending screen for the tutorial and suggest other modes to
 * the user.
 */
void tutorialFinishScreen();

/**
 * Pause/unpause the speaker and set the necessary flags for playMorse.
 * This functions immediately starts another timer with the length of the
 * return value, unless it returns 0. The next timer factors in how long
 * this callback takes to run, so it doesn't need to return immediately.
 *
 * \param interval the length of the timer that was just set off
 *                 (also unused).
 * \param params unused.

 * \returns the length of the next timer in ms. If 0, no new timer is
 *          set.
 */
uint32_t timerCallback(uint32_t interval, void* params);

/**
 * Buffer additional audio data as needed. This function is called
 * automatically.
 *
 * \param userdata unused.
 * \param stream pointer where audio data is stored in memory.
 * \param bytes the number of bytes that must be stored at stream
 */
void SDLCALL audioCallback(void* userdata, uint8_t* stream, int bytes);