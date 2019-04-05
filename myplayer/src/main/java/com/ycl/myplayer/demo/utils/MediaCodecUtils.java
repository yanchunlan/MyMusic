package com.ycl.myplayer.demo.utils;

import android.annotation.TargetApi;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;
import android.support.annotation.RequiresApi;

import java.util.HashMap;
import java.util.Map;

/**
 * author:  ycl
 * date:  2019/4/5 11:37
 * desc:
 */
public class MediaCodecUtils {

    private static Map<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264", MediaFormat.MIMETYPE_VIDEO_AVC);
    }

    public static String findVideoCodecName(String ffcodecName) {
        if (codecMap.containsKey(ffcodecName)) {
            return codecMap.get(ffcodecName);
        }
        return "";
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public static boolean isSupportCodec(String ffcodecName) {
        boolean supportCodec = false;
        int count = MediaCodecList.getCodecCount();
        for (int i = 0; i < count; i++) {
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equals(findVideoCodecName(ffcodecName))) {
                    supportCodec = true;
                    break;
                }
            }
            if (supportCodec) {
                break;
            }
        }
        return supportCodec;
    }
}
