#include <sp2/tweak.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/widget/root.h>
#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/layout/vertical.h>

namespace sp {

template<typename T> class TweakEntry : public sp::gui::Widget
{
public:
    TweakEntry(P<Widget> parent, string name, T value, T min, T max)
    : Widget(parent), name(name)
    {
        layout.match_content_size = true;

        label = new sp::gui::Label(this);
        label->layout.size = Vector2d(128, 16);
        label->setLabel(name + ": " + string(value));

        slider = new sp::gui::Slider(this);
        slider->layout.size = Vector2d(128, 16);
        slider->layout.position = Vector2d(0, 16);
        slider->setAttribute("min", string(min));
        slider->setAttribute("max", string(max));
        slider->setValue(value);
        
        slider->setEventCallback([this](Variant value)
        {
            label->setLabel(this->name + ": " + string(T(value.getDouble())));
        });
    }

    T update(T value)
    {
        return slider->getValue();
    }
private:
    string name;
    
    sp::gui::Slider* slider;
    sp::gui::Label* label;
};

class TweakScene : public sp::gui::Scene
{
public:
    TweakScene()
    : gui::Scene(sp::Vector2d(640, 480), "TWEAK_GUI", 101)
    {
        getRootWidget()->setAttribute("layout", "vertical");
        getRootWidget()->setAttribute("stretch", "true");
    }

    static P<TweakScene> get()
    {
        if (!instance)
            instance = new TweakScene();
        return instance;
    }

    template<typename T> T update(string name, T value, T min, T max)
    {
        auto it = entries.find(name);
        if (it == entries.end())
        {
            entries[name] = new TweakEntry<T>(getRootWidget(), name, value, min, max);
        }
        else
        {
            P<TweakEntry<T>> entry = it->second;
            if (entry)
                value = entry->update(value);
        }
        return value;
    }
private:
    P<gui::VerticalLayout> vertical_layout;
    std::map<string, P<gui::Widget>> entries;

    static P<TweakScene> instance;
};

P<TweakScene> TweakScene::instance;

int tweak(string name, int value, int min, int max)
{
    return TweakScene::get()->update(name, value, min, max);
}

double tweak(string name, double value, double min, double max)
{
    return TweakScene::get()->update(name, value, min, max);
}

Color tweak(string name, Color color)
{
    return Color(
        tweak(name + ".r", color.r, 0.0, 1.0), 
        tweak(name + ".g", color.g, 0.0, 1.0), 
        tweak(name + ".b", color.b, 0.0, 1.0), 
        tweak(name + ".a", color.a, 0.0, 1.0));
}

};//namespace sp
