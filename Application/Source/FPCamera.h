#ifndef FPCAMERA_H
#define FPCAMERA_H

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

class FPCamera
{
public:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    // Add azimuth and altitude for mouse look support
    float azimuth = 0.0f;
    float altitude = 0.0f;

    // Add yaw and pitch for compatibility with RenderFlashlight
    float yaw = 0.0f;
    float pitch = 0.0f;

    // Add front vector for compatibility with code using cam.front
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);

    FPCamera();
    ~FPCamera();

    void Init(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0,1,0));
    void Reset();
    void Update(double dt);

private:

    bool isDirty;   // indicate if there is a need to recalculate the camera attributes

    void Refresh();
};

#endif

