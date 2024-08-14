#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x;
    float y;
    float z;
} vertex;

typedef struct {
    int count;
    vertex* vertices;
} vertices;

typedef struct {
    float x;
    float y;
    float z;
} normal;

typedef struct {
    int count;
    normal* normals;
} normals;

typedef struct {
    int vertexIndices[3];
    int normalIndices[3];
} face;

typedef struct {
    int count;
    face* faces;
} faces;

typedef struct {
    vertices v;
    normals n;
    faces f;
} model;

void loadObj(const char* filename, model* m);