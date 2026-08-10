#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image/stb_image.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <atomic>

#include "windowsMain.h"

static std::vector<std::string> s_arg;

// Vertex shader source code
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)glsl";

const GLchar* vertex_shader_glsl_130 =
"uniform mat4 ProjMtx;\n"
"in vec2 Position;\n"
"in vec2 UV;\n"
"in vec4 Color;\n"
"out vec2 Frag_UV;\n"
"out vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"    Frag_UV = UV;\n"
"    Frag_Color = Color;\n"
"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
"}\n";

// Fragment shader source code
const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D ourTexture;

    void main()
    {
        FragColor = texture(ourTexture, TexCoord);
    }
)glsl";

const GLchar* fragment_shader_glsl_130 =
"uniform sampler2D Texture;\n"
"in vec2 Frag_UV;\n"
"in vec4 Frag_Color;\n"
"out vec4 Out_Color;\n"
"void main()\n"
"{\n"
"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
"}\n";


// Function to compile a shader
GLuint compileShader(GLenum type, const char* source) 
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

// Function to create a shader program
GLuint createShaderProgram() 
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

static std::string WCharToString(const wchar_t* w)
{
    if (!w) return {};

    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        w, -1,
        nullptr, 0,
        nullptr, nullptr
    );

    std::string str(size, 0);

    WideCharToMultiByte(
        CP_UTF8, 0,
        w, -1,
        &str[0], size,
        nullptr, nullptr
    );

    // Remove Windows null terminator
    if (!str.empty() && str.back() == '\0')
        str.pop_back();

    return str;
}

#ifdef WL_DIST

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR cmdline, int cmdshow)

#else

int wmain(int argc, wchar_t* wargv[])

#endif
{

    s_arg.clear();

    if (__argc < 1)
        return 0;

    s_arg.emplace_back(WCharToString(__wargv[0]));
    for (int i = 1; i < __argc; i++)
    {
        if (std::filesystem::u8path(s_arg[s_arg.size() - 1]).has_extension())
            s_arg.emplace_back(WCharToString(__wargv[i]));
        else
        {
            s_arg[s_arg.size() - 1] += ' ';
            s_arg[s_arg.size() - 1] += WCharToString(__wargv[i]);
        }
    }

#if defined(WL_DIST)

    if (!s_arg.empty())
        std::filesystem::current_path(std::filesystem::u8path(s_arg[0]).parent_path());
#endif
    //fragmentShaderSource = fragment_shader_glsl_130;
	//vertexShaderSource = vertex_shader_glsl_130;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (3.3 core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TITLEBAR, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);


    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        fprintf(stderr, "Failed to get primary monitor\n");
        glfwTerminate();
        return -1;
    }

    // Get the video mode of the monitor
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode) {
        fprintf(stderr, "Failed to get video mode\n");
        glfwTerminate();
        return -1;
    }

    // Create a window
    GLFWwindow* window = glfwCreateWindow(mode->height / 2.5f, mode->height / 2.5f, "Update", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set icon
    GLFWimage icon;
    int channels;

#ifdef WL_DIST

    std::string iconPathStr = "..\\ChessLabApp\\Resources\\ChessLab\\clb.png";
	std::string imagePathStr = "..\\IntallingCL.png";

#else

    std::string iconPathStr = "..\\ChessLab-Lobby\\ChessLabApp\\Resources\\ChessLab\\clb.png";
    std::string imagePathStr = "IntallingCL.png";

#endif

    icon.pixels = stbi_load(iconPathStr.c_str(), &icon.width, &icon.height, &channels, 4);
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);

    // Load image
    int widthImage, heightImage, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(imagePathStr.c_str(), &widthImage, &heightImage, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(false);

    // Get the monitor position (in case it's not at the origin)
    int monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);

    // Get the window size
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    // Calculate the centered position of the window
    int xpos = monitorX + (mode->width - windowWidth) / 2;
    int ypos = monitorY + (mode->height - windowHeight) / 2;

    // Set the window position to the calculated position
    glfwSetWindowPos(window, xpos, ypos);

    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Define vertices and texture coordinates for a rectangle
    float vertices[] = {
        // Positions          // Texture Coords
         1.0f,   1.0f, 0.0f,   1.0f, 1.0f,   // Top Right
          1.0f, -1.0f, 0.0f,   1.0f, 0.0f,   // Bottom Right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,   // Bottom Left
        -1.0f,   1.0f, 0.0f,   0.0f, 1.0f    // Top Left 
    };

    unsigned int indices[] = {
        0, 1, 3,   // First Triangle
        1, 2, 3    // Second Triangle
    };

    glGenerateMipmap(GL_TEXTURE_2D);

    // Vertex Array Object (VAO), Vertex Buffer Object (VBO), and Element Buffer Object (EBO)
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load and create a texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S-axis (x-axis)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T-axis (y-axis)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Create and compile the shader program
    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

    std::atomic<bool> threadEnded = false;
    std::thread updater(
        [&threadEnded]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(6));
    
            std::error_code ec;
            std::filesystem::copy(std::filesystem::current_path() / "ChessLab", std::filesystem::canonical(std::filesystem::current_path() / "..\\..\\ChessLab"), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
                
            threadEnded = true;
        });

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind texture and draw the rectangle
        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (threadEnded)
        {
			updater.join();
            break;
        }
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    Process RunAgain(L"..\\ChessLab.exe", L"");

    return 0;
}
