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
/*const char* vertex_prog1 = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out float ilu;

	uniform mat4 M;
	uniform mat4 PV;
	uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);

	void main() {
		gl_Position = PV*M*vec4(pos, 1);

		mat3 M_adj = mat3(transpose(inverse(M)));
		vec3 n = M_adj * normal;

		vec3 nn = normalize(n);
		float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
		ilu = (0.15 + 0.85*difusa);  //15% Ambiente + 85% difusa
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
);*/

//////////////////////////////////////////////////////////////////////////////////


//  PROGRAMA 2 (aqui implementaremos version en fragmentos)
const char* vertex_prog2 = GLSL(       
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec2 uv;
	out vec3 n;
	out vec2 UV;

	uniform mat4 M;
	uniform mat4 PV;
	
	void main() {
		gl_Position = PV * M * vec4(pos, 1);

		vec3 normal = normalize(pos);

		mat3 M_adj = mat3(transpose(inverse(M)));
		n = M_adj * normal;

		UV = uv;
	}
);

const char* fragment_prog2 = GLSL(
	in vec3 n;     // Entrada = iluminaci�n de vertices (interpolados en fragmentos)
	in vec2 UV;
	out vec3 col;  // Color fragmento
	uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
	uniform sampler2D unit;
	void main()
	{
		vec3 nn = normalize(n);
		float difusa = sqrt(dot(luz, nn)); if (difusa < 0) difusa = 0;
		float ilu = (0.05 + 0.95 * difusa);  //5% Ambiente + 95% difusa

		col = texture(unit, UV).rgb;
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
vec3 campos=vec3(2.5f,0.0f,0.0f);											
vec3 target=vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0, 0, 2);
float elev = 0.0f;


// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Opciones generales de render de OpenGL
void init_scene()
{
	GLuint tex0;
	// prog[0] = Compile_Link_Shaders(vertex_prog1, fragment_prog1); // Compile shaders prog1
	prog[0] = Compile_Link_Shaders(vertex_prog2, fragment_prog2); // Compile shaders prog2
	
	glUseProgram(prog[0]);

	modelo=cargar_modelo("./data/esfera_961.bix");
	tex0 = cargar_textura("./data/planisferio.jpg", GL_TEXTURE0);

	Proy = glm::perspective(glm::radians(55.0f), 4.0f / 3.0f, 0.1f, 100.0f); 
	View = glm::lookAt(campos,target,up);

	glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST);
}

const float rotation = (90 * M_PIf64)/180;
const float estacion = (23.5 * M_PIf64)/180;



//variables globales
float az = 0, el = 0.0;
// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 

	campos = 2.5f * vec3(cos(elev),0, sin(elev));
	View = glm::lookAt(campos,target,up);

	az = 0.05 * tt; // Azimuth avanza a razón de 0.05 rads/s
	
	vec3 luz = glm::vec3(cos(az), -sin(az), 0);

	vec3 xy = vec3(0.0f, 0.0f, 0.0f);
	
	M = translate(xy) * (rotate(rotation*tt, vec3(0,-sin(estacion), cos(estacion))) * rotate(estacion, vec3(1.0f,0.0f,0.0f)));   // Mov modelo 
	
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
	 	if (action) {
			if (actual) actual = 0; else actual = 1;
			glUseProgram(prog[actual]);
		}
		break;
	 case GLFW_KEY_UP:
	 	if (action) {
			if (el < tmp) {
				el += 0.02;
			}
			elev += 0.1f;
			if (elev>1.5f){
				elev = 1.5f;
			}
		}
	 	break;
	 case GLFW_KEY_DOWN:
	 	if (action) {
			if (-tmp < el) {
				el -= 0.02;
			}
			elev -= 0.1f;
			if (elev<-1.5f){
				elev = -1.5f;
			}
		}
	 	break;
	 case GLFW_KEY_RIGHT:
	 	if (action) {
			if (az < tmp) {
				az += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_LEFT:
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



 
