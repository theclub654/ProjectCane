#ifndef MOUSE_H
#define MOUSE_H
#include <GLFW/glfw3.h>

/*
    mouse class to handle mouse callbacks
*/

class MOUSE 
{
public:
    /*
        callbacks
    */

    // cursor position changed
    static void CursorPosCallback(GLFWwindow* window, double _x, double _y);
    // mouse button state changed
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    // scroll wheel moved
    static void MouseWheelCallback(GLFWwindow* window, double dx, double dy);

    /*
        accessors
    */

    // get mouse x position
    static double GetMouseX();
    // get mouse y position
    static double GetMouseY();

    // get mouse change in x
    static double GetDX();
    // get mouse change in y
    static double GetDY();

    // get scroll value in x
    static double GetScrollDX();
    // get scroll value in y
    static double GetScrollDY();

    // get button state
    static bool Button(int button);
    // return if button changed then reset it in the changed array
    static bool ButtonChanged(int button);
    // return if button changed and is now up
    static bool ButtonWentUp(int button);
    // return if button changed and is now down
    static bool ButtonWentDown(int button);

private:
    /*
        static mouse values
    */

    // x posiiton
    static double x;
    // y position
    static double y;

    // previous x position
    static double lastX;
    // previous y position
    static double lastY;

    // change in x position from lastX
    static double dx;
    // change in y position from lastY
    static double dy;

    // change in scroll x
    static double scrollDx;
    // change in scroll y
    static double scrollDy;

    // if this is the first change in the mouse position
    static bool firstMouse;

    // button state array (true for down, false for up)
    static bool buttons[];
    // button changed array (true if changed)
    static bool buttonsChanged[];
};

#endif