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

#include "stb_image.h"
#include "json.hpp"

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

using namespace std;
using json = nlohmann::json;

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void mouse_callback(GLFWwindow *window, double xPos, double yPos);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX = false, rotateY = false, rotateZ = false;


//Variáveis globais da câmera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

struct Curve {
    std::vector<glm::vec3> controlPoints; // Pontos de controle da curva
    std::vector<glm::vec3> curvePoints;   // Pontos da curva
    glm::mat4 M;                          // Matriz dos coeficientes da curva
};

struct Object {
    GLuint VAO; //Índice do buffer de geometria
    GLuint texID;
    int nVertices; //nro de vértices
    glm::mat4 model; //matriz de transformações do objeto
    float ka, kd, ks, q;
    glm::vec3 scale;
    glm::vec3 pos;
    glm::vec3 rotationDegrees;
    bool hasCurve = false;
    Curve curve;
    int curvePointIndex = 0;
    float lastCurveUpdate = 0.0f;
    float updateRate = 0.05f;
    float curveAngle;
};

glm::vec3 objectScale = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);

bool firstMouse = true;
float lastX = 500, lastY = 500;
float yaw = 0.0f;
float pitch = 0.0f;

glm::vec3 lightPos;
glm::vec3 lightColor;

int objectVectorIndex = 0;
std::vector<Object> objectVector;

// Protótipos das funções
int loadSimpleOBJ(string filePATH, int &nVertices);

GLuint loadTexture(string filePath, int &width, int &height);

void loadSceneFromJson(const string &filePath);

void loadSimpleMtl(string filePath, Object &obj);

void initializeCatmullRomMatrix(glm::mat4x4 &matrix);

void generateCatmullRomCurvePoints(Curve &curve, int numPoints);

std::vector<glm::vec3> generateCurveControlPoints(int numPoints = 20);

// Função MAIN
int main() {
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
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Rossana!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    glfwSetCursorPosCallback(window, mouse_callback);

    // GLAD: carrega todos os ponteiros d funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;

    }

    // Obtendo as informações de versão
    const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte *version = glGetString(GL_VERSION); /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader shader("../../shaders/phong.vert", "../../shaders/phong.frag");

    loadSceneFromJson("../../scene.json");

    // Curvas paramétricas
    Curve catmullRom;
    std::vector<glm::vec3> controlPoints = generateCurveControlPoints();

    catmullRom.controlPoints.push_back(controlPoints[0]);
    for (auto controlPoint: controlPoints) {
        catmullRom.controlPoints.push_back(controlPoint);
    }
    catmullRom.controlPoints.push_back(controlPoints[controlPoints.size() - 1]);

    int numCurvePoints = 100;

    generateCatmullRomCurvePoints(catmullRom, 20);

    objectVector[0].curve = catmullRom;
    objectVector[0].hasCurve = true;
    objectVector[0].pos = catmullRom.curvePoints[0];
    glm::vec3 nextPos = objectVector[0].curve.curvePoints[1];
    glm::vec3 dir = glm::normalize(nextPos - objectVector[0].pos);
    objectVector[0].curveAngle = atan2(dir.y, dir.x) + glm::radians(-90.0f);

    shader.use();

    GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 projection = glm::perspective(glm::radians(39.6f), (float) WIDTH / HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //Buffer de textura no shader
    glUniform1i(glGetUniformLocation(shader.ID, "texBuffer"), 0);

    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Light
    shader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
    shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window)) {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Objects
        for (auto &obj: objectVector) {
            obj.model = glm::mat4(1); //matriz identidade

            if (obj.hasCurve) {
                obj.pos = obj.curve.curvePoints[obj.curvePointIndex];
                auto time = (GLfloat) glfwGetTime();
                auto dt = time - obj.lastCurveUpdate;
                if (dt > obj.updateRate) {
                    obj.curvePointIndex = (obj.curvePointIndex + 1) % ((int) obj.curve.curvePoints.size());
                    obj.lastCurveUpdate = time;
                    glm::vec3 nextPos = obj.curve.curvePoints[obj.curvePointIndex];
                    glm::vec3 dir = glm::normalize(nextPos - obj.pos);
                    obj.curveAngle = atan2(dir.y, dir.x) + glm::radians(-90.0f);
                }
            }

            obj.model = glm::translate(obj.model, obj.pos);
            obj.model = glm::scale(obj.model, obj.scale);
            obj.model = glm::rotate(obj.model, glm::radians(obj.rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
            obj.model = glm::rotate(obj.model, glm::radians(obj.rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
            obj.model = glm::rotate(obj.model, glm::radians(obj.rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
            obj.model = glm::rotate(obj.model, obj.curveAngle, glm::vec3(0.0f, 0.0f, 1.0f));

            // Surface
            shader.setFloat("ka", obj.ka);
            shader.setFloat("ks", obj.ks);
            shader.setFloat("kd", obj.kd);
            shader.setFloat("q", obj.q);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(obj.model));
            glBindVertexArray(obj.VAO);
            glBindTexture(GL_TEXTURE_2D, obj.texID);
            glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);
        }

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    for (auto &obj: objectVector) {
        glDeleteVertexArrays(1, &obj.VAO);
    }
    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    Object &obj = objectVector.at(abs(int(objectVectorIndex % objectVector.size())));

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        obj.rotationDegrees.x += 5.f;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        obj.rotationDegrees.y += 5.f;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        obj.rotationDegrees.z += 5.f;
    }

    //Verifica a movimentação da câmera
    float cameraSpeed = 0.05f;

    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraUp;
    }

    float scaleChangeRate = 0.05f;
    if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
        obj.scale += scaleChangeRate;
    }
    if (key == GLFW_KEY_END && action == GLFW_PRESS) {
        obj.scale -= scaleChangeRate;
    }

    // Translação do objeto
    float translationChangeRate = 0.1f;
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        obj.pos.y += translationChangeRate;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        obj.pos.y -= translationChangeRate;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        obj.pos.x += translationChangeRate;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        obj.pos.x -= translationChangeRate;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        obj.pos.z += translationChangeRate;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        obj.pos.z -= translationChangeRate;
    }

    // Seleção de objeto
    if (key == GLFW_KEY_COMMA && action == GLFW_PRESS) {
        objectVectorIndex--;
    }
    if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {
        objectVectorIndex++;
    }
}

void loadSceneFromJson(const string &filePath) {
    std::ifstream i(filePath);
    json j;
    i >> j;

    json objectsArray = j["objects"];
    std::cout << std::setw(4) << objectsArray << std::endl;
    for (auto &object: objectsArray) {
        auto objectPathString = object["objPath"].template get<std::string>();
        auto texPathString = object["texPath"].template get<std::string>();
        auto mtlPathString = object["mtlPath"].template get<std::string>();
        auto posVector = object["pos"].template get<std::vector<float>>();
        auto scaleVector = object["scale"].template get<std::vector<float>>();
        auto rotationDegrees = object["rotationDegrees"].template get<std::vector<float>>();
        auto ka = object["ka"].template get<float>();
        auto ks = object["ks"].template get<float>();
        auto kd = object["kd"].template get<float>();
        auto q = object["q"].template get<float>();

        Object obj{};
        obj.VAO = loadSimpleOBJ(objectPathString, obj.nVertices);
        int texWidth, texHeight;
        obj.texID = loadTexture(texPathString, texWidth, texHeight);
        obj.pos = glm::vec3(posVector[0], posVector[1], posVector[2]);
        obj.scale = glm::vec3(scaleVector[0], scaleVector[1], scaleVector[2]);
        obj.rotationDegrees = glm::vec3(rotationDegrees[0], rotationDegrees[1], rotationDegrees[2]);
        // Serão sobrescritos se houver um arquivo mtl com valores para esses parâmetros
        obj.ka = ka;
        obj.ks = ks;
        obj.kd = kd;
        obj.q = q;

        if (!mtlPathString.empty()) {
            loadSimpleMtl(mtlPathString, obj);
        }

        objectVector.push_back(obj);
    }

    if (!j["camera"].empty()) {
        auto cameraPosVector = j["camera"]["pos"].template get<std::vector<float>>();
        auto cameraFrontVector = j["camera"]["front"].template get<std::vector<float>>();
        auto cameraUpVector = j["camera"]["up"].template get<std::vector<float>>();

        cameraPos = glm::vec3(cameraPosVector[0], cameraPosVector[1], cameraPosVector[2]);
        cameraFront = glm::vec3(cameraFrontVector[0], cameraFrontVector[1], cameraFrontVector[2]);
        cameraUp = glm::vec3(cameraUpVector[0], cameraUpVector[1], cameraUpVector[2]);
    }

    if (!j["light"].empty()) {
        auto lightPosVector = j["light"]["pos"].template get<std::vector<float>>();
        auto lightColorVector = j["light"]["color"].template get<std::vector<float>>();

        lightPos = glm::vec3(lightPosVector[0], lightPosVector[1], lightPosVector[2]);
        lightColor = glm::vec3(lightColorVector[0], lightColorVector[1], lightColorVector[2]);
    }
}

void mouse_callback(GLFWwindow *window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = (float) xPos;
        lastY = (float) yPos;
        firstMouse = false;
    }

    auto xOffset = float(xPos - lastX);
    auto yOffset = float(lastY - yPos);
    lastX = (float) xPos;
    lastY = (float) yPos;

    float sensitivity = 0.5f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

int loadSimpleOBJ(string filePath, int &nVertices) {
    vector<glm::vec3> vertices;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normals;
    vector<GLfloat> vBuffer;

    glm::vec3 color = glm::vec3(1.0, 0.0, 0.0);

    ifstream arqEntrada;

    arqEntrada.open(filePath.c_str());
    if (arqEntrada.is_open()) {
        //Fazer o parsing
        string line;
        while (!arqEntrada.eof()) {
            getline(arqEntrada, line);
            istringstream ssline(line);
            string word;
            ssline >> word;
            if (word == "v") {
                glm::vec3 vertice;
                ssline >> vertice.x >> vertice.y >> vertice.z;
                //cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
                vertices.push_back(vertice);

            }
            if (word == "vt") {
                glm::vec2 vt;
                ssline >> vt.s >> vt.t;
                //cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
                texCoords.push_back(vt);

            }
            if (word == "vn") {
                glm::vec3 normal;
                ssline >> normal.x >> normal.y >> normal.z;
                //cout << vertice.x << " " << vertice.y << " " << vertice.z << endl;
                normals.push_back(normal);

            } else if (word == "f") {
                while (ssline >> word) {
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *) 0);
        glEnableVertexAttribArray(0);

        //Atributo cor (r, g, b)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        // texCoords (s, t)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *) (6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        // vetor normal (x, y, z)
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid *) (8 * sizeof(GLfloat)));
        glEnableVertexAttribArray(3);

        // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
        // atualmente vinculado - para que depois possamos desvincular com segurança
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
        glBindVertexArray(0);

        nVertices = vBuffer.size() / 11;
        return VAO;

    } else {
        cout << "Erro ao tentar ler o arquivo " << filePath << endl;
        return -1;
    }
}

void loadSimpleMtl(string filePath, Object &obj) {
    ifstream arqEntrada;

    arqEntrada.open(filePath.c_str());
    if (arqEntrada.is_open()) {
        string line;
        while (!arqEntrada.eof()) {
            getline(arqEntrada, line);
            istringstream ssline(line);
            string word;
            ssline >> word;
            if (word == "Kd") {
                glm::vec3 value;
                ssline >> value.x >> value.y >> value.z;
                obj.kd = (value.x + value.y + value.z) / 3.0f;
            } else if (word == "Ka") {
                glm::vec3 value;
                ssline >> value.x >> value.y >> value.z;
                obj.ka = (value.x + value.y + value.z) / 3.0f;
            } else if (word == "Ks") {
                glm::vec3 value;
                ssline >> value.x >> value.y >> value.z;
                obj.ks = (value.x + value.y + value.z) / 3.0f;
            }
        }

        std::cout << "Leitura de arquivo .mtl terminada!" << std::endl;
        std::cout << obj.kd << " " << obj.ka << " " << obj.ks << std::endl;
    }
}

GLuint loadTexture(string filePath, int &width, int &height) {
    GLuint texID; // id da textura a ser carregada

    // Gera o identificador da textura na memória
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Ajuste dos parâmetros de wrapping e filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carregamento da imagem usando a função stbi_load da biblioteca stb_image
    int nrChannels;

    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

//    std::cout << data << std::endl;
//    std::cout << *data << std::endl;

    if (data) {
        if (nrChannels == 3) // jpg, bmp
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else // assume que é 4 canais png
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << filePath << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void initializeCatmullRomMatrix(glm::mat4 &matrix) {
    matrix[0] = glm::vec4(-0.5f, 1.5f, -1.5f, 0.5f); // Primeira linha
    matrix[1] = glm::vec4(1.0f, -2.5f, 2.0f, -0.5f); // Segunda linha
    matrix[2] = glm::vec4(-0.5f, 0.0f, 0.5f, 0.0f);  // Terceira linha
    matrix[3] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);   // Quarta linha
}

void generateCatmullRomCurvePoints(Curve &curve, int numPoints) {
    curve.curvePoints.clear(); // Limpa quaisquer pontos antigos da curva

    initializeCatmullRomMatrix(curve.M);
    // Calcular os pontos ao longo da curva com base em Bernstein
    // Loop sobre os pontos de controle em grupos de 4

    float piece = 1.0 / (float) numPoints;
    float t;
    for (int i = 0; i < curve.controlPoints.size() - 3; i++) {

        // Gera pontos para o segmento atual
        for (int j = 0; j < numPoints; j++) {
            t = j * piece;

            // Vetor t para o polinômio de Bernstein
            glm::vec4 T(t * t * t, t * t, t, 1);

            glm::vec3 P0 = curve.controlPoints[i];
            glm::vec3 P1 = curve.controlPoints[i + 1];
            glm::vec3 P2 = curve.controlPoints[i + 2];
            glm::vec3 P3 = curve.controlPoints[i + 3];

            glm::mat4x3 G(P0, P1, P2, P3);

            // Calcula o ponto da curva multiplicando tVector, a matriz de Bernstein e os pontos de controle
            glm::vec3 point = G * curve.M * T;
            curve.curvePoints.push_back(point);
        }
    }
}

GLuint generateControlPointsBuffer(vector<glm::vec3> controlPoints) {
    GLuint VBO, VAO;

    // Geração do identificador do VBO
    glGenBuffers(1, &VBO);

    // Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat) * 3, controlPoints.data(), GL_STATIC_DRAW);

    // Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);

    // Atributo posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
    // atualmente vinculado - para que depois possamos desvincular com segurança
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

std::vector<glm::vec3> generateCurveControlPoints(int numPoints) {
    std::vector<glm::vec3> controlPoints;

    // Define o intervalo para t: de 0 a 2 * PI, dividido em numPoints
    float step = 2 * 3.14159 / (numPoints - 1);

    for (int i = 0; i < numPoints - 1; i++) {
        float t = i * step;

        // Calcula x(t) e y(t) usando as fórmulas paramétricas
        float x = 16 * pow(sin(t), 3);
        float y = 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);

        // Normaliza os pontos para mantê-los dentro de [-1, 1] no espaço 3D
        x /= 16.0f; // Dividir por 16 para normalizar x entre -1 e 1
        y /= 16.0f; // Dividir por 16 para normalizar y aproximadamente entre -1 e 1
        y += 0.15;
        // Adiciona o ponto ao vetor de pontos de controle
        controlPoints.push_back(glm::vec3(x, y, 0.0f));
    }
    controlPoints.push_back(controlPoints[0]);

    return controlPoints;
}