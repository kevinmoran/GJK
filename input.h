#pragma once
//Kevin's Input Layer using GLFW 

//List of all possible commands in the game!
enum INPUT_COMMANDS{
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_FORWARD,
    MOVE_BACK,
    RAISE_CAM,
    LOWER_CAM,
    TILT_CAM_DOWN,
    TILT_CAM_UP,
    TURN_CAM_LEFT,
    TURN_CAM_RIGHT,
    NUM_INPUT_COMMANDS
};

//Global input state for game code to query (e.g.   if(g_input[MOVE_LEFT]) move_left(); )
bool g_input[NUM_INPUT_COMMANDS] = {0};

//For custom user key mappings (e.g.  g_key_mapping[DASH_MOVE] returns GLFW_KEY_ENTER)
//int g_key_mapping[NUM_INPUT_COMMANDS];

//glfwSetKeyCallback(window, key_callback);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //Immutable input keys:
    bool is_pressed = (action != GLFW_RELEASE);
    switch(key){
        case GLFW_KEY_W:        g_input[MOVE_FORWARD]   = is_pressed; return;
        case GLFW_KEY_A:        g_input[MOVE_LEFT]      = is_pressed; return;
        case GLFW_KEY_S:        g_input[MOVE_BACK]      = is_pressed; return;
        case GLFW_KEY_D:        g_input[MOVE_RIGHT]     = is_pressed; return;
        case GLFW_KEY_Q:        g_input[RAISE_CAM]      = is_pressed; return;
        case GLFW_KEY_E:        g_input[LOWER_CAM]      = is_pressed; return;
        case GLFW_KEY_UP:       g_input[TILT_CAM_UP]    = is_pressed; return;
        case GLFW_KEY_DOWN:     g_input[TILT_CAM_DOWN]  = is_pressed; return;
        case GLFW_KEY_LEFT:     g_input[TURN_CAM_LEFT]  = is_pressed; return;
        case GLFW_KEY_RIGHT:    g_input[TURN_CAM_RIGHT] = is_pressed; return;
        default: break;
    }
    //Custom key mapping:
    // if(key == g_key_mapping[DASH_MOVE]){  g_input[DASH_MOVE] = is_pressed; return; }
    // if(key == g_key_mapping[JUMP_MOVE]){  g_input[JUMP_MOVE] = is_pressed; return; }
}

//Joystick stuff
/*

bool controller_is_connected = false;

if(glfwJoystickPresent(GLFW_JOYSTICK_1) == GLFW_TRUE){
    controller_is_connected = true;
}

//glfwSetJoystickCallback(joystick_callback);
void joystick_callback(int joy, int event)
{
    if (event == GLFW_CONNECTED)
    {
        controller_is_connected = true;
    }
    else if (event == GLFW_DISCONNECTED)
    {
        controller_is_connected = false;
    }
}

int axis_count;
float* axis_values = glfwGetJoystickAxes (GLFW_JOYSTICK_1, &axis_count);

int button_count;
unsigned char* controller_buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &button_count);
*/

//Mouse stuff
/*
//glfwSetMouseButtonCallback(window, mouse_button_callback);
void mouse_button_callback(GLFW* window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        //Or poll with: if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)))
    }
}

//glfwSetCursorPosCallback(window, cursor_pos_callback);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos){
    //Don't think this is necessary
    //Just poll with glfwGetCursorPos(window, &xpos, &ypos);
}

//glfwSetCursorEnterCallback(window, cursor_enter_callback);
void cursor_enter_callback(GLFWwindow *window, int entered){
    if(entered){
        //cursor has entered window
    }
    else {
        //cursor has left window
    }
}

//glfwSetScrollCallback(window, scroll_callback);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}
*/

//Clipboard Stuff
/*
//COPY
glfwSetClipboardString(window, "A string with words in it");

//PASTE
const char* text = glfwGetClipboardString(window);
if (text)
    insert_text(text);

*/

//File/Path Drop
/*
//glfwSetDropCallback(window, drop_callback);
void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    for (int i = 0;  i < count;  i++)
        handle_dropped_file(paths[i]);
}
*/
