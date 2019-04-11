# MyMusic ffmpeg 解码音频并控制OpenSLES播放，解码视频在OpenGLES上播放
ffmpeg 版本为 4.0.3
nkd 编译环境是r16b

master 分支为万能音频播放器<br>
video 分支为视频播放器<br>

音频部分：<br>
        01. ffmpeg导入库<br>
        02. 音视频解码流程<br>
        03. ffmpeg对音频数据重采样生成PCM数据<br>
        04. 重采样<br>
        05. OPenSL ES 实现音频PCM播放<br>
        06. ffmpeg + OPenSL ES 实现音频播放<br>
        07. 添加加载、暂停、播放功能<br>
        08. 播放时间计算<br>
        09. 停止播放释放内存<br>
        10. 出错回调<br>
        11. Seek功能和完成播放回调<br>
        12. 播放切换功能<br>
视频部分：<br>
        01. ffmpeg获取视频原始AVPacket<br>
        02. ffmpeg获取视频AVFrame<br>
        03. 获取YUV数据，并用OpenGLES渲染数据<br>
        04.	音视频同步<br>
        05.	视频seek功能，暂停，播放<br>
        06.	MediaCodec硬解码<br>
            	1>	检测是否可以被硬解码：<br>
            	2>	AVPacket添加解码头信息<br>
            	3>	MediaCodec解码AVPacket<br>
            	4>	OpenGL渲染MediaCodec解码数据<br>
        07.	优化<br>
附件：<br>
    ffmpeg编译脚本： ffmpeg_android__build全平台（除mips）.sh