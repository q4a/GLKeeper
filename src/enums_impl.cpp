#include "pch.h"
#include "CommonTypes.h"
#include "GraphicsDefs.h"
#include "InputsDefs.h"

impl_enum_strings(eLogMessage)
{
    {eLogMessage_Debug, "debug"},
    {eLogMessage_Info, "info"},
    {eLogMessage_Warning, "warning"},
    {eLogMessage_Error, "error"},
};

impl_enum_strings(eTextureWrapMode)
{
    {eTextureWrapMode_Repeat, "repeat"},
    {eTextureWrapMode_ClampToEdge, "clamp_to_edge"},
};

impl_enum_strings(eTextureFilterMode)
{
    {eTextureFilterMode_Nearest, "nearest"},
    {eTextureFilterMode_Bilinear, "bilinear"},
    {eTextureFilterMode_Trilinear, "trilinear"},
};

impl_enum_strings(eTextureFormat)
{
    {eTextureFormat_Null, "null"},
    {eTextureFormat_R8, "r8"},
    {eTextureFormat_R8_G8, "r8_g8"},
    {eTextureFormat_RGB8, "rgb8"},
    {eTextureFormat_RGBA8, "rgba8"},
    {eTextureFormat_R8UI, "r8ui"},
    {eTextureFormat_RGBA8UI, "rgba8ui"},
    {eTextureFormat_R16UI, "r16ui"},
};

impl_enum_strings(ePrimitiveType)
{
    {ePrimitiveType_Points, "points"},
    {ePrimitiveType_Lines, "lines"},
    {ePrimitiveType_LineLoop, "line_loop"},
    {ePrimitiveType_Triangles, "triangles"},
    {ePrimitiveType_TriangleStrip, "triangle_strip"},
    {ePrimitiveType_TriangleFan, "triangle_fan"},
};

impl_enum_strings(eIndicesType)
{
    {eIndicesType_i16, "i16"},
    {eIndicesType_i32, "i32"},
};

impl_enum_strings(eTextureUnit)
{
    {eTextureUnit_0, "tex_0"},
    {eTextureUnit_1, "tex_1"},
    {eTextureUnit_2, "tex_2"},
    {eTextureUnit_3, "tex_3"},
    {eTextureUnit_4, "tex_4"},
    {eTextureUnit_5, "tex_5"},
    {eTextureUnit_6, "tex_6"},
    {eTextureUnit_7, "tex_7"},
    {eTextureUnit_8, "tex_8"},
    {eTextureUnit_9, "tex_9"},
    {eTextureUnit_10, "tex_10"},
    {eTextureUnit_11, "tex_11"},
    {eTextureUnit_12, "tex_12"},
    {eTextureUnit_13, "tex_13"},
    {eTextureUnit_14, "tex_14"},
    {eTextureUnit_15, "tex_15"},
};

impl_enum_strings(eVertexAttributeFormat)
{
    {eVertexAttributeFormat_2F, "2f"},
    {eVertexAttributeFormat_3F, "3f"},
    {eVertexAttributeFormat_4UB, "4ub"},
    {eVertexAttributeFormat_1US, "1us"},
    {eVertexAttributeFormat_Unknown, "unknown"},
};

impl_enum_strings(eVertexAttribute)
{
    {eVertexAttribute_Position0, "in_pos0"},
    {eVertexAttribute_Position1, "in_pos1"},
    {eVertexAttribute_Texcoord0, "in_texcoord0"},
    {eVertexAttribute_Texcoord1, "in_texcoord1"},
    {eVertexAttribute_Normal0, "in_normal0"},
    {eVertexAttribute_Normal1, "in_normal1"},
    {eVertexAttribute_Color0, "in_color0"},
    {eVertexAttribute_Color1, "in_color1"},
};

impl_enum_strings(eBufferContent)
{
    {eBufferContent_Vertices, "vertices"},
    {eBufferContent_Indices, "indices"},
};

impl_enum_strings(eBufferUsage)
{
    {eBufferUsage_Static, "static"},
    {eBufferUsage_Dynamic, "dynamic"},
    {eBufferUsage_Stream, "stream"},
};

impl_enum_strings(eBlendingMode)
{
    {eBlendingMode_Alpha, "alpha"},
    {eBlendingMode_Additive, "additive"},
    {eBlendingMode_Multiply, "multiply"},
    {eBlendingMode_Premultiplied, "premultiplied"},
    {eBlendingMode_Screen, "screen"},
};

impl_enum_strings(eDepthTestFunc)
{
    {eDepthTestFunc_Always, "always"},
    {eDepthTestFunc_Equal, "equal"},
    {eDepthTestFunc_NotEqual, "not_equal"},
    {eDepthTestFunc_Less, "less"},
    {eDepthTestFunc_Greater, "greater"},
    {eDepthTestFunc_LessEqual, "less_equal"},
    {eDepthTestFunc_GreaterEqual, "greater_equal"},
};

impl_enum_strings(eCullMode)
{
    {eCullMode_Front, "front"},
    {eCullMode_Back, "back"},
    {eCullMode_FrontAndBack, "front_and_back"},
};

impl_enum_strings(ePolygonFillMode)
{
    {ePolygonFillMode_Solid, "solid"},
    {ePolygonFillMode_WireFrame, "wireframe"},
};

impl_enum_strings(eKeycode)
{
    {eKeycode_ESCAPE, "Escape"},
    {eKeycode_SPACE, "Space"},
    {eKeycode_PAGE_UP, "PageUp"},
    {eKeycode_PAGE_DOWN, "PageDown"},
    {eKeycode_HOME, "Home"},
    {eKeycode_END, "End"},
    {eKeycode_INSERT, "Insert"},
    {eKeycode_DELETE, "Delete"},
    {eKeycode_RIGHT_CTRL, "RCtrl"},
    {eKeycode_LEFT_CTRL, "LCtrl"},
    {eKeycode_BACKSPACE, "Backspace"},
    {eKeycode_ENTER, "Enter"},
    {eKeycode_TAB, "Tab"},
    {eKeycode_TILDE, "~"},
    {eKeycode_F1, "F1"},
    {eKeycode_F2, "F2"},
    {eKeycode_F3, "F3"},
    {eKeycode_F4, "F4"},
    {eKeycode_F5, "F5"},
    {eKeycode_F6, "F6"},
    {eKeycode_F7, "F7"},
    {eKeycode_F8, "F8"},
    {eKeycode_F9, "F9"},
    {eKeycode_F10, "F10"},
    {eKeycode_F11, "F11"},
    {eKeycode_F12, "F12"},
    {eKeycode_A, "a"},
    {eKeycode_C, "c"},
    {eKeycode_F, "f"},
    {eKeycode_V, "v"},
    {eKeycode_X, "x"},
    {eKeycode_W, "w"},
    {eKeycode_D, "d"},
    {eKeycode_S, "s"},
    {eKeycode_Y, "y"},
    {eKeycode_Z, "z"},
    {eKeycode_R, "r"},
    {eKeycode_0, "0"},
    {eKeycode_1, "1"},
    {eKeycode_2, "2"},
    {eKeycode_3, "3"},
    {eKeycode_4, "4"},
    {eKeycode_5, "5"},
    {eKeycode_6, "6"},
    {eKeycode_7, "7"},
    {eKeycode_8, "8"},
    {eKeycode_9, "9"},
    {eKeycode_LEFT, "Left"},
    {eKeycode_RIGHT, "Right"},
    {eKeycode_UP, "Up"},
    {eKeycode_DOWN, "Down"},
};

impl_enum_strings(eMouseButton)
{
    {eMouseButton_Left, "Left"},
    {eMouseButton_Right, "Right"},
    {eMouseButton_Middle, "Middle"},
};