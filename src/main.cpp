/*TRECHOS DE https://randomnerdtutorials.com/esp32-microsd-card-arduino/*/

#include "main.h"

QueueHandle_t queue;
TaskHandle_t col, ins;

void setup(){

    /*Configuracao de pinos*/
    pinMode(RECORD_PIN, INPUT_PULLUP);
    pinMode(LED_RECORD, OUTPUT);

    /*Inicializacao*/
    Serial.begin(115200);
    sdCardInit();

    /*Criacao da fila*/
    queue = xQueueCreate(QSIZE, sizeof( row ) );
    if(queue == NULL){
    Serial.println("Error creating the queue");
    }
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    listDir(SD, "/", 0);
    }

void loop(){
    //Serial.print(">ESPACOS: ");
    //Serial.println(uxQueueSpacesAvailable(queue));
    digitalWrite(LED_RECORD, ledState);
    readRecordButton();

    switch(state){

      case STATE_IDLE:
        break;
      case STATE_COMMAND:
        state = STATE_RECORDING;
        filename = "teste03-09.csv";
        datarate = 500;
        interval = 1/datarate*1000000;
        header = "CH1, CH2, CH3, CH4\n";

        if(checkFileNames(SD, "/", 0, filename)){
          Serial.print("ALARME: existe arquivo com nome ");
          Serial.println(filename);
        }else{
          Serial.println("Nome OK!");
        }

        String filePath2 = "/" + filename;
        writeFile(SD, filePath2.c_str(), header.c_str());

        /*Task para coleta de dados*/
        xTaskCreate(
        collector,    // Função a ser chamada
        "collector",   // Nome da tarefa
        10000,            // Tamanho (bytes)
        NULL,            // Parametro a ser passado
        1,               // Prioridade da Tarefa
        &col            // Task handle
        );

      case STATE_RECORDING:

        if(uxQueueSpacesAvailable(queue) <= QSIZE/6 && !esvaziafila){
          esvaziafila = true;
          /*Criacao das tasks*/
          xTaskCreate(
          inserter,    // Função a ser chamada
          "inserter",   // Nome da tarefa
          10000,            // Tamanho (bytes)
          NULL,            // Parametro a ser passado
          1,               // Prioridade da Tarefa
          &ins          // Task handle
          );
        }
        break;
    }
    vTaskDelay(5);
    verboseStates();
}

void readRecordButton(){
  /*Debounce*/
  int reading = digitalRead(RECORD_PIN);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
        switch (state)
        {
        case STATE_IDLE:
          state = STATE_COMMAND;
          break;

        case STATE_RECORDING:
          finalizagravacao = true;
          if(!esvaziafila){
            /*Task de gavacao*/
            xTaskCreate(
            inserter,    // Função a ser chamada
            "inserter",   // Nome da tarefa
            10000,            // Tamanho (bytes)
            NULL,            // Parametro a ser passado
            1,               // Prioridade da Tarefa
            &ins          // Task handle
            );
          }
          break;

        }
      }
    }
  }
  lastButtonState = reading;
}

void collector(void * parameter){
    row datapt;
    int lastMicros = 0;
  for(;;){ // loop infinito
    memset(&datapt, 0, sizeofrow);
    datapt.time = micros();
    datapt.ch1 = analogRead(CH1);
    datapt.ch2 = analogRead(CH2);
    datapt.ch3 = analogRead(CH3);
    datapt.ch4 = analogRead(CH4);
    if(micros() - lastMicros >= interval){
        xQueueSend(queue, &datapt, 10);
        lastMicros = micros();
    }
    // Pausa a tarefa por 500ms
    vTaskDelay(2);
  }
}

void inserter(void * parameter){
    row datapt2;
  for(;;){ // loop infinito
    xQueueReceive(queue, &datapt2, 50);
    insert += String(datapt2.time) + "," + String(datapt2.ch1) + "," + String(datapt2.ch2) + "," + String(datapt2.ch3) + "," + String(datapt2.ch4) + "\n";   
    memset(&datapt2, 0, sizeofrow);
    vTaskDelay(pdMS_TO_TICKS(1));

    if(uxQueueSpacesAvailable(queue) == QSIZE && esvaziafila){
        esvaziafila = false;
        //Serial.println(insert);
        appendFile(SD, filename.c_str(), insert.c_str());
        insert.clear();
        vTaskDelete(ins);
        if(finalizagravacao){
          vTaskDelete(col);
          finalizagravacao = false;
        }
    }
  }
}