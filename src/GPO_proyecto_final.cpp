/************************  GPO_01 ************************************
ATG, 2019
******************************************************************************/

#include <GpO.h>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "Mallas en OpenGL(GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

const char* vertex_prog = GLSL(
	layout(location = 0) in vec3 pos;
	out vec3 color;

	uniform mat4 MVP=mat4(1.0f);
	float t;
	void main()
	{
		t = smoothstep(0,1.25,pos.z);
		color = vec3(pow(t,2), 4*t*(1-t), pow((1-t),2));
		gl_Position = MVP*vec4(pos,1); // Coord homog�neas y aplicacion matriz transformacion MVP
	}
);

const char* fragment_prog = GLSL(
	in vec3 color;
	void main() 
	{	
		gl_FragColor = vec4(color,1); 
	}
);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog;
objeto malla;
GLuint buffer;

#define N 71
#define M 71

vec3 vert[N][M];
GLuint indexes[N-1][2*M+1];

float coordenada_z(float x, float y) {
	//variables locales
	float r,res;
	//calculos
	r = 5 * M_PI * sqrt(pow(x,2) + pow(y,2));
	
	if (r > 0) {
		res = (sin(r) / r) + 0.25;
	} else {
		res = 1.25;
	}
	return res;
}

objeto crear_malla(void)
{
	objeto obj;
	GLuint VAO;
	GLuint i_buffer;

	unsigned int i, j;

	// RELLENAR POSICIONES (x,y,z) de los v�rtices de la malla
	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			vert[i][j].x = 2 * (float)j / (M-1) - 1.0;
			vert[i][j].y = 2 * (float)i / (N-1) - 1.0;
			vert[i][j].z = coordenada_z(vert[i][j].x, vert[i][j].y);
		}
	}
	
	// Rellenar matriz de �ndices. 22
	//primera tira
	for (i = 0; i < 2*M; i++) {
		if ((i % 2) == 0) { //i es par
			indexes[0][i] = (i>>1);			
		} else {
			indexes[0][i] = (i>>1) + M;
		}
	}
	indexes[0][2*M] = 0xffffffff;
	//resto de tiras
	for (i = 1; i < N-1; i++) {
		for (j = 0; j < 2*M; j++) {
			indexes[i][j] = indexes[i-1][j] + M;
		}
		indexes[i][2*M] = 0xffffffff;
	}
	//imprime
	/*for (i = 0; i < N-1; i++) {
		for (j = 0; j < 2*M; j++) {
			printf("%d  ",indexes[i][j]);
		}
		printf("\n");
	}*/
	
	
	// Transferencia de posiciones e indices a la GPU
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	printf("Transferidos vertices\n");

	// Especifico como encontrar 1er argumento (atributo 0) del vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER


	glGenBuffers(1, &i_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
	printf("Transferidos indices\n");

	glBindVertexArray(0);  //Cerramos Vertex Array con todo lidto para ser pintado

	obj.VAO = VAO;
	obj.tipo_indice = GL_UNSIGNED_INT;
	obj.Nv = N*M; 
	obj.Ni = (N - 1)*(2 * M + 1);  // (N-1) tiras con 2*M vertices por tira + restart

	return obj;
}


void dibujar_strip(objeto obj)
{
  glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
  glDrawElements(GL_TRIANGLE_STRIP,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
  glBindVertexArray(0);
}


void init_scene()  
{
  prog = Compile_Link_Shaders(vertex_prog, fragment_prog);
  glUseProgram(prog);    // Indicamos que programa vamos a usar 

  malla = crear_malla();  // Datos del objeto, mandar a GPU

  glEnable(GL_CULL_FACE); glFrontFace(GL_CW);
  glEnable(GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex(0xffffffff);
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

//variables para controles
float tmp = 0.0f, tmp2 = 0.0f;
int cont = 0; //si esta a 0, no rota

float d=8.0f, az = 0, el = 0.75;
float t1 = 0.0f, t2 = 0.0f, t3 = 0.5f;


vec3 pos_obs=vec3(3.0f,3.0f,2.0f);
vec3 target = vec3(t1,t2,t3);
vec3 up= vec3(0,0,1);

mat4 VV, PP, Model, Matriz, mvp; 

// Dibujar objetos 
// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Especifica color (RGB+alfa)	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	///////// Aqui vendr�a nuestr c�digo para actualizar escena  /////////

	pos_obs = d * vec3(sin(az) * cos(el),  cos(az) * cos(el),  sin(el));

	if (cont == 1) {
		tmp = tmp2 * tt;
	}

	target = vec3(t1,t2,t3);
		
	Model = mat4(1.0);  // Matriz identidad (no movemos malla)
	Matriz = rotate(glm::radians(tmp), vec3(0.0f, 0.0f, 1.0f));
	VV = lookAt(pos_obs, target, up);
	PP = perspective(glm::radians(28.0f), 4.0f / 3.0f, 1.0f, 20.0f);

	mvp = PP*VV*Model*Matriz;

	transfer_mat4("MVP", mvp);
	
    //dibujar_strip(malla);
	glBindVertexArray(malla.VAO);              // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLE_STRIP, malla.Ni, malla.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);

	////////////////////////////////////////////////////////

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
		render_scene();  glfwSwapBuffers(window); glfwPollEvents();
		show_info();
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}


//////////  FUNCION PARA MOSTRAR INFO OPCIONAL EN EL TITULO DE VENTANA  //////////
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

int flag_malla = 0;
static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{
	//fprintf(stdout, "Key %d Code %d Act %d Mode %d\n", key, code, action, mode);
	float var = M_PI/2;
	switch (key)
	{
	case GLFW_KEY_ESCAPE:	glfwSetWindowShouldClose(window, true); break;	
	case GLFW_KEY_SPACE: 
		if (action) {
			if (cont == 0) {
				tmp2 = 10.0f; 
				cont = 1;
			} else {
				tmp2 = 0.0f;
				cont = 0;
			}
		} break;
	
	case GLFW_KEY_UP:
	 	if (action) {
			if (el < var) {
				el += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_DOWN:
	 	if (action) {
			if (-var <= el) {
				el -= 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_LEFT:
	 	if (action) {
			if (az < var) {
				az += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_RIGHT:
	 	if (action) {
			if (-var <= az) {
				az -= 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_TAB:
	 	if (action) {
			if (flag_malla == 0) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				flag_malla = 1;
			} else {
				glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
				flag_malla = 0;
			}
		}break;
	 case GLFW_KEY_D:
	 	if (action) {
			if (t1 < 1.5f) {
				t1 += 0.1f;
			}
		} break;
	 case GLFW_KEY_A:
	 	if (action) {
			if (-1.5f <= t1) {
				t1 -= 0.1f;
			}
		} break;
	 case GLFW_KEY_W:
	 	if (action) {
			if (-2.5f <= t3) {
				t3 -= 0.1f;
			}
		} break;
	 case GLFW_KEY_S:
	 	if (action) {
			if (t3 < 2.5f) {
				t3 += 0.1f;
			}
		} break;
	}

}

// Callback de cambio tama�o de ventana
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
    ALTO = height;
    ANCHO = width;
}


void ScrollCallback(GLFWwindow* window, double dx, double dy) {
	if (dy < 0) {
        if (d < 18) {
            d += 0.25;
        } else printf("No se puede mover mas, ya esta muy lejos.\n");
    } else if (dy > 0) {
        if (d > 4) {
            d -= 0.25;
        } else printf("No se puede alejar mas, ya esta muy cerca.\n");
    }
}

void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, ScrollCallback);
}


