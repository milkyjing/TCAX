# constraints on the draw string:
# 1. should start with the ASS draw command 'm'
# 2. can only contains 'm' 'l' 'b' 'c'
# 3. cannot contain scale code

def AssDraw(ctx, draw):
    first = True
    lastCmd = None
    elem = draw.split()
    num = len(elem)
    i = 0
    while i < num:
        if elem[i] == 'm':
            if first:
                ctx.new_path()
                first = False
            ctx.move_to(int(elem[i + 1]), int(elem[i + 2]))
            lastCmd = 'm'
            i += 2
        elif elem[i] == 'l':
            ctx.line_to(int(elem[i + 1]), int(elem[i + 2]))
            lastCmd = 'l'
            i += 2
        elif elem[i] == 'b':
            ctx.curve_to(int(elem[i + 1]), int(elem[i + 2]), int(elem[i + 3]), int(elem[i + 4]), int(elem[i + 5]), int(elem[i + 6]))
            lastCmd = 'b'
            i += 6
        elif elem[i] == 'c':
            ctx.close_path()
            lastCmd = 'c'
        elif lastCmd == 'l':
            i -= 1
            ctx.line_to(int(elem[i + 1]), int(elem[i + 2]))
            i += 2
        elif lastCmd == 'b':
            i -= 1
            ctx.curve_to(int(elem[i + 1]), int(elem[i + 2]), int(elem[i + 3]), int(elem[i + 4]), int(elem[i + 5]), int(elem[i + 6]))
            i += 6
        i += 1


from ._cairo import *
from math import *

def surface_clear(ctx):
    ctx.save()
    ctx.set_source_rgba(0, 0, 0, 0)
    ctx.set_operator(OPERATOR_SOURCE)
    ctx.paint()
    ctx.restore()

def path_trans(ctx, func, param):
    first = True
    for type, points in ctx.copy_path():
        if type == PATH_MOVE_TO:
            if first:
                ctx.new_path()
                first = False
            x, y = func(points[0], points[1], param)
            ctx.move_to(x, y)
        elif type == PATH_LINE_TO:
            x, y = func(points[0], points[1], param)
            ctx.line_to(x, y)
        elif type == PATH_CURVE_TO:
            x1, y1, x2, y2, x3, y3 = points
            x1, y1 = func(x1, y1, param)
            x2, y2 = func(x2, y2, param)
            x3, y3 = func(x3, y3, param)
            ctx.curve_to(x1, y1, x2, y2, x3, y3)
        elif type == PATH_CLOSE_PATH:
            ctx.close_path()

def ca_frz(x, y, a):
    x2 = x * cos(a) + y * sin(a)
    y2 = x * sin(a) + y * cos(a)
    return x2, y2

def ca_fry(x, y, a):
    x2 = x * cos(a)
    y2 = y
    return x2, y2

def ca_frx(x, y, a):
    x2 = x
    y2 = y * cos(a)
    return x2, y2

def ToAssDraw(ctx):
    assDraw = ''
    lastCmd = None
    for type, points in ctx.copy_path():
        if type == PATH_MOVE_TO:
            if lastCmd == 'm':
                assDraw += '{0} {1} '.format(int(points[0] + 0.5), int(points[1] + 0.5))
            else:
                assDraw += 'm {0} {1} '.format(int(points[0] + 0.5), int(points[1] + 0.5))
                lastCmd = 'm'
        elif type == PATH_LINE_TO:
            if lastCmd == 'l':
                assDraw += '{0} {1} '.format(int(points[0] + 0.5), int(points[1] + 0.5))
            else:
                assDraw += 'l {0} {1} '.format(int(points[0] + 0.5), int(points[1] + 0.5))
                lastCmd = 'l'
        elif type == PATH_CURVE_TO:
            if lastCmd == 'b':
                assDraw += '{0} {1} {2} {3} {4} {5} '.format(int(points[0] + 0.5), int(points[1] + 0.5), int(points[2] + 0.5), int(points[3] + 0.5), int(points[4] + 0.5), int(points[5] + 0.5))
            else:
                assDraw += 'b {0} {1} {2} {3} {4} {5} '.format(int(points[0] + 0.5), int(points[1] + 0.5), int(points[2] + 0.5), int(points[3] + 0.5), int(points[4] + 0.5), int(points[5] + 0.5))
                lastCmd = 'b'
        elif type == PATH_CLOSE_PATH:
            assDraw += 'c '
            lastCmd = 'c'
    return assDraw.rsplit('m', 1)[0].rstrip()

