#include "Base.h"
#include "Control.h"
#include "FlowLayout.h"
#include "Container.h"

namespace gameplay
{

static FlowLayout* __instance;

FlowLayout::FlowLayout()
{
}

FlowLayout::FlowLayout(const FlowLayout& copy)
{
}

FlowLayout::~FlowLayout()
{
}

FlowLayout* FlowLayout::create()
{
    if (!__instance)
    {
        __instance = new FlowLayout();
    }
    else
    {
        __instance->addRef();
    }

    return __instance;
}

Layout::Type FlowLayout::getType()
{
    return Layout::LAYOUT_FLOW;
}

void FlowLayout::update(const Container* container)
{
    const Rectangle& containerBounds = container->getClipBounds();
    const Theme::Border& containerBorder = container->getBorder(container->getState());
    const Theme::Padding& containerPadding = container->getPadding();

    float clipWidth = containerBounds.width - containerBorder.left - containerBorder.right - containerPadding.left - containerPadding.right;
    float clipHeight = containerBounds.height - containerBorder.top - containerBorder.bottom - containerPadding.top - containerPadding.bottom;

    float xPosition = 0;
    float yPosition = 0;
    float rowY = 0;
    float tallestHeight = 0;

    std::vector<Control*> controls = container->getControls();
    unsigned int controlsCount = controls.size();
    for (unsigned int i = 0; i < controlsCount; i++)
    {
        Control* control = controls.at(i);

        const Rectangle& bounds = control->getBounds();
        const Theme::Margin& margin = control->getMargin();

        xPosition += margin.left;

        // Wrap to next row if we've gone past the edge of the container.
        if (xPosition + bounds.width >= clipWidth)
        {
            xPosition = margin.left;
            rowY += tallestHeight;
        }

        yPosition = rowY + margin.top;

        control->setPosition(xPosition, yPosition);
        if (control->isDirty() || control->isContainer())
        {
            control->update(container->getClip());
        }

        xPosition += bounds.width + margin.right;

        float height = bounds.height + margin.top + margin.bottom;
        if (height > tallestHeight)
        {
            tallestHeight = height;
        }
    }
}

}