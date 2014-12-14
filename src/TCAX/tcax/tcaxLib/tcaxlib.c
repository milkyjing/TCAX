/* 
 *  Copyright (C) 2009-2011 milkyjing <milkyjing@gmail.com>
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "file.h"
#include "text.h"
#include "image.h"
#include "utility.h"
#include "tcasfunc.h"


static TCAX_PyString tcaxlib_get_version(PyObject *self, PyObject *args) {
    return PyUnicode_FromUnicode((const Py_UNICODE *)TCAXLIB_VER_STR, wcslen(TCAXLIB_VER_STR));
}

static PyMethodDef tcaxLib_Methods[] = {
    { "tcaxLibGetVersion" ,  tcaxlib_get_version                     ,  METH_VARARGS ,  "tcaxLibGetVersion()" },
    /* in pix.h */
    { "PixPoints"         ,  tcaxlib_pix_points                      ,  METH_VARARGS ,  "PixPoints(PIX)" },
    { "BlankPix"          ,  tcaxlib_create_blank_pix                ,  METH_VARARGS ,  "BlankPix(width, height, rgba)" },
    { "PixResize"         ,  tcaxlib_resample_py_pix                 ,  METH_VARARGS ,  "PixResize(PIX, width, height)" },
    { "PixResizeF"        ,  tcaxlib_resample_py_pix_ex              ,  METH_VARARGS ,  "PixResizeF(PIX, width, height)" },
    { "PixEnlarge"        ,  tcaxlib_enlarge_py_pix                  ,  METH_VARARGS ,  "PixEnlarge(PIX, x, y)" },
    { "PixCrop"           ,  tcaxlib_crop_py_pix                     ,  METH_VARARGS ,  "PixCrop(PIX, left, top, right, bottom)" },
    { "PixStrip"          ,  tcaxlib_py_pix_strip_blank              ,  METH_VARARGS ,  "PixStrip(PIX)" },
    { "PixTexture"        ,  tcaxlib_apply_pix_texture               ,  METH_VARARGS ,  "PixTexture(PIX, texture)" },
    { "PixMask"           ,  tcaxlib_apply_pix_mask                  ,  METH_VARARGS ,  "PixMask(PIX, mask)" },
    { "PixReplaceAlpha"   ,  tcaxlib_pix_replace_alpha               ,  METH_VARARGS ,  "PixReplaceAlpha(PIX, PIX_alpha)" },
    { "PixBlur"           ,  tcaxlib_apply_gauss_blur                ,  METH_VARARGS ,  "PixBlur(PIX, radius)" },
    { "CombinePixs"       ,  tcaxlib_combine_two_pixs                ,  METH_VARARGS ,  "CombinePixs(back, overlay)" },
    { "PixColorMul"       ,  tcaxlib_pix_color_multiply              ,  METH_VARARGS ,  "PixColorMul(PIX, r_f, g_f, b_f, a_f)" },
    { "PixColorShift"     ,  tcaxlib_pix_color_shift                 ,  METH_VARARGS ,  "PixColorShift(PIX, r, g, b, a)" },
    { "PixColorTrans"     ,  tcaxlib_pix_color_transparent           ,  METH_VARARGS ,  "PixColorTrans(PIX, rgb)" },
    { "PixColorRGBA"      ,  tcaxlib_pix_color_flat_rgba             ,  METH_VARARGS ,  "PixColorRGBA(PIX, rgba)" },
    { "PixColorRGB"       ,  tcaxlib_pix_color_flat_rgb              ,  METH_VARARGS ,  "PixColorRGB(PIX, rgb)" },
    { "PixColorA"         ,  tcaxlib_pix_color_flat_alpha            ,  METH_VARARGS ,  "PixColorA(PIX, alpha)" },
    { "BilinearFilter"    ,  tcaxlib_bilinear_filter                 ,  METH_VARARGS ,  "BilinearFilter(PIX, offset_x, offset_y)" },
    { "ScaleFilter"       ,  tcaxlib_scale_filter                    ,  METH_VARARGS ,  "ScaleFilter(PIX, offset_x, offset_y)" },
    { "InitBigPix"        ,  tcaxlib_init_big_pix                    ,  METH_VARARGS ,  "InitBigPix()" },
    { "BigPixAdd"         ,  tcaxlib_big_pix_add                     ,  METH_VARARGS ,  "BigPixAdd(BIG_PIX, PIX, offset_x, offset_y, layer)" },
    { "ConvertBigPix"     ,  tcaxlib_convert_big_pix                 ,  METH_VARARGS ,  "ConvertBigPix(BIG_PIX)" },
    /* in file.h */
    { "CreateAssFile"     ,  tcaxlib_create_ass_file                 ,  METH_VARARGS ,  "CreateAssFile(ass_file, ass_header)" },
    { "AppendAssFile"     ,  tcaxlib_append_ass_file                 ,  METH_VARARGS ,  "AppendAssFile(ass_file)" },
    { "WriteAssFile"      ,  tcaxlib_write_ass_file                  ,  METH_VARARGS ,  "WriteAssFile(pyAssFile, ASS_BUF)" },
    { "FinAssFile"        ,  tcaxlib_fin_ass_file                    ,  METH_VARARGS ,  "FinAssFile(pyAssFile)" },
    { "CreateTcasFile"    ,  tcaxlib_create_tcas_file                ,  METH_VARARGS ,  "CreateTcasFile(tcas_file, fx_width, fx_height, fx_fps)" },
    { "WriteTcasFile"     ,  tcaxlib_write_tcas_file                 ,  METH_VARARGS ,  "WriteTcasFile(pyTcasFile, TCAS_BUF)" },
    { "FinTcasFile"       ,  tcaxlib_fin_tcas_file                   ,  METH_VARARGS ,  "FinTcasFile(pyTcasFile)" },
    /* in text.h */
    { "InitFont"          ,  tcaxlib_init_py_font                    ,  METH_VARARGS ,  "InitFont(font_file, face_id, font_size, spacing, space_scale, color, bord, is_outline)" },
    { "FinFont"           ,  tcaxlib_fin_py_font                     ,  METH_VARARGS ,  "FinFont(pyFont)" },
    { "TextPix"           ,  tcaxlib_get_pix_from_text               ,  METH_VARARGS ,  "TextPix(pyFont, texts) or TextPix(font_file, face_id, font_size, spacing, space_scale, color, bord, is_outline, texts)" },
    { "TextOutlinePoints" ,  tcaxlib_get_text_outline_as_points      ,  METH_VARARGS ,  "TextOutlinePoints(pyFont, text, density) or TextOutlinePoints(font_file, face_id, font_size, text, density)" },
    { "TextMetrics"       ,  tcaxlib_get_text_metrics                ,  METH_VARARGS ,  "TextMetrics(pyFont, text) or TextMetrics(font_file, face_id, font_size, spacing, space_scale, text)" },
    /* in image.h */
    { "ImagePix"          ,  tcaxlib_get_pix_from_image              ,  METH_VARARGS ,  "ImagePix(filename) or ImagePix(filename, width, height)" },
    { "SavePix"           ,  tcaxlib_save_pix_to_image               ,  METH_VARARGS ,  "SavePix(filename, PIX) or SavePix(filename, PIX, width, height)" },
    /* in utility.h */
    { "TextOutlineDraw"   ,  tcaxlib_get_text_outline_as_string      ,  METH_VARARGS ,  "TextOutlineDraw(pyFont, text, x, y) or TextOutlineDraw(font_file, face_id, font_size, text, x, y)" },
    { "IsCjk"             ,  tcaxlib_is_c_or_j_or_k                  ,  METH_VARARGS ,  "IsCjk(text)" },
    { "VertLayout"        ,  tcaxlib_vertical_layout_ass             ,  METH_VARARGS ,  "VertLayout(text)" },
    { "ShowProgress"      ,  tcaxlib_show_progress                   ,  METH_VARARGS ,  "ShowProgress(total, completed, file_id, file_num)" },
    { "Bezier1"           ,  tcaxlib_bezier_curve_linear             ,  METH_VARARGS ,  "Bezier1(nPoints, xs, ys, xe, ye)" },
    { "Bezier2"           ,  tcaxlib_bezier_curve_quadratic          ,  METH_VARARGS ,  "Bezier2(nPoints, xs, ys, xe, ye, xc, yc)" },
    { "Bezier3"           ,  tcaxlib_bezier_curve_cubic              ,  METH_VARARGS ,  "Bezier3(nPoints, xs, ys, xe, ye, xc1, yc1, xc2, yc2)" },
    { "BezierN"           ,  tcaxlib_bezier_curve_random             ,  METH_VARARGS ,  "BezierN(nPoints, xs, ys, xe, ye, xl1, yl1, xl2, yl2, order)" },
    { "GetFontSize"       ,  tcaxlib_get_actual_font_size            ,  METH_VARARGS ,  "GetFontSize(font_file, face_id, font_size)" },
    { "CairoFontSize"     ,  tcaxlib_calc_cairo_font_size            ,  METH_VARARGS ,  "CairoFontSize(font_file, face_id, font_size)" },
    /* in tcasfunc.h */
    { "tcas_main"         ,  tcaxlib_tcas_list_append_pix            ,  METH_VARARGS ,  "tcas_main(TCAS_BUF, pix, start, end, offsetX, offsetY, layer)" },
    { "tcas_keyframe"     ,  tcaxlib_tcas_list_append_key_pixs       ,  METH_VARARGS ,  "tcas_keyframe(TCAS_BUF, pix_start, pix_end, start, end, offsetX, offsetY, type, layer)" },
    { "tcas_parse"        ,  tcaxlib_tcas_list_parse                 ,  METH_VARARGS ,  "tcas_parse(TCAS_BUF, width, height, fps, layer)" },
    { NULL                ,  NULL                                    ,  0            ,  NULL }
};

static struct PyModuleDef tcaxLib_Module = {
    PyModuleDef_HEAD_INIT, 
    "tcaxLib", 
    "Module tcaxLib is a component of tcax - developed by milkyjing", 
    -1, 
    tcaxLib_Methods
};

PyMODINIT_FUNC PyInit_tcaxLib() {
    return PyModule_Create(&tcaxLib_Module);
}

