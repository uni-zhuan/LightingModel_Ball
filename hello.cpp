#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;

// �ӿڵĻص�����,���û��ı䴰�ڵĴ�С��ʱ���ӿ�ҲӦ�ñ�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// �������
void processInput(GLFWwindow* window);
//��д���ص�����
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// ����
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;
//ȫ�ֱ�������ʱ���
float deltaTime = 0.0f; // ��ǰ֡����һ֡��ʱ���
float lastFrame = 0.0f; // ��һ֡��ʱ��
//�洢��һ֡���λ��
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float yaw = 0, pitch = 0;
bool firstMouse = true;

//�ӽ��ƶ�
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::mat4 view;

// ���嶥����ɫ��,��ɫ������GLSL
//Uniform��һ�ִ�CPU�е�Ӧ����GPU�е���ɫ���������ݵķ�ʽ,ȫ��,�κ���ɫ�����ɶ���
const char* vertexShaderSource = "#version 330 core\n"//�汾����
"layout (location = 0) in vec3 aPos;\n"//����ÿ�����㶼��һ��3D���꣬����һ��vec3�������aPos,layout�趨���������λ��ֵ
"layout (location = 1) in vec3 aNormal;\n"//���¹��յĶ�����ɫ��
"out vec3 FragPos;\n"
"out vec3 Normal;\n"//���������
"uniform mat4 model;\n"//ģ�;���,����������Ӿֲ��任������ռ�,�任����
"uniform mat4 view;\n"//�۲����,��ʶ������任���۲�ռ�
"uniform mat4 projection;\n"//ͶӰ����,��������ӹ۲�任���ü��ռ�
"void main()\n"
"{\n"
"   FragPos = vec3(model * vec4(aPos, 1.0));\n"
"   Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
"}\0";

// ����Ƭ����ɫ��
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal;\n"//���������
"in vec3 FragPos;\n"//Ƭ�ε�λ��
"uniform vec3 lightPos;\n"//��Դ��λ��,��Ϊ��̬����
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 objectColor;\n"
"void main()\n"
"{\n"
// ��������
"   float ambientStrength = 0.1;\n"
"   vec3 ambient = ambientStrength * lightColor;\n"
// ���������
"   vec3 norm = normalize(Normal);\n"//��׼������
"   vec3 lightDir = normalize(lightPos - FragPos);\n"//��׼����������,����ߵķ�������ʵ�������Դλ�ú�Ƭ��λ�õ�������
//��norm��lightDir�������е�ˣ������Դ�Ե�ǰƬ��ʵ�ʵ�������Ӱ�졣���ֵ�ٳ��Թ����ɫ���õ��������������������֮��ĽǶ�Խ������������ͻ�ԽС,max��֤�������������ɸ���
"   float diff = max(dot(norm, lightDir), 0.0);\n"
"   vec3 diffuse = diff * lightColor;\n"
// �������
"   float specularStrength = 1.0f;\n"//���徵��ǿ�ȱ���
"   vec3 viewDir = normalize(viewPos - FragPos);\n"//�������߷�������
"   vec3 reflectDir = reflect(-lightDir, norm);"//�����Ӧ�����ŷ�����ķ�������
"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"//�������߷����뷴�䷽��ĵ�ˣ���ȷ�������Ǹ�ֵ����Ȼ��ȡ����32���ݡ����32�Ǹ߹�ķ����
"   vec3 specular = specularStrength * spec * lightColor;\n"//�����ӵ�������������������������ý�������������ɫ
// ���_phong
"   vec3 result = (ambient + diffuse + specular) * objectColor;\n"
//���_lambert
//"   vec3 result = (ambient + diffuse ) * objectColor;\n"
"   FragColor = vec4(result, 1.0);\n"
"}\0";

// ����,��������
glm::vec3 lightPos(0.6f, 0.5f, 1.0f);

int main()
{
    // ʵ����GLFW����
    glfwInit();//����glfwInit��������ʼ��GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//���汾�ţ�glfwWindowHint����������GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�ΰ汾��
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����ģʽ

    // �������ڶ���
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "A Simple Ball", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //����GLFW����Ӧ�����ع�꣬����׽(Capture)��
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ��ʼ��glad,GLAD����������OpenGL�ĺ���ָ��ģ������ڵ����κ�OpenGL�ĺ���֮ǰ����Ҫ��ʼ��GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ������Ȳ���
    //���ֵ�洢��ÿ��Ƭ�����棨��ΪƬ�ε�zֵ������Ƭ����Ҫ���������ɫʱ��OpenGL�Ὣ�������ֵ��z������бȽϣ������ǰ��Ƭ��������Ƭ��֮�������ᱻ���������򽫻Ḳ�ǡ�
    glEnable(GL_DEPTH_TEST);

    // ������������
    int nSlices = 50; // ����
    int nStacks = 50; // γ��
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
    // ����������������
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

    // ���嶥�㻺�����
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    // ���嶥���������
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    // ���������������
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // �󶨶��㻺�����
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // ���ƶ������ݵ������ڴ�
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    // �󶨶����������
    glBindVertexArray(VAO);

    // �������������
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //�Ѷ������鸴�Ƶ������й�OpenGLʹ��
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);


    // ����������ɫ������,��3D����תΪ��һ��3D����
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // ���Ӷ�����ɫ�����뵽������ɫ������
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // ���붥����ɫ��
    glCompileShader(vertexShader);

    // ����Ƭ����ɫ������,����һ�����ص�������ɫ
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // ����Ƭ����ɫ�����뵽Ƭ����ɫ������
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // ����Ƭ����ɫ��
    glCompileShader(fragmentShader);

    // ����һ����ɫ���������
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    // ����ɫ�����󸽼ӵ�������
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // ������ɫ������
    glLinkProgram(shaderProgram);

    // ��ɫ���������ӳɹ������ɾ����
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // λ������,����OpenGL����ν�����������
    //��һ������ָ������Ҫ���õĶ�������;�ڶ�������ָ���������ԵĴ�С;����������ָ�����ݵ�����;�¸��������������Ƿ�ϣ�����ݱ���׼��;�����������������;��ʾλ�������ڻ�������ʼλ�õ�ƫ����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //�Զ�������λ��ֵ��Ϊ���������ö�������
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);



    // �����ӿ�
    //ǰ�����������ƴ������½ǵ�λ�á��������͵��ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶�
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // ע��ص�����,����GLFWϣ��ÿ�����ڵ�����С��ʱ�����
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    // ��Ⱦѭ��,���������������ر���֮ǰ���ϻ���ͼ���ܹ������û�����
    //glfwWindowShouldClose����������ÿ��ѭ���Ŀ�ʼǰ���һ��GLFW�Ƿ�Ҫ���˳�
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        //ÿһ֡�����Ǽ�����µ�deltaTime�Ա�����
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //������glClearColor�����������Ļ���õ���ɫ
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //glClear�����������Ļ����ɫ����,ָ��DEPTH_BUFFER_BITλ�������Ȼ���
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ������ɫ���������
        glUseProgram(shaderProgram);


        // �󶨶�������
        glBindVertexArray(VAO);

        //// 0. ���ƶ������鵽�����й�OpenGLʹ��
        // �������������
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        // ʹ���߿�ģʽ
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // ʹ�����ģʽ��Ĭ�ϣ�
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        /*lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;// 0. ���ƶ������鵽�����й�OpenGLʹ��
        lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;*/

        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.24f, 0.51f, 0.71f);//������ɫ
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);//��Դ��ɫ
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &glm::vec3(0.0f, 0.0f, 3.0f)[0]);

        // ����ģ�;���
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(30.0f), glm::vec3(0.5f, 1.0f, 0.0f));//������ʱ����ת

        // ����۲����
        //glm::mat4 view = glm::mat4(1.0f);
        // ע�⣬���ǽ�����������Ҫ�����ƶ������ķ������ƶ���
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));


        // ����͸��ͶӰ����,��ʹ��͸�ӷ������������˿��ӿռ�Ĵ�ƽ��ͷ����вü�
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        //processInput(window);

        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
        // ���������ͳһλ��
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        // �����Ǵ��ݸ���ɫ��,ͨ����ÿ�ε���Ⱦ�����н��У���Ϊ�任����ᾭ���䶯
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);


        // ����,glDrawElements���滻glDrawArrays��������ָ�����Ǵ�����������Ⱦ
        glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, elements);
        //glfwSwapBuffers�����ύ����ɫ���壨����һ��������GLFW����ÿһ��������ɫֵ�Ĵ󻺳壩��������һ�����б��������ƣ����ҽ�����Ϊ�����ʾ����Ļ��
        glfwSwapBuffers(window);
        //glfwPollEvents���������û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬�������ö�Ӧ�Ļص�����
        glfwPollEvents();
    }

    // �ͷ���Դ
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // �ͷ�֮ǰ�������Դ
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
