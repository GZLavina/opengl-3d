/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 12/08/2024
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void mouse_callback(GLFWwindow* window, double xPos, double yPos);

// Protótipos das funções
int setupGeometry();
int loadSimpleOBJ(string filePATH, int &nVertices);


// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX=false, rotateY=false, rotateZ=false;


//Variáveis globais da câmera
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);

struct Object
{
    GLuint VAO; //Índice do buffer de geometria
    int nVertices; //nro de vértices
    glm::mat4 model; //matriz de transformações do objeto
};

glm::vec3 objectScale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);

bool firstMouse = true;
float lastX = 500, lastY = 500;
float yaw = 0.0f;
float pitch = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int objectVectorIndex = 0;

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();

    //Muita atenção aqui: alguns ambientes não aceitam essas configurações
    //Você deve adaptar para a versão do OpenGL suportada por sua placa
    //Sugestão: comente essas linhas de código para desobrir a versão e
    //depois atualize (por exemplo: 4.5 com 4 e 5)
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Rossana!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    glfwSetCursorPosCallback(window, mouse_callback);

    // GLAD: carrega todos os ponteiros d funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;

    }

    // Obtendo as informações de versão
    const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader shader("../../shaders/phong.vert", "../../shaders/phong.frag");

    std::vector<Object> objectVector;

    Object obj;
    obj.VAO = loadSimpleOBJ("./../../Suzanne.obj",obj.nVertices);
    objectVector.push_back(obj);

    Object obj2;
    obj2.VAO = loadSimpleOBJ("./../../Nave.obj",obj.nVertices);
    objectVector.push_back(obj2);


    shader.use();


    //Matriz de modelo
    glm::mat4 model = glm::mat4(1); //matriz identidade;
    GLint modelLoc = glGetUniformLocation(shader.ID, "model");
    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //Matriz de view
    glm::mat4 view = glm::lookAt(cameraPos,glm::vec3(0.0f,0.0f,0.0f),cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    //Matriz de projeção
    //glm::mat4 projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);
    glm::mat4 projection = glm::perspective(glm::radians(39.6f),(float)WIDTH/HEIGHT,0.1f,100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST);

    // Surface
    shader.setFloat("ka", 0.2f);
    shader.setFloat("ks", 0.5f);
    shader.setFloat("kd", 0.5f);
    shader.setFloat("q", 10.f);

    // Light
    shader.setVec3("lightPos", -2.0f, -10.0f, -3.0f);
    shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        auto angle = (GLfloat)glfwGetTime();

        obj.model = glm::mat4(1); //matriz identidade

        obj.model = glm::translate(obj.model, objectPos);
        obj.model = glm::scale(obj.model, objectScale);

        if (rotateX)
        {
            obj.model = glm::rotate(obj.model, angle, glm::vec3(1.0f, 0.0f, 0.0f));

        }
        else if (rotateY)
        {
            obj.model = glm::rotate(obj.model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        }
        else if (rotateZ)
        {
            obj.model = glm::rotate(obj.model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        }

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));

        //Atualizar a matriz de view
        //Matriz de view
        glm::mat4 view = glm::lookAt(cameraPos,cameraPos + cameraFront,cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

        shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);


        // Chamada de desenho - drawcall
        // Poligono Preenchido - GL_TRIANGLES
        glBindVertexArray(objectVector.at(abs(int (objectVectorIndex % objectVector.size()))).VAO);
        glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);


        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    glDeleteVertexArrays(1, &obj.VAO);
    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    //Verifica a movimentação da câmera
    float cameraSpeed = 0.05f;

    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraUp;
    }

    float scaleChangeRate = 0.05f;
    if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
        objectScale += scaleChangeRate;
    }
    if (key == GLFW_KEY_END && action == GLFW_PRESS) {
        objectScale -= scaleChangeRate;
    }

    // Translação do objeto
    float translationChangeRate = 0.1f;
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        objectPos.y += translationChangeRate;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        objectPos.y -= translationChangeRate;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        objectPos.x += translationChangeRate;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        objectPos.x -= translationChangeRate;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        objectPos.z += translationChangeRate;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        objectPos.z -= translationChangeRate;
    }

    // Seleção de objeto
    if (key == GLFW_KEY_COMMA && action == GLFW_PRESS) {
        objectVectorIndex--;
    }
    if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {
        objectVectorIndex++;
    }
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = (float) xPos;
        lastY = (float) yPos;
        firstMouse = false;
    }

    auto xOffset = float (xPos - lastX);
    auto yOffset = float (lastY - yPos);
    lastX = (float) xPos;
    lastY = (float) yPos;

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
    // Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
    // sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
    // Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
    // Pode ser arazenado em um VBO único ou em VBOs separados
    GLfloat vertices[] = {

            //Base da pirâmide: 2 triângulos
            //x    y    z    r    g    b
            -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
            -0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
            0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

            -0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
            0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
            0.5, -0.5, -0.5, 1.0, 0.0, 1.0,

            //
            -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
            0.0,  0.5,  0.0, 1.0, 1.0, 0.0,
            0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

            -0.5, -0.5, -0.5, 1.0, 0.0, 1.0,
            0.0,  0.5,  0.0, 1.0, 0.0, 1.0,
            -0.5, -0.5, 0.5, 1.0, 0.0, 1.0,

            -0.5, -0.5, 0.5, 1.0, 1.0, 0.0,
            0.0,  0.5,  0.0, 1.0, 1.0, 0.0,
            0.5, -0.5, 0.5, 1.0, 1.0, 0.0,

            0.5, -0.5, 0.5, 0.0, 1.0, 1.0,
            0.0,  0.5,  0.0, 0.0, 1.0, 1.0,
            0.5, -0.5, -0.5, 0.0, 1.0, 1.0,


    };

    GLuint VBO, VAO;

    //Geração do identificador do VBO
    glGenBuffers(1, &VBO);

    //Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);

    //Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
    // Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
    // Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
    // Tipo do dado
    // Se está normalizado (entre zero e um)
    // Tamanho em bytes
    // Deslocamento a partir do byte zero

    //Atributo posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //Atributo cor (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);


    // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
    // atualmente vinculado - para que depois possamos desvincular com segurança
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

int loadSimpleOBJ(string filePath, int &nVertices)
{
    vector <glm::vec3> vertices;
    vector <glm::vec2> texCoords;
    vector <glm::vec3> normals;
    vector <GLfloat> vBuffer;

    glm::vec3 color = glm::vec3(1.0, 0.0, 0.0);

    ifstream arqEntrada;

    arqEntrada.open(filePath.c_str());
    if (arqEntrada.is_open())
    {
        //Fazer o parsing
        string line;
        while (!arqEntrada.eof())
        {
            getline(arqEntrada,line);
            istringstream ssline(line);
            string word;
            ssline >> word;
            if (word == "v")
            {
                glm::vec3 vertice;
                ssline >> vertice.x >> vertice.y >> vertice.z;
                //cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
                vertices.push_back(vertice);

            }
            if (word == "vt")
            {
                glm::vec2 vt;
                ssline >> vt.s >> vt.t;
                //cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
                texCoords.push_back(vt);

            }
            if (word == "vn")
            {
                glm::vec3 normal;
                ssline >> normal.x >> normal.y >> normal.z;
                //cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
                normals.push_back(normal);

            }
            else if (word == "f")
            {
                while (ssline >> word)
                {
                    int vi, ti, ni;
                    istringstream ss(word);
                    std::string index;

                    // Pega o índice do vértice
                    std::getline(ss, index, '/');
                    vi = std::stoi(index) - 1;  // Ajusta para índice 0

                    // Pega o índice da coordenada de textura
                    std::getline(ss, index, '/');
                    ti = std::stoi(index) - 1;

                    // Pega o índice da normal
                    std::getline(ss, index);
                    ni = std::stoi(index) - 1;

                    //Recuperando os vértices do indice lido
                    vBuffer.push_back(vertices[vi].x);
                    vBuffer.push_back(vertices[vi].y);
                    vBuffer.push_back(vertices[vi].z);

                    //Atributo cor
                    vBuffer.push_back(color.r);
                    vBuffer.push_back(color.g);
                    vBuffer.push_back(color.b);

                    vBuffer.push_back(texCoords[ti].s);
                    vBuffer.push_back(texCoords[ti].t);

                    vBuffer.push_back(normals[ni].x);
                    vBuffer.push_back(normals[ni].y);
                    vBuffer.push_back(normals[ni].z);


                    // Exibindo os índices para verificação
                    // std::cout << "v: " << vi << ", vt: " << ti << ", vn: " << ni << std::endl;
                }

            }
        }

        arqEntrada.close();

        cout << "Gerando o buffer de geometria..." << endl;
        GLuint VBO, VAO;

        //Geração do identificador do VBO
        glGenBuffers(1, &VBO);

        //Faz a conexão (vincula) do buffer como um buffer de array
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //Envia os dados do array de floats para o buffer da OpenGl
        glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

        //Geração do identificador do VAO (Vertex Array Object)
        glGenVertexArrays(1, &VAO);

        // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
        // e os ponteiros para os atributos
        glBindVertexArray(VAO);

        //Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
        // Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
        // Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
        // Tipo do dado
        // Se está normalizado (entre zero e um)
        // Tamanho em bytes
        // Deslocamento a partir do byte zero

        //Atributo posição (x, y, z)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        //Atributo cor (r, g, b)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        // texCoords (s, t)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        // vetor normal (x, y, z)
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8*sizeof(GLfloat)));
        glEnableVertexAttribArray(3);

        // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
        // atualmente vinculado - para que depois possamos desvincular com segurança
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
        glBindVertexArray(0);

        nVertices = vBuffer.size() / 2;
        return VAO;

    }
    else
    {
        cout << "Erro ao tentar ler o arquivo " << filePath << endl;
        return -1;
    }
}
