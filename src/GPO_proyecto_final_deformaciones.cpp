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

const char* vertex_prog_terreno = GLSL(
	layout(location = 0) in vec3 pos;
	out float h;
	uniform mat4 MVP=mat4(1.0f);
	float t;
	void main()
	{
		h = pos.z;
		gl_Position = MVP*vec4(pos,1); // Coord homog�neas y aplicacion matriz transformacion MVP
	}
);

const char* fragment_prog_terreno = GLSL(
	in float h;
	void main() 
	{	
		vec3 color;
		if (h < 0) {
			color = vec3(0.52,0.44,0.36);
		} else if (h >= 0 && h < 0.1) {
			color = vec3(0.52,0.44,0.36);
		} else if (h >= 0.1 && h < 0.2) {
			color = vec3(0.76,0.69,0.49);
		} else if (h >= 0.2 && h < 0.34) {
			color = vec3(0.86,0.76,0.46);
		} else if (h >= 0.34 && h < 0.4) {
			color = vec3(0.74,0.78,0.29);
		} else if (h >= 0.4 && h < 0.5) {
			color = vec3(0.49,0.70,0.18);
		} else if (h >= 0.5 && h < 0.7) {
			color = vec3(0.36,0.46,0.22);
		} else if (h >= 0.7 && h < 0.8) {
			color = vec3(0.41,0.45,0.34);
		} else {
			color = vec3(0.75,0.76,0.72);
		}
		gl_FragColor = vec4(color,1); 
	}
);

// -------------------------SHADER AGUA-------------------------------

const char* vertex_prog_agua = GLSL(
	layout(location = 0) in vec3 pos;

	uniform mat4 MVP;
	uniform float time;

	void main()
	{
		// movemos los fragmentos para que parezca que hay marea
		vec3 newPosition = pos + vec3(0.0, 0.0, sin(time * 0.5 + pos.x * 1.0) * cos(time * 1.0 + pos.y * 1.0)*0.035);
		gl_Position = MVP*vec4(newPosition,1); // Coord homog�neas y aplicacion matriz transformacion MVP
	}
);

const char* fragment_prog_agua = GLSL(

	void main() 
	{	
		vec3 color = vec3(0,0.9,0.8);
		gl_FragColor = vec4(color,0.6); 
	}
);

//---------------- SHADER DEL CURSOR ---------------------//
const char* vertex_prog_cursor = GLSL(
	layout(location = 0) in vec3 pos;
	layout(location = 1) in vec2 uv;
	uniform mat4 MVP;
	void main(){
		gl_Position = MVP*vec4(pos, 1);
	}
);

const char* fragment_prog_cursor = GLSL(

	out vec3 col;
	void main()
	{
		vec3 color = vec3(1,0.0,0.0);
		gl_FragColor = vec4(color,0.6); 
	}
);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog_terreno;
GLuint prog_agua;
GLuint prog_cursor;
objeto malla_terreno;
objeto malla_agua;
objeto cursor;
GLuint buffer;

#define N 513
#define M 513

vec3 vert[N][M];
vec3 vert_norm[N][M];
GLuint indexes[N-1][2*M+1];
float alturas[N][M];
bool alturas_inicializadas = false;

float coordenada_z(int x, int y) {
	return alturas[x][y];
}

objeto crear_malla(int tipoMalla)
{
	objeto obj;
	GLuint VAO;
	GLuint i_buffer;

	unsigned int i, j;


	FILE* fid;
	fopen_s(&fid, "./data/perlin.dat", "rb");
	if(fid != NULL) {
		if (!alturas_inicializadas){
			unsigned int count = fread((void *) alturas, sizeof(float), N*M, fid);
			alturas_inicializadas = true;
			fclose(fid);
		}
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
			if (tipoMalla == MALLA_AGUA)
				vert[i][j].z = 0.3;
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
  prog_terreno = Compile_Link_Shaders(vertex_prog_terreno, fragment_prog_terreno);
  prog_agua = Compile_Link_Shaders(vertex_prog_agua, fragment_prog_agua);
  prog_cursor = Compile_Link_Shaders(vertex_prog_cursor, fragment_prog_cursor);
  glUseProgram(prog_terreno);    // Indicamos que programa vamos a usar 

  malla_terreno = crear_malla(MALLA_TERRENO);  // Datos del objeto, mandar a GPU
  malla_agua = crear_malla(MALLA_AGUA);
  cursor = cargar_modelo("./data/esfera_256.bix");

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

// Almacena la posicion del puntero
vec3 pointerPosition = vec3(0.0f,0.0f,0.5f);

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
	Model = R*T*S;

	VV = lookAt(pos_obs, target, up);
	PP = perspective(glm::radians(28.0f), 4.0f / 3.0f, znear, zfar);
	glUseProgram(prog_terreno);    // Indicamos que programa vamos a usar 
	transfer_mat4("MVP", PP*VV*Model);
	glBindVertexArray(malla_terreno.VAO);  // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLE_STRIP, malla_terreno.Ni, malla_terreno.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);

	glUseProgram(prog_agua);    // Cambiamos programa del shader
	transfer_float("time", tt);
	transfer_mat4("MVP", PP*VV*Model);

	glBindVertexArray(malla_agua.VAO); // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLE_STRIP, malla_agua.Ni, malla_agua.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);


	// Pintamos el cursor
	glUseProgram(prog_cursor);
	T = glm::translate(pointerPosition);
	S = scale(vec3(0.1,0.1,0.1));
	Model = T * S;
	transfer_mat4("MVP",PP*VV*Model);
	glBindVertexArray(cursor.VAO);
	glDrawElements(GL_TRIANGLES, cursor.Ni, cursor.tipo_indice, (void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);  //Desactivamos VAO activo.

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

bool calcular_posiciones_mapa_altura(float x, float y, int* result){
	printf("Posiciones a comprobar: %f, %f\n",x,y);
	float best = 1.0f;
	for(int i = 0; i < M; i++){
		for (int j = 0; j < N; j++){
			vec3 item = vert[i][j];
			float distance = sqrt(pow(item.x - x,2) + pow(item.y - y,2));
			if (distance < best){
				result[0] = i;
				result[1] = j;
				best = distance;
			}
			/*if (distance < 0.01){
				printf("Posiciones de resultado: %f, %f\n",item.x,item.y);
				result[0] = i;
				result[1] = j;
				return true;
			}*/
		}
	}
	vec3 item = vert[result[0]][result[1]];
	printf("Posiciones obtenidas: %f, %f\n",item.x, item.y);
	return best < 0.1f;


}

const float increment = 0.1f;
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
	case GLFW_KEY_SPACE: 
		if (action != GLFW_PRESS){
			return;
		}
		pointerPosition[2] += increment;
		if (pointerPosition[2] > 10.0f){
			pointerPosition[2] = 10.0f;
		}
		break;
	case GLFW_KEY_LEFT_CONTROL:
		if (action != GLFW_PRESS){
			return;
		}
		pointerPosition[2] -= increment;
		if (pointerPosition[2] < 0.0f){
			pointerPosition[2] = 0.0f;
		}
		break;
	
	case GLFW_KEY_UP:
	 	if (action == GLFW_PRESS) {
			if (el < var) {
				el += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_DOWN:
	 	if (action == GLFW_PRESS) {
			if (-var <= el) {
				el -= 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_LEFT:
	 	if (action == GLFW_PRESS) {
			if (az < var) {
				az += 0.02;
			}
		}
	 	break;
	 case GLFW_KEY_RIGHT:
	 	if (action == GLFW_PRESS) {
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
	 case GLFW_KEY_D:
	 	if (action == GLFW_PRESS) {
			pointerPosition[1] += increment;
			if (pointerPosition[1] > 10.0f){
				pointerPosition[1] = 10.0f;
			}
		} break;
	 case GLFW_KEY_A:
	 	if (action == GLFW_PRESS) {
			pointerPosition[1] -= increment;
			if (pointerPosition[1] < -10.0f){
				pointerPosition[1] = -10.0f;
			}
		} break;
	 case GLFW_KEY_W:
	 	if (action == GLFW_PRESS) {
			pointerPosition[0] += increment;
			if (pointerPosition[0] > 10.0f){
				pointerPosition[0] = 10.0f;
			}
		} break;
	 case GLFW_KEY_S:
	 	if (action == GLFW_PRESS) {
			pointerPosition[0] -= increment;
			if (pointerPosition[0] < -10.0f){
				pointerPosition[0] = -10.0f;
			}
		} break;

	 case GLFW_KEY_U:
	 	if (action == GLFW_PRESS) {
			if (-2.5f <= t3) {
				t3 -= 0.1f;
			}
		} break;
	 case GLFW_KEY_H:
	 	if (action == GLFW_PRESS) {
			if (-1.5f <= t1) {
				t1 -= 0.1f;
			}
		} break;
	 case GLFW_KEY_J:
	 	if (action == GLFW_PRESS) {
			if (t3 < 2.5f) {
				t3 += 0.1f;
			}
		} break;
	 case GLFW_KEY_K:
	 	if (action == GLFW_PRESS) {
			if (t1 < 1.5f) {
				t1 += 0.1f;
			}
		} break;

	 case GLFW_KEY_T:
	 	if (action == GLFW_PRESS) {
			int posiciones[2];
			float x = pointerPosition[0];
			float y = pointerPosition[1];
			if (calcular_posiciones_mapa_altura(x,y, posiciones)){
				for (int i = -20; i <= 20; i++){
					int xAdapted = posiciones[0] + i;
					if (xAdapted < 0 || xAdapted >= M){
						continue;
					}
					for (int j = -20; j <= 20; j++){
						int yAdapted = posiciones[1] + j;
						if (yAdapted < 0 || yAdapted >= N){
							continue;
						}
						float adjustFactor = (abs(j) + abs(i))/40.0f + 1.0f;
						alturas[xAdapted][yAdapted] += 0.1f/adjustFactor;
					}
				}
				//alturas[posiciones[0]][posiciones[1]] += 1.0f;
				printf("Valor de altura: %f\n", alturas[posiciones[0]][posiciones[1]]);
				malla_terreno = crear_malla(MALLA_TERRENO);  // Datos del objeto, mandar a GPU
			}
		} break;
	  case GLFW_KEY_V:
	  	if (action == GLFW_PRESS) {
			int posiciones[2];
			float x = pointerPosition[0];
			float y = pointerPosition[1];
			if (calcular_posiciones_mapa_altura(x,y, posiciones)){
				for (int i = -20; i <= 20; i++){
					int xAdapted = posiciones[0] + i;
					if (xAdapted < 0 || xAdapted >= M){
						continue;
					}
					for (int j = -20; j <= 20; j++){
						int yAdapted = posiciones[1] + j;
						if (yAdapted < 0 || yAdapted >= N){
							continue;
						}
						float adjustFactor = (abs(j) + abs(i))/40.0f + 1.0f;
						alturas[xAdapted][yAdapted] -= 0.1f/adjustFactor;
					}
				}
				//alturas[posiciones[0]][posiciones[1]] += 1.0f;
				printf("Valor de altura: %f\n", alturas[posiciones[0]][posiciones[1]]);
				malla_terreno = crear_malla(MALLA_TERRENO);  // Datos del objeto, mandar a GPU
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


