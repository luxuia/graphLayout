#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#include <GL/glew.h>

#include <GL/glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "layout.h"


struct Particle {
	glm::vec3 pos;
	unsigned char r,g,b,a; // Color
	float size;
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};



struct ParticleConfig {
	int count;
	static const int MaxParticles = 100000;
	Particle particlesContainer[MaxParticles];
	BaseLayout* layout;
	GLfloat* positionSizeData;
	GLubyte* colorData;
	GLuint vertexArrayID;
	GLuint programID;
	GLuint cameraRightWorldspaceID;
	GLuint cameraUpWorldspaceID;
	GLuint viewProjectMatrixID;
	GLuint textureID;
	GLuint texture;
	GLuint vertexBuffer;
	GLuint positionBuffer;
	GLuint colorBuffer;


	void init() {
		initBuffer();
		initParticle();
	}

	void initBuffer() {
		
		// Create and compile our GLSL program from the shaders
		programID = LoadShaders( "Particle.vertexshader", "Particle.fragmentshader" );
		// Vertex shader
		cameraRightWorldspaceID  = glGetUniformLocation(programID, "CameraRight_worldspace");
		cameraUpWorldspaceID  = glGetUniformLocation(programID, "CameraUp_worldspace");
		viewProjectMatrixID = glGetUniformLocation(programID, "VP");
		// fragment shader
		textureID  = glGetUniformLocation(programID, "mygTextureSampler");
		texture = loadDDS("particle.DDS");
		bindBuffer();
	}

	void bindBuffer() {
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);
		static const GLfloat g_vertex_buffer_data[] = { 
				 -0.5f, -0.5f, 0.0f,
				  0.5f, -0.5f, 0.0f,
				 -0.5f,  0.5f, 0.0f,
				  0.5f,  0.5f, 0.0f,
			};
			
			glGenBuffers(1, &vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

			// The VBO containing the positions and sizes of the particles
			
			glGenBuffers(1, &positionBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			// Initialize with empty (NULL) buffer : it will be updated later, each frame.
			glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

			// The VBO containing the colors of the particles
			
			glGenBuffers(1, &colorBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
			// Initialize with empty (NULL) buffer : it will be updated later, each frame.
			glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	}

	void fillData(int init = 1) {
		count = 0;
		for (int i = 0; i < layout->g->num; ++i) {
			if (layout->g->pre[i] != -1) {
				//g.particleContainer[count].pos = layout->pos[i];
				// fill buff
				positionSizeData[count*4] = layout->pos[i].x;
				positionSizeData[count*4+1] = layout->pos[i].y;
				positionSizeData[count*4+2] = layout->pos[i].z;
				//layout->pos[i].z = -10.0f;
				//// TODO sphere 3D

				positionSizeData[count*4+3] = (rand()%1000)/2000.0+0.004;//layout->g->inDegree[i];

				if (init) {
					colorData[count*4] = rand()%256;
					colorData[count*4+1] = rand()%256;
					colorData[count*4+2] = rand()%256;
					colorData[count*4+3] = (rand()%256)/3;
				}
				count++;
			}
		}
	}

	void initParticle() {
		colorData = new GLubyte[MaxParticles*4];
		positionSizeData = new GLfloat[MaxParticles*4];
		FILE* fin = fopen("test.graph", "r");
		Graph* graph = new Graph(100000);

		int u, v;
		float w;
		while (fscanf(fin, "%d%d%f", &u, &v, &w) != EOF && graph->num < 100) {
			graph->add(u, v, w);
			graph->add(v, u, w);
		}
		layout = new BaseLayout(graph);
		layout->init2D(100);
		//graph->calculateDegree();
		fillData();
	}


	void printParticle() {
		for (int i = 0; i < count; ++i) {
			fprintf(stderr, "%d-(%f,%f,%f,%f)/(%d,%d,%d,%d)\n", i,
									positionSizeData[i*4], 
									positionSizeData[i*4+1], 
									positionSizeData[i*4+2], 
									positionSizeData[i*4+3], 
									colorData[i*4],
									colorData[i*4+1],
									colorData[i*4+2],
									colorData[i*4+3]
								);
		}
	}

	void update(Control &control) {
		updateData();
		//updateBuffer(control);
		updateBufferPoint(control);
	}


	void updateData() {
		static int iter = 0;
		if (layout->iterate(iter++)) {
			//printParticle();
			layout->normalToSpace();
			fillData();
		}
	}

	void updateBufferPoint(Control &ctr) {
		// Link program
		glUseProgram(programID);

		// Use position buffer, upload data to gpu
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles*4*sizeof(GLfloat), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, count*sizeof(GLfloat)*4, positionSizeData);

		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles*4*sizeof(GLubyte), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, count*sizeof(GLubyte), colorData);

		glm::mat4 vpMatrix = ctr.ProjectionMatrix*ctr.ViewMatrix;
		glUniformMatrix4fv(viewProjectMatrixID, 1, GL_FALSE, &vpMatrix[0][0]);

		glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				4,                                // size : x + y + z + size => 4
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// 3rd attribute buffer : particles' colors
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
			glVertexAttribPointer(
				2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				4,                                // size : r + g + b + a => 4
				GL_UNSIGNED_BYTE,                 // type
				GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			glEnable(GL_PROGRAM_POINT_SIZE);
			glDrawArrays(GL_POINTS, 0, count);

	}


	void updateBuffer(Control &ctr) {


			//printf("%d ",count);


			// Update the buffers that OpenGL uses for rendering.
			// There are much more sophisticated means to stream data from the CPU to the GPU, 
			// but this is outside the scope of this tutorial.
			// http://www.opengl.org/wiki/Buffer_Object_Streaming


			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
			glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(GLfloat) * 4, positionSizeData);

			glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
			glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
			glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(GLubyte) * 4, colorData);


			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Use our shader
			glUseProgram(programID);

			// Bind our texture in texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			// Set our "mytextureSampler" sampler to user texture Unit 0
			glUniform1i(textureID, 0);



			// Same as the billboards tutorial
			glUniform3f(cameraRightWorldspaceID, ctr.ViewMatrix[0][0], ctr.ViewMatrix[1][0], ctr.ViewMatrix[2][0]);
			glUniform3f(cameraUpWorldspaceID   , ctr.ViewMatrix[0][1], ctr.ViewMatrix[1][1], ctr.ViewMatrix[2][1]);
			glm::mat4 vpMatrix = ctr.ProjectionMatrix*ctr.ViewMatrix;
			glUniformMatrix4fv(viewProjectMatrixID, 1, GL_FALSE, &vpMatrix[0][0]);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glVertexAttribPointer(
				0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			
			// 2nd attribute buffer : positions of particles' centers
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glVertexAttribPointer(
				1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				4,                                // size : x + y + z + size => 4
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// 3rd attribute buffer : particles' colors
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
			glVertexAttribPointer(
				2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
				4,                                // size : r + g + b + a => 4
				GL_UNSIGNED_BYTE,                 // type
				GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// These functions are specific to glDrawArrays*Instanced*.
			// The first parameter is the attribute buffer we're talking about.
			// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
			// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
			glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
			glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
			glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

			// Draw the particules !
			// This draws many times a small triangle_strip (which looks like a quad).
			// This is equivalent to :
			// for(i in count) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
			// but faster.
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
	}


};


#endif // __PARTICLE_H__

