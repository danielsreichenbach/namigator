#include "Camera.hpp"
#include "Common.hpp"
#include "DetourDebugDraw.hpp"
#include "Renderer.hpp"
#include "parser/Adt/Adt.hpp"
#include "parser/Map/Map.hpp"
#include "parser/MpqManager.hpp"
#include "pathfind/Map.hpp"
#include "recastnavigation/DebugUtils/Include/DetourDebugDraw.h"
#include "utility/Exception.hpp"
#include "utility/MathHelper.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Constants
constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 800;
constexpr float CAMERA_STEP = 2.0f;

// Global state
struct AppState
{
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<parser::Map> map;
    std::unique_ptr<pathfind::Map> navMesh;

    fs::path navDataPath;
    fs::path wowDataPath;

    bool hasStart = false;
    math::Vertex startPoint;

    // Camera movement
    int movingForward = 0;
    int movingRight = 0;
    int movingUp = 0;
    int movingVertical = 0;

    // Input state
    bool mouseRightDown = false;
    int lastMouseX = 0;
    int lastMouseY = 0;

    // Settings
    int selectedMap = 0;
    char adtX[32] = "-8925";
    char adtY[32] = "-120";
    char doodadDisplayId[32] = "0";
};

// Debug visualization helper
void DrawDebugText(const std::string& text)
{
    std::cout << text << std::endl;
}

// Navigation mesh visualization
void DrawNavMesh(AppState& state)
{
    if (!state.navMesh)
        return;

    DetourDebugDraw dd(state.renderer.get());

    // Draw normal polygons
    dd.Steep(false);
    duDebugDrawNavMesh(&dd, state.navMesh->GetNavMesh(),
                       DU_DRAWNAVMESH_OFFMESHCONS);

    // Draw steep polygons
    dd.Steep(true);
    for (int i = 0; i < state.navMesh->GetNavMesh().getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = state.navMesh->GetNavMesh().getTile(i);
        if (!tile || !tile->header)
            continue;

        dtPolyRef base = state.navMesh->GetNavMesh().getPolyRefBase(tile);
        for (int j = 0; j < tile->header->polyCount; ++j)
        {
            const dtPoly* p = &tile->polys[j];
            if (p->flags & PolyFlags::Steep)
                duDebugDrawNavMeshPoly(&dd, state.navMesh->GetNavMesh(),
                                       base | (dtPolyRef)j, 0);
        }
    }
}

// Load ADT helper
void LoadAdt(AppState& state, int x, int y)
{
    if (!state.map || !state.map->HasAdt(x, y))
        return;

    auto adt = state.map->GetAdt(x, y);

    // Load terrain, liquids, WMOs, doodads
    for (int chunkX = 0; chunkX < MeshSettings::ChunksPerAdt; ++chunkX)
    {
        for (int chunkY = 0; chunkY < MeshSettings::ChunksPerAdt; ++chunkY)
        {
            auto chunk = adt->GetChunk(chunkX, chunkY);

            state.renderer->AddTerrain(chunk->m_terrainVertices,
                                       chunk->m_terrainIndices, chunk->m_areaId);
            state.renderer->AddLiquid(chunk->m_liquidVertices,
                                      chunk->m_liquidIndices);

            // Load doodads
            for (auto& d : chunk->m_doodadInstances)
            {
                if (state.renderer->HasDoodad(d))
                    continue;

                auto doodad = state.map->GetDoodadInstance(d);
                if (!doodad)
                    continue;

                std::vector<math::Vertex> vertices;
                std::vector<int> indices;
                doodad->BuildTriangles(vertices, indices);
                state.renderer->AddDoodad(d, vertices, indices);
            }

            // Load WMOs
            for (auto& w : chunk->m_wmoInstances)
            {
                if (state.renderer->HasWmo(w))
                    continue;

                auto wmo = state.map->GetWmoInstance(w);
                if (!wmo)
                    continue;

                std::vector<math::Vertex> vertices;
                std::vector<int> indices;

                wmo->BuildTriangles(vertices, indices);
                state.renderer->AddWmo(w, vertices, indices);

                wmo->BuildLiquidTriangles(vertices, indices);
                state.renderer->AddLiquid(vertices, indices);

                if (!state.renderer->HasDoodad(w))
                {
                    wmo->BuildDoodadTriangles(vertices, indices);
                    state.renderer->AddDoodad(w, vertices, indices);
                }
            }
        }
    }

    // Load navigation mesh for this ADT
    if (state.navMesh && state.navMesh->LoadADT(adt->X, adt->Y))
    {
        DrawNavMesh(state);
    }

    // Center camera on ADT
    float cx = (adt->Bounds.MaxCorner.X + adt->Bounds.MinCorner.X) / 2.f;
    float cy = (adt->Bounds.MaxCorner.Y + adt->Bounds.MinCorner.Y) / 2.f;
    float cz = (adt->Bounds.MaxCorner.Z + adt->Bounds.MinCorner.Z) / 2.f;

    state.camera->Move(cx + 300.f, cy + 300.f, cz + 300.f);
    state.camera->LookAt(cx, cy, cz);
}

// Map selection
const std::vector<std::pair<int, std::string>> availableMaps = {
    {0, "Azeroth"},           {1, "Kalimdor"},
    {13, "Test"},             {30, "Alterac Valley"},
    {33, "Shadowfang Keep"},  {34, "Stormwind Stockades"},
    {43, "Wailing Caverns"},  {90, "Gnomeregan"},
    {229, "Blackrock Spire"}, {429, "Dire Maul"},
    {489, "Warsong Gulch"},   {529, "Arathi Basin"},
    {530, "Outland"},         {562, "Blade's Edge Arena"},
    {571, "Northrend"},       {603, "Ulduar"},
};

void ChangeMap(AppState& state, int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= availableMaps.size())
        return;

    state.hasStart = false;
    state.renderer->ClearBuffers();

    auto [mapId, mapName] = availableMaps[mapIndex];

    try
    {
        state.map = std::make_unique<parser::Map>(mapName);
        state.navMesh =
            std::make_unique<pathfind::Map>(state.navDataPath.string(), mapName);

        DrawDebugText("Loaded map: " + mapName);
    }
    catch (const utility::exception& e)
    {
        std::cerr << "Error loading map: " << e.what() << std::endl;
    }
}

// Handle mouse click for pathfinding
void HandleMouseClick(AppState& state, int x, int y, bool shiftHeld)
{
    if (!state.renderer || !state.camera)
        return;

    std::uint32_t param = 0;
    math::Vertex hit;

    if (shiftHeld)
    {
        // Info mode: show terrain information
        if (state.renderer->HitTest(*state.camera, x, y,
                                    Renderer::CollidableGeometryFlag, hit, param))
        {
            std::stringstream ss;
            ss << "Hit terrain at (" << hit.X << ", " << hit.Y << ", " << hit.Z
               << ")\n";

            int adtX, adtY, chunkX, chunkY;
            math::Convert::WorldToAdt(hit, adtX, adtY, chunkX, chunkY);
            ss << "ADT: (" << adtX << ", " << adtY << ") Chunk: (" << chunkX << ", "
               << chunkY << ")\n";

            if (state.navMesh)
            {
                unsigned int zone, area;
                if (state.navMesh->ZoneAndArea(hit, zone, area))
                    ss << "Zone: " << zone << " Area: " << area << "\n";

                std::vector<float> heights;
                if (state.navMesh->FindHeights(hit.X, hit.Y, heights))
                {
                    ss << "Found " << heights.size() << " height values:\n";
                    for (auto h : heights)
                        ss << "  " << h << "\n";
                }
            }

            DrawDebugText(ss.str());
        }
    }
    else
    {
        // Pathfinding mode
        if (state.renderer->HitTest(*state.camera, x, y,
                                    Renderer::NavMeshGeometryFlag, hit, param))
        {
            state.renderer->ClearSprites();

            if (state.hasStart && state.navMesh)
            {
                // Find path from start to hit
                std::vector<math::Vertex> path;
                if (state.navMesh->FindPath(state.startPoint, hit, path, false))
                {
                    state.renderer->AddPath(path);
                    DrawDebugText("Path found with " + std::to_string(path.size()) +
                                  " waypoints");
                }
                else
                {
                    DrawDebugText("Failed to find path");
                }

                state.hasStart = false;
            }
            else
            {
                // Set start point
                state.hasStart = true;
                state.startPoint = hit;
                state.renderer->AddSphere(hit, 3.f, 2);
                DrawDebugText("Start point set");
            }
        }
    }
}

// Render GUI
void RenderGUI(AppState& state)
{
    ImGui::Begin("Navigation Mesh Viewer", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize);

    // Map selection
    if (ImGui::BeginCombo("Map", availableMaps[state.selectedMap].second.c_str()))
    {
        for (int i = 0; i < availableMaps.size(); ++i)
        {
            bool isSelected = (state.selectedMap == i);
            if (ImGui::Selectable(availableMaps[i].second.c_str(), isSelected))
            {
                state.selectedMap = i;
                ChangeMap(state, i);
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    // ADT coordinates
    ImGui::InputText("X", state.adtX, sizeof(state.adtX));
    ImGui::InputText("Y", state.adtY, sizeof(state.adtY));

    if (ImGui::Button("Load ADT"))
    {
        try
        {
            float x = std::stof(state.adtX);
            float y = std::stof(state.adtY);

            int adtX, adtY;
            math::Convert::WorldToAdt({x, y, 0.f}, adtX, adtY);

            if (state.map && state.map->HasAdt(adtX, adtY))
            {
                LoadAdt(state, adtX, adtY);
            }
            else
            {
                DrawDebugText("ADT not found");
            }
        }
        catch (...)
        {
            DrawDebugText("Invalid coordinates");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Z Search"))
    {
        try
        {
            float x = std::stof(state.adtX);
            float y = std::stof(state.adtY);

            if (state.navMesh)
            {
                std::vector<float> heights;
                if (state.navMesh->FindHeights(x, y, heights))
                {
                    state.renderer->ClearSprites();
                    std::stringstream ss;
                    ss << "Heights at (" << x << ", " << y << "):\n";
                    for (auto h : heights)
                    {
                        ss << "  " << h << "\n";
                        state.renderer->AddSphere({x, y, h}, 0.75f, 1);
                    }
                    DrawDebugText(ss.str());
                }
            }
        }
        catch (...)
        {
            DrawDebugText("Invalid coordinates");
        }
    }

    ImGui::Separator();

    // Rendering options
    static bool wireframe = false;
    static bool renderADT = true;
    static bool renderLiquid = true;
    static bool renderWMO = true;
    static bool renderDoodad = true;
    static bool renderMesh = true;

    if (ImGui::Checkbox("Wireframe", &wireframe))
        state.renderer->SetWireframe(wireframe);
    if (ImGui::Checkbox("Render ADT", &renderADT))
        state.renderer->SetRenderADT(renderADT);
    if (ImGui::Checkbox("Render Liquid", &renderLiquid))
        state.renderer->SetRenderLiquid(renderLiquid);
    if (ImGui::Checkbox("Render WMO", &renderWMO))
        state.renderer->SetRenderWMO(renderWMO);
    if (ImGui::Checkbox("Render Doodad", &renderDoodad))
        state.renderer->SetRenderDoodad(renderDoodad);
    if (ImGui::Checkbox("Render Mesh", &renderMesh))
        state.renderer->SetRenderMesh(renderMesh);

    ImGui::Separator();

    // Instructions
    ImGui::Text("Controls:");
    ImGui::BulletText("WASD: Move camera");
    ImGui::BulletText("Q/E: Move up/down");
    ImGui::BulletText("Space/X: Move vertically");
    ImGui::BulletText("Right-click drag: Look around");
    ImGui::BulletText("Mouse wheel: Zoom");
    ImGui::BulletText("Left-click: Set start/end for pathfinding");
    ImGui::BulletText("Shift+click: Show terrain info");

    ImGui::End();
}

// Main function
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <nav_data_path> <wow_data_path>"
                  << std::endl;
        return 1;
    }

    AppState state;
    state.navDataPath = argv[1];
    state.wowDataPath = argv[2];

    if (!fs::is_directory(state.navDataPath))
    {
        std::cerr << "Navigation data directory not found: " << state.navDataPath
                  << std::endl;
        return 1;
    }

    if (!fs::is_directory(state.wowDataPath))
    {
        std::cerr << "WoW data directory not found: " << state.wowDataPath
                  << std::endl;
        return 1;
    }

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "namigator Navigation Mesh Viewer - SDL3", WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Initialize parser
    parser::sMpqManager.Initialize(state.wowDataPath.string());

    // Initialize renderer and camera
    state.renderer = std::make_unique<Renderer>();
    state.renderer->Initialize();

    state.camera = std::make_unique<Camera>();
    state.camera->UpdateProjection(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running)
    {
        // Handle events
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                state.camera->UpdateProjection(w, h);
            }
            else if (event.type == SDL_EVENT_KEY_DOWN && !io.WantCaptureKeyboard)
            {
                switch (event.key.key)
                {
                    case SDLK_W:
                        state.movingForward = 1;
                        break;
                    case SDLK_S:
                        state.movingForward = -1;
                        break;
                    case SDLK_A:
                        state.movingRight = -1;
                        break;
                    case SDLK_D:
                        state.movingRight = 1;
                        break;
                    case SDLK_Q:
                        state.movingUp = 1;
                        break;
                    case SDLK_E:
                        state.movingUp = -1;
                        break;
                    case SDLK_SPACE:
                        state.movingVertical = 1;
                        break;
                    case SDLK_X:
                        state.movingVertical = -1;
                        break;
                }
            }
            else if (event.type == SDL_EVENT_KEY_UP && !io.WantCaptureKeyboard)
            {
                switch (event.key.key)
                {
                    case SDLK_W:
                    case SDLK_S:
                        state.movingForward = 0;
                        break;
                    case SDLK_A:
                    case SDLK_D:
                        state.movingRight = 0;
                        break;
                    case SDLK_Q:
                    case SDLK_E:
                        state.movingUp = 0;
                        break;
                    case SDLK_SPACE:
                    case SDLK_X:
                        state.movingVertical = 0;
                        break;
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                     !io.WantCaptureMouse)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    bool shiftHeld =
                        (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;
                    HandleMouseClick(state, event.button.x, event.button.y,
                                     shiftHeld);
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    state.mouseRightDown = true;
                    state.camera->BeginMousePan(event.button.x, event.button.y);
                    SDL_SetWindowRelativeMouseMode(window, true);
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    state.mouseRightDown = false;
                    state.camera->EndMousePan();
                    SDL_SetWindowRelativeMouseMode(window, false);
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION &&
                     !io.WantCaptureMouse)
            {
                if (state.mouseRightDown)
                {
                    state.camera->UpdateMousePan(event.motion.x, event.motion.y);
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL && !io.WantCaptureMouse)
            {
                state.camera->MoveIn(event.wheel.y * 10.0f);
            }
        }

        // Update camera movement
        if (state.movingForward)
            state.camera->MoveIn(CAMERA_STEP * state.movingForward);
        if (state.movingRight)
            state.camera->MoveRight(CAMERA_STEP * state.movingRight);
        if (state.movingUp)
            state.camera->MoveUp(CAMERA_STEP * state.movingUp);
        if (state.movingVertical)
            state.camera->MoveVertical(CAMERA_STEP * state.movingVertical);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Render GUI
        RenderGUI(state);

        // Render scene
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        state.renderer->Render(*state.camera, w, h);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    state.renderer->Cleanup();

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
