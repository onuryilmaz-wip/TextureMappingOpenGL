/*
 * Onur Yilmaz
 * CENG 477
 * HW#3
 * 
 */

// Headers
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <cmath>
#include <vector> 
#include <stdio.h>
#include <GL/glut.h>

using namespace std;

// Light related variables
GLfloat lightPosition[4];
GLfloat lightAmbient[4];
GLfloat lightDiffuse[4];
GLfloat lightSpecular[4];

// Material related variables
GLfloat materialAmbient[4];
GLfloat materialDiffuse[4];
GLfloat materialSpecular[4];

// Box related variables
GLfloat boxVertices[8][3];
int boxPolygons[12][3];

// Keyboard button related booleans
bool lPressed;
bool kPressed;
bool iPressed;
bool jPressed;
bool oPressed;
bool uPressed;
bool upPressed;
bool downPressed;
bool leftPressed;
bool rightPressed;

// Number of models in the scene
int numberOfMesh = 0;

// Rotation related variables
float rotateInX = 0;
float rotateInY = 0;

// All textures in the scene
GLuint *textures;

// Vertex structure
typedef struct Vertex {
    int VertexId;

    GLfloat x;
    GLfloat y;
    GLfloat z;

    GLfloat u;
    GLfloat v;

} Vertex;

// Polygon structure 
typedef struct VertexIndex {
    int first;
    int second;
    int third;

    GLfloat polygonNormal[3];

} VertexIndex;

// Mesh structure
typedef struct Mesh {
    // Mesh ID
    int MeshId;

    // Filename for texture
    char* fileName;

    // Number variables
    int numberOfVertices; //d1
    int numberOfTextureCoordinates; //d2
    int numberOfPolygons; //d3

    // Transformation, rotation and scale
    GLfloat transformation[3];
    GLfloat rotation[3];
    GLfloat scale[3];

    // Texture
    unsigned char* texturePointer;
    int textureWidth;
    int textureHeight;

    // Vertices and polygon indices
    vector <Vertex*> vertices;
    vector <VertexIndex*> polygonIndices;

} Mesh;

// All models in the scene
vector<Mesh*> allMeshes;

/*
 * Function for ppm reading
 * Taken from the course website
 * 
 */
unsigned char* ppmRead(char* filename, int* width, int* height) {

    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];

    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }

    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }

    i = 0;
    while (i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#')
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }

    image = (unsigned char*) malloc(sizeof ( unsigned char) * w * h * 3);
    fread(image, sizeof ( unsigned char), w * h * 3, fp);
    fclose(fp);

    *width = w;
    *height = h;
    return image;

}

/*
 * Function for calculating the surface normal of a triangle
 */
void calculateTriangleNormal(float *a, float *b, float *c, float* returnArray) {

    // b-a
    float v11 = b[0] - a[0];
    float v12 = b[1] - a[2];
    float v13 = b[2] - a[2];

    // c-a
    float v21 = c[0] - a[0];
    float v22 = c[1] - a[2];
    float v23 = c[2] - a[2];

    float normal_x, normal_y, normal_z, length;

    // Cross product for (b-a) x (c-a)
    normal_x = v12 * v23 - v13 * v22;
    normal_y = v13 * v21 - v11 * v23;
    normal_z = v11 * v22 - v12 * v21;

    // Normalize
    length = 1.0f / sqrt(normal_x * normal_x + normal_y * normal_y + normal_z * normal_z);

    // Return results
    returnArray[0] = -normal_x * length;
    returnArray[1] = -normal_y * length,
            returnArray[2] = -normal_z * length;

}

/*
 * Screen initializing for rendering
 */
void screenInitializing() {

    // Enable normalization
    glEnable(GL_NORMALIZE);

    // Enable light
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Light assignments
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Material assignments
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);

    // Blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Anti-aliasing
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Viewport
    glViewport(0, 0, 600, 600);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Perspective
    gluPerspective(80, 1, 1, 12000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera location
    gluLookAt(0, 0, -75, 0, 1, 1, 0, 1, 0);

    // Read texture files
    for (int k = 0; k < numberOfMesh; k++) {

        // Filename 
        char* temporarySend = allMeshes[k]->fileName;
        // cout << "Read file: " << temporarySend;

        allMeshes[k]->texturePointer = ppmRead(temporarySend, &(allMeshes[k]->textureWidth), &(allMeshes[k]->textureHeight));
    }

    // Assign arbitrary number of indices
    textures = new GLuint[100];

    // Initialize as needed
    glGenTextures(numberOfMesh, textures);

    // Pixel storage
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create each texture
    for (int k = 0; k < numberOfMesh; k++) {
        // Bind
        glBindTexture(GL_TEXTURE_2D, textures[k]);

        // Read
        glTexImage2D(GL_TEXTURE_2D, 0, 3, allMeshes[k]->textureWidth, allMeshes[k]->textureHeight,
                0, GL_RGB, GL_UNSIGNED_BYTE, allMeshes[k]->texturePointer);

        // Parameters
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    }
}

/*
 * Function for drawing the mesh models with texture mapping
 */
void drawModels() {

    // For each model
    for (int j = 0; j < numberOfMesh; j++) {

        // Read number of polygons
        int numberOfPolygonsToDraw = allMeshes[j]->numberOfPolygons;

        // For each polygon
        for (int i = 0; i < numberOfPolygonsToDraw; i++) {

            glPushMatrix();

            // Enable texturing
            glEnable(GL_TEXTURE_2D);

            // Translate, rotate and scale
            glTranslatef(0.0, 0.0, 0.0);
            glTranslatef(allMeshes[j]->transformation[0], allMeshes[j]->transformation[1], allMeshes[j]->transformation[2]);
            glRotatef(allMeshes[j]->rotation[0] + rotateInX, 1.0, 0.0, 0.0);
            glRotatef(allMeshes[j]->rotation[1] + rotateInY, 0.0, 1.0, 0.0);
            glRotatef(allMeshes[j]->rotation[2], 0.0, 0.0, 1.0);
            glScalef(allMeshes[j]->scale[0], allMeshes[j]->scale[1], allMeshes[j]->scale[2]);

            // Normal for surface
            glNormal3f(allMeshes[j]->polygonIndices[i]->polygonNormal[0], allMeshes[j]->polygonIndices[i]->polygonNormal[1], allMeshes[j]->polygonIndices[i]->polygonNormal[2]);

            // Bind texture
            glBindTexture(GL_TEXTURE_2D, textures[j]);

            glBegin(GL_TRIANGLES);

            // First vertex
            int temp = allMeshes[j]->polygonIndices[i]->first;
            glTexCoord2f(allMeshes[j]->vertices[temp]->u, allMeshes[j]->vertices[temp]->v);
            glVertex3f(allMeshes[j]->vertices[temp]->x, allMeshes[j]->vertices[temp]->y, allMeshes[j]->vertices[temp]->z);

            // Second vertex
            temp = allMeshes[j]->polygonIndices[i]->second;
            glTexCoord2f(allMeshes[j]->vertices[temp]->u, allMeshes[j]->vertices[temp]->v);
            glVertex3f(allMeshes[j]->vertices[temp]->x, allMeshes[j]->vertices[temp]->y, allMeshes[j]->vertices[temp]->z);

            // Third vertex
            temp = allMeshes[j]->polygonIndices[i]->third;
            glTexCoord2f(allMeshes[j]->vertices[temp]->u, allMeshes[j]->vertices[temp]->v);
            glVertex3f(allMeshes[j]->vertices[temp]->x, allMeshes[j]->vertices[temp]->y, allMeshes[j]->vertices[temp]->z);

            glEnd();

            // Disable texturing
            glDisable(GL_TEXTURE_2D);

            glPopMatrix();
        }
    }
}

/*
 * Function for drawing the scene
 */
void drawScene() {

    // Clear
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Light position
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition); //set position

    // For each vertex
    for (int i = 0; i < 12; i++) {
        float normalArray[3];

        // Calculate normal
        calculateTriangleNormal(boxVertices[boxPolygons[i][1]], boxVertices[boxPolygons[i][2]], boxVertices[boxPolygons[i][0]], normalArray);

        if (i == 0 || i == 1)
            glNormal3f(0, 0, 1); // Viewing face
        if (i == 2 || i == 3)
            glNormal3f(0, 1, 0); // Bottom face  
        if (i == 4 || i == 5)
            glNormal3f(0, -1, 0); // Top face
        if (i == 6 || i == 7)
            glNormal3f(0, 0, -1); // Viewing back face
        if (i == 8 || i == 9)
            glNormal3f(1, 0, 0); // Left face
        if (i == 10 || i == 11)
            glNormal3f(-1, 0, 0); // Right face

        glPushMatrix();

        // Translate and assign color
        glTranslatef(0.0, 0.0, 0.0);
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(200 / 256.0, 200 / 256.0, 200 / 256.0);

        glBegin(GL_TRIANGLES);

        // First vertex
        int temp = boxPolygons[i][0];
        glVertex3f(boxVertices[temp][0], boxVertices[temp][1], boxVertices[temp][2]);

        // Second vertex
        temp = boxPolygons[i][1];
        glVertex3f(boxVertices[temp][0], boxVertices[temp][1], boxVertices[temp][2]);

        // Third vertex
        temp = boxPolygons[i][2];
        glVertex3f(boxVertices[temp][0], boxVertices[temp][1], boxVertices[temp][2]);

        glEnd();

        // Disable color
        glDisable(GL_COLOR_MATERIAL);
        glPopMatrix();
    }

    // Draw the box assigned to light
    glPushMatrix();

    glEnable(GL_COLOR_MATERIAL);
    glColor3f(1.0, 0, 0);
    glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);

    glutSolidSphere(0.4, 20, 20);

    glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();

    // Draw the mesh objects
    drawModels();

    // Swap buffers
    glutSwapBuffers();
}

/*
 * Function for light movement keyboard buttons
 */
void keyboardLightMove1st(unsigned char key, int x, int y) {

    // Switch key and assign boolean
    switch (key) {
        case 'l': case 'L':
            lPressed = true;
            //cout << "l" << endl;
            break;
        case 'k': case 'K':
            kPressed = true;
            //cout << "k" << endl;
            break;
        case 'i': case 'I':
            iPressed = true;
            //cout << "i" << endl;
            break;
        case 'j': case 'J':
            jPressed = true;
            //cout << "j" << endl;
            break;
        case 'o': case 'O':
            oPressed = true;
            //cout << "o" << endl;
            break;
        case 'u': case 'U':
            uPressed = true;
            //cout << "u" << endl;
            break;
            // Written for easiness, although not demanded
        case 27:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

/*
 * Function for toggling light movement keyboard buttons
 */
void keyboardLightMove2nd(unsigned char key, int x, int y) {

    // Switch key and assign boolean
    switch (key) {
        case 'l': case 'L':
            lPressed = false;
            //cout << "l" << endl;
        case 'k': case 'K':
            kPressed = false;
            //cout << "k" << endl;
            break;
        case 'i': case 'I':
            iPressed = false;
            //cout << "i" << endl;
            break;
        case 'j': case 'J':
            jPressed = false;
            //cout << "j" << endl;
            break;
        case 'o': case 'O':
            oPressed = false;
            //cout << "o" << endl;
            break;
        case 'u': case 'U':
            uPressed = false;
            //  cout << "u" << endl;
            break;

    }
}

/*
 * Function for rotating keyboard buttons
 */
void keyboardRotate1st(int key, int x, int y) {

    // Switch key and assign boolean
    switch (key) {
        case GLUT_KEY_UP:
            upPressed = true;
            // cout << "yukari" << endl;
            break;

        case GLUT_KEY_DOWN:
            downPressed = true;
            // cout << "asagi" << endl;
            break;

        case GLUT_KEY_LEFT:
            leftPressed = true;
            // cout << "sol" << endl;
            break;

        case GLUT_KEY_RIGHT:
            rightPressed = true;
            // cout << "sağ" << endl;
            break;
    }

}

/*
 * Function for toggling rotating keyboard buttons
 */
void keyboardRotate2nd(int key, int x, int y) {

    // Switch key and assign boolean
    switch (key) {
        case GLUT_KEY_UP:
            upPressed = false;
            // cout << "yukari 2" << endl;
            break;

        case GLUT_KEY_DOWN:
            downPressed = false;
            // cout << "asagi 2" << endl;
            break;

        case GLUT_KEY_LEFT:
            leftPressed = false;
            // cout << "sol 2" << endl;
            break;

        case GLUT_KEY_RIGHT:
            rightPressed = false;
            // cout << "sağ 2" << endl;
            break;
    }
}

/*
 * Function for changing variables on time basis
 */
void changeVariables(int input) {


    if (lPressed)
        lightPosition[0] += 0.3;

    if (kPressed)
        lightPosition[0] -= 0.3;

    if (iPressed)
        lightPosition[1] += 0.3;

    if (jPressed)
        lightPosition[1] -= 0.3;

    if (oPressed)
        lightPosition[2] += 0.3;

    if (uPressed)
        lightPosition[2] -= 0.3;

    if (upPressed)
        rotateInX += 3;

    if (downPressed)
        rotateInX -= 3;

    if (leftPressed)
        rotateInY += 3;

    if (rightPressed)
        rotateInY -= 3;

    glutPostRedisplay();
    glutTimerFunc(20, changeVariables, 1);
}

/*
 * Main function 
 */
int main(int argc, char ** argv) {

    // Stream for input file
    ifstream inputFile;
    inputFile.open(argv[1], ios::in);

    // Open file
    if (inputFile.is_open()) {

        float x, y, z, t;

        // Read tag
        string tag;
        inputFile >> tag;

        // Until the end of file
        while (!inputFile.eof()) {

            // Read light
            if (tag == "#Light") {

                // Light position
                for (int i = 0; i < 4; i++) {
                    inputFile >> x;
                    lightPosition[i] = x;
                }

                // Light ambient
                for (int i = 0; i < 4; i++) {
                    inputFile >> x;
                    lightAmbient[i] = x;
                }

                // Light diffuse
                for (int i = 0; i < 4; i++) {
                    inputFile >> x;
                    lightDiffuse[i] = x;
                }

                // Light specular
                for (int i = 0; i < 4; i++) {
                    inputFile >> x;
                    lightSpecular[i] = x;
                }
            }

                // Read box
            else if (tag == "#Box") {

                // Box vertices
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 3; j++) {
                        inputFile >> x;
                        boxVertices[i][j] = x;
                        //  cout << " " << x;
                    }
                }

                // Box polygon indices
                for (int i = 0; i < 12; i++) {
                    for (int j = 0; j < 3; j++) {
                        inputFile >> x;
                        boxPolygons[i][j] = x;
                        // cout << " " << x;
                    }
                }

            }

                // Read material
            else if (tag == "#Material") {

                // Ambient
                for (int j = 0; j < 3; j++) {
                    inputFile >> x;
                    materialAmbient[j] = x;
                }

                // Diffuse
                for (int j = 0; j < 3; j++) {
                    inputFile >> x;
                    materialDiffuse[j] = x;
                }

                // Specular
                for (int j = 0; j < 3; j++) {
                    inputFile >> x;
                    materialSpecular[j] = x;
                }
            }
                // Read mesh
            else if (tag == "#Mesh") {

                // Increase number of mesh
                numberOfMesh++;

                // Allocate space for model
                Mesh* model = (Mesh*) malloc(sizeof (Mesh));

                // Assign ID
                model->MeshId = numberOfMesh;

                // Read and assign texture filename
                char * temporaryChar = (char*) malloc(sizeof (char) *50);
                inputFile >> temporaryChar;
                model->fileName = temporaryChar;
                //cout << model->fileName << endl;

                // Read transformation
                for (int j = 0; j < 3; j++) {
                    inputFile >> x;
                    model->transformation[j] = x;
                    // cout << "Transformation :" << model->transformation[j] << endl;
                }

                // Read rotation
                for (int j = 0; j < 3; j++) {
                    inputFile >> x;
                    model->rotation[j] = x;
                }

                // Read scale
                for (int j = 0; j < 3; j++) {
                    inputFile >> x;
                    model->scale[j] = x;
                }

                // d1: Number of vertices in the mesh
                inputFile >> model->numberOfVertices;
                int verticeCount = model->numberOfVertices;
                //cout << "Vertex count: " << model->numberOfVertices << endl;

                // For each vertex 
                for (int t = 0; t < verticeCount; t++) {

                    // Allocate space for vertex
                    Vertex* toInsert = (Vertex*) malloc(sizeof (Vertex));
                    inputFile >> x >> y >> z;

                    // Read
                    toInsert->VertexId = t;
                    toInsert->x = x;
                    toInsert->y = y;
                    toInsert->z = z;

                    // Push back to list
                    model->vertices.push_back(toInsert);
                }
                //cout << "d1: " << model->vertices.size() << endl;

                // d2: Number of texture coordinates in mesh
                inputFile >> model->numberOfTextureCoordinates;
                int textureCount = model->numberOfTextureCoordinates;
                // cout << "Texture count: " << model->numberOfTextureCoordinates << endl;

                // For each texture part
                for (int t = 0; t < textureCount; t++) {

                    // Read
                    inputFile >> x >> y;
                    model->vertices[t]->u = x;
                    model->vertices[t]->v = y;

                }

                // d3: Number of polygons in mesh
                int indexCount;
                inputFile >> model->numberOfPolygons;
                indexCount = model->numberOfPolygons;
                //cout << "Polygon count: " << model->numberOfPolygons << endl;

                // For each polygon
                for (int t = 0; t < indexCount; t++) {
                    int i1, i2, i3;

                    // Allocate space for vertex index
                    VertexIndex* toInsert = (VertexIndex*) malloc(sizeof (VertexIndex));
                    inputFile >> i1 >> i2 >> i3;

                    // Read
                    toInsert->first = i1;
                    toInsert->second = i2;
                    toInsert->third = i3;

                    // Calculate normal
                    float normal1[3] = {model->vertices[i1]->x, model->vertices[i1]->y, model->vertices[i1]->z};
                    float normal2[3] = {model->vertices[i2]->x, model->vertices[i2]->y, model->vertices[i2]->z};
                    float normal3[3] = {model->vertices[i3]->x, model->vertices[i3]->y, model->vertices[i3]->z};
                    calculateTriangleNormal(normal1, normal2, normal3, toInsert->polygonNormal);

                    // Insert into list
                    model->polygonIndices.push_back(toInsert);
                }
                //cout << "d3: " << model->polygonIndices.size() << endl;

                // Push the model into list
                allMeshes.push_back(model);
            }
            // Read tag
            inputFile >> tag;
        }
    }

    // Initialize
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    // Window
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("CENG477 - HW#3 No: 1627868");

    // Display
    glutDisplayFunc(drawScene);

    // Keyboard functions
    glutKeyboardFunc(keyboardLightMove1st);
    glutKeyboardUpFunc(keyboardLightMove2nd);
    glutSpecialFunc(keyboardRotate1st);
    glutSpecialUpFunc(keyboardRotate2nd);

    // Time basis function
    glutTimerFunc(20, changeVariables, 1);

    // Initialize screen
    screenInitializing();

    // Main loop
    glutMainLoop();
}

// End of code
