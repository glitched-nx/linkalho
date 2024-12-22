#include "styles/visual_overrides.hpp"
#include <stdio.h>

brls::Style* VisualOverrides::LinkalhoStyle()
{
    brls::Style* style = new brls::HorizonStyle();
    style->AppletFrame.titleSize = 30;
    style->AppletFrame.titleStart = 35;
    style->List.marginLeftRight = 40;
    style->List.marginTopBottom = 38;
    style->Dialog.height = 380;
    style->Dialog.paddingLeftRight = 56;
    return style;
}

brls::LibraryViewsThemeVariantsWrapper* VisualOverrides::LinkalhoTheme()
{
    brls::Theme* custom_light = new brls::HorizonLightTheme();
    custom_light->buttonPrimaryDisabledBackgroundColor = nvgRGB(12, 12, 60);
    custom_light->buttonPrimaryDisabledTextColor = nvgRGB(30, 30, 90);
    custom_light->buttonPrimaryEnabledBackgroundColor = nvgRGB(139, 17, 163);
    custom_light->buttonPrimaryEnabledTextColor = nvgRGB(251, 186, 43);

    brls::Theme* custom_dark = new brls::HorizonDarkTheme();
    custom_dark->buttonPrimaryDisabledBackgroundColor = nvgRGB(12, 12, 60);
    custom_dark->buttonPrimaryDisabledTextColor = nvgRGB(35, 35, 90);
    custom_dark->buttonPrimaryEnabledBackgroundColor = nvgRGB(139, 17, 163);
    custom_dark->buttonPrimaryEnabledTextColor = nvgRGB(251, 186, 43);

    return new brls::LibraryViewsThemeVariantsWrapper(custom_light, custom_dark);
}
