from tcaxPy import *


def tcaxPy_Init():
    print('Tips: you can initialize your global variables here.')
    print('This function will be executed once if you set `< tcaxpy init = true >\'')
    print(GetHelp())


def tcaxPy_User():
    print('User defined function.')
    print('This function will be executed once if you set `< tcaxpy user = true >\'')
    print('Otherwise, tcaxPy_Main will be executed (many times)')


def tcaxPy_Fin():
    print('Tips: you can finalize your global variables here.')
    print('This function will be executed once if you set `< tcaxpy fin = true >\'')
    print('Note: you do not need to finalize the global variables got from function GetVal()')


def tcaxPy_Main(_i, _j, _n, _start, _end, _elapk, _k, _x, _y, _a, _txt):

    ASS_BUF  = []        # used for saving ASS FX lines
    TCAS_BUF = []        # used for saving TCAS FX raw data

    #############################
    # TODO: write your codes here #

    ass_main(ASS_BUF, SubL(_start, _end), pos(_x, _y) + K(_elapk) + K(_k), _txt)

    #############################

    return (ASS_BUF, TCAS_BUF)





