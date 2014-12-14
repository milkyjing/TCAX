from tcaxPy import *
from util.cairo import *

# 初始化cairo
def cairo_init(width, height):
    global surface
    global ctx
    surface = ImageSurface(FORMAT_ARGB32, width, height)
    ctx = Context(surface)
    # 增加额外的初始化代码
    ctx.set_line_width(1)
    ctx.set_source_rgba(1, 1, 1, 1)

# ASS绘图代码转PIX
def draw2pix(draw):
    # 自己定制绘制效果
    offset_x = surface.get_width() // 2
    offset_y = surface.get_height() // 2
    ctx.translate(offset_x, offset_y)
    #ctx.scale(2, 2)
    AssDraw(ctx, draw)
    ctx.fill_preserve()
    ctx.stroke()
    #ctx.scale(1/2, 1/2)
    ctx.translate(-offset_x, -offset_y)
    # 获取PIX, 并清空画布
    PIX = surface.get_pix()
    surface_clear(ctx)
    return PIX


