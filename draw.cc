#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glfw.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/texture.hpp"
#include <algorithm>
#include <layout.h>

struct Particle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a;
	float size, life;
	float depth; // -1 for dead Particle
	bool operator<(const Particle& that) const {
		return depth > that.depth;
	}
};

struct PARTICLE_CONFIG {
	GLuint vertexArrayID;
	GLuint programID;
	GLuint textureID;
	GLuint particleVertexBuff;
	GLuint particlePositionSizeBuff;
	GLuint particleColorBuff;

	float 		lastTime;

	const int 	MaxParticleSize = 1000000;
	Particle 	particleContainer[MaxParticleSize];
	GLfloat 	particlePositionSize[MaxParticleSize*4];
	GLuchar 	particleColor[MaxParticleSize*4];
	int 		particleNum = 0;

	glm::mat4 	projectMatrix;
	glm::mat4 	viewMatrix;
} g;

struct Layout {
	void init();
	int iterate();

} layout;


void sortParticle(Particle* particles, int particleNum) {
	std::sort( particles, particles+particleNum );
}



void initWindow(void ) {
	if ( !glfwInit() ) {
		fprintf(stderr, "cant init glfw!\n");
		exit;
	}

	if ( !glfwOpenWindow(1024, 768, 0, 0, 0, 0, 32, 0, GLFW_WINDOW ) ) {
		glfwTerminate();
	}

	if ( glewInit() != GLEW_OK ) {
		fprintf(stderr, "glew init failed!\n");
		exit;
	}
	glfwSetWindowTitle("Particle graph layout");
	glfwEnable( GLFW_STICKY_KEYS );
}

void initGL(void) {
	glClearColor( 0.0f, 0.0f, 0.4f, 0.0f );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	fprintf(stderr, "start drawing\n");

	GLGenVertexArrays(1, &g.vertexArrayID);
	GLBindVertexArray(g.vertexArrayID);
	g.programID = loadShaders("Particle.vertexshader", "Particle.fragmentshader");

	g.ViewProjectMatrixID = glGetUniformLocation(g.programID, "VP");
	g.TextureiD = glGetUniformLocation(g.programID, "myTextureSample");
	g.Texture = loadDDS("particle.DDS");



	GLfloat vertexBuffData[] = {
		-0.5, -0.5, 0,
		 0.5, -0.5, 0,
		-0.5, 0.5, 0,
		 0.5, 0.5, 0
	};
	GLGenBuffers(1, &g.particleVertexBuff);
	GLBindBuffer(GL_ARRAY_BUFFER, g.particleVertexBuff);
	GLBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffData), vertexBuffData, GL_STATIC_DRAW);

	GLGenBuffers(1, &g.particlePositionSizeBuff);
	GLBindBuffer(GL_ARRAY_BUFFER, g.particlePositionSizeBuff);
	GLBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*g.MaxParticleSize, NULL, GL_STREAM_DRAW);

	GLGenBuffers(1, &g.particleColorBuff);
	GLBindBuffer(GL_ARRAY_BUFFER, g.particleColorBuff);
	GLBufferData(GL_ARRAY_BUFFER, sizeof(GLuchar)*4*g.MaxParticleSize, NULL, GL_STREAM_DRAW);

	g.lastTime = glfwGetTime();


}

void initParticle() {
	for (int i = 0; i < g.MaxParticleSize; ++i) {
		g.particleContainer.life = -1;
	}
	layout.init();
}

void buildMatrix() {
	g.projectMatrix = glm::perspective(0.45, 4.0/3.0, 0.1, 1000.0);
	g.viewMatrix = glm::lookAt(
			glm::vec3(0, 0, 5), // position
			glm::vec3(0, 0, 0), // direction
			glm::vec3(0, 1, 0)  // up
		);
	g.camerPosition = glm::inverse(g.viewMatrix)[3];

}

void updateParticle();

void updateBuffer() {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  float currentTime = glfwGetTime();
  float delta = currentTime - g.lastTime;
  g.lastTime = currentTime;
  buildMatrix();
  if (layout.iterate()) {
  	updateParticle();
  }
  sortParticle(g.particleContainer, g.particleNum);

  GLBindBuffer(GL_ARRAY_BUFFER, g.particlePositionSizeBuff);
  GLBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*g.MaxParticleSize, NULL, GL_STREAM_DRAW);
  GLBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*4*g.particleNum, g.particlePositionSize);

  GLBindBuffer(GL_ARRAY_BUFFER, g.particleColorBuff);
  GLBufferData(GL_ARRAY_BUFFER, sizeof(GLuchar)*4*g.MaxParticleSize, NULL, GL_STREAM_DRAW);
  GLBufferSubData(GL_ARRAY_BUFFER, 0, sizoef(GLuchar)*4*g.MaxParticleSize, g.particleColor);

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glUseProgram( g.programID );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE, g.Texture );
  glUniform1i(g.TextureID);

  glUniformMatrix4fv(g.ViewProjectMatrixID, 1, GL_FALSE, g.projectMatrix*g.viewMatrix);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, g.particleVertexBuff);
  glVertexAttribPointer(
  	0, 
  	3, 
  	GL_FLOAT,
  	GL_FALSE,
  	0, 
  	(void*)0
  );

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, g.particlePositionSizeBuff);
  glVertexAttribPointer(
  	1, 
  	4,
  	GL_FLOAT,
  	GL_FALSE,
  	0, 
  	(void*)0
  );

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, g.particleColorBuff);
  glvertexAttribPointer(
  	2,
  	4, 
  	GL_UNSIGNED_CHAR,
  	GL_TURE,
  	0,
  	(void*)0
  );

  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 1);
  glVertexAttribDivisor(2, 1);

  glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, g.particleNum);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);


}




int main(void) {

	initWindow();
	initGL();
	initParticle();
	do {
		updateBuffer();
		glfwSwapBuffers();
	}while ( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS );

	cleanUp();

	glfwTerminate();

	return 0;
}