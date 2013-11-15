#ifndef __GRAPH_NETWORK_H__
#define __GRAPH_NETWORK_H__


#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>

#include "common/shader.hpp"
#include "common/controls.hpp"
#include "layout.h"

struct  GraphNetwork{
	GLuint programID;
	GLuint vao;
	GLuint vertexBuffer;
	GLuint viewProjectionID;
	GLfloat* vertexData;

	void init(BaseLayout *layout) {
		programID = LoadShaders("network.vert", "network.frag");
		glGetUniformLocation(programID, "viewProjection");
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*layout->g->total*3*layout->g->total, NULL, GL_STREAM_DRAW);
		vertexData = new GLfloat[layout->g->total*layout->g->total*3];
	}

	int setupData(BaseLayout *layout);
	void update(BaseLayout *layout, Control &control);

};



#endif // __GRAPH_NETWORK_H__