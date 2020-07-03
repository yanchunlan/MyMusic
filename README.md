# MyMusic ffmpeg 解码音频并控制OpenSLES播放
环境： ffmpeg 版本为 4.0.3 , nkd 编译环境是r16b

master 分支为万能音频播放器<br>
video 分支为视频播放器<br>

音频部分：<br>

        01.ffmpeg导入库<br>
        02.音视频解码流程<br>
        03.FFmpeg对音频数据重采样生成PCM数据<br>
        04.重采样<br>
        05.OPenSL ES 实现音频PCM播放<br>
        06.FFmpeg + OPenSL ES 实现音频播放<br>
        07.添加加载、暂停、播放功能<br>
        08.播放时间计算<br>
        09.停止播放释放内存<br>
        10.出错回调<br>
        11.Seek功能和完成播放回调<br>
        12.播放切换功能<br>
        
音频高级部分：待完成<br>
        
        13.音频控制<br>
        14.左右声道切换<br>
        15.变速变调功能<br>
        16.计算pcm分贝值<br>
        17.mediaCodec编码pcm为aac音频边播边录<br>
        18.添加边播边录时间回调<br>
        19.优化sdk<br>
        拓展<br>
            1.播放.ape音乐异常原因分析和解决<br>
            2.音频裁剪返回pcm数据可进行二次开发<br>
            3.pcm大数据分包（和解决录音奔溃问题）<br>
            


附件：

    ffmpeg编译脚本： ffmpeg_android__build全平台（除mips）.sh