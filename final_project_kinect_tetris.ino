#include<TFT.h>
#include<Wire.h>
#include<I2Cdev.h>
#include<MPU6050.h>
#include<SeeedTouchScreen.h>
//---------------------------------------------------------------------------------
//here define some contants used in finite state machine
#define GENERAL_WELCOME    0
#define DRAWING_MODECHOOSE 1
#define DRAWING_DRAW       2
#define MP3                3
#define TETRIS_LEVELCHOOSE 4
#define TETRIS_GAME        5
#define TETRIS_GAMEOVER    6
#define uchar unsigned char
#define uint unsigned int

#define Max7219_pinCLK    28
#define Max7219_pinCS     26
#define Max7219_pinDIN    24

#define YP A2
#define XM A1
#define YM 54
#define XP 57

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2

boolean firstTimeEnterGeneralWelcome = true;
boolean firstTimeEnterDrawingModeChoose = true;
boolean firstTimeEnterDrawingDraw = true;
boolean firstTimeEnterMp3 = true;
boolean firstTimeEnterTetrisLevelChoose = true;
boolean firstTimeEnterTetrisGame = true;
boolean firstTimeEnterTetrisGameOver = true;
int arduinoState = 0;
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//this block is for general welcome screen

//define some variables using to determine enter which game
long drawingTime;
long mp3Time;
long tetrisTime;
boolean drawingChoose = false;
boolean mp3Choose = false;
boolean tetrisChoose = false;

//generate the general welcome screen
void generalWelcome(){
  Tft.drawString("Choose the one", 30, 20, 2, WHITE); 
  Tft.drawString("you like:",65,45,2,WHITE);
  //drawing board
  Tft.drawRectangle(20,80,200,50,WHITE);
  Tft.drawString("Arduino drawing",28,85,2,WHITE);
  Tft.drawString("board",90,110,2,WHITE);
  //mp3 player
  Tft.drawRectangle(20,160,200,50,WHITE);
  Tft.drawString("Arduino MP3",55,165,2,WHITE);
  Tft.drawString("player",85,190,2,WHITE);
  //tetris
  Tft.drawRectangle(20,240,200,50,WHITE);
  Tft.drawString("Arduino tetris",38,257,2,WHITE);
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//this big block belong to arduino drawing board
//color:RED,GREEN,BLUE,BLACK,YELLOW,WHITE,CYAN,BRIGHT_RED,GRAY1,GRAY2;

static int DOT_MODE = 0;
static int LINE_MODE = 1;
int x,y;
int oldx = 0;
int oldy = 0;
int strokeWeightChooserColor = WHITE;
int defaultStrokeWeightChooserColor = WHITE;
int statusColor = RED;
int strokeColor = BLACK;
int defaultStrokeColor = BLACK;
int fingerFlag;
int strokeWeight = 5;
boolean handDectected;
long handDectectedTime;
long colorChooseTime;
long resetChooseTime;
long dotModeTime;
long lineModeTime;
boolean dotChoose = false;
boolean lineChoose = false;
boolean colorChoose = false;
boolean weightChoose = false;
boolean resetChoose = false;
long weightChooseTime;
int drawMode = 0;

//drawing board mode choose screen
void drawingModechooseScreen(){
  Tft.drawString("Choose one mode",25,20,2,WHITE); 
  Tft.drawString("to start",68,43,2,WHITE);
  Tft.drawString("The data from Kinect is seperated",20,70,1,WHITE);
  Tft.drawString("dots, choose the way you like",20,80,1,WHITE); 
  Tft.drawString("these dots to be shown.",20,90,1,WHITE);
  Tft.drawRectangle(20,110,200,60,WHITE);
  Tft.drawString("Dot Mode",75,132,2,WHITE);
  Tft.drawString("This mode will show dots as dots.",20,175,1,WHITE);
  Tft.drawRectangle(20,210,200,60,WHITE);
  Tft.drawString("Line Mode", 70, 232, 2, WHITE);
  Tft.drawString("This mode will connect dots by line.",20,275,1,WHITE);
}

//drawing board initialization
void drawingInit(){
  Tft.fillRectangle(1,40,238,240,WHITE);
  colorChooser();
  strokeWeightChooser(strokeWeightChooserColor);
  statusIndicator(statusColor);
  resetButton();
}


//draw the color chooser bar
void colorChooser(){
  Tft.drawHorizontalLine(0,280,240,BLACK);
  for(int i = 0; i < 5; i++){
    Tft.drawVerticalLine(40*(i+1),281,39,BLACK);
  }
  
  Tft.fillRectangle(2,282,37,37,BLACK); 
  Tft.drawRectangle(1,281,38,37,WHITE);
  
  Tft.fillRectangle(42,282,37,37,RED);
  Tft.drawRectangle(41,281,38,37,WHITE);
  
  Tft.fillRectangle(82,282,37,37,GREEN);
  Tft.drawRectangle(81,281,38,37,WHITE);
  
  Tft.fillRectangle(122,282,37,37,BLUE);
  Tft.drawRectangle(121,281,38,37,WHITE);
  
  Tft.fillRectangle(162,282,37,37,YELLOW);
  Tft.drawRectangle(161,281,38,37,WHITE);
  
  Tft.fillRectangle(202,282,37,37,CYAN);
  Tft.drawRectangle(201,281,37,37,WHITE);
}

//draw the stroke weight chooser bar
void strokeWeightChooser(int weightChooserColor){
  for(int i = 0; i < 4; i++){
    Tft.drawRectangle(1+40*i,1,37,37,WHITE);
    Tft.fillCircle(20+40*i,20,i+2,weightChooserColor);
  }
}

//draw the status indicator
void statusIndicator(int statusColorParameter){
  Tft.drawRectangle(161,1,37,37,WHITE);
  Tft.fillRectangle(171,11,20,20,statusColorParameter);
}

//draw the reset button
void resetButton(){
  Tft.drawRectangle(201,1,37,37,WHITE);
  Tft.drawString("Reset",205,16,1,BRIGHT_RED);
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//this block is for arduino tetris


//#define Matrix_pinCLK    8
//#define Matrix_pinCS     9
//#define Matrix_pinDIN    10



TouchScreen ts = TouchScreen(XP,YP,XM,YM);

MPU6050 accelgyro;

int touchX,touchY;
int16_t ax,ay,az;
int16_t gx,gy,gz;
float xAcc,yAcc;
int type = 0;
int Rbias = -2;
int Cbias = 2;
unsigned long t1 = 0;
unsigned long t2 = 0;
unsigned long t3 = 0;
unsigned long tLeft = 0;
unsigned long tRight = 0;
unsigned long tUp = 0;
unsigned long tDown = 0;
unsigned long tClear = 0;
unsigned long tAnimation = 0;
int count1 = 0;
int count2 = 0;
int count3 = 0;
int count4 = 0;
int countLeft = 0;
int countRight = 0;
int countUp = 0;
int countDown = 0;
boolean gameFailFlag = false;
int gameState = 1;
int fallSpeed = 0;
long oldScore[2] = {0,0};
long newScore[2] = {0,0};
int clearedLine = 0;
int work = 0;
int counter = 0;
float coefficient;
int animationFrame = 0;

uchar disp1[8][8]={

{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
};

uchar final[8][8]={
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
};

//little animation and "GAMEOVER"
uchar animation[17][8][8] = {
  {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,1,1,1,0,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,0,0,1,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,1,1,1,0,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,1,1,1,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,1,0,0,0,1,0,0},
    {0,1,1,0,1,1,0,0},
    {0,1,0,1,0,1,0,0},
    {0,1,0,1,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,1,1,1,1,1,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,1,1,1,0,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,0,1,1,1,0,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,0,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,0,1,0,1,0,0,0},
    {0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,1,1,1,1,1,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,1,1,1,0,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0},
    {0,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0},
  },
  {
    {0,1,1,1,1,0,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,0,0,0,1,0,0},
    {0,1,1,1,1,0,0,0},
    {0,1,0,1,0,0,0,0},
    {0,1,0,0,1,0,0,0},
    {0,1,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0},
  },
};

uchar pattern[19][9] = 
{
    { 1,1,0,2,1,2,2,2,1  },   //     *
    { 1,1,1,2,1,3,2,2,2  },   //    ***
    { 0,2,1,2,2,2,1,3,3  },   //
    { 1,1,0,2,1,2,1,3,0  },   
    
    { 0,0,1,0,0,1,0,2,5  },   //       *
    { 0,0,1,0,2,0,2,1,6  },   //     ***
    { 2,0,2,1,2,2,1,2,7  },   //
    { 0,0,0,1,1,1,2,1,4  },   //
    
    { 1,0,2,0,2,1,2,2,9  },   //    *
    { 2,0,0,1,1,1,2,1,10 },   //    ***
    { 0,0,0,1,0,2,1,2,11 },   
    { 0,0,1,0,2,0,0,1,8  },   
    
    { 0,1,0,2,1,2,1,3,13 },   //    **
    { 1,1,2,1,0,2,1,2,12 },   //     **
    
    { 1,0,0,1,1,1,0,2,15 },   //      **
    { 0,1,1,1,1,2,2,2,14 },   //     **
    
    { 1,0,1,1,1,2,1,3,17 },   //    ****
    { 0,1,1,1,2,1,3,1,16 },   
    
    { 0,1,1,1,0,2,1,2,18 }    //    **
                              //    **
};

uchar displayInBytes[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};



int bottomCheck(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for(i = 0; i < 4; i++)
  {
     if ((Rbias + pattern[type][2*i] >= 0) && (Rbias + pattern[type][2*i] <= 6))
     {
       if(exist[Rbias + pattern[type][2*i] + 1][Cbias + pattern[type][2*i+1]] == 1)
        {
          CheckResult = 1;
        }
     } 
  }
    return CheckResult;
}

int rightCheck(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for(i = 0; i < 4; i++)
  {
     if (Rbias + pattern[type][2*i] >= 0)
     {
       if(exist[Rbias + pattern[type][2*i]][Cbias + pattern[type][2*i+1] + 1] == 1)
         CheckResult = 1;
     } 
  }
    return CheckResult;
}

int leftCheck(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for(i = 0; i < 4; i++)
  {
     if (Rbias + pattern[type][2*i] >= 0)
     {
       if(exist[Rbias + pattern[type][2*i]][Cbias + pattern[type][2*i+1] - 1] == 1)
         CheckResult = 1;
     } 
  }
    return CheckResult;
}

int changeCheck(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for(i = 0; i < 4; i++)
  {
     if (Rbias + pattern[type][2*i] >= 0)
     {
       if(exist[Rbias + pattern[pattern[type][8]][2*i]][Cbias + pattern[pattern[type][8]][2*i+1]] == 1)
         {
           CheckResult = 1;
         }
       if(Cbias + (int)pattern[pattern[type][8]][2*i+1] >= 8 || Cbias + (int)pattern[pattern[type][8]][2*i+1] < 0)
       {
           CheckResult = 1; 
       }
     } 
  }
  return CheckResult;
}

int reachBottom(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for(i = 0; i < 4; i++)
  {
    if (Rbias + pattern[type][2*i] == 7)
    CheckResult = 1;
  }
  return CheckResult;
}

int reachRight(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for (i = 0; i < 4; i++)
    {
      if(Cbias + pattern[type][2*i + 1] == 7)
      CheckResult = 1;
    }
  return CheckResult;
}

int reachLeft(int Rbias, int Cbias, uchar type, uchar exist[8][8])
{
  int i;
  int CheckResult = 0;
  for (i = 0; i < 4; i++)
    {
      if(Cbias + pattern[type][2*i + 1] == 0)
      CheckResult = 1;
    }
  return CheckResult;
}

boolean gameFailCheck(uchar exist[8][8]){
  if((exist[0][3] || exist[0][4]) == 1)
    return true;
  else 
    return false;
}

//clear one line
long clearLine(uchar ledMatrix[8][8])
{
  work = 0;
  counter = 0;
  int i,j,k,n;
  for(i = 8; i > 0; i--)
  {
    for(j = 8; j > 0; j--)
    {
      if(ledMatrix[i-1][j-1] == 1)
      {
        counter++;
      }
    }
    if(counter == 8)
    {
      for(n = 0; n < 8; n++){
        ledMatrix[i-1][n] = 0;
      }
      for(j = i; j > 0; j--)
      {
        for(k = 0; k < 8; k++)
        {
          if(j == 1)
          {
            ledMatrix[j-1][k] = 0;
          }
          else
          {
            ledMatrix[j-1][k] = ledMatrix[j-2][k];
          }
        }
      }
      /*for(int o = 0; o < 8; o++){
        for(int l = 0; l < 8; l++){
          Serial.print(ledMatrix[o][l]);
        }
        Serial.println();
      }*/
      //Serial.print("line:");Serial.println(i);
      i++;
      work++;
    }
    counter = 0;
    //Serial.print("counter");Serial.println(counter);
  }
  //Serial.println(work);
  //newScore[0] = newScore[0] + 100*work*work;
  return work;
}

//add patterns to existed matrix
void patternAdd(int Rbias, int Cbias, uchar type, uchar exist[8][8], uchar fianl[8][8])
{
   uint i;
   uint j,k;
   for(j=0;j<8;j++)
     for(k=0;k<8;k++)
     {
       final[j][k] = exist[j][k];
     }
   for (i = 0; i < 4; i++)
   {
     if(Rbias + pattern[type][2*i] >= 0)
      {
        final[Rbias + pattern[type][2*i]][Cbias + pattern[type][2*i + 1]] = 1; 
      }
   }
}


//clear the screen
void totalClear(uchar exist[8][8])
{
  uint i,j;
  for(i=0;i<8;i++)
   for(j = 0;j<8;j++)
  exist[i][j] = 0; 
}


//convert the matrix to the form that can be used by MAX7219
void convert(uchar source[8][8], uchar target[8])
{
    uint i,j;
    for (i = 0; i < 8; i++)
    {
        for(j = 0; j < 8; j++)
       {
           target[i] = (target[i] << 1) | (source[i][j]);
       } 
    }
}


//send information to MAX7219
void Write_Max7219_byte(uchar DATA)         
{
	uchar i;    
	digitalWrite(Max7219_pinCS,LOW);
	for(i=8;i>=1;i--)
	{		  
		digitalWrite(Max7219_pinCLK,LOW);
		if(DATA&0x80)
		{
			digitalWrite(Max7219_pinDIN,HIGH);
		}
		else
		{
			digitalWrite(Max7219_pinDIN,LOW);
		}
		DATA=DATA<<1;
		digitalWrite(Max7219_pinCLK,HIGH);
	}                                 
}
void Write_Max7219(uchar address,uchar dat)
{ 
	digitalWrite(Max7219_pinCS,LOW);
	Write_Max7219_byte(address);          
	Write_Max7219_byte(dat);              
	digitalWrite(Max7219_pinCS,HIGH);                       
}

//initialization MAX7219
void Init_MAX7219(void)
{
	Write_Max7219(0x09, 0x00);      
	Write_Max7219(0x0a, 0x01);      
	Write_Max7219(0x0b, 0x07);      
	Write_Max7219(0x0c, 0x01);      
	Write_Max7219(0x0f, 0x00);      
}


//display the matrix
void TotalDisplay(uchar display[8][8])
{
       convert(display,displayInBytes);
       uchar i;
	for(i=1;i<9;i++)
	{
	    Write_Max7219(i,displayInBytes[i-1]);
	}
}

void rightShift(uchar disp1[8][8])
{
        uchar lastColumn[8] = {disp1[0][7],disp1[1][7],disp1[2][7],disp1[3][7],disp1[4][7],disp1[5][7],disp1[6][7],disp1[7][7]};
        uint i,j;
        for(i = 0; i < 8; i++)
        {
          for(j = 8; j >= 1; j--)
          {
            if(j >= 2)
              disp1[i][j-1] = disp1[i][j-2];
            else
              disp1[i][j-1] = lastColumn[i]; 
          }
        }   
}

void welcomeScreen(){
  Tft.drawString("Welcome to",62,50,2,WHITE);
  Tft.drawString("Arduino Tetris",40,75,2,WHITE);
  Tft.drawString("Please choose a",30,150,2,WHITE);
  Tft.drawString("level",93,170,2,WHITE);
  Tft.drawRectangle(70,190,100,30,WHITE);
  Tft.drawRectangle(70,230,100,30,WHITE);
  Tft.drawRectangle(70,270,100,30,WHITE);
  Tft.drawString("Easy",95,198,2,WHITE);
  Tft.drawString("Medium",84,238,2,WHITE);
  Tft.drawString("Hard",95,278,2,WHITE);
}

void runtimeScreen(){
  Tft.drawString("Current score:",40,70,2,WHITE);
  Tft.drawString("Highest score:",40,160,2,WHITE);
  Tft.drawNumber(newScore[0],40,100,2,WHITE);
  Tft.drawNumber(newScore[1],40,190,2,WHITE);
}

void gameOverScreen(){
  Tft.drawString("Game Over",67,40,2,WHITE);
  Tft.drawString("Your score:",40,90,2,WHITE);
  Tft.drawString("Highest score:",40,170,2,WHITE);
  Tft.drawNumber(newScore[0],40,120,2,WHITE);
  Tft.drawNumber(newScore[1],40,200,2,WHITE);
  Tft.drawRectangle(40,230,160,40,WHITE);
  Tft.drawString("Try again",65,243,2,WHITE);
  Tft.drawRectangle(70,280,100,30,WHITE);
  Tft.drawString("Reset",93,290,2,WHITE);
}

void gameOverAnimation(){
  if(animationFrame < 9){
    if(millis() - tAnimation >= 200){
      TotalDisplay(animation[animationFrame]);
      tAnimation = millis();
      animationFrame++;
    }
  }
  else{
    if(millis() - tAnimation >= 800){
      TotalDisplay(animation[animationFrame]);
      tAnimation = millis();
      animationFrame++;
      if(animationFrame == 17)
        animationFrame = 0;
    }
  }
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//this block is for MP3
char realName[5][25] = {"Someone like you",
                      "He's a pirate",
                      "Rolling in the deep",
                      "Just one last dance",
                      "I knew you were trouble"};                   
                        
unsigned long trackTime[5] = {4*60+47, 3*60+2, 3*60+58, 4*60+29, 3*60+39};

unsigned long currentTime, currentTimeTemp;

char artists[5][15] = {"Adele","David Garrett","Adele","Sarah Connor","Taylor Swift"};

int handX[13];
int handY[13];
char track = 0;
int handCounter;
unsigned long rightSlideTime, leftSlideTime, upSlideTime, downSlideTime;
boolean rightSlideChoose = false;
boolean leftSlideChoose = false;
boolean upSlideChoose = false;
boolean downSlideChoose = false;

void mp3Welcome(){
  Tft.drawString("Song:",20,20,1,WHITE);
  Tft.drawString(realName[0],20,45,1,WHITE);
  Tft.drawString("Artist:",20,80,1,WHITE);
  Tft.drawString(artists[0],20,105,1,WHITE);
  Tft.drawRectangle(70,280,100,30,WHITE);
  Tft.drawString("Reset",93,290,2,WHITE);
}

boolean rightSlide(int x[13], int y[13]){
  if((x[12] - x[0]) > 90 && abs(y[12] - y[0]) < 30 && x[12] != 0)
    return true;
  else
    return false;
}

boolean leftSlide(int x[13], int y[13]){
  if((x[0] - x[12]) > 90 && abs(y[0] - y[12]) < 30 && x[12] != 0)
    return true;
  else
    return false;
}

boolean upSlide(int x[13], int y[13]){
  if((y[0] - y[12]) > 90 && abs(x[12] - x[0]) < 30 && x[12] != 0)
    return true;
  else 
    return false;
}

boolean downSlide(int x[13], int y[13]){
  if((y[12] - y[0]) > 130 && abs(x[12] - x[0]) < 30 && x[12] != 0)
    return true;
  else 
    return false;
}
//---------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(38,OUTPUT);
  pinMode(40,OUTPUT);
  pinMode(42,OUTPUT);
  pinMode(44,OUTPUT);
  //Serial1.begin(115200);
  Tft.init();
  generalWelcome();
  Wire.begin();
  accelgyro.initialize();
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  randomSeed(analogRead(0));
  Init_MAX7219();
  type = analogRead(0) % 19;
}

void loop() {
  switch(arduinoState){
    case GENERAL_WELCOME:{
      if(firstTimeEnterGeneralWelcome){
        Tft.fillScreen();
        generalWelcome();
        firstTimeEnterGeneralWelcome = false;
        drawingChoose = false;
        mp3Choose = false;
        tetrisChoose = false;
        Serial.begin(115200);
      }
      if(Serial.available() > 7){
    
        //this block read the correct data from serial port
        if(Serial.read() == 'P' && Serial.read() == 'O' && Serial.read() == 'S'){
        
          //read whether hand is open and the coordination of hand
          fingerFlag = Serial.read();
          
          x = Serial.read();
          int y1 = Serial.read();
          int y2 = Serial.read();
          y = (y1 << 8) | y2;
          
          if(x > 0 && x < 240 && y > 0 && y < 320){
            Tft.fillCircle(oldx,oldy,5,BLACK);
            if(oldy > 0 && oldy < 75){
              Tft.drawString("Choose the one", 30, 20, 2, WHITE); 
              Tft.drawString("you like:",65,45,2,WHITE);
            }
            else if(oldy >= 75 && oldy < 155){
              Tft.drawRectangle(20,80,200,50,WHITE);
              Tft.drawString("Arduino drawing",28,85,2,WHITE);
              Tft.drawString("board",90,110,2,WHITE);
            }
            else if(oldy >= 155 && oldy < 235){
              Tft.drawRectangle(20,160,200,50,WHITE);
              Tft.drawString("Arduino MP3",55,165,2,WHITE);
              Tft.drawString("player",85,190,2,WHITE);
            }
            else{
              Tft.drawRectangle(20,240,200,50,WHITE);
              Tft.drawString("Arduino tetris",38,257,2,WHITE);
            }  
            Tft.fillCircle(x,y,5,CYAN);
            oldx = x;
            oldy = y;
            
            //drawing board button
            if(x > 20 && x<= 220 && y > 80 && y <= 130 && drawingChoose == false){
              drawingChoose = true;
              drawingTime = millis();
            }
            if(x > 20 && x <= 220 && y > 80 && y <= 130 && drawingChoose == true && (millis() - drawingTime) >= 4000 ){
              arduinoState = DRAWING_MODECHOOSE;
              drawingChoose = false;
              firstTimeEnterGeneralWelcome = true;
              Serial.end();
            }
            if(y < 80 || y > 130){
              drawingChoose = false;
            }
            
            //mp3 button
            if(x > 20 && x <= 220 && y > 160 && y<= 210 && mp3Choose == false){
              mp3Choose = true;
              mp3Time = millis();
            }
            if(x > 20 && x <= 220 && y > 160 && y<= 210 && mp3Choose == true && (millis() - mp3Time) >= 4000 ){
              arduinoState = MP3;
              mp3Choose = false;
              firstTimeEnterGeneralWelcome = true;
              Serial.end();
            }
            if(y < 160 || y > 210){
              mp3Choose = false;
            }
            
            //tetris button
            if(x > 20 && x <= 220 && y > 240 && y<= 290 && tetrisChoose == false){
              tetrisChoose = true;
              tetrisTime = millis();
            }
            if(x > 20 && x <= 220 && y > 240 && y<= 290 && tetrisChoose == true && (millis() - drawingTime) >= 4000 ){
              arduinoState = TETRIS_LEVELCHOOSE;
              tetrisChoose = false;
              firstTimeEnterGeneralWelcome = true;
              Serial.end();
            }  
            if(y < 240 || y > 290){
              tetrisChoose = false;
            }
          }
        }
      }
      break;
    }
    
    case DRAWING_MODECHOOSE:{
      if(firstTimeEnterDrawingModeChoose){
        Tft.fillScreen();
        drawingModechooseScreen();
        firstTimeEnterDrawingModeChoose = false;
        Serial.begin(115200);
      }
      if(Serial.available() > 7){
        if(Serial.read() == 'P' && Serial.read() == 'O' && Serial.read() == 'S'){
          fingerFlag = Serial.read();
          
          x = Serial.read();
          int y1 = Serial.read();
          int y2 = Serial.read();
          y = (y1 << 8) | y2;
          
          if(x> 0 && x < 240 && y > 0 && y < 320){
            Tft.fillCircle(oldx,oldy,5,BLACK);
            if(oldy > 0 && oldy < 65){
              Tft.drawString("Choose one mode",25,20,2,WHITE); 
              Tft.drawString("to start",68,43,2,WHITE);
            }
            else if(oldy >= 65 && oldy < 105){
              Tft.drawString("The data from Kinect is seperated",20,70,1,WHITE);
              Tft.drawString("dots, choose the way you like",20,80,1,WHITE); 
              Tft.drawString("these dots to be shown.",20,90,1,WHITE);
            }
            else if(oldy >= 105 && oldy < 205){
              Tft.drawRectangle(20,110,200,60,WHITE);
              Tft.drawString("Dot Mode",75,132,2,WHITE);
              Tft.drawString("This mode will show dots as dots.",20,175,1,WHITE);
            }
            else{
              Tft.drawRectangle(20,210,200,60,WHITE);
              Tft.drawString("Line Mode", 70, 232, 2, WHITE);
              Tft.drawString("This mode will connect dots by line.",20,275,1,WHITE);
            }  
            Tft.fillCircle(x,y,5,CYAN);
            oldx = x;
            oldy = y;
            
            if(x > 20 && x <= 220 && y > 110 && y <= 170 && dotChoose == false){
              dotChoose = true;
              dotModeTime = millis();
            }
            if(x > 20 && x <= 220 && y > 110 && y <= 170 && dotChoose == true && (millis() - dotModeTime) > 4000){
              dotChoose = false;
              drawMode = 0;
              arduinoState = DRAWING_DRAW;
              firstTimeEnterDrawingModeChoose = true;
              Serial.end();
            }
            if(y <= 110 || y > 170){
              dotChoose = false;
            }
            
            if(x > 20 && x <= 220 && y > 210 && y <= 270 && lineChoose == false){
              lineChoose = true;
              lineModeTime = millis();
            }
            if(x > 20 && x <= 220 && y > 210 && y <= 270 && lineChoose == true && (millis() - lineModeTime) > 4000){
              lineChoose = false;
              drawMode = 1;
              arduinoState = DRAWING_DRAW;
              firstTimeEnterDrawingModeChoose = true;
              Serial.end();
            }
            if(y <= 210 || y > 270){
              lineChoose = false;
            }
          }
        }
      }
      break;
    }
    
    case DRAWING_DRAW:{
      if(firstTimeEnterDrawingDraw){
        Tft.fillScreen();
        drawingInit();
        firstTimeEnterDrawingDraw = false;
        Serial.begin(115200);
      }
      if(Serial.available() > 7){
    
        //this block read the correct data from serial port
        if(Serial.read() == 'P' && Serial.read() == 'O' && Serial.read() == 'S'){
          
          //read whether hand is open and the coordination of hand
          fingerFlag = Serial.read();
          
          x = Serial.read();
          int y1 = Serial.read();
          int y2 = Serial.read();
          y = (y1 << 8) | y2;
          //end
          
          //this block change the status indicator's color 
          //according to whether hand and finger are dectected
          handDectected = true;
          handDectectedTime = millis();
          if(fingerFlag == 1 && handDectected == true){
            statusColor = GREEN;
            statusIndicator(statusColor);  
          }
          else if(fingerFlag == 0 && handDectected == true){
            statusColor = YELLOW;
            statusIndicator(statusColor);
          }
          //end
          
          //if hand is open, keep the last point;
          //if hand is close, discard the last point.
          
          
          if(x > 0 && x < 240 && y > 0 && y < 320){ // this line make sure the data is within the valid
                                                    // range otherwise may cause screen overflow
            //if the last position of the mouse if inside the color chooser bar,
            //re-render the color choose bar 
            //warning: this may cause mouse move become not smooth
            if(oldy >= 280){
              colorChooser();
            }
            
            //if the last position of the mouse if inside the stroke weight chooser bar,
            //re-render the stroke weight chooser bar 
            //warning: this may cause mouse move become not smooth
            if(oldy <= 40){
              strokeWeightChooser(strokeWeightChooserColor);
            }
            
            //if the last position of the mouse is inside the white board,
            //and fingers are not dectected, erase the last drawing
            if(fingerFlag == false && oldy > 40 && oldy <= 280)
              Tft.drawCircle(oldx,oldy,strokeWeight,WHITE);
            
            //if finger dectected, draw filled circle, if not, draw circle
            if(drawMode == 0){ //dot mode
              if(fingerFlag == false)
                Tft.drawCircle(x,y,strokeWeight,strokeColor);
              else if(fingerFlag == true){
                Tft.fillCircle(x,y,strokeWeight,strokeColor);
              }
            }
            else if(drawMode == 1){  // line mode
              if(fingerFlag == false)
                Tft.drawCircle(x,y,strokeWeight,strokeColor);
              else if(fingerFlag == true){
                if(oldx == 0 && oldy == 0)
                  Tft.fillCircle(x,y,strokeWeight,strokeColor);
                else{
                  if(x >= oldx){
                    for(int k = oldx; k <= x; k++){
                      int tempy = int(double(y-oldy))/(x-oldx)*(k-oldx) + oldy;
                      Tft.fillCircle(k,tempy,strokeWeight,strokeColor);
                    }
                  }
                  if(x < oldx){
                    for(int k = x; k <= oldx; k++){
                      int tempy = int(double(y-oldy))/(x-oldx)*(k-oldx) + oldy;
                      Tft.fillCircle(k,tempy,strokeWeight,strokeColor);
                    }
                  }
                }
              }
            }
            //update the last position
            oldx = x;
            oldy = y; 
            
            //this block make the corresponding color being choosen after 1s when mouse enter the area
            //and also change the stroke weight chooser's color
            if(y > 280 && colorChoose == false){
              colorChooseTime = millis();
              colorChoose = true;
            }
            if(y > 280 && colorChoose == true && (millis() - colorChooseTime) > 1000){
              if(x < 40){
                strokeColor = BLACK;
                strokeWeightChooserColor = WHITE;
              }
              else if(x < 80){
                strokeColor = RED;
                strokeWeightChooserColor = RED;
              }
              else if(x < 120){
                strokeColor = GREEN;
                strokeWeightChooserColor = GREEN;
              }
              else if(x < 160){
                strokeColor = BLUE;
                strokeWeightChooserColor = BLUE;
              }
              else if(x < 200){
                strokeColor = YELLOW;
                strokeWeightChooserColor = YELLOW;
              }
              else{
                strokeColor = CYAN;
                strokeWeightChooserColor = CYAN;
              }
              strokeWeightChooser(strokeWeightChooserColor);
              colorChoose = false;
            }
            //end
           
           //this block make the corresponding stroke weight being choosen after 1s when mouse enter
           //the area
           if(y < 40 && x < 200 && weightChoose == false){
             weightChooseTime = millis();
             weightChoose = true;
           }
           if(y < 40 && weightChoose == true && (millis() - weightChooseTime) > 1000){
             if(x < 40){
               strokeWeight = 2;
             }
             else if(x < 80){
               strokeWeight = 3;
             }
             else if(x < 120){
               strokeWeight = 4;
             }
             else if(x < 160){
               strokeWeight = 5;
             }
             weightChoose = false;
           }
           //end
           
           //this block reset the whole screen to its initial condition after 3s when mouse enter 
           //the reset area
           //warning: this process takes much longer time, the indicator will blink about 10 times
           //         before you can start drawing again.
           if(y < 40 && x > 200 && resetChoose == false){
             resetChooseTime = millis();
             resetChoose = true;
           }
           if(y < 40 && x > 200 && resetChoose == true && (millis() - resetChooseTime) > 3000){
//             drawingInit();
             arduinoState = GENERAL_WELCOME;
             firstTimeEnterDrawingDraw = true;
             Serial.end();
           }
           if(x <= 200 || y >= 40){
             resetChoose = false;
           }
           //end
          }
        }
      }
      if((millis() - handDectectedTime) > 1000){
        statusIndicator(RED);
        colorChooser();
      }
      break;
    }
    
    case TETRIS_LEVELCHOOSE:{
      if(firstTimeEnterTetrisLevelChoose){
        Tft.fillScreen();
        welcomeScreen();
        firstTimeEnterTetrisLevelChoose = false;
        Serial.begin(115200);
      }
      totalClear(final);
      totalClear(disp1);
      TotalDisplay(final);
      welcomeScreen();
      Point p = ts.getPoint();
      touchX = map(p.x, TS_MINX, TS_MAXX, 0, 240);
      touchY = map(p.y, TS_MINY, TS_MAXY, 0, 320);
      if (touchX >= 70 && touchX <= 170 && touchY >= 190 && touchY <= 220){
        Tft.fillRectangle(70,190,100,30,GRAY1);
        Tft.drawString("Easy",95,198,2,WHITE);
        fallSpeed = 2000;
        totalClear(disp1);
        arduinoState = TETRIS_GAME;
        firstTimeEnterTetrisLevelChoose = true;
        coefficient = 1;
      }
      else if(touchX >= 70 && touchX <= 170 && touchY >= 230 && touchY <= 260){
        Tft.fillRectangle(70,230,100,30,GRAY1);
        Tft.drawString("Medium",84,238,2,WHITE);
        fallSpeed = 1500;
        totalClear(disp1);
        for (int i = 0; i < 8; i++){
          disp1[7][i] = random(299) % 2;
        }
        arduinoState = TETRIS_GAME;
        firstTimeEnterTetrisLevelChoose = true;
        coefficient = 1.2;
      }
      else if(touchX >= 70 && touchX <= 170 && touchY >= 270 && touchY <= 300){
        Tft.fillRectangle(70,270,100,30,GRAY1);
        Tft.drawString("Hard",95,278,2,WHITE);
        fallSpeed = 1000;
        totalClear(disp1);
        for (int j = 7; j > 5; j--){
          for (int i = 0; i < 8; i++){
            disp1[j][i] = random(299) % 2;
          }
        }
        arduinoState = TETRIS_GAME;
        firstTimeEnterTetrisLevelChoose = true;
        coefficient = 1.4;
      }
      break;
    }
    
    case TETRIS_GAME:{
      if(firstTimeEnterTetrisGame){
        Tft.fillScreen();
        runtimeScreen();
        handCounter = 0;
        firstTimeEnterTetrisGame = false;
        Serial.begin(115200);
      }
      
      
      if(Serial.available() > 7){
       if(Serial.read() == 'P' && Serial.read() == 'O' && Serial.read() == 'S'){
          
          //read whether hand is open and the coordination of hand
          fingerFlag = Serial.read();
          
          x = Serial.read();
          int y1 = Serial.read();
          int y2 = Serial.read();
          y = (y1 << 8) | y2;
          
          if(handCounter < 13){
            handX[handCounter] = x;
            handY[handCounter] = y;
            handCounter++;
          }
          else if(handCounter >= 13){
            for(int j = 0; j < 12; j++){
              handX[j] = handX[j+1];
              handY[j] = handY[j+1];
            }
            handX[12] = x;
            handY[12] = y;
          }  
       }
      }
      
      boolean left = leftSlide(handX,handY);
      boolean right = rightSlide(handX,handY);
      boolean up = upSlide(handX,handY);
      boolean down = downSlide(handX,handY);
      
      if(handCounter == 13){
        for(int e = 0; e < 20; e++){
          handX[e] = 0;
          handY[e] = 0;
        }
        handCounter = 0;
      }
      
      accelgyro.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
      xAcc = ax/16384.0;
      yAcc = ay/16384.0;
      if(countLeft == 0){
        tLeft = millis();
        countLeft = 1;
      }
      else if((xAcc >= 0.55 || left) && leftCheck(Rbias,Cbias,type,disp1) == 0 && reachLeft(Rbias,Cbias,type,disp1) == 0 && countLeft == 1 && (millis() - tLeft) >= 200){
        Cbias--;
        countLeft = 0;
      }
      
      if(countRight == 0){
        tRight = millis();
        countRight = 1;
      }
      else if((xAcc <= -0.55 || right) && rightCheck(Rbias,Cbias,type,disp1) == 0 && reachRight(Rbias,Cbias,type,disp1) == 0 && countRight == 1 && (millis() - tRight) >= 200){
        countRight = 0;
        Cbias++;
      }
      
      if(countUp == 0){
        tUp = millis();
        countUp = 1;
      }
      else if((yAcc >= 0.55 || down) && bottomCheck(Rbias,Cbias,type,disp1) == 0 && reachBottom(Rbias,Cbias,type,disp1) == 0 && countUp == 1 && (millis() - tUp) >= 200){
        Rbias++;
        countUp = 0;
      }
      
      if(countDown == 0){
        tDown = millis();
        countDown = 1;
      }
      else if((yAcc <= -0.55 || up) && changeCheck(Rbias,Cbias,type,disp1) == 0 && countDown == 1 && (millis() - tDown) >= 200){
        countDown = 0;
        type = pattern[type][8];
      }
      
      /*switch (Serial.read())
      {
        case 0x61:   //A left
        {
        if(leftCheck(Rbias,Cbias,type,disp1) == 0)
        {
           if(reachLeft(Rbias,Cbias,type,disp1) == 0)  
          {
            Cbias--;
          }
        }
        break;
        }
        case 0x73:    //S  down
        {
        if(bottomCheck(Rbias,Cbias,type,disp1) == 0)
         {
           if(reachBottom(Rbias,Cbias,type,disp1) == 0)
           {
             Rbias++;
           }
         }
        break;
        }
        case 0x64:    //D  right
        {
        if(rightCheck(Rbias,Cbias,type,disp1) == 0)
           { 
             if(reachRight(Rbias,Cbias,type,disp1) == 0)
             {
               Cbias++;
             }
           }
        break;
        }
        case 0x77:     //W  up
        {
        if(changeCheck(Rbias,Cbias,type,disp1) == 0)
            type = pattern[type][8];
        break;
        }
      }*/
      
      
      if(count3 == 0)
      {
        t3 = millis();
        count3 = 1;
      }
      if((count3 == 1) && ((millis() - t3) >= fallSpeed))
      {
        count3 = 0;
        if((bottomCheck(Rbias,Cbias,type,disp1) == 0) && (reachBottom(Rbias,Cbias,type,disp1) == 0))
        Rbias++;
      }
      
      
      
      patternAdd(Rbias, Cbias, type, disp1, final);
      TotalDisplay(final);         
       
      //check if the object reach other objects or reach the bottom
      if((bottomCheck(Rbias,Cbias,type,disp1) == 1)&&(count1 == 0))
      {
        t1 = millis();
        count1 = 1;
      }
      if((bottomCheck(Rbias,Cbias,type,disp1) == 1)&&(count1 == 1)&&((millis() - t1) >= 1000))
      {
          clearedLine = clearLine(final);
          if(clearedLine >= 1)
          {
              newScore[0] = newScore[0] + (long)100*clearedLine*clearedLine*coefficient;
              delay(250);
              clearedLine = 0;
          }
         for(int i = 0; i < 8; i++)
           for(int j = 0; j < 8; j++)
           {
             disp1[i][j] = final[i][j];
           }
         if(newScore[0] > oldScore[0]){
           Tft.fillRectangle(40,100,150,30,BLACK);
           Tft.drawNumber(newScore[0],40,100,2,WHITE);
           oldScore[0] = newScore[0];
         }
         if(newScore[0] > newScore[1]){
           newScore[1] = newScore[0];
           Tft.fillRectangle(40,190,150,30,BLACK);
           Tft.drawNumber(newScore[1],40,190,2,WHITE);
           oldScore[1] = newScore[1];
         }
         gameFailFlag = gameFailCheck(disp1);
         count1 = 0;
         Rbias = -2;
         Cbias = 2;
         type = millis() % 19;
      }
      
      if((reachBottom(Rbias,Cbias,type,disp1) == 1) && (count2 == 0))
      {
        t2 = millis();
        count2 = 1;
      }
      if((reachBottom(Rbias,Cbias,type,disp1) == 1)&&(count2 == 1)&&((millis() - t2) >= 1000))
      {
        clearedLine = clearLine(final);
        if(clearedLine >= 1)
        {
            newScore[0] = newScore[0] + (long)100*clearedLine*clearedLine*coefficient;
            delay(250);
            clearedLine = 0;
        }
        for(int i = 0; i < 8; i++)
           for(int j = 0; j < 8; j++)
           {
             disp1[i][j] = final[i][j];
           }
         if(newScore[0] > oldScore[0]){
           Tft.fillRectangle(40,100,150,30,BLACK);
           Tft.drawNumber(newScore[0],40,100,2,WHITE);
           oldScore[0] = newScore[0];
         }
         if(newScore[0] > newScore[1]){
           newScore[1] = newScore[0];
           Tft.fillRectangle(40,190,150,30,BLACK);
           Tft.drawNumber(newScore[1],40,190,2,WHITE);
           oldScore[1] = newScore[1];
         }
         gameFailFlag = gameFailCheck(disp1);
         count2 = 0;
         Rbias = -2;
         Cbias = 2;
         type = millis() % 19;     
      }
      
      if(gameFailFlag){
        arduinoState = TETRIS_GAMEOVER;
        firstTimeEnterTetrisGame = true;
        Serial.end();
      }
      break;
    }
    
    case TETRIS_GAMEOVER:{
      if(firstTimeEnterTetrisGameOver){
        Tft.fillScreen();
        gameOverScreen();
        firstTimeEnterTetrisGameOver = false;
      }
      gameOverAnimation();
      Point p = ts.getPoint();
      touchX = map(p.x, TS_MINX, TS_MAXX, 0, 240);
      touchY = map(p.y, TS_MINY, TS_MAXY, 0, 320);
      if(touchX >= 40 && touchX <= 200 && touchY >= 230 && touchY <= 270){
        Tft.fillRectangle(40,230,160,40,GRAY1);
        Tft.drawString("Try again",65,243,2,WHITE);
        arduinoState = TETRIS_LEVELCHOOSE;
        firstTimeEnterTetrisGameOver = true;
        totalClear(disp1);
        totalClear(final);
        for(int o = 0; o < 8; o++){
          for(int l = 0; l < 8; l++){
            Serial.print(disp1[o][l]);
          }
          Serial.println();
        }
        Serial.println();
        gameFailFlag = false;
        animationFrame = 0;
        Tft.fillScreen();
        newScore[0] = 0;
        oldScore[0] = 0;
      }
      if(touchX >= 70 && touchX <= 170 && touchY >= 280 && touchY <= 310){
        Tft.fillRectangle(70,280,100,30,GRAY1);
        Tft.drawString("Reset",93,290,2,WHITE);
        arduinoState = GENERAL_WELCOME;
        Serial.end();
        firstTimeEnterTetrisGameOver = true;
        totalClear(disp1);
        totalClear(final);
        TotalDisplay(final);
        gameFailFlag = false;
        animationFrame = 0;
        Tft.fillScreen();
        newScore[0] = 0;
        oldScore[0] = 0;
      }
      break;
    }
   
   case MP3:{
     if(firstTimeEnterMp3){
       Tft.fillScreen();
       firstTimeEnterMp3 = false;
       mp3Welcome();
       handCounter = 0;
       track = 1;
       Serial.begin(115200);
       Serial1.begin(115200);
       Serial1.write('s');
       Serial1.write(49);
     }
     if(Serial.available() > 7){
       if(Serial.read() == 'P' && Serial.read() == 'O' && Serial.read() == 'S'){
          
          //read whether hand is open and the coordination of hand
          fingerFlag = Serial.read();
          
          x = Serial.read();
          int y1 = Serial.read();
          int y2 = Serial.read();
          y = (y1 << 8) | y2;
          
          if(handCounter < 20){
            handX[handCounter] = x;
            handY[handCounter] = y;
            handCounter++;
          }
          else if(handCounter >= 20){
            for(int j = 0; j < 19; j++){
              handX[j] = handX[j+1];
              handY[j] = handY[j+1];
            }
            handX[19] = x;
            handY[19] = y;
          }
          
          boolean left = leftSlide(handX,handY);
          boolean right = rightSlide(handX,handY);
          boolean up = upSlide(handX,handY);
          boolean down = downSlide(handX,handY);
          
          if(left == true && leftSlideChoose == false){
            leftSlideTime = millis();
            leftSlideChoose = true;
          }
          if(left == true && (millis() - leftSlideTime) < 10 && leftSlideChoose == true){
            Serial1.write('s');
            if((track + 1) > 5)
              track = 5;
            else
              track = track + 1;
            Serial1.write(track+48);
            Tft.fillRectangle(20,45,200,20,BLACK);
            Tft.drawString(realName[track-1],20,45,1,WHITE);
            Tft.fillRectangle(20,105,200,20,BLACK);
            Tft.drawString(artists[track-1],20,105,1,WHITE);
            digitalWrite(38,HIGH);
          }
          if(left == false)
             leftSlideChoose = false;
             
          if(right == true && rightSlideChoose == false){
            rightSlideTime = millis();
            rightSlideChoose = true;
          }
          if(right == true && (millis() - rightSlideTime) < 10 && rightSlideChoose == true){
            Serial1.write('s');
            if((track - 1) < 1)
              track = 1;
            else
              track = track - 1;
            Serial1.write(track+48);
            Tft.fillRectangle(20,45,200,20,BLACK);
            Tft.drawString(realName[track-1],20,45,1,WHITE);
            Tft.fillRectangle(20,105,200,20,BLACK);
            Tft.drawString(artists[track-1],20,105,1,WHITE);
            digitalWrite(40,HIGH);
          } 
          if(right == true)
            rightSlideChoose = false;
          
          if(up == true && upSlideChoose == false){
            upSlideTime = millis();
            upSlideChoose = true;
          }
          if(up == true && (millis() - upSlideTime) < 2 && upSlideChoose == true){
            Serial1.write('+');
            digitalWrite(42,HIGH);
          }
          if(up == true)
            upSlideChoose = false;
          
          if(down == true && downSlideChoose == false){
            downSlideTime = millis();
            downSlideChoose = true;
          }
          if(down == true && (millis() - downSlideTime) < 2 && downSlideChoose == true){
            Serial1.write('-');
            digitalWrite(44,HIGH);
          }
          if(down == true)
            downSlideChoose = false;
       }
     }
     Point p = ts.getPoint();
     touchX = map(p.x, TS_MINX, TS_MAXX, 0, 240);
     touchY = map(p.y, TS_MINY, TS_MAXY, 0, 320);
     if(touchX >= 70 && touchX <= 170 && touchY >= 280 && touchY <= 310){
       Tft.fillRectangle(70,280,100,30,GRAY1);
        Tft.drawString("Reset",93,290,2,WHITE);
        arduinoState = GENERAL_WELCOME;
        Serial1.write('s');
        Serial.end();
        Serial1.end();
        firstTimeEnterMp3 = true;
        
        Tft.fillScreen();
     }
     break;
   } 
  }
}
//---------------------------------------------------------------------------------------
