//#define GLEW_STATIC 1

#include "display.h"
#include <GL/glew.h>
#include <stdio.h>

Display::Display(int width, int height, const std::string& title) {
    Width = width;
    Height = height;
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Could not init SDL");
        return;
    }
	// ben�tigt opengl 4.0
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

    // bit gr�e der Farben
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 3);
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 3);
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 2);
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1);
    // Tiefenbuffergr��e
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 1);
    // Doublebuffer
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 2);

    // SDL Init
    // Erstelle Fenster
	p_window = SDL_CreateWindow( title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if( p_window == NULL)
        printf( "error\n");
	p_glContext = SDL_GL_CreateContext(p_window);
	if( p_glContext == NULL)
        printf( "error\n");
    p_surface = SDL_CreateRGBSurface( 0, GetTilesetWidth(), GetTilesetHeight(), 32, rmask, gmask, bmask, amask);
    if( p_surface == NULL) {
        printf( "Display: Surface cant create %s\n", SDL_GetError());
    }
    // Renderer
    p_renderer = SDL_CreateRenderer( p_window, -1, SDL_RENDERER_ACCELERATED);
    if( p_renderer == NULL) {
        printf( "Display: Renderer could not be created! SDL Error \"%s\"", SDL_GetError());
    }
	// Glew starten
	GLenum res = glewInit();
    if(res != GLEW_OK) {
		printf( "Glew failed to initialize!" );
    }
    printf("OpenGL version supported by this platform (%s): \n",glGetString(GL_VERSION));

    int Depth;
    SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &Depth);
    printf( "%d Depth\n", Depth);
    // Tiefe n�tzen
    glEnable( GL_DEPTH_TEST);

    // Cull Face
	glEnable( GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Blend
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// background color
	SetBackgroundColor();
}

Display::~Display() {
    SDL_GL_DeleteContext( p_glContext);
	SDL_DestroyWindow( p_window);
	SDL_FreeSurface( p_surface);
	SDL_Quit();
}

void Display::Clear() {
    // setzte background color und clear BUFFER
    glClearColor( p_red, p_green, p_blue, p_alpha);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
}

void Display::SwapBuffers() {
	SDL_GL_SwapWindow( p_window);
}

void Display::SetTitle( std::string& title) {
    SDL_SetWindowTitle( p_window, title.c_str());
}
