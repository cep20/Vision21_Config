#include "AppLayer.h"

#include "Brigerad/Renderer/Renderer2D.h"
#include "File.h"

#include "ImGui/imgui.h"

#include <algorithm>
#include <fstream>

AppLayer::AppLayer()
{
}


void AppLayer::OnAttach()
{
    FramebufferSpecification spec;
    spec.width  = Application::Get().GetWindow().GetWidth();
    spec.height = Application::Get().GetWindow().GetHeight();
    m_fb        = Framebuffer::Create(spec);

    m_scene      = CreateRef<Scene>();
    m_background = m_scene->CreateEntity("bg");
    m_background.AddComponent<TextureRendererComponent>("assets/textures/background.png");
    m_background.GetComponentRef<TransformComponent>().scale = glm::vec3 {7.5f, 7.5f, 1.0f};

    m_camera = m_scene->CreateEntity("cam");
    m_camera.AddComponent<CameraComponent>();
}


void AppLayer::OnDetach()
{
    Brigerad::Renderer2D::Shutdown();
}


void AppLayer::OnUpdate(Brigerad::Timestep ts)
{
    BR_PROFILE_FUNCTION();

    // Resize.
    FramebufferSpecification spec = m_fb->GetSpecification();
    if (m_viewportSize.x > 0.0f &&
        m_viewportSize.y > 0.0f &&    // zero sized framebuffer is invalid.
        (spec.width != m_viewportSize.x || spec.height != m_viewportSize.y))
    {
        m_fb->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);

        m_scene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
    }

    // Render.
    Renderer2D::ResetStats();
    m_fb->Bind();
    RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    RenderCommand::Clear();

    // Update scene.
    m_scene->OnUpdate(ts);

    m_fb->Unbind();

    // Handle keybinds.
    if (Input::IsKeyPressed(KeyCode::LeftControl))
    {
        if (Input::IsKeyPressed(KeyCode::N))
        {
            NewFile();
        }
        if (Input::IsKeyPressed(KeyCode::S))
        {
            SaveFile();
        }
        if (Input::IsKeyPressed(KeyCode::O))
        {
            OpenFile();
        }
    }
}


void AppLayer::OnImGuiRender()
{
    // Background
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {0.0f, 0.0f});
    ImGui::Begin("bg", nullptr, windowFlags);
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    if (m_viewportSize != *((glm::vec2*)&viewportPanelSize))
    {
        m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};
    }
    uint32_t textureId = m_fb->GetColorAttachmentRenderID();
    // ImGui takes in a void* for its images.
    ImGui::Image((void*)(uint64_t)textureId,
                 ImVec2(m_viewportSize.x, m_viewportSize.y),
                 ImVec2(0.0f, 0.0f),
                 ImVec2(1.0f, 1.0f));
    ImGui::PopStyleVar(3);

    ImGui::End();


    // Make the window full screen.
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Configurator", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Fichier"))
        {
            if (ImGui::MenuItem("Nouveau", "Ctrl+N"))
            {
                NewFile();
            }
            if (ImGui::MenuItem("Ouvrir", "Ctrl+O"))
            {
                OpenFile();
            }
            if (ImGui::MenuItem("Sauvegarder", "Ctrl+S"))
            {
                SaveFile();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quitter", "Alt+F4"))
            {
                Brigerad::Application::Get().Close();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    HandleConfigRendering();

    ImGui::End();
}


void AppLayer::OnEvent(Brigerad::Event& e)
{
    m_scene->OnEvent(e);
}


void AppLayer::NewFile()
{
    m_config  = {};
    m_isNew   = true;
    m_isSaved = false;
    m_path    = "";
}

void AppLayer::OpenFile()
{
    std::string path = File::OpenFile(FileTypes::Ini, FileMode::Open);

    if (path.empty() == true)
    {
        // Path is empty, user did not pick a file.
        return;
    }

    // Open the file, set the cursor at EOF so we can get the size of it.
    std::ifstream file = std::ifstream(path, std::ios::binary | std::ios::ate);
    if (file.is_open() == false)
    {
        // Unable to open file.
        BR_ERROR("Unable to open '{}'.", path.c_str());
        return;
    }

    // File is valid.
    m_path    = path;
    m_isNew   = false;
    m_isSaved = false;

    // Get the size of the file. (Size = position of EOF)
    size_t s = file.tellg();

    // Construct a string of the same size.
    std::string content = std::string(s, '\0');
    // Return the pointer back to the beginning of the file.
    file.seekg(0);

    // Read the content of the file.
    if (file.read(&content[0], s))
    {
        m_config = Config(reinterpret_cast<const uint8_t*>(content.c_str()), s);
    }
    else
    {
        BR_ERROR("Unable to read the content of '{}'", path.c_str());
    }
    file.close();
}


void AppLayer::SaveFile()
{
    std::string path = File::OpenFile(FileTypes::Ini, FileMode::Save);

    if (path.empty() == true)
    {
        // Path is empty, user did not pick a file.
        return;
    }
    std::ofstream file = std::ofstream(path, std::ios::binary);
    if (file.is_open() == false)
    {
        // Unable to open file.
        BR_ERROR("Unable to open '{}'.", path.c_str());
        return;
    }

    // File is valid.
    m_path    = path;
    m_isNew   = false;
    m_isSaved = true;

    // Serialize the configuration.
    uint8_t* data = nullptr;
    size_t   len  = m_config.Serialize(&data);

    file.write(reinterpret_cast<char*>(data), len);

    file.close();
    delete[] data;
}


void AppLayer::HandleConfigRendering()
{
    static const char* truckTypes[] = {"Semi-Remorque", "Train", "10 Roues", "12 Roues", "Cube"};
    static uint8_t     samples[]    = {1, 2, 4, 8, 16, 32};
    static const char* units[]      = {"Metrique", "Imperial"};
    static const char* languages[]  = {"Francais", "Anglais"};

    int sn = m_config.sn;
    ImGui::TextUnformatted("Numero de serie");
    ImGui::SameLine();
    ImGui::InputInt("##Numero de serie", &sn);
    m_config.sn = std::max(sn, 0);    // Don't allow negative serial numbers.

    char*  buf    = m_config.btName.data();
    size_t bufLen = m_config.btName.capacity();
    ImGui::TextUnformatted("Nom");
    ImGui::SameLine();
    if (ImGui::InputText("##Nom", buf, bufLen))
    {
        m_config.btName = m_config.btName.c_str();
    }

    ImGui::TextUnformatted("Type de camion");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##Type de camion", truckTypes[m_config.truckType]))
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            if (ImGui::Selectable(truckTypes[i]))
            {
                m_config.truckType = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::TextUnformatted("Date de derniere calibration");
    ImGui::SameLine();
    ImGui::Text("%s", m_config.lastCalibDate.c_str());

    ImGui::TextUnformatted("Option LoRa");
    ImGui::SameLine();
    ImGui::Checkbox("##LoRa", &m_config.isLoraEn);

    ImGui::TextUnformatted("Loggeur");
    ImGui::SameLine();
    ImGui::Checkbox("##Log", &m_config.isLogToFileEn);

    ImGui::TextUnformatted("Option Afficheur");
    ImGui::SameLine();
    ImGui::Checkbox("##Display", &m_config.isDisplayEn);

    ImGui::TextUnformatted("Option Isaac");
    ImGui::SameLine();
    ImGui::Checkbox("##isaac", &m_config.isIsaacEn);

    ImGui::TextUnformatted("Option Relais");
    ImGui::SameLine();
    ImGui::Checkbox("##relay", &m_config.isRelayOutEn);

    ImGui::TextUnformatted("Option Benne");
    ImGui::SameLine();
    ImGui::Checkbox("##dutyBox", &m_config.isDutyBoxEn);

    ImGui::TextUnformatted("Limite de Vitesse");
    ImGui::SameLine();
    int speed = m_config.dutyBoxSpeedLimit;
    if (ImGui::InputInt("##speedDutyBox", &speed))
    {
        m_config.dutyBoxSpeedLimit = std::clamp(speed, 1, 50);
    }

    HandleSensorTypeSelect("Capteur 1", m_config.sensor1Type);
    HandleSensorTypeSelect("Capteur 2", m_config.sensor2Type);
    HandleSensorTypeSelect("Capteur 3", m_config.sensor3Type);
    HandleSensorTypeSelect("Capteur 4", m_config.sensor4Type);

    HandleLineConfig("Ligne A", m_config.lineA);
    HandleLineConfig("Ligne B", m_config.lineB);
    HandleLineConfig("Ligne C", m_config.lineC);

    ImGui::TextUnformatted("Nombre d'echantillons");
    ImGui::SameLine();
    std::string l = std::to_string(m_config.samplesToTake);
    if (ImGui::BeginCombo("##samples", l.c_str()))
    {
        for (unsigned char sample : samples)
        {
            l = std::to_string(sample);
            if (ImGui::Selectable(l.c_str()))
            {
                m_config.samplesToTake = sample;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::TextUnformatted("Unites");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##units", units[m_config.unitType]))
    {
        for (uint8_t i = 0; i < 2; i++)
        {
            if (ImGui::Selectable(units[i]))
            {
                m_config.unitType = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::TextUnformatted("Langue");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##language", languages[m_config.language]))
    {
        for (uint8_t i = 0; i < 2; i++)
        {
            if (ImGui::Selectable(languages[i]))
            {
                m_config.language = i;
            }
        }
        ImGui::EndCombo();
    }
}

void AppLayer::HandleSensorTypeSelect(const std::string& label, uint8_t& sensor)
{
    static const char* sensorTypes[] = {"N/A", "Pneumatique", "Mecanique", "Inclinometre"};
    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();
    if (ImGui::BeginCombo(std::string("##" + label).c_str(), sensorTypes[sensor]))
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            if (ImGui::Selectable(sensorTypes[i]))
            {
                sensor = i;
            }
        }
        ImGui::EndCombo();
    }
}

void AppLayer::HandleLineConfig(const std::string& label, uint8_t& line)
{
    // A "line" specifies which sensors should be used to use to compute the measured weight.
    // A line can be one sensor, a combinations of two, or none.
    static const char* sensors[] = {"", "c1", "c2", "c3", "c4"};
    std::string        imguiId1  = "##" + label + "1";
    std::string        imguiId2  = "##" + label + "2";
    uint8_t            c1        = std::clamp(((line >> 4) & 0x0F), 0, 4);
    uint8_t            c2        = std::clamp((line & 0x0F), 0, 4);

    ImGui::Columns(2, nullptr, false);
    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine();
    if (ImGui::BeginCombo(imguiId1.c_str(), sensors[c1]))
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            if (ImGui::Selectable(sensors[i]))
            {
                c1 = i;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::NextColumn();

    if (c1 != 0)
    {
        ImGui::TextUnformatted("+");
        ImGui::SameLine();
        if (ImGui::BeginCombo(imguiId2.c_str(), sensors[c2]))
        {
            for (uint8_t i = 0; i < 5; i++)
            {
                if (ImGui::Selectable(sensors[i]))
                {
                    c2 = i;
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Columns(1);

    line = (c1 << 4) | c2;
}
