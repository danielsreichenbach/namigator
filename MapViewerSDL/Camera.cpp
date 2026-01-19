#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

Camera::Camera()
    : m_position(0.f, 0.f, 0.f), m_forward(0.f, 0.f, -1.f), m_up(0.f, 1.f, 0.f),
      m_right(1.f, 0.f, 0.f), m_mousePanning(false), m_mousePanX(0),
      m_mousePanY(0), m_viewportX(0.f), m_viewportY(0.f), m_viewportWidth(800.f),
      m_viewportHeight(600.f), m_nearPlane(0.1f), m_farPlane(100000.f)
{
    UpdateViewMatrix();
    UpdateProjection(m_viewportWidth, m_viewportHeight);
}

void Camera::UpdateViewMatrix()
{
    m_viewMatrix = glm::lookAt(m_position, m_position + m_forward, m_up);
}

void Camera::Move(const glm::vec3& position)
{
    m_position = position;
    UpdateViewMatrix();
}

void Camera::LookAt(const glm::vec3& target)
{
    m_forward = glm::normalize(target - m_position);
    m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.f, 0.f, 1.f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
    UpdateViewMatrix();
}

void Camera::MoveUp(float delta)
{
    // Move in world Z direction
    m_position.z += delta;
    UpdateViewMatrix();
}

void Camera::MoveIn(float delta)
{
    // Move along forward vector
    m_position += m_forward * delta;
    UpdateViewMatrix();
}

void Camera::MoveRight(float delta)
{
    // Move along right vector
    m_position += m_right * delta;
    UpdateViewMatrix();
}

void Camera::MoveVertical(float delta)
{
    // Move along up vector
    m_position += m_up * delta;
    UpdateViewMatrix();
}

void Camera::Yaw(float delta)
{
    // Rotate around Z axis (WoW coordinate system)
    glm::vec3 zAxis(0.f, 0.f, 1.f);
    m_forward = glm::rotate(m_forward, delta, zAxis);
    m_right = glm::rotate(m_right, delta, zAxis);
    m_up = glm::rotate(m_up, delta, zAxis);
    UpdateViewMatrix();
}

void Camera::Pitch(float delta)
{
    // Rotate around right vector
    m_forward = glm::rotate(m_forward, delta, m_right);
    m_up = glm::rotate(m_up, delta, m_right);
    UpdateViewMatrix();
}

void Camera::BeginMousePan(int screenX, int screenY)
{
    m_mousePanning = true;
    m_mousePanX = screenX;
    m_mousePanY = screenY;
}

void Camera::EndMousePan()
{
    m_mousePanning = false;
}

void Camera::UpdateMousePan(int newX, int newY)
{
    if (!m_mousePanning)
        return;

    float deltaX = static_cast<float>(newX - m_mousePanX);
    float deltaY = static_cast<float>(newY - m_mousePanY);

    // Sensitivity factors
    float yawSensitivity = 0.005f;
    float pitchSensitivity = 0.005f;

    Yaw(-deltaX * yawSensitivity);
    Pitch(-deltaY * pitchSensitivity);

    m_mousePanX = newX;
    m_mousePanY = newY;
}

void Camera::GetMousePanStart(int& x, int& y) const
{
    x = m_mousePanX;
    y = m_mousePanY;
}

void Camera::UpdateProjection(float width, float height)
{
    m_viewportWidth = width;
    m_viewportHeight = height;

    float aspect = width / height;
    float fov = glm::radians(45.0f);
    m_projMatrix = glm::perspective(fov, aspect, m_nearPlane, m_farPlane);
}

glm::vec3 Camera::ProjectPoint(const glm::vec3& worldPos) const
{
    glm::vec4 clipPos = m_projMatrix * m_viewMatrix * glm::vec4(worldPos, 1.0f);

    if (clipPos.w == 0.0f)
        return glm::vec3(0.0f);

    // Perspective divide
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

    // Convert to screen coordinates
    glm::vec3 screenPos;
    screenPos.x = (ndcPos.x + 1.0f) * 0.5f * m_viewportWidth + m_viewportX;
    screenPos.y = (1.0f - ndcPos.y) * 0.5f * m_viewportHeight + m_viewportY;
    screenPos.z = (ndcPos.z + 1.0f) * 0.5f;

    return screenPos;
}

glm::vec3 Camera::UnprojectPoint(int screenX, int screenY, float depth) const
{
    // Convert screen coordinates to NDC
    glm::vec3 ndcPos;
    ndcPos.x = (screenX - m_viewportX) / m_viewportWidth * 2.0f - 1.0f;
    ndcPos.y = 1.0f - (screenY - m_viewportY) / m_viewportHeight * 2.0f;
    ndcPos.z = depth * 2.0f - 1.0f;

    // Create clip coordinates
    glm::vec4 clipPos(ndcPos, 1.0f);

    // Transform to view space
    glm::mat4 invProj = glm::inverse(m_projMatrix);
    glm::vec4 viewPos = invProj * clipPos;

    // Transform to world space
    glm::mat4 invView = glm::inverse(m_viewMatrix);
    glm::vec4 worldPos = invView * viewPos;

    return glm::vec3(worldPos) / worldPos.w;
}

void Camera::GetPickRay(int screenX, int screenY, glm::vec3& rayOrigin, glm::vec3& rayDir) const
{
    rayOrigin = m_position;

    // Get point at near plane
    glm::vec3 nearPoint = UnprojectPoint(screenX, screenY, 0.0f);

    // Ray direction from camera to near point
    rayDir = glm::normalize(nearPoint - rayOrigin);
}
