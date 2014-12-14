# -*- coding: utf-8 -*-


########################################## Declaration ######################################################

# Declaration:
# tcxPy.pyc's source code
# Copyright (c) 2008 - 2010 milkyjing (milkyjing@gmail.com). All rights reserved.
# Please visit www.tcsub.com to get the latest information
# Version: ver0.7.3.5 build20100109
# Revision: ver0.7.3.8 build20100904

######################################### Modules ###########################################################

from math   import *
from random import *
#from tcxLib import *

######################################### Global Constant ###################################################

Main_Style          = 'TCMS'	# 主Style名称 SubL函数默认使用的Style
Pix_Style           = 'TCPS'	# 粒子特效的Style名称

val_AssHeader       = 0		# Ass文件头部信息 用于tcxPy_User函数
val_OutFile         = 1		# 输出的文件名 不含拓展名 用于tcxPy_User函数

# tcc info
val_FontFileName    = 2		# 特效使用的字体
val_FaceID          = 3		# 字体名称ID
val_FontSize        = 4		# 字体大小
val_ResolutionX     = 5		# 水平分辨率
val_ResolutionY     = 6		# 垂直分辨率
val_FXFPS           = 7		# 特效的帧率
val_Alignment       = 8		# 对齐方式
val_OffsetX         = 9		# 水平偏移量
val_OffsetY         = 10	# 垂直偏移量
val_Spacing         = 11	# 字体间距
val_SpaceScale      = 12	# 空格跨度百分比
val_Bord            = 13	# 字体边框厚度
val_Shad            = 14	# 字体阴影宽度
val_1C              = 15	# 字体主颜色
val_2C              = 16	# 字体辅助颜色
val_3C              = 17	# 字体边框颜色
val_4C              = 18	# 字体阴影颜色
val_1A              = 19	# 字体主透明度
val_2A              = 20	# 字体辅助透明度
val_3A              = 21	# 字体边框透明度
val_4A              = 22	# 字体阴影透明度
val_Blur            = 23	# 模糊值

# syl info
val_nLines          = 24	# SYL文件的卡拉OK歌词句子总数
val_SylLine         = 25	# 第i句卡拉OK歌词
val_BegTime         = 26	# 第i句卡拉OK歌词的起始时间
val_EndTime         = 27	# 第i句卡拉OK歌词的结束时间
val_nTexts          = 28	# 第i句卡拉OK歌词包含的字符数
val_KarTime         = 29	# 第i句卡拉OK歌词的第j个字符的Karaoke时间
val_KarTimeDiff     = 30	# 到第i句卡拉OK歌词的第j个字符之前的Karaoke时间总和
val_Text            = 31	# 第i句卡拉OK歌词的第j个字符

# tm info
val_Ascender        = 32	# 字体上行高度
val_Descender       = 33	# 字体下行高度 通常为负数
val_TextWidth       = 34	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeight      = 35	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerning     = 36	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0
val_TextAdvance     = 37	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiff = 38	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLength      = 39	# 第i句卡拉OK歌词的文字总长度
val_TextInitX       = 40	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitY       = 41	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingY    = 42	# 第i句卡拉OK歌词的j个字符的Y轴bearing

######################################### Data Management Function ##########################################

def tcxGetValueIDsInfo():
    str_val_info = '''

# Version: ver0.7.3.5 build20100109
# Revision: ver0.7.3.8 build20100904

Main_Style          = 'TCMS'	# 主Style名称 SubL函数默认使用的Style
Pix_Style           = 'TCPS'	# 粒子特效的Style名称

val_AssHeader       = 0		# Ass文件头部信息 用于tcxPy_User函数
val_OutFile         = 1		# 输出的文件名 不含拓展名 用于tcxPy_User函数

# tcc info
val_FontFileName    = 2		# 特效使用的字体
val_FaceID          = 3		# 字体名称ID
val_FontSize        = 4		# 字体大小
val_ResolutionX     = 5		# 水平分辨率
val_ResolutionY     = 6		# 垂直分辨率
val_TimePerFrame    = 7		# 每一帧的时间
val_Alignment       = 8		# 对齐方式
val_OffsetX         = 9		# 水平偏移量
val_OffsetY         = 10	# 垂直偏移量
val_Spacing         = 11	# 字体间距
val_SpaceScale      = 12	# 空格跨度百分比
val_Bord            = 13	# 字体边框厚度
val_Shad            = 14	# 字体阴影宽度
val_1C              = 15	# 字体主颜色
val_2C              = 16	# 字体辅助颜色
val_3C              = 17	# 字体边框颜色
val_4C              = 18	# 字体阴影颜色
val_1A              = 19	# 字体主透明度
val_2A              = 20	# 字体辅助透明度
val_3A              = 21	# 字体边框透明度
val_4A              = 22	# 字体阴影透明度
val_Blur            = 23	# 模糊值

# syl info
val_nLines          = 24	# SYL文件的卡拉OK歌词句子总数
val_SylLine         = 25	# 第i句卡拉OK歌词
val_BegTime         = 26	# 第i句卡拉OK歌词的起始时间
val_EndTime         = 27	# 第i句卡拉OK歌词的结束时间
val_nTexts          = 28	# 第i句卡拉OK歌词包含的字符数
val_KarTime         = 29	# 第i句卡拉OK歌词的第j个字符的Karaoke时间
val_KarTimeDiff     = 30	# 到第i句卡拉OK歌词的第j个字符之前的Karaoke时间总和
val_Text            = 31	# 第i句卡拉OK歌词的第j个字符

# tm info
val_Ascender        = 32	# 字体上行高度
val_Descender       = 33	# 字体下行高度 通常为负数
val_TextWidth       = 34	# 第i句卡拉OK歌词的j个字符的宽度
val_TextHeight      = 35	# 第i句卡拉OK歌词的j个字符的高度
val_TextKerning     = 36	# 第i句卡拉OK歌词的j个字符与之前一个字符的kerning 一句歌词第一个字符的kerning为0
val_TextAdvance     = 37	# 第i句卡拉OK歌词的j个字符的水平步距
val_TextAdvanceDiff = 38	# 从初始位置到第i句卡拉OK歌词的j个字符的水平步距
val_TextLength      = 39	# 第i句卡拉OK歌词的文字总长度
val_TextInitX       = 40	# 第i句卡拉OK歌词的j个字符的左上角X轴坐标
val_TextInitY       = 41	# 第i句卡拉OK歌词的j个字符的左上角Y轴坐标
val_TextBearingY    = 42	# 第i句卡拉OK歌词的j个字符的Y轴bearing

'''
    return str_val_info

def tcxInitData(data):
    global __tcx_data
    __tcx_data = data

def tcxGetData():
    return __tcx_data

def GetVal(item):
    return __tcx_data[item]

def getval_kartxt(i):
    return __tcx_data[val_SylLine][i].split(',,', 1)[1]

######################################### Useful Funtions ###################################################

#--------------------------------------------- Basic Function ----------------------------------------------#

# 格式化相关的函数

def FmtHex(n):                                   # dec to hex FormatHex
    if n <= 0:
        return '00'
    elif n >= 255:
        return 'FF'
    else:
        FMT_HEX = hex(int(n)).replace('0x', '').rjust(2, '0').upper()
        return FMT_HEX

def FmtDec(a):                                   # hex to dec FormatDec
    FMT_DEC = int(a, 16)
    return FMT_DEC

def FmtFlt(f):
    if f == int(f):
        F = str(int(f))
    else:
        F = str(format(f, '.2f'))
    return F

def FmtRGB(r, g, b):                             # return a formated RGB string
    RGB = FmtHex(b) + FmtHex(g) + FmtHex(r)
    return RGB

def DeFmtRGB(CLR):                               # convert RGB string to RGB tuple
    CLR_LIS_3 = FmtDec(CLR[0:2])
    CLR_LIS_2 = FmtDec(CLR[2:4])
    CLR_LIS_1 = FmtDec(CLR[4:6])
    CLR = (CLR_LIS_1, CLR_LIS_2, CLR_LIS_3)
    return CLR

def DecRGB(RGB):                                 # convert RGB string to RGB dec value
    Clr = int('0x' + RGB, 16)
    return Clr

def MakeRGB(r, g, b):
    if r < 0:
       r = 0
    elif r > 255:
       r = 255
    if g < 0:
       g = 0
    elif g > 255:
       g = 255
    if b < 0:
       b = 0
    elif b > 255:
       b = 255
    return r + g * 256 + b * 65536

def MakeRGBA(r, g, b, a):
    if r < 0:
       r = 0
    elif r > 255:
       r = 255
    if g < 0:
       g = 0
    elif g > 255:
       g = 255
    if b < 0:
       b = 0
    elif b > 255:
       b = 255
    if a < 0:
       a = 0
    elif a > 255:
       a = 255
    return r + g * 256 + b * 65536 + a * 16777216

#-------------------------------- Ass tag functions -------------------------------#

def an(a):
    A = '\\an' + str(int(a))
    return A

def K(a):
    Kar = '\\K' + str(int(a))
    return Kar

def k(a):
    kar = '\\k' + str(int(a))
    return kar

def ko(a):
    Ko = '\\ko' + str(int(a))
    return Ko

def t(code):
    T = '\\t(,' + code + ')'
    return T

def t1(t1, t2, code):
    T1 = '\\t(' + str(int(t1)) + ',' + str(int(t2)) + ',' + code + ')'
    return T1

def t2(t1, t2, a, code):
    if a == int(a):
        A = str(int(a))
    else:
        A = str(format(a, '.2f'))
    T2 = '\\t(' + str(int(t1)) + ',' + str(int(t2)) + ',' + A + ',' + code + ')'
    return T2

def fscx(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    FSCX = '\\fscx' + X
    return FSCX

def fscy(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    FSCY = '\\fscy' + Y
    return FSCY

def fsc(x, y):
    if x == int(x) and y == int(y):
        X = str(int(x))
        Y = str(int(y))
    else:
        X = str(format(x, '.2f'))
        Y = str(format(y, '.2f'))
    FSC = '\\fscx' + X + '\\fscy' + Y
    return FSC

def fs(x):
    FS = '\\fs' + str(int(x))
    return FS

def fad(t1, t2):
    FAD = '\\fad(' + str(int(t1)) + ',' + str(int(t2)) + ')'
    return FAD

def bord(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    BORD = '\\bord' + X
    return BORD

def shad(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    SHAD = '\\shad' + X
    return SHAD

def blur(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    BLUR = '\\blur' + X
    return BLUR

def be(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    BE = '\\be' + X
    return BE

def xbord(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    XBORD = '\\xbord' + X
    return XBORD

def ybord(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    YBORD = '\\ybord' + Y
    return YBORD

def xshad(x):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    XSHAD = '\\xshad' + X
    return XSHAD

def yshad(y):
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    YSHAD = '\\yshad' + Y
    return YSHAD

def fax(x):
    FAX = '\\fax' + str(int(x))
    return FAX

def fay(y):
    FAY = '\\fay' + str(int(y))
    return FAY

def frx(a):
    FRX = '\\frx' + str(int(a))
    return FRX

def fry(a):
    FRY = '\\fry' + str(int(a))
    return FRY

def frz(a):
    FRZ = '\\frz' + str(int(a))
    return FRZ

def pos(x, y):
    if x == int(x):
        X = str(int(x))
    else:
        X = str(format(x, '.2f'))
    if y == int(y):
        Y = str(int(y))
    else:
        Y = str(format(y, '.2f'))
    POS = '\\pos(' + X + ',' + Y + ')'
    return POS

def org(x, y):
    ORG = '\\org(' + str(int(x)) + ',' + str(int(y)) + ')'
    return ORG

def alpha(a):                                    # note that a is a dec value
    ALPHA = '\\alpha&H' + FmtHex(a) + '&'
    return ALPHA

def alpha1(a):
    ALPHA1 = '\\1a&H' + FmtHex(a) + '&'
    return ALPHA1

def alpha2(a):
    ALPHA2 = '\\2a&H' + FmtHex(a) + '&'
    return ALPHA2

def alpha3(a):
    ALPHA3 = '\\3a&H' + FmtHex(a) + '&'
    return ALPHA3

def alpha4(a):
    ALPHA4 = '\\4a&H' + FmtHex(a) + '&'
    return ALPHA4

def color(c):                                    # note that c is a RGB string
    COLOR = '\\c&H' + c + '&'
    return COLOR

def color1(c):
    COLOR1 = '\\1c&H' + c + '&'
    return COLOR1

def color2(c):
    COLOR2 = '\\2c&H' + c + '&'
    return COLOR2

def color3(c):
    COLOR3 = '\\3c&H' + c + '&'
    return COLOR3

def color4(c):
    COLOR4 = '\\4c&H' + c + '&'
    return COLOR4

def mov(x1, y1, x2, y2):
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
    MOV = '\\move(' + X1 + ',' + Y1 + ',' + X2 + ',' + Y2 + ')'
    return MOV

def move(x1, y1, x2, y2, t1, t2):
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
    MOVE = '\\move(' + X1 + ',' + Y1 + ',' + X2 + ',' + Y2 + ',' + str(int(t1)) + ',' + str(int(t2)) + ')'
    return MOVE

def clip(x1, y1, x2, y2):
    CLIP = '\\clip(' + str(int(x1)) + ',' + str(int(y1)) + ',' + str(int(x2)) + ',' + str(int(y2)) + ')'
    return CLIP

def clip1(Draw):
    CLIP = '\\clip(' + Draw + ')'
    return CLIP

def clip2(Scale, Draw):
    CLIP = '\\clip(' + str(Scale) + ',' + Draw + ')'
    return CLIP

def iclip(x1, y1, x2, y2):
    ICLIP = '\\iclip(' + str(int(x1)) + ',' + str(int(y1)) + ',' + str(int(x2)) + ',' + str(int(y2)) + ')'
    return ICLIP

def iclip1(Draw):
    ICLIP = '\\iclip(' + Draw + ')'
    return ICLIP

def iclip2(Scale, Draw):
    ICLIP = '\\iclip(' + str(Scale) + ',' + Draw + ')'
    return ICLIP

#--------------------------------------------- Advanced Function -------------------------------------------#

def MakePath(FolderIndex = 0, ImageIndex = 0, MainFolder = 'src', SubFolder = 'list', ImageName = 'img', ImageType = '.png', PathType = 'pi'):
    if PathType == 'pi':
        img_path = '%s\%s%d\%s%04d%s' % (MainFolder, SubFolder, FolderIndex, ImageName, ImageIndex, ImageType)
    elif PathType == 'sys':
        img_path = '%s\%s%d\%s (%d)%s' % (MainFolder, SubFolder, FolderIndex, ImageName, ImageIndex, ImageType)
    else:
        img_path = '%s\%s%d\%s%04d%s' % (MainFolder, SubFolder, FolderIndex, ImageName, ImageIndex, ImageType)
    return img_path

def tcxLog(info):
    s = str(info) + '\r\n'
    logfile = open(GetVal(val_OutFile) + '_data.log', 'ab')
    logfile.write(b'\xef\xbb\xbf')             # codecs.BOM_UTF8
    logfile.write(s.encode('utf-8'))
    logfile.close()

def UserShowProgress(i, j, type = 1):
    if type == 1:
        nTotal = 0
        for l in range(GetVal(val_nLines)):
            nTotal += GetVal(val_nTexts)[l]
        nCompleted = 0
        for l in range(i):
            nCompleted += GetVal(val_nTexts)[l]
        nCompleted += j + 1
        tcxPy_UserShowProgress(nCompleted, nTotal)
    else:
        nTotal = 0
        for l in range(GetVal(val_nLines)):
            nTotal += GetVal(val_nWords)[l]
        nCompleted = 0
        for l in range(i):
            nCompleted += GetVal(val_nWords)[l]
        nCompleted += j + 1
        tcxPy_UserShowProgress(nCompleted, nTotal)

def AssBufToBytes(ASS_BUF):
    return AssBufToStr(ASS_BUF).encode('utf-8')

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
    t = int(TIME[0:1]) * 60 * 60 * 100 + int(TIME[2:4]) * 60 * 100 + int(TIME[5:7]) * 100 + int(TIME[8:10])
    return t

def FmtTime(t):       # 格式化时间
    Hour = int(t) / 360000
    Minute = (int(t) / 6000) % 60
    Second = (int(t) / 100) % 60
    SemiSecond = int(t) % 100
    HOUR = str(int(Hour))
    MINUTE = str(int(Minute))
    SECOND = str(int(Second))
    SEMISECOND = str(int(SemiSecond))
    TIME = HOUR + ':' + MINUTE.rjust(2, '0') + ':' + SECOND.rjust(2, '0') + '.' + SEMISECOND.rjust(2, '0')
    return TIME

def PixPt():       # 返回一个像素点 PixelPoint
    PIX_P = '{\\p1}m 0 0 l 1 0 1 1 0 1{\\p0}'
    return PIX_P 

def MovPxl():       # 移动单位个像素 MovePixel
    if randint(0, 1) == 0:
        return -1
    else:
        return 1

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

def RandRGB():       # 返回一个随机的RGB值 RandomRGB
    RAND_RGB = FmtRGB(randint(0, 255), randint(0, 255), randint(0, 255))
    return RAND_RGB

def RandA():       # 返回一个随机的Alpha值 RandomAlpha
    RAND_A = FmtHex(randint(0, 255))
    return RAND_A

def RandImg(n, IMG_WD, IMG_HT):       # 随机图形 RandomImage
    STR_RAND_IMG = 'm ' + str(randint(0, IMG_WD)) + ' ' + str(randint(0, IMG_HT)) + ' b '
    for i in range(6 * n):
        if i % 2 == 0:
            STR_RAND_IMG += str(randint(0, IMG_WD)) + ' '
        else:
            STR_RAND_IMG += str(randint(0, IMG_HT)) + ' '
    return STR_RAND_IMG + 'c'

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
    CLR_LIS_11 = FmtDec(COLOR1[0:2])
    CLR_LIS_12 = FmtDec(COLOR1[2:4])
    CLR_LIS_13 = FmtDec(COLOR1[4:6])
    CLR_LIS_21 = FmtDec(COLOR2[0:2])
    CLR_LIS_22 = FmtDec(COLOR2[2:4])
    CLR_LIS_23 = FmtDec(COLOR2[4:6])
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

def MovToPos(x1, y1, x2, y2, t1, t2):       # 把一个点的移动分解成一系列点的出现、消失 MoveToPos
    POS_LIS = []
    n = max(abs(x2 - x1), abs(y2 - y1))
    x1 = float(x1)
    y1 = float(y1)
    x2 = float(x2)
    y2 = float(y2)
    t1 = float(t1)
    t2 = float(t2)
    for i in range(1, n + 1):
        x = AdvInt(x1 + (x2 - x1) * i / n)
        y = AdvInt(y1 + (y2 - y1) * i / n)
        t = AdvInt(t1 + (t2 - t1) * i / n)
        POS_LIS.append([x, y, t])
    return POS_LIS

def RectClip(InitPosX, InitPosY, WD, HT, AN = 7, DENS = 1.0, SIZE = 1):       # 用点或方格的形式切割文字
    RCLIP = []
    for i in range(AdvInt(DENS * WD)):
        for j in range(AdvInt(DENS * HT)):
            RCLIP.append(clip(InitPosX + AdvInt(i / DENS), InitPosY + AdvInt(j / DENS), InitPosX + AdvInt(i / DENS) + SIZE, InitPosY + AdvInt(j / DENS) + SIZE))
    return RCLIP

def CirClip(a, b, r, d):      # 将字体以圆形形式切割
    NBR = []
    CLIP = []
    N = 0
    for i in range(2, r, d):      # 切割成环的次数以半径来计算
        n = 2 * 3 * i
        CIR = Cir(n, a, b, i)
        N += 1
        NBR.append(n)
        for j in range(len(CIR)):
            CLIP.append([CIR[j][0] - d, CIR[j][1] - d, CIR[j][0], CIR[j][1]])
    CIR_CLIP = (N, NBR, CLIP)
    return CIR_CLIP

def RandClip(Scale, n, IMG_WD, IMG_HT):     # 任意形状切割
    RAND_CLIP = clip2(Scale, RandImg(n, IMG_WD, IMG_HT))
    return RAND_CLIP

def Jump(P_START, P_END, P_TOP, T):         # 跳跃特效函数
    JUMP = []
    d = P_TOP[0]
    for i in range(T):
        a = (P_START[1] - P_TOP[1]) / float((P_START[0] - d) * (P_START[0] - d))
        P_X = AdvInt(P_START[0] + i * (P_END[0] - P_START[0]) / float(T))
        P_Y = AdvInt(a * (P_X - d) * (P_X - d) + P_TOP[1])
        JUMP.append([P_X, P_Y])
    return JUMP

#############################################################################################################


########################################## Main FX Function #################################################

def MainL(SubDlg, Event = '', Text = ''):    # deprecated in order to make it compatible with the old versions
    Dlg = SubDlg + '{' + Event + '}' + Text + '\r\n'
    return Dlg

def MainP(Start = 0, End = 0, Pos = (0, 0), RGB = 'FFFFFF', Alpha = 0):    # deprecated
    x = Pos[0]
    y = Pos[1]
    if x < 0:
        x = 65535
    if y < 0:
        y = 65535
    a = int(Alpha)
    if a < 0:
        a = 0
    elif a > 255:
        a = 255
    return (int(Start), int(End), int(x), int(y), int('0x' + RGB, 16), a)


def SubL(Start = 0, End = 0, Layer = 0, Style = 'TCMS'):
    SubDlg = 'Dialogue: ' + str(int(Layer)) + ',' + FmtTime(Start) + ',' + FmtTime(End) + ',*' + Style + ',NTP,0000,0000,0000,,'
    return SubDlg

def ass_main(ASS_BUF, SubDlg = '', Event = '', Text = ''):
    ASS_BUF.append(SubDlg + '{' + Event + '}' + Text + '\r\n')

def tcs_user(TCS_BUF, Start = 0, End = 0, PosX = 0, PosY = 0, RGB = 0xFFFFFF, Alpha = 0, Layer = 0):
    if Alpha < 0:
        Alpha = 0
    TCS_BUF.append((int(Start), int(End), int(Layer), int(PosX), int(PosY), int(RGB), int(Alpha)))

#############################################################################################################



