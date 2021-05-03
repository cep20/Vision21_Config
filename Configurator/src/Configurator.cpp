#include "Brigerad.h"
#include "Brigerad/Core/EntryPoint.h"

#include "AppLayer.h"


class Configurator : public Brigerad::Application
{
public:
    Configurator() : Brigerad::Application("Vision21 Configurator") { PushLayer(new AppLayer()); }

    ~Configurator() override = default;
};

Brigerad::Application* Brigerad::CreateApplication()
{
    return new Configurator();
}
