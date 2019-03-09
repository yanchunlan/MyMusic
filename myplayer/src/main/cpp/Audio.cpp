//
// Created by pc on 2019/3/8.
//

#include "Audio.h"

Audio::Audio(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    queue = new Queue(playStatus);
}

Audio::~Audio() {
//    playStatus = NULL;
//    free(queue);
}
