#include "./glad/glad.h"
#include <GLFW/glfw3.h>
#include "./gl/shader_m.h"

namespace custom_gl
{
  class gl_viewer
  {
    public:
      GLFWwindow *window;
      unsigned int VAO, VBO, EBO;
      void view(size_t const &size);
      void preset();
      void buffer_ebo(size_t const &size, unsigned int const * ebo = nullptr);
      void buffer_vbo(size_t const &size, float const * vbo = nullptr);
      gl_viewer(bool hide_window = false);
      ~gl_viewer();
  };

  gl_viewer::gl_viewer(bool hide_window)
  {
    std::cout << "constructor\n";
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (hide_window)
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(800, 600, "", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
 
    Shader shader("shader.vs", "shader.fs");
    shader.use();
  
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
  }

  void gl_viewer::view(size_t const &size)
  {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_LINES, size, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    return;
  }

  void gl_viewer::preset()
  {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    return;
  }

  void gl_viewer::buffer_ebo(size_t const &size, unsigned int const * ebo)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*size, ebo, GL_STATIC_DRAW);
    return;
  }

  void gl_viewer::buffer_vbo(size_t const &size, float const * vbo)
  {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*size, vbo, GL_DYNAMIC_DRAW);
    return;
  }

  gl_viewer::~gl_viewer()
  {
    std::cout << "destructor\n";
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
  }
}
