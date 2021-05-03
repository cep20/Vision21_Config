/**
 ******************************************************************************
 * @addtogroup AppLayer
 * @{
 * @file    AppLayer
 * @author  Samuel Martel
 * @brief   Header for the AppLayer module.
 *
 * @date 5/3/2021 9:04:28 AM
 *
 ******************************************************************************
 */
#ifndef _AppLayer
#define _AppLayer

/*****************************************************************************/
/* Includes */
#include "Brigerad.h"
#include "Brigerad/Renderer/Texture.h"

#include "Config.h"

#include <string>

/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */
using namespace Brigerad;
class AppLayer : public Brigerad::Layer
{
public:
    AppLayer();
    virtual ~AppLayer() override = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Brigerad::Timestep ts) override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(Brigerad::Event& e) override;

private:
    void NewFile();
    void OpenFile();
    void SaveFile();
    void HandleConfigRendering();
    void HandleSensorTypeSelect(const std::string& label, uint8_t& sensor);
    void HandleLineConfig(const std::string& label, uint8_t& line);

private:
    Config      m_config  = {};
    bool        m_isNew   = false;
    bool        m_isSaved = false;
    std::string m_path    = "";

    Ref<Scene>       m_scene;
    Entity           m_background;
    Entity           m_camera;
    Ref<Framebuffer> m_fb;

    glm::vec2 m_viewportSize = glm::vec2 {0.0f};
};

/*****************************************************************************/
/* Exported functions */


/* Have a wonderful day :) */
#endif /* _AppLayer */
/**
 * @}
 */
/****** END OF FILE ******/
