//
// Created by pc on 2019/3/9.
//

#ifndef MYMUSIC_PLAYSTATUS_H
#define MYMUSIC_PLAYSTATUS_H

class PlayStatus {
public:
    bool exit= false;
    bool load = true;
    bool seek = false;
    bool pause = false;

public:
    PlayStatus();

    virtual ~PlayStatus();
};

#endif //MYMUSIC_PLAYSTATUS_H
