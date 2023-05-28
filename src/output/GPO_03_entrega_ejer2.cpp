/************************  GPO_03 ************************************
ATG, 2022
******************************************************************************/


#include <GpO.h>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL Texturas (GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

const char* vertex_prog = GLSL(
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
out vec2 UV;
uniform mat4 MVP;
void main(){
	gl_Position = MVP*vec4(pos, 1);
	UV = uv;
}
);

const char* fragment_prog = GLSL(
in vec2 UV;
out vec3 col;
uniform sampler2D unit;
void main()
{
	col= texture(unit, UV).rgb;
}
);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog;
objeto obj, obj2;

objeto crear_cuadrado(void)
{
	objeto obj;
	GLuint VAO;
	GLuint buffer, i_buffer;


	static const GLfloat vertex_data[] = {   //  POsicion XYZ      (u,v)
											-1.0f,  1.0f, 0.0f,   1,1,  // Vertice 0
											 1.0f,  1.0f, 0.0f,   1,0,  // Vertice 1
											 1.0f, -1.0f, 0.0f,   0,0,  // Vertice 2
											-1.0f, -1.0f, 0.0f,   0,1}; // Vertice 3
	

	GLbyte indices[] = { 0, 3, 2, 0, 2, 1 };
	obj.Ni = 2 * 3;
	obj.tipo_indice = GL_UNSIGNED_BYTE;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);


	// Defino 1er argumento (atributo 0) del vertex shader (pos)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	// Defino 2� argumento (atributo 1) del vertex shader  (uv)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	glGenBuffers(1, &i_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Cerramos Vertex Array con todo lidto para ser pintado. Dejamos enlazado
	// el buffer de indices (i_buffer) dentro del VAO para que cuando enlacemos 
	// el VAO para pintar este todo listo sin tener que enlazar indices
	glBindVertexArray(0);

	obj.VAO = VAO;

	return obj;

}


void dibujar_indexado(objeto obj)
{
	// Activamos VAO asociado a obj y lo dibujamos con glDrawElements 
	glBindVertexArray(obj.VAO);
	glDrawElements(GL_TRIANGLES, obj.Ni, obj.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);  //Desactivamos VAO activo.
}


vec3 pos_obs = vec3(3.0f, 3.0f, 2.0f);
vec3 target = vec3(0.0f, 0.0f, 1.0f);
vec3 up = vec3(0, 0, 1);

mat4 PP, VV; // matrices de proyeccion y perspectiva

// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Opciones generales de render de OpenGL
// Creaci�n de matrices de perspectiva y punto de vista.
void init_scene()
{
	GLuint tex0, tex1;

	prog = Compile_Link_Shaders(vertex_prog, fragment_prog); // Mandar programas a GPU, compilar y crear programa en GPU
	glUseProgram(prog);    // Indicamos que programa vamos a usar 

	//obj = crear_cuadrado();  // Datos del objeto, mandar a GPU
	obj = cargar_modelo("./data/spider.bix");
    obj2 = cargar_modelo("./data/halo.bix");

	PP = perspective(glm::radians(25.0f), 4.0f / 3.0f, 0.1f, 20.0f);  //40� Y-FOV,  4:3 ,  Znear=0.1, Zfar=20
	VV = lookAt(pos_obs, target, up);  // Pos camara, Lookat, head up

	// Carga imagen (en el directorio /data/ al mismo nivel que /Debug),
	// la guarda en el objeto tex0 y lo asocia al texture slot 0 (GL_TEXTURE0)
	tex0 = cargar_textura("./data/spider.jpg", GL_TEXTURE0);
    tex1 = cargar_textura("./data/halo.jpg", GL_TEXTURE1);

	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}


// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.6f,0.0f,1.0f); // Especifica color para el fondo (RGB+alfa)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Aplica color asignado borrando el buffer

	float t = (float)glfwGetTime();  // Contador de tiempo en segundos 

	///////// Aqui vendr�a nuestr c�digo para actualizar escena  /////////	
	mat4 M, M1, M2, T, R, S;
	M = rotate(glm::radians(5.0f*t), vec3(0.0f, 0.0f, 1.0f));
    M2 = rotate(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    M2 = M * M2;
    T = translate(glm::vec3(0.5,-0.5f,0.0f));

	transfer_mat4("MVP", T*PP*VV*M); 
	transfer_int("unit", 0);
	dibujar_indexado(obj);

    T = translate(glm::vec3(-0.5f,0.5f,0.0f));
    transfer_mat4("MVP", T*PP*VV*M2); 
	transfer_int("unit", 1);
    dibujar_indexado(obj2);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{		
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL	X.Y
	load_Opengl();         // Carga funciones de OpenGL, comprueba versi�n.
	init_scene();          // Prepara escena

	while (!glfwWindowShouldClose(window))
	{
		render_scene();
		glfwSwapBuffers(window); glfwPollEvents();
		show_info();		
	}

	glfwTerminate();

	exit(EXIT_SUCCESS);
}



/////////////////////  FUNCION PARA MOSTRAR INFO EN TITULO DE VENTANA  //////////////
void show_info()
{
	static int fps = 0;
	static double last_tt = 0;
	double elapsed, tt;
	char nombre_ventana[128];   // buffer para modificar titulo de la ventana

	fps++; tt = glfwGetTime();  // Contador de tiempo en segundos 

	elapsed = (tt - last_tt);
	if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
	{
		sprintf_s(nombre_ventana, 128, "%s: %4.0f FPS @ %d x %d", prac, fps / elapsed, ANCHO, ALTO);
		glfwSetWindowTitle(window, nombre_ventana);
		last_tt = tt; fps = 0;
	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  ASIGNACON FUNCIONES CALLBACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Callback de cambio tama�o
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	ALTO = height;
	ANCHO = width;
}

int vista = 0, textura = 0;
static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{
	printf("Key %d Code %d Act %d Mode %d\n", key, code, action, mode);
	switch (key)
	{
	  case GLFW_KEY_ESCAPE:	glfwSetWindowShouldClose(window, true); break;
      case GLFW_KEY_F1:
	  	if (action) {
            if (!vista) {
                glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);  // Rasteriza solo líneas
                vista = 1;
            } else {
                glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);  // Rasteriza triángulos
                vista = 0;
            }
        }
		break;
     case GLFW_KEY_F2:
        if (action) {
            if (textura) {
                transfer_int("unit", textura);
                textura = 0;
            } else {
                transfer_int("unit", textura);
                textura = 1;
            }
        }
        break;
	}

}

void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}
