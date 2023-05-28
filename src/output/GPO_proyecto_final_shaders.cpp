/************************  GPO_01 ************************************
ATG, 2019
******************************************************************************/

#include <GpO.h>

#define MALLA_AGUA 0
#define MALLA_TERRENO 1
#define WATER_HEIGHT 0.3

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "Mallas en OpenGL(GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////     CODIGO SHADERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GLSL(src) "#version 330 core\n" #src

// -------------------------SHADER TERRENO-------------------------------

const char* vertex_prog_terreno_ilu = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out vec3 n;
    out vec3 v;
	out float h;

	uniform mat4 M;
	uniform mat4 VP;
	uniform vec3 pos_obs;

	float t;
	void main()
	{
		gl_Position = VP*M*vec4(pos,1); // Coord homog�neas y aplicacion matriz transformacion MVP
		
		h = pos.z;
		mat3 M_adj = mat3(transpose(inverse(M)));
		n = M_adj * normal;
        v = normalize(pos_obs - vec3(M * vec4(pos,1)));
	}
);

const char* fragment_prog_terreno_ilu = GLSL(
	in float h;
	in vec3 n;
    in vec3 v;
	uniform vec3 luz;
	uniform vec4 components;


	void main() 
	{	
		vec3 color1;
		vec3 color2;
		vec3 result;
		float min_height;
		float max_height;

		// degradado de colores según la altura del fragmento
		if (h < 0 || (h >= 0 && h < 0.2)) {
			color1 = vec3(0.52,0.44,0.36);
			color2 = vec3(0.76,0.69,0.49);
			min_height = 0;
			max_height = 0.2;
		} else if (h >= 0.2 && h < 0.4) {
			color1 = vec3(0.86,0.76,0.46);
			color2 = vec3(0.74,0.78,0.29);
			min_height = 0.2;
			max_height = 0.4;
		} else if (h >= 0.4 && h < 0.6) {
			color1 = vec3(0.49,0.70,0.18);
			color2 = vec3(0.36,0.46,0.22);
			min_height = 0.4;
			max_height = 0.7;
		}  else if (h >= 0.6 && h < 0.8) {
			color1 = vec3(0.36,0.46,0.22);
			color2 = vec3(0.40,0.45,0.30);
			min_height = 0.6;
			max_height = 0.8;
		} else {
			color1 = vec3(0.40,0.45,0.30);
			color2 = vec3(0.75,0.76,0.72);
			min_height = 0.8;
			max_height = 1;
		}
		
		// hay que normalizar la altura segun el intervalo en el que se encuentre.
		// de este modo si la altura normalizada es 0, cogerá el color1
		// si es 1, cogerá el color 2
		// y si no, una interpolación entre ambos colores según su altura
		float norm_height = (h - min_height) / (max_height - min_height);
		result = mix(color1, color2, norm_height);

		// iluminación
		vec3 nn = normalize(n);
        vec3 r = reflect(-luz,nn);

		float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
        float especular = dot(r,v); if (especular < 0) especular = 0; especular = pow(especular,components[3]);
		float ilu = (components[0] + components[1]*difusa + components[2] * especular);  //10% Ambiente + 60% difusa + 30% especular
	
		gl_FragColor = vec4(result*ilu,1); 
	}
);

// -------------------------SHADER AGUA-------------------------------

const char* vertex_prog_agua = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec3 normal;
	out vec3 n;
    out vec3 v;

	uniform mat4 M;
	uniform mat4 VP;
	uniform float time;
	uniform vec3 pos_obs;

	void main()
	{
		// movemos los vertices para que parezca que hay marea
		vec3 newPosition = pos + vec3(0.0, 0.0, sin(time * 0.5 + pos.x * 1.0) * cos(time * 1.0 + pos.y * 1.0)*0.05);
		gl_Position = VP*M*vec4(newPosition,1); // Coord homog�neas y aplicacion matriz transformacion MVP
		mat3 M_adj = mat3(transpose(inverse(M)));
		n = M_adj * normal;
        v = normalize(pos_obs - vec3(M * vec4(pos,1)));
	}
);

const char* fragment_prog_agua = GLSL(
	in vec3 n;
    in vec3 v;
	uniform vec3 luz;
	uniform vec4 components;

	void main() 
	{	
		vec3 color = vec3(0,0.9,0.8);

		// iluminación
		vec3 nn = normalize(n);
        vec3 r = reflect(-luz,nn);

		float difusa = dot(luz,nn); if (difusa < 0) difusa = 0; 
        float especular = dot(r,v); if (especular < 0) especular = 0; especular = pow(especular,components[3]);
		float ilu = (components[0] + components[1]*difusa + components[2] * especular);  //10% Ambiente + 60% difusa + 30% especular
	
		gl_FragColor = vec4(color*ilu,0.6); 
	}
);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog_terreno;
GLuint prog_agua;
objeto malla_terreno;
objeto malla_agua;
GLuint buffer;

#define N 513
#define M 513

vec3 vert[N][M];
vec3 vert_norm[N][M];
GLuint indexes[N-1][2*M+1];
float alturas[N][M];

float coordenada_z(int x, int y) {
	return alturas[x][y];
}

objeto crear_malla(int tipoMalla)
{
	objeto obj;
	GLuint VAO;
	GLuint i_buffer;
	GLuint n_buffer;

	unsigned int i, j;


	FILE* fid;
	fopen_s(&fid, "./data/perlin.dat", "rb");
	if(fid != NULL) {
		unsigned int count = fread((void *) alturas, sizeof(float), N*M, fid);
		fclose(fid);
	} else {
		printf("No existe el fichero perlin.dat\n");
	}


	// RELLENAR POSICIONES (x,y,z) de los v�rtices de la malla
	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			vert[i][j].x = 2 * (float)j / (M-1) - 1.0;
			vert[i][j].y = 2 * (float)i / (N-1) - 1.0;
			if (tipoMalla == MALLA_TERRENO)
				vert[i][j].z = coordenada_z(i, j);
			if (tipoMalla == MALLA_AGUA){
				vert[i][j].z = 0.3;
				vert_norm[i][j] = vec3(0,0,1); // la normal del agua siempre es la misma
			}
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

	// calculamos normales del terreno
	for (i = 0; i < N-1 && tipoMalla == MALLA_TERRENO; i++) {
		for (j = 0; j < 2*M-1; j+=2) {
			/*
			printf("%d %d\n", indexes[i][j], indexes[i][j+1]);
			printf("	%d %d\n", indexes[i][j+2], indexes[i][j+3]);
			*/

			int vert1 = indexes[i][j];
			int vert2 = indexes[i][j+1];
			int vert3 = indexes[i][j+2];
			int vert4 = indexes[i][j+3];

			/*
			printf("	mod %d %d\n", i, indexes[i][j+1]%M);
			printf("	v1 %d\n", vert1);
			printf("	v2 %d\n", vert2);
			printf("	v3 %d\n", vert3);
			printf("	v4 %d\n", vert4);
			*/

			vec3 pos1 = vert[i][vert1%M];
			vec3 pos2 = vert[i+1][vert2%M];
			vec3 pos3 = vert[i][vert3%M];
			vec3 pos4 = vert[i+1][vert4%M];

			vec3 normal1 = pos3 - pos1;
			vec3 normal2 = pos2 - pos1;
			vec3 faceNormal = glm::cross(normal1, normal2);

			/*
			printf("	p1 %f %f %f\n", pos1.x, pos1.y, pos1.z);
			printf("	p2 %f %f %f\n", pos2.x, pos2.y, pos2.z);
			printf("	p3 %f %f %f\n", pos3.x, pos3.y, pos3.z);

			printf("	n1 %f %f %f\n", normal1.x, normal1.y, normal1.z);
			printf("	n2 %f %f %f\n", normal2.x, normal2.y, normal2.z);
			printf("	face %f %f %f\n", faceNormal.x, faceNormal.y, faceNormal.z);

			*/
			vert_norm[i][vert1%M] += faceNormal;
			vert_norm[i+1][vert2%M] += faceNormal;
			vert_norm[i][vert3%M] += faceNormal;
			vert_norm[i+1][vert4%M] += faceNormal;
			
			/*
			printf("	pos %f %f %f\n", pos1.x, pos1.y, pos1.z);
			printf("	norm %f %f %f\n", vert_norm[i][vert1%M].x, vert_norm[i][vert1%M].y, vert_norm[i][vert1%M].z);
			*/
		}
	}

	// normalizamos las normales
	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			vert_norm[i][j] = normalize(vert_norm[i][j]);
			//printf("	n1 %f %f %f\n", vert_norm[i][j].x, vert_norm[i][j].y, vert_norm[i][j].z);
		}
	}

	
	// Transferencia de posiciones, normales e indices a la GPU
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// TRANSFERENCIA DE VERTICES
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
	printf("Transferidos vertices\n");

	// Especifico como encontrar 1er argumento (atributo 0) del vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	/*
	*/
	// TRANSFERENCIA DE NORMALES
	glGenBuffers(1, &n_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, n_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert_norm), vert_norm, GL_STATIC_DRAW);
	printf("Transferidas las normales\n");
	// Especifico como encontrar 2o argumento (atributo 1) del vertex shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);  // Asignados atributos, podemos desconectar BUFFER

	// TRANSFERENCIA DE INDICES
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
  //prog_terreno = Compile_Link_Shaders(vertex_prog_terreno, fragment_prog_terreno);
  prog_terreno = Compile_Link_Shaders(vertex_prog_terreno_ilu, fragment_prog_terreno_ilu);
  prog_agua = Compile_Link_Shaders(vertex_prog_agua, fragment_prog_agua);
  glUseProgram(prog_terreno);    // Indicamos que programa vamos a usar 

  malla_terreno = crear_malla(MALLA_TERRENO);  // Datos del objeto, mandar a GPU
  malla_agua = crear_malla(MALLA_AGUA);

  glEnable(GL_CULL_FACE); glFrontFace(GL_CW);
  glEnable(GL_PRIMITIVE_RESTART);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPrimitiveRestartIndex(0xffffffff);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


//variables para controles
float tmp = 0.0f, tmp2 = 0.0f;
int cont = 0; //si esta a 0, no rota
float t = 0.0f; //tiempo que pasa

float d=8.0f, az = -0.58f, el = 0.5f;
float t1 = 0.0f, t2 = 0.0f, t3 = 0.5f;

float zfar = 20.0f;
float znear = 1.0f;

vec3 pos_obs;
vec3 target = vec3(t1,t2,t3);
vec3 up= vec3(0,0,1);

// PARÁMETROS DE ILUMINACIÓN
vec3 luz = vec3(1, 1.5, 0.5);
// Components (ambiental, difusa, especular, exponente)
vec4 components = vec4(0.4,0.4,0.2,5);//vec4(0.6,0.2,0.2,80);

mat4 VV, PP, Model, Matriz, mvp, T, R, S; 


// Dibujar objetos 
// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	float tt = (float)glfwGetTime();  // Contador de tiempo en segundos 

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Especifica color (RGB+alfa)	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// actualiza posicion observador
	pos_obs = d * vec3(sin(az) * cos(el),  cos(az) * cos(el),  sin(el));
	//cuanto ha girado
	if (cont == 1) {
		tmp = tmp2 * t;
		t += 0.01f;
	}
	target = vec3(t1,t2,t3);

	// Translacion - no
	T = mat4(1.0);
	// Rotacion
	R = rotate(glm::radians(tmp), vec3(0.0f, 0.0f, 1.0f));
	// Escalado
	S = scale(vec3(2,2.5,1));
	// Matriz de modelado
	Model = T*R*S;

	VV = lookAt(pos_obs, target, up);
	PP = perspective(glm::radians(28.0f), 4.0f / 3.0f, znear, zfar);

	// RENDER TERRENO
	glUseProgram(prog_terreno);    // Indicamos que programa vamos a usar 
	transfer_mat4("VP", PP*VV);
	transfer_mat4("M", Model);
	transfer_vec3("pos_obs",pos_obs);
	transfer_vec3("luz", luz);
	transfer_vec4("components", components);
	glBindVertexArray(malla_terreno.VAO);  // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLE_STRIP, malla_terreno.Ni, malla_terreno.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);

	// RENDER AGUA
	glUseProgram(prog_agua);    // Cambiamos programa del shader
	transfer_float("time", tt);
	transfer_mat4("VP", PP*VV);
	transfer_mat4("M", Model);
	transfer_vec3("pos_obs",pos_obs);
	transfer_vec3("luz", luz);
	transfer_vec4("components", components);
	glBindVertexArray(malla_agua.VAO); // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLE_STRIP, malla_agua.Ni, malla_agua.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);

	glUseProgram(0);

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
	case GLFW_KEY_ENTER: 
		if (action == GLFW_PRESS) {
			if (cont == 0) {
				tmp2 = 20.0f; 
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
	 	if (action == GLFW_PRESS) {
			if (flag_malla == 0) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				flag_malla = 1;
			} else {
				glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
				flag_malla = 0;
			}
		}break;
	 case GLFW_KEY_K:
	 	if (action == GLFW_PRESS) {
			if (t1 < 1.5f) {
				t1 += 0.1f;
			}
		} break;
	 case GLFW_KEY_H:
	 	if (action == GLFW_PRESS) {
			if (-1.5f <= t1) {
				t1 -= 0.1f;
			}
		} break;
	 case GLFW_KEY_U:
	 	if (action == GLFW_PRESS) {
			if (-2.5f <= t3) {
				t3 -= 0.1f;
			}
		} break;
	 case GLFW_KEY_J:
	 	if (action == GLFW_PRESS) {
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

void MouseCallback(GLFWwindow* window, int Button, int Action, int Mode) {

}

void ScrollCallback(GLFWwindow* window, double dx, double dy) {
	if (dy < 0) {
        if (d < 18) {
            d += 0.25;
        } else printf("No se puede mover mas, ya esta muy lejos.\n");
    } else if (dy > 0) {
        if (d > 2) {
            d -= 0.25;
        } else printf("No se puede alejar mas, ya esta muy cerca.\n");
    }
}

void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetMouseButtonCallback(window, MouseCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, ScrollCallback);
}


