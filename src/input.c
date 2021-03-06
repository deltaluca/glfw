//========================================================================
// GLFW - An OpenGL library
// Platform:    Any
// API version: 3.0
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

// Internal key state used for sticky keys
#define _GLFW_STICK 3


// Sets the cursor mode for the specified window
//
static void setCursorMode(_GLFWwindow* window, int newMode)
{
    int oldMode;

    if (newMode != GLFW_CURSOR_NORMAL &&
        newMode != GLFW_CURSOR_HIDDEN &&
        newMode != GLFW_CURSOR_DISABLED)
    {
        _glfwInputError(GLFW_INVALID_ENUM, NULL);
        return;
    }

    oldMode = window->cursorMode;
    if (oldMode == newMode)
        return;

    if (window == _glfw.focusedWindow)
    {
        if (oldMode == GLFW_CURSOR_DISABLED)
            _glfwPlatformSetCursorPos(window, _glfw.cursorPosX, _glfw.cursorPosY);
        else if (newMode == GLFW_CURSOR_DISABLED)
        {
            int width, height;

            _glfw.cursorPosX = window->cursorPosX;
            _glfw.cursorPosY = window->cursorPosY;

            _glfwPlatformGetWindowSize(window, &width, &height);
            _glfwPlatformSetCursorPos(window, width / 2.0, height / 2.0);
        }

        _glfwPlatformSetCursorMode(window, newMode);
    }

    window->cursorMode = newMode;
}

// Set sticky keys mode for the specified window
//
static void setStickyKeys(_GLFWwindow* window, int enabled)
{
    if (window->stickyKeys == enabled)
        return;

    if (!enabled)
    {
        int i;

        // Release all sticky keys
        for (i = 0;  i <= GLFW_KEY_LAST;  i++)
        {
            if (window->key[i] == _GLFW_STICK)
                window->key[i] = GLFW_RELEASE;
        }
    }

    window->stickyKeys = enabled;
}

// Set sticky mouse buttons mode for the specified window
//
static void setStickyMouseButtons(_GLFWwindow* window, int enabled)
{
    if (window->stickyMouseButtons == enabled)
        return;

    if (!enabled)
    {
        int i;

        // Release all sticky mouse buttons
        for (i = 0;  i <= GLFW_MOUSE_BUTTON_LAST;  i++)
        {
            if (window->mouseButton[i] == _GLFW_STICK)
                window->mouseButton[i] = GLFW_RELEASE;
        }
    }

    window->stickyMouseButtons = enabled;
}


//////////////////////////////////////////////////////////////////////////
//////                         GLFW event API                       //////
//////////////////////////////////////////////////////////////////////////

void _glfwInputKey(_GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLboolean repeated = GL_FALSE;

    if (action == GLFW_RELEASE && window->key[key] == GLFW_RELEASE)
        return;

    if (key >= 0 && key <= GLFW_KEY_LAST)
    {
        if (action == GLFW_PRESS && window->key[key] == GLFW_PRESS)
            repeated = GL_TRUE;

        if (action == GLFW_RELEASE && window->stickyKeys)
            window->key[key] = _GLFW_STICK;
        else
            window->key[key] = (char) action;
    }

    if (repeated)
        action = GLFW_REPEAT;

    if (window->callbacks.key)
        window->callbacks.key((GLFWwindow*) window, key, scancode, action, mods, window->callbacks.key_data);
}

void _glfwInputChar(_GLFWwindow* window, unsigned int character)
{
    if (character == -1)
        return;

    if (character < 32 || (character > 126 && character < 160))
        return;

    if (window->callbacks.character)
        window->callbacks.character((GLFWwindow*) window, character, window->callbacks.character_data);
}

void _glfwInputScroll(_GLFWwindow* window, double xoffset, double yoffset)
{
    if (window->callbacks.scroll)
        window->callbacks.scroll((GLFWwindow*) window, xoffset, yoffset, window->callbacks.scroll_data);
}

void _glfwInputMouseClick(_GLFWwindow* window, int button, int action, int mods)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        return;

    // Register mouse button action
    if (action == GLFW_RELEASE && window->stickyMouseButtons)
        window->mouseButton[button] = _GLFW_STICK;
    else
        window->mouseButton[button] = (char) action;

    if (window->callbacks.mouseButton)
        window->callbacks.mouseButton((GLFWwindow*) window, button, action, mods, window->callbacks.mouseButton_data);
}

void _glfwInputCursorMotion(_GLFWwindow* window, double x, double y)
{
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        if (x == 0.0 && y == 0.0)
            return;

        window->cursorPosX += x;
        window->cursorPosY += y;
    }
    else
    {
        if (window->cursorPosX == x && window->cursorPosY == y)
            return;

        window->cursorPosX = x;
        window->cursorPosY = y;
    }

    if (window->callbacks.cursorPos)
    {
        window->callbacks.cursorPos((GLFWwindow*) window,
                                    window->cursorPosX,
                                    window->cursorPosY,
                                    window->callbacks.cursorPos_data);
    }
}

void _glfwInputCursorEnter(_GLFWwindow* window, int entered)
{
    if (window->callbacks.cursorEnter)
        window->callbacks.cursorEnter((GLFWwindow*) window, entered, window->callbacks.cursorEnter_data);
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetInputMode(GLFWwindow* handle, int mode)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(0);

    switch (mode)
    {
        case GLFW_CURSOR:
            return window->cursorMode;
        case GLFW_STICKY_KEYS:
            return window->stickyKeys;
        case GLFW_STICKY_MOUSE_BUTTONS:
            return window->stickyMouseButtons;
        default:
            _glfwInputError(GLFW_INVALID_ENUM, NULL);
            return 0;
    }
}

GLFWAPI void glfwSetInputMode(GLFWwindow* handle, int mode, int value)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT();

    switch (mode)
    {
        case GLFW_CURSOR:
            setCursorMode(window, value);
            break;
        case GLFW_STICKY_KEYS:
            setStickyKeys(window, value ? GL_TRUE : GL_FALSE);
            break;
        case GLFW_STICKY_MOUSE_BUTTONS:
            setStickyMouseButtons(window, value ? GL_TRUE : GL_FALSE);
            break;
        default:
            _glfwInputError(GLFW_INVALID_ENUM, NULL);
            break;
    }
}

GLFWAPI int glfwGetKey(GLFWwindow* handle, int key)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);

    if (key < 0 || key > GLFW_KEY_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "The specified key is invalid");
        return GLFW_RELEASE;
    }

    if (window->key[key] == _GLFW_STICK)
    {
        // Sticky mode: release key now
        window->key[key] = GLFW_RELEASE;
        return GLFW_PRESS;
    }

    return (int) window->key[key];
}

GLFWAPI int glfwGetMouseButton(GLFWwindow* handle, int button)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT_OR_RETURN(GLFW_RELEASE);

    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        _glfwInputError(GLFW_INVALID_ENUM,
                        "The specified mouse button is invalid");
        return GLFW_RELEASE;
    }

    if (window->mouseButton[button] == _GLFW_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButton[button] = GLFW_RELEASE;
        return GLFW_PRESS;
    }

    return (int) window->mouseButton[button];
}

GLFWAPI void glfwGetCursorPos(GLFWwindow* handle, double* xpos, double* ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT();

    if (xpos)
        *xpos = window->cursorPosX;

    if (ypos)
        *ypos = window->cursorPosY;
}

GLFWAPI void glfwSetCursorPos(GLFWwindow* handle, double xpos, double ypos)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    _GLFW_REQUIRE_INIT();

    if (_glfw.focusedWindow != window)
        return;

    // Don't do anything if the cursor position did not change
    if (xpos == window->cursorPosX && ypos == window->cursorPosY)
        return;

    // Set GLFW cursor position
    window->cursorPosX = xpos;
    window->cursorPosY = ypos;

    // Do not move physical cursor if it is disabled
    if (window->cursorMode == GLFW_CURSOR_DISABLED)
        return;

    // Update physical cursor position
    _glfwPlatformSetCursorPos(window, xpos, ypos);
}

GLFWAPI void* glfwSetKeyCallback(GLFWwindow* handle, GLFWkeyfun cbfun, void* data)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    void* previous;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    previous = window->callbacks.key_data;
    window->callbacks.key = cbfun;
    window->callbacks.key_data = data;
    return previous;
}

GLFWAPI void* glfwSetCharCallback(GLFWwindow* handle, GLFWcharfun cbfun, void* data)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    void* previous;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    previous = window->callbacks.character_data;
    window->callbacks.character = cbfun;
    window->callbacks.character_data = data;
    return previous;
}

GLFWAPI void* glfwSetMouseButtonCallback(GLFWwindow* handle,
                                         GLFWmousebuttonfun cbfun,
                                         void* data)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    void* previous;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    previous = window->callbacks.mouseButton_data;
    window->callbacks.mouseButton = cbfun;
    window->callbacks.mouseButton_data = data;
    return previous;
}

GLFWAPI void* glfwSetCursorPosCallback(GLFWwindow* handle,
                                       GLFWcursorposfun cbfun,
                                       void* data)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    void* previous;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    previous = window->callbacks.cursorPos_data;
    window->callbacks.cursorPos = cbfun;
    window->callbacks.cursorPos_data = data;
    return previous;
}

GLFWAPI void* glfwSetCursorEnterCallback(GLFWwindow* handle,
                                         GLFWcursorenterfun cbfun,
                                         void* data)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    void* previous;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    previous = window->callbacks.cursorEnter_data;
    window->callbacks.cursorEnter = cbfun;
    window->callbacks.cursorEnter_data = data;
    return previous;
}

GLFWAPI void* glfwSetScrollCallback(GLFWwindow* handle,
                                    GLFWscrollfun cbfun,
                                    void* data)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    void* previous;

    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);

    previous = window->callbacks.scroll_data;
    window->callbacks.scroll = cbfun;
    window->callbacks.scroll_data = data;
    return previous;
}

