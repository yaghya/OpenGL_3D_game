#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
struct VAO* Basket1;
struct VAO* CannonBase;
struct VAO* Basket2;
struct VAO* CannonFace;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float temp=0;
int s1=0,s2=0,s3=0,s4=0,s5=0,s6=0,f1=0,f2=0,f3=0,f4=0,f21=0,f22=0,f23=0,f24=0,neg=0;
long long int FLAG_GOLI[1000],flagblack[1000],FLAG_GRN[1000],flagred[1000],FLAG_GOLIMIRR1[1000],FLAG_GOLIMIRR2[1000],FLAG_GOLIMIRR3[1000],FLAG_GOLIMIRR4[1000];
int score=0;
float change=0,current_time,REDCHANGE[1000]={4.5},GREENCHANGE[1000]={4.5},BLACKCHANGE[1000]={4.5},YBULLET[1000],CANNONY[1000],ROTATEBULLET[1000];
void initialise()
{
        int i;
        srand(time(NULL));
        for(i=0;i<1000;i++)
        {
                REDCHANGE[i]=4.5;
               // xred[i]=(float)(((rand())%680-280)/100.0);
               // xgreen[i]=(float)(((rand())%680-280)/100.0);
               // xblack[i]=(float)(((rand())%680-280)/100.0);
                BLACKCHANGE[i]=4.5;
                GREENCHANGE[i]=4.5;
               // bulletx[i]=0;
                YBULLET[i]=0;
                CANNONY[i]=0;
                ROTATEBULLET[i]=0;
                FLAG_GOLI[i]=0;
                flagblack[i]=0;
                FLAG_GRN[i]=0;
                flagred[i]=0;
                FLAG_GOLIMIRR1[i]=0;
                FLAG_GOLIMIRR2[i]=0;
                FLAG_GOLIMIRR3[i]=0;
                FLAG_GOLIMIRR4[i]=0;

        }

}
/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}

float movcannon=0;
float rotatecannon=0;
float speed=1;
float dist=0.1;
long long count=0;int Ctrl=0,Alt=0;
long long int cur;
	long long int l=0;
/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
	case 'n':
		speed++;
		break;
	case 'm':
		speed--;
		if(speed==0)
			speed==1;
		break;
	case 'a':
		rotatecannon+=10;
		if(rotatecannon>=90)
			rotatecannon=90;
//cout << glutGet(GLUT_ELAPSED_TIME);
		break;
	case 'd':

		rotatecannon-=10;
		if(rotatecannon<=-90)
			rotatecannon=-90;
		break;
	case 's':
		movcannon+=0.2;
		break;
	case 'f':
		movcannon-=0.2;
		break;
        
        case 'x':
		count++;
            // do something
            break;
	case 32:
	
	cur = glutGet(GLUT_ELAPSED_TIME); 
        if ((cur - l) >= 1000)
	{
                        Ctrl=0;
                        Alt=0;
                       // count++;
                        CANNONY[count]=movcannon;
                        ROTATEBULLET[count]=rotatecannon;
			count++;
			l=cur;
   	}
                     break;

        default:
            break;
    }
}

//Ctrl=0;
//Alt=0;
float mov1=0;
float mov2=0,xchang=0,ychang=0,zoom=1;
/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
	switch (key) {
              

	 case 101: 
                zoom+=0.2;
                break;
        case 103: 
                if(zoom-0.2>=0.2)
                zoom-=0.2;
                break;

		case 116:
			Ctrl=0;
			Alt=1;
	//	cout << "116" << Ctrl << " "<< Alt;
			break;
        case 115: 
                Ctrl=1;
                Alt=0;
	//	cout << "115" << Ctrl << " "<< Alt;
                break;
       case 117: 
                Alt=1;
                Ctrl=0;
	//	cout << "117" << Ctrl << " "<< Alt;
                break;

		case 100:
			if(Ctrl==1)
			{
			if(mov1>-0.5)	
			mov1-=0.1;
			Ctrl=0;
			}
			else if(Alt==1)
			{
				if(mov2>-4.5)
				mov2-=0.1;
			
				Alt=0;		
			}
			else 
				xchang+=0.2;
			//Alt=0;
			//Ctrl=0;
			break;		
		case 114:
			Ctrl=1;
			Alt=0;
			break;	
		case 102:
			if(Ctrl==1)
			{
				if(mov1<4.5)
				mov1+=0.1;
				Ctrl=0;
			}
			else if(Alt==1)
			{
				if(mov2<0.5)
				mov2+=0.1;
				Alt=0;
			}
			else 
				xchang-=0.2;
			//Alt=0;
			//Ctrl=0;
			break;	
		default:
			Ctrl=0;
			Alt=0;
			break;	
	}
}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
int mouse_clicked=0;
void mouseClick (int button, int state, int x, int y)
{
    switch (button) {
                case GLUT_LEFT_BUTTON:

                        break;

        case GLUT_RIGHT_BUTTON:
                        if (state == GLUT_DOWN)
                        {cur = glutGet(GLUT_ELAPSED_TIME); 
        if ((cur - l) >= 1000)
	{
                        Ctrl=0;
                        Alt=0;
                       // count++;
                        CANNONY[count]=movcannon;
                        ROTATEBULLET[count]=rotatecannon;
			count++;
			l=cur;
   	}
		mouse_clicked=1;
			
                        }
			else if(state == GLUT_UP)
				mouse_clicked=0;
            break;
        default:
            break;
    }
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{
	//cout  << "chal raha \n";
	int x1,y1;
	x1=x-400;y1=400-y;
	int redx1,redx2,greenx1,greenx2;
	redx1=(-0.5+mov1 -2)*100;
	redx2=(-0.5+mov1)*100;
	greenx1=(0.5+mov2)*100;
	greenx2=(0.5+mov2 + 2 )*100;
	if(y>=600)
	{
		if((redx1 <= x1) && (redx2 >= x1))
		{
			if(x<=750 && x>=250)
			{
				mov1=(x1/100.0f) + 1;
			}
		} 
		else if((greenx1 <= x1) && (greenx2 >= x1))
		{
			if(x<=650 && x>=250)
			{
				mov2=(x1/100.0f) - 2;
			}
		} 
	} 
	if(x<=50)
	{
		int z1,z2;
			z1=(movcannon+0.5)*100;
			z2=(movcannon-0.5)*100;
		if(z1>=y1 && z2 <= y1 && y>=100 && y<=700)
			movcannon=y1/100.0f;
		
	}
	
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -2,-1,0, // vertex 1
    2,-1,0, // vertex 2
    1, 1,0, // vertex 3

    1, 1,0, // vertex 3
    -1, 1,0, // vertex 4
    -2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBasket1 ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -2,0,0, // vertex 1
    -1,0,0, // vertex 2
    -0.5, 1,0, // vertex 3

   -0.5, 1,0, // vertex 3
    -2.5, 1,0, // vertex 4
    -2,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Basket1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBasket2 ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    1,0,0, // vertex 1
    2,0,0, // vertex 2
    2.5, 1,0, // vertex 3

   2.5, 1,0, // vertex 3
    0.5, 1,0, // vertex 4
    1,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Basket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCannonBase ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,-0.5,0, // vertex 1
    0.5,-0.5,0, // vertex 2
    0.5, 0.5,0, // vertex 3

   0.5, 0.5,0, // vertex 3
    0, 0.5,0, // vertex 4
    0,-0.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  CannonBase = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannonFace ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,-0.2,0, // vertex 1
    1,-0.2,0, // vertex 2
    1, 0.2,0, // vertex 3

   1, 0.2,0, // vertex 3
    0, 0.2,0, // vertex 4
    0,-0.2,0//i,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  CannonFace = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
struct VAO* Line;
void createLine()
{
// GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -3.5,0,0, // vertex 1
    3.5,0,0, // vertex 2
    3.5, 0.025,0, // vertex 3

   3.5, 0.025,0, // vertex 3
    -3.5, 0.025,0, // vertex 4
    -3.5,0,0//i,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Line = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
struct VAO* seedha;
void createseedha()
{
// GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.3,0,0, // vertex 2
    0.3, 0.1,0, // vertex 3

   0.3, 0.1,0, // vertex 3
    0, 0.1,0, // vertex 4
    0,0,0//i,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  seedha = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
struct VAO* tedha;
void createtedha()
{
// GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.05,0,0, // vertex 2
    0.05, 0.3,0, // vertex 3

   0.05, 0.3,0, // vertex 3
    0, 0.3,0, // vertex 4
    0,0,0//i,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  tedha = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
struct VAO* Red;
struct VAO* Black;
struct VAO* Green;
struct VAO* Bullet;
struct VAO* MIRR1;
struct VAO* MIRR2;
struct VAO* MIRR3;
struct VAO* Board;
void createRed ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.2,0,0, // vertex 2
    0.2,0.3,0, // vertex 3

    0.2,0.3,0, // vertex 3
    0,0.3,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Red = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBlack ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.2,0,0, // vertex 2
    0.2,0.3,0, // vertex 3

   0.2,0.3,0, // vertex 3
    0,0.3,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Black = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createGreen ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.2,0,0, // vertex 2
    0.2, 0.3,0, // vertex 3

   0.2, 0.3,0, // vertex 3
    0, 0.3,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Green = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBullet ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.1,0,0, // vertex 2
    0.1, 0.05,0, // vertex 3

   0.1, 0.05,0, // vertex 3
    0, 0.05,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Bullet = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBoard ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    1,0,0, // vertex 2
    1, 1,0, // vertex 3

   1, 1,0, // vertex 3
    0, 1,0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Board = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createMIRR1 ()
{
        // GL3 accepts only Triangles. Quads are not supported static
        const GLfloat vertex_buffer_data [] = {
                -0.4,0.025,0, // vertex 1
                -0.4,-0.025,0, // vertex 2
                0.4, -0.025,0, // vertex 3
     
                0.4, -0.025,0, // vertex 3
                0.4, 0.025,0, // vertex 4
                -0.4,0.025,0,  // vertex 1
        };
        
        static const GLfloat color_buffer_data [] = {
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
     
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
        
        };
        
        // create3DObject creates and returns a handle to a VAO that can be used later
        MIRR1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createMIRR2 ()
{
        // GL3 accepts only Triangles. Quads are not supported static
        const GLfloat vertex_buffer_data [] = {
                -0.4,0.025,0, // vertex 1
                -0.4,-0.025,0, // vertex 2
                0.4, -0.025,0, // vertex 3

                0.4, -0.025,0, // vertex 3
                0.4, 0.025,0, // vertex 4
                -0.4,0.025,0,  // vertex 1
        };

        static const GLfloat color_buffer_data [] = {
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1

                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1

        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        MIRR2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createMIRR3 ()
{
        // GL3 accepts only Triangles. Quads are not supported static
        const GLfloat vertex_buffer_data [] = {
                -0.4,0.025,0, // vertex 1
                -0.4,-0.025,0, // vertex 2
                0.4, -0.025,0, // vertex 3

                0.4, -0.025,0, // vertex 3
                0.4, 0.025,0, // vertex 4
                -0.4,0.025,0,  // vertex 1
        };

        static const GLfloat color_buffer_data [] = {
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1

                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1
                0.52f,0.8f,0.98f, // color 1

        };

        // create3DObject creates and returns a handle to a VAO that can be used later
        MIRR3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
long long int last_update_time=0;
long long int countred=0;
float red[1000]={4};
float redy[1000]={4};
long long int countblack=0;
float black[1000]={4};
float blacky[1000]={4};
long long int countgreen=0;
float green[1000]={4};
float greeny[1000]={4};
float bulletx[1000]={0};
long long int countbullet=0;
void declare()
{
	long long int current_time;
	current_time = glutGet(GLUT_ELAPSED_TIME); // Time in seconds
        if ((current_time - last_update_time) >= 1000/speed) { // atleast 1000 milisecond s elapsed since last frame
            for(int j=0;j<countred;j++)
            {
                    redy[j]=redy[j]-0.3*speed;
                   // cout << redy[j] << endl;

            }
            for(int j=0;j<countblack;j++)
            {
                    blacky[j]=blacky[j]-0.3*speed;
                   // cout << redy[j] << endl;

            }
            for(int j=0;j<countgreen;j++)
            {
                    greeny[j]=greeny[j]-0.3*speed;
                   // cout << redy[j] << endl;

            }
            int value=rand()%5;
	    int position=rand()%6-3;
	    //if(position<-3)
            //position++;
            //else if (position>4)
   	    //position--;
            if(value==0)
            {
                    countred++;        
                    red[countred-1]=position;
                    redy[countred-1]=4;
            }
	else if(value==2)
            {
                    countblack++;
          //          int position=rand()%8-4;
                    black[countblack-1]=position;
                    blacky[countblack-1]=4;
            }
            else if(value==4)
            {
                    countgreen++;
            //        int position=rand()%8-4;
                    green[countgreen-1]=position;
                    greeny[countgreen-1]=4;
            }
            last_update_time = current_time;
        }
	/*long long time=0;
		current_time = glutGet(GLUT_ELAPSED_TIME); // Time in seconds
	if(current_time - time >100)
	{	countbullet++;
	bulletx[countbullet-1]+=0.05;
	}*/
 
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
Matrices.projection = glm::ortho(-(4.0f)/zoom+xchang, (4.0f)/zoom+xchang, (-4.0f)/zoom+ychang, (4.0f)/zoom+ychang, 0.1f, 500.0f);

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
 /* Matrices.model = glm::mat4(1.0f);

  /* Render your scene 

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);*/
	//MIRR1
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateMIRR1 = glm::translate (glm::vec3(3.0f, 0.0f, 0.0f)); // glTranslatef
        glm::mat4 rotateMIRR1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= translateMIRR1*rotateMIRR1;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        srand(time(NULL));
        draw3DObject(MIRR1);
       //MIRR2
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateMIRR2 = glm::translate (glm::vec3(2.0f, 3.0f, 0.0f)); // glTranslatef
        glm::mat4 rotateMIRR2 = glm::rotate((float)(120*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= translateMIRR2*rotateMIRR2;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        srand(time(NULL));
        draw3DObject(MIRR2);
        //MIRR3
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateMIRR3 = glm::translate (glm::vec3(1.0f, -2.0f, 0.0f)); // glTranslatef
        glm::mat4 rotateMIRR3 = glm::rotate((float)(60*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= translateMIRR3*rotateMIRR3;
        MVP = VP * Matrices.model; // MVP = p * V * M
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        srand(time(NULL));
        draw3DObject(MIRR3);
if(neg){
/*neg*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translaten = glm::translate (glm::vec3(2.5, 3.45, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translaten);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(s1)
{
/*seedha1*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate1 = glm::translate (glm::vec3(3.1, 3.15, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(s2){
/*seedha2*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate2 = glm::translate (glm::vec3(3.1, 3.45, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(s3){
/*seedha3*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate3 = glm::translate (glm::vec3(3.1, 3.75, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate3);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(f1){
/*tedha1*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate11 = glm::translate (glm::vec3(3.05, 3.2, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate11);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(f2){
/*tedha2*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate12= glm::translate (glm::vec3(3.05, 3.5, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate12);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(f3){
/*tedha4*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate14 = glm::translate (glm::vec3(3.4, 3.2, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate14);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(f4){
/*tedha3*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate13= glm::translate (glm::vec3(3.4, 3.5, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate13);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(s4){
/*seedha4*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate4 = glm::translate (glm::vec3(3.6, 3.15, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate4);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(s5){
/*seedha5*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate5 = glm::translate (glm::vec3(3.6, 3.45, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate5);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(s6){
/*seedha6*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate6 = glm::translate (glm::vec3(3.6, 3.75, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate6);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(seedha);}
if(f21){
/*tedha21*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate21 = glm::translate (glm::vec3(3.55, 3.2, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate21);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(f22){
/*tedha22*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate22= glm::translate (glm::vec3(3.55, 3.5, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate22);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(f23){
/*tedha24*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate24 = glm::translate (glm::vec3(3.9, 3.2, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate24);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
if(f24){
/*tedha23*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translate23= glm::translate (glm::vec3(3.9, 3.5, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translate23);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(tedha);}
 /*Basket 1*/ Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateBasket1 = glm::translate (glm::vec3(-0.5+mov1, -4, 0));        // glTranslatef
  //glm::mat4 rotateBasket1 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateBasket1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(Basket1);
	
  /*Basket 2*/Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateBasket2 = glm::translate (glm::vec3(0.5+mov2, -4, 0));        // glTranslatef
  //glm::mat4 rotateBasket2 = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateBasket2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(Basket2);
  
  /*cannonBase */Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCannonBase = glm::translate (glm::vec3(-4, 0+movcannon, 0));        // glTranslatef
  //glm::mat4 rotateCannonBase = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateCannonBase);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(CannonBase);
  
  /*cannonFace */Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCannonFace = glm::translate (glm::vec3(-3.75, 0+movcannon, 0));        // glTranslatef
  glm::mat4 rotateCannonFace = glm::rotate((float)(rotatecannon*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateCannonFace*rotateCannonFace);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(CannonFace);
  
  /*Line */Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateLine = glm::translate (glm::vec3(0, -3, 0));        // glTranslatef
  //glm::mat4 rotateCannonFace = glm::rotate((float)(rotatecannon*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateLine);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(Line);
/*bullet*/
	long long int i;

for(i=0;i<count;i++)
  {
	if(FLAG_GOLI[i]==0){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBrick = glm::translate (glm::vec3(bulletx[i], YBULLET[i], 0));  // glTranslatef
  glm::mat4 translate1 = glm::translate (glm::vec3(-3.75, CANNONY[i], 0));  // glTranslatef
  glm::mat4 translate2 = glm::translate (glm::vec3(3.75, 0, 0));  // glTranslatef
  glm::mat4 rotateBullet = glm::rotate((float)((ROTATEBULLET[i])*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateBrick*translate1*rotateBullet);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(Bullet);
	}

  }
/*bocks*/
  for(i=0;i<countred;i++)
  {
	
		if(flagred[i]==0)
	{
	int flag=0;
	if(redy[i]<=(-3))
		flag=1;
	if((redy[i]<=(-3) && red[i]<=(2.5+mov2)) || (redy[i]<=(-3) && (red[i] +0.2)>=(0.5+mov2)))
		{
			flag=1;
			score+=1;
			flagred[i]=1;
		}
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBrick = glm::translate (glm::vec3(red[i], redy[i], 0));  // glTranslatef
  Matrices.model *= (translateBrick);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(flag==0)
  draw3DObject(Red);
	}
  }
  for(i=0;i<countblack;i++)
  { 
  if(flagblack[i]==0)
  {
	int flag=0;
	if((blacky[i]<=(-3) && black[i]<=(-0.5+mov1)) || (blacky[i]<=(-3) && (black[i] +0.2)>=(-2.5+mov1)))
		flag=1;
	if((blacky[i]<=(-3) && black[i]<=(2.5+mov2)) || (blacky[i]<=(-3) && (black[i] +0.2)>=(0.5+mov2)))
		flag=1;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateStone = glm::translate (glm::vec3(black[i], blacky[i], 0));  // glTranslatef
  Matrices.model *= (translateStone);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(flag==0)
  draw3DObject(Black);
	else
		{
	cout << score <<"\n";
	cout << "GAME OVER\n";
	exit(0);
	   }
	}
  }
  for(i=0;i<countgreen;i++)
  {
	if(FLAG_GRN[i]==0)
	{
	int flag=0;
	if((greeny[i]<=(-3) && green[i]<=(-0.5+mov1)) || (greeny[i]<=(-3) && (green[i] +0.2)>=(-2.5+mov1)))
		{
			flag=1;
			score+=1;
			FLAG_GRN[i]=1;
		}
	if(greeny[i]<=(-3))
		flag=1;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateGrass = glm::translate (glm::vec3(green[i], greeny[i], 0));  // glTranslatef
  Matrices.model *= (translateGrass);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(flag==0)
  draw3DObject(Green);
	} 
 }
long long int j;
        float cx,bx,cy,by,w,bw;
        for(i=0;i<count;i++)
        {
                cx=-3.75+bulletx[i];
                cy=CANNONY[i]+YBULLET[i];
                w=0.05;
                if(FLAG_GOLI[i]==0 && FLAG_GOLIMIRR1[i]==0)
                {
                                        bx=3;
                                        by=0;
                                        bw=0.025;
                                        if((abs(bx-cx)<=w+bw) && (abs(by-cy)<=w+0.4))
                                        {
                                                FLAG_GOLIMIRR1[i]=1;
                                                ROTATEBULLET[i]=2*90-ROTATEBULLET[i];
                                                break;
                                        }
                }
		if(FLAG_GOLI[i]==0 && FLAG_GOLIMIRR2[i]==0)
                {
                                        bx=2;
                                        by=3;
                                        bw=0;
                                        if((abs(bx-cx)<=w+bw) && (abs(by-cy)<=w+0.3))
                                        {
                                                FLAG_GOLIMIRR2[i]=1;
                                                ROTATEBULLET[i]=2*120-ROTATEBULLET[i];
                                                break;
                                        }




                }
                if(FLAG_GOLI[i]==0 && FLAG_GOLIMIRR3[i]==0)
                {
                                        bx=1;
                                        by=-2;
                                        bw=0;
                                        if((abs(bx-cx)<=w+bw) && (abs(by-cy)<=w+0.3))
                                        {
                                                FLAG_GOLIMIRR3[i]=1;
                                                ROTATEBULLET[i]=2*60-ROTATEBULLET[i];
                                                break;
                                        }

                }

	}


        for(i=0;i<count;i++)
        {
                cx=-3.75+bulletx[i];
                cy=CANNONY[i]+YBULLET[i];
                w=0.01;
                if(FLAG_GOLI[i]==0)
                {
                        for(j=0;j<countblack;j++)
                        {
                                if(flagblack[i]==0)
                                {
                                        bx=black[j];
                                        by=blacky[j];
                                        if((abs(bx-cx)<=0.075) && (abs(by-cy)<=0.3))
                                        {
                                                FLAG_GOLI[i]=1;
                                                flagblack[j]=1;
                                             //   perfect shoot
                                                score+=2;
                                                break;

                                        }
                                }

                        }
                }
                if(FLAG_GOLI[i]==0)
                {
                        for(j=0;j<countred;j++)
                        {
                                if(flagred[i]==0)
                                {
                                        bx=red[j];
                                        by=redy[j];
                                        //bw=0.1;
                                        if((abs(bx-cx)<=0.075) && (abs(by-cy)<=0.3))
                                        {
                                                FLAG_GOLI[i]=1;
                                                flagred[j]=1;
                                     		score-=2;
                                                break;

                                        }
                                }



                        }
                }
                 if(FLAG_GOLI[i]==0)
                {
                        for(j=0;j<countgreen;j++)
                        {
                                if(FLAG_GRN[i]==0)
                                {
                                        bx=green[j];
                                        by=greeny[j];
                                        bw=0.1;
                                        if((abs(bx-cx)<=0.075) && (abs(by-cy)<=0.3))
                                        {
                                                FLAG_GOLI[i]=1;
                                                FLAG_GRN[j]=1;
                                              	score-=2;
                                                break;

                                        }
                                }



                        }
                }

        }

  // Swap the frame buffers
  glutSwapBuffers ();
	

  // Increment angles
  //float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scenei
//bulletx[0]=-3.75;
int a,b,s;
if(score>=100)
{
	cout << score <<"\n";
	cout << "YOU WON\n";
	exit(0);
}
if(score<0)
{
	neg=1;
	s=score*-1;
}
else
{
	neg=0;
	s=score;
}
b=s%10;
a=s/10;
switch(a)
{
case 0:
	f1=1;
	f2=1;
	f3=1;
	f4=1;
	s1=1;
	s2=0;
	s3=1;
	break;
case 1:
	f1=0;
	f2=0;
	f3=1;
	f4=1;
	s1=0;
	s2=0;
	s3=0;
	break;
case 2:
	f1=1;
	f2=0;
	f3=0;
	f4=1;
	s1=1;
	s2=1;
	s3=1;
	break;
case 3:
	f1=0;
	f2=0;
	f3=1;
	f4=1;
	s1=1;
	s2=1;
	s3=1;
	break;
case 4:
	f1=0;
	f2=1;
	f3=1;
	f4=1;
	s1=0;
	s2=1;
	s3=0;
	break;
case 5:
	f1=0;
	f2=1;
	f3=1;
	f4=0;
	s1=1;
	s2=1;
	s3=1;
	break;
case 6:
	f1=1;
	f2=1;
	f3=1;
	f4=0;
	s1=1;
	s2=1;
	s3=1;
	break;
case 7:
	f1=0;
	f2=0;
	f3=1;
	f4=1;
	s1=0;
	s2=0;
	s3=1;
	break;
case 8:
	f1=1;
	f2=1;
	f3=1;
	f4=1;
	s1=1;
	s2=1;
	s3=1;
	break;
case 9:
	f1=0;
	f2=1;
	f3=1;
	f4=1;
	s1=1;
	s2=1;
	s3=1;
	break;
default:
	break;
}
switch(b)
{
case 0:
	f21=1;
	f22=1;
	f23=1;
	f24=1;
	s4=1;
	s5=0;
	s6=1;
	break;
case 1:
	f21=0;
	f22=0;
	f23=1;
	f24=1;
	s4=0;
	s5=0;
	s6=0;
	break;
case 2:
	f21=1;
	f22=0;
	f23=0;
	f24=1;
	s4=1;
	s5=1;
	s6=1;
	break;
case 3:
	f21=0;
	f22=0;
	f23=1;
	f24=1;
	s4=1;
	s5=1;
	s6=1;
	break;
case 4:
	f21=0;
	f22=1;
	f23=1;
	f24=1;
	s4=0;
	s5=1;
	s6=0;
	break;
case 5:
	f21=0;
	f22=1;
	f23=1;
	f24=0;
	s4=1;
	s5=1;
	s6=1;
	break;
case 6:
	f21=1;
	f22=1;
	f23=1;
	f24=0;
	s4=1;
	s5=1;
	s6=1;
	break;
case 7:
	f21=0;
	f22=0;
	f23=1;
	f24=1;
	s4=0;
	s5=0;
	s6=1;
	break;
case 8:
	f21=1;
	f22=1;
	f23=1;
	f24=1;
	s4=1;
	s5=1;
	s6=1;
	break;
case 9:
	f21=0;
	f22=1;
	f23=1;
	f24=1;
	s4=1;
	s5=1;
	s6=1;
	break;
default:
	break;
}
int i;
float angle=rotatecannon;
for(i=0;i<count;i++)
{		if(YBULLET[i]+CANNONY[i]<=4 && YBULLET[i]+CANNONY[i]>=-3 )
	        YBULLET[i]+=0.1*sin((ROTATEBULLET[i]*M_PI)/180.0f);
		else FLAG_GOLI[i]=1;
		if(bulletx[i]-3.75<=4 && bulletx[i]-3.75>=-4)
                bulletx[i]+=0.1*cos((ROTATEBULLET[i]*M_PI)/180.0f);
		else FLAG_GOLI[i]=1;
}
	declare();
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	
	createBasket1 ();
	createBasket2 ();
	createCannonBase ();
	createCannonFace ();
	createRed();
	createGreen();
	createBlack();
	createBullet();
	createMIRR1();
	createMIRR2();
	createMIRR3();
	createLine();
	createBoard();
	createseedha();
	createtedha();
	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 800;
	initialise();
	/*long long int cur=0,time;
	time=glutGet(GLUT_ELAPSED_TIME);
	if(cur!=time){
	declare();
	cur=time;
	time=glutGet(GLUT_ELAPSED_TIME);
	}
*/	
    initGLUT (argc, argv, width, height);
//cout << glutGet(GLUT_ELAPSED_TIME);
    addGLUTMenus ();

	initGL (width, height);

    glutMainLoop ();

    return 0;
}
