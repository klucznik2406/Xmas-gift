
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DISPLAY_PIN 13

#define TIME_SDA 6
#define TIME_CLK 4
#define TIME_CE A2

#define SET_PIN A0
#define MENU_PIN 2

#define DALLAS_PIN 9

#define PHOTO_CONTROL_PIN 7
#define PHOTO_ANALOG_PIN A3

#define CHRISTMAS_IN_PIN 11
#define CHRISTMAS_OUT_PIN 12

#define TIME_REFRESH_MILIS 5000
#define TEMP_REFRESH_MILIS 5000

#define TEMP_CALIBRATION 2.0



void writeData(uint8_t data);
uint8_t readData(void);
void ReadFromTimer(uint8_t * data, uint8_t starting_address);
void WriteToTimer(uint8_t data, uint8_t starting_address);
void drawDigt(uint8_t digt, uint8_t x, uint8_t y, uint16_t color);
void SetTimeDate(void);
void displayTime(void);
void displayTemperature(void);
uint16_t randomizeSolidifiedColor(void);
void adjustScreenBrightness(void);

void displayChristmasTree(void);
void displayChristmasTextPolish(void);
void displayChristmasTextEnglish(void);
void displayRawChristmasTree(void);
void displayRawChristmasTreeWithEffects(void);
void drawDeer(void);
void drawPresent(void);

void callRandomChristmasFunc(void);

const void (*xmas_display_functions[]) (void) = 
{
    &displayChristmasTree,
    &displayChristmasTextPolish,
    &displayChristmasTextEnglish,
    &displayRawChristmasTree,
    &displayRawChristmasTreeWithEffects,
    &drawDeer,
    &drawPresent
};

#define NUM_XMAS_FUN (sizeof(xmas_display_functions) / sizeof(xmas_display_functions[0]))

const void (*working_display[]) (void) = 
{
    &displayTime,
    &displayTemperature,
    &callRandomChristmasFunc,
    &adjustScreenBrightness
};

#define NUM_WORK_FUN (sizeof(working_display) / sizeof(working_display[0]))


static bool timeSetSignaled = false;
static char time_string[10];
static char temp_string[10];
static long time_refresh_timer=0;
static long temp_refresh_timer=0;

void modeInterrupt(void)
{
    detachInterrupt(digitalPinToInterrupt(MENU_PIN));
    timeSetSignaled = true;
}

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 9, DISPLAY_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);


OneWire oneWire(DALLAS_PIN);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature temp_sensor(&oneWire);

const uint16_t baseColors[] = 
{
    matrix.Color(255,0,0),
    matrix.Color(255,0,128),
    matrix.Color(255,0,255),
    matrix.Color(128,0,255),
    matrix.Color(0,0,255),
    matrix.Color(0,128,255),
    matrix.Color(0,255,255),
    matrix.Color(0,255,128),
    matrix.Color(0,255,0),
    matrix.Color(128,255,0),
    matrix.Color(255,255,0),
    matrix.Color(255,128,0)
};

void setup()
{

    pinMode(TIME_SDA, INPUT);
    pinMode(TIME_CLK, OUTPUT);
    pinMode(TIME_CE, OUTPUT);
    pinMode(SET_PIN, INPUT);
    pinMode(MENU_PIN, INPUT);
    pinMode(PHOTO_CONTROL_PIN, OUTPUT);
    pinMode(CHRISTMAS_IN_PIN, INPUT);
    pinMode(CHRISTMAS_OUT_PIN, OUTPUT);
    
    attachInterrupt(digitalPinToInterrupt(MENU_PIN), &modeInterrupt, LOW);

    digitalWrite(CHRISTMAS_OUT_PIN, HIGH);
    digitalWrite(PHOTO_CONTROL_PIN, LOW);
    digitalWrite(TIME_SDA, LOW);
    digitalWrite(TIME_CLK, LOW);
    digitalWrite(TIME_CE, LOW);
    

    Serial.begin(9600);
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(20);
    matrix.setTextColor(baseColors[0]);
    temp_sensor.begin();
    temp_sensor.setResolution(11);

    randomSeed(analogRead(PHOTO_ANALOG_PIN));

    // Enabling Timer register editing
    WriteToTimer(0, 0x8E);
}

void loop()
{
    for(uint8_t idx = 0; idx < NUM_WORK_FUN; idx++)
    {
        if (timeSetSignaled)
        {
            matrix.fillScreen(0);
            delay(2000);
            timeSetSignaled = false;
            SetTimeDate();
            delay(2000);
            attachInterrupt(digitalPinToInterrupt(MENU_PIN), &modeInterrupt, LOW);
            idx = 0;
        }
        working_display[idx]();

    }
}

void callRandomChristmasFunc(void)
{
    digitalWrite(CHRISTMAS_OUT_PIN, HIGH);
    if (digitalRead(CHRISTMAS_IN_PIN) == HIGH)
    {
        digitalWrite(CHRISTMAS_OUT_PIN, LOW);
        if(digitalRead(CHRISTMAS_IN_PIN) == LOW)
        {
            xmas_display_functions[random(NUM_XMAS_FUN)]();
        }
    }
}

void adjustScreenBrightness(void)
{
    uint16_t analogValue;
    uint8_t screenBrightnes;
    digitalWrite(PHOTO_CONTROL_PIN, LOW);
    // clearing display to do not affect reading
    matrix.fillScreen(0);
    matrix.show();
    
    analogValue = analogRead(PHOTO_ANALOG_PIN);

    if (analogValue > 330)
    {
        screenBrightnes = 40;
    }
    else if (analogValue >250)
    {
        screenBrightnes = 35;
    }
    else if (analogValue > 180)
    {
        screenBrightnes = 30;
    }
    else if (analogValue > 120)
    {
        screenBrightnes = 25;
    }
    else if (analogValue > 70)
    {
        screenBrightnes = 20;
    }
    else
    {
        screenBrightnes = 15;
    }
    matrix.setBrightness(screenBrightnes);

    digitalWrite(PHOTO_CONTROL_PIN, HIGH);
    Serial.print("Setting screen brightness to: ");
    Serial.print(screenBrightnes);
    Serial.print(" Analog Val: ");
    Serial.println(analogValue);
}

void writeData(uint8_t data)
{
    pinMode(TIME_SDA, OUTPUT);
    for(uint8_t i = 0; i < 8 ; i++)
    {
        
        digitalWrite(TIME_SDA, data & 1);
        digitalWrite(TIME_CLK, 1);
        delayMicroseconds(1);
        if(i==7)
        {
            break;
        }
        digitalWrite(TIME_CLK, 0);
        delayMicroseconds(1);
        data = data >> 1;
    }
    pinMode(TIME_SDA, INPUT);
    digitalWrite(TIME_CLK, 0);
    delayMicroseconds(1);
}

uint8_t readData(void)
{
    uint8_t data = 0;
    bool read_val=0;
    for(uint8_t i = 0; i < 8 ; i++)
    {
        read_val = digitalRead(TIME_SDA);
        data = data + (read_val << i);
        digitalWrite(TIME_CLK, 1);
        delayMicroseconds(1);
        digitalWrite(TIME_CLK, 0);
        delayMicroseconds(1);
    }
    return data;
}

void ReadFromTimer(uint8_t * data, uint8_t starting_address)
{
    delay(1);
    digitalWrite(TIME_CE, 1);
    delayMicroseconds(2);
    writeData(starting_address);
    *data = readData();
    delayMicroseconds(2);
    digitalWrite(TIME_CE, 0);
    delay(1);
}

void WriteToTimer(uint8_t data, uint8_t starting_address)
{
    delay(1);
    digitalWrite(TIME_CE, 1);
    delayMicroseconds(2);
    writeData(starting_address);
    writeData(data);
    delayMicroseconds(2);
    digitalWrite(TIME_CE, 0);
    delay(1);
}

bool isMenuButtonSet(void)
{
    int button_state;
    button_state = digitalRead(MENU_PIN);
    if (button_state == LOW)
    {
        delay(50);
        button_state = digitalRead(MENU_PIN);
        return button_state == LOW;

    }
    return false;
}

bool isSetButtonSet(void)
{
    int button_state;
    button_state = digitalRead(SET_PIN);
    if (button_state == LOW)
    {
        delay(50);
        button_state = digitalRead(SET_PIN);
        return button_state == LOW;

    }
    return false;
}

void showTwoNumbers(uint8_t n1, uint8_t n2)
{
    matrix.fillScreen(0);
    drawDigt(n1,0,0,matrix.Color(0,0,255));
    drawDigt(n2,5,0,matrix.Color(0,0,255));
    matrix.show();
}

void SetTimeDate(void)
{
    uint8_t tens;
    uint8_t digt;
    uint8_t liczba;
    uint8_t increment_count;
    // Setting time/Data values

    // minutes
    ReadFromTimer(&liczba, 0x83);
    Serial.print("minuty na wejsciu ");
    Serial.println((liczba & 0xF) + (liczba >> 4) *10);
    digt = liczba & 0xF;
    tens = liczba >> 4;
    // minutes
    while(true)
    {
        showTwoNumbers(tens, digt);
        if (isSetButtonSet())
        {
            increment_count++;
            digt ++;
            if (digt > 9)
            {
                digt = 0;
                tens++;
                if(tens > 5)
                {
                    tens = 0;
                }
            }
            if (increment_count < 10)
            {   
                delay(500);
            }
            else if (increment_count < 15)
            {
                delay(300);
            }
            else
            {
                delay(150);
            }
        }
        else if (isMenuButtonSet())
        {
            // clearing seconds timer prior to minutes update
            WriteToTimer(0, 0x80);
            WriteToTimer(digt + (tens << 4), 0x82);
            
            matrix.fillScreen(0);
            matrix.show();
            delay(2000);
            break;
        }
        else
        {
            increment_count = 0;
        }
        
    }
    ReadFromTimer(&liczba, 0x83);
    Serial.print("minuty zapisane ");
    Serial.println((liczba & 0xF) + (liczba >> 4) *10);


    // godziny
    ReadFromTimer(&liczba, 0x85);
    Serial.print("godziny na wejsciu ");
    Serial.println((liczba & 0xF) + (liczba >> 4) *10);
    digt = liczba & 0xF;
    tens = liczba >> 4;
    while(true)
    {
        showTwoNumbers(tens, digt);
        if (isSetButtonSet())
        {
            digt++;
            if (digt > 9)
            {
                digt = 0;
                tens++;
            }
            if(digt + tens * 10 > 23)
            {
                digt = 0;
                tens = 0;
            }

            delay(500);

        }
        else if (isMenuButtonSet())
        {
            WriteToTimer(digt + (tens << 4), 0x84);
            matrix.fillScreen(0);
            matrix.show();
            delay(2000);
            break;
        }
    } 
    ReadFromTimer(&liczba, 0x85);
    Serial.print("godziny zapisane ");
    Serial.println((liczba & 0xF) + (liczba >> 4) *10);
    

}



void drawDigt(uint8_t digt, uint8_t x, uint8_t y, uint16_t color)
{
    uint16_t bitmap;
    //uint16_t mask = 0x40; //B100000000000000;
    uint8_t column = 0;
    uint8_t row = 0;
    //bitmap
    // * * *
    // * * * 
    // * * * 
    // * * * 
    // * * * 
    switch (digt)
    {
        case 0:
            bitmap = 0x7b6f; //B111101101101111;
            break;
        case 1:
            bitmap = 0x2492; //B010010010010010;
            break;
        case 2:
            bitmap = 0x73e7; //B111001111100111;
            break;
        case 3:
            bitmap = 0x73cf; //B111001111001111;
            break;
        case 4:
            bitmap = 0x5bc9; //B101101111001001;
            break;
        case 5:
            bitmap = 0x79cf; //B111100111001111;
            break;
        case 6:
            bitmap = 0x79ef; //B111100111101111;
            break;
        case 7:
            bitmap = 0x7249; //B111001001001001;
            break;
        case 8:
            bitmap = 0x7bef; //B111101111101111;
            break;
        case 9:
            bitmap = 0x7bcf; //B111101111001111;
            break;
        default:
            bitmap = 0;
    }
    /*Serial.print("Liczba: ");
    Serial.print(digt);
    Serial.print("mapa: ");
    Serial.println(bitmap, HEX);*/

    for(uint8_t bit = 14; bit >= 0; bit--)
    {
        if(bitmap & (1 << bit))
        {
            matrix.drawPixel(x + column ,y + row ,color);
            /*Serial.print("called draw pixel x:");
            Serial.print(x + column);
            Serial.print("y:");
            Serial.println(y + row);*/
        }

        column++;

        if (column > 2)
        {
            row++;
            column = 0;
        }
        if (bit == 0)
        {
            break;
        }
    }
}

void displayTime(void)
{
    matrix.setTextColor(matrix.Color(0, (int)random(200), 255));
    uint8_t minutes_read;
    uint8_t hours_read;
    int x = matrix.width();
    long current_time = millis();

    if ((current_time > (time_refresh_timer + TIME_REFRESH_MILIS)) || time_refresh_timer == 0)
    {
        time_refresh_timer = current_time;
        ReadFromTimer(&minutes_read, 0x83);
        ReadFromTimer(&hours_read, 0x85);
        sprintf(time_string, "%1d%1d:%1d%1d", (hours_read >> 4), (hours_read & 0xF), (minutes_read >> 4), (minutes_read & 0xF));
    }
    Serial.print("Time string: ");
    Serial.println(time_string);
    while(true)
    {
        matrix.fillScreen(0);
        matrix.setCursor(x, 1);
        matrix.print(time_string);
        // text will use about 30 pixels to print
        if(--x < -30) {
            break;
        }
        matrix.show();
        delay(120);
    }
    matrix.setCursor(0, 0);
}

void displayTemperature(void)
{
    matrix.setTextColor(matrix.Color((int)random(200), 255, 0));
    float temp;
    int x = matrix.width();
    long current_time = millis();

    if ((current_time > (temp_refresh_timer + TIME_REFRESH_MILIS)) || temp_refresh_timer == 0)
    {
        temp_refresh_timer = current_time;
        temp_sensor.requestTemperatures();
        temp = temp_sensor.getTempCByIndex(0);
        temp += TEMP_CALIBRATION;
        //sprintf(temp_string,"%f", temp);
        dtostrf(temp, 4, 1, temp_string);
        Serial.print("temperature: ");
        Serial.print(temp);
        Serial.print("string ");
        Serial.println(temp_string);
    }

    while(true)
    {
        matrix.fillScreen(0);
        matrix.setCursor(x, 1);
        matrix.print(temp_string);
        // text will use about 30 pixels to print
        if(--x < -24) {
            break;
        }
        matrix.show();
        delay(120);
    }
    matrix.setCursor(0, 0);

}

void displayChristmasTree(void)
{
    matrix.fillScreen(matrix.Color(0, 0, 255));
    matrix.fillTriangle(0, 7, 7,7, 4,1,matrix.Color(0, 255, 0));
    matrix.drawPixel(4,0,matrix.Color(200,255,0));
    matrix.show();
    delay(3000);

    matrix.drawPixel(4,0,matrix.Color(100,128,0));
    matrix.show();
    delay(10);
    matrix.drawPixel(4,0,matrix.Color(50,64,0));
    matrix.show();
    delay(10);
    matrix.drawPixel(4,0,matrix.Color(0,0,255));
    matrix.show();
    delay(50);
    matrix.drawPixel(4,0,matrix.Color(50,64,0));
    matrix.show();
    delay(10);
    matrix.drawPixel(4,0,matrix.Color(100,128,0));
    matrix.show();
    delay(10);
    matrix.drawPixel(4,0,matrix.Color(200,255,0));
    matrix.show();

    delay(2000);
}

void displayChristmasTextPolish(void)
{
    matrix.setTextColor(randomizeSolidifiedColor());
    int x = matrix.width();

    while(true)
    {
        matrix.fillScreen(0);
        matrix.setCursor(x, 1);
        matrix.print(F("Wesolych swiat"));
        if(--x < -80) {
            break;
        }
        matrix.show();
        delay(150);
    }
    matrix.setCursor(0, 0);
}

void displayChristmasTextEnglish(void)
{
    matrix.setTextColor(randomizeSolidifiedColor());
    int x = matrix.width();

    while(true)
    {
        matrix.fillScreen(0);
        matrix.setCursor(x, 1);
        matrix.print(F("Merry Christmas"));
        if(--x < -87) {
            break;
        }
        matrix.show();
        delay(150);
    }
    matrix.setCursor(0, 0);
}


void displayRawChristmasTree(void)
{
    // whole screen as green
    matrix.fillScreen(matrix.Color(0, 255, 0));

    matrix.drawLine(0,8,7,8,0);
    matrix.drawLine(0,0,7,0,0);
    matrix.drawLine(0,0,0,6,0);
    matrix.drawLine(1,0,1,4,0);
    matrix.drawLine(2,0,2,2,0);
    matrix.drawLine(5,0,5,2,0);
    matrix.drawLine(6,0,6,4,0);
    matrix.drawLine(7,0,7,6,0);

    matrix.drawLine(3,8,4,8,matrix.Color(255,200,0));
    matrix.show();
    delay(4000);
}

void rand_coordinates(uint8_t * x, uint8_t * y) 
{
    *y = random(2,8);
    switch (*y)
    {
        case 2:
            *x = random(3,5);
            break;
        case 3:
        case 4:
            *x = random(2,6);
            break;
        case 5:
        case 6:
        case 7:
            *x = random(1,7);
            break;
    }
}

void displayRawChristmasTreeWithEffects(void)
{
    // whole screen as green
    uint8_t random_x;
    uint8_t random_y;
    matrix.fillScreen(matrix.Color(0, 255, 0));

    matrix.drawLine(0,8,7,8,0);
    matrix.drawLine(0,0,7,0,0);
    matrix.drawLine(0,0,0,6,0);
    matrix.drawLine(1,0,1,4,0);
    matrix.drawLine(2,0,2,2,0);
    matrix.drawLine(5,0,5,2,0);
    matrix.drawLine(6,0,6,4,0);
    matrix.drawLine(7,0,7,6,0);

    matrix.drawLine(3,8,4,8,matrix.Color(255,200,0));

    for(uint8_t item =0; item < 5; item++)
    {
        rand_coordinates(&random_x, &random_y);
        matrix.drawPixel(random_x, random_y, randomizeSolidifiedColor());
    }
    
    matrix.show();
    delay(4000);
}

uint16_t randomizeSolidifiedColor(void)
{
    return baseColors[random(0, sizeof(baseColors) / sizeof(baseColors[0]) )];
}

void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
{
    uint16_t i, j;
    uint8_t * bitmapp = bitmap;
    uint8_t current_val = *bitmapp;
    uint8_t bit_position = 0;

    for(i=0; i < h; i++)
    {
        for(j=0; j < w; j++)
        {
            if((current_val & (1 << bit_position)))
            {
                matrix.drawPixel(x + j, y + i, color);
                //Serial.println(sprintf("drawing x = %d, y= %d", x+j, y+1));
            }
            bit_position ++;
            if (bit_position == 8)
            {
                bit_position = 0;
                bitmapp ++;
                current_val = *bitmapp;
            }
        }
    }
}

void drawDeer(void)
{   
    const uint8_t deer[] = {
        0x1B,
        0x12,
        0x0C,
        0x06,
        0x47,
        0x3E,
        0x3E,
        0x3F,
        0x25,
        0x25,
        0x25,
        0x24
    };

    uint16_t color = randomizeSolidifiedColor();

    matrix.fillScreen(0);
    
    for(int16_t y = 8; y > -13; y--)
    {
        matrix.fillScreen(0);
        drawBitmap(0, y, deer, 8, 12, color);
        matrix.show();
        delay(200);
    }

    //drawBitmap(0, 0, deer, 8, 9, matrix.Color(0,0,255));
    matrix.show();
}

void drawPresent(void)
{   
    const uint8_t present[] = {
        0x63,
        0x55,
        0x5D,
        0x7f,
        0x49,
        0x49,
        0x7f,
        0x49,
        0x49,
        0x7f,
    };

    uint16_t color = randomizeSolidifiedColor();

    matrix.fillScreen(0);
    
    for(int16_t y = 8; y > -9; y--)
    {
        matrix.fillScreen(0);
        drawBitmap(0, y, present, 8, 10, color);
        matrix.show();
        delay(150);
    }
}
