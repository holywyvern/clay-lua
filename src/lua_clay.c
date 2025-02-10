#include <string.h>

#define LUA_LIB

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define CLAY_IMPLEMENTATION

#include "clay.h"

void
clay_lua_initStringCache(void);

const char *
clay_lua_storeString(const char *text, size_t len);

static void
clay_lua_handleError(Clay_ErrorData error)
{
  lua_State *L = error.userData;
  lua_pushliteral(L, "Clay Error: ");
  switch (error.errorType)
  {
    case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED:
    {
      lua_pushliteral(L, "[Measure Text Function not Provided] ");
      break;
    }
    case CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED:
    {
      lua_pushliteral(L, "[Arena Capacity Exceeded] ");
      break;
    }
    case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED:
    {
      lua_pushliteral(L, "[Elements Capacity Exceeded] ");
      break;
    }
    case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
    {
      lua_pushliteral(L, "[Text Measurement Capaicty Exceeded] ");
      break;
    }
    case CLAY_ERROR_TYPE_DUPLICATE_ID:
    {
      lua_pushliteral(L, "[Duplicate ID] ");
      break;
    }
    case CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND:
    {
      lua_pushliteral(L, "[Parent Not Found] ");
      break;
    }
    case CLAY_ERROR_TYPE_PERCENTAGE_OVER_1:
    {
      lua_pushliteral(L, "[Percentage Over 1] ");
      break;
    }
    case CLAY_ERROR_TYPE_INTERNAL_ERROR:
    {
      lua_pushliteral(L, "[Internal Error] ");
      break;
    }  
    default:
    {
      lua_pushliteral(L, "[Unknown Error] ");
      break;
    }
  }
  lua_pushlstring(L, error.errorText.chars, error.errorText.length);
  lua_concat(L, 3);
  lua_error(L);
}

static int
l_setCurrentContext(lua_State *L)
{
  Clay_Context *ctx = lua_touserdata(L, 1);
  Clay_SetCurrentContext(ctx);
  return 0;
}

static int
l_getCurrentContext(lua_State *L)
{
  lua_pushlightuserdata(L, Clay_GetCurrentContext());
  int ctx = lua_gettop(L);
  lua_newtable(L);
  int meta = lua_gettop(L);
  lua_pushvalue(L, meta);
  lua_setfield(L, meta, "__index");
  lua_pushboolean(L, 0);
  lua_setfield(L, meta, "__metatable");
  lua_pushcfunction(L, l_setCurrentContext);
  lua_setfield(L, meta, "setCurrent");
  lua_pushvalue(L, meta);
  lua_setmetatable(L, ctx);
  lua_pushvalue(L, ctx);
  return 1;
}

static int
l_initialize(lua_State *L)
{
  uint32_t size = Clay_MinMemorySize();
  Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(size, malloc(size));
  float w = (float)lua_tonumber(L, 1);
  float h = (float)lua_tonumber(L, 2);
  Clay_Initialize(arena, (Clay_Dimensions) { w, h }, (Clay_ErrorHandler) { clay_lua_handleError, L });
  return 0;
}

static int
l_setLayoutDimensions(lua_State *L)
{
  float w = (float)lua_tonumber(L, 1);
  float h = (float)lua_tonumber(L, 2);
  Clay_SetLayoutDimensions((Clay_Dimensions){w, h});
  return 0;
}

static int
l_setPointerState(lua_State *L)
{
  float x = (float)lua_tonumber(L, 1);
  float y = (float)lua_tonumber(L, 2);
  int isdown = lua_toboolean(L, 3);
  Clay_SetPointerState((Clay_Vector2){x, y}, isdown);
  return 0;
}

static int
l_updateScrollContainers(lua_State *L)
{
  int drag = lua_toboolean(L, 1);
  float x = (float)lua_tonumber(L, 2);
  float y = (float)lua_tonumber(L, 3);
  float dt = (float)lua_tonumber(L, 4);
  Clay_UpdateScrollContainers(drag, (Clay_Vector2){x, y}, dt);
  return 0;
}

static int
l_beginLayout(lua_State *L)
{
  Clay_BeginLayout();
  return 0;
}

static void
clay_lua_build_bounding_box(lua_State *L, Clay_BoundingBox *box)
{
  lua_newtable(L);
  int bb = lua_gettop(L);
  lua_pushnumber(L, box->x);
  lua_setfield(L, bb, "x");
  lua_pushnumber(L, box->y);
  lua_setfield(L, bb, "y");
  lua_pushnumber(L, box->width);
  lua_setfield(L, bb, "width");
  lua_pushnumber(L, box->height);
  lua_setfield(L, bb, "height");
  lua_pushvalue(L, bb);
}

static void
clay_lua_build_color(lua_State *L, Clay_Color *data)
{
  lua_newtable(L);
  int color = lua_gettop(L);
  lua_pushnumber(L, data->r);
  lua_setfield(L, color, "r");
  lua_pushnumber(L, data->g);
  lua_setfield(L, color, "g");
  lua_pushnumber(L, data->b);
  lua_setfield(L, color, "b");
  lua_pushnumber(L, data->a);
  lua_setfield(L, color, "a");
  lua_pushvalue(L, color);
}

static void
clay_lua_build_cornerRadius(lua_State *L, Clay_CornerRadius *data)
{
  lua_newtable(L);
  int corner = lua_gettop(L);
  lua_pushnumber(L, data->topLeft);
  lua_setfield(L, corner, "topLeft");
  lua_pushnumber(L, data->topRight);
  lua_setfield(L, corner, "topRight");
  lua_pushnumber(L, data->bottomLeft);
  lua_setfield(L, corner, "bottomLeft");
  lua_pushnumber(L, data->bottomRight);
  lua_setfield(L, corner, "bottomRight");
  lua_pushvalue(L, corner);  
}

static void
clay_lua_build_borderWidth(lua_State *L, Clay_BorderWidth *data)
{
  lua_newtable(L);
  int w = lua_gettop(L);
  lua_pushnumber(L, data->top);
  lua_setfield(L, w, "top");
  lua_pushnumber(L, data->bottom);
  lua_setfield(L, w, "bottom");
  lua_pushnumber(L, data->left);
  lua_setfield(L, w, "left");
  lua_pushnumber(L, data->right);
  lua_setfield(L, w, "right");
  lua_pushnumber(L, data->betweenChildren);
  lua_setfield(L, w, "betweenChildren");
  lua_pushvalue(L, w);
}

static void
clay_lua_build_dimensions(lua_State *L, Clay_Dimensions *data)
{
  lua_newtable(L);
  int dim = lua_gettop(L);
  lua_pushnumber(L, data->width);
  lua_setfield(L, dim, "width");
  lua_pushnumber(L, data->height);
  lua_setfield(L, dim, "height");
  lua_pushvalue(L, dim);
}

static void
clay_lua_build_rectangle_command(lua_State *L, int cmd, Clay_RectangleRenderData *data)
{
  clay_lua_build_color(L, &(data->backgroundColor)); 
  lua_setfield(L, cmd, "backgroundColor");
  clay_lua_build_cornerRadius(L, &(data->cornerRadius));
  lua_setfield(L, cmd, "cornerRadius");
}

static void
clay_lua_build_border_command(lua_State *L, int cmd, Clay_BorderRenderData *data)
{
  clay_lua_build_color(L, &(data->color));
  lua_setfield(L, cmd, "color");
  clay_lua_build_cornerRadius(L, &(data->cornerRadius));
  lua_setfield(L, cmd, "cornerRadius");
  clay_lua_build_borderWidth(L, &(data->width));
  lua_setfield(L, cmd, "width");
}

static void
clay_lua_build_text_command(lua_State *L, int cmd, Clay_TextRenderData *data)
{
  lua_pushnumber(L, data->fontId);
  lua_setfield(L, cmd, "fontId");
  lua_pushnumber(L, data->fontSize);
  lua_setfield(L, cmd, "fontSize");
  lua_pushnumber(L, data->letterSpacing);
  lua_setfield(L, cmd, "letterSpacing");
  lua_pushnumber(L, data->lineHeight);
  lua_setfield(L, cmd, "lineHeight");
  clay_lua_build_color(L, &(data->textColor));
  lua_setfield(L, cmd, "textColor");
  lua_pushlstring(L, data->stringContents.chars, data->stringContents.length);
  lua_setfield(L, cmd, "stringContents");
}

static void
clay_lua_build_image_command(lua_State *L, int cmd, Clay_ImageRenderData *data)
{
  if (data->imageData)
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, (int)(uintptr_t)data->imageData);
    lua_setfield(L, cmd, "imageData");
  }
  clay_lua_build_dimensions(L, &(data->sourceDimensions));
  lua_setfield(L, cmd, "sourceDimensions");
  clay_lua_build_cornerRadius(L, &(data->cornerRadius));
  lua_setfield(L, cmd, "cornerRadius");
  clay_lua_build_color(L, &(data->backgroundColor));
  lua_setfield(L, cmd, "backgroundColor");  
}

static void
clay_lua_build_custom_command(lua_State *L, int cmd, Clay_CustomRenderData *data)
{
  clay_lua_build_color(L, &(data->backgroundColor));
  lua_setfield(L, cmd, "backgroundColor");
  clay_lua_build_cornerRadius(L, &(data->cornerRadius));
  lua_setfield(L, cmd, "cornerRadius");
  lua_rawgeti(L, LUA_REGISTRYINDEX, (int)(uintptr_t)data->customData);
  lua_setfield(L, cmd, "customData");
}

static void
clay_lua_build_render_command(lua_State *L, Clay_RenderCommand *data)
{
  lua_newtable(L);
  int cmd = lua_gettop(L);
  lua_pushnumber(L, data->id);
  lua_setfield(L, cmd, "id");
  clay_lua_build_bounding_box(L, &(data->boundingBox));
  lua_setfield(L, cmd, "boundingBox");
  lua_pushnumber(L, data->zIndex);
  lua_setfield(L, cmd, "zIndex");
  if (data->userData)
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, (int)(uintptr_t)data->userData);
    lua_setfield(L, cmd, "userData");
  }
  switch (data->commandType)
  {
    case CLAY_RENDER_COMMAND_TYPE_NONE:
    {
      lua_pushliteral(L, "none");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
    {
      clay_lua_build_rectangle_command(L, cmd, &(data->renderData.rectangle));
      lua_pushliteral(L, "rectangle");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_BORDER:
    {
      clay_lua_build_border_command(L, cmd, &(data->renderData.border));
      lua_pushliteral(L, "border");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_TEXT:
    {
      clay_lua_build_text_command(L, cmd, &(data->renderData.text));
      lua_pushliteral(L, "text");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_IMAGE:
    {
      clay_lua_build_image_command(L, cmd, &(data->renderData.image));
      lua_pushliteral(L, "image");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
    {
      lua_pushliteral(L, "scissorStart");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
    {
      lua_pushliteral(L, "scissorEnd");
      break;
    }
    case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
    {
      clay_lua_build_custom_command(L, cmd, &(data->renderData.custom));
      lua_pushliteral(L, "custom");
      break;
    }
    default:
    {
      lua_pushliteral(L, "unknown");
      break;
    }
  }
  lua_setfield(L, cmd, "commandType");
  lua_pushvalue(L, cmd);
}

static int
l_endLayout(lua_State *L)
{
  lua_newtable(L);
  int list = lua_gettop(L);
  Clay_RenderCommandArray commands = Clay_EndLayout();
  for (int i = 0; i < commands.length; ++i)
  {
    int top = lua_gettop(L);
    clay_lua_build_render_command(L, &(commands.internalArray[i]));
    lua_rawseti(L, list, i + 1);
    lua_settop(L, top);
  }
  lua_pushvalue(L, list);
  return 1;
}

static int
l_openElement(lua_State *L)
{
  Clay__OpenElement();
  return 0;
}

static int
l_closeElement(lua_State *L)
{
  Clay__CloseElement();
  return 0;
}

static void
clay_lua_build_element_color(lua_State *L, int idx, Clay_Color *color)
{
  if (!lua_istable(L, idx)) return;

  lua_getfield(L, idx, "r");
  if (lua_isnumber(L, -1)) color->r = (float)lua_tonumber(L, -1);
  lua_getfield(L, idx, "g");
  if (lua_isnumber(L, -1)) color->g = (float)lua_tonumber(L, -1);
  lua_getfield(L, idx, "b");
  if (lua_isnumber(L, -1)) color->b = (float)lua_tonumber(L, -1);
  lua_getfield(L, idx, "a");
  if (lua_isnumber(L, -1)) color->a = (float)lua_tonumber(L, -1);
}

static void
clay_lua_build_element_backgroundColor(lua_State *L, int idx, Clay_Color *color)
{
  lua_getfield(L, idx, "backgroundColor");
  clay_lua_build_element_color(L, lua_gettop(L), color);
}

static void
clay_lua_build_element_border_width(lua_State *L, int idx, Clay_BorderWidth *width)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    width->top = (uint16_t)lua_tonumber(L, idx);
    width->bottom = width->left = width->right = width->top;
  }
  if (!lua_istable(L, idx)) return;

  lua_getfield(L, idx, "top");
  if (lua_isnumber(L, -1)) width->top = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "bottom");
  if (lua_isnumber(L, -1)) width->bottom = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "left");
  if (lua_isnumber(L, -1)) width->left = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "right");
  if (lua_isnumber(L, -1)) width->right = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "betweenChildren");
  if (lua_isnumber(L, -1)) width->betweenChildren = (uint16_t)lua_tonumber(L, -1);
}

static void
clay_lua_build_element_border(lua_State *L, int idx, Clay_BorderElementConfig *border)
{
  lua_getfield(L, idx, "border");
  if (!lua_istable(L, -1)) return;

  int b = lua_gettop(L);
  lua_getfield(L, b, "color");
  clay_lua_build_element_color(L, lua_gettop(L), &(border->color));
  lua_getfield(L, b, "size");
  clay_lua_build_element_border_width(L, lua_gettop(L),  &(border->width));
}

static void
clay_lua_build_element_cornerRadius(lua_State *L, int idx, Clay_CornerRadius *corner)
{
  lua_getfield(L, idx, "cornerRadius");
  if (lua_isnil(L, -1)) return;

  if (lua_isnumber(L, -1))
  {
    corner->bottomLeft = (float)lua_tonumber(L, -1);
    corner->bottomRight = corner->topLeft = corner->topRight = corner->bottomLeft;
  }
  if (!lua_istable(L, -1)) return;

  int t = lua_gettop(L);
  lua_getfield(L, t, "bottomLeft");
  if (lua_isnumber(L, -1)) corner->bottomLeft = (float)lua_tonumber(L, -1);
  lua_getfield(L, t, "bottomRight");
  if (lua_isnumber(L, -1)) corner->bottomRight = (float)lua_tonumber(L, -1);
  lua_getfield(L, t, "topLeft");
  if (lua_isnumber(L, -1)) corner->topLeft = (float)lua_tonumber(L, -1);
  lua_getfield(L, t, "topRight");
  if (lua_isnumber(L, -1)) corner->topRight = (float)lua_tonumber(L, -1);
}

static void
clay_lua_build_element_custom(lua_State *L, int idx, Clay_CustomElementConfig *custom)
{
  lua_getfield(L, idx, "custom");
  if (lua_isnil(L, -1)) return;

  custom->customData = (void *)(uintptr_t)luaL_ref(L, lua_gettop(L));
}

static void
clay_lua_build_element_vector2(lua_State *L, int idx, Clay_Vector2 *vec)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    vec->x = vec->y = (float)lua_tonumber(L, idx);
  }
  if (!lua_istable(L, idx)) return;

  lua_getfield(L, idx, "x");
  if (lua_isnumber(L, -1)) vec->x = (float)lua_tonumber(L, -1);
  lua_getfield(L, idx, "y");
  if (lua_isnumber(L, -1)) vec->y = (float)lua_tonumber(L, -1);
}

static void
clay_lua_build_element_dimensions(lua_State *L, int idx, Clay_Dimensions *dim)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    dim->width = dim->height = (float)lua_tonumber(L, idx);
  }
  if (!lua_istable(L, idx)) return;

  lua_getfield(L, idx, "width");
  if (lua_isnumber(L, -1)) dim->width = (float)lua_tonumber(L, -1);
  lua_getfield(L, idx, "height");
  if (lua_isnumber(L, -1)) dim->height = (float)lua_tonumber(L, -1);
}

static void
clay_lua_build_element_floatingAttatchTo(lua_State *L, int idx, Clay_FloatingAttachToElement *to)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    *to = (Clay_FloatingAttachToElement)lua_tonumber(L, idx);
    return;
  }
  const char *s = lua_tostring(L, idx);
  if (strcmp(s, "none") == 0)
  {
    *to = CLAY_ATTACH_TO_NONE;
  }
  else if (strcmp(s, "parent") == 0)
  {
    *to = CLAY_ATTACH_TO_PARENT;
  }
  else if (strcmp(s, "element") == 0)
  {
    *to = CLAY_ATTACH_TO_ELEMENT_WITH_ID;
  }
  else if (strcmp(s, "root") == 0)
  {
    *to = CLAY_ATTACH_TO_ROOT;
  }
}

static void
clay_lua_build_element_parentId(lua_State *L, int idx, uint32_t *parentId)
{
  if (lua_isnil(L, idx)) return;

  size_t len;
  const char *str = lua_tolstring(L, idx, &len);
  Clay_String key = (Clay_String){(int32_t)len, clay_lua_storeString(str, len)};
  *parentId = Clay_GetElementId(key).id;
}

static void
clay_lua_build_element_floatingAttachPointType(lua_State *L, int idx, Clay_FloatingAttachPointType *type)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    *type = lua_tonumber(L, idx);
    return;
  }

  const char *s = lua_tostring(L, idx);
  if (strcmp(s, "top left") == 0 || strcmp(s, "left top") == 0)
  {
    *type = CLAY_ATTACH_POINT_LEFT_TOP;
  }
  else if (strcmp(s, "left center") == 0 || strcmp(s, "center left") == 0)
  {
    *type = CLAY_ATTACH_POINT_LEFT_CENTER;
  }
  else if (strcmp(s, "left bottom") == 0 || strcmp(s, "bottom left") == 0)
  {
    *type = CLAY_ATTACH_POINT_LEFT_BOTTOM;
  }
  else if (strcmp(s, "top") == 0 || strcmp(s, "center top") == 0 || strcmp(s, "top center") == 0)
  {
    *type = CLAY_ATTACH_POINT_CENTER_TOP;
  }
  else if (strcmp(s, "center") == 0 || strcmp(s, "center center") == 0)
  {
    *type = CLAY_ATTACH_POINT_CENTER_CENTER;
  }
  else if (strcmp(s, "bottom") == 0 || strcmp(s, "center bottom") == 0 || strcmp(s, "bottom center") == 0)
  {
    *type = CLAY_ATTACH_POINT_CENTER_BOTTOM;
  }
  else if (strcmp(s, "right top") == 0 || strcmp(s, "top right") == 0)
  {
    *type = CLAY_ATTACH_POINT_RIGHT_TOP;
  }
  else if (strcmp(s, "right") == 0 || strcmp(s, "center right") == 0 || strcmp(s, "right center") == 0)
  {
    *type = CLAY_ATTACH_POINT_RIGHT_CENTER;
  }
  else if (strcmp(s, "right bottom") == 0 || strcmp(s, "bottom right") == 0)
  {
    *type = CLAY_ATTACH_POINT_RIGHT_BOTTOM;
  }
}

static void
clay_lua_build_element_floatingAttachPoints(lua_State *L, int idx, Clay_FloatingAttachPoints *points)
{
  if (!lua_istable(L, idx)) return;

  lua_getfield(L, idx, "element");
  clay_lua_build_element_floatingAttachPointType(L, lua_gettop(L), &(points->element));
  lua_getfield(L, idx, "parent");
  clay_lua_build_element_floatingAttachPointType(L, lua_gettop(L), &(points->parent));
}

static void
clay_lua_build_element_floating(lua_State *L, int idx, Clay_FloatingElementConfig *floating)
{
  lua_getfield(L, idx, "floating");
  if (!lua_istable(L, -1)) return;

  int f = lua_gettop(L);
  lua_getfield(L, f, "attachPoints");
  clay_lua_build_element_floatingAttachPoints(L, lua_gettop(L), &(floating->attachPoints));
  lua_getfield(L, f, "attachTo");
  clay_lua_build_element_floatingAttatchTo(L, lua_gettop(L), &(floating->attachTo));
  lua_getfield(L, f, "expand");
  clay_lua_build_element_dimensions(L, lua_gettop(L), &(floating->expand));
  lua_getfield(L, f, "offset");
  clay_lua_build_element_vector2(L, lua_gettop(L), &(floating->offset));
  lua_getfield(L, f, "parentId");
  clay_lua_build_element_parentId(L, lua_gettop(L), &(floating->parentId));  
  lua_getfield(L, f, "pointerCaptureMode");
  if (lua_isnumber(L, -1))
  {
    floating->pointerCaptureMode = (Clay_PointerCaptureMode)lua_tonumber(L, -1);
  }
  else if (lua_isstring(L, -1))
  {
    const char *pointer = lua_tostring(L, -1);
    if (strcmp(pointer, "passthrough") == 0)
    {
      floating->pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH;
    }
    else if (strcmp(pointer, "capture") == 0)
    {
      floating->pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE;
    }
  }
  lua_getfield(L, f, "zIndex");
  if (lua_isnumber(L, -1)) floating->zIndex = (int16_t)lua_tonumber(L, -1);
}

static void
clay_lua_build_element_id(lua_State *L, int idx, Clay_ElementId *id)
{
  lua_getfield(L, idx, "id");
  if (lua_isnil(L, -1)) return;

  size_t len;
  const char *str = lua_tolstring(L, -1, &len);
  Clay_String key = (Clay_String){(int32_t)len, clay_lua_storeString(str, len)};
  *id = Clay__HashString(key, 0, 0);
}

static void
clay_lua_build_element_image(lua_State *L, int idx, Clay_ImageElementConfig *img)
{
  lua_getfield(L, idx, "image");
  if (!lua_istable(L, -1)) return;

  lua_getfield(L, idx, "imageData");
  if (!lua_isnil(L, -1))
  {
    img->imageData = (void *)(uintptr_t)luaL_ref(L, -1);
  }
  lua_getfield(L, idx, "sourceDimensions");
  clay_lua_build_element_dimensions(L, lua_gettop(L), &(img->sourceDimensions));
}

static void
clay_lua_build_alignment(lua_State *L, int idx, Clay_ChildAlignment *align)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    align->x = align->y = (Clay_LayoutAlignmentX)lua_tonumber(L, idx);
    return;
  }
  if (lua_isstring(L, idx))
  {
    const char *str = lua_tostring(L, idx);
    if (strcmp(str, "center") == 0)
    {
      align->x = CLAY_ALIGN_X_CENTER;
      align->y = CLAY_ALIGN_Y_CENTER;      
    }
    else if (strcmp(str, "start"))
    {
      align->x = CLAY_ALIGN_X_LEFT;
      align->y = CLAY_ALIGN_Y_TOP;
    }
    else if (strcmp(str, "end"))
    {
      align->x = CLAY_ALIGN_X_RIGHT;
      align->y = CLAY_ALIGN_Y_BOTTOM;
    }
    return;
  }
  if (lua_istable(L, idx))
  {
    lua_getfield(L, idx, "x");
    if (lua_isnumber(L, -1))
    {
      align->x = (Clay_LayoutAlignmentX)lua_tonumber(L, -1);
    }
    else if (lua_isstring(L, -1))
    {
      const char *x = lua_tostring(L, -1);
      if (strcmp("left", x) == 0 || strcmp(x, "start") == 0)
      {
        align->x = CLAY_ALIGN_X_LEFT;
      }
      else if (strcmp("right", x) == 0 || strcmp(x, "end") == 0)
      {
        align->x = CLAY_ALIGN_X_RIGHT;
      }
      else if (strcmp("center", x) == 0 || strcmp(x, "middle") == 0)
      {
        align->x = CLAY_ALIGN_X_CENTER;
      }
    }
    lua_getfield(L, idx, "y");
    if (lua_isnumber(L, -1))
    {
      align->y = (Clay_LayoutAlignmentY)lua_tonumber(L, -1);
    }
    else if (lua_isstring(L, -1))
    {
      const char *y = lua_tostring(L, -1);
      if (strcmp(y, "top") == 0 || strcmp(y, "start") == 0)
      {
        align->y = CLAY_ALIGN_Y_TOP;
      }
      else if (strcmp(y, "bottom") == 0 || strcmp(y, "end") == 0)
      {
        align->y = CLAY_ALIGN_Y_BOTTOM;
      }
      else if (strcmp(y, "center") == 0 || strcmp(y, "middle") == 0)
      {
        align->y = CLAY_ALIGN_Y_CENTER;
      }
    }
  }
}

static void
clay_lua_build_layout_direction(lua_State *L, int idx, Clay_LayoutDirection *direction)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    *direction = (Clay_LayoutDirection)lua_tonumber(L, idx);
  }
  else if (lua_isstring(L, idx))
  {
    const char *d = lua_tostring(L, idx);
    if (strcmp(d, "row") == 0 || strcmp(d, "left to right") == 0)
    {
      *direction = CLAY_LEFT_TO_RIGHT;
    }
    else if (strcmp(d, "column") == 0 || strcmp(d, "top to bottom") == 0)
    {
      *direction = CLAY_TOP_TO_BOTTOM;
    }
  }
}

static void
clay_lua_build_padding(lua_State *L, int idx, Clay_Padding *padding)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isnumber(L, idx))
  {
    padding->right = (uint16_t)lua_tonumber(L, idx);
    padding->top = padding->bottom = padding->left = padding->right;
  }
  else if (lua_istable(L, idx))
  {
    lua_getfield(L, idx, "top");
    if (lua_isnumber(L, -1)) padding->top = (uint16_t)lua_tonumber(L, -1);
    lua_getfield(L, idx, "bottom");
    if (lua_isnumber(L, -1)) padding->bottom = (uint16_t)lua_tonumber(L, -1);
    lua_getfield(L, idx, "left");
    if (lua_isnumber(L, -1)) padding->left = (uint16_t)lua_tonumber(L, -1);
    lua_getfield(L, idx, "right");
    if (lua_isnumber(L, -1)) padding->right = (uint16_t)lua_tonumber(L, -1);
  }
}

static void
clay_lua_build_sizing_axis(lua_State *L, int idx, Clay_SizingAxis *axis)
{
  if (lua_isnil(L, idx)) return;
  if (lua_isnumber(L, idx))
  {
    float n = (float)lua_tonumber(L, idx);
    *axis = CLAY_SIZING_FIXED(n);
  }
  else if (lua_isstring(L, idx))
  {
    const char *s = lua_tostring(L, idx);
    if (strcmp(s, "grow") == 0)
    {
      *axis = CLAY_SIZING_GROW(0);
    }
    else if (strcmp(s, "fit") == 0)
    {
      *axis = CLAY_SIZING_FIT(0);
    }
  }
  else if (lua_istable(L, idx))
  {
    lua_getfield(L, idx, "type");
    if (lua_isnumber(L, -1))
    {
      axis->type = (Clay__SizingType)lua_tonumber(L, -1);
    }
    else if (lua_isstring(L, -1))
    {
      const char *type = lua_tostring(L, -1);
      if (strcmp(type, "grow") == 0)
      {
        axis->type = CLAY__SIZING_TYPE_GROW;
      }
      else if (strcmp(type, "fixed") == 0)
      {
        axis->type = CLAY__SIZING_TYPE_FIXED;
      }
      else if (strcmp(type, "fit") == 0)
      {
        axis->type = CLAY__SIZING_TYPE_FIT;
      }
    }
    lua_getfield(L, idx, "minMax");
    if (lua_isnumber(L, -1))
    {
      axis->size.minMax.min = axis->size.minMax.max = (float)lua_tonumber(L, -1);
    }
    lua_getfield(L, idx, "min");
    if (lua_isnumber(L, -1)) axis->size.minMax.min = (float)lua_tonumber(L, -1);
    lua_getfield(L, idx, "max");
    if (lua_isnumber(L, -1)) axis->size.minMax.max = (float)lua_tonumber(L, -1);
    lua_getfield(L, idx, "percent");
    if (lua_isnumber(L, -1))
    {
      float percent = (float)lua_tonumber(L, -1);
      *axis = CLAY_SIZING_PERCENT(percent);
    }
  }
}

static void
clay_lua_build_sizing(lua_State *L, int idx, Clay_Sizing *sizing)
{
  if (lua_isnil(L, idx)) return;

  if (lua_isstring(L, idx))
  {
    clay_lua_build_sizing_axis(L, idx, &(sizing->width));
    sizing->height = sizing->width;
  }
  else if (lua_istable(L, idx))
  {
    clay_lua_build_sizing_axis(L, idx, &(sizing->width));
    sizing->height = sizing->width;
    lua_getfield(L, idx, "width");
    clay_lua_build_sizing_axis(L, lua_gettop(L), &(sizing->width));
    lua_getfield(L, idx, "height");
    clay_lua_build_sizing_axis(L, lua_gettop(L), &(sizing->height));    
  }
}

static void
clay_lua_build_element_layout(lua_State *L, int idx, Clay_LayoutConfig *data)
{
  lua_getfield(L, idx, "layout");
  if (!lua_istable(L, -1)) return;
  
  int layout = lua_gettop(L);
  lua_getfield(L, layout, "childAlignment");
  clay_lua_build_alignment(L, lua_gettop(L), &(data->childAlignment));
  lua_getfield(L, layout, "childGap");
  if (lua_isnumber(L, -1))
  {
    data->childGap = (uint16_t)lua_tonumber(L, -1);
  }
  lua_getfield(L, layout, "layoutDirection");
  clay_lua_build_layout_direction(L, lua_gettop(L), &(data->layoutDirection));
  lua_getfield(L, layout, "padding");
  clay_lua_build_padding(L, lua_gettop(L), &(data->padding));
  lua_getfield(L, layout, "sizing");
  clay_lua_build_sizing(L, lua_gettop(L), &(data->sizing));
}

static void
clay_lua_build_element_scroll(lua_State *L, int idx, Clay_ScrollElementConfig *scroll)
{
  lua_getfield(L, idx, "scroll");
  if (lua_isnil(L, -1)) return;

  if (lua_istable(L, -1))
  {
    int s = lua_gettop(L);
    lua_getfield(L, s, "vertical");
    scroll->vertical = lua_toboolean(L, -1);
    lua_getfield(L, s, "horizontal");
    scroll->horizontal = lua_toboolean(L, -1);
  }
  else
  {
    scroll->horizontal = scroll->vertical = lua_toboolean(L, -1);
  }
}

static void
clay_lua_build_element_userData(lua_State *L, int idx, void **data)
{
  lua_getfield(L, idx, "userData");
  if (lua_isnil(L, -1)) return;

  *data = (void *)(uintptr_t)luaL_ref(L, -1);
}

static void
clay_lua_configure_element(lua_State *L, int idx, Clay_ElementDeclaration *config)
{
  if (!lua_istable(L, idx)) return;

  clay_lua_build_element_backgroundColor(L, idx, &(config->backgroundColor));
  clay_lua_build_element_border(L, idx, &(config->border));
  clay_lua_build_element_cornerRadius(L, idx, &(config->cornerRadius));
  clay_lua_build_element_custom(L, idx, &(config->custom));
  clay_lua_build_element_floating(L, idx, &(config->floating));
  clay_lua_build_element_id(L, idx, &(config->id));
  clay_lua_build_element_image(L, idx, &(config->image));
  clay_lua_build_element_layout(L, idx, &(config->layout));
  clay_lua_build_element_scroll(L, idx, &(config->scroll));
  clay_lua_build_element_userData(L, idx, &(config->userData));  
}

static int
l_configureOpenElement(lua_State *L)
{
  Clay_ElementDeclaration config = (Clay_ElementDeclaration){0};
  clay_lua_configure_element(L, 1, &config);
  Clay__ConfigureOpenElement(config);
  return 0;
}

static void
clay_lua_build_wrapMode(lua_State *L, Clay_TextElementConfigWrapMode mode)
{
  switch (mode)
  {
    case CLAY_TEXT_WRAP_WORDS:
    {
      lua_pushliteral(L, "words");
      return;
    }
    case CLAY_TEXT_WRAP_NEWLINES:
    {
      lua_pushliteral(L, "newlines");
      return;
    }
    case CLAY_TEXT_WRAP_NONE:
    {
      lua_pushliteral(L, "none");
      return;
    }
    default:
    {
      lua_pushnil(L);
      break;
    }
  }
}

static void
clay_lua_build_textConfig(lua_State *L, Clay_TextElementConfig *config)
{
  lua_newtable(L);
  int txt = lua_gettop(L);
  lua_pushnumber(L, config->fontId);
  lua_setfield(L, txt, "fontId");
  lua_pushnumber(L, config->fontSize);
  lua_setfield(L, txt, "fontSize");
  lua_pushboolean(L, config->hashStringContents);
  lua_setfield(L, txt, "hashStringContents");
  lua_pushnumber(L, config->letterSpacing);
  lua_setfield(L, txt, "letterSpacing");
  lua_pushnumber(L, config->lineHeight);
  lua_setfield(L, txt, "lineHeight");
  clay_lua_build_color(L, &(config->textColor));
  lua_setfield(L, txt, "textColor");
  clay_lua_build_wrapMode(L, config->wrapMode);
  lua_setfield(L, txt, "wrapMode");
  lua_pushvalue(L, txt);
}

static int
l_resetMeasureTextCache(lua_State *L)
{
  Clay_ResetMeasureTextCache();
  return 0;
}

static int
l_setMaxElementCount(lua_State *L)
{
  Clay_SetMaxElementCount((int32_t)lua_tonumber(L, 1));
  return 0;
}

static int
l_setMaxMeasureTextCacheWordCount(lua_State *L)
{
  Clay_SetMaxMeasureTextCacheWordCount((int32_t)lua_tonumber(L, 1));
  return 0;
}

/*
 * clay.hovered()
 */
static int
l_hovered(lua_State *L)
{
  lua_pushboolean(L, Clay_Hovered());
  return 1;
}

struct HoverData
{
  lua_State *L;
  int ref;
};

void
clay_lua_onHover(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData)
{
  struct HoverData *data = (struct HoverData *)userData;
  lua_State *L = data->L;
  int ref = data->ref;
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  lua_pushlstring(L, elementId.stringId.chars, elementId.stringId.length);
  lua_pushnumber(L, pointerData.position.x);
  lua_pushnumber(L, pointerData.position.y);
  switch (pointerData.state)
  {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
    {
      lua_pushliteral(L, "pressed this frame");
    };
    case CLAY_POINTER_DATA_PRESSED:
    {
      lua_pushliteral(L, "pressed");
    };
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
    {
      lua_pushliteral(L, "released this frame");
    };
    case CLAY_POINTER_DATA_RELEASED:
    {
      lua_pushliteral(L, "released");
    };
    default:
    {
      lua_pushliteral(L, "unknown");
      break;
    }
  }
  lua_call(L, 4, 0);
}

/*
 * clay.onHover(callback)
 */
static int
l_onHover(lua_State *L)
{
  struct HoverData *data = lua_newuserdata(L, sizeof*data);
  lua_pushvalue(L, -1);
  luaL_ref(L, -1);
  data->L = L;
  data->ref = luaL_ref(L, 1);
  Clay_OnHover(clay_lua_onHover, (intptr_t)data);
  return 0;
}

/*
 * clay.pointerOver(id)
 *
 * Checks if the mouse is over the element with the specific id.
 */
static int
l_pointerOver(lua_State *L)
{
  size_t len;
  const char *str = lua_tolstring(L, 1, &len);
  Clay_String key = (Clay_String){(int32_t)len, clay_lua_storeString(str, len)};
  Clay_ElementId id = Clay_GetElementId(key);
  lua_pushboolean(L, Clay_PointerOver(id));
  return 1;
}

static int
l_scrollPosition__index(lua_State *L)
{
  Clay_Vector2 *vec = lua_touserdata(L, 1);
  const char *index = lua_tostring(L, 2);
  if (strcmp(index, "x") == 0)
  {
    lua_pushnumber(L, vec->x);
  }
  else if (strcmp(index, "y") == 0)
  {
    lua_pushnumber(L, vec->y);
  }
  else
  {
    lua_pushnil(L);
  }
  return 1;
}

static int
l_scrollPosition__newindex(lua_State *L)
{
  Clay_Vector2 *vec = lua_touserdata(L, 1);
  const char *index = lua_tostring(L, 2);
  if (strcmp(index, "x") == 0)
  {
    vec->x = (float)lua_tonumber(L, 3);
  }
  else if (strcmp(index, "y") == 0)
  {
    vec->y = (float)lua_tonumber(L, 3);
  }
  return 0;
}

/**
 * clay.getScrollContainerData(id)
 * 
 * Retrieves the scroll container data of an element based on id.
 * The scrollPosition field acts as a pointer and can be modifiyed.
 */
static int
l_getScrollContainerData(lua_State *L)
{
  size_t len;
  const char *str = lua_tolstring(L, 1, &len);
  Clay_String key = (Clay_String){(int32_t)len, clay_lua_storeString(str, len)};
  Clay_ElementId id = Clay_GetElementId(key);
  Clay_ScrollContainerData data = Clay_GetScrollContainerData(id);
  lua_newtable(L);
  int scroll = lua_gettop(L);
  lua_pushboolean(L, data.found);
  lua_setfield(L, scroll, "found");
  clay_lua_build_dimensions(L, &(data.contentDimensions));
  lua_setfield(L, scroll, "contentDimensions");
  clay_lua_build_dimensions(L, &(data.scrollContainerDimensions));
  lua_setfield(L, scroll, "scrollContainerDimensions");
  lua_pushlightuserdata(L, data.scrollPosition);
  int pos = lua_gettop(L);
  lua_pushvalue(L, pos);
  lua_setmetatable(L, pos);
  lua_pushvalue(L, pos);
  lua_setfield(L, scroll, "scrollPosition");
  lua_pushcfunction(L, l_scrollPosition__index);
  lua_setfield(L, pos, "__index");
  lua_pushcfunction(L, l_scrollPosition__newindex);
  lua_setfield(L, pos, "__newindex");
  lua_pushboolean(L, 0);
  lua_setfield(L, pos, "__metatable");
  lua_newtable(L);
  int config = lua_gettop(L);
  lua_pushboolean(L, data.config.horizontal);
  lua_setfield(L, config, "horizontal");
  lua_pushboolean(L, data.config.vertical);
  lua_setfield(L, config, "vertical");
  lua_pushvalue(L, config);
  lua_setfield(L, scroll, "config");
  lua_pushvalue(L, scroll);
  return 1;
}

static void
clay_lua_build_element_textConfig(lua_State *L, int idx, Clay_TextElementConfig *config)
{
  if (!lua_istable(L, idx)) return;

  lua_getfield(L, idx, "fontId");
  if (!lua_isnil(L, -1)) config->fontId = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "fontSize");
  if (!lua_isnil(L, -1)) config->fontSize = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "hashStringContents");
  config->hashStringContents = lua_toboolean(L, -1);
  lua_getfield(L, idx, "letterSpacing");
  if (!lua_isnil(L, -1)) config->letterSpacing = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "lineHeight");
  if (!lua_isnil(L, -1)) config->lineHeight = (uint16_t)lua_tonumber(L, -1);
  lua_getfield(L, idx, "textColor");
  clay_lua_build_element_color(L, lua_gettop(L), &(config->textColor));
  lua_getfield(L, idx, "wrapMode");
  if (lua_isnumber(L, -1))
  {
    config->wrapMode = lua_tonumber(L, -1);
  }
  else if (lua_isstring(L, -1))
  {
    const char *mode = lua_tostring(L, -1);
    if (strcmp(mode, "words") == 0)
    {
      config->wrapMode = CLAY_TEXT_WRAP_WORDS;
    }
    else if (strcmp(mode, "newlines") == 0)
    {
      config->wrapMode = CLAY_TEXT_WRAP_NEWLINES;
    }
    else if (strcmp(mode, "none") == 0)
    {
      config->wrapMode = CLAY_TEXT_WRAP_NONE;
    }
  }
} 

/*
 * clay.text(config)
 *
 * Equivalent to CLAY_TEXT(config).
 * Adds a text component into the stack.
 * For this to work, it needs for you to first call clay.setMeasureTextFunction.
 */
static int
l_text(lua_State *L)
{
  size_t len;
  const char *str = lua_tolstring(L, 1, &len);
  Clay_String text = (Clay_String){(int32_t)len, clay_lua_storeString(str, len)};
  Clay_TextElementConfig *config = &Clay_TextElementConfig_DEFAULT;
  if (lua_istable(L, 2))
  {
    lua_getfield(L, 2, "__data__");
    if (lua_isuserdata(L, -1))
    {
      config = lua_touserdata(L, -1);
    }
    else
    {
      config = lua_newuserdata(L, sizeof(Clay_TextElementConfig));
    }
    lua_pushvalue(L, -1);
    lua_setfield(L, 2, "__data__");
    *config = (Clay_TextElementConfig){0};
    clay_lua_build_element_textConfig(L, 2, config);
  }
  CLAY_TEXT(text, config);
  return 0;
}

static struct {
  lua_State *L;
  int ref;
} MeasureTextData;

static Clay_Dimensions
clay_lua_measureText(Clay_StringSlice str, Clay_TextElementConfig *config, void *_)
{
  lua_State *L = MeasureTextData.L;
  Clay_Dimensions result = (Clay_Dimensions){0};
  clay_lua_build_textConfig(L, config);
  int conf = lua_gettop(L);
  lua_rawgeti(L, LUA_REGISTRYINDEX, MeasureTextData.ref);
  if (lua_isnil(L, -1)) return result;
  lua_pushlstring(L, str.chars, str.length);
  lua_pushvalue(L, conf);
  lua_call(L, 2, 2);
  result.width = (float)lua_tonumber(L, -2);
  result.height = (float)lua_tonumber(L, -1);
  return result;
}

/*
 * clay.setMeasureTextFunction(function)
 *
 * Sets the function clay will use for measure text
 * The function must return 2 values, width and height.
 * 
 * Example:
 * 
 * local function measureText(text, config)
 *    local w, h = -- ... measure the text size
 *    return w, h
 * end
 * 
 * clay.setMeasureTextFunction(measureText)
 */
static int
l_setMeasureTextFunction(lua_State *L)
{
  if (lua_isnil(L, 1))
  {
    luaL_error(L, "measure text function cannot be nil");
  }
  MeasureTextData.L = L;
  MeasureTextData.ref = luaL_ref(L, 1);
  Clay_SetMeasureTextFunction(clay_lua_measureText, 0);
  return 0;
}

/*
 * clay(config, [function])
 *
 * Equivalent to CLAY(config) {} in clay.
 * 
 * Example:
 * 
 * clay({ sizing = 100 }, function ()
 *   -- children
 * end)
 * 
 * This is the equivalent to:
 * 
 * clay.openElement()
 * clay.configureOpenElement({ sizing = 100 })
 *   -- children
 * clay.closeElement()
 * 
 * If the element has no children, you can skip the function
 */
static int
l__call(lua_State *L)
{
  Clay__OpenElement();
  Clay_ElementDeclaration config = (Clay_ElementDeclaration){0};
  clay_lua_configure_element(L, 2, &config);
  Clay__ConfigureOpenElement(config);
  lua_pushvalue(L, 3);
  if (lua_isfunction(L, -1)) lua_call(L, 0, 0);
  Clay__CloseElement();
  return 0;
}

#define CLAY_LUA_FN(name) lua_pushcfunction(L, l_ ## name); lua_setfield(L, clay, #name)
#define CLAY_LUA_CONST(name) lua_pushnumber(L, CLAY_ ## name); lua_setfield(L, clay, #name)

LUALIB_API int
luaopen_clay(lua_State *L)
{
  clay_lua_initStringCache();
  lua_newtable(L);
  int clay = lua_gettop(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, "clay");
  /* Functions */
  CLAY_LUA_FN(initialize);
  CLAY_LUA_FN(getCurrentContext);
  CLAY_LUA_FN(setCurrentContext);
  CLAY_LUA_FN(setLayoutDimensions);
  CLAY_LUA_FN(setPointerState);
  CLAY_LUA_FN(updateScrollContainers); 
  CLAY_LUA_FN(beginLayout); 
  CLAY_LUA_FN(endLayout); 
  CLAY_LUA_FN(openElement); 
  CLAY_LUA_FN(closeElement); 
  CLAY_LUA_FN(configureOpenElement); 
  CLAY_LUA_FN(resetMeasureTextCache); 
  CLAY_LUA_FN(setMaxElementCount); 
  CLAY_LUA_FN(setMaxMeasureTextCacheWordCount);
  CLAY_LUA_FN(setMeasureTextFunction);
  CLAY_LUA_FN(hovered);
  CLAY_LUA_FN(onHover);
  CLAY_LUA_FN(pointerOver);
  CLAY_LUA_FN(getScrollContainerData);
  CLAY_LUA_FN(text);
  /* Constants */
  CLAY_LUA_CONST(LEFT_TO_RIGHT);
  CLAY_LUA_CONST(TOP_TO_BOTTOM);
  CLAY_LUA_CONST(ALIGN_X_LEFT);
  CLAY_LUA_CONST(ALIGN_X_RIGHT);
  CLAY_LUA_CONST(ALIGN_X_CENTER);
  CLAY_LUA_CONST(ALIGN_Y_BOTTOM);
  CLAY_LUA_CONST(ALIGN_Y_TOP);
  CLAY_LUA_CONST(ALIGN_Y_CENTER);
  CLAY_LUA_CONST(TEXT_WRAP_WORDS);
  CLAY_LUA_CONST(TEXT_WRAP_NEWLINES);
  CLAY_LUA_CONST(TEXT_WRAP_NONE); 
  CLAY_LUA_CONST(ATTACH_POINT_LEFT_TOP);
  CLAY_LUA_CONST(ATTACH_POINT_LEFT_CENTER);
  CLAY_LUA_CONST(ATTACH_POINT_LEFT_BOTTOM);
  CLAY_LUA_CONST(ATTACH_POINT_CENTER_TOP);
  CLAY_LUA_CONST(ATTACH_POINT_CENTER_CENTER);
  CLAY_LUA_CONST(ATTACH_POINT_CENTER_BOTTOM);
  CLAY_LUA_CONST(ATTACH_POINT_RIGHT_TOP);
  CLAY_LUA_CONST(ATTACH_POINT_RIGHT_CENTER);
  CLAY_LUA_CONST(ATTACH_POINT_RIGHT_BOTTOM); 
  CLAY_LUA_CONST(ATTACH_TO_NONE);
  CLAY_LUA_CONST(ATTACH_TO_PARENT);
  CLAY_LUA_CONST(ATTACH_TO_ROOT);
  CLAY_LUA_CONST(ATTACH_TO_ELEMENT_WITH_ID);  

  lua_newtable(L);
  int meta = lua_gettop(L);
  lua_pushboolean(L, 0);
  lua_setfield(L, meta, "__metatable");
  lua_pushcfunction(L, l__call);
  lua_setfield(L, meta, "__call");
  lua_pushvalue(L, meta);
  lua_setmetatable(L, clay);

  lua_pushvalue(L, clay);
  return 1;
}

#undef CLAY_LUA_FN
#undef CLAY_LUA_CONST

