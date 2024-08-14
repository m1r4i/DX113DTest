#include "ObjLoader.h"

#pragma warning(disable:4996)

void loadObj(const char* filename, model* m) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("Loading... %s\n\n",filename);

    // ‰Šú‰»
    m->v.count = 0;
    m->v.vertices = NULL;
    m->n.count = 0;
    m->n.normals = NULL;
    m->f.count = 0;
    m->f.faces = NULL;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            // ’¸“_‚Ìˆ—
            m->v.count++;
            m->v.vertices = (vertex*)realloc(m->v.vertices, m->v.count * sizeof(vertex));
            sscanf(line, "v %f %f %f", &m->v.vertices[m->v.count - 1].x, &m->v.vertices[m->v.count - 1].y, &m->v.vertices[m->v.count - 1].z);
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            // –@ü‚Ìˆ—
            m->n.count++;
            m->n.normals = (normal*)realloc(m->n.normals, m->n.count * sizeof(normal));
            sscanf(line, "vn %f %f %f", &m->n.normals[m->n.count - 1].x, &m->n.normals[m->n.count - 1].y, &m->n.normals[m->n.count - 1].z);
        }
        else if (strncmp(line, "f ", 2) == 0) {
            // –Ê‚Ìˆ—
            m->f.count++;
            m->f.faces = (face*)realloc(m->f.faces, m->f.count * sizeof(face));
            face* currentFace = &m->f.faces[m->f.count - 1];
            sscanf(line, "f %d//%d %d//%d %d//%d",
                &currentFace->vertexIndices[0], &currentFace->normalIndices[0],
                &currentFace->vertexIndices[1], &currentFace->normalIndices[1],
                &currentFace->vertexIndices[2], &currentFace->normalIndices[2]);
        }
    }

    fclose(file);
}
