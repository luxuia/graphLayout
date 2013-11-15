#include "network.h"


void GraphNetwork::update(BaseLayout* layout, Control &control) {
	glUseProgram(programID);
	glm::mat4 viewProjectionMatrix = control.ProjectionMatrix*control.ViewMatrix;
	glUniformMatrix4fv(viewProjectionID, 1, GL_FALSE, &viewProjectionMatrix[0][0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, layout->g->total*layout->g->total*sizeof(GLfloat)*3, NULL, GL_STREAM_DRAW);
	int count = setupData(layout);	
	glBufferSubData(GL_ARRAY_BUFFER, 0, count*sizeof(GLfloat)*3, vertexData);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
	glDrawArrays(GL_LINES, 0, count);
}

int GraphNetwork::setupData(BaseLayout *layout) {
	int count = 0;
	for (int i = 0; i < layout->g->num; ++i) {
		if (layout->g->pre[i] != -1) {
			for (int v, e = layout->g->pre[i]; e != -1; e = layout->g->edge[e].next) {
				v = layout->g->edge[e].u;
				if (i == v) {
					continue;
				}
				vertexData[count*3] = layout->pos[i].x;
				vertexData[count*3+1] = layout->pos[i].y;
				vertexData[count*3+2] = layout->pos[i].z;
				count++;
				vertexData[count*3] = layout->pos[v].x;
				vertexData[count*3+1] = layout->pos[v].y;
				vertexData[count*3+2] = layout->pos[v].z;
				count++;
			}
		}
	}
	return count;
}


