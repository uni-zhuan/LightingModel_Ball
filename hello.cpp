#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;

// 视口的回调函数,当用户改变窗口的大小的时候，视口也应该被调整
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// 输入控制
void processInput(GLFWwindow* window);
//重写鼠标回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// 设置
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;
//全局变量跟踪时间差
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
//存储上一帧鼠标位置
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float yaw = 0, pitch = 0;
bool firstMouse = true;

//视角移动
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 view;

// 定义顶点着色器,着色器语言GLSL
//Uniform是一种从CPU中的应用向GPU中的着色器发送数据的方式,全局,任何着色器都可定义
const char* vertexShaderSource = "#version 330 core\n"//版本声明
"layout (location = 0) in vec3 aPos;\n"//由于每个顶点都有一个3D坐标，创建一个vec3输入变量aPos,layout设定输入变量的位置值
"layout (location = 1) in vec3 aNormal;\n"//更新光照的顶点着色器
"out vec3 FragPos;\n"
"out vec3 Normal;\n"//漫反射光照
"uniform mat4 model;\n"//模型矩阵,将物体坐标从局部变换到世界空间,变换矩阵
"uniform mat4 view;\n"//观察矩阵,将识别坐标变换到观察空间
"uniform mat4 projection;\n"//投影矩阵,顶点坐标从观察变换到裁剪空间
"void main()\n"
"{\n"
"   FragPos = vec3(model * vec4(aPos, 1.0));\n"
"   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
"}\0";

// 定义片段着色器
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal;\n"//漫反射光照
"in vec3 FragPos;\n"//片段的位置
"uniform vec3 lightPos;\n"//光源的位置,作为静态变量
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 objectColor;\n"
"void main()\n"
"{\n"
// 环境光照
"   float ambientStrength = 0.1;\n"
"   vec3 ambient = ambientStrength * lightColor;\n"
// 漫反射光照
"   vec3 norm = normalize(Normal);\n"//标准化法线
"   vec3 lightDir = normalize(lightPos - FragPos);\n"//标准化方向向量,求光线的方向向量实际是求光源位置和片段位置的向量差
//对norm和lightDir向量进行点乘，计算光源对当前片段实际的漫发射影响。结果值再乘以光的颜色，得到漫反射分量。两个向量之间的角度越大，漫反射分量就会越小,max保证漫反射分量不变成负数
"   float diff = max(dot(norm, lightDir), 0.0);\n"
"   vec3 diffuse = diff * lightColor;\n"
// 镜面光照
"   float specularStrength = 1.0f;\n"//定义镜面强度变量
"   vec3 viewDir = normalize(viewPos - FragPos);\n"//计算视线方向向量
"   vec3 reflectDir = reflect(-lightDir, norm);"//计算对应的沿着法线轴的反射向量
"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"//计算视线方向与反射方向的点乘（并确保它不是负值），然后取它的32次幂。这个32是高光的反光度
"   vec3 specular = specularStrength * spec * lightColor;\n"//把它加到环境光分量和漫反射分量里，再用结果乘以物体的颜色
// 结果_phong
"   vec3 result = (ambient + diffuse + specular) * objectColor;\n"
//结果_lambert
//"   vec3 result = (ambient + diffuse ) * objectColor;\n"
"   FragColor = vec4(result, 1.0);\n"
"}\0";

// 光照,做个输入
glm::vec3 lightPos(0.6f, 0.5f, 1.0f);

int main()
{
    // 实例化GLFW窗口
    glfwInit();//调用glfwInit函数来初始化GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//主版本号，glfwWindowHint函数来配置GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//核心模式

    // 创建窗口对象
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "A Simple Ball", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //告诉GLFW，它应该隐藏光标，并捕捉(Capture)它
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化glad,GLAD是用来管理OpenGL的函数指针的，所以在调用任何OpenGL的函数之前都需要初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 启用深度测试
    //深度值存储在每个片段里面（作为片段的z值），当片段想要输出它的颜色时，OpenGL会将它的深度值和z缓冲进行比较，如果当前的片段在其它片段之后，它将会被丢弃，否则将会覆盖。
    glEnable(GL_DEPTH_TEST);

    // 定义球体坐标
    int nSlices = 50; // 经线
    int nStacks = 50; // 纬线
    int nVerts = (nStacks + 1) * (nSlices + 1);
    int elements = nSlices * nStacks * 6;
    float theta, phi;
    float thetaFac = 3.14f * 2.0f / nSlices;
    float phiFac = 3.14f * 1.0f / nStacks;
    float nx, ny, nz;
    int idx = 0;
    float sphere_vertices[51 * 51 * 3];
    int sphere_indices[50 * 50 * 6];
    for (int i = 0; i <= nSlices; i++)
    {
        theta = i * thetaFac;
        for (int j = 0; j <= nStacks; j++)
        {
            phi = j * phiFac;
            nx = sinf(phi) * cosf(theta);
            ny = sinf(phi) * sinf(theta);
            nz = cosf(phi);
            sphere_vertices[idx * 3] = 0.5f * nx;
            sphere_vertices[idx * 3 + 1] = 0.5f * ny;
            sphere_vertices[idx * 3 + 2] = 0.5f * nz;
            idx++;
        }
    }
    // 定义球体坐标索引
    int indx = 0;
    for (int i = 0; i < nStacks; i++)
    {
        for (int j = 0; j < nSlices; j++)
        {
            int i0 = i * (nSlices + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (nSlices + 1);
            int i3 = i2 + 1;
            if ((j + i) % 2 == 1) {
                sphere_indices[indx] = i0;
                sphere_indices[indx + 1] = i2;
                sphere_indices[indx + 2] = i1;
                sphere_indices[indx + 3] = i1;
                sphere_indices[indx + 4] = i2;
                sphere_indices[indx + 5] = i3;
                indx += 6;
            }
            else
            {
                sphere_indices[indx] = i0;
                sphere_indices[indx + 1] = i2;
                sphere_indices[indx + 2] = i3;
                sphere_indices[indx + 3] = i0;
                sphere_indices[indx + 4] = i3;
                sphere_indices[indx + 5] = i1;
                indx += 6;
            }
        }

    }

    // 定义顶点缓冲对象
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    // 定义顶点数组对象
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    // 定义索引缓冲对象
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 复制顶点数据到缓冲内存
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);

    // 绑定索引缓冲对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //把顶点数组复制到缓冲中供OpenGL使用
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);


    // 创建顶点着色器对象,把3D坐标转为另一种3D坐标
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // 附加顶点着色器代码到顶点着色器对象
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 创建片段着色器对象,计算一个像素的最终颜色
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // 附加片段着色器代码到片段着色器对象
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 创建一个着色器程序对象
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    // 把着色器对象附加到程序上
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 着色器对象链接成功后可以删掉了
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 位置属性,告诉OpenGL该如何解析顶点数据
    //第一个参数指定我们要配置的顶点属性;第二个参数指定顶点属性的大小;第三个参数指定数据的类型;下个参数定义我们是否希望数据被标准化;第五个参数叫做步长;表示位置数据在缓冲中起始位置的偏移量
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //以顶点属性位置值作为参数，启用顶点属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);



    // 定义视口
    //前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // 注册回调函数,告诉GLFW希望每当窗口调整大小的时候调用
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    // 渲染循环,程序在我们主动关闭它之前不断绘制图像并能够接受用户输入
    //glfwWindowShouldClose函数在我们每次循环的开始前检查一次GLFW是否被要求退出
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        //每一帧中我们计算出新的deltaTime以备后用
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //调用了glClearColor来设置清空屏幕所用的颜色
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //glClear函数来清空屏幕的颜色缓冲,指定DEPTH_BUFFER_BIT位来清除深度缓冲
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器程序对象
        glUseProgram(shaderProgram);


        // 绑定顶点数组
        glBindVertexArray(VAO);

        //// 0. 复制顶点数组到缓冲中供OpenGL使用
        // 绑定索引缓冲对象
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        // 使用线框模式
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // 使用填充模式（默认）
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        /*lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;// 0. 复制顶点数组到缓冲中供OpenGL使用
        lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;*/

        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.24f, 0.51f, 0.71f);//物体颜色
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);//光源颜色
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &glm::vec3(0.0f, 0.0f, 3.0f)[0]);

        // 定义模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(30.0f), glm::vec3(0.5f, 1.0f, 0.0f));//球体随时间旋转

        // 定义观察矩阵
        //glm::mat4 view = glm::mat4(1.0f);
        // 注意，我们将矩阵向我们要进行移动场景的反方向移动。
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));


        // 定义透视投影矩阵,即使用透视方法创建定义了可视空间的大平截头体进行裁剪
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        //processInput(window);

        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
        // 检索矩阵的统一位置
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        // 将它们传递给着色器,通常在每次的渲染迭代中进行，因为变换矩阵会经常变动
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);


        // 绘制,glDrawElements来替换glDrawArrays函数，来指明我们从索引缓冲渲染
        glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, elements);
        //glfwSwapBuffers函数会交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色值的大缓冲），它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上
        glfwSwapBuffers(window);
        //glfwPollEvents函数检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数
        glfwPollEvents();
    }

    // 释放资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // 释放之前分配的资源
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraSpeed = 3.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        //cout << "false";
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 20.0f)
        pitch = 20.0f;
    if (pitch < -20.0f)
        pitch = -20.0f;
    if (yaw > 300.0f)
        yaw = 300.0f;
    if (yaw < 240.0f)
        yaw = 240.0f;
    //yaw = 270.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    //cout << "move";
}
