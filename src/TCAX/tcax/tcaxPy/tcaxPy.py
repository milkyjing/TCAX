# -*- coding: utf-8 -*-


########################################## Declaration ######################################################

# Declaration:
# tcaxPy.pyc's source code
# Copyright (c) 2008 - 2012 milkyjing (milkyjing@gmail.com). All rights reserved.
# Please visit tcax.rhacg.com to get the latest information
# Version: ver0.7.3.5 build20100109
# Revision: ver0.7.5.0 build20100912
# Revision: ver0.7.8.1 build20110703
# Revision: ver0.7.9.0 build20110721
# Revision: ver0.7.9.2 build20110812
# Revision: ver0.7.9.3 build20110813
# Revision: ver0.7.9.8 build20110901
# Revision: ver0.7.9.9 build20111222
# Revision: ver0.8.0.0 build20120722

######################################### Modules ###########################################################

from math    import *
from random  import *
from tcaxLib import *
import sys

######################################### Global Constant ###################################################

tcaxPy_Version       = '0.8.0.0'

Main_Style           = 'TCMS'	# 主Style名称 SubL函数默认使用的Style
Pix_Style            = 'TCPS'	# 粒子特效的Style名称

val_AssHeader        = 0	# Ass文件头部信息 用于tcaxPy_User函数
val_OutFile          = 1	# 输出的文件名 不含拓展名 用于tcaxPy_User函数

# tcc info
val_FontFileName     = 2	# 特效使用的字体
val_FaceID           = 3	# 字体名称ID
val_FontSize         = 4	# 字体大小
val_ResolutionX      = 5	# 水平分辨率
val_ResolutionY      = 6	# 垂直分辨率
val_FXFPS            = 7	# 特效的帧率
val_Alignment        = 8	# 对齐方式
val_OffsetX          = 9	# 水平偏移量
val_OffsetY          = 10	# 垂直偏移量
val_Spacing          = 11	# 字体间距
val_SpaceScale       = 12	# 空格跨度百分比
val_FontFaceName     = 13	# 特效使用的字体Face名
val_Bord             = 14	# 字体边框厚度
val_Shad             = 15	# 字体阴影宽度
val_1C               = 16	# 字体主颜色
val_2C               = 17	# 字体辅助颜色
val_3C               = 18	# 字体边框颜色
val_4C               = 19	# 字体阴影颜色
val_1A               = 20	# 字体主透明度
val_2A               = 21	# 字体辅助透明度
val_3A               = 22	# 字体边框透明度
val_4A               = 23	# 字体阴影透明度
val_Blur             = 24	# 模糊值

# syl info
val_nLines           = 25	# SYL文件的卡拉OK歌词句子总数
val_SylLine          = 26	# 第i句卡拉OK歌词
val_BegTime          = 27	# 第i句卡拉OK歌词的起始时间
val_EndTime          = 28	# 第i句卡拉OK歌词的结束时间
val_nTexts           = 29	# 第i句卡拉OK歌词包含的字符数
val_KarTime          = 30	# 第i句卡拉OK歌词的第j个字符的Karaoke时间
val_KarTimeDiff      = 31	# 到第i句卡拉OK歌词的第j个字符之前的Karaoke时间总和
val_Text             = 32	# 第i句卡拉OK歌词的第j个字符

# tm info (horizontal)
val_Ascender         = 33	# 字体上行高度
val_Descender        = 34	# 字体下行高度 通常为负数
val_TextWidth        = 35	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeight       = 36	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerning      = 37	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0
val_TextAdvance      = 38	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiff  = 39	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLength       = 40	# 第i句卡拉OK歌词的文字总长度
val_TextInitX        = 41	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitY        = 42	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingY     = 43	# 第i句卡拉OK歌词的j个字符的Y轴bearing

# tm info horizontal
val_TextWidthH       = 35	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeightH      = 36	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerningH     = 37	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0
val_TextAdvanceH     = 38	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiffH = 39	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLengthH      = 40	# 第i句卡拉OK歌词的文字总长度
val_TextInitXH       = 41	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitYH       = 42	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingYH    = 43	# 第i句卡拉OK歌词的j个字符的Y轴bearing

# tm info vertical
val_TextWidthV       = 44	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeightV      = 45	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerningV     = 46	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0 (暂时不支持vertical版本的kerning，其值全部为0)
val_TextAdvanceV     = 47	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiffV = 48	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLengthV      = 49	# 第i句卡拉OK歌词的文字总长度
val_TextInitXV       = 50	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitYV       = 51	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingXV    = 52	# 第i句卡拉OK歌词的j个字符的X轴bearing

######################################### Data Management Function ##########################################

def GetValueIDsInfo():
    val_info = '''

tcaxPy_Version       = '0.8.0.0'

Main_Style           = 'TCMS'	# 主Style名称 SubL函数默认使用的Style
Pix_Style            = 'TCPS'	# 粒子特效的Style名称

val_AssHeader        = 0	# Ass文件头部信息 用于tcaxPy_User函数
val_OutFile          = 1	# 输出的文件名 不含拓展名 用于tcaxPy_User函数

# tcc info
val_FontFileName     = 2	# 特效使用的字体
val_FaceID           = 3	# 字体名称ID
val_FontSize         = 4	# 字体大小
val_ResolutionX      = 5	# 水平分辨率
val_ResolutionY      = 6	# 垂直分辨率
val_FXFPS            = 7	# 特效的帧率
val_Alignment        = 8	# 对齐方式
val_OffsetX          = 9	# 水平偏移量
val_OffsetY          = 10	# 垂直偏移量
val_Spacing          = 11	# 字体间距
val_SpaceScale       = 12	# 空格跨度百分比
val_FontFaceName     = 13	# 特效使用的字体Face名
val_Bord             = 14	# 字体边框厚度
val_Shad             = 15	# 字体阴影宽度
val_1C               = 16	# 字体主颜色
val_2C               = 17	# 字体辅助颜色
val_3C               = 18	# 字体边框颜色
val_4C               = 19	# 字体阴影颜色
val_1A               = 20	# 字体主透明度
val_2A               = 21	# 字体辅助透明度
val_3A               = 22	# 字体边框透明度
val_4A               = 23	# 字体阴影透明度
val_Blur             = 24	# 模糊值

# syl info
val_nLines           = 25	# SYL文件的卡拉OK歌词句子总数
val_SylLine          = 26	# 第i句卡拉OK歌词
val_BegTime          = 27	# 第i句卡拉OK歌词的起始时间
val_EndTime          = 28	# 第i句卡拉OK歌词的结束时间
val_nTexts           = 29	# 第i句卡拉OK歌词包含的字符数
val_KarTime          = 30	# 第i句卡拉OK歌词的第j个字符的Karaoke时间
val_KarTimeDiff      = 31	# 到第i句卡拉OK歌词的第j个字符之前的Karaoke时间总和
val_Text             = 32	# 第i句卡拉OK歌词的第j个字符

# tm info (horizontal)
val_Ascender         = 33	# 字体上行高度
val_Descender        = 34	# 字体下行高度 通常为负数
val_TextWidth        = 35	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeight       = 36	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerning      = 37	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0
val_TextAdvance      = 38	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiff  = 39	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLength       = 40	# 第i句卡拉OK歌词的文字总长度
val_TextInitX        = 41	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitY        = 42	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingY     = 43	# 第i句卡拉OK歌词的j个字符的Y轴bearing

# tm info horizontal
val_TextWidthH       = 35	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeightH      = 36	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerningH     = 37	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0
val_TextAdvanceH     = 38	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiffH = 39	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLengthH      = 40	# 第i句卡拉OK歌词的文字总长度
val_TextInitXH       = 41	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitYH       = 42	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingYH    = 43	# 第i句卡拉OK歌词的j个字符的Y轴bearing

# tm info vertical
val_TextWidthV       = 44	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeightV      = 45	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerningV     = 46	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0 (暂时不支持vertical版本的kerning，其值全部为0)
val_TextAdvanceV     = 47	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiffV = 48	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLengthV      = 49	# 第i句卡拉OK歌词的文字总长度
val_TextInitXV       = 50	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitYV       = 51	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingXV    = 52	# 第i句卡拉OK歌词的j个字符的X轴bearing

'''
    return val_info

def tcaxPy_InitData(data):
    global __tcax_data
    __tcax_data = data
    tcaxPy_UtilInitGlobal()

def GetData():
    return __tcax_data

def GetVal(item):
    return __tcax_data[item]

def getval_kartxt(i):
    return __tcax_data[val_SylLine][i].split(',,')[-1]

### Utility ###

def tcaxPy_UtilInitGlobal():
    global __util_temp
    __util_temp = [None]

def IsLineChanged(i):
    if i != __util_temp[0]:
        __util_temp[0] = i
        return True
    else:
        return False

def UseSitePackages():      # invoke this function right after importing tcaxPy module if you want to use Python 3rd party packages
    sys.path.append(sys.prefix)
    sys.path.append(sys.prefix + '\\DLLs')
    sys.path.append(sys.prefix + '\\Lib')
    sys.path.append(sys.prefix + '\\Lib\\site-packages')

########################################## Main FX Function #################################################
def getpos(i,j,an):
    #__tcax_data = GetData()
    if an == 1:
    	initPosX =  __tcax_data[9]
    	initPosY = __tcax_data[6] - __tcax_data[10] - __tcax_data[36][i][j]
    elif an == 2:
    	initPosX = (__tcax_data[5] - __tcax_data[40][i]) / 2 + __tcax_data[9]
    	initPosY =  __tcax_data[6] - __tcax_data[10] - __tcax_data[36][i][j]
    elif an == 3:
    	initPosX = __tcax_data[5] - __tcax_data[40][i] - __tcax_data[9]
    	initPosY = __tcax_data[6] - __tcax_data[10] - __tcax_data[36][i][j]
    elif an == 4:
    	initPosX =  __tcax_data[9]
    	initPosY = __tcax_data[6]/2 + __tcax_data[10]
    elif an == 5:
    	initPosX = (__tcax_data[5] - __tcax_data[40][i]) / 2 + __tcax_data[9]
    	initPosY = __tcax_data[6]/2 + __tcax_data[10]
    elif an == 6:
    	initPosX = __tcax_data[5] - __tcax_data[40][i] - __tcax_data[9]
    	initPosY = __tcax_data[6]/2 + __tcax_data[10]
    elif an == 7:
    	initPosX =  __tcax_data[9]
    	initPosY = __tcax_data[10] + __tcax_data[36][i][j]
    elif an == 8:
    	initPosX = (__tcax_data[5] - __tcax_data[40][i]) / 2 + __tcax_data[9]
    	initPosY =  __tcax_data[10] + __tcax_data[4]
    elif an == 9:
    	initPosX = (__tcax_data[5] - __tcax_data[40][i]) / 2 + __tcax_data[9]
    	initPosY =  __tcax_data[10] + __tcax_data[36][i][j]

    posX = initPosX + __tcax_data[39][i][j] + __tcax_data[38][i][j] / 2
    posY = initPosY

    return (posX,posY)

def SubL(Start = 0, End = 0, Layer = 0, Style = 'TCMS'):
    return 'Dialogue: {0},{1},{2},{3},NTP,0000,0000,0000,,'.format(int(Layer), FmtTime(Start), FmtTime(End), Style)

def ass_main(ASS_BUF, SubDlg = '', Event = '', Text = ''):
    if Event == '':
        ASS_BUF.append('{0}{1}{2}\r\n'.format(SubDlg, Event, Text))
    else:
        ASS_BUF.append('{0}{{{1}}}{2}\r\n'.format(SubDlg, Event, Text))

def tcas_user(TCAS_BUF, Start = 0, End = 0, PosX = 0, PosY = 0, RGB = 0xFFFFFF, Alpha = 0, Layer = 0):
    if Alpha < 0:
        Alpha = 0
    if Start <= End:
        TCAS_BUF.append((int(Start), int(End), int(Layer), int(PosX), int(PosY), int(RGB), int(Alpha)))

#############################################################################################################

######################################### Useful Funtions ###################################################

#--------------------------------------------- Format Function ----------------------------------------------#

# 格式化相关的函数

def FmtHex(n):                                   # dec to hex FormatHex
    if n <= 0:
        return '00'
    elif n >= 255:
        return 'FF'
    else:
        return '{0:02X}'.format(int(n))

def HexToDec(a):                                 # hex to dec
    return int(a, 16)

def FmtFloat(f):
    if f == int(f):
        F = str(int(f))
    else:
        F = str(format(f, '.2f'))
    return F

def FmtRGB(r, g, b):                             # return a formated RGB string
    return FmtHex(b) + FmtHex(g) + FmtHex(r)

def DeFmtRGB(CLR):                               # convert RGB string to RGB tuple
    CLR_LIS_3 = HexToDec(CLR[0:2])
    CLR_LIS_2 = HexToDec(CLR[2:4])
    CLR_LIS_1 = HexToDec(CLR[4:6])
    return (CLR_LIS_1, CLR_LIS_2, CLR_LIS_3)

def DecRGB(RGB):                                 # convert RGB string to RGB dec value
    return int('0x' + RGB, 16)

def clip_0_255(a):
    if a < 0:
        return 0
    elif a > 255:
        return 255
    else:
        return int(a)

def MakeRGB(r, g, b):
    return clip_0_255(r) + clip_0_255(g) * 256 + clip_0_255(b) * 65536

def MakeRGBA(r, g, b, a):
    return clip_0_255(r) + clip_0_255(g) * 256 + clip_0_255(b) * 65536 + clip_0_255(a) * 16777216

#-------------------------------- Ass tag functions -------------------------------#

def an(a):
    return '\\an{0}'.format(int(a))

def K(a):
    return '\\K{0}'.format(int(a))

def k(a):
    return '\\k{0}'.format(int(a))

def ko(a):
    return '\\ko{0}'.format(int(a))

def kf(a):
    return '\\kf{0}'.format(int(a))

def t(a1, a2 = None, a3 = None, a4 = None):
    if a2 == None:
        return '\\t({code})'.format(code = a1)
    elif a3 == None:
        if a1 == int(a1):
            A = str(int(a1))
        else:
            A = str(format(a1, '.2f'))
        return '\\t({a},{code})'.format(a = A, code = a2)
    elif a4 == None:
        return '\\t({t1},{t2},{code})'.format(t1 = int(a1), t2 = int(a2), code = a3)
    else:
        if a3 == int(a3):
            A = str(int(a3))
        else:
            A = str(format(a3, '.2f'))
        return '\\t({t1},{t2},{a},{code})'.format(t1 = int(a1), t2 = int(a2), a = A, code = a4)

def animation(a1, a2 = None, a3 = None, a4 = None):
    if a2 == None:
        return '\\t({code})'.format(code = a1)
    elif a3 == None:
        if a1 == int(a1):
            A = str(int(a1))
        else:
            A = str(format(a1, '.2f'))
        return '\\t({a},{code})'.format(a = A, code = a2)
    elif a4 == None:
        return '\\t({t1},{t2},{code})'.format(t1 = int(a1), t2 = int(a2), code = a3)
    else:
        if a3 == int(a3):
            A = str(int(a3))
        else:
            A = str(format(a3, '.2f'))
        return '\\t({t1},{t2},{a},{code})'.format(t1 = int(a1), t2 = int(a2), a = A, code = a4)

def t1(t1, t2, code):    # deprecated
    return '\\t({0},{1},{2})'.format(int(t1), int(t2), code)

def animation1(t1, t2, code):    # deprecated
    return '\\t({0},{1},{2})'.format(int(t1), int(t2), code)

def t2(t1, t2, a, code):    # deprecated
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    return '\\t({0},{1},{2},{3})'.format(int(t1), int(t2), A, code)

def animation2(t1, t2, a, code):    # deprecated
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    return '\\t({0},{1},{2},{3})'.format(int(t1), int(t2), A, code)

def fscx(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\fscx{0}'.format(X)

def fscy(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    return '\\fscy{0}'.format(Y)

def fsc(x, y):
    if x == int(x) and y == int(y):
        X = str(int(x))
        Y = str(int(y))
    else:
        X = str(format(x, '.2f'))
        Y = str(format(y, '.2f'))
    return '\\fscx{0}\\fscy{1}'.format(X, Y)

def fn(a):
    return '\\fn{0}'.format(a)

def fs(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\fs{0}'.format(X)

def fad(t1, t2):
    return '\\fad({0},{1})'.format(int(t1), int(t2))

def fade(a1, a2, a3, t1, t2, t3, t4):
    return '\\fade({0},{1},{2},{3},{4},{5},{6})'.format(clip_0_255(a1), clip_0_255(a2), clip_0_255(a3), int(t1), int(t2), int(t3), int(t4))

def bord(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\bord{0}'.format(X)

def shad(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\shad{0}'.format(X)

def blur(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\blur{0}'.format(X)

def p(a):
    return '\\p{0}'.format(int(a))

def b():
    return '\\b{0}'.format(int(a))

def be(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\be{0}'.format(X)

def xbord(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\xbord{0}'.format(X)

def ybord(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    return '\\ybord{0}'.format(Y)

def xshad(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\xshad{0}'.format(X)

def yshad(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    return '\\yshad{0}'.format(Y)

def fax(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    return '\\fax{0}'.format(X)

def fay(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    return '\\fay{0}'.format(Y)

def frx(a):
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    return '\\frx{0}'.format(A)

def fry(a):
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    return '\\fry{0}'.format(A)

def frz(a):
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    return '\\frz{0}'.format(A)

def fr(a):
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    return '\\fr{0}'.format(A)

def pos(x, y):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    return '\\pos({0},{1})'.format(X, Y)

def org(x, y):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    return '\\org({0},{1})'.format(X, Y)

def alpha(a):                                    # note that a is a dec value
    return '\\alpha&H{0}&'.format(FmtHex(a))

def alpha1(a):
    return '\\1a&H{0}&'.format(FmtHex(a))

def alpha2(a):
    return '\\2a&H{0}&'.format(FmtHex(a))

def alpha3(a):
    return '\\3a&H{0}&'.format(FmtHex(a))

def alpha4(a):
    return '\\4a&H{0}&'.format(FmtHex(a))

def color(c):                                    # note that c is a RGB string
    return '\\c&H{0}&'.format(c)

def color1(c):
    return '\\1c&H{0}&'.format(c)

def color2(c):
    return '\\2c&H{0}&'.format(c)

def color3(c):
    return '\\3c&H{0}&'.format(c)

def color4(c):
    return '\\4c&H{0}&'.format(c)

def mov(x1, y1, x2, y2):    # deprecated
    if x1 == int(x1) and y1 == int(y1) and x2 == int(x2) and y2 == int(y2):
        X1 = str(int(x1))
        Y1 = str(int(y1))
        X2 = str(int(x2))
        Y2 = str(int(y2))
    else:
        X1 = str(format(x1, '.2f'))
        Y1 = str(format(y1, '.2f'))
        X2 = str(format(x2, '.2f'))
        Y2 = str(format(y2, '.2f'))
    return '\\move({0},{1},{2},{3})'.format(X1, Y1, X2, Y2)

def move(x1, y1, x2, y2, t1 = None, t2 = None):
    if x1 == int(x1) and y1 == int(y1) and x2 == int(x2) and y2 == int(y2):
        X1 = str(int(x1))
        Y1 = str(int(y1))
        X2 = str(int(x2))
        Y2 = str(int(y2))
    else:
        X1 = str(format(x1, '.2f'))
        Y1 = str(format(y1, '.2f'))
        X2 = str(format(x2, '.2f'))
        Y2 = str(format(y2, '.2f'))
    if t2 == None:
        return '\\move({0},{1},{2},{3})'.format(X1, Y1, X2, Y2)
    else:
        return '\\move({0},{1},{2},{3},{4},{5})'.format(X1, Y1, X2, Y2, int(t1), int(t2))

def clip(a1, a2 = None, a3 = None, a4 = None):
    if a2 == None:
        return '\\clip({draw})'.format(draw = a1)
    elif a3 == None:
        return '\\clip({scale},{draw})'.format(scale = a1, draw = a2)
    else:
        return '\\clip({x1},{y1},{x2},{y2})'.format(x1 = int(a1), y1 = int(a2), x2 = int(a3), y2 = int(a4))

def clip1(Draw):    # deprecated
    return '\\clip({0})'.format(Draw)

def clip2(Scale, Draw):    # deprecated
    return '\\clip({0},{1})'.format(Scale, Draw)

def iclip(a1, a2 = None, a3 = None, a4 = None):
    if a2 == None:
        return '\\iclip({draw})'.format(draw = a1)
    elif a3 == None:
        return '\\iclip({scale},{draw})'.format(scale = a1, draw = a2)
    else:
        return '\\iclip({x1},{y1},{x2},{y2})'.format(x1 = int(a1), y1 = int(a2), x2 = int(a3), y2 = int(a4))

def iclip1(Draw):    # deprecated
    return '\\iclip({0})'.format(Draw)

def iclip2(Scale, Draw):    # deprecated
    return '\\iclip({0},{1})'.format(Scale, Draw)

#--------------------------------------------- Utility Function -------------------------------------------#

def GetVersion():
    print('tcaxLib version: ' + tcaxLibGetVersion() + '\ntcaxPy version: ' + tcaxPy_Version)

def GetHelp():
    print(GetValueIDsInfo())

def Pause():
    print('Press any key to continue...')
    sys.stdin.readline()

def GetRootDir():
    return __file__.rsplit('\\', 1)[0]

def GetWorkingDir():
    return sys.path[0]

def abspath(filename):
    return sys.path[0] + '\\' + filename

def MakePath(FolderIndex = 0, ImageIndex = 0, MainFolder = 'src', SubFolder = 'list', ImageName = 'img', ImageType = '.png', PathType = 'pi'):
    if PathType == 'pi':
        img_path = '%s\%s%d\%s%04d%s' % (MainFolder, SubFolder, FolderIndex, ImageName, ImageIndex, ImageType)
    elif PathType == 'sys':
        img_path = '%s\%s%d\%s (%d)%s' % (MainFolder, SubFolder, FolderIndex, ImageName, ImageIndex, ImageType)
    else:
        img_path = '%s\%s%d\%s%04d%s' % (MainFolder, SubFolder, FolderIndex, ImageName, ImageIndex, ImageType)
    return img_path

def tcaxLog(info):
    s = str(info) + '\r\n'
    logfile = open(GetVal(val_OutFile) + '_data.log', 'ab')
    logfile.write(b'\xef\xbb\xbf')             # codecs.BOM_UTF8
    logfile.write(s.encode('utf-8'))
    logfile.close()

def Progress(i, j, file_id = 1, file_num = 1):
    total = 0
    for l in range(__tcax_data[val_nLines]):
        total += __tcax_data[val_nTexts][l]
    completed = 0
    for l in range(i):
        completed += __tcax_data[val_nTexts][l]
    completed += j + 1
    ShowProgress(total, completed, file_id - 1, file_num)

def progress(completed, total):
    ShowProgress(total, completed, 0, 1)

#--------------------------------------------- Advanced Function -------------------------------------------#

def Sum(LIS, I, diff = 0):       # 高级加法函数 在数列LIS里 从第一项累加到第I项
    SUM = 0
    for i in range(I):
        SUM += LIS[i] + diff
    return SUM

def AdvInt(a):       # 高级取整函数 AdvancedIntegrate
    if a >= 0 and a - int(a) < 0.5:
        return int(a)
    elif a >= 0 and a - int(a) >= 0.5:
        return int(a) + 1
    elif a < 0 and a - int(a) <= -0.5:
        return int(a) - 1
    else:                # elif a < 0 and a - int(a) > -0.5:
        return int(a)

def DeFmtTime(TIME):       # 重新数字化已被格式化的时间 TIME = '0:00:00.00'
    return int(TIME[0:1]) * 60 * 60 * 100 + int(TIME[2:4]) * 60 * 100 + int(TIME[5:7]) * 100 + int(TIME[8:10])

def FmtTime(t):       # 格式化时间
    t = int(t)
    hour = int(t / 360000)
    minute = int((t / 6000) % 60)
    second = int((t / 100) % 60)
    semiSecond = int(t % 100)
    return '{0}:{1:02d}:{2:02d}.{3:02d}'.format(hour, minute, second, semiSecond)

def PixPt():       # 返回一个像素点 PixelPoint, deprecated, use DrawPoint instead
    return '{\\p1}m 0 0 l 1 0 1 1 0 1{\\p0}'

def DrawPoint():
    return '{\\p1}m 0 0 l 1 0 1 1 0 1{\\p0}'

def DrawLight(l):
    light = '{\\p4}'
    l *= 8
    light += 'm 0 0 l -{length} 0 0 8 c'.format(length = int(l))
    light += '{\\p0}'
    return light

def MovPxl():       # 移动单位个像素 MovePixel
    if randint(0, 1) == 0:
        return -1
    else:
        return 1

def GetDistance(x1, y1, x2, y2):
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))

def GetAngle(x, y, ox, oy):
    r = GetDistance(x, y, ox, oy)
    if r == 0:
        return 0
    a = math.acos((x - ox) / r)
    if y > oy:
        a = math.pi * 2 - a
    return a

def RandSign():
    if randint(0, 1) == 1:
        return 1
    else:
        return -1

def RandomGauss(l, h, g):
    sum = 0
    for i in range(g):
        sum += l + (h - l) * random()
    return sum / g

def RandomDouble(l, h):
    return l + (h - l) * random()

def RandGauss(l, h):
    sum = 0
    for i in range(6):
        sum += l + (h - l) * random()
    return sum / 6

def RandRGB():       # 返回一个随机的RGB值 RandomRGB
    return FmtRGB(randint(0, 255), randint(0, 255), randint(0, 255))

def RandA():       # 返回一个随机的Alpha值 RandomAlpha
    return FmtHex(randint(0, 255))

def RandImg(n, IMG_WD, IMG_HT):       # 随机图形 RandomImage
    STR_RAND_IMG = 'm ' + str(randint(0, IMG_WD)) + ' ' + str(randint(0, IMG_HT)) + ' b '
    for i in range(6 * n):
        if i % 2 == 0:
            STR_RAND_IMG += str(randint(0, IMG_WD)) + ' '
        else:
            STR_RAND_IMG += str(randint(0, IMG_HT)) + ' '
    return STR_RAND_IMG + 'c'

def RandCir(a, b, r):       # 随机圆分布函数 RandomCircle1
    R = randint(0, r)
    theta = randint(0, 2 * 314) / 100.0
    x = AdvInt(R * cos(theta) + a)
    y = AdvInt(R * sin(theta) + b)
    return (x, y)

def RandCir2(a, b, r1, r2):      # 随机环分布函数 RandomCircle2
    R = randint(r1, r2)
    theta = randint(0, 2 * 314) / 100.0
    x = AdvInt(R * cos(theta) + a)
    y = AdvInt(R * sin(theta) + b)
    return (x, y)

def RandCir3(a, b, r1, r2, theta1, theta2):      # 带缺口的随机环分布函数 RandomCircle3
    R = randint(r1, r2)
    Tmp = min(theta1, theta2)
    theta2 = max(theta1, theta2)
    theta1 = Tmp
    theta = randint(AdvInt(314 * theta1 / 180.0), AdvInt(314 * theta2 / 180.0)) / 100.0
    x = AdvInt(R * cos(theta) + a)
    y = AdvInt(R * sin(theta) + b)
    return (x, y)

def Cir(n, a, b, r):       # 圆函数
    CIR = []
    for i in range(n):
        theta = (n - i - 1) * 2 * pi / n
        x = AdvInt(r * cos(theta) + a)
        y = AdvInt(r * sin(theta) + b)
        CIR.append((x, y))
    return CIR

def Circle(n, a, b, r):       # 圆函数
    CIR = []
    for i in range(n):
        theta = (n - i - 1) * 2 * pi / n
        x = r * cos(theta) + a
        y = r * sin(theta) + b
        CIR.append((x, y))
    return CIR

def RandPolygon(r1, r2, v):
    s = '{\\p4}'
    vertex = []
    r1 *= 8
    r2 *= 8
    num = int(v)
    for i in range(num):
        vertex.append(RandomDouble(0, 2 * pi))
    vertex.sort()
    x = cos(vertex[0]) * r1
    y = sin(vertex[0]) * r2
    s += 'm {0} {1} '.format(AdvInt(x), AdvInt(y))
    for i in range(1, num):
        x = cos(vertex[i]) * r1
        y = sin(vertex[i]) * r2
        s += 'l {0} {1} '.format(AdvInt(x), AdvInt(y))
    s += 'c{\\p0}'
    return s

def DevImg(IMG, DEVX, DEVY):       # 图形偏移 DeviateIamge
    NEW_IMG_LIS = IMG.split()
    NEW_IMG = 'm ' + str(int(NEW_IMG_LIS[1]) + DEVX) + ' ' + str(int(NEW_IMG_LIS[2]) + DEVY) + ' b '
    n = len(NEW_IMG_LIS)
    for i in range(4, n - 1):
        if i % 2 == 0:
            NEW_IMG_LIS[i] = int(NEW_IMG_LIS[i]) + DEVX
            NEW_IMG += str(NEW_IMG_LIS[i]) + ' '
        else:
            NEW_IMG_LIS[i] = int(NEW_IMG_LIS[i]) + DEVY
            NEW_IMG += str(NEW_IMG_LIS[i]) + ' '
    return NEW_IMG + 'c'

def RevImgX(IMG):       # 图形按照x轴对换 ReverseIamgeX
    NEW_IMG_LIS = IMG.split()
    NEW_IMG = 'm ' + str(-int(NEW_IMG_LIS[1])) + ' ' + NEW_IMG_LIS[2] + ' b '
    n = len(NEW_IMG_LIS)
    for i in range(4, n - 1):
        if i % 2 == 0:
            NEW_IMG_LIS[i] = -int(NEW_IMG_LIS[i])
            NEW_IMG += str(NEW_IMG_LIS[i]) + ' '
        else:
            NEW_IMG += NEW_IMG_LIS[i] + ' '
    return NEW_IMG + 'c'

def RevImgY(IMG):       # 图形按照y轴对换 ReverseIamgeY
    NEW_IMG_LIS = IMG.split()
    NEW_IMG = 'm ' + NEW_IMG_LIS[1] + ' ' + str(-int(NEW_IMG_LIS[2])) + ' b '
    n = len(NEW_IMG_LIS)
    for i in range(4, n - 1):
        if i % 2 == 0:
            NEW_IMG += NEW_IMG_LIS[i] + ' '
        else:
            NEW_IMG_LIS[i] = -int(NEW_IMG_LIS[i])
            NEW_IMG += str(NEW_IMG_LIS[i]) + ' '
    return NEW_IMG + 'c'

def DivClr(COLOR1, COLOR2, n):       # 拆分颜色 DivideColor
    CLR_LIS_11 = HexToDec(COLOR1[0:2])
    CLR_LIS_12 = HexToDec(COLOR1[2:4])
    CLR_LIS_13 = HexToDec(COLOR1[4:6])
    CLR_LIS_21 = HexToDec(COLOR2[0:2])
    CLR_LIS_22 = HexToDec(COLOR2[2:4])
    CLR_LIS_23 = HexToDec(COLOR2[4:6])
    CLR_LIS = []
    if n == 2:
        return [COLOR1, COLOR2]
    else:
        for i in range(0, n - 1):
            CLR_LIS_I1 = FmtHex(CLR_LIS_11 + (CLR_LIS_21 - CLR_LIS_11) * i / (n - 1))
            CLR_LIS_I2 = FmtHex(CLR_LIS_12 + (CLR_LIS_22 - CLR_LIS_12) * i / (n - 1))
            CLR_LIS_I3 = FmtHex(CLR_LIS_13 + (CLR_LIS_23 - CLR_LIS_13) * i / (n - 1))
            CLR_LIS.append(CLR_LIS_I1 + CLR_LIS_I2 + CLR_LIS_I3)
        CLR_LIS.append(COLOR2)
        return CLR_LIS

def Jump(P_START, P_END, P_TOP, T):         # 跳跃特效函数
    JUMP = []
    d = P_TOP[0]
    for i in range(T):
        a = (P_START[1] - P_TOP[1]) / float((P_START[0] - d) * (P_START[0] - d))
        P_X = AdvInt(P_START[0] + i * (P_END[0] - P_START[0]) / float(T))
        P_Y = AdvInt(a * (P_X - d) * (P_X - d) + P_TOP[1])
        JUMP.append([P_X, P_Y])
    return JUMP

def AssDrawOffset(draw, x, y):      # 平移ASS绘图代码
    ret = ' '
    lastCmd = None
    elem = draw.split()
    num = len(elem)
    i = 0
    while i < num:
        if elem[i] == 'm':
            ret += 'm {} {} '.format(int(float(elem[i + 1]) + x + 0.5), int(float(elem[i + 2]) + y + 0.5))
            lastCmd = 'm'
            i += 2
        elif elem[i] == 'l':
            ret += 'l {} {} '.format(int(float(elem[i + 1]) + x + 0.5), int(float(elem[i + 2]) + y + 0.5))
            lastCmd = 'l'
            i += 2
        elif elem[i] == 'b':
            ret += 'b {} {} {} {} {} {} '.format(int(float(elem[i + 1]) + x + 0.5), int(float(elem[i + 2]) + y + 0.5), int(float(elem[i + 3]) + x + 0.5), int(float(elem[i + 4]) + y + 0.5), int(float(elem[i + 5]) + x + 0.5), int(float(elem[i + 6]) + y + 0.5))
            lastCmd = 'b'
            i += 6
        elif elem[i] == 'c':
            ret += 'c '
            lastCmd = 'c'
        elif lastCmd == 'l':
            i -= 1
            ret += '{} {} '.format(int(float(elem[i + 1]) + x + 0.5), int(float(elem[i + 2]) + y + 0.5))
            i += 2
        elif lastCmd == 'b':
            i -= 1
            ret += '{} {} {} {} {} {} '.format(int(float(elem[i + 1]) + x + 0.5), int(float(elem[i + 2]) + y + 0.5), int(float(elem[i + 3]) + x + 0.5), int(float(elem[i + 4]) + y + 0.5), int(float(elem[i + 5]) + x + 0.5), int(float(elem[i + 6]) + y + 0.5))
            i += 6
        i += 1
    return ret

#--------------------------------------------- PIX Function -------------------------------------------#

def PixPos(PIX, x, y):
    return ((x, y), PIX[1], PIX[2])

def PixPosShift(PIX, dx, dy):
    x = PIX[0][0]
    y = PIX[0][1]
    return ((x + dx, y + dy), PIX[1], PIX[2])

def PixFromPoints(points):
    pt_num = len(points)
    minX = points[0][0]
    minY = points[0][1]
    maxX = points[0][0]
    maxY = points[0][1]
    for i in range(pt_num):
        minX = min(minX, points[i][0])
        minY = min(minY, points[i][1])
        maxX = max(maxX, points[i][0])
        maxY = max(maxY, points[i][1])
    buf = []
    width = int(maxX - minX + 0.5) + 1
    height = int(maxY - minY + 0.5) + 1
    size = height * width * 4
    for i in range(size):
        buf.append(0)
    for i in range(pt_num):
        buf[(int(points[i][1] - minY + 0.5) * width + int(points[i][0] - minX + 0.5)) * 4 + 3] = points[i][2]
    return ((minX, minY), (width, height), tuple(buf))

def PixPointsV(PIX):
    width = PIX[1][0]
    height = PIX[1][1]
    points = []
    for w in range(width):
        for h in range(height):
            idx = 4 * (h * width + w)
            pixA = PIX[2][idx + 3]
            if pixA != 0:
                points.append((w, h, pixA))
    return points

def PixInvertA(PIX):
    buf = list(PIX[2])
    width = PIX[1][0]
    height = PIX[1][1]
    for h in range(height):
        for w in range(width):
            index = 4 * (h * width + w) + 3
            buf[index] = 255 - buf[index]
    return (PIX[0], PIX[1], tuple(buf))

def PixR2A(PIX):
    buf = []
    width = PIX[1][0]
    height = PIX[1][1]
    for h in range(height):
        for w in range(width):
            index = 4 * (h * width + w)
            buf.append(0)
            buf.append(0)
            buf.append(0)
            buf.append(PIX[2][index + 0])
    return (PIX[0], PIX[1], tuple(buf))

def PixA2RGB(PIX):
    buf = list(PIX[2])
    width = PIX[1][0]
    height = PIX[1][1]
    for h in range(height):
        for w in range(width):
            index = 4 * (h * width + w)
            buf[index] = buf[index + 3]
            buf[index + 1] = buf[index + 3]
            buf[index + 2] = buf[index + 3]
            buf[index + 3] = 255
    return (PIX[0], PIX[1], tuple(buf))

def PixAddA(PIX, step):
    buf = list(PIX[2])
    width = PIX[1][0]
    height = PIX[1][1]
    for h in range(height):
        for w in range(width):
            index = 4 * (h * width + w) + 3
            buf[index] += int(step + 0.5)
            if buf[index] < 0:
                buf[index] = 0
            if buf[index] > 255:
                buf[index] = 255
    return (PIX[0], PIX[1], tuple(buf))

def PixSwitchRB(PIX):
    buf = list(PIX[2])
    width = PIX[1][0]
    height = PIX[1][1]
    for h in range(height):
        for w in range(width):
            index = 4 * (h * width + w)
            temp = buf[index]
            buf[index] = buf[index + 2]
            buf[index + 2] = temp
    return (PIX[0], PIX[1], tuple(buf))

#############################################################################################################




