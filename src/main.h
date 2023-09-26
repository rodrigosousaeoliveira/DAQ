#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include "SDCard.h"
#include "record.h"
#include "debug.h"

#define QSIZE 1000 // Tamanho da fila
#define CH1 13 // Pino do canal 1
#define CH2 12 // Pino do canal 2
#define CH3 14 // Pino do canal 3
#define CH4 27 // Pino do canal 4
#define RECORD_PIN 22 // Pino do botao de gravacao
#define LED_RECORD LED_BUILTIN // Pino do LED de gravacao

/*Variaveis SD*/
int datarate; // Taxa de aaquisicao
int interval; // Periodo de aquisicao - 1/datarate
bool esvaziafila = false; // Flag para nao esvaziar a fila sempre
bool finalizagravacao = false;
String filename; // Nome do arquivo
String insert; // Variavel de linha de insercao
String header; // Cabecalho do arquivo

/*Variaveis debounce*/
int lastButtonState = 1;
int lastDebounceTime = 0;
int debounceDelay = 50;
int buttonState = 1;
int ledState = 0;

struct row
{
    int time;
    int ch1;
    int ch2;
    int ch3;
    int ch4;
};
int sizeofrow = sizeof(row);

enum states{
    STATE_IDLE,
    STATE_COMMAND,
    STATE_RECORDING
};
int state = STATE_IDLE; 
int lastState = STATE_IDLE;

void collector(void * parameter);
void inserter(void * parameter);
void readRecordButton();
void verboseStates(){
    if(lastState != state){
        switch(lastState){
            case STATE_IDLE:
                Serial.print("STATE_IDLE > ");
                break;
            case STATE_COMMAND:
                Serial.print("STATE_COMMAND > ");
                break;
            case STATE_RECORDING:
                Serial.print("STATE_RECORDING > ");
                break;
        }
        switch(state){
            case STATE_IDLE:
                Serial.println("STATE_IDLE");
                break;
            case STATE_COMMAND:
                Serial.println("STATE_COMMAND");
                break;
            case STATE_RECORDING:
                Serial.println("STATE_RECORDING");
                break;
        }
        lastState = state;
    }
}


#endif /*MAIN_H*/