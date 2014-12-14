# Version: initial version

from tcaxPy import *


def tcaxPy_Init():

    # some common pre-defined global values

    global fontSize    # as name implies
    global resX        # horizontal resolution
    global resY        # vertical resolution
    global marginX     # horizontal margin
    global marginY     # vertical margin
    global spacing     # space between texts
    global frameDur    # milliseconds per frame
    global lineNum     # number of lines
    global textNum     # textNum[i], number of texts in ith line
    global start       # start[i], start time of a line
    global end         # end[i], end time of a line
    global kar         # kar[i][j], karaoke time of a syllable
    global elapKar     # elapKar[i][j], elapsed karaoke time before reaching a certain syllable
    global text        # text[i][j], as name implies
    global textLength  # textLength[i], total width of a line
    global width       # width[i][j], width of a text
    global height      # height[i][j], height of a text
    global advance     # advance[i][j], advance of a text, usually larger than width
    global advDiff     # advDiff[i][j], distance between the current text to the first text of the line
    fontSize     = GetVal(val_FontSize)
    resX         = GetVal(val_ResolutionX)
    resY         = GetVal(val_ResolutionY)
    marginX      = GetVal(val_OffsetX)
    marginY      = GetVal(val_OffsetY)
    spacing      = GetVal(val_Spacing)
    frameDur     = 1000 / GetVal(val_FXFPS)
    lineNum      = GetVal(val_nLines)
    textNum      = GetVal(val_nTexts)
    start        = GetVal(val_BegTime)
    end          = GetVal(val_EndTime)
    kar          = GetVal(val_KarTime)
    elapKar      = GetVal(val_KarTimeDiff)
    text         = GetVal(val_Text)
    textLength   = GetVal(val_TextLength)
    width        = GetVal(val_TextWidth)
    height       = GetVal(val_TextHeight)
    advance      = GetVal(val_TextAdvance)
    advDiff      = GetVal(val_TextAdvanceDiff)

    # some user-defined global values

    global font
    global fontBord
    font     = InitFont(GetVal(val_FontFileName), GetVal(val_FaceID), fontSize, GetVal(val_Spacing), GetVal(val_SpaceScale), MakeRGB(255, 255, 255), 0, 0)
    fontBord = InitFont(GetVal(val_FontFileName), GetVal(val_FaceID), fontSize, GetVal(val_Spacing), GetVal(val_SpaceScale), MakeRGB(0, 0, 0), 2, 1)


def tcaxPy_Fin():
    return


def tcaxPy_User():
    ASS = CreateAssFile(GetVal(val_OutFile) + '.ass', GetVal(val_AssHeader))

    for i in range(lineNum):
        initPosX = (resX - textLength[i]) / 2 + marginX        # if marginX = 0, then it's just on the middle
        initPosY = marginY
        for j in range(textNum[i]):
            ASS_BUF = []    # you can put the BUF anywhere according to your usage
            if text[i][j] == '' or text[i][j] == ' ' or text[i][j] == '　':
                continue
            posX = initPosX + advDiff[i][j] + advance[i][j] / 2
            posY = initPosY
            ass_main(ASS_BUF, SubL(start[i], end[i]), an(8) + pos(posX, posY) + K(elapKar[i][j]) + K(kar[i][j]), text[i][j])
            WriteAssFile(ASS, ASS_BUF)
            Progress(i, j)

    FinAssFile(ASS)





