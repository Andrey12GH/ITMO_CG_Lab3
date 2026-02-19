#include <vector>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "own_gl.h"
#include "camera.h"  

Model* model = NULL;
const int width = 800;
const int height = 800;

Vec3f light_dir(1, 1, 1);
Camera camera(
    Vec3f(0, 0, 3),
    Vec3f(0, 0, 0),
    Vec3f(0, 1, 0)
);

struct Cube {
    std::vector<Vec3f> vertices;
    std::vector<std::vector<int>> faces;

    Cube() {
        vertices = {
            Vec3f(-0.5, -0.5, -0.5),
            Vec3f(0.5, -0.5, -0.5),
            Vec3f(0.5, 0.5, -0.5),
            Vec3f(-0.5, 0.5, -0.5),
            Vec3f(-0.5, -0.5, 0.5),
            Vec3f(0.5, -0.5, 0.5),
            Vec3f(0.5, 0.5, 0.5),
            Vec3f(-0.5, 0.5, 0.5)
        };

        faces = {
            {0, 1, 2}, {0, 2, 3},
            {1, 5, 6}, {1, 6, 2},
            {5, 4, 7}, {5, 7, 6},
            {4, 0, 3}, {4, 3, 7},
            {3, 2, 6}, {3, 6, 7},
            {4, 5, 1}, {4, 1, 0}
        };
    }
};

struct PhongShader : public IShader {
    Camera* camera;
    bool isCube = false;
    float alpha = 1.0f;

    Vec3f varying_norm[3];
    Vec3f varying_pos[3];
    Vec2f varying_uv[3];  

    PhongShader(Camera* cam, bool cube = false, float a = 1.0f)
        : camera(cam), isCube(cube), alpha(a) {
    }

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec3f v;
        Vec3f normal;

        if (isCube) {
            static Cube cube;
            int idx = cube.faces[iface][nthvert];
            v = cube.vertices[idx];

            int face_group = iface / 2;
            switch (face_group) {
            case 0: normal = Vec3f(0, 0, -1); break;
            case 1: normal = Vec3f(1, 0, 0); break;
            case 2: normal = Vec3f(0, 0, 1); break;
            case 3: normal = Vec3f(-1, 0, 0); break;
            case 4: normal = Vec3f(0, 1, 0); break;
            case 5: normal = Vec3f(0, -1, 0); break;
            }
        }
        else {
            v = model->vert(iface, nthvert);
            normal = model->normal(iface, nthvert).normalize();
            varying_uv[nthvert] = model->uv(iface, nthvert);
        }

        varying_pos[nthvert] = v;
        varying_norm[nthvert] = normal;

        Vec4f gl_Vertex = embed<4>(v);
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        Vec3f n = (varying_norm[0] * bar.x + varying_norm[1] * bar.y + varying_norm[2] * bar.z).normalize();
        Vec3f pos = varying_pos[0] * bar.x + varying_pos[1] * bar.y + varying_pos[2] * bar.z;

        Vec3f view_dir = camera->viewDir(pos);
        Vec3f light = light_dir.normalize();

        float ambient = 0.1f;
        float diffuse = std::max(0.f, n * light);
        Vec3f reflect_dir = (n * 2.f * (n * light) - light).normalize();
        float specular = std::pow(std::max(0.f, reflect_dir * view_dir), 32.f);

        float intensity = ambient + 0.8f * diffuse + 0.5f * specular;
        intensity = std::min(1.f, std::max(0.f, intensity));

        if (isCube) {
            unsigned char alpha_val = static_cast<unsigned char>(alpha * 255);
            unsigned char r = static_cast<unsigned char>(100 * intensity);
            unsigned char g = static_cast<unsigned char>(150 * intensity);
            unsigned char b = static_cast<unsigned char>(255 * intensity);

            color = TGAColor(r, g, b, alpha_val);
        }
        else {
            Vec2f uv = varying_uv[0] * bar.x + varying_uv[1] * bar.y + varying_uv[2] * bar.z;
            
            TGAColor tex_color = model->diffuse(uv);
            
            unsigned char r = static_cast<unsigned char>(tex_color[2] * intensity);  
            unsigned char g = static_cast<unsigned char>(tex_color[1] * intensity); 
            unsigned char b = static_cast<unsigned char>(tex_color[0] * intensity);  
            
            color = TGAColor(r, g, b, 255);
        }

        return false;
    }
};

int main(int argc, char** argv) {
    if (argc == 2) model = new Model(argv[1]);
    else model = new Model("obj/african_head.obj");

    camera.apply();
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (camera.eye - camera.center).norm());

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    PhongShader headShader(&camera, false, 1.0f);

    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = headShader.vertex(i, j);
        }
        triangle(screen_coords, headShader, image, zbuffer);
    }
    
    float minX = 1e9, maxX = -1e9;
    float minY = 1e9, maxY = -1e9;
    float minZ = 1e9, maxZ = -1e9;

    for (int i = 0; i < model->nverts(); i++) {
        Vec3f v = model->vert(i);
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
        minZ = std::min(minZ, v.z); maxZ = std::max(maxZ, v.z);
    }

    float centerX = (minX + maxX) / 2;
    float centerY = (minY + maxY) / 2;
    float centerZ = (minZ + maxZ) / 2;

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;

    Matrix oldModelView = ModelView;

    float scaleFactor = 1.2f;
    Matrix scaleM = Matrix::identity();
    scaleM[0][0] = sizeX * scaleFactor;
    scaleM[1][1] = sizeY * scaleFactor;
    scaleM[2][2] = sizeZ * scaleFactor;

    Matrix translateM = Matrix::identity();
    translateM[0][3] = centerX;
    translateM[1][3] = centerY;
    translateM[2][3] = centerZ;

    ModelView = ModelView * translateM * scaleM;

    PhongShader cubeShader(&camera, true, 0.3f);

    Cube cube;
    for (size_t i = 0; i < cube.faces.size(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = cubeShader.vertex(i, j);
        }
        triangle(screen_coords, cubeShader, image, zbuffer);
    }

    ModelView = oldModelView;

    image.flip_vertically();
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    return 0;
}