from tcaxPy import *

# 用户修订版的tcaxPy_Main函数, 保留原始tcaxPy_Main风格是为了减少额外记忆量, 实际上该函数形式可以非常多样化
def tcaxPy_Main_Fake(_i, _j, _n, _start, _end, _elapk, _k, _x, _y, _a, _txt, param):
    ASS_BUF  = []        # 保存ASS特效
    TCAS_BUF = []        # 保存TCAS特效
    ##### 主要特效编写操作 ####
    _Spacing = GetVal(val_Spacing)
    _Fs = GetVal(val_FontSize)
    dx = _x - int((_a + _Spacing) / 2 + 0.5)     # 一个固定操作, 将an5的坐标转换为an7
    dy = _y - int(_Fs / 2 + 0.5)                 # ASS特效默认采用an5坐标, TCAS特效则采用an7坐标
    # 增加特效
    # ass_main...
    ass_main(ASS_BUF, SubL(_start, _end), pos(_x, _y), _txt)
    # tcas_main...
    ##### 将结果返回给tcax进行处理 #####
    return (ASS_BUF, TCAS_BUF)

# 除了少数几个细节的不同, 本例子实际上就是TCAX内部对tcaxPy_Main函数的封装方法
def tcaxPy_User():
    # 创建ASS文件
    file_name  = GetVal(val_OutFile) + '.ass'
    ass_header = GetVal(val_AssHeader)
    ASS_FILE   = CreateAssFile(file_name, ass_header)
    # 创建TCAS文件
    file_name  = GetVal(val_OutFile) + '.tcas'
    fx_width   = GetVal(val_ResolutionX)
    fx_height  = GetVal(val_ResolutionY)
    fx_fps     = GetVal(val_FXFPS)
    TCAS_FILE  = CreateTcasFile(file_name, fx_width, fx_height, fx_fps)
    # 基本数据, 如果需要查看含义, 请取消下面两行代码的注释
    #GetHelp()
    #Pause()
    fontSize   = GetVal(val_FontSize)
    marginX    = GetVal(val_OffsetX)
    marginY    = GetVal(val_OffsetY)
    spacing    = GetVal(val_Spacing)
    lineNum    = GetVal(val_nLines)
    textNum    = GetVal(val_nTexts)
    start      = GetVal(val_BegTime)
    end        = GetVal(val_EndTime)
    kar        = GetVal(val_KarTime)
    elapKar    = GetVal(val_KarTimeDiff)
    text       = GetVal(val_Text)
    textLength = GetVal(val_TextLength)
    advance    = GetVal(val_TextAdvance)
    advDiff    = GetVal(val_TextAdvanceDiff)
    # 主循环
    for i in range(lineNum):
        initPosX = marginX                                          # an 7, 4, 1
        #initPosX = (fx_width - textLength[i]) / 2 + marginX        # an 8, 5, 2
        #initPosX = fx_width - textLength[i] - marginX              # an 9, 6, 3
        initPosY = marginY                                          # an 7, 8, 9
        #initPosY = fx_height / 2 - marginY                         # an 4, 5, 6
        #initPosY = fx_height - fontSize - marginY                  # an 1, 2, 3
        for j in range(textNum[i]):
            if text[i][j] == '' or text[i][j] == ' ' or text[i][j] == '　':
                continue
            posX = initPosX + advDiff[i][j] + advance[i][j] / 2
            posY = initPosY + fontSize / 2
            param = [None]      # 传递需要的参数到tcaxPy_Main_Fake函数中
            ASS_BUF, TCAS_BUF = tcaxPy_Main_Fake(i, j, textNum[i], start[i], end[i], elapKar[i][j], kar[i][j], posX, posY, advance[i][j], text[i][j], param)
            if len(ASS_BUF) > 0:
                WriteAssFile(ASS_FILE, ASS_BUF)
            if len(TCAS_BUF) > 0:
                WriteTcasFile(TCAS_FILE, TCAS_BUF)
            Progress(i, j)      # 显示进度
    FinAssFile(ASS_FILE)
    FinTcasFile(TCAS_FILE)

