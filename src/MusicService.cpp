#include "Audio.h"

#include "config.h"

struct audioMessage{
    uint8_t     cmd;
    const char* txt;
    uint32_t    value;
    uint32_t    ret;
} audioTxMessage, audioRxMessage;

enum : uint8_t { SET_VOLUME, GET_VOLUME, CONNECTTOSD };

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

Audio audio;

void CreateQueues(){
    audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
    audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter) {
    CreateQueues();
    if(!audioSetQueue || !audioGetQueue){
        log_e("queues are not initialized");
        while(true){;}  // endless loop
    }

    struct audioMessage audioRxTaskMessage;
    struct audioMessage audioTxTaskMessage;

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(1); // 0...21

    while(true){
        if(xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
            if(audioRxTaskMessage.cmd == SET_VOLUME){
                audioTxTaskMessage.cmd = SET_VOLUME;
                audio.setVolume(audioRxTaskMessage.value);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOSD){
                audioTxTaskMessage.cmd = CONNECTTOSD;
                audioTxTaskMessage.ret = audio.connecttoSD(audioRxTaskMessage.txt);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_VOLUME){
                audioTxTaskMessage.cmd = GET_VOLUME;
                audioTxTaskMessage.ret = audio.getVolume();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else{
                log_i("error");
            }
        }
        audio.loop();
    }
}

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,             /* Function to implement the task */
        "audioplay",           /* Name of the task */
        5000,                  /* Stack size in words */
        NULL,                  /* Task input parameter */
        2 | portPRIVILEGE_BIT, /* Priority of the task */
        NULL,                  /* Task handle. */
        1                      /* Core where the task should run */
    );
}

audioMessage transmitReceive(audioMessage msg){
    xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
    if(xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS){
        if(msg.cmd != audioRxMessage.cmd){
            log_e("wrong reply from message queue");
        }
    }
    return audioRxMessage;
}

void audioSetVolume(uint8_t vol){
    audioTxMessage.cmd = SET_VOLUME;
    audioTxMessage.value = vol;
    audioMessage RX = transmitReceive(audioTxMessage);
}

uint8_t audioGetVolume(){
    audioTxMessage.cmd = GET_VOLUME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

bool audioConnecttoSD(const char* filename){
    audioTxMessage.cmd = CONNECTTOSD;
    audioTxMessage.txt = filename;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}
