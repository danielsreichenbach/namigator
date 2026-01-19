#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    // Camera position in world coordinates
    glm::vec3 m_position;

    // Camera view forward direction in world coordinate system
    glm::vec3 m_forward;

    // Camera view up direction in world coordinate system
    glm::vec3 m_up;

    // Camera view right direction in world coordinate system
    glm::vec3 m_right;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projMatrix;

    void UpdateViewMatrix();

    bool m_mousePanning;
    int m_mousePanX;
    int m_mousePanY;

    float m_viewportX;
    float m_viewportY;

    float m_viewportWidth;
    float m_viewportHeight;

    float m_nearPlane;
    float m_farPlane;

public:
    Camera();

    void Move(float x, float y, float z) { Move(glm::vec3(x, y, z)); }
    void Move(const glm::vec3& position);

    void LookAt(float x, float y, float z) { LookAt(glm::vec3(x, y, z)); }
    void LookAt(const glm::vec3& target);

    void MoveUp(float delta);
    void MoveIn(float delta);
    void MoveRight(float delta);
    void MoveVertical(float delta);

    void Yaw(float delta);
    void Pitch(float delta);

    bool IsMousePanning() const { return m_mousePanning; }

    void BeginMousePan(int screenX, int screenY);
    void EndMousePan();
    void UpdateMousePan(int newX, int newY);
    void GetMousePanStart(int& x, int& y) const;

    void UpdateProjection(float width, float height);

    glm::vec3 ProjectPoint(const glm::vec3& worldPos) const;
    glm::vec3 UnprojectPoint(int screenX, int screenY, float depth) const;

    // Get ray from camera through screen coordinates
    void GetPickRay(int screenX, int screenY, glm::vec3& rayOrigin, glm::vec3& rayDir) const;

    const glm::vec3& GetPosition() const { return m_position; }
    const glm::vec3& GetForward() const { return m_forward; }
    const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
    const glm::mat4& GetProjMatrix() const { return m_projMatrix; }
};
