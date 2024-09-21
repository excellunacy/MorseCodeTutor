//Using SDL2 and standard IO
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

//Game Modes for Practicing Morse Tapping
#define MORSE_CHARACTERS 1
#define MORSE_WORDS 2
#define MORSE_PHRASES 3

#define strLength 200 //Has a significant impact on RAM
#define input_length    18
#define numOptions      7
#define numWords        30
#define numPhrases      15
#define excluded_length 10
#define textBoxSize		200
#define fast            true
#define slow            false
#define DOWN            true
#define UP              false

//GUI Common Values
#define lineSpace1    20
#define lineSpace2    25
#define escX		  10
#define escY		  20

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const char charDelimiter[2] = ",";
const char myMorse[64][8] = { " ", "-.-.--", ".-..-.", "x", "...-..-", "X", ".-...", ".----.", "-.--.", "-.--.-", "X", ".-.-.", "--..--", "-....-", ".-.-.-", "-..-.", "-----", ".----",
						 "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "---...", "-.-.-.", "X", "-...-", "X", "..--..", ".--.-.", ".-", "-...", "-.-.", "-..", ".", "..-.",
						 "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "X", "X", "X", "X", "..--.-"
};   //morse ascii table
//Array of words for both hearing and tapping games
const char words[numWords][15] = {
  "HELLO",
  "HI",
  "WORLD",
  "START",
  "END",
  "MESSAGE",
  "PRACTICE",
  "TODAY",
  "MORNING",
  "AFTERNOON",
  "EVENING",
  "GOOD",
  "GOODBYE",
  "RAIDER",
  "POWER",
  "SCARLET",
  "ROGERS",
  "IEEE",
  "MCARTHUR",
  "MORSE",
  "EVAN",
  "JEFF",
  "KATHERINE",
  "XYLOPHONE",
  "ZOMBIE",
  "THE",
  "IMMACULATE",
  "HALLELUJAH",
  "EASY",
  "QUESADILLA"
};
// Array of phrases/sentences (Must be 52 characters or fewer and should be aligned to 26 chars per line)
const char phrases[numPhrases][30] = {
	"HELLO WORLD",
	"GOOD MORNING",
	"HAVE A NICE DAY",
	"TEXAS TECH IS AWESOME",
	"START MESSAGE",
	"END MESSAGE",
	"WILL ROGERS AND SOAPSUDS",
	"GET YOUR GUNS UP",
	"HAPPY BIRTHDAY",
	"RING THE VICTORY BELLS",
	"YES WE HAVE NO BANANAS",
	"I AM LEARNING MORSE CODE",
	"RAIDER POWER",
	"GO TO CLASS",
	"PLEASE GIVE US A GOOD GRADE"
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
  "Can you acknowledge receipt?"  //QSL
};
const char qCodeCodes[16][4] = {  "QRX", "QRH", "QTR", "QRL", "QRM", "QRT", "QRO", "QRP", "QRQ", "QRS", "QRU", "QRV", "QRZ", "QSB", "QSD", "QSL"};
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
const char PopAbbKey[16][6] = {  "AGN", "BK", "BTR", "C", "CFM", "CS", "CX", "EMRG", "FER", "FM", "FREQ", "GA", "GUD", "II", "KN", "N"};
extern uint16_t lastRandomIndex;

const uint8_t fontSizes[7] = { 12, 18, 24, 36, 48, 60, 72 };
const uint8_t fontPixels[7] = { 7, 11, 14, 22, 29, 36, 43 };

//Global variables
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Color Pen;
extern uint8_t textSize;
extern TTF_Font* myFont;
extern char tempStr[strLength];
extern char text[strLength];
extern uint16_t text_i;
extern uint32_t msPerDit;
extern uint32_t msPerFDit;
extern uint16_t wpm;
extern uint16_t fwpm;
extern bool fEnable; //Determines whether to use Farnsworth timing
extern uint32_t tolerance;
//Formula for std dev = sqrt(sum(differenceFromMean^2)/samples)
//std dev = sqrt(variance)
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
extern bool error[8];
extern uint8_t error_i;
extern bool morseDelete;


//Function headers from Shorthand.cpp
void drawText(const char* str, int x, int y);
void drawTextCentered(const char* str, int x, int y);
void setFontSize(uint8_t in);
void drawTextWrapped(const char* str, int x, int y, int w, int lineSpace);
void drawTextCenteredWrapped(const char* str, int x, int y, int w, int lineSpace);
void background(uint8_t r, uint8_t g, uint8_t b);
void asciiToMorse(char* ascii, char* morse);
void typeTextbox(int keyInput);
void bkspcTextbox();
bool isValidChar(char c);
void setWPM(int newWPM);
void setFWPM(int newFWPM);
void clearStats();
void pollStraightKey(SDL_Event* event);
void drawFilledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b);
void readDashDot();
char morseToAscii();

//Main.cpp
void loop();
void startScreen();
void mainMenuScreen();
void typeAtoMScreen();
void playMorse();
void calibrationQueryScreen();
void calibrationScreen();
void keyMtoAScreen();
void showStats();
uint32_t timerCallback(uint32_t interval, void* params);
void SDLCALL audioCallback(void* userdata, uint8_t* stream, int len);