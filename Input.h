#pragma once
//Kevin's Input Layer using GLFW 

//List of all possible commands in the game!
enum INPUT_COMMANDS{
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_FORWARD,
    MOVE_BACK,
    JUMP,
    RAISE_CAM,
    LOWER_CAM,
    TILT_CAM_DOWN,
    TILT_CAM_UP,
    TURN_CAM_LEFT,
    TURN_CAM_RIGHT,
    NUM_INPUT_COMMANDS
};

#define MOUSE_DEFAULT_SENSITIVITY 0.4f
struct Mouse {
    bool click_left;
    bool click_right;
    double xpos, ypos;
    double prev_xpos, prev_ypos;
    double xscroll, yscroll;
    float sensitivity;
    bool is_in_window;
};
Mouse g_mouse = {
    false, false, 0, 0, 0, 0, 0, 0, MOUSE_DEFAULT_SENSITIVITY, false
};

//Global input state for game code to query (e.g.   if(g_input[MOVE_LEFT]) move_left(); )
bool g_input[NUM_INPUT_COMMANDS] = {0};

//For custom user key mappings (e.g.  g_key_mapping[DASH_MOVE] returns GLFW_KEY_ENTER)
//int g_key_mapping[NUM_INPUT_COMMANDS];

//Use Command instead of Control on Mac
#ifdef __APPLE__
int CTRL_KEY_LEFT = GLFW_KEY_LEFT_SUPER;
int CTRL_KEY_RIGHT = GLFW_KEY_RIGHT_SUPER;
#else
int CTRL_KEY_LEFT = GLFW_KEY_LEFT_CONTROL;
int CTRL_KEY_RIGHT = GLFW_KEY_RIGHT_CONTROL;
#endif

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
        case GLFW_KEY_SPACE:    g_input[JUMP]           = is_pressed; return;
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

//Mouse stuff

//glfwSetMouseButtonCallback(window, mouse_button_callback);
//Or poll with: if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)))
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    bool is_pressed = (action != GLFW_RELEASE);
    switch(button){
        case GLFW_MOUSE_BUTTON_LEFT:  g_mouse.click_left = is_pressed;  return;
        case GLFW_MOUSE_BUTTON_RIGHT: g_mouse.click_right = is_pressed; return;
        default:
            printf("How the heck did we get here?\n");
            printf("Mouse button: %d\n", button);
    }
}

//glfwSetCursorPosCallback(window, cursor_pos_callback);
//Just poll with glfwGetCursorPos(window, &xpos, &ypos);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos){
    g_mouse.xpos = xpos;
    g_mouse.ypos = ypos;
}

//glfwSetScrollCallback(window, scroll_callback);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    g_mouse.xscroll = xoffset;
    g_mouse.yscroll = yoffset;
}

//glfwSetCursorEnterCallback(window, cursor_enter_callback);
void cursor_enter_callback(GLFWwindow *window, int entered){
    g_mouse.is_in_window = entered;
}

//Joystick stuff
/*
bool controller_is_connected = false;

if(glfwJoystickPresent(GLFW_JOYSTICK_1) == GLFW_TRUE){ controller_is_connected = true; }

//glfwSetJoystickCallback(joystick_callback);
void joystick_callback(int joy, int event){
    if(event == GLFW_CONNECTED){ controller_is_connected = true; }
    else if(event == GLFW_DISCONNECTED){ controller_is_connected = false; }
}

int axis_count;
float* axis_values = glfwGetJoystickAxes (GLFW_JOYSTICK_1, &axis_count);

int button_count;
unsigned char* controller_buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &button_count);
*/

//Clipboard Stuff
/* //COPY
glfwSetClipboardString(window, "A string with words in it");

//PASTE
const char* text = glfwGetClipboardString(window);
if(text) insert_text(text);
*/

/* //File/Path Drop
//glfwSetDropCallback(window, drop_callback);
void drop_callback(GLFWwindow* window, int count, const char** paths){
    for(int i = 0;  i < count;  i++) handle_dropped_file(paths[i]);
}
*/
