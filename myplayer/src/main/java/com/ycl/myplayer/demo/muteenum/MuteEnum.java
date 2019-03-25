package com.ycl.myplayer.demo.muteenum;

/**
 * author:  ycl
 * date:  2019/3/25 10:50
 * desc:
 */
public enum MuteEnum {
    MUTE_RIGHT("RIGHT", 0),
    MUTE_LEFT("LEFT", 1),
    MUTE_CENTER("CENTER", 2);

    private String name;
    private int  value;

    MuteEnum(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
