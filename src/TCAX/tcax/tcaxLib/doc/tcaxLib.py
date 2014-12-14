## @package tcaxLib
# tcaxLib documentation
#
# API references and usage examples of tcaxLib.pyd


## @struct PIX
# The basic element of a TCAS FX, it contains dozens of DIPs (dynamic isolated pixels). 
# It is a tuple that has the following structure ((initX, initY), (width, height), (r1, g1, b1, a1, r2, g2, b2, a2, r3, ...)).
#
# initX a float, offset of the PIX in the horizontal direction
#
# initY a float, offset of the PIX in the vertical direction
#
# width an integer, width of the PIX
#
# height an integer, height of the PIX
#
# rgba all integers, rgba colors of the PIX
#
# Remarks: Although DIP is the basic unit of TCAS files, it is not easy to handle when come to make TCAS FX. 
# So in TCAS FX, PIX is the one which is regarded as the basic unit and most tcaxLib APIs are used to manipulate PIXs. 
# However DIP can still be used when needed. Most tcaxLib APIs receive PIX as an input parameter and return a new PIX as the output result.
# Note that, you should be definitely sure if you are going to construct your own PIX object through simple assignment operation other than 
# APIs provided by tcaxLib, for instance, PIX = ((initX, initY), (width, height), (r1, g1, b1, a1, r2, g2, b2, a2, r3, ...)), any mistake may 
# lead to the crash of the host program.


## Get the current tcaxLib version.
# @return unicode string the current version
def tcaxLibGetVersion():


# pix.h

## Get points from PIX.
# @param pix a PIX, the source PIX
# @return ((x1, y1, a1), (x2, y2, a2), ...)
def PixPoints(pix):


## Create a blank PIX.
# @param width an integer, target width of the PIX
# @param height an integer, target height of the PIX
# @param rgba an integer, target rgba of the PIX, usually in hexadecimal
# @return PIX the target PIX
def BlankPix(width, height, rgba):


## Resize a PIX.
# @param pix a PIX, the source PIX that is going to be resized
# @param width an integer, target width
# @param height an integer, target height
# @return PIX the resized PIX
def PixResize(pix, width, height):

## Resize a PIX, the same as PixResize but accept fractional width and height
# @param pix a PIX, the source PIX that is going to be resized
# @param width a float, target width
# @param height a float, target height
# @return PIX the resized PIX
def PixResizeF(pix, width, height):


## Enlarge a PIX.
# @param pix a PIX, the source PIX that is going to be enlarged
# @param x an integer, the value that is increased in the horizontal direction
# @param y an integer, the value that is increased in the vertical direction
# @return PIX the target PIX
def PixEnlarge(pix, x, y):


## Crop a PIX.
# @param pix a PIX, the source PIX that is going to be cropped
# @param left an integer, offset from the left side
# @param top an integer, offset from the top
# @param right an integer, offset from the left side, or when it is negative, offset from the right side
# @param bottom an integer, offset from the top, or when it is negative, offset from the bottom
# @return PIX the target PIX
def PixCrop(pix, left, top, right, bottom):


## Strip the blank border of a PIX.
# @param pix a PIX, the source PIX whose blank border if any is going to be stripped out
# @return PIX the target PIX
def PixStrip(pix):


## Apply a texture on a PIX.
# Remark: the texture will be resized to fit the target pix, and the positions will be just ignored.
# @param pix a PIX, the source PIX that is going to be overlayed with the texture
# @param texture a PIX, the texture PIX
# @return PIX the target PIX
def PixTexture(pix, texture):


## Apply a mask to a PIX.
# Remark: unlike the PixTexture() function, the positions will be used.
# @param pix a PIX, the source PIX that is going to be masked with the mask
# @param mask a PIX, the mask PIX
# @return PIX the target PIX
def PixMask(pix, mask):


## Replace the alpha channel of the source PIX with the alpha channel of the destination PIX. 
# Remark: the transparent pixels of the source PIX will stay unchanged.
# @param pix a PIX, the source PIX whose alpha is going to be replaced by the alpha of the alpha PIX
# @param alpha a PIX, the alpha PIX
# @return PIX the target PIX
def PixReplaceAlpha(pix, alpha):


## Blur a PIX using Gauss IIR blur.
# @param pix a PIX, the source PIX that is going to be blurred
# @param radius an integer, radius of the Gauss kernel
# @return PIX the target PIX
def PixBlur(pix, radius):


## Combine two PIXs.
# @param back a PIX, the back PIX
# @param overlay a PIX, the overlay PIX
# @return PIX the target PIX
def CombinePixs(back, overlay):


## Change the color of a PIX by multiplying factors to each channel of RGBA. The value of each channel will be clipped into [0, 255]
# @param pix a PIX, the source PIX
# @param r_f a float, the red channel factor, if r_f == 1, the red channel will stay the same
# @param g_f a float, the green channel factor, if g_f == 1, the green channel will stay the same
# @param b_f a float, the blue channel factor, if b_f == 1, the blue channel will stay the same
# @param a_f a float, the alpha channel factor, if a_f == 1, the alpha channel will stay the same
# @return PIX the target PIX
def PixColorMul(PIX, r_f, g_f, b_f, a_f):


## Change the color of a PIX by shifting values of each channel of RGBA. The value of each channel will be clipped into [0, 255]
# @param pix a PIX, the source PIX
# @param r an integer, the red channel offset, can be negative, if r == 0, the red channel will stay the same
# @param g an integer, the green channel offset, can be negative, if g == 0, the green channel will stay the same
# @param b an integer, the blue channel offset, can be negative, if b == 0, the blue channel will stay the same
# @param a an integer, the alpha channel offset, can be negative, if a == 0, the alpha channel will stay the same
# @return PIX the target PIX
def PixColorShift(PIX, r, g, b, a):


## Make the color in a PIX with specified rgb value transparent.
# @param pix a PIX, the source PIX
# @param rgb an integer, the target color, not include the alpha channel
# @return PIX the target PIX
def PixColorTrans(PIX, rgb):


## Change the value of colors including alpha channels in the PIX with flat rgba value.
# @param pix a PIX, the source PIX
# @param rgba an integer, the target color
# @return PIX the target PIX
def PixColorRGBA(PIX, rgba):


## Change the value of colors in the PIX with flat rgb value.
# @param pix a PIX, the source PIX
# @param rgb an integer, the target color, not include the alpha channel
# @return PIX the target PIX
def PixColorRGB(PIX, rgb):


## Change the value of alpha channels in the PIX with constant alpha value.
# @param pix a PIX, the source PIX
# @param alpha an integer, the target alpha, in [0, 255]
# @return PIX the target PIX
def PixColorA(PIX, alpha):


## Bilinear Filter, mainly used when creating effects of moving a PIX so that it will look smoother.
# @param pix a PIX, the source PIX
# @param offset_x a float, offset of the PIX in the horizontal direction
# @param offset_y a float, offset of the PIX in the vertical direction
# @return PIX the target PIX
def BilinearFilter(pix, offset_x, offset_y):

## Scale Filter, mainly used when creating effects of moving a PIX so that it will look smoother.
# Remark: unlike BilinearFilter, this filter is best fit for complicated pictures such as textured text, but is bad for solid colored square.
# @param pix a PIX, the source PIX
# @param offset_x a float, offset of the PIX in the horizontal direction
# @param offset_y a float, offset of the PIX in the vertical direction
# @return PIX the target PIX
def ScaleFilter(pix, offset_x, offset_y):


## Create a blank BigPIX. 
# Remark: a BigPix is indeed a list that has the following structure [(PIX, offset_x, offset_y, layer), (PIX, offset_x, offset_y, layer), ...], 
# BigPix is used to put several PIXs together, therefore you can treate several PIXs as a whole. 
# Unlike CombinePix(), BigPix usually contains PIXs that are in different positions. You can use BigPixAdd() to add a PIX to the BigPix, and use 
# ConvertBigPix() to convert the BigPix back to PIX. By using BigPix you can even treate a whole text line as a single PIX.
# @return BigPIX the target BigPIX
def InitBigPix():


## Add a PIX to the BigPIX.
# @param bigPix a BigPIX, the source BigPIX
# @param pix a PIX, the PIX that is going to be added
# @param offset_x a float, offset of the PIX in the horizontal direction
# @param offset_y a float, offset of the PIX in the vertical direction
# @param layer an integer, layer of the PIX
# @return 0 on success
def BigPixAdd(bigPix, pix, offset_x, offset_y, layer):


## Convert a BigPIX to PIX.
# @param bigPix a BigPIX, the source BigPIX
# @return PIX the target PIX
def ConvertBigPix(bigPix):


# file.h

## Create an ASS file and returns the handler to the file.
# @param ass_file a string, filename of the target ASS file
# @param ass_header a string, the ASS file header
# @return assFile the handler to the ASS file
def CreateAssFile(ass_file, ass_header):


## Append to an ASS file and returns the handler to the file.
# @param ass_file a string, filename of the target ASS file
# @return assFile the handler to the ASS file
def AppendAssFile(ass_file):


## Write ASS strings to ASS file.
# @param assFile a handler, handler to the ASS file
# @param ASS_BUF a list, the list containing the ASS FX strings
# @return 0 on success
def WriteAssFile(assFile, ASS_BUF):


## Finalize the ASS file (close the handler to the file and destroy contents assigned to it).
# @param assFile a handler, handler to the ASS file
# @return 0 on success
def FinAssFile(assFile):


## Create an TCAS file and returns the handler to the file.
# @param tcas_file a string, filename of the target TCAS file
# @param fx_width an integer, width of the FX
# @param fx_height an integer, height of the FX
# @param fx_fps a float, FPS of the FX
# @return tcasFile the handler to the TCAS file
def CreateTcasFile(tcas_file, fx_width, fx_height, fx_fps):


## Write ASS strings to ASS file.
# @param tcasFile a handler, handler to the TCAS file
# @param TCAS_BUF a list, the list containing the raw TCAS FX data chunks
# @return 0 on success
def WriteTcasFile(tcasFile, TCAS_BUF):


## Finalize the TCAS file (close the handler to the file and destroy contents assigned to it).
# @param tcasFile a handler, handler to the TCAS file
# @return 0 on success
def FinTcasFile(tcasFile):


# text.h

## Initialize a font and returns the handler to the font.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @param spacing an integer, spacing between texts, can be negative
# @param space_scale a float, scale of white spaces
# @param color an integer, rgb color of the font
# @param bord an integer, border of the font, can be negative
# @param is_outline an integer, 0 - include body, 1 - only retrieve the outline
# @return font the handler to the font
def InitFont(font_file, face_id, font_size, spacing, space_scale, color, bord, is_outline):


## Finalize the font and destroy the handler.
# @param font a handler, the handler to the font
# @return 0 on success
def FinFont(font):


## Initialize a font and retrieves a PIX of the specified text from the font.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @param spacing an integer, spacing between texts, can be negative
# @param space_scale a float, scale of white spaces
# @param color an integer, rgb color of the font
# @param bord an integer, border of the font, can be negative
# @param is_outline an integer, 0 - include body, 1 - only retrieve the outline
# @param text a string, the text that is going to generate a PIX
# @return PIX the target PIX
def TextPix(font_file, face_id, font_size, spacing, space_scale, color, bord, is_outline, text):

## Retrieve a PIX of the specified text from a font.
# @param font a handler, the handler to the font
# @param text a string, the text that is going to generate a PIX
# @return PIX the target PIX
def TextPix(font, text):


## Get text outline points.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @param text a string, the text of which the outline will be retrieved
# @param density a float, the density of points that we want to have, 1.0 is commonly used, however it can be less or larger than 1 according to the need
# @return ((x1, y1), (x2, y2), ...)
def TextOutlinePoints(font_file, face_id, font_size, text, density):

## Get text outline points.
# @param font a handler, the handler to the font
# @param text a string, the text of which the outline will be retrieved
# @param density a float, the density of points that we want to have, 1.0 is commonly used, however it can be less or larger than 1 according to the need
# @return ((x1, y1, a1), (x2, y2, a2), ...)
def TextOutlinePoints(pyFont, text, density):


## Initialize a font and retrieves the metrics of the specified text from the font.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @param spacing an integer, spacing between texts, can be negative
# @param space_scale a float, scale of white spaces
# @param text a string, the text that is going to be measured
# @return (0 width, 1 height, 2 horiBearingX, 3 horiBearingY, 4 horiAdvance, 5 vertBearingX, 6 vertBearingY, 7 vertAdvance, 8 x_ppem, 9 y_ppem, 10 x_scale, 11 y_scale, 12 ascender, 13 descender, 14 px_height, 15 max_advance)
def TextMetrics(font_file, face_id, font_size, spacing, space_scale, text):

## Retrieve the metrics of the specified text from a font.
# @param font a handler, the handler to the font
# @param text a string, the text that is going to generate a PIX
# @return (0 width, 1 height, 2 horiBearingX, 3 horiBearingY, 4 horiAdvance, 5 vertBearingX, 6 vertBearingY, 7 vertAdvance, 8 x_ppem, 9 y_ppem, 10 x_scale, 11 y_scale, 12 ascender, 13 descender, 14 px_height, 15 max_advance)
def TextMetrics(font, text):


# image.h

## Retrieve a PIX from a PNG image.
# @param filename a string, filename of the target image, it should be a PNG file
# @return PIX the target PIX
def ImagePix(filename):

## Retrieve a PIX from a PNG image, and resize the PIX to specified size.
# @param filename a string, filename of the target image, it should be a PNG file
# @param width an integer, width of the target PIX
# @param height an integer, height of the target PIX
# @return PIX the target PIX
def ImagePix(filename, width, height):


## Save a PIX to a PNG file.
# @param filename a string, filename of the target image
# @param pix a PIX, the target PIX that is going to be saved
# @return 0 on success
def SavePix(filename, pix):

## Resize a PIX to specified size and save it to a PNG file.
# @param filename a string, filename of the target image
# @param pix a PIX, the target PIX that is going to be saved
# @param width an integer, width of the target PIX
# @param height an integer, height of the target PIX
# @return 0 on success
def SavePix(filename, pix, width, height):


# utility.h

## Get the text outline (which is consisted of contours, lines, bezier arcs) as a string of ASS drawing commands.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @param text a string, the text of which the outline will be retrieved
# @param x an integer, position x of the offset
# @param y an integer, position y of the offset
# @return unicode string the target ASS drawing commands
def TextOutlineDraw(font_file, face_id, font_size, text, x, y):

## Get the text outline (which is consisted of contours, lines, bezier arcs) as a string of ASS drawing commands.
# @param font a handler, the handler to the font
# @param text a string, the text of which the outline will be retrieved
# @param x an integer, position x of the offset
# @param y an integer, position y of the offset
# @return unicode string the target ASS drawing commands
def TextOutlineDraw(pyFont, text, x, y):


## Check if a text string is a CJK string, here CJK means "Chinese or Japanese or Korea" not "Chinese & Japanese & Korea".
# @param text a string, the target text that is going to be checked
# @return 1 - true, 0 - false
def IsCjk(text):


## Change the text to fit the vertical text layout, only used for ASS FX.
# @param text a string, the target text that is going to be updated
# @return string the target text
def VertLayout(text):


## Show progress only needed in tcax py user mode.
# @param total an integer, total texts
# @param completed an integer, texts that had been completed
# @param file_id an integer, tcax py script file id
# @param file_num an integer, number of tcax py scripts that the FX used
# @return 0 on success
def ShowProgress(total, completed, file_id, file_num):


## Get points of a linear bezier curve.
# @param nPoints an integer, number of points you want to have
# @param xs a float, position x of the start point
# @param ys a float, position y of the start point
# @param xe a float, position x of the end point
# @param ye a float, position y of the end point
# @return ((x1, y1, a1), (x2, y2, a2), ...)
def Bezier1(nPoints, xs, ys, xe, ye):

## Get points of a quadratic bezier curve.
# @param nPoints an integer, number of points you want to have
# @param xs a float, position x of the start point
# @param ys a float, position y of the start point
# @param xe a float, position x of the end point
# @param ye a float, position y of the end point
# @param xc a float, position x of the control point
# @param yc a float, position y of the control point
# @return ((x1, y1, a1), (x2, y2, a2), ...)
def Bezier2(nPoints, xs, ys, xe, ye, xc, yc):

## Get points of a cubic bezier curve.
# @param nPoints an integer, number of points you want to have
# @param xs a float, position x of the start point
# @param ys a float, position y of the start point
# @param xe a float, position x of the end point
# @param ye a float, position y of the end point
# @param xc1 a float, position x of the 1st control point
# @param yc1 a float, position y of the 1st control point
# @param xc2 a float, position x of the 2nd control point
# @param yc2 a float, position y of the 2nd control point
# @return ((x1, y1, a1), (x2, y2, a2), ...)
def Bezier3(nPoints, xs, ys, xe, ye, xc1, yc1, xc2, yc2):

## Get points of a Nth order bezier curve with random control points within the specified interval.
# @param nPoints an integer, number of points you want to have
# @param xs a float, position x of the start point
# @param ys a float, position y of the start point
# @param xe a float, position x of the end point
# @param ye a float, position y of the end point
# @param xl1 a float, position x of the 1st limit point
# @param yl1 a float, position y of the 1st limit point
# @param xl2 a float, position x of the 2nd limit point
# @param yl2 a float, position y of the 2nd limit point
# @param order an integer, the order of the bezier curve
# @return ((x1, y1, a1), (x2, y2, a2), ...)
def BezierN(nPoints, xs, ys, xe, ye, xl1, yl1, xl2, yl2, order):


## Calculate the actual font size used in tcaxLib and util.magick modules.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @return float the actual font size used
def GetFontSize(font_file, face_id, font_size):

## Calculate a correct font size for cairo from GDI font size.
# @param font_file a string, filename of the target font file
# @param face_id an integer, font face id, should always be 1 in TTF files
# @param font_size an integer, font size, in em height
# @return integer the correct font size for cairo
def CairoFontSize(font_file, face_id, font_size):


# tcasfunc.h

## Append a PIX to the TCAS_BUF list.
# @param TCAS_BUF a list, the list containing the raw TCAS FX data chunks
# @param pix a PIX, the target PIX that is going to be added into the TCAS_BUF
# @param start an integer, the starting time of the PIX
# @param end an integer, the end time of the PIX
# @param offsetX a float, offset of the PIX in the horizontal direction
# @param offsetY a float, offset of the PIX in the vertical direction
# @param layer an integer, layer of the PIX
# @return 0 on success
def tcas_main(TCAS_BUF, pix, start, end, offsetX, offsetY, layer):

## Append a pair of key frame PIXs to the TCAS_BUF list.
# @param TCAS_BUF a list, the list containing the raw TCAS FX data chunks
# @param pix_start a PIX, the target start key frame PIX that is going to be added into the TCAS_BUF
# @param pix_end a PIX, the target end key frame PIX that is going to be added into the TCAS_BUF
# @param start an integer, the starting time of the PIX
# @param end an integer, the end time of the PIX
# @param offsetX a float, offset of the PIX in the horizontal direction
# @param offsetY a float, offset of the PIX in the vertical direction
# @param type an integer, type of the key frame PIX, 1 - normal, 2 - use bilinear filtering to make smooth motion
# @param layer an integer, layer of the PIX
# @return 0 on success
def tcas_keyframe(TCAS_BUF, pix_start, pix_end, start, end, offsetX, offsetY, type, layer):


## Parse non-linear TCAS_BUF to linear TCAS_BUF.
# @param TCAS_BUF a list, the list containing the raw TCAS FX data chunks (non-linear)
# @param width an integer, the horizontal resolution of the FX video
# @param height an integer, the vertical resolution of the FX video
# @param fps a float, FPS of the FX video
# @param layer an integer, layer of the resulting FX
# @return TCAS_BUF (parsed)
def tcas_parse(TCAS_BUF, width, height, fps, layer):







