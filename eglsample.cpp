#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include "myegl.h"
#include "mypng.h"

#define degree2radian(degree) ((degree * M_PI) / 180.0F)

GLuint loadShader(GLenum shaderType, const char *source)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        std::cerr << "Error glCompileShader." << std::endl;
        exit(EXIT_FAILURE);
    }
    return shader;
}

GLuint createProgram(const char *vshader, const char *fshader)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fshader);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        std::cerr << "Error glLinkProgram." << std::endl;
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    return program;
}

void deleteShaderProgram(GLuint shaderProgram)
{
    glDeleteProgram(shaderProgram);
}

void mainloop(Display *xdisplay, EGLDisplay display, EGLSurface surface)
{
    const char *vshader = R"(
        attribute vec4 vPosition;
        attribute vec4 vColor;
        varying vec4 color;
        uniform mediump mat4 mRotation;
        void main() {
            gl_Position = mRotation * vPosition;
            color = vColor;
        }
    )";

    const char *fshader = R"(
        precision mediump float;
        varying vec4 color;
        void main() {
            gl_FragColor = color;
        }
    )";

    GLuint program = createProgram(vshader, fshader);
    glUseProgram(program);
    const GLfloat vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
    const GLfloat vertex_color[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f};
    GLint gvPositionHandle = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(gvPositionHandle);
    GLint gvColorLocation = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(gvColorLocation);
    GLint gmRotationHandle = glGetUniformLocation(program, "mRotation");
    int degree = 0;
    while (true)
    {
        XPending(xdisplay);

        const GLfloat matrix[] = {
            static_cast<GLfloat>(cos(degree2radian(degree))), 0.0f, static_cast<GLfloat>(sin(degree2radian(degree))), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            static_cast<GLfloat>(-sin(degree2radian(degree))), 0.0f, static_cast<GLfloat>(cos(degree2radian(degree))), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f};

        glClearColor(0.25f, 0.25f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(gvColorLocation, 4, GL_FLOAT, GL_FALSE, 0, vertex_color);
        glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        eglSwapBuffers(display, surface);
        degree = (degree + 1) % 360;
        usleep(16600);
    }
    deleteShaderProgram(program);
}

void mainloopPng(Display *xdisplay, EGLDisplay display, EGLSurface surface)
{
    const char *vshader = R"(
        attribute vec4 position;
        attribute vec2 texcoord;
        varying vec2 texcoordVarying;
        void main() {
            gl_Position = position;
            texcoordVarying = texcoord;
        }
    )";

    const char *fshader = R"(
        precision mediump float;
        varying vec2 texcoordVarying;
        uniform sampler2D texture;
        void main() {
            gl_FragColor = texture2D(texture, texcoordVarying);
            gl_FragColor.a = 0.5;
        }
    )";

    GLuint program = createProgram(vshader, fshader);
    glUseProgram(program);

    auto png = loadPng("../sample.png");
    GLuint position = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position);
    GLuint texcoord = glGetAttribLocation(program, "texcoord");
    glEnableVertexAttribArray(texcoord);

    GLuint textures[] = {static_cast<GLuint>(glGetUniformLocation(program, "texture"))};
    glGenTextures(1, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, png.has_alpha ? GL_RGBA : GL_RGB, png.width, png.height,
                 0, png.has_alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, png.data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    const float vertices[] = {-1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f};
    const float texcoords[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    while (true)
    {
        XPending(xdisplay);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.25f, 0.25f, 0.5f, 1.0f);

        glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
        glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        eglSwapBuffers(display, surface);
        usleep(1000);
    }

    deletePng(png);
    deleteShaderProgram(program);
}

int main(int argc, char *argv[])
{
    Display *xdisplay = XOpenDisplay(nullptr);
    if (xdisplay == nullptr)
    {
        std::cerr << "Error XOpenDisplay." << std::endl;
        exit(EXIT_FAILURE);
    }

    Window xwindow = XCreateSimpleWindow(xdisplay, DefaultRootWindow(xdisplay), 100, 100, 640, 480, 1, BlackPixel(xdisplay, 0), WhitePixel(xdisplay, 0));

    XMapWindow(xdisplay, xwindow);

    EGLDisplay display = nullptr;
    EGLContext context = nullptr;
    EGLSurface surface = nullptr;
    if (initializeEGL(xdisplay, xwindow, display, context, surface) < 0)
    {
        std::cerr << "Error initializeEGL." << std::endl;
        exit(EXIT_FAILURE);
    }

#if LOAD_PNG
    mainloopPng(xdisplay, display, surface);
#else
    mainloop(xdisplay, display, surface);
#endif

    destroyEGL(display, context, surface);
    XDestroyWindow(xdisplay, xwindow);
    XCloseDisplay(xdisplay);

    return 0;
}
