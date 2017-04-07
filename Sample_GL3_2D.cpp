#include <iostream>
#include <cmath>
#include <stdio.h>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
const GLfloat DEG2RAD = 3.14159/180.0;
GLfloat posX = 0.01; GLfloat posY = 0.0; GLfloat posZ = 0.0;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
    float CX;
    float CY;
    float r;
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

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
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

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float x = -20.0 ; float y = -84.0; float z = 0.0;
float X = 20.0 ; float Y = -84.0; float Z = 0.0;
float move_unit = 1.5f;
float rot_ang = 0;
double p = 0, q = 0, t=0, u=15, tim =0, tim2=0, tim3=0;
double key_press_time = 0;double key_release_time = 0 , u_f;
int flag = 0, flag1 = 0, flag2 = 0;
int Obs1_o = 0 ;
int Obs2_o = 0 ;
int Obs3_o = 0 ;
int score = 0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    int state1 = glfwGetKey(window, GLFW_KEY_RIGHT_ALT);
    int state2 = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_SPACE:
                key_release_time = glfwGetTime();
                u_f = key_release_time - key_press_time;
                u = u*u_f*2.5;
                flag=1;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;

            case GLFW_KEY_W:
                if( rot_ang >= 50 )
                    rot_ang = 50 ;
                else
                    rot_ang += 5;
                break;
            case GLFW_KEY_S:
                if ( rot_ang <= -50)
                    rot_ang = -50;
                else
                    rot_ang -= 5;
                break;   
            default:
                break;
              }

        switch (key) {

            case GLFW_KEY_RIGHT:
              if(state1==GLFW_PRESS)
                X += move_unit;
              break;
            case GLFW_KEY_LEFT:
              if(state1==GLFW_PRESS)
                X -= move_unit;
              break;
            default:
              break;
        }


    switch (key) {

      case GLFW_KEY_RIGHT:
        if(state2==GLFW_PRESS)
          x += move_unit;
        break;
      case GLFW_KEY_LEFT:
        if(state2==GLFW_PRESS)
          x -= move_unit;
        break;
      default:
        break; 
      }

     
}
}


/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    Matrices.projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, -100.0f, 100.0f);
}

VAO *triangle, *rectangle, *rectangle1, *rectangle2, *rectangle3, *rectangle4, *circle1, *circle2, *circle3, *Obs1, *Obs2, *Obs3, *obj[20], *line;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 10 ,0, // vertex 0
    -10,-10,0, // vertex 1

    10,-10,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -99,0,0, // vertex 1
    -69,0,0, // vertex 2
    -69, 5,0, // vertex 3

    -69,5,0, // vertex 3
    -99, 5, 0, // vertex 4
    -99,0,0  // vertex 1
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
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, 0,0,0);
}


void drawline ()
{
  static const GLfloat vertex_buffer_data [] = {
    72, -2, 0, //vertex1
    82, 22, 0, //vertex2
    72, -2, 0, //vertex3
};

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
};

  line = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);

}

void drawCircle1(GLfloat radius, GLint numberOfSides)
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle1 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,1);

}


void drawCircle2(GLfloat radius, GLint numberOfSides )
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = X;
  circleVerticesY[0] = Y;
  circleVerticesZ[0] = Z;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = X + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = Y + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = Z;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle2 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0,0,0);

}


void drawCircle3(GLfloat a, GLfloat b, GLfloat c, GLfloat radius, GLint numberOfSides )
{
  int numberOfVertices = numberOfSides + 2;

  GLfloat twicePi = 2.0f * M_PI;

  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = a;
  circleVerticesY[0] = b;
  circleVerticesZ[0] = c;

  for ( int i = 1; i < numberOfVertices; i++ )
  {
    circleVerticesX[i] = a + ( radius * cos( i *  twicePi / numberOfSides ) );
    circleVerticesY[i] = b + ( radius * sin( i * twicePi / numberOfSides ) );
    circleVerticesZ[i] = c;
  }

  GLfloat allCircleVertices[( numberOfVertices ) * 3];

  for ( int i = 0; i < numberOfVertices; i++ )
  {
    allCircleVertices[i * 3] = circleVerticesX[i];
    allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
    allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
  }

  circle3 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,1);

}


void createRectangle1 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    0,99,0, // vertex 1
    0,93,0, // vertex 2
    6, 93,0, // vertex 3

    6, 93,0, // vertex 3
    6, 99,0, // vertex 4
    0,99,0  // vertex 1
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
  rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, 2,2,2, GL_FILL);

  //Obs1->r = 1;
  //Obs1->CX = 1;
  //Obs1->CY = 97;

  if(Obs1_o == 0 )
  {
    obj[1] = rectangle1;
    
  }
}

void createRectangle2 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    14,95,0, // vertex 1
    14,89,0, // vertex 2
    20, 89,0, // vertex 3

    20, 89,0, // vertex 3
    20, 95,0, // vertex 4
    14,95,0  // vertex 1
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
  rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0, GL_FILL);

  //Obs1->r = 1;
  //Obs1->CX = 1;
  //Obs1->CY = 97;

  if(Obs2_o == 0)
  {
    obj[2] = rectangle2;   
  }
}

void createRectangle3 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    28,94,0, // vertex 1
    28,88,0, // vertex 2
    34, 88,0, // vertex 3

    34, 88,0, // vertex 3
    34, 94,0, // vertex 4
    28,94,0  // vertex 1
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
  rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0, GL_FILL);

  //Obs1->r = 1;
  //Obs1->CX = 1;
  //Obs1->CY = 97;

  if(Obs3_o == 0)
  {
    obj[3] = rectangle3;   
  }
}


void whichbasket()
{
  if(Obs1_o == 0)
  {
  if(rectangle1->CX > X-12 && rectangle1->CY < X+12 && rectangle1->CY < Y+12 && rectangle1->CY > Y-12)
    {
      //cout<<"1"<<endl;
      tim=0;
      score -= 10;
    }
  else if(rectangle1->CX > x-12 && rectangle1->CY < x+12 && rectangle1->CY < y+12 && rectangle1->CY > y-12)
    {
      //cout<<"21"<<endl;
      tim=0;
      score += 10;
    }
  }
  if(Obs2_o == 0)
  {
   if(rectangle2->CX > X-32 && rectangle2->CY < X+32 && rectangle2->CY < Y+32 && rectangle2->CY > Y-32)
    {
      //cout<<"1"<<endl;
      tim2=0;
      score += 10;
    }
  else if(rectangle2->CX > x-32 && rectangle2->CY < x+32 && rectangle2->CY < y+32 && rectangle2->CY > y-32)
    {
      //cout<<"22"<<endl;
      tim2=0;
      score -= 10;
    }
  }

}

void checkCollision()
{
  float x1 = -99 + p/10;
  float y1 = 2+q/10;
  float r1 = 1;
  //cout<<x1<<"\t"<<y1<<endl;
  float x2 , y2 , d1 , r2 , x3 , y3 , d2 , r3 , x4 , y4, d3 , r4;
  x2 = obj[1]->CX;
  y2 = 100 + obj[1]->CY; 
  d1 = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
  r2 = obj[1]->r;
  if ( d1 < r1 + r2)
  {
    Obs1_o = 1;
    score++;
  }

  x3 = obj[2]->CX;
  y3 = obj[2]->CY+100; 
  d2 = sqrt((x3-x1)*(x3-x1) + (y3-y1)*(y3-y1));
  r3 = obj[2]->r;
  if ( d2 < r1 + r3)
  {
    Obs2_o = 1;
    score++;
  }

  x4 = obj[3]->CX;
  y4 = obj[3]->CY+100; 
  d3 = sqrt((x4-x1)*(x4-x1) + (y4-y1)*(y4-y1));
  r4 = obj[3]->r;
  if ( d3 < r1 + r4)
  {
    Obs3_o = 1;
    score++;
  }
  
}





float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

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

  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices.projection * Matrices.view;

  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 tr = glm::translate (glm::vec3(99, -5, 0));        // glTranslatef
  glm::mat4 rr = glm::rotate((float)(rot_ang*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  glm::mat4 tr1 = glm::translate (glm::vec3(-99, 5, 0));
  Matrices.model *= (  tr1*rr*tr  );

  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
  

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCircle = glm::translate (glm::vec3(posX, posY, posZ));
  
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


  drawCircle1 (12, 360);
  draw3DObject(circle1);

  drawCircle2 (12, 360);
  draw3DObject(circle2);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateLine1 = glm::translate (glm::vec3(72, -2, 0));
  Matrices.model *= (translateLine1 );
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  drawline();
  draw3DObject(line);

  if( flag == 1)
  {
    float z1,z2;
    z1 = -99 + p/10;
    z2 = 2 + q/10;
    //cout<<"\nz1 = "<<z1<<"\t"<<z2<<endl;
    if( z1 > 99.0 || z2 > 100.0 || z2 < -100.0)
    {
      flag = 0;
      p=0;
      q=0;
      u=15;
      t=0;
    }
    drawCircle3 (0, 0, 0, 1,360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCircle = glm::translate (glm::vec3(z1, z2, 0 ));   // glTranslatef
    Matrices.model *= (translateCircle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle3);
    if( z2 == (2.4*z1 - 174.8))
    {
      p = u*cos( 2*atan(2.4) + rot_ang*M_PI/180)*t;
      q = u*sin( 2*atan(2.4) + rot_ang*M_PI/180)*t -t*t;
    }
    else
    { 
      p = u*cos(rot_ang*M_PI/180)*t;
      q = u*sin(rot_ang*M_PI/180)*t -t*t;
    }
    t += 0.08;
  }

  
  
  if( Obs1_o == 0)
  {
    int temp=0;
    float b1;
    if(temp==0)
      b1=0;
    //cout<<"HELLO"<<endl;
    Matrices.model = glm::mat4(1.0f);
    
    glm::mat4 translateObs1 = glm::translate (glm::vec3(0, -tim*tim, 0));       // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      
    Matrices.model *= (translateObs1 );//rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle1);
    //b1= -tim*tim;
    tim+=0.02;
    rectangle1->r = 7;
    rectangle1->CX = 0;
    rectangle1->CY = -tim*tim-3;

    if(tim*tim >= 170)
      tim =0;
    
  }


  if( Obs2_o == 0)
  {
    
    //cout<<"HELLO"<<endl;
    Matrices.model = glm::mat4(1.0f);
    
    glm::mat4 translateObs2 = glm::translate (glm::vec3(0, -tim2*tim2, 0));       // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      
    Matrices.model *= (translateObs2 );//rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle2);
    //b1= -tim*tim;
    
    tim2+=0.03;
    rectangle2->r = 7;
    rectangle2->CX = 0;
    rectangle2->CY = -tim2*tim2-3;

    if(tim2*tim2 >= 170)
      tim2=0;

  }

  if( Obs3_o == 0)
  {
    
    //cout<<"HELLO"<<endl;
    Matrices.model = glm::mat4(1.0f);
    
    glm::mat4 translateObs2 = glm::translate (glm::vec3(0, -tim3*tim3, 0));       // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
      
    Matrices.model *= (translateObs2 );//rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(rectangle3);
    //b1= -tim*tim;
    
    tim3+=0.04;
    rectangle3->r = 7;
    rectangle3->CX = 0;
    rectangle3->CY = -tim3*tim3-3;

    if(tim3*tim3 >= 170)
      tim3=0;

  }


  if( Obs1_o == 1 && Obs2_o == 1 && Obs3_o == 1)
  {
    float z1 = -99 + p/10;
    float z2 = 2 + q/10;
    if( z1 > 99.0 || z2 > 100.0 || z2 < -100.0 )
    {
      cout<<endl<<endl<<"YOU WON!!!  SCORE: "<<score<<endl;
    exit(0);
  }
}


  checkCollision();
  //whichbasket();
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

   // glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
  createRectangle1 ();
  createRectangle2 ();
  createRectangle3 ();
	//createObs1();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.2f, 0.3f, 0.3f, 1.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
    
        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }
    glfwTerminate();
    //exit(EXIT_SUCCESS);
}
