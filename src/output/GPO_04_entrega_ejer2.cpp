/************************  GPO Iluminacion  **********************************
ATG, 2020
******************************************************************************/

#include <GpO.h>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL(GpO) Iluminacion";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

//  PROGRAMA 1 (Implementacion por vertices de iluminacion difusa con luz lejana)
const char* vertex_prog1 = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out float ilu;

	uniform mat4 M;
	uniform mat4 PV;
	uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
    uniform vec3 campos;

	void main() {
		gl_Position = PV*M*vec4(pos, 1);

		mat3 M_adj = mat3(transpose(inverse(M)));
		vec3 n = M_adj * normal;
        vec3 nn = normalize(n);
        vec3 v = normalize(campos - vec3(M * vec4(pos,1)));
        vec3 r = reflect(-luz,nn);
		
		float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
        float especular = dot(r,v); if (especular < 0) especular = 0; especular = pow(especular,16);
		ilu = (0.1 + 0.6*difusa + 0.3 * especular);  //10% Ambiente + 60% difusa + 30% especular
	}
);

const char* fragment_prog1 = GLSL(
	in float ilu;     // Entrada = iluminaci�n de vertices (interpolados en fragmentos)
	out vec3 col;  // Color fragmento
	void main()
	{
		col = vec3(1, 1, 0.9);
		col = col*ilu;
	}
);

//////////////////////////////////////////////////////////////////////////////////


//  PROGRAMA 2 (aqui implementaremos version en fragmentos)
const char* vertex_prog2 = GLSL(       
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out vec3 n;
    out vec3 v;

	uniform mat4 M;
	uniform mat4 PV;
    uniform vec3 campos;
	
	void main() {
		gl_Position = PV * M * vec4(pos, 1);

		mat3 M_adj = mat3(transpose(inverse(M)));
		n = M_adj * normal;
        v = normalize(campos - vec3(M * vec4(pos,1)));
	}
);

const char* fragment_prog2 = GLSL(
	in vec3 n;     // Entrada = iluminaci�n de vertices (interpolados en fragmentos)
    in vec3 v;
	out vec3 col;  // Color fragmento
	uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);

	void main()
	{
		vec3 nn = normalize(n);
        vec3 r = reflect(-luz,nn);

		float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
        float especular = dot(r,v); if (especular < 0) especular = 0; especular = pow(especular,16);
		float ilu = (0.1 + 0.6*difusa + 0.3 * especular);  //10% Ambiente + 60% difusa + 30% especular
	

		col = vec3(1, 1, 1);
		col = col*ilu;
	}
);

////////////////////////////////  FIN PROGRAMAS GPU (SHADERS) //////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[2];
objeto modelo;


// Dibuja objeto indexado
void dibujar_indexado(objeto obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
  glBindVertexArray(0);
}


// Variables globales
mat4 Proy,View,M;
vec3 campos=vec3(0.0f,1.0f,4.0f);											
vec3 target=vec3(0.0f,0.9f,0.0f);
vec3 up = vec3(0, 1, 0);


// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	prog[0] = Compile_Link_Shaders(vertex_prog1, fragment_prog1); // Compile shaders prog1
	prog[1] = Compile_Link_Shaders(vertex_prog2, fragment_prog2); // Compile shaders prog2
	
	glUseProgram(prog[0]);

	modelo=cargar_modelo("./data/esfera_106_n.bix");

	Proy = glm::perspective(glm::radians(55.0f), 4.0f / 3.0f, 0.1f, 100.0f); 
	View = glm::lookAt(campos,target,up);

	glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST);
}

//variables globales
float az = 0, el = 0.75;
// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 
	
	vec3 luz = glm::vec3(cos(el) * cos(az), sin(el), cos(el) * sin(az));

	vec3 xy = vec3(cos(tt), 1.0f, sin(tt));
	M = translate(xy)*rotate(glm::radians(50*tt), vec3(0.0f, 1.0f, 0.0f));   // Mov modelo 

    transfer_vec3("campos",campos);
	
	transfer_mat4("PV",Proy*View); transfer_mat4("M", M); transfer_vec3("luz",luz);
	dibujar_indexado(modelo);
	
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
//////////////////////  INTERACCION  TECLADO RATON
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Callback de cambio tama�o
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	ALTO = height;
	ANCHO = width;
}

int actual = 0;
static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{	
	float tmp = M_PI / 2;
	switch (key)
	{
	 case GLFW_KEY_ESCAPE:	glfwSetWindowShouldClose(window, true); break;	
	 case GLFW_KEY_TAB:
	 	if (action == GLFW_PRESS) {
			if (actual) actual = 0; else actual = 1;
			glUseProgram(prog[actual]);
		}
		break;
	 case GLFW_KEY_UP:
	 	if (action) {
			if (el < tmp) {
				el += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_DOWN:
	 	if (action) {
			if (-tmp < el) {
				el -= 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_LEFT:
	 	if (action) {
			if (az < tmp) {
				az += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_RIGHT:
	 	if (action) {
			if (-tmp < az) {
				az -= 0.02;
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



 
